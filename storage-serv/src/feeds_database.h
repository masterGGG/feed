/**
 * =====================================================================================
 *       @file  feeds_database.h
 *      @brief  header file <wrapper of feeds operations related database>
 *
 *     Created  06/07/2011 01:19:42 PM 
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __FEEDS_DATABASE_H__
#define  __FEEDS_DATABASE_H__

#include  <vector>
#include  "util.h"
#include  "i_mysql_iface.h"

/*wrapper database operation
 * insert, update, delete, get indexs
 */
class feeds_database
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
         * @param  db           database handle to insert one feed
         * @param  feedid       feedid for inserted  
         * @param  feeddata     escaped data for inserted
         * @param  datalen      length of escaped data(not include terminated null)
         * @return 0 => success, others => failed code (common set to -1)
         */
        static int insert(i_mysql_iface *db, feedid_t *feedid, const char * feeddata, const int datalen);

        /** 
         * @brief update one feed (feedid, data) by old feedid
         * @param   
         * @return 0 => success, others => failed code (common set to -1)
         */
        static int update2(i_mysql_iface *db, const feedid_t* oldfid, const feedid_t* newfid, 
                const char* feeddata, const int datalen);
        static int del(i_mysql_iface *db, feedid_t *fid);

        /** 
         * @brief  batch del feeds in feedid_list_t 
         * @param   feedids     should contain feedids to delete and set cur_used member
         * @param 
         * @return  return successfully delete feed num ( >= 0 )
         */
        static int dels(i_mysql_iface *db, feedid_list_t *feedids);

        /** 
         * @brief  delete feed in database, without timestamp used in filter cond.
         * @param   
         * @return delete success (0) or failure (-1)
         */
        static int del2(i_mysql_iface *db, uint32_t mimi, uint16_t cmd_id, uint32_t app_id, uint64_t magic);

        /** 
         * @brief  get one feed package from database
         * @param   
         * @return return feed num ( 0, 1 ), negtive num means error code
         */
        static int get_pkg(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *pkg_len, feedid_t *fid);

        /** 
         * @brief  get feed (id, data) in same database and table, build feed pkg from db only
         * @param   
         * @return  <= -1 => failed, >=0 => fetched feeds num
         */
        static int gets_pkg(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *pkgs_len, feedid_list_t * flist);

        static int gets2_pkg(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *pkgs_len, get2_pkg_t *pkg);
        
        static int gets_key_pkg(char *buf, size_t buflen, uint32_t *pkgs_len, feedid_list_t * flist);
        static int get_key(feedid_t *fid, feedkey_t *fkey);

        /** 
         * @brief  get feedid index up to specified item num
         * @param   
         * @param   fids_len    length of fetched feedids
         * @return  <= -1 => failed, >=0 => fetched feedids num
         */
        static int get_nindexs(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *fids_len,
                uint32_t mimi, uint16_t flag, uint16_t cmd_id, uint32_t app_id, 
                uint32_t starttime, uint16_t after, uint16_t before);

        /** 
         * @brief  get feedid index between specified time span
         * @param   
         * @return  <= -1 => failed, >=0 => fetched feedids num
         */
        static int get_spanindexs(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *fids_len,
                uint32_t mimi, uint16_t flag, uint16_t cmd_id, uint32_t app_id, 
                uint32_t starttime, uint32_t endtime);

        /** 
         * @brief  check feed existed or not
         * @param   
         * @return  true => existed in db, false => not
         */
        static bool check_existed(i_mysql_iface *db, feedid_t *fid);

        /** 
         * @brief  get feedid list by delete2_pkg_t
         * @param   
         * @return feedid num ( >=0 ), others error code
         */
        static int get_feedids(i_mysql_iface *db, uint32_t mimi, uint16_t cmd_id, uint32_t app_id, uint64_t magic, feedid_list_t *list);
    protected:
        /** 
         * @brief  build get prefix sql for get indexs from database
         * 
         * @param   withdata  row include data field or not
         * @return start of prefix sql in char[]
         */
        static char* get_nidx_presql(char *sqlbuf, size_t buflen, uint32_t mimi, uint16_t cmd_id, uint32_t app_id, 
                bool use_cmd_id, bool use_app_id, bool with_data);

        /** 
         * @brief  build one feed package from MYSQL ROW
         * @param  
         * @return length of one feed package length or error code (<0)
         */
        static int build_feed(MYSQL_ROW row, char *buf, size_t buflen, bool withdata);

        /** 
         * @brief  get feeds from db by sql, denote with data field or not
         *
         * @param   pkgs_len    length of feeds package fetched
         *
         * @return feeds num fetched from database
         */
        static int get_feedspkg(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *pkgs_len, char *sql, bool withdata);

};/* -----  end of class  ----- */

#endif  /*__FEEDS_DATABASE_H__*/
