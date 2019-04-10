/**
 * =====================================================================================
 *       @file  feeds_memcached.h
 *      @brief  wrapper of feeds operations related memcached
 *
 *     Created  06/09/2011 02:55:21 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __FEEDS_MEMCACHED_H__
#define  __FEEDS_MEMCACHED_H__

#include  "util.h"
#include  "config.h"
#include  <libmemcached/memcached.h>

//only cache feed data in memcached servers
//item => (key, feed data)
//key  => (string representation of feedid: mimi + cmd_id + app_id + timestamp + magic)
class feeds_memcached
{
    public:
        /* ====================  OPERATIONS   ======================================= */
        /*-----------------------------------------------------------------------------
         *  wrapper memcached operations, common 0 => success, others => error code
         *-----------------------------------------------------------------------------*/
        /**
         * @brief  add one new feed to memcached server
         * @param   memc        memcached server handle
         * @param   feedid      feedid to operate
         * @param   escape_data escaped feeddata to operate
         * @return  0 => success, others => error code
         */
        static int add(memcached_st *memc, feedid_t *feedid, const char* escape_data, const size_t datalen);

        /**
         * @brief  set feed data to memcached server 
         * @param
         * @return  0 => success, others => error code
         */
        static int set(memcached_st *memc, feedid_t *feedid, const char* escape_data, const size_t datalen);

        static int replace(memcached_st *memc, feedid_t *feedid, const char* escape_data, const size_t datalen);

        /**
         * @brief  get feeds (feedid, data) for batch feedids
         * @param   memc        memcached server handle
         * @param   gets_buf    buf to store response feeds package related batch feedids(not include response header)
         * @param   buflen      max size len of gets_buf
         * @param   datalen     length of feedsdata
         * @param   feedids     batch operation feedids
         * @return  <0 => failed, >=0 => fetched feeds num
         */
        static int gets(memcached_st *memc, char* gets_buf, size_t buflen, uint32_t *datalen, feedid_list_t *feedids);


        /**
         * @brief gets feed package with feed_key_t
         * 
         */
        static int gets_key_pkg(memcached_st *memc, char* gets_buf, size_t buflen, uint32_t *datalen, feedid_list_t *flist);
        static int get_key(memcached_st *memc, feedid_t *fid, feedkey_t *fkey);
        static int set_key(memcached_st *memc, feedid_t *fid, feedkey_t *fkey);

        /**
         * @brief  get feed data from memcached server 
         * @param
         * @return  0 => success, others => error code
         */
        static int get(memcached_st *memc, char* get_buf, size_t buflen, uint32_t *datalen, feedid_t *fid);

        /**
         * @brief  get feed (feedid, data) for one feedid
         * @param   
         * @return  <= -1 => failed, >=0 => fetched feed num( === 1 )
         */

        static int del(memcached_st *memc, feedid_t *fid);

        /**
         * @brief  batch delete feeds by feedids
         * @param
         * @return  0 => success, others => error code
         */
        static int dels(memcached_st *memc, feedid_list_t *feedids);

    protected:
        static int generate_feedkey(feedid_t *fid, char * fkey);

    private:
        /* ====================  DATA MEMBERS ======================================= */
        //store string format of feedid
        //feedkey in memcached: ("%x_%x_%x_%x_%lx", mimi, cmd_id, app_id, timestamp, magic)
        static char feedkey[MEM_FEEDKEY_MAX];

        static memcached_return_t ret_memc;
};/* -----  end of class  Feeds_memcached  ----- */
#endif  /*__FEEDS_MEMCACHED_H__*/
