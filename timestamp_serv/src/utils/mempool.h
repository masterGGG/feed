/*
 * =====================================================================================
 *
 *       Filename:  mempool.h
 *
 *    Description:  内存池
 *
 *        Version:  1.0
 *        Created:  2010年12月03日 11时24分59秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jerryshao (邵赛赛), jerryshao@taomee.com
 *        Company:  TaoMee.Inc, ShangHai
 *
 * =====================================================================================
 */

#ifndef _H_MEMPOOL_H_
#define _H_MEMPOOL_H_

#ifdef _cplusplus
extern "C"
{
#endif

#include <unistd.h>
#include "list.h"

typedef struct mempool
{
    size_t item_size;
    size_t alloc_size;
    size_t total_alloc_item;

    char *reuse_ptr;
    char *alloc_ptr;
    size_t alloc_pool_pos;

    int ref_count;

} mempool_t;

extern mempool_t* mempool_create(size_t item_size, size_t alloc_size);
extern void* mempool_alloc(mempool_t *pool);
extern void* mempool_calloc(mempool_t *pool);
extern int mempool_inc_refcount(mempool_t *pool);
extern int mempool_free(mempool_t* pool, void *data);
extern int mempool_destroy(mempool_t *pool);

#ifdef _cplusplus
}
#endif

#endif
