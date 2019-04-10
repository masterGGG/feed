/* vim: set tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file work_proc.cpp
 * @author richard <richard@taomee.com>
 * @date 2010-04-28
 */

#ifndef WORK_PROC_2010_04_28
#define WORK_PROC_2010_04_28

#include <sys/types.h>            // for pid_t

#include "i_cache_queue.h"
#include "child_proc_type.h"
#include "defines.h"

class c_work_proc : public i_work_proc
{
public:
	c_work_proc();
	~c_work_proc();
	virtual int init(i_config *p_config, i_ring_queue *p_request_queue, i_ring_queue *p_response_queue, int cache_mem_len, const char *dbname, int namelen);
	virtual int uninit();
	virtual int release();

protected:
	int child_main();
	int deal_request_msg();
	int deal_task_request(int connection_id);
	int deal_so_request(int connection_id);
	int get_stat_log_info(char *logfile, int data_len, int *interval, uint32_t* msg_id);  
	int stat_log();
	static int stat_log_wrapper(void *p_obj);

private:
	typedef struct {
		volatile int inited;      // 初始化的状态(0：未初始化，1：初始化成功，－1：初始化失败)
		volatile int terminal;    // 子进程是否终止的状态(0：不终止，1：通知子进程终止，2：子进程终止完成)
		volatile time_t heartbeat;// 子进程的心跳
	} proc_ctl_t;

private:
	i_config *m_p_config;
	i_ring_queue *m_p_request_queue;
	i_ring_queue *m_p_response_queue;
	i_cache_queue *m_p_cache_queue;
    i_timer *m_p_timer;
	proc_ctl_t *m_p_proc_ctl;

	int m_cache_mem_len;
	int m_namelen;
	int m_log_interval;
	uint32_t m_msgid;

	char m_dbname[PATH_MAX];
	char m_logfile[PATH_MAX];
	char m_request_buffer[MAX_BUF_LEN];
	char m_response_buffer[MAX_BUF_LEN];
};

#endif /* WORK_PROC_2010_04_28 */

