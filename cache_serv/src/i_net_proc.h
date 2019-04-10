/* vim: set tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file i_net_proc.h
 * @author richard <richard@taomee.com>
 * @date 2010-04-28
 */

#ifndef I_NET_PROC_H_2010_04_28
#define I_NET_PROC_H_2010_04_28

#include "i_config.h"
#include "i_ring_queue.h"

/**
 * @brief net_proc接口类
 */
struct i_net_proc
{
public:
	/**
	 * @brief  初始化网络进程。
	 *
	 * @param p_config 指向配置实例的指针。
	 * @param work_proc_count 工作进程实例数组。
	 *
	 * @returns 0成功，-1失败。 
	 */
	virtual int init(i_config *p_config, 
					 int work_proc_count,
					 i_ring_queue **pp_request_queue,
					 i_ring_queue **pp_response_queue) = 0;

	/**
	 * @brief 反初始化网络进程，释放动态申请的内存。
	 *
	 * @returns 0成功，-1失败。  
	 */
	virtual int uninit() = 0;
	
	/**
	 * @brief 调用析构函数，释放网络进程实例。
	 *
	 * @returns 0成功，-1失败。  
	 */
	virtual int release() = 0;
};

/**
 * @brief 创建net_proc接口的实例
 */
int create_net_proc_instance(i_net_proc **pp_instance);

#endif /* I_NET_PROC_H_2010_04_28 */
