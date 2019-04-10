/* vim: set tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file i_work_proc.h
 * @author richard <richard@taomee.com>
 * @date 2010-04-28
 */

#ifndef I_WORK_PROC_H_2010_04_28
#define I_WORK_PROC_H_2010_04_28

#include "i_config.h"
#include "i_ring_queue.h"
#include "i_timer.h"

/**
 * @brief work_proc接口类
 */
struct i_work_proc
{
public:
	/**
	 * @brief  初始化工作进程。
	 *
	 * @param p_config 指向配置实例的指针。
	 * @param p_request_queue 指向存放请求消息环形缓冲区实例的指针。
	 * @param p_response_queue 指向存放回复消息环形缓冲区的指针。
	 *
	 * @returns 0成功，-1失败。
	 */
	virtual int init(i_config *p_config, 
				     i_ring_queue *p_request_queue, 
					 i_ring_queue *p_response_queue,
					 int cache_mem_len,
					 const char *dbname,
					 int namelen) = 0;

	/**
	 * @brief 反初始化工作进程，释放动态申请的内存
	 *
	 * @returns 0成功，-1失败  
	 */
	virtual int uninit() = 0;

	/**
	 * @brief 调用析构函数，释放工作进程实例。
	 *
	 * @returns 0成功，-1失败。  
	 */
	virtual int release() = 0;
};

/**
 * @brief 创建work_proc接口的实例
 */
int create_work_proc_instance(i_work_proc **pp_instance);

#endif /* I_WORK_PROC_H_2010_04_28 */
