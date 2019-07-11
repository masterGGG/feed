/**
 * =====================================================================================
 *       @file  pfeeds_database.cpp
 *      @brief  impl of passive feed databases operations
 *
 *     Created  09/02/2011 10:53:40 AM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#include <string.h>

#include "config.h"
#include "storage_proto.h"
#include "pfeeds_database.h"

//
//passive feed distribute in mysql db by target_id
//
//+-------------+---------------------+------+-----+---------+-------+
//| Field       | Type                | Null | Key | Default | Extra |
//+-------------+---------------------+------+-----+---------+-------+
//| user_id     | int(11) unsigned    | NO   |     |         |       |
//| cmd_id      | int(6) unsigned     | NO   |     |         |       |
//| app_id      | int(11) unsigned    | NO   |     |         |       |
//| timestamp   | int(11) unsigned    | NO   |     |         |       |
//| magic       | bigint(20) unsigned | NO   |     |         |       |
//| sender_id   | int(11) unsigned    | NO   |     |         |       |
//| target_id   | int(11) unsigned    | NO   | MUL |         |       |
//| p_magic     | int(11) unsigned    | NO   |     |         |       |
//| active_time | int(11) unsigned    | NO   |     |         |       |
//| data        | text                | NO   |     |         |       |
//+-------------+---------------------+------+-----+---------+-------+

static char sql_buf[SQL_MAX_SZ] = {0};
static char esc_data_buf[SQL_MAX_SZ] = {0};
static const int max_buf_size = SQL_MAX_SZ - 1;

int pfeeds_database::insert(i_mysql_iface *db, pfeed_pkg_t *pkg)
{
    if (db == NULL || pkg == NULL) {
        ERROR_LOG("get null value for pfeed insert");
        return -1;
    }
    int esc_datalen = mysql_real_escape_string(db->get_conn(), esc_data_buf, (char*)pkg->data, pkg->len - P_FEED_HEAD_SZ);
    if (esc_datalen >= max_buf_size) {
        ERROR_LOG("passive package data too large, datalen %d", pkg->len - P_FEED_HEAD_SZ);
        return -1;
    }

    feedid_t *src_id = &pkg->fid.src_fid;
    uint32_t dis_id = pkg->fid.target_id;
    snprintf(sql_buf, max_buf_size, "insert into db_newsfeed_%d.t_pass_newsfeed_%d "
            "(user_id,cmd_id,app_id,timestamp,magic,sender_id,target_id,p_magic,active_time,data) values "
            "(%u,%u,%u,%u,%lu,%u,%u,%u,%u,'%s');",
            get_dbid(dis_id), get_tid(dis_id),
            src_id->mimi, src_id->cmd_id, src_id->app_id, src_id->timestamp, src_id->magic,
            pkg->fid.sender_id, pkg->fid.target_id, pkg->fid.p_magic,
            pkg->active_time, esc_data_buf);

DEBUG_SQL(sql_buf);

    int ret = db->execsql("%s", sql_buf);
    if (ret <= 0) {
        ERROR_LOG("fail to execsql(insert) %s", sql_buf);
        return -1;
    }
    else {
        return 0;
    }
}

int pfeeds_database::update(i_mysql_iface *db, pfeedid_t *fid, pfeed_pkg_t *pkg)
{
    if (db== NULL || pkg == NULL || fid == NULL) {
        ERROR_LOG("get null value for pfeed update");
        return -1;
    }
    if ( ! pfeedid_equal(fid, &pkg->fid)) {
        ERROR_LOG("not matched passive feed id for update, fid1 %s , fid2 %s", get_pfeedid_str(fid), get_pfeedid_str(&pkg->fid));
        return -1;
    }

    int esc_datalen = mysql_real_escape_string(db->get_conn(), esc_data_buf, (char*)pkg->data, pkg->len - P_FEED_HEAD_SZ);
    if (esc_datalen >= max_buf_size) {
        ERROR_LOG("passive package data too large, datalen %d", pkg->len - P_FEED_HEAD_SZ);
        return -1;
    }

    feedid_t *src_id = &pkg->fid.src_fid;
    uint32_t dis_id = pkg->fid.target_id;
    snprintf(sql_buf, max_buf_size, "update db_newsfeed_%d.t_pass_newsfeed_%d "
            "set user_id=%u,cmd_id=%u,app_id=%u,timestamp=%u,magic=%lu,sender_id=%u,target_id=%u,p_magic=%u,active_time=%u,data='%s' "
            "where user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu and sender_id=%u and target_id=%u and p_magic=%u",
            get_dbid(dis_id), get_tid(dis_id),
            src_id->mimi, src_id->cmd_id, src_id->app_id, src_id->timestamp, src_id->magic,
            pkg->fid.sender_id, pkg->fid.target_id, pkg->fid.p_magic, pkg->active_time, esc_data_buf, 
            fid->src_fid.mimi, fid->src_fid.cmd_id, fid->src_fid.app_id, fid->src_fid.timestamp, fid->src_fid.magic,
            fid->sender_id, fid->target_id, fid->p_magic);

DEBUG_SQL(sql_buf);
    int ret = db->execsql("%s", sql_buf);
    if (ret < 0) {
        ERROR_LOG("fail to execsql(update) %s", sql_buf);
        return -1;
    }
    else if (ret == 0) {
        WARN_LOG("effect 0 row for update pfeed, %s", get_pfeedid_str(fid) );
    }
    return 0;
}

int pfeeds_database::del(i_mysql_iface *db, pfeedid_t *fid)
{
    if (db== NULL || fid == NULL) {
        ERROR_LOG("null value for pfeed del");
        return -1;
    }
    uint32_t dis_id = fid->target_id;
    snprintf(sql_buf, max_buf_size, "delete from db_newsfeed_%d.t_pass_newsfeed_%d "
            "where user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu and sender_id=%u and target_id=%u and p_magic=%u",
            get_dbid(dis_id), get_tid(dis_id),
            fid->src_fid.mimi, fid->src_fid.cmd_id, fid->src_fid.app_id, fid->src_fid.timestamp, fid->src_fid.magic,
            fid->sender_id, fid->target_id, fid->p_magic );

DEBUG_SQL(sql_buf);

    int ret = db->execsql("%s", sql_buf);
    if (ret < 0) {
        ERROR_LOG("fail to execsql(delete) %s", sql_buf);
        return -1;
    }
    else if (ret == 0) {
        WARN_LOG("effect 0 row for delete pfeed, %s", get_pfeedid_str(fid) );
        return 0;
    }
    else {
        return 0;
    }
}

bool pfeeds_database::check_existed(i_mysql_iface *db, pfeedid_t *fid)
{
    if (db== NULL || fid == NULL) {
        ERROR_LOG("null value for check pfeed existed");
        return false;
    }

    uint32_t dis_id = fid->target_id;
    snprintf(sql_buf, max_buf_size, "select target_id from db_newsfeed_%d.t_pass_newsfeed_%d "
            "where user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu and sender_id=%u and target_id=%u and p_magic=%u",
            get_dbid(dis_id), get_tid(dis_id),
            fid->src_fid.mimi, fid->src_fid.cmd_id, fid->src_fid.app_id, fid->src_fid.timestamp, fid->src_fid.magic,
            fid->sender_id, fid->target_id, fid->p_magic );

DEBUG_SQL(sql_buf);
    MYSQL_ROW row;
    int ret = db->select_first_row(&row, "%s", sql_buf);
    if (ret < 0) {
        ERROR_LOG("fail to execsql(check_existed) %s", sql_buf);
        return false;
    }
    return ret >= 1 ? true : false; 
}

int pfeeds_database::get_pkgs(char *resbuf, const int buflen, int *reslen, pfeedid_list_t *pfids)
{
    if (resbuf == NULL || pfids == NULL) {
        ERROR_LOG("null value for get_pkgs");
        return -1;
    }

    int total_cnt = 0;
    char *buf_idx = resbuf;
    for (int i = 0 ; i < pfids->cur_used ; i++) {
        if (pfids->flag[i] != F_NONE) {
            continue;
        }

        //do fetch from mysql
        pfeedid_t *fid = &(*pfids)[i];
        uint32_t dis_id = (*pfids)[i].target_id;
        snprintf(sql_buf, max_buf_size, "select user_id,cmd_id,app_id,timestamp,magic,sender_id,target_id,p_magic,active_time,data "
                "from db_newsfeed_%d.t_pass_newsfeed_%d "
                "where user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu and sender_id=%u and target_id=%u and p_magic=%u",
                get_dbid(dis_id), get_tid(dis_id), 
                fid->src_fid.mimi, fid->src_fid.cmd_id, fid->src_fid.app_id, fid->src_fid.timestamp, fid->src_fid.magic,
                fid->sender_id, fid->target_id, fid->p_magic );

    DEBUG_SQL(sql_buf);
        i_mysql_iface *db = gconfig::get_db_conn(dis_id);
        if (db == NULL) {
            ERROR_LOG("[get pfeed package] fail to get database connection for mimi: %u", dis_id);
            return -1;
        }
        MYSQL_ROW row;
        int effect_rows = db->select_first_row(&row, "%s", sql_buf);
        if (effect_rows < 0) {
            ERROR_LOG("[get pfeed package] database failure, sql:%s, err str: %s", sql_buf, db->get_last_errstr());
            return -1;
        }
        else if (effect_rows == 0) {
            WARN_LOG("get 0 pfeed, sql: %s, last mysql err str: %s", sql_buf, db->get_last_errstr());
        }
        else {
            char *endptr = NULL;
            for (int rowid = 0; rowid < effect_rows; rowid++) {
                pfeed_pkg_t *p_pfeed = (pfeed_pkg_t *)buf_idx;
                p_pfeed->fid.src_fid.mimi = strtoul(row[0], &endptr, 10);
                p_pfeed->fid.src_fid.cmd_id = strtoul(row[1], &endptr, 10);
                p_pfeed->fid.src_fid.app_id = strtoul(row[2], &endptr, 10);
                p_pfeed->fid.src_fid.timestamp = strtoul(row[3], &endptr, 10);
                p_pfeed->fid.src_fid.magic = (uint64_t)strtoull(row[4], &endptr, 10);
                p_pfeed->fid.sender_id = strtoul(row[5], &endptr, 10);
                p_pfeed->fid.target_id = strtoul(row[6], &endptr, 10);
                p_pfeed->fid.p_magic = strtoul(row[7], &endptr, 10);
                p_pfeed->active_time = strtoul(row[8], &endptr, 10);
                int datalen = strlen(row[9]);
                memcpy(p_pfeed->data, row[9], datalen);
                p_pfeed->len = P_FEED_HEAD_SZ + datalen;

                total_cnt ++;

                buf_idx += p_pfeed->len;
                if (buf_idx - resbuf > buflen) {
                    ERROR_LOG("[get pfeed package] response package too long %ld bytes", buf_idx - resbuf);
                    return -1;
                }

                row = db->select_next_row(false);
            } //end of once fetch
        }
    } //end of batch fetch 

    *reslen = buf_idx - resbuf;
    return total_cnt;
}

int pfeeds_database::get_indexs(char *resbuf, const int buflen, int *reslen, const int units, indexn_p_pkg_t *pkg)
{
    if (resbuf == NULL || pkg == NULL || units < 0 || units > MAX_ALLOW_UNITS) {
        ERROR_LOG("null value, or units not allowed %d for get_indexs", units);
        return -1;
    }

    int total_cnt = 0;
    char *buf_idx = resbuf;

    for (int i=0 ; i<units ; i++) { //fetch for target user of each package
        indexn_p_pkg_t *cur_pkg = pkg + i;
        /*
         * 2019-07-10
         * 支持按协议号过滤功能
         *///>>>>>>>>>>>>>>>>>>begin
        bool use_cmd_id = pkg->flag & 0x1;
        bool use_app_id = pkg->flag & 0x2;
        bool with_data  = pkg->flag & 0x4;
        const char * data_fld = with_data ? ",data " : " ";
        const char * end_fld = " and active_time < %u order by target_id, active_time desc limit %u;";
        //<<<<<<<<<<<<<<<<end
        uint32_t dis_id = pkg->mimi;
        uint32_t starttime = cur_pkg->starttime == 0 ? time(NULL) : cur_pkg->starttime;
        int sql_cnt = snprintf(sql_buf, max_buf_size, "select user_id,cmd_id,app_id,timestamp,magic,sender_id,target_id,p_magic,active_time %s"
                "from db_newsfeed_%d.t_pass_newsfeed_%d "
                "where target_id=%u",
                data_fld, get_dbid(dis_id), get_tid(dis_id), cur_pkg->mimi);
//, cur_pkg->cmd_id, cur_pkg->app_id, starttime, cur_pkg->prev_num );
//                "where target_id=%u and cmd_id=%u and app_id=%u and active_time < %u order by target_id, active_time desc limit %u;",
        char *new_buf = nullptr;
        if (use_cmd_id == true) {
            new_buf = sql_buf + sql_cnt;
            sql_cnt +=snprintf(new_buf, max_buf_size - sql_cnt, " and cmd_id = %u", cur_pkg->cmd_id);
        }
        if (use_app_id == true) {
            new_buf = sql_buf + sql_cnt;
            sql_cnt +=snprintf(new_buf, max_buf_size - sql_cnt, " and app_id = %u", cur_pkg->app_id);
        }
        new_buf = sql_buf + sql_cnt;
        snprintf(new_buf, max_buf_size - sql_cnt, end_fld, starttime, cur_pkg->prev_num);

    DEBUG_SQL(sql_buf);
        i_mysql_iface *db = gconfig::get_db_conn(dis_id);
        if (db == NULL) {
            ERROR_LOG("[get passive idxs] fail to get database connection for mimi: %u", dis_id);
            return -1;
        }
        MYSQL_ROW row;
        int effect_rows = db->select_first_row(&row, "%s", sql_buf);
        if (effect_rows < 0) {
            ERROR_LOG("[get passive idxs] database failure, sql:%s, err str: %s", sql_buf, db->get_last_errstr());
            return -1;
        }
        else if (effect_rows == 0) {
            WARN_LOG("get 0 pfeedid, sql: %s", sql_buf);
        }
        else {
            char *endptr = NULL;
            for (int rowid = 0; rowid < effect_rows; rowid++) {
                pfeed_pkg_t *p_pfeed = (pfeed_pkg_t *)buf_idx;
                p_pfeed->fid.src_fid.mimi = strtoul(row[0], &endptr, 10);
                p_pfeed->fid.src_fid.cmd_id = strtoul(row[1], &endptr, 10);
                p_pfeed->fid.src_fid.app_id = strtoul(row[2], &endptr, 10);
                p_pfeed->fid.src_fid.timestamp = strtoul(row[3], &endptr, 10);
                p_pfeed->fid.src_fid.magic = (uint64_t)strtoull(row[4], &endptr, 10);
                p_pfeed->fid.sender_id = strtoul(row[5], &endptr, 10);
                p_pfeed->fid.target_id = strtoul(row[6], &endptr, 10);
                p_pfeed->fid.p_magic = strtoul(row[7], &endptr, 10);
                p_pfeed->active_time = strtoul(row[8], &endptr, 10);
                if (with_data == false) {
                    p_pfeed->len = P_FEED_HEAD_SZ;
                } else {
                    size_t vlen = strlen(row[9]); 
                    p_pfeed->len = P_FEED_HEAD_SZ + vlen;
                    memcpy(p_pfeed->data, row[9], vlen);
                }

                buf_idx += p_pfeed->len;
                if (buf_idx - resbuf > buflen) {
                    ERROR_LOG("[get passive idxs] response package too long %ld bytes", buf_idx - resbuf);
                    return -1;
                }

                total_cnt ++;
                row = db->select_next_row(false);
            } //end of once fetch
        }
    } // end of once get indexs process

    *reslen = buf_idx - resbuf;
    return total_cnt;
}

int pfeeds_database::get_key_pkgs(char *resbuf, const int buflen, int *reslen, pfeedid_list_t *pfids)
{
    if (resbuf == NULL || pfids == NULL) {
        ERROR_LOG("null value for get_pkgs");
        return -1;
    }

    int total_cnt = 0;
    char *buf_idx = resbuf;
    for (int i = 0 ; i < pfids->cur_used ; i++) {
        if (pfids->flag[i] != F_NONE) {
            continue;
        }

        //do fetch from mysql
        pfeedid_t *fid = &(*pfids)[i];
        uint32_t dis_id = (*pfids)[i].target_id;
        int db_id = get_dbid(dis_id);
        int t_id = get_tid(dis_id);
        snprintf(sql_buf, max_buf_size, "select user_id, active_time, data from db_newsfeed_%d.t_pass_newsfeed_%d "
                "where user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu and sender_id=%u and target_id=%u and p_magic=%u",
                db_id, t_id, fid->src_fid.mimi, fid->src_fid.cmd_id, fid->src_fid.app_id, fid->src_fid.timestamp, fid->src_fid.magic,
                fid->sender_id, fid->target_id, fid->p_magic );

    DEBUG_SQL(sql_buf);
        i_mysql_iface *db = gconfig::get_db_conn(dis_id);
        if (db == NULL) {
            ERROR_LOG("[get pfeed package] fail to get database connection for mimi: %u", dis_id);
            return -1;
        }
        MYSQL_ROW row;
        int effect_rows = db->select_first_row(&row, "%s", sql_buf);
        if (effect_rows < 0) {
            ERROR_LOG("[get pfeed package] database failure, sql:%s, err str: %s", sql_buf, db->get_last_errstr());
            *reslen = 0;
            return -1;
        }
        else if (effect_rows == 0) {
            WARN_LOG("get 0 passive key feed , sql: %s", sql_buf);
        }
        else {
            char *endptr = NULL;
            int datalen = strlen(row[2]);
            int pkg_len = sizeof(pfeed_key_pkg_t) + datalen;
            if (buf_idx - resbuf + pkg_len > buflen) {
                ERROR_LOG("[get pfeed key package] response package too long %ld bytes", buf_idx - resbuf);
                break;
            }
            pfeed_key_pkg_t *p_pfeed = (pfeed_key_pkg_t *)buf_idx;
            p_pfeed->key.dbid = db_id;
            p_pfeed->key.ftid = 0xFF;
            p_pfeed->key.pftid = t_id;
            p_pfeed->key.flag = 0;
            p_pfeed->key.id = strtoul(row[0], &endptr, 10);

            memcpy(&p_pfeed->id, fid, sizeof(pfeedid_t));
            p_pfeed->active_time = strtoul(row[1], &endptr, 10);
            memcpy(p_pfeed->data, row[2], datalen);

            p_pfeed->len = pkg_len;
            buf_idx += p_pfeed->len;

            total_cnt ++;

            row = db->select_next_row(false);
        }
    } //end of batch fetch 

    *reslen = buf_idx - resbuf;
    return total_cnt;
}

int pfeeds_database::get_key(pfeedid_t *fid, feedkey_t *fkey)
{
    if (fid == 0 || fkey == 0) {
        ERROR_LOG("NULL value for feed database get key");
        return -1;
    }
        
    uint32_t dis_id = fid->target_id;
    int db_id = get_dbid(dis_id);
    int t_id = get_tid(dis_id);
    feedid_t *sfid = &fid->src_fid;
    snprintf(sql_buf, SQL_MAX_SZ -1, "select id from db_newsfeed_%d.t_pass_newsfeed_%d where "
            "user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu and "
            "sender_id=%u and target_id=%u and p_magic=%u;",
            db_id, t_id, sfid->mimi, sfid->cmd_id, sfid->app_id, sfid->timestamp, sfid->magic, 
            fid->sender_id, fid->target_id, fid->p_magic);
DEBUG_SQL(sql_buf);
        
    MYSQL_ROW row;
    i_mysql_iface *db = gconfig::get_db_conn(dis_id);
    if (db == NULL) {
        ERROR_LOG("[get pfeed key] fail to get database connection for mimi: %u", dis_id);
        return -1;
    }
    int row_cnt = db->select_first_row(&row, "%s", sql_buf);
    if (row_cnt > 0) {
        char *endptr = NULL;
        fkey->dbid = db_id;
        fkey->ftid = 0xFF;
        fkey->pftid = t_id;
        fkey->flag = 0;
        fkey->id = strtoul(row[0], &endptr, 10);
        return 0;
    }
    else if (row_cnt == 0) {
        WARN_LOG("get 0 row for get passive feed_key sql: %s", sql_buf);
        return -1;
    } else {
        ERROR_LOG("database error for get passive feed_key sql: %s, err str: %s", sql_buf, db->get_last_errstr());
        return -1;
    }
}

int pfeeds_database::get_feedid_by_cmdid(char *resbuf, const int buflen, int &reslen, const int units, get_p_feedid_by_cmdid_pkg_t *pkg)
{
    if (resbuf == NULL || units < 0 || units > MAX_ALLOW_UNITS) {
        ERROR_LOG("null value, or units not allowed %d for get_indexs", units);
        return -1;
    }

    int total_cnt = 0;
    char *buf_idx = resbuf;
    uint32_t dis_id = pkg->target_id;
    
    bool use_cmd_id = pkg->flag & 0x1;
    bool use_app_id = pkg->flag & 0x2;
    bool with_data  = pkg->flag & 0x4;
    bool use_limit  = pkg->flag & 0x8;
    const char * data_fld = with_data ? ",data " : " ";

    int sql_cnt = snprintf(sql_buf, max_buf_size, "select app_id,timestamp,magic,p_magic,active_time %s"
                "from db_newsfeed_%d.t_pass_newsfeed_%d "
                "where target_id=%u and user_id=%u", 
                data_fld, get_dbid(dis_id), get_tid(dis_id), pkg->target_id, pkg->mimi);

    char *new_buf = nullptr;
    if (use_cmd_id == true) {
        new_buf = sql_buf + sql_cnt;
        sql_cnt +=snprintf(new_buf, max_buf_size - sql_cnt, " and cmd_id = %u", pkg->cmd_id);
    }
    if (use_app_id == true) {
        new_buf = sql_buf + sql_cnt;
        sql_cnt +=snprintf(new_buf, max_buf_size - sql_cnt, " and app_id = %u", pkg->app_id);
    }
    if (use_limit == true) {
        new_buf = sql_buf + sql_cnt;
        sql_cnt += snprintf(new_buf, max_buf_size - sql_cnt, " order by timestamp desc limit 1");
    }
    DEBUG_SQL(sql_buf);
        
    i_mysql_iface *db = gconfig::get_db_conn(dis_id);
    if (db == NULL) {
        ERROR_LOG("[get passive feedid] fail to get database connection for mimi: %u", dis_id);
        return -1;
    }
    
    MYSQL_ROW row;
    int effect_rows = db->select_first_row(&row, "%s", sql_buf);
    if (effect_rows < 0) {
        ERROR_LOG("[get passive feedid] database failure, sql:%s, err str: %s", sql_buf, db->get_last_errstr());
        return -1;
    }
    else if (effect_rows == 0) {
        WARN_LOG("get 0 pfeedid, sql: %s", sql_buf);
        return total_cnt;    
    }
        
    char *endptr = NULL;
    for (int rowid = 0; rowid < effect_rows; rowid++) {
        pfeed_pkg_t *p_pfeed = (pfeed_pkg_t *)buf_idx;
        p_pfeed->fid.src_fid.mimi = pkg->mimi;
        p_pfeed->fid.src_fid.cmd_id = pkg->cmd_id;
        p_pfeed->fid.src_fid.app_id = strtoul(row[0], &endptr, 10);
        p_pfeed->fid.src_fid.timestamp = strtoul(row[1], &endptr, 10);
        p_pfeed->fid.src_fid.magic = (uint64_t)strtoull(row[2], &endptr, 10);
        p_pfeed->fid.sender_id = pkg->mimi;
        p_pfeed->fid.target_id = pkg->target_id;
        p_pfeed->fid.p_magic = strtoul(row[3], &endptr, 10);
        p_pfeed->active_time = strtoul(row[4], &endptr, 10);
    
        if (with_data == false) {
            p_pfeed->len = P_FEED_HEAD_SZ;
        } else {
            size_t vlen = strlen(row[5]); 
            p_pfeed->len = P_FEED_HEAD_SZ+vlen;
            memcpy(p_pfeed->data, row[5], vlen);
        }
        
        buf_idx += p_pfeed->len;
        if (buf_idx - resbuf > buflen) {
            ERROR_LOG("[get pfeed package] response package too long %ld bytes", buf_idx - resbuf);
            return -1;
        }
        total_cnt ++;         
        row = db->select_next_row(false);
    } //end of once fetch
    

    reslen = buf_idx - resbuf;
    return total_cnt;
}

int pfeeds_database::get_feedcnt_by_cmdid(char *resbuf, const int buflen, int &reslen, const int units, get_p_feed_cnt_t *pkg)
{
    if (resbuf == NULL || units < 0 || units > MAX_ALLOW_UNITS) {
        ERROR_LOG("null value, or units not allowed %d for get_indexs", units);
        return -1;
    }

    int total_cnt = 0;
    char *buf_idx = resbuf;
    uint32_t dis_id = pkg->mimi;
    
    snprintf(sql_buf, max_buf_size, "select count(*) from db_newsfeed_%d.t_pass_newsfeed_%d" 
            " where target_id = %u"
            " and cmd_id = %u"
            " and app_id = %u",
            get_dbid(dis_id), 
            get_tid(dis_id), 
            pkg->mimi,
            pkg->cmd_id,
            pkg->app_id);

    DEBUG_SQL(sql_buf);
        
    i_mysql_iface *db = gconfig::get_db_conn(dis_id);
    if (db == NULL) {
        ERROR_LOG("[get passive feedid] fail to get database connection for mimi: %u", dis_id);
        return -1;
    }
    
    MYSQL_ROW row;
    int effect_rows = db->select_first_row(&row, "%s", sql_buf);
    if (effect_rows < 0) {
        ERROR_LOG("[get passive feedid] database failure, sql:%s, err str: %s", sql_buf, db->get_last_errstr());
        return -1;
    }
    else if (effect_rows == 0) {
        WARN_LOG("get 0 pfeedid, sql: %s", sql_buf);
        return total_cnt;    
    }
        
    if (effect_rows != 1) {
        WARN_LOG("get 0 pfeed cnt, sql: %s", sql_buf);
        return 0;
    }

    pfeed_cnt_t *p_pfeed = (pfeed_cnt_t *)buf_idx;
    char *endptr = NULL;
    p_pfeed->cnt = strtoul(row[0], &endptr, 10);        
    DEBUG_LOG("[get passive feedcnt] database %u", p_pfeed->cnt);

    reslen = sizeof(uint32_t);
    return 1;
}
