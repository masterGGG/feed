/**
 * =====================================================================================
 *       @file  config.cpp
 *      @brief  wrapper for reading configurations
 *
 *     Created  05/30/2011 11:02:28 AM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#include  "config.h"

/*-----------------------------------------------------------------------------
 *  init static variables for config class
 *-----------------------------------------------------------------------------*/
bool gconfig::m_inited                  = false;
bool gconfig::m_ini_inited              = false;
i_ini_file* gconfig::m_ini_handle       = NULL;
char gconfig::m_cfgfile[PATH_MAX_LEN]   = { 0 };
char gconfig::m_msgfile[PATH_MAX_LEN]   = { 0 };
char gconfig::m_statpath[PATH_MAX_LEN]  = { 0 };
char gconfig::m_dbhost[STR_MAX_LEN]     = { 0 };
char gconfig::m_dbname[STR_MAX_LEN]     = { 0 };
int gconfig::m_dbport                   =  -1;
char gconfig::m_dbusername[STR_MAX_LEN] = { 0 };
char gconfig::m_dbpassword[STR_MAX_LEN] = { 0 };
i_mysql_iface* gconfig::m_config_db     = NULL;
db_conns  gconfig::m_feeds_databases    = { { {0, {0}, 0, {0}, {0}, {0}, false, NULL} },
    std::map<std::string, i_mysql_iface *>()};
memcached_srv_t  gconfig::m_feeds_memcached   = { false, {0}, NULL};
std::set<uint32_t> gconfig::start_users;

void db_conns::uninit() {
    for (int i=0; i<DB_CONN_NUM; i++) {
        if (dbs[i].inited == true) {
            dbs[i].handle->uninit();
        }
    }
}

i_mysql_iface *db_conns::get_bydbid(uint32_t dbid) {
    static char         conn_key_buf[1024];
    db_info_t *p = &dbs[dbid];
    if (p->inited == true) {
        return p->handle;
    }

    //conn identification string: host_port_username_password
    snprintf(conn_key_buf, sizeof(conn_key_buf), "%s_%u_%s_%s", p->host, p->port, p->username, p->password);
    std::string tmp_key(conn_key_buf);

    db_conns_handle_t::iterator it = conns_pool.find(tmp_key);
    if (it != conns_pool.end()) {
        p->handle = it->second;
        p->inited = true;
        return p->handle;
    }

    int ret = create_mysql_iface_instance(&(p->handle));
    if (ret == -1) {
        ERROR_LOG("create mysql instance failed, db_id:%u", dbid);
        return NULL;
    }
    ret = p->handle->init(p->host, p->port, NULL, p->username, p->password, "utf8");
    if (ret != 0) {
        ERROR_LOG("database:{%s:%u %s} init failed:%s", p->host, p->port, p->db_name, p->handle->get_last_errstr());
        return NULL;
    }

    conns_pool.insert(std::pair<std::string, i_mysql_iface*>(tmp_key, p->handle));
    p->inited = true;
    return p->handle;
}

i_mysql_iface *db_conns::get_handle(uint32_t mimi) {
    uint32_t db_id = get_dbid(mimi);
    return get_bydbid(db_id);
}

void memcached_server::uninit() {
    if (inited && handle) {
        memcached_free(handle);
    }
}

memcached_st * memcached_server::get_handle() {
    if (inited == false || handle == NULL) {
        handle = memcached_create(NULL);
        if (!handle) {
            ERROR_LOG("create memcached failed");
            return NULL;
        }
        memcached_server_st *servers = memcached_servers_parse(servers_list);
        memcached_return_t mem_ret = memcached_server_push(handle, servers);
        if (mem_ret != MEMCACHED_SUCCESS) {
            ERROR_LOG("memcached servers:%s, init failed: %s", servers_list, memcached_strerror(handle, mem_ret));
            return NULL;
        } else {//init success and can do behaviour set for memcached
            memcached_server_list_free(servers);
        }

        inited = true;
    }
    return handle;
}

bool gconfig::init(const char* cfg_file, bool reload) {
    if (m_inited == true && reload == false) { //support reload configuration
        return true;
    }

    //base cfg
    strncpy(m_cfgfile, cfg_file, sizeof(m_cfgfile));
    m_dbport = -1;
    memset(m_dbhost, 0, sizeof(m_dbhost));
    memset(m_dbusername, 0, sizeof(m_dbusername));
    memset(m_dbpassword, 0, sizeof(m_dbpassword));
    create_ini_file_instance(&m_ini_handle);
    int ret = m_ini_handle->init(cfg_file);
    if (ret != 0) {
        ERROR_LOG("fail to init configuration ini file handle: %s", m_ini_handle->get_last_errstr());
        return false;
    }
    m_ini_inited = true;

    //feeds database cfg
    ret = create_mysql_iface_instance(&m_config_db);
    if (ret == -1) {
        return false;
    }
    ret = m_config_db->init(get_dbhost(), get_dbport(), get_dbname(), get_dbusername(), get_dbpassword(), "utf8");
    if (ret != 0) {
        ERROR_LOG("fail to init connection to configuration db: %s", m_config_db->get_last_errstr());
        return false;
    }
    MYSQL_ROW row = NULL;
    int cnt = m_config_db->select_first_row(&row,
            "select db_id, db_name, db_host, db_port, db_user, db_passwd from t_db_info order by db_id");
    if (cnt > 0) {
        char *endptr = NULL;
        for (int i=0 ; i<cnt && row != NULL ; i++) {
            db_info_t *p = &(m_feeds_databases.dbs[i]);

            p->id = strtoul(row[0], &endptr, 10);
            strncpy(p->db_name, row[1], sizeof(p->db_name));
            strncpy(p->host, row[2], sizeof(p->host));
            p->port = strtoul(row[3], &endptr, 10);
            strncpy(p->username, row[4], sizeof(p->username));
            strncpy(p->password, row[5], sizeof(p->password));
            if (p->inited == true) {
                p->handle->uninit();
            }
            p->inited = false;
            p->handle = NULL;

            row = m_config_db->select_next_row(false);
        }
    } else {
        ERROR_LOG("fetch 0 row for feeds database info");
    }

    //feeds memcached cfg
    strncpy(m_feeds_memcached.servers_list, get_memserverlist(), sizeof(m_feeds_memcached.servers_list));

    //start users cfg
    int tmp_bufsize = 1024*1024;
    char *tmp_buf = (char*)malloc(tmp_bufsize);
    if (tmp_buf == 0) {
        ERROR_LOG("[gconfig::init] can not get buf , memory out of reach");
        return false;
    }
    if (m_ini_handle->read_string("Backward", "startusers",tmp_buf, tmp_bufsize,"") < 0) {
        printf("[gconfig::init] fail to read startusers list\n");
        return false;
    } else {
        DEBUG_LOG("start users list: %s", tmp_buf);
    }
    char *endptr = 0;
    char *intstr = tmp_buf;
    start_users.clear();
    for (char *p = tmp_buf; *p != 0; p++) {
        if (*p != ',') {
            continue;
        } else {
            *p = '\0';
            uint32_t mimi = strtoul(intstr, &endptr, 10);
            start_users.insert(mimi);
            intstr = p + 1;
        }
    }
    if (*intstr != 0) {
        uint32_t mimi = strtoul(intstr, &endptr, 10);
        start_users.insert(mimi);
    }
    free(tmp_buf);

    m_inited = true;
    return m_inited;
}

void gconfig::uninit() {
    if (m_inited == true) {
        //uninit ini config file hanle
        m_ini_handle->uninit();
        //uinit configuration database conn
        m_config_db->uninit();
        //uninit feeds database
        m_feeds_databases.uninit();
        //uninit feeds memcached
        m_feeds_memcached.uninit();
        //clear and release start users
        start_users.clear();

        m_inited = false;
    }
}

const char* gconfig::get_dbhost() {
    if (get_ini_str_cfg(m_dbhost, sizeof(m_dbhost), "Backward", "dbhost") == true) {
        return m_dbhost;
    } else {
        return NULL;
    }
}
const char* gconfig::get_dbname() {
    if (get_ini_str_cfg(m_dbname, sizeof(m_dbname), "Backward", "dbname") == true) {
        return m_dbname;
    } else {
        return NULL;
    }
}

const int gconfig::get_dbport() {
    if (m_ini_inited == false) {
        return -1;
    }
    if (m_dbport != -1) {
        return m_dbport;
    }

    m_dbport = m_ini_handle->read_int("Backward", "dbport", 3366);
    /* get db port for configuration fil e */
    return m_dbport;
}

const char* gconfig::get_dbusername() {
    if (get_ini_str_cfg(m_dbusername, sizeof(m_dbusername), "Backward", "dbusername") == true) {
        return m_dbusername;
    } else {
        return NULL;
    }
}

const char* gconfig::get_dbpassword() {
    if (get_ini_str_cfg(m_dbpassword, sizeof(m_dbpassword), "Backward", "dbpassword") == true) {
        return m_dbpassword;
    } else {
        return NULL;
    }
}

const char* gconfig::get_memserverlist() {
    if (get_ini_str_cfg(m_feeds_memcached.servers_list, sizeof(m_feeds_memcached.servers_list),
                "Backward", "memserverlist") == true) {
        return m_feeds_memcached.servers_list;
    } else {
        return NULL;
    }
}

i_mysql_iface * gconfig::get_db_conn(uint32_t mimi){
    return m_feeds_databases.get_handle(mimi);
}

i_mysql_iface * gconfig::get_bydbid(uint32_t dbid){
    return m_feeds_databases.get_bydbid(dbid);
}

const char* gconfig::get_table_name(uint32_t mimi) {
    static char tmp_buf[512];
    snprintf(tmp_buf, sizeof(tmp_buf), "%s.t_newsfeed_%u",
            m_feeds_databases.dbs[get_dbid(mimi)].db_name, get_tid(mimi));
    return tmp_buf;
}

memcached_st * gconfig::get_mem_handle() {
    return m_feeds_memcached.get_handle();
}

bool gconfig::is_startuser(uint32_t mimi)
{
    std::set<uint32_t>::iterator it = start_users.find(mimi);
    if (it != start_users.end()) {
        return true;
    } else {
        return false;
    }
}
char* gconfig::get_msg_file_path()
{
    static bool inited = false;
    if (!inited) {
        if (m_ini_handle->read_string("Backward", "msg_dir", m_msgfile, sizeof(m_msgfile), "")  <  0) {
            ERROR_LOG("fail to get [Backward::msg_dir] ini key");
            return NULL;
        }
        strcat(m_msgfile, "/storage.log");
    }
    return m_msgfile;
}
char* gconfig::get_stat_path()
{
    static bool inited = false;
    if (!inited) {
        if (m_ini_handle->read_string("Backward", "stat_dir", m_statpath, sizeof(m_statpath), "")  <  0) {
            ERROR_LOG("fail to get [Backward::stat_dir] ini key");
            return NULL;
        }
    }
    return m_statpath;
}

//get configuration from ini file, first check inited?
inline bool gconfig::get_ini_str_cfg(char *buf, size_t bufsz, const char *section, const char * key)
{
    if (m_ini_inited == false) {
        return false;
    }
    size_t i = 0;
    for (; i < bufsz; i++) {  //check configuration string inited
        if (buf[i] != 0) return true;
    }

    if (i == bufsz) {
        m_ini_handle->read_string(section, key, buf, bufsz, "");
    }
    return true;
}/* -----  end of function get_ini_cfg  ----- */

void gconfig::dump_cfg_info(bool withinfo) {
    if (withinfo == true) {
        for (int i = 0; i<DB_CONN_NUM ; i++) {
            db_info_t *p = &(m_feeds_databases.dbs[i]);
            DEBUG_LOG("db id:%u, host:%s,%u db_name:%s cert: %s, %s",
                    p->id, p->host, p->port, p->db_name, p->username, p->password);
        }
    }

    DEBUG_LOG("there are %lu real db connections:", m_feeds_databases.conns_pool.size());
    db_conns_handle_t::iterator  it = m_feeds_databases.conns_pool.begin();
    for (; it != m_feeds_databases.conns_pool.end(); ++it) {
        DEBUG_LOG("db conn_key: %s", it->first.c_str());
    }

    DEBUG_LOG("memc server list: %s", m_feeds_memcached.servers_list);

    DEBUG_LOG("storage stat message file path: %s", get_msg_file_path());
    DEBUG_LOG("storage analysis log dir: %s", get_stat_path());
}
