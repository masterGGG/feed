/**
 * =====================================================================================
 *       @file  feeds_store.h
 *      @brief  header file of feeds storage wrapper
 *
 *     Created  06/13/2011 10:27:37 AM 
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __FEEDS_STORE_H__
#define  __FEEDS_STORE_H__

#include  <stdlib.h>
#include  <map>
#include  <vector>
#include  "util.h"
#include  "storage_proto.h"
#include  "i_mysql_iface.h"

class feeds_store
{
    public:
        static bool init();
        static void uninit();
        /* ====================  OPERATIONS   ======================================= */
        /*-----------------------------------------------------------------------------
         *  wrapper database operations, common 0 => success, others => error code
         *-----------------------------------------------------------------------------*/
        /**
         * @brief  handle insert package
         * @param  buf          used to store response package
         * @param  buflen       length of buf
         * @param  res_pkg_len  response package length
         * @param  units        how many insert_package to insert 
         * @param  pkg          pointer to first insert_package
         * @return 0 => success, others => failed code 
         */
        static int insert(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, insert_pkg_t *pkg);

        static int update(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, update_pkg_t *pkg);
        /** 
         * @brief update one feed (feedid, data) by old feedid
         * @param   
         * @return 0 => success, others => failed code 
         */
        static int update2(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, update2_pkg_t *pkg);

        static int set(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, set_pkg_t *pkg);

        static int del(char *send_buf, const size_t buflen, int *res_pkg_len, 
                const int units, delete_pkg_t *pkg);
        static int del2(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, delete2_pkg_t *pkg);

        ///
        /// @brief get packages according to feedids in request 
        /// @param  send_buf   buffer for response package
        ///
        /// @param  units      batch operation feeds num
        /// @return 0 => success, others => error code
        ///
        static int get_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, 
                const int units, get_pkg_t *pkg);
        static int get2_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, 
                const int units, get2_pkg_t *pkg);
        static int get_nindexs(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, indexn_pkg_t *pkg);
        static int get_spanindexs(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, indexspan_pkg_t *pkg);

        static int get_key_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, 
                const int units, get_key_pkg_t *pkg);

        /************************** passive feed operation interface *************************/
        /**
         * @brief  handle insert package
         * @param  buf          used to store response package
         * @param  buflen       length of buf
         * @param  res_pkg_len  response package length
         * @param  units        how many insert_package to insert 
         * @param  pkg          pointer to first insert_package
         * @return 0 => success, others => failed code 
         */
        static int insert_p(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, insert_p_pkg_t *pkg);

        static int update_p(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, update_p_pkg_t *pkg);

        static int del_p(char *send_buf, const size_t buflen, int *res_pkg_len, 
                const int units, del_p_pkg_t *pkg);

        static int get_p_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, 
                const int units, get_p_pkg_t *pkg);

        static int get_p_nindexs(char *send_buf, const size_t buflen, int *res_pkg_len,
                const int units, indexn_p_pkg_t *pkg);

        static int get_p_key_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, 
                const int units, get_p_key_pkg_t *pkg);

        static int get_p_feedid_by_cmdid_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, 
                const int units, get_p_feedid_by_cmdid_pkg_t *pkg);
        static int get_p_feedcnt(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, get_p_feed_cnt_t *pkg);
        /************************** <end of> passive feed operation interface *************************/

    protected:
        static bool feedid_equal(feedid_t *lhs, feedid_t *rhs);

        /** 
         * @brief   build one feed status (feedid, status) 
         * @param   
         * @return  length of statu package (fixed size>0) or error code
         */
        static int build_stat_pkg(char *buf, size_t buflen, feedid_t *fid, uint16_t status);

    private:
        /* ====================  DATA MEMBERS ======================================= */
        //buffer to store escape feed data
        static char *esc_data_buf;

        //used to batch operate feedids
        static feedid_list_t fidlist;

        //used to batch operate feedids
        static pfeedid_list_t pfidlist;

        //length of status package(fixed)
        const static int stat_pkg_len = ((int)sizeof(feed_status_pkg_t));

        //denote feeds store is inited or not?
        static bool inited;
};/* -----  end of class  feeds_store  ----- */

#endif  /*__FEEDS_STORE_H__*/
