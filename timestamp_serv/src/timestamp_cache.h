/**
 * @file timestamp_cache.h
 * @brief timestamp缓存区结构
 * @author jerryshao jerryshao@taomee.com
 * @version
 * @date 2011-08-30
 */
#ifndef _H_TIMESTAMP_CACHE_H_
#define _H_TIMESTAMP_CACHE_H_

#include <unistd.h>
#include <stdint.h>

#include "list.h"
#include "rbtree.h"
#include "mempool.h"

/**
 * @brief 时间戳快速缓存结构体
 */

typedef struct {
    struct list_head lru_list_head;     /**<@最近最久未用链表*/
    struct rb_root tm_tree_head;        /**<@查询红黑树根节点*/

    u_int current_node_num;             /**<@当前缓存中的节点数量*/
    u_int max_node_num;                 /**<@缓存中的节点上限*/

    mempool_t* p_mempool;               /**<@内存池指针*/
} tm_cache_head_t;

typedef struct {
    struct list_head lru_list_node;     /**<@最近最久未用链表*/
    struct rb_node tm_tree_node;        /**<@查询红黑树节点*/

    uint32_t userid;                    /**<@红黑树key值*/
    uint32_t timestamp;                 /**<@红黑树value值*/
} tm_cache_node_t;

extern tm_cache_head_t* tm_cache_create(u_int max_node_num);
extern tm_cache_node_t* tm_cache_search(tm_cache_head_t* p_head, uint32_t userid);
extern tm_cache_node_t* tm_cache_insert(tm_cache_head_t* p_head, uint32_t userid, uint32_t timestamp);
extern int tm_cache_delete(tm_cache_head_t* p_head, uint32_t userid);
extern int tm_cache_destroy(tm_cache_head_t* p_head);

#endif

