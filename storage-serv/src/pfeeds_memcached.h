/**
 * =====================================================================================
 *       @file  pfeeds_memcached.h
 *      @brief  header file 
 *
 *     Created  09/02/2011 01:31:16 PM 
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __PFEEDS_MEMCACHED_H__
#define  __PFEEDS_MEMCACHED_H__
#include  "util.h"
#include  <libmemcached/memcached.h>

class pfeeds_memcached
{
    public:

        /** 
         * @brief  add one passive feed to memcached server
         * @param   
         * @return 0 => success, other => error code
         */
        static int add(memcached_st *memc, pfeed_pkg_t *pkg);

    
        //pkg_data  contain active time and passive feed data
        static int set(memcached_st *memc, pfeedid_t *pfid, char *pkg_data, size_t datalen);

        static int replace(memcached_st *memc, pfeedid_t *fid, pfeed_pkg_t *pkg);

        static int del(memcached_st *memc, pfeedid_t *fid);

        static bool check_existed(memcached_st *memc, pfeedid_t *fid);
        /** 
         * @brief  get units passive feeds from db
         * @param   
         * @return >=0 => feeds num, <0 => error code 
         */
        static int get_pkgs(memcached_st *memc, char *resbuf, const int buflen, int *reslen, pfeedid_list_t *pfids);
        
        static int get_key_pkgs(memcached_st *memc, char *resbuf, const int buflen, int *reslen, pfeedid_list_t *pfids);
        static int get_key(memcached_st *memc, pfeedid_t *fid, feedkey_t *fkey);
        static int set_key(memcached_st *memc, pfeedid_t *fid, feedkey_t *fkey);
};
#endif  /*__PFEEDS_MEMCACHED_H__*/
