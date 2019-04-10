/* vim: set tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file net_proc.h
 * @author richard <richard@taomee.com>
 * @author hansel <hansel@taomee.com>
 * @date 2010-04-28
 */

#ifndef NET_PROC_2010_04_28
#define NET_PROC_2010_04_28

#include <vector>
#include <map>

#include <sys/types.h>            // for pid_t

#include "defines.h"
#include "i_net_io_server.h"
#include "child_proc_type.h"


class c_net_proc : public i_net_proc, public i_net_io_event_handler
{
public:
	c_net_proc();
	virtual ~c_net_proc();

	// 从i_net_proc继承
	virtual int init(i_config *p_config, int work_proc_count, i_ring_queue **pp_request_queue, i_ring_queue **pp_response_queue);
	virtual int uninit();
	virtual int release();

	// 从i_net_io_events继承
	virtual int on_new_connection(void *p_net_io, int connection_id, int fd, const char *peer_ip, int peer_port, union net_io_storage *p_storage);
	virtual int on_recv_data(void *p_net_io, int connection_id, int fd, const char *ip, int port, char *p_data, int data_len, union net_io_storage *p_storage);
	virtual int on_connection_closed(void *p_net_io, int connection_id, int fd, const char *ip, int port, union net_io_storage *p_storage);
	virtual int on_wakeup(void *p_net_io_server);

protected:
	int child_main();
	int deal_request_msg(const char *p_msg, int connection_id);
	int deal_task_request(const char *p_msg, int connection_id);
	int deal_task_conn_request(const char *p_msg, int connection_id);
	int deal_task_feed_request(const char *p_msg, int connection_id);
	int deal_so_request(const char *p_msg, int connection_id);
	int deal_response_msg();
	int send_response_buffer();
	int send_data(int connection_id, char *p_data, int data_len);

private:
	typedef struct {
		volatile int inited;      // 初始化的状态(0：未初始化，1：初始化成功，－1：初始化失败)
		volatile int terminal;    // 子进程是否终止的状态(0：不终止，1：通知子进程终止，2：子进程终止完成)
		volatile time_t heartbeat;// 子进程的心跳
	} proc_ctl_t;

	typedef struct {
		char peer_ip[16];
		int peer_port;
		char recv_buffer[MAX_BUF_LEN];
		int recv_buffer_data_len;
		char send_buffer[MAX_BUF_LEN];
		int send_buffer_data_len;
	} conn_info_t;

	typedef struct {
		i_ring_queue *p_queue;
		int used_flag;
	} queue_info_t;

private:
	typedef std::vector<queue_info_t> queue_info_vec_t;
	typedef std::map<int, conn_info_t> conn_info_map_t;
	typedef std::map<int, queue_info_vec_t::iterator> queue_info_map_t;

private:
	i_config *m_p_config;
	i_net_io_server *m_p_net_io;
	proc_ctl_t *m_p_proc_ctl;

	queue_info_vec_t m_request_queue_vec;
	queue_info_vec_t m_response_queue_vec;
	queue_info_map_t m_task_map;
	conn_info_map_t m_conn_map;

	unsigned int m_work_proc_count;
	char m_request_buffer[MAX_BUF_LEN];
	char m_response_buffer[MAX_BUF_LEN];
};

#endif // H_NET_PROC_H_2010_10_12
