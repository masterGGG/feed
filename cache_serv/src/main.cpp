/**
 * @file main.cpp
 * @brief 统计系统服务端框架
 * @author Hansel
 * @version 1.0.0
 * @date 2010-12-15
 */

#include <algorithm>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include "log.h"
#include "i_config.h"
#include "utils.h"
#include "child_proc_type.h"
#include "i_timer.h"

using namespace std;

//版本
const static string g_version = "1.0.0";

// 配置文件列表
const static char g_config_file_list[][PATH_MAX] = {
	"../conf/configure.ini"
};

// 配置文件个数
const static int g_config_file_count = sizeof(g_config_file_list) / PATH_MAX;

volatile static sig_atomic_t g_got_sig_term = 0;
volatile static sig_atomic_t g_got_sig_int = 0;
volatile static sig_atomic_t g_got_sig_chld = 0;
volatile static sig_atomic_t g_got_sig_quit = 0;

const static string g_red_clr = "\e[1m\e[31m";
const static string g_grn_clr = "\e[1m\e[32m";
const static string g_end_clr = "\e[m";


/**
 * @brief 信号处理函数。
 *
 * @param sig 触发的信号。
 */
static void signal_handler(int sig)
{
	switch (sig) {
		case SIGTERM:
			g_got_sig_term = 1;
			break;
		case SIGINT:
			g_got_sig_int = 1;
			break;
		case SIGCHLD:
			g_got_sig_chld = 1;
			break;
		case SIGQUIT:
			g_got_sig_quit = 1;
			break;
		default:
			ERROR_LOG("ERROR: it should never come here!");
			break;
	}
}

/**
 * @brief 获取工作进程数。
 *
 * @param p_config 指向配置实例的指针。
 *
 * @returns 0成功，-1失败。  
 */
int get_work_proc_count(i_config *p_config)
{
	if (p_config == NULL) {
		return -1;
	}

	istringstream iss;
	char buffer[10] = {0};
	int work_proc_count = 0;

	if (p_config->get_config("monitor-proc", "work_proc_count", buffer, sizeof(buffer)) != 0) {
		return -1;
	}

	iss.str(string(buffer));
	iss >> dec >> work_proc_count;
	if (!iss) {
		ERROR_LOG("ERROR: monitor-proc: work_proc_count.");
		return -1;
	}

	return work_proc_count;
}

/**
 * @brief 创建并初始化配置实例。
 *
 * @param config_file_list 配置文件列表。
 * @param config_file_count 配置文件个数。
 * @param pp_config 指向配置实例的二级指针。
 *
 * @returns 0成功，-1失败。 
 */
static int create_and_init_config_instance(const char (*config_file_list)[PATH_MAX],
		int config_file_count,
		i_config **pp_config)
{
	if (config_file_list == NULL || pp_config == NULL) {
		return -1;
	}

	//创建配置接口的实例
	if (create_config_instance(pp_config) != 0) {
		return -1;
	}

	// 初始化配置接口
	if ((*pp_config)->init(config_file_list, config_file_count) != 0) {
		return -1;
	}

	return 0;
}

/**
 * @brief 反初始化并释放配置实例。
 *
 * @param p_config 指向配置实例的指针。
 *
 * @returns 0成功，-1失败。 
 */
static int uninit_and_release_config_instance(i_config *p_config)
{
	if (p_config == NULL) {
		return -1;
	}

	if (p_config->uninit() != 0) {
		ERROR_LOG("ERROR: p_config->uninit().");
		return -1;
	}

	if (p_config->release() != 0) {
		ERROR_LOG("ERROR: p_config->release().");
		return -1;
	}

	return 0;
}

/**
 * @brief 初始化日志模块。
 *
 * @param p_config 指向配置实例的指针。
 *
 * @returns 0成功，-1失败。 
 */
static int init_log(const i_config *p_config)
{
	if (p_config == NULL) {
		return -1;
	}

	istringstream iss;
	char buffer[1024] = {0};
	char log_dir[PATH_MAX] = {0};
	char log_prefix[NAME_MAX] = {0};
	int log_lvl = 0;
	uint32_t log_size = 0;
	int log_count = 0;

	if (p_config->get_config("log", "log_dir", log_dir, sizeof(log_dir)) != 0) {
		cerr << "ERROR: p_config->get_config(\"log\", \"log_dir\", log_dir, sizeof(log_dir))."
			<< endl;
		return -1;
	}

	if (p_config->get_config("log", "log_prefix", log_prefix, sizeof(log_prefix)) != 0) {
		cerr << "ERROR: p_config->get_config(\"log\", \"log_prefix\", log_dir, sizeof(log_dir))."
			<< endl;
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));
	if (p_config->get_config("log", "log_count", buffer, sizeof(buffer)) != 0) {
		cerr << "ERROR: p_config->get_config(\"log\", \"log_count\", buffer, sizeof(buffer))."
			<< endl;
		return -1;
	}
	iss.str(string(iss.str()).append(buffer).append(" "));
	memset(buffer, 0, sizeof(buffer));
	if (p_config->get_config("log", "log_lvl", buffer, sizeof(buffer)) != 0) {
		cerr << "ERROR: p_config->get_config(\"log\", \"log_lvl\", buffer, sizeof(buffer))."
			<< endl;
		return -1;
	}
	iss.str(string(iss.str()).append(buffer).append(" "));
	memset(buffer, 0, sizeof(buffer));
	if (p_config->get_config("log", "log_size", buffer, sizeof(buffer)) != 0) {
		cerr << "ERROR: p_config->get_config(\"log\", \"log_size\", buffer, sizeof(buffer))."
			<< endl;
		return -1;
	}
	iss.str(string(iss.str()).append(buffer));

	iss >> dec >> log_count;
	if (!iss) {
		cerr << "ERROR: log: log_count." << endl;
		return -1;
	}
	iss >> dec >> log_lvl;
	if (!iss) {
		cerr << "ERROR: log: log_lvl." << endl;
		return -1;
	}
	iss >> dec >> log_size;
	if (!iss) {
		cerr << "ERROR: log: log_size." << endl;
		return -1;
	}

	enable_multi_thread();
	if (log_init(log_dir, (log_lvl_t)log_lvl, log_size, log_count, log_prefix) != 0) {
		cerr << "log_init error." << endl;
		return -1;
	}
	enable_multi_thread();
	set_log_dest(log_dest_file);

	return 0;
}

/**
 * @brief 从配置文件获取ringqueue长度。
 *
 * @param p_config 指向配置实例的指针。
 * @param p_ring_queue 指向ring_queue名字的字符指针。
 *
 * @returns 成功返回ring_queue长度，失败返回-1。 
 */
int get_ring_queue_len(i_config *p_config, const char *p_ring_queue)
{
	if (p_config == NULL || p_ring_queue == NULL) {
		return -1;
	}

	istringstream iss;
	char buffer[10] = {0};
	int ring_queue_len = 0;

	if (p_config->get_config("monitor-proc", p_ring_queue, buffer, sizeof(buffer)) != 0) {
		ERROR_LOG("ERROR: p_config->get_config(\"monitor-proc\", \" << p_ring_queue << \", "
				"buffer, sizeof(buffer)).");
		return -1;
	}

	iss.str(string(buffer));
	iss >> dec >> ring_queue_len;
	if (!iss) {
		ERROR_LOG("ERROR: monitor-proc: ring_queue_len.");
		return -1;
	}

	return ring_queue_len;
}

/**
 * @brief 从配置文件获取工作进程ringqueue长度。
 *
 * @param p_config 指向配置实例的指针。
 * @param p_ring_queue 指向ring_queue名字的字符指针。
 *
 * @returns 成功返回ring_queue长度，失败返回-1。 
 */
int get_work_queue_len(i_config *p_config, const char *p_ring_queue)
{
	if (p_config == NULL || p_ring_queue == NULL) {
		return -1;
	}

	istringstream iss;
	char buffer[10] = {0};
	int ring_queue_len = 0;

	if (p_config->get_config("work-proc", p_ring_queue, buffer, sizeof(buffer)) != 0) {
		ERROR_LOG("ERROR: p_config->get_config(\"work-proc\", \" << p_ring_queue << \", "
				"buffer, sizeof(buffer)).");
		return -1;
	}

	iss.str(string(buffer));
	iss >> dec >> ring_queue_len;
	if (!iss) {
		return -1;
	}

	return ring_queue_len;
}

/**
 * @brief  创建进程实例。
 *
 * @param child_proc_vec 存放进程实例数组。
 * @param child_proc_count 进程实例个数。
 * @param p_config 指向配置实例的指针。
 *
 * @returns 0成功，-1失败。
 */
static int create_instance(child_proc_vec_t &work_proc_vec, 
						   child_proc_vec_t &net_proc_vec, 
						   i_config *p_config)
{
	for (int i = 0; i != (int)work_proc_vec.size(); ++i) {
		if (create_work_proc_instance(&(work_proc_vec[i].work_proc.p_work_proc)) != 0) {
			ERROR_LOG("ERROR: create_work_proc_instance.");
			return -1;
		}

		int rv = create_variable_queue_instance(&(work_proc_vec[i].work_proc.
												p_response_queue), 2);
		if (rv != 0) {
			ERROR_LOG("ERROR: create_variable_queue_instance.");
			return -1;
		}

		work_proc_vec[i].work_proc.p_response_queue = create_waitable_queue_instance(
				work_proc_vec[i].work_proc.p_response_queue);

		if (work_proc_vec[i].work_proc.p_response_queue == NULL) {
			ERROR_LOG("ERROR: create_waitable_queue_instance.");
			return -1;
		}

		rv = create_variable_queue_instance(&(work_proc_vec[i].work_proc.p_request_queue), 2);
		if (rv != 0) {
			ERROR_LOG("ERROR: create_variable_queue_instance.");
			return -1;
		}

		work_proc_vec[i].work_proc.p_request_queue = create_waitable_queue_instance(
				work_proc_vec[i].work_proc.p_request_queue);

		if (work_proc_vec[i].work_proc.p_request_queue == NULL) {
			ERROR_LOG("ERROR: create_waitable_queue_instance.");
			return -1;
		}
	}

	if (create_net_proc_instance(&(net_proc_vec[0].net_proc.p_net_proc)) != 0) {
		ERROR_LOG("ERROR: create_net_proc_instance.");
		return -1;
	}

	return 0;
}

/**
 * @brief 释放进程实例。
 *
 * @param work_proc_vec 工作进程对象数组的引用。
 * @param net_proc_vec 网络进程对象数组的引用。
 *
 * @returns 0成功，-1失败。
 */
static int release_instance(child_proc_vec_t &work_proc_vec, child_proc_vec_t &net_proc_vec)
{
	child_proc_vec_t::iterator it = net_proc_vec.begin();
	if (it->net_proc.p_net_proc->release() != 0) {
		ERROR_LOG("ERROR: p_proc_list->net_proc.p_net_proc->release().");
		return -1;
	}

	for (it = work_proc_vec.begin(); it != work_proc_vec.end(); ++it) {
		if (it->work_proc.p_work_proc->release() != 0) {
			ERROR_LOG("ERROR: p_proc_list->work_proc.p_work_proc->release().");
			return -1;
		}
		if (it->work_proc.p_request_queue->release() != 0) {
			ERROR_LOG("ERROR: p_proc_list->work_proc.p_request_queue->release().");
			return -1;
		}
		if (it->work_proc.p_response_queue->release() != 0) {
			ERROR_LOG("ERROR: p_proc_list->work_proc.p_response_queue->release().");
			return -1;
		}
	}

	return 0;
}

/**
 * @brief 初始化工作进程实例。
 *
 * @param work_proc_vec 工作进程对象数组的引用。
 * @param p_config 指向配置实例的引用。
 *
 * @returns 0成功，-1失败。
 */
static int init_work_proc_instance(child_proc_vec_t &work_proc_vec, i_config *p_config)
{
	if (p_config == NULL) {
		ERROR_LOG("ERROR: parameter error.");
		return -1;
	}

	int work_mem_queue_len = get_work_queue_len(p_config, "ring_queue_len");
	if (work_mem_queue_len <= 0) {
		ERROR_LOG("ERROR: get_work_queue_len: ring_queue_len");
		return -1;
	}

	char dbname_prefix[PATH_MAX] = {0};
	if (p_config->get_config("work-proc", "dbname_prefix", dbname_prefix, sizeof(dbname_prefix)) != 0) {
		ERROR_LOG("ERROR: get dbname prefix.");
		return -1;
	}

	child_proc_vec_t::iterator it;
	int i = 0;
	for (it = work_proc_vec.begin(); it != work_proc_vec.end(); ++it, ++i) {
		assert(it->work_proc.p_request_queue != NULL);
		assert(it->work_proc.p_response_queue != NULL);
		assert(it->work_proc.p_work_proc != NULL);

		int work_request_queue_len = get_ring_queue_len(p_config, "request_queue");
		if (work_request_queue_len <= 0) {
			ERROR_LOG("ERROR: get_ring_queue_len: request_queue");
			return -1;
		}

		int work_response_queue_len = get_ring_queue_len(p_config, "response_queue");
		if (work_response_queue_len <= 0) {
			ERROR_LOG("ERROR: get_ring_queue_len: response_queue");
			return -1;
		}

		if (it->work_proc.p_request_queue->init(work_request_queue_len) != 0) {
			ERROR_LOG("ERROR: p_request_queue->init(%d).", work_request_queue_len);
			return -1;
		}
		if (it->work_proc.p_response_queue->init(work_response_queue_len) != 0) {
			ERROR_LOG("ERROR: p_response_queue->init(%d).", work_response_queue_len);
			return -1;
		}

		char dbname[PATH_MAX] = {0};
		int rv = snprintf(dbname, sizeof(dbname), "%s%d", dbname_prefix, i);
		if (rv > (int)sizeof(dbname)) {
			ERROR_LOG("ERROR: db path name is too long.");
			return -1;
		}
		it->child_pid = it->work_proc.p_work_proc->init(p_config, 
														it->work_proc.p_request_queue,
														it->work_proc.p_response_queue,
														work_mem_queue_len,
														dbname,
														strlen(dbname));
		if (it->child_pid <= 0) {
			ERROR_LOG("ERROR: p_proc->work_proc.p_work_proc->init().");
			return -1;
		}
		cout << setw(70) << left << "CACHE_SERVER: work_proc" << g_grn_clr << "[ ok ]" << g_end_clr << endl;
	}

	return 0;
}

/**
 * @brief 初始化网络进程。
 *
 * @param net_proc_vec 网络进程对象数组的引用。
 * @param work_proc_vec 工作进程对象数组的引用。
 * @param p_config 指向配置实例的指针。
 *
 * @returns   
 */
static int init_net_proc_instance(child_proc_vec_t &net_proc_vec, 
								  child_proc_vec_t &work_proc_vec, 
								  i_config *p_config)
{
	if (p_config == NULL) {
		return -1;
	}
	vector<i_ring_queue *> p_request_queue_vec;
	vector<i_ring_queue *> p_response_queue_vec;

	for (child_proc_vec_t::iterator it = work_proc_vec.begin();
		 it != work_proc_vec.end(); 
		 ++it) {
		p_request_queue_vec.push_back(it->work_proc.p_request_queue);
		p_response_queue_vec.push_back(it->work_proc.p_response_queue);
	}

	child_proc_vec_t::iterator nit = net_proc_vec.begin();
	nit->child_pid = nit->net_proc.p_net_proc->init(p_config, 
													work_proc_vec.size(),
													p_request_queue_vec.data(), 
													p_response_queue_vec.data());

	if (net_proc_vec.begin()->child_pid <= 0) {
		ERROR_LOG("ERROR: net_proc: init().");
		return -1;
	}
	return 0;
}

/**
 * @brief 初始化实例。
 *
 * @param work_proc_vec 工作进程对象数组的引用。
 * @param net_proc_vec 网络进程对象数组的引用。
 * @param p_config 指向配置实例的指针。
 *
 * @returns 0成功，-1失败。
 */
static int init_instance(child_proc_vec_t &work_proc_vec, 
						 child_proc_vec_t &net_proc_vec,
						 i_config *p_config)
{
	if (p_config == NULL) {
		ERROR_LOG("ERROR: parameter error.");
		return -1;
	}

	if (init_work_proc_instance(work_proc_vec, p_config) != 0) {
		ERROR_LOG("ERROR: init_work_proc_instance.");
		return -1;
	}

	if (init_net_proc_instance(net_proc_vec, work_proc_vec, p_config) != 0) {
		ERROR_LOG("ERROR: init_net_proc_instance.");
		return -1;
	}

	cout << setw(70) << left << "CACHE_SERVER: net_proc" << g_grn_clr << "[ ok ]" << g_end_clr << endl;

	return 0;
}

/**
 * @brief  反初始化工作进程及网络进程实例。
 *
 * @param work_proc_vec 工作进程实例数组。
 * @param net_proc_vec 网络进程实例数组。
 *
 * @returns 0成功，-1失败。 
 */
static int uninit_instance(child_proc_vec_t &work_proc_vec, 
						   child_proc_vec_t &net_proc_vec)
{
	if (net_proc_vec.begin()->net_proc.p_net_proc->uninit() != 0) {
		ERROR_LOG("ERROR: iter->net_proc.p_net_proc->uninit().");
		return -1;
	}

	for (child_proc_vec_t::iterator it = work_proc_vec.begin(); 
		 it != work_proc_vec.end(); 
		 ++it) {
		if (it->work_proc.p_work_proc->uninit() != 0) {
			ERROR_LOG("ERROR: it->work_proc.p_work_proc->uninit().");
			return -1;
		}
		if (it->work_proc.p_request_queue->uninit() != 0) {
			ERROR_LOG("ERROR: it->work_proc.p_request_queue->uninit().");
			return -1;
		}
		if (it->work_proc.p_response_queue->uninit() != 0) {
			ERROR_LOG("ERROR: it->work_proc.p_response_queue->uninit().");
			return -1;
		}
	}
	return 0;
}

/**
 * @brief 子进程退出时，回收子进程并重启子进程。 
 *
 * @param work_proc_vec 工作进程实例数组。
 * @param net_proc_vec 网络进程实例数组。
 * @param p_config 指向配置实例的指针。
 *
 * @returns 0成功，-1失败。 
 */
static int reboot_children(child_proc_vec_t &work_proc_vec, 
						   child_proc_vec_t &net_proc_vec, 
						   i_config *p_config)
{
	pid_t pid = 0;
	int status = 0;
	int exit_status = 0;
	int term_sig = 0;

	while (1) {
		pid = waitpid(-1, &status, WNOHANG);
		if (pid <= 0) {
			break;
		}
		if (WIFEXITED(status)) {
			exit_status = WEXITSTATUS(status);
			if (exit_status) {
				ERROR_LOG("ERROR: child process %d exited with status %d.", 
						  (int)pid, 
						  exit_status);
			}
		} else if (WIFSIGNALED(status)) {
			term_sig = WTERMSIG(status);
			ERROR_LOG("ERROR: child process %d killed by signal %d.", 
					  (int)pid, 
					  term_sig);
		} else {
			ERROR_LOG("ERROR: child process %d stopped!?", (int)pid);
		}

		int i = 0;
		child_proc_vec_t::iterator iter = work_proc_vec.begin();
		for (; iter != work_proc_vec.end(); ++iter, ++i) {
			if (iter->child_pid == pid) {
				break;
			}
		}
		if (iter != work_proc_vec.end()) {
			set_proc_title("CACHE_SERVER:work_proc:%d:abnormal:exit", (int)pid);

			if (iter->work_proc.p_work_proc->uninit() != 0) {
				ERROR_LOG("ERROR: p_proc->work_proc.p_work_proc->uninit().");
				return -1;
			}

			int work_mem_queue_len = get_work_queue_len(p_config, "ring_queue_len");
			if (work_mem_queue_len <= 0) {
				ERROR_LOG("ERROR: get_work_queue_len: ring_queue_len");
				return -1;
			}

			char dbname_prefix[PATH_MAX] = {0};
			if (p_config->get_config("work-proc", 
									 "dbname_prefix", 
									 dbname_prefix, 
									 sizeof(dbname_prefix)) != 0) {
				ERROR_LOG("ERROR: get dbname prefix.");
				return -1;
			}

			char dbname[PATH_MAX] = {0};
			int rv = snprintf(dbname, sizeof(dbname), "%s%d", dbname_prefix, i);
			if (rv > (int)sizeof(dbname)) {
				ERROR_LOG("ERROR: db path name is too long.");
				return -1;
			}

			iter->child_pid = iter->work_proc.p_work_proc->init(p_config,
																iter->work_proc.p_request_queue,
																iter->work_proc.p_response_queue,
																work_mem_queue_len,
																dbname,
																strlen(dbname));
			if (iter->child_pid <= 0) {
				ERROR_LOG("ERROR: init_work_proc_instance.");
				return -1;
			}
			continue;
		}

		iter = find_if(net_proc_vec.begin(), net_proc_vec.end(), child_proc_item_eq_t(pid));
		if (iter != net_proc_vec.end()) {
			set_proc_title("CACHE_SERVER:net_proc:%d:abnormal:exit", (int)pid);

			if (iter->net_proc.p_net_proc->uninit() != 0) {
				ERROR_LOG("ERROR: p_proc->net_proc.p_net_proc->uninit().");
				return -1;
			}

			if (init_net_proc_instance(net_proc_vec, work_proc_vec, p_config) != 0) {
				ERROR_LOG("ERROR: init_net_proc_instance.");
				return -1;
			}
			continue;
		}
		ERROR_LOG("It should never come here!");
		return -1;
	}

	if (pid < 0 && errno != ECHILD) {
		ERROR_LOG("ERROR: waitpid.");
		return -1;
	}

	return 0;
}

/**
 * @brief 程序入口。 
 *
 * @returns 0成功，-1失败。
 */
int main(int argc, char **argv)
{
	cout << "CACHE_SERVER: version: " << g_version
		<< " build time: " << __DATE__" "__TIME__ << endl;

	// 作为后台程序运行，不改变根路径，不关闭标准终端
	if (daemon(1, 1) != 0) {
		return -1;
	}

	// 判断是否已经运行
	if (already_running() != 0) {
		cerr << g_red_clr << setw(70) << left << "Already running." << "[failed]" << g_end_clr << endl;
		return -1;
	}

	// 上调打开文件数的限制
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
		cerr << "ERROR: getrlimit." << endl;
		return -1;
	}
	rl.rlim_cur = rl.rlim_max;
	if (setrlimit(RLIMIT_NOFILE, &rl) != 0 ) {
		cerr << "ERROR: setrlimit." << endl;
		return -1;
	}

	// 允许产生CORE文件
	if (getrlimit(RLIMIT_CORE, &rl) != 0) {
		cerr << "ERROR: getrlimit." << endl;
		return -1;
	}
	rl.rlim_cur = rl.rlim_max;
	if (setrlimit(RLIMIT_CORE, &rl) != 0) {
		cerr << "ERROR: setrlimit." << endl;
		return -1;
	}

	// 处理信号
	mysignal(SIGCHLD, signal_handler);
	mysignal(SIGTERM, signal_handler);
	mysignal(SIGINT, signal_handler);
	mysignal(SIGQUIT, signal_handler);
	mysignal(SIGPIPE, SIG_IGN);

	// 初始化设置proc标题
	init_proc_title(argc, argv);

	// 设置proc的标题，方便ps时查看进程
	set_proc_title("CACHE_SERVER:monitor_proc");

	// 定义创建并初始化配置接口
	i_config *p_config = NULL;
	if (create_and_init_config_instance(g_config_file_list, g_config_file_count, &p_config) != 0) {
		cerr << g_red_clr << setw(70) << left << "ERROR: init config." << "[failed]"
			<< g_end_clr << endl;
		if (p_config != NULL) {
			p_config->release();
		}
		return -1;
	}

	// 初始化日志模块
	if (init_log(p_config) != 0) {
		cerr << "ERROR: init_log." << endl;
		return -1;
	}

	DEBUG_LOG("CACHE_SERVER: version: %s build time: %s",
			g_version.c_str(), __DATE__" "__TIME__);

	int work_proc_count = get_work_proc_count(p_config);
	child_proc_vec_t work_proc_vec(work_proc_count);
	child_proc_vec_t net_proc_vec(1);

	// 创建接口实例
	if (create_instance(work_proc_vec, net_proc_vec, p_config) != 0) {
		cerr << g_red_clr << setw(70) << left << "in main.cpp:" << endl << "ERROR: create_instance()."
			<< g_end_clr << endl;
		ERROR_LOG("ERROR: create_instance().");
		if (p_config != NULL) {
			p_config->uninit();
			p_config->release();
		}
		release_instance(work_proc_vec, net_proc_vec);
		return -1;
	}
	DEBUG_LOG("SUCCESS: create_instance.");

	// 初始化接口
	if (init_instance(work_proc_vec, net_proc_vec, p_config) != 0) {
		cerr << g_red_clr << setw(70) << left << "ERROR: init_instance." << "[failed]" << g_end_clr << endl;
		ERROR_LOG("ERROR: init_instance.");
		if (p_config != NULL) {
			p_config->uninit();
			p_config->release();
		}
		if (uninit_instance(work_proc_vec, net_proc_vec) != 0) {
			ERROR_LOG("ERROR: uninit_instance().");
			return -1;
		}
		release_instance(work_proc_vec, net_proc_vec);
		return -1;
	}
	DEBUG_LOG("SUCCESS: init_instance.");
	cout << setw(70) << left << "CACHE_SERVER: monitor_proc" << g_grn_clr << "[ ok ]" << g_end_clr << endl;

	// 重定向标准输入输出到/dev/null
	int null_fd = open("/dev/null", O_RDWR);
	if (null_fd == -1) {
		ERROR_LOG("ERROR: open(\"/dev/null\", O_RDWR).");
		return -1;
	}
	dup2(null_fd, 0);
	dup2(null_fd, 1);
	dup2(null_fd, 2);
	close(null_fd);

	// 主循环
	DEBUG_LOG("Enter main loop.");
	while (!g_got_sig_term) {
		if (g_got_sig_chld) {
			g_got_sig_chld = 0;
			if (reboot_children(work_proc_vec, net_proc_vec, p_config) != 0) {
				ERROR_LOG("ERROR: reboot_children.");
				return -1;
			}
		}
		if (g_got_sig_int) {
			g_got_sig_int = 0;
			DEBUG_LOG("got_sig_int.");
		}
		if (g_got_sig_quit) {
			g_got_sig_quit = 0;
			DEBUG_LOG("got_sig_quit.");
		}
		pause();
	}
	DEBUG_LOG("Leave main loop.");

	// 反初始化接口
	if (uninit_instance(work_proc_vec, net_proc_vec) != 0) {
		ERROR_LOG("ERROR: uninit_instance().");
		return -1;
	}

	// 释放接口
	if (release_instance(work_proc_vec, net_proc_vec) != 0) {
		ERROR_LOG("ERROR: release_instance().");
		return -1;
	}

	// 反初始化并释放配置接口实例
	if (uninit_and_release_config_instance(p_config) != 0) {
		ERROR_LOG("ERROR: uninit_and_release_config_instance(p_config).");
		return -1;
	}

	// 反初始化设置proc标题
	uninit_proc_title();

	return 0;
}
