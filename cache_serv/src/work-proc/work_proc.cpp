/**
 * @file work_proc.cpp
 * @brief 工作进程实现 
 * @author Hansel
 * @date 2010-12-16
 */

#include <new>                    // for std::nothrow
#include <cstdlib>                // for exit
#include <iostream>
#include <cstring>                // for memset
#include <sstream>

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "log.h"
#include "utils.h"
#include "proto.h"
#include "work_proc.h"
#include "msglog.h"

using namespace std;

/** 
 * @brief 创建写进程接口实例
 * @param pp_instance
 * @return 0成功 -1失败
 */
int create_work_proc_instance(i_work_proc **pp_instance)
{
	if (pp_instance == NULL) {
		return -1;
	}

	c_work_proc *p_work_proc = new (nothrow)c_work_proc();
	if (p_work_proc == NULL) {
		return -1;
	}

	*pp_instance = dynamic_cast<i_work_proc *>(p_work_proc);

	return 0;
}

/** 
 * @brief 构造函数
 * @param 
 * @return
 */
c_work_proc::c_work_proc() 
{
	m_p_config = NULL;
	m_p_request_queue = NULL;
	m_p_response_queue = NULL;
	m_p_cache_queue = NULL;
	m_p_timer = NULL;
	m_p_proc_ctl = NULL;

	m_msgid=0;
	m_namelen = 0;
	m_log_interval = 0;
	m_cache_mem_len = 0;
}

/** 
 * @brief 析构函数
 * @param
 * @return
 */
c_work_proc::~c_work_proc()
{
	if (m_p_proc_ctl != NULL) {
		uninit();
	}
}

/** 
 * @brief 初始化函数
 * @param p_config
 * @param p_request_queue
 * @param p_response_queue
 * @return 0成功 -1失败
 */
int c_work_proc::init(i_config *p_config, i_ring_queue *p_request_queue, i_ring_queue *p_response_queue, int cache_mem_len, const char *dbname, int namelen)
{
	if (m_p_proc_ctl != NULL) {
		return -1;
	}

	if (p_config == NULL || p_request_queue == NULL || p_response_queue == NULL) {
		return -1;
	}

	m_p_config = p_config;
	m_p_request_queue = p_request_queue;
	m_p_response_queue = p_response_queue;
	m_cache_mem_len = cache_mem_len;
	m_namelen = namelen;
	memcpy(m_dbname, dbname, m_namelen);

	if (get_stat_log_info(m_logfile, (int)sizeof(m_logfile), &m_log_interval, &m_msgid) != 0) {
		ERROR_LOG("ERROR: get_stat_log_info().");
		return -1;
	}

	if (create_timer_instance(&m_p_timer) != 0) {
		ERROR_LOG("ERROR: create_timer_instance.");
		return -1;
	}

	if (m_p_timer->init() != 0) {
		ERROR_LOG("ERROR: m_p_timer()->init().");
		return -1;
	}

	m_p_proc_ctl = (proc_ctl_t *)mmap(NULL, sizeof(proc_ctl_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (m_p_proc_ctl == MAP_FAILED) {
		return -1;
	}
	memset(m_p_proc_ctl, 0, sizeof(*m_p_proc_ctl));

	pid_t pid = fork();
	if (pid < 0) {              // 出错
		ERROR_LOG("ERROR: fork: %s", strerror(errno));
		return -1;   
	} else if (pid == 0) {      // 子进程
		if (child_main() != 0) {
			ERROR_LOG("ERROR: child_main()");
			exit(1);
		}
		exit(0);
	} 

	// 父进程，等待子进程初始化完成
	while (m_p_proc_ctl->inited == 0) {
		sleep(1);
	}

	// 子进程初始化失败
	if (m_p_proc_ctl->inited == -1) {
		return -1;
	}

	// 返回子进程的ID
	return pid;
}

/** 
 * @brief 子进程主函数
 * @param
 * @return 0成功 -1失败
 */
int c_work_proc::child_main()
{
	// 设置proc的标题，方便ps时查看进程
	set_proc_title("CACHE_SERVER:work_proc");

	// 子进程忽略所有的信号
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = SIG_IGN;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(SIGTERM, &act, NULL) != 0) {
		m_p_proc_ctl->inited = -1;
		return -1;
	}
	if (sigaction(SIGINT, &act, NULL) != 0) {
		m_p_proc_ctl->inited = -1;
		return -1;
	}
	if (sigaction(SIGQUIT, &act, NULL) != 0) {
		m_p_proc_ctl->inited = -1;
		return -1;
	}
	if (sigaction(SIGPIPE, &act, NULL) != 0) {
		m_p_proc_ctl->inited = -1;
		return -1;
	}

	if (create_cache_queue_instance(&m_p_cache_queue)) {
		ERROR_LOG("ERROR: create_cache_queue_instance.");
		m_p_proc_ctl->inited = -1;
		return -1;
	}

	if (m_p_cache_queue->init(m_cache_mem_len, m_dbname, m_namelen) != 0) {
		m_p_cache_queue->release();
		m_p_proc_ctl->inited = -1;
		ERROR_LOG("ERROR: m_p_cache_queue->init().");
		return -1;
	}

	i_timer::timer_id_t timer_id = m_p_timer->add(m_log_interval, c_work_proc::stat_log_wrapper, this);
	if (timer_id == -1) {
		ERROR_LOG("ERROR: m_p_timer->add(stat_log_wrapper).");
		return -1;
	}

	// 子进程初始化完成
	m_p_proc_ctl->inited = 1;

	time_t last_check_time;
	time(&last_check_time);

	// 子进程进入主循环
	DEBUG_LOG("work_proc enter main loop.");
	while (!m_p_proc_ctl->terminal) {
		m_p_proc_ctl->heartbeat = time(NULL);

		if(time(NULL) - last_check_time >= 1)
		{
			time(&last_check_time);        
			if(m_p_timer->check() != 0)   
			{
				ERROR_LOG("m_p_timer->check().");
			}
		}

		if (deal_request_msg() != 0) {
			ERROR_LOG("ERROR: deal_request_msg.");
			// XXX 出错继续执行
		}
	}

	if (m_p_cache_queue->uninit() != 0) {
		ERROR_LOG("ERROR: m_p_cache_queue->uninit().");
		return -1;
	}

	if (m_p_cache_queue->release() != 0) {
		ERROR_LOG("ERROR: m_p_cache_queue->release().");
		return -1;
	}

	DEBUG_LOG("WORK_PROC leave main loop.");
	m_p_proc_ctl->terminal = 2;

	return 0;
}

/** 
 * @brief 处理请求消息
 * @param
 * @return 0成功 -1失败
 */
int c_work_proc::deal_request_msg()
{
	memset(m_request_buffer, 0, sizeof(m_request_buffer));
	memset(m_response_buffer, 0, sizeof(m_response_buffer));

	int rv = m_p_request_queue->pop_data(m_request_buffer, sizeof(m_request_buffer), 1);
	int connection_id = 0;

	if (rv > 0) {
		MSG_LEN(m_request_buffer) -= sizeof(connection_id);
		memcpy(&connection_id, m_request_buffer + MSG_LEN(m_request_buffer), sizeof(connection_id));

		int result = 0;
		if (CMD_ID(m_request_buffer) == SYSTEM_MSG) {
			result = deal_task_request(connection_id);
		}
		else {
			result = deal_so_request(connection_id);
		}

		if (result == -1) {
			ERROR_LOG("ERROR: deal with so or task request.");
			return -1;
		}
		else if (result > 0){
			int rv = m_p_response_queue->push_data(m_response_buffer, result, 1);
			if (rv != result) {
				ERROR_LOG("ERROR: m_p_response_queue->push_data(%d).rv:%d", result, rv);
				return -1;
			}
		}
		else {
			// do nothing.
		}
	} else if (rv == 0){
		return 0;
	} else {
		ERROR_LOG("ERROR: m_p_request_queue->pop_data.");
		return -1;
	}
	return 0;
}

int c_work_proc::deal_task_request(int connection_id)
{
	task_feed_response_msg_t *p_feed_response_msg = (task_feed_response_msg_t *)m_response_buffer;

	if (CMD_ID(m_request_buffer) == SYSTEM_MSG) {
		int rv = m_p_cache_queue->pop(m_response_buffer, sizeof(m_response_buffer));	
		if (rv > 0) {
			//do nothing.
		}
		else if (rv == 0) {
			p_feed_response_msg->length = sizeof(task_feed_response_msg_t);
			p_feed_response_msg->cmd_id = 0xFFFF;
		}
		else {
			ERROR_LOG("m_p_cache_queue->pop().");
			return -1;
		}
	}
	else {
		return -1;
	}
	memcpy(m_response_buffer + MSG_LEN(m_response_buffer), &connection_id, sizeof(connection_id));
	MSG_LEN(m_response_buffer) += sizeof(connection_id);

	return MSG_LEN(m_response_buffer);
}

int c_work_proc::deal_so_request(int connection_id)
{
	int rv = m_p_cache_queue->push(m_request_buffer, MSG_LEN(m_request_buffer));
	if (rv > 0) {
		// do nothing.
	}
	else {
		ERROR_LOG("push into cache queue failed,rv: %d length:%d", rv, MSG_LEN(m_request_buffer));
		return -1;
	}

	return 0;
}

int c_work_proc::get_stat_log_info(char *logfile,  int data_len, int *interval, uint32_t *msg_id) 
{
	if (logfile == NULL || m_p_config == NULL || interval == NULL || msg_id == NULL) { 
		return -1; 
	} 

	if (m_p_config->get_config("stat", "logfile", logfile, data_len) != 0) { 
		ERROR_LOG("1");
		return -1; 
	} 

	char buf[16] = {0}; 
	if (m_p_config->get_config("stat", "msg_id", buf, sizeof(buf)) != 0) { 
		ERROR_LOG("2");
		return -1; 
	} 
	sscanf(buf, "%x", msg_id); 

	memset(buf, 0, sizeof(buf));
	if (m_p_config->get_config("stat", "log_interval", buf, sizeof(buf)) != 0) {
		ERROR_LOG("3");
		return -1;
	}
	sscanf(buf, "%d", interval);

	DEBUG_LOG("msg_id:%#x, log_interval:%d, logfile:%s", *msg_id, *interval, logfile);

	return 0; 
} 

int c_work_proc::stat_log()
{
	if (m_p_cache_queue == NULL) {
		return -1;
	}

	int tmp_buf[3] = {0};
	tmp_buf[0] = m_p_cache_queue->get_mem_feed_num();
	tmp_buf[1] = m_p_cache_queue->get_db_feed_num();
	tmp_buf[2] = tmp_buf[0] + tmp_buf[1];

	if ((tmp_buf[0] == -1) || (tmp_buf[1] == -1)) {
		ERROR_LOG("ERROR: get feed num error.");
		return -1;
	}

	if (msglog(m_logfile, m_msgid, time(NULL), tmp_buf, sizeof(tmp_buf)) != 0) {
		ERROR_LOG("ERROR: msglog().");
		return -1;
	}

	return 0;
}

/** 
 * @brief 反初始化函数
 * @param
 * @return 0成功 -1失败
 */
int c_work_proc::uninit()
{
	if (m_p_proc_ctl == NULL) {
		return -1;
	}

	// 通知子进程结束运行
	m_p_proc_ctl->terminal = 1;

	// 等待子进程运行结束
	while (m_p_proc_ctl->terminal != 2) {
		if (time(NULL) - m_p_proc_ctl->heartbeat > HEARTBEAT_TIMEOUT) {
			ERROR_LOG("ERROR: heartbeat_timeout.");
			break;
		}
		sleep(1);
	}

	if (munmap(m_p_proc_ctl, sizeof(*m_p_proc_ctl)) != 0) {
		return -1;
	}
	if (m_p_timer->uninit() != 0) {
		ERROR_LOG("ERROR: m_p_timer->uninit().");
		return -1;
	}
	if (m_p_timer->release() != 0) {
		ERROR_LOG("ERROR: m_p_timer->release().");
		return -1;
	}

	m_p_proc_ctl = NULL;
	m_p_timer = NULL;

	return 0;
}

int c_work_proc::release()
{
	delete this;

	return 0;
}

int c_work_proc::stat_log_wrapper(void *p_obj) 
{ 
	c_work_proc *p_work_proc = (c_work_proc *)(p_obj); 
	return p_work_proc->stat_log(); 
}
