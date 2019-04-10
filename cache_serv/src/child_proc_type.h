/**
 * @file child_proc_type.h
 * @brief  子进程信息结构定义
 * @author Hansel
 * @version 
 * @date 2011-01-13
 */
#ifndef H_CHILD_PROC_TYPE_H_2010_10_11
#define H_CHILD_PROC_TYPE_H_2010_10_11

#include <vector>
#include <map>
#include <functional>

#include "i_work_proc.h"
#include "i_net_proc.h"

/**
 * @struct work_proc_item_t
 * @brief work子进程信息结构。
 */
typedef struct {
	i_work_proc *p_work_proc;				/**< 指向工作进程接口实例的指针 */
	i_ring_queue *p_request_queue;          /**< 该工作进程所对应的请求环形队列实例的指针 */
	i_ring_queue *p_response_queue;         /**< 该工作进程所对应的应答环形队列实例的指针 */
} work_proc_item_t;

/**
 * @struct net_proc_item_t
 * @brief 网络子进程信息结构。
 */
typedef struct {
	i_net_proc *p_net_proc;					/**< 指向网络进程接口实例的指针 */
} net_proc_item_t;

/**
 * @struct child_proc_item_t
 * @brief 子进程信息结构。
 */
typedef struct {
	pid_t child_pid;                        /**< 子进程的进程号 */
	union {
		net_proc_item_t net_proc;           /**< 网络进程信息结构 */
		work_proc_item_t work_proc;			/**< 工作进程信息结构 */
	};
} child_proc_item_t;

/**
 * @brief  存放一个网络进程，后续全为工作进程。
 */
typedef std::vector<child_proc_item_t> child_proc_vec_t;

/**
 * @brief  一元函数对象，用来找到对应pid的子进程对象。
 */
struct child_proc_item_eq_t : public std::unary_function<child_proc_item_t, bool> {
	explicit child_proc_item_eq_t(pid_t pid) : m_pid(pid) { }
	bool operator()(const child_proc_item_t & proc_item) const {
		return proc_item.child_pid == m_pid;
	}  
	private:
	pid_t m_pid;
};
#endif//H_CHILD_PROC_TYPE_H_2010_10_11
