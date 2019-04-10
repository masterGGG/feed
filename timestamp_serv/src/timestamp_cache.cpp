/**
 * @file timestamp_cache.cpp
 * @brief timestamp缓存区结构
 * @author jerryshao jerryshao@taomee.com
 * @version
 * @date 2011-08-30
 */
#include <stdlib.h>
#include <string.h>

#include "benchapi.h"
#include "timestamp_cache.h"

tm_cache_head_t* tm_cache_create(u_int max_node_num)
{
    tm_cache_head_t* p_head = (tm_cache_head_t*) calloc(1, sizeof(tm_cache_head_t));
    if (!p_head) {
        return NULL;
    }

    INIT_LIST_HEAD(&p_head->lru_list_head);
    p_head->current_node_num = 0;
    p_head->max_node_num = max_node_num;

    p_head->p_mempool = mempool_create(sizeof(tm_cache_node_t), max_node_num * sizeof(tm_cache_node_t));
    if (!p_head->p_mempool) {
        return NULL;
    }

    return p_head;
}

tm_cache_node_t* tm_cache_search(tm_cache_head_t* p_head, uint32_t userid)
{
    if (!p_head) {
        return NULL;
    }

    struct rb_node* node = p_head->tm_tree_head.rb_node;
    while (node) {
        tm_cache_node_t* cache_node = rb_entry(node, tm_cache_node_t, tm_tree_node);
        if (userid < cache_node->userid) {
            node = node->rb_left;
        } else if (userid > cache_node->userid) {
            node = node->rb_right;
        } else {
            list_move(&cache_node->lru_list_node, &p_head->lru_list_head);
            return cache_node;
        }
    }

    return NULL;
}

tm_cache_node_t* tm_cache_insert(tm_cache_head_t* p_head, uint32_t userid, uint32_t timestamp)
{
    if (!p_head) {
        return NULL;
    }

    tm_cache_node_t* new_node = (tm_cache_node_t*) mempool_calloc(p_head->p_mempool);
    if (!new_node) {
        return NULL;
    }

    new_node->userid = userid;
    new_node->timestamp = timestamp;
    INIT_LIST_HEAD(&new_node->lru_list_node);

    if (p_head->current_node_num >= p_head->max_node_num) {
        ///但前缓存中的节点数量已到上限
        tm_cache_node_t *last_node = list_entry(p_head->lru_list_head.prev, tm_cache_node_t, lru_list_node);
        list_del(&last_node->lru_list_node);
        rb_erase(&last_node->tm_tree_node, &p_head->tm_tree_head);

#if _DEBUG
        INFO_LOG("delete last node, userid[%u], timestamp[%u]", last_node->userid, last_node->timestamp);
#endif
        mempool_free(p_head->p_mempool, last_node);
        p_head->current_node_num--;
    }

    struct rb_node** pp_node = &(p_head->tm_tree_head.rb_node);
    struct rb_node* parent = NULL;
    while(*pp_node) {
        tm_cache_node_t* cache_node = rb_entry(*pp_node, tm_cache_node_t, tm_tree_node);
        parent = *pp_node;
        if (userid < cache_node->userid) {
            pp_node = &((*pp_node)->rb_left);
        } else if (userid > cache_node->userid) {
            pp_node = &((*pp_node)->rb_right);
        } else {
            cache_node->timestamp = timestamp;
            list_move(&cache_node->lru_list_node, &p_head->lru_list_head);
            mempool_free(p_head->p_mempool, new_node);
            return cache_node;
        }
    }

    rb_link_node(&new_node->tm_tree_node, parent, pp_node);
    rb_insert_color(&new_node->tm_tree_node, &p_head->tm_tree_head);

    list__add(&new_node->lru_list_node, &p_head->lru_list_head);

    p_head->current_node_num++;

    return new_node;
}

int tm_cache_delete(tm_cache_head_t* p_head, uint32_t userid)
{
    if (p_head) {
        return -1;
    }

    tm_cache_node_t* cache_node = NULL;
    struct rb_node* node = p_head->tm_tree_head.rb_node;
    while (node) {
        cache_node = rb_entry(node, tm_cache_node_t, tm_tree_node);
        if (userid < cache_node->userid) {
            node = node->rb_left;
        } else if (userid > cache_node->userid) {
            node = node->rb_right;
        } else {
            goto out;
        }
    }
    return 0;
out:
    rb_erase(&cache_node->tm_tree_node, &p_head->tm_tree_head);
    list_del(&cache_node->lru_list_node);
    p_head->current_node_num--;

    mempool_free(p_head->p_mempool, cache_node);

    return 0;
}

int tm_cache_destroy(tm_cache_head_t* p_head)
{
    struct list_head* p = NULL;
    tm_cache_node_t* node = NULL;
    list_for_each(p, &p_head->lru_list_head) {
        struct list_head tmp;
        tmp = *p;
        node = list_entry(p, tm_cache_node_t, lru_list_node);
        list_del(p);
        mempool_free(p_head->p_mempool, node);
        node = NULL;
        p = &tmp;
    }

    mempool_destroy(p_head->p_mempool);
    free(p_head);
    p_head = NULL;

    return 0;
}
