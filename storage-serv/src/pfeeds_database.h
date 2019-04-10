/**
 * =====================================================================================
 *       @file  pfeeds_database.h
 *      @brief  header file
 *
 *     Created  09/02/2011 10:54:04 AM 
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __PFEEDS_DATABASE_H__
#define  __PFEEDS_DATABASE_H__

#include  "util.h"
#include  "i_mysql_iface.h"

class pfeeds_database
{
public:
    /** 
     * @brief  insert passive feed to db
     * @param  
     * @return 0 => success, other => error code 
     */
    static int insert(i_mysql_iface *db, pfeed_pkg_t *pkg);

    static int update(i_mysql_iface *db, pfeedid_t *fid, pfeed_pkg_t *pkg);

    static int del(i_mysql_iface *db, pfeedid_t *fid);

    static bool check_existed(i_mysql_iface *db, pfeedid_t *fid);
    /** 
     * @brief  get units passive feeds from db
     * @param   
     * @return >=0 => feeds num, <0 => error code 
     */
    static int get_pkgs(char *resbuf, const int buflen, int *reslen, pfeedid_list_t *list);

    static int get_key_pkgs(char *resbuf, const int buflen, int *reslen, pfeedid_list_t *list);
    static int get_key(pfeedid_t *fid, feedkey_t *fkey);
    /** 
     * @brief  get units users' passive feeds indexs
     * @param   
     * @return >=0 => feeds index num , <0 => error code
     */
    static int get_indexs(char *resbuf, const int buflen, int *reslen, const int units, indexn_p_pkg_t *pkg);
};

#endif  /*__PFEEDS_DATABASE_H__*/
