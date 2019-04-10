/**
 * @file net_proc.cpp
 * @brief 网络进程实现 
 * @author Hansel
 * @date 2010-10-16
 */

#include <new>
#include <cstdlib>
#include <cstring>

using namespace std;

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "log.h"
#include "utils.h"
#include "net_proc.h"
#include "proto.h"
#include "child_proc_type.h"


int create_net_proc_instance(i_net_proc **pp_instance)
{
	if (pp_instance == NULL) {
		return -1;
	}

	c_net_proc *p_net_proc = new (nothrow)c_net_proc();
	if (p_net_proc == NULL) {
		return -1;
	}

	*pp_instance = dynamic_cast<i_net_proc *>(p_net_proc);

	return 0;
}

c_net_proc::c_net_proc()
{
	m_p_proc_ctl = NULL;
	m_p_config = NULL;
	m_p_net_io = NULL;
	m_work_proc_count = 0;
}

c_net_proc::~c_net_proc()
{
	if (m_p_proc_ctl != NULL) {
		uninit();
	}
}

/**
 * @brief  网络进程初始化
 *
 * @param p_config 指向配置实例指针
 * @param work_proc_count 工作进程数
 * @param pp_request_queue 指向请求环形队列列表指针
 * @param pp_response_queue 指向回复环形队列列表指针
 *
 * @returns  0成功，-1失败 
 */
int c_net_proc::init(i_config *p_config, int work_proc_count, i_ring_queue **pp_request_queue, i_ring_queue **pp_response_queue)
{
	if (m_p_proc_ctl != NULL) {
		return -1;
	}

	if (p_config == NULL || pp_response_queue == NULL || pp_request_queue == NULL) {
		return -1;
	}

	m_p_config = p_config;
	m_work_proc_count = work_proc_count;

	for (int i = 0; i != work_proc_count; ++i) {
		static queue_info_t queue_info;
		queue_info.used_flag = 0;
		queue_info.p_queue = pp_request_queue[i];
		m_request_queue_vec.push_back(queue_info);
		queue_info.p_queue = pp_response_queue[i];
		m_response_queue_vec.push_back(queue_info);
	}

	m_p_proc_ctl = (proc_ctl_t *)mmap(NULL, sizeof(proc_ctl_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (m_p_proc_ctl == MAP_FAILED) {
		return -1;
	}
	memset(m_p_proc_ctl, 0, sizeof(*m_p_proc_ctl));

	pid_t pid = fork();
	if (pid < 0) { 
		ERROR_LOG("ERROR: fork: %s", strerror(errno));
		return -1;
	} else if (pid == 0) {
		child_main();
		exit(0);
	}

	while (m_p_proc_ctl->inited == 0) {
		sleep(1);
	}

	if (m_p_proc_ctl->inited == -1) {
		return -1;
	}

	return pid;
}

/**
 * @brief  子进程主函数
 *
 * @returns  0成功，-1失败。 
 */
int c_net_proc::child_main()
{
	set_proc_title("CACHE_SERVER:net_proc");

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

	char listen_ip[16] = {0};
	char listen_port_buffer[6] = {0};
	if (m_p_config->get_config("net-proc", "listen_ip", listen_ip, sizeof(listen_ip)) != 0 ||
			m_p_config->get_config("net-proc", "listen_port", listen_port_buffer, sizeof(listen_port_buffer)) != 0) {
		ERROR_LOG("ERROR: m_p_config->get_config.");
		m_p_proc_ctl->inited = -1;
		return -1;
	}
	int listen_port = atoi(listen_port_buffer);
	if (listen_port < 0 || listen_port > 65535) {
		ERROR_LOG("ERROR: illegal listen_port.");
		m_p_proc_ctl->inited = -1;
		return -1;
	}

	if (net_io_server_create(&m_p_net_io) != 0) {
		ERROR_LOG("ERROR: create_net_io_instance.");
		m_p_proc_ctl->inited = -1;
		return -1;
	}

	if (m_p_net_io->init(listen_ip, listen_port, this, NULL, 1, 1) != 0) {
		ERROR_LOG("ERROR: p_net_io->init.");
		m_p_net_io->uninit();
		m_p_net_io->release();
		m_p_proc_ctl->inited = -1;
		return -1;
	}

	// 子进程初始化完成
	m_p_proc_ctl->inited = 1;

	// 子进程进入主循环
	DEBUG_LOG("NET_PROC enter main loop.");
	while (!m_p_proc_ctl->terminal) {
		m_p_proc_ctl->heartbeat = time(NULL);
		m_p_net_io->do_io(EPOLL_WAIT_TIME, NET_IO_SERVER_CMD_ACCEPT | NET_IO_SERVER_CMD_READ | NET_IO_SERVER_CMD_WRITE);
		if (deal_response_msg() == -1) {
			ERROR_LOG("ERROR: deal_response_msg.");
		}
		if (send_response_buffer() == -1) {
			ERROR_LOG("ERROR: send_response_buffer.");
			return -1;
		}
	}
	DEBUG_LOG("NET_PROC leave main loop.");

	m_p_net_io->uninit();
	m_p_net_io->release();

	// 等待队列中的所有消息都被处理
	for (queue_info_vec_t::iterator iter = m_request_queue_vec.begin();
			iter != m_request_queue_vec.end(); 
			++iter) {
		while (iter->p_queue->get_data_len() > 0) {
			m_p_proc_ctl->heartbeat = time(NULL);
			sleep(1);
		}
	}

	// 发送最后一批回复消息。
	if (deal_response_msg() == -1) {
		ERROR_LOG("ERROR: deal_response_msg.");
	}
	if (send_response_buffer() == -1) {
		ERROR_LOG("ERROR: send_response_buffer.");
		return -1;
	}

	m_p_proc_ctl->terminal = 2;

	return 0;
}

/**
 * @brief  反初始化
 *
 * @returns  0成功，-1失败 
 */
int c_net_proc::uninit()
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
	m_p_proc_ctl = NULL;

	return 0;
}

/**
 * @brief  释放资源
 *
 * @returns   
 */
int c_net_proc::release()
{
	delete this;

	return 0;
}

/**
 * @brief 处理task请求
 *
 * @param p_msg 指向数据缓冲区指针
 * @param response_buf 返回回复消息缓冲区指针
 * @param connection_id 连接标识符
 *
 * @returns  成功返回回复消息长度，0表示没有回复消息，-1失败(会导致关闭对端连接) 
 */
int c_net_proc::deal_task_request(const char *p_msg, int connection_id)
{
	if (p_msg == NULL) {
		return -1;
	}

	if (SUB_CMD(p_msg) == CONN_REQUEST) {
		return deal_task_conn_request(p_msg, connection_id);
	}
	else if (SUB_CMD(p_msg) == FEED_REQUEST) {
		return deal_task_feed_request(p_msg, connection_id);
	}
	else {
		ERROR_LOG("invalid sub_cmd.");
		return -1;
	}

	// 不会执行到此处
	return 0;
}

int c_net_proc::deal_task_conn_request(const char *p_msg, int connection_id)
{
	if (p_msg == NULL) {
		return -1;
	}

	memset(m_response_buffer, 0, sizeof(m_response_buffer));

	task_conn_response_msg_t *p_response_msg = (task_conn_response_msg_t *)m_response_buffer;
	if (m_task_map.size() == m_work_proc_count) {
		p_response_msg->length = sizeof(task_conn_response_msg_t);
		p_response_msg->task_id = 0xFFFF;
		return p_response_msg->length;
	}
	else if(m_task_map.size() < m_work_proc_count) {
		pair<queue_info_map_t::iterator, bool> ret;
		queue_info_vec_t::iterator it = m_request_queue_vec.begin();
		for ( ; it != m_request_queue_vec.end(); ++it) {
			if (it->used_flag == 0) {
				ret = m_task_map.insert(make_pair(connection_id, it));
				if (!ret.second) {
					return -1;
				}
				it->used_flag = 1;
				p_response_msg->length = sizeof(task_conn_response_msg_t);
				p_response_msg->task_id = ret.first->first;

				DEBUG_LOG("task with connection_id: %d dispatched.", connection_id);
				return p_response_msg->length;
			}
		}
	}
	else {
		ERROR_LOG("ERROR: task count is more than work proc count!!");
		return -1;
	}

	// 不会执行到此处
	return 0;
}

int c_net_proc::deal_task_feed_request(const char *p_msg, int connection_id)
{
	if (p_msg == NULL) {
		return -1;
	}

	memset(m_request_buffer, 0, sizeof(m_request_buffer));

	task_feed_request_msg_t *p_feed_request_msg = (task_feed_request_msg_t *)p_msg;
	if (p_feed_request_msg->task_id != connection_id) {
		ERROR_LOG("task with connection_id: %d asked for task: %d's data", connection_id, p_feed_request_msg->task_id);
		return -1;
	}
	queue_info_map_t::iterator it = m_task_map.find(p_feed_request_msg->task_id);	
	if (it == m_task_map.end()) {
		ERROR_LOG("task %d is not registered.", p_feed_request_msg->task_id);
		return -1;
	}

	memcpy(m_request_buffer, p_msg, MSG_LEN(p_msg));
	memcpy(m_request_buffer + MSG_LEN(p_msg), &connection_id, sizeof(connection_id));
	MSG_LEN(m_request_buffer) += sizeof(connection_id);

	int ret = it->second->p_queue->push_data(m_request_buffer, MSG_LEN(m_request_buffer), 1);
	if(ret != MSG_LEN(m_request_buffer)) {
		ERROR_LOG("ERROR: it->second->push_data().");
		return -1;
	}
	return 0;
}

/**
 * @brief  处理so请求
 *
 * @param p_msg 指向消息缓冲区指针
 * @param response_buf 返回回复包缓冲区指针
 * @param connection_id 连接标识符
 *
 * @returns 成功返回需要回复的字节数，0表示无回复消息，-1失败(会导致关闭对端连接)   
 */
int c_net_proc::deal_so_request(const char *p_msg, int connection_id)
{
	if (p_msg == NULL) {
		return -1;
	}

	memset(m_response_buffer, 0, sizeof(m_response_buffer));
	memset(m_request_buffer, 0, sizeof(m_request_buffer));

	memcpy(m_request_buffer, p_msg, MSG_LEN(p_msg));
	memcpy(m_request_buffer + MSG_LEN(p_msg), &connection_id, sizeof(connection_id));
	MSG_LEN(m_request_buffer) += sizeof(connection_id);

	so_request_msg_t *p_so_request_msg = (so_request_msg_t *)p_msg;
	int index = p_so_request_msg->user_id % m_work_proc_count;
	int ret = m_request_queue_vec[index].p_queue->push_data(m_request_buffer, MSG_LEN(m_request_buffer), 1);

	so_response_msg_t *p_so_response_msg = (so_response_msg_t *)m_response_buffer;
	p_so_response_msg->length = sizeof(so_response_msg_t);
	if (ret != MSG_LEN(m_request_buffer)) {
		ERROR_LOG("index:%d request queue can't deal with so request, memory is full.", index);
		p_so_response_msg->result = 0xFFFF;
	}
	else {
		p_so_response_msg->result = 0;
	}
	return p_so_response_msg->length;
}

/**
 * @brief  处理请求消息
 *
 * @param p_msg 指向数据缓冲区指针
 * @param connection_id 连接标识符
 *
 * @returns  0成功，-1失败(会导致关闭对端连接) 
 */
int c_net_proc::deal_request_msg(const char *p_msg, int connection_id)
{
	if (p_msg == NULL) {
		return -1;
	}

	conn_info_map_t::iterator cmap_iter = m_conn_map.find(connection_id);

	if (m_work_proc_count == 0) {
		if (connection_id < 0) {
			return -1;
		}
		if (cmap_iter == m_conn_map.end()) {
			return -1;
		}
		m_p_net_io->close_connection(connection_id, 1);
	}
	else {
		int ret = 0;
		if (CMD_ID(p_msg) == SYSTEM_MSG) {
			ret = deal_task_request(p_msg, connection_id);
		}
		else {
			ret = deal_so_request(p_msg, connection_id);
		}

		if (ret == -1) {
			return -1;
		}

		if (send_data(connection_id, m_response_buffer, ret) != 0) {
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  处理所有回复环形队列的消息，仅当处理完才退出
 *
 * @returns   
 */
int c_net_proc::deal_response_msg()
{
	int connection_id = 0;
	for (unsigned int i = 0; i != m_work_proc_count; ++i) {
		for(;;) {
			memset(m_response_buffer, 0, sizeof(m_response_buffer));
			int pop_len = m_response_queue_vec[i].p_queue->pop_data(m_response_buffer, sizeof(m_response_buffer), 1);
			if (pop_len == 0) {
				break;
			}
			else if (pop_len == -1) {
				ERROR_LOG("ERROR: pop_data.");
				return -1;
			}
			MSG_LEN(m_response_buffer) -= sizeof(connection_id);
			memcpy(&connection_id, m_response_buffer + MSG_LEN(m_response_buffer), sizeof(connection_id));

			if (send_data(connection_id, m_response_buffer, MSG_LEN(m_response_buffer)) != 0) {
				ERROR_LOG("ERROR: send_data.");
				m_p_net_io->close_connection(connection_id, 1);
			}
		}
	}
	return 0;
}

/**
 * @brief  把每个连接回复缓冲区中剩余数据交给net-io模块
 *
 * @returns   
 */
int c_net_proc::send_response_buffer()
{
	conn_info_map_t::iterator iter = m_conn_map.begin();
	for (; iter != m_conn_map.end(); ++iter) {
		if (iter->second.send_buffer_data_len > 0) {
			int bytes_send = m_p_net_io->send_data(iter->first, iter->second.send_buffer, iter->second.send_buffer_data_len);
			if (bytes_send == -1) {
				m_p_net_io->close_connection(iter->first, 1);
				return -1;
			}
			if ((iter->second.send_buffer_data_len - bytes_send) > 0) {
				memmove(iter->second.send_buffer, iter->second.send_buffer + bytes_send, iter->second.send_buffer_data_len - bytes_send);
			}
			iter->second.send_buffer_data_len -= bytes_send;
		}
	}

	return 0;
}

/**
 * @brief 把回复消息交给底层net-io模块
 *
 * @param connection_id 连接标识符
 * @param p_data 待发送数据
 * @param data_len 数据长度
 *
 * @returns  0成功，-1失败(失败将导致关闭对端连接) 
 */
int c_net_proc::send_data(int connection_id, char *p_data, int data_len)
{
	if (data_len == 0) {
		return 0;
	}

	conn_info_map_t::iterator iter = m_conn_map.find(connection_id);
	if (iter == m_conn_map.end()) {
		ERROR_LOG("ERROR: connection_id %d not found.", connection_id);
		return -1;
	}

	int data_sent = 0;
	if (iter->second.send_buffer_data_len == 0) {
		data_sent = m_p_net_io->send_data(connection_id, p_data, data_len);
		if (data_sent == -1) {
			ERROR_LOG("data_sent == -1");
			return -1;
		}
	}

	int data_remain = data_len - data_sent;
	int buf_empty = sizeof(iter->second.send_buffer) - iter->second.send_buffer_data_len;

	if (data_remain > 0) {
		if (data_remain > buf_empty) {
			ERROR_LOG("ERROR: empty buffer data_remain: %d > buf_empty: %d.", data_remain, buf_empty);
			return -1;
		}

		memcpy(iter->second.send_buffer + iter->second.send_buffer_data_len, p_data + data_sent, data_remain);
		iter->second.send_buffer_data_len += data_remain;
	}

	return 0;
}

/**
 * @brief  net-io模块收到数据时触发
 *
 * @param p_net_io  指向net-io实例
 * @param connection_id 连接标识符
 * @param p_data 指向收到数据缓冲区指针
 * @param data_len 数据长度
 *
 * @returns  0成功，-1失败(失败会导致关闭对端连接) 
 */
int c_net_proc::on_recv_data(void *p_net_io, int connection_id, int fd, const char *ip, int port, char *p_data, int data_len, union net_io_storage *p_storage)
{
	if (p_net_io == NULL || connection_id < 0 || p_data == NULL || data_len < 0) {
		return -1;
	}

	conn_info_map_t::iterator iter = m_conn_map.find(connection_id);
	if (iter == m_conn_map.end()) {
		ERROR_LOG("ERROR: unknown connection_id: %d.", connection_id);
		return -1;
	}

	int *mem_used = &(iter->second.recv_buffer_data_len);
	int mem_empt = 0; 
	int mem_move = 0;
	int loop_count = 0;

	while (data_len != 0) {
		mem_empt = sizeof(iter->second.recv_buffer) - *mem_used;

		if (mem_empt >= data_len) {
			mem_move = data_len;
		}
		else {
			mem_move = mem_empt;
		}
		memcpy(iter->second.recv_buffer + *mem_used, p_data, mem_move);
		*mem_used += mem_move;
		p_data += mem_move;
		data_len -= mem_move;

		if ((*mem_used >= 2) && (MSG_LEN(iter->second.recv_buffer) > MAX_BUF_LEN - sizeof(connection_id))) {
			ERROR_LOG("pack length is too long. len: %d, cmd_id: %d, loop_count: %d, data_len: %d, *mem_used: %d, mem_move: %d", MSG_LEN(iter->second.recv_buffer), CMD_ID(iter->second.recv_buffer), loop_count, data_len, *mem_used, mem_move);
			return -1;
		}

		if ((*mem_used >= 2) && MSG_LEN(iter->second.recv_buffer) == 0) {
			ERROR_LOG("pack length shouldn't be zero.");
			return -1;
		}

		while(*mem_used >=2 && *mem_used >= MSG_LEN(iter->second.recv_buffer)) {
			if (deal_request_msg(iter->second.recv_buffer, connection_id) != 0) {
				ERROR_LOG("ERROR: deal_request_msg().");
				//m_p_net_io->close_connection(connection_id, 1);
				return -1;
			}
			*mem_used -= MSG_LEN(iter->second.recv_buffer);
			memmove(iter->second.recv_buffer, iter->second.recv_buffer + MSG_LEN(iter->second.recv_buffer), sizeof(iter->second.recv_buffer) - MSG_LEN(iter->second.recv_buffer));
		}
		++loop_count;
	}

	return 0;
}

/**
 * @brief  收到新连接时触发
 *
 * @param p_net_io 指向net-io实例指针
 * @param connection_id 连接标识符
 * @param peer_ip 对端IP(点分十进制表示)
 * @param peer_port 对端端口(主机字节序)
 *
 * @returns  0成功，-1失败(失败会导致关闭对端连接) 
 */
int c_net_proc::on_new_connection(void *p_net_io, int connection_id, int fd, const char *peer_ip, int peer_port, union net_io_storage *p_storage)
{
	if (p_net_io == NULL || peer_ip == NULL || connection_id < 0 || peer_port < 0) {
		return -1;
	}

	DEBUG_LOG("New connection from: %s:%d at %d.", peer_ip, peer_port, connection_id);
	conn_info_map_t::iterator iter = m_conn_map.find(connection_id);
	if (iter != m_conn_map.end()) {
		ERROR_LOG("It should never come here!");
		return -1;
	}

	conn_info_t conn_info = {{'0'},0,{'0'},0,{'0'},0};
	strncpy(conn_info.peer_ip, peer_ip, sizeof(conn_info.peer_ip));
	conn_info.peer_port = peer_port;
	conn_info.recv_buffer_data_len = 0;
	conn_info.send_buffer_data_len = 0;
	m_conn_map.insert(make_pair(connection_id, conn_info));

	return 0;
}

/**
 * @brief  连接关闭时触发。
 *
 * @param p_net_io 指向net-io实例指针
 * @param connection_id 连接标识符
 *
 * @returns  0成功，-1失败 
 */
int c_net_proc::on_connection_closed(void *p_net_io, int connection_id, int fd, const char *ip, int port, union net_io_storage *p_storage)
{
	if (p_net_io == NULL || connection_id < 0) {
		return -1;
	}

	conn_info_map_t::iterator iter = m_conn_map.find(connection_id);
	if (iter == m_conn_map.end()) {
		ERROR_LOG("It should never come here.");
		return -1;
	}

	queue_info_map_t::iterator it = m_task_map.find(connection_id);
	if (it != m_task_map.end()) {
		DEBUG_LOG("Session with task %d closed.", connection_id);
		it->second->used_flag = 0;
	}

	DEBUG_LOG("Connection with %s:%d at %d is closed.", iter->second.peer_ip, iter->second.peer_port,
			connection_id);

	m_conn_map.erase(connection_id);
	m_task_map.erase(connection_id);

	return 0;
}

int c_net_proc::on_wakeup(void *p_net_io_server)
{
	return 0;
}
