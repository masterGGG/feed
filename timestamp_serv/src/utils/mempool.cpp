/*
 * =====================================================================================
 *
 *       Filename:  mempool.c
 *
 *    Description:  内存池
 *
 *        Version:  1.0
 *        Created:  2010年12月03日 11时44分05秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jerryshao (邵赛赛), jerryshao@taomee.com
 *        Company:  TaoMee.Inc, ShangHai
 *
 * =====================================================================================
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "mempool.h"

#define WORD_SIZE (sizeof(void*))
#define DEFAULT_ALLOC_SIZE 100

mempool_t* mempool_create(size_t item_size, size_t alloc_size)
{
    mempool_t *p = (mempool_t*)calloc(1, sizeof(mempool_t));
    if(p == NULL) return NULL;

    if(item_size < WORD_SIZE) item_size = WORD_SIZE;
    if(item_size%WORD_SIZE)
        item_size = item_size + WORD_SIZE - item_size % WORD_SIZE;

    p->item_size = item_size;
    int pagesize = getpagesize();
    if(alloc_size <= item_size * DEFAULT_ALLOC_SIZE + WORD_SIZE)
        alloc_size = item_size * DEFAULT_ALLOC_SIZE + WORD_SIZE;
    if(alloc_size <= DEFAULT_ALLOC_SIZE * (size_t)pagesize)
        alloc_size = DEFAULT_ALLOC_SIZE * pagesize;

    if(alloc_size%pagesize)
        alloc_size = alloc_size + pagesize - alloc_size % pagesize;

    p->alloc_size = alloc_size;

    p->alloc_ptr = (char*)malloc(alloc_size);
    if(p->alloc_ptr == NULL)
    {
        free(p);
        return NULL;
    }

    p->total_alloc_item = (alloc_size - WORD_SIZE) / item_size;

    *((void**)p->alloc_ptr) = NULL;

    return p;
}

void* mempool_alloc(mempool_t *pool)
{
    if(!pool)
    {
        return NULL;
    }

    void *p = NULL;
    if(pool->reuse_ptr)
    {
        p = pool->reuse_ptr;
        pool->reuse_ptr = *(char **)p;
    }
    else
    {
        if(pool->alloc_pool_pos == pool->total_alloc_item)
        {/* malloc的内存已经分配满了,需要再分配 */
            p = malloc(pool->alloc_size);
            if(p == NULL) return NULL;

            *(void**)p = pool->alloc_ptr;
            pool->alloc_ptr = (char *)p;

            pool->alloc_pool_pos = 0;
        }

        p = pool->alloc_ptr + WORD_SIZE + pool->item_size * pool->alloc_pool_pos++;


    }

    mempool_inc_refcount(pool);
    return p;

}

void* mempool_calloc(mempool_t *pool)
{
    if(!pool)
    {
        return NULL;
    }

    void *p = mempool_alloc(pool);
    if(p)
    {
        memset(p, 0, sizeof(pool->item_size));
    }

    return p;
}

int mempool_inc_refcount(mempool_t *pool)
{
    if(!pool)
    {
        return -1;
    }

    pool->ref_count++;

    return 0;
}

int mempool_free(mempool_t *pool, void* data)
{
    if(!pool)
    {
        return -1;
    }

    *(void**)data = pool->reuse_ptr;
    pool->reuse_ptr = (char *)data;
    pool->ref_count--;

    return 0;
}

int mempool_destroy(mempool_t *pool)
{
    if(!pool)
        return -1;

    if(pool->ref_count <= 0)
    {
        void *p = NULL;
        while((p = pool->alloc_ptr))
        {
            pool->alloc_ptr = *((char**)pool->alloc_ptr);
            free(p);
        }

        free(pool);
        pool = NULL;
    }

    return 0;
}

