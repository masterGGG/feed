/**
 * =====================================================================================
 *       @file  config.h
 *      @brief
 *
 *     Created  05/30/2011 06:55:03 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */

#ifndef  __CONFIG_CPP__
#define  __CONFIG_CPP__

#include  <stdlib.h>
#include  <cstring>
#include  <map>
#include  <set>

#include  "log.h"
#include  "i_ini_file.h"
#include  "i_mysql_iface.h"
#include  <libmemcached/memcached.h>

#define     get_dbid(mimi)      ((mimi)%10000/100)
#define     get_tid(mimi)      ((mimi)%100)

//feed, distribute by mimi
//passive feed, distribute by target_id

#define     STR_MAX_LEN     100
typedef struct db_info
{
    unsigned int        id;
    char                host[20];
    unsigned int        port;
    char                db_name[STR_MAX_LEN];
    char                username[STR_MAX_LEN];
    char                password[STR_MAX_LEN];
    bool                inited;
    i_mysql_iface       *handle;
} db_info_t;

//database config
#define     DB_CONN_NUM         100
typedef std::map<std::string, i_mysql_iface *> db_conns_handle_t; 
class db_conns 
{
    public:
    db_info_t           dbs[DB_CONN_NUM];
    db_conns_handle_t   conns_pool;

    void uninit();
    i_mysql_iface * get_bydbid(uint32_t dbid);
    i_mysql_iface * get_handle(uint32_t mimi);
};

//memcached config
#define     MEMSET_MAX_LEN  1024
typedef struct memcached_server 
{
    bool inited;
    char servers_list[MEMSET_MAX_LEN];
    memcached_st  *handle;

    void uninit();
    memcached_st * get_handle();
}memcached_srv_t;

class gconfig
{
    public:
        static bool init(const char* cfg_file, bool reload = false);
        static void uninit();

        static const char* get_dbhost();
        static const char* get_dbname();
        static const int get_dbport();
        static const char* get_dbusername();
        static const char* get_dbpassword();
        static const char* get_memserverlist();
        static i_mysql_iface * get_db_conn(uint32_t mimi);
        static i_mysql_iface * get_bydbid(uint32_t dbid);
        static const char* get_table_name(uint32_t mimi);
        static memcached_st * get_mem_handle();
        static bool is_startuser(uint32_t mimi);
        static char* get_msg_file_path();
        static char* get_stat_path();
        static void dump_cfg_info(bool withdbinfo = false);
    private:
        /* ====================  INTERNAL FUNCTIONS  ======================================= */
        static bool get_ini_str_cfg(char *buf, size_t bufsz, const char *section, const char * key);

        /* ====================  DATA MEMBERS ======================================= */
        //internal status
#define     PATH_MAX_LEN    255
        static bool             m_inited;
        static bool             m_ini_inited;
        static char             m_cfgfile[PATH_MAX_LEN];
        static i_ini_file*      m_ini_handle;
        static char             m_msgfile[PATH_MAX_LEN];
        static char             m_statpath[PATH_MAX_LEN];

        //configuration database 
        static char     m_dbhost[STR_MAX_LEN];
        static char     m_dbname[STR_MAX_LEN];
        static int      m_dbport;
        static char     m_dbusername[STR_MAX_LEN];
        static char     m_dbpassword[STR_MAX_LEN];
        static i_mysql_iface *m_config_db;  //conn for configuration database
        //feeds database 
        static db_conns  m_feeds_databases;
        //feeds memcached 
        static memcached_srv_t  m_feeds_memcached;

        static std::set<uint32_t> start_users;
};/* -----  end of class  gconfig  ----- */
#endif  /*__CONFIG_CPP__*/
