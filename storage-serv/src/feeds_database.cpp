/**
 * =====================================================================================
 *       @file  feeds_database.cpp
 *      @brief  impl of feeds operations related database (insert/update/delete/get index/...)
 *
 *     Created  06/07/2011 01:33:15 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#include  "feeds_database.h"
#include  "config.h"
#include  "storage_proto.h"
/* feed struct in newsfeed system v1.1
 * +-----------+---------------------+------+-----+---------+-------+
 * | Field     | Type                | Null | Key | Default | Extra |
 * +-----------+---------------------+------+-----+---------+-------+
 * | id        | varchar(50)         | NO   |     | NULL    |       |
 * | user_id   | int(11) unsigned    | NO   |     | NULL    |       |
 * | cmd_id    | int(6) unsigned     | NO   |     | NULL    |       |
 * | app_id    | int(11) unsigned    | NO   |     | NULL    |       |
 * | timestamp | int(11) unsigned    | NO   |     | NULL    |       |
 * | magic     | bigint(20) unsigned | NO   |     | NULL    |       |
 * | data      | text                | NO   |     | NULL    |       |
 * +-----------+---------------------+------+-----+---------+-------+
 */
//const char *feeds_database::flds_list = " user_id, cmd_id, app_id, timestamp, magic, data ";

//char *feeds_database::sql_buf = NULL;
//char *feeds_database::sql2_buf = NULL;
//char *feeds_database::sql3_buf = NULL;
//bool feeds_database::inited = false;

static char sql_buf[SQL_MAX_SZ];
static char sql2_buf[SQL_MAX_SZ];
static char sql3_buf[SQL_MAX_SZ];

static const int sql_buf_max = SQL_MAX_SZ - 1;

bool feeds_database::init()
{
    return true;
}
void feeds_database::uninit()
{

}

int feeds_database::insert(i_mysql_iface *h_db, feedid_t *feedid, const char * feeddata, const int datalen)
{
    uint32_t mimi = feedid->mimi;

    int len = snprintf(sql_buf, sql_buf_max,
            "insert into %s (user_id, cmd_id, app_id, timestamp, magic, data) values (%u, %u, %u, %u, %lu, '%s')",
            gconfig::get_table_name(mimi),
            mimi, feedid->cmd_id, feedid->app_id, feedid->timestamp, feedid->magic, feeddata);
    if (len >= sql_buf_max || len < 0) {
        ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql_buf );
        return -1;
    }

DEBUG_SQL(sql_buf);
    int effect_row_cnt = h_db->execsql("%s", sql_buf);

    if (effect_row_cnt <= 0) {
        ERROR_LOG("fail to exec insert sql: %s, effect rows: %d,  sql err string: %s",
                sql_buf, effect_row_cnt, h_db->get_last_errstr());
        return -1;
    }
    else {
        return 0;
    }
}

int feeds_database::update2(i_mysql_iface *db, const feedid_t* oldfid, const feedid_t* newfid,
        const char* feeddata, const int datalen)
{
    int len = snprintf(sql_buf, sql_buf_max, "update %s "
            "set user_id=%u, cmd_id=%u, app_id=%u, timestamp=%u, magic=%lu, data = '%s' "
            "where user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu;",
            gconfig::get_table_name(newfid->mimi),
            newfid->mimi, newfid->cmd_id, newfid->app_id, newfid->timestamp, newfid->magic, feeddata,
            oldfid->mimi, oldfid->cmd_id, oldfid->app_id, oldfid->timestamp, oldfid->magic);
    if (len >= sql_buf_max || len < 0) {
        ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql_buf );
        return -1;
    }

DEBUG_SQL(sql_buf);
    int effect_row_cnt = db->execsql("%s", sql_buf);
    if (effect_row_cnt == 0) {
        WARN_LOG("update2 effect 0 row, update2 sql: %s, err str: %s",
                sql_buf, db->get_last_errstr());
    }
    else if (effect_row_cnt < 0) {
        ERROR_LOG("fail to execute sql:%s, err str: %s", sql_buf, db->get_last_errstr());
        return -1;
    }
    return 0;
}

int feeds_database::del(i_mysql_iface *db, feedid_t *fid)
{
    int len = snprintf(sql_buf, sql_buf_max, "delete from %s "
            " where user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu;",
            gconfig::get_table_name(fid->mimi),
            fid->mimi, fid->cmd_id, fid->app_id, fid->timestamp, fid->magic);
    if (len >= sql_buf_max || len < 0) {
        ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql_buf );
        return -1;
    }

DEBUG_SQL(sql_buf);
    int effect_row_cnt = db->execsql("%s", sql_buf);
    if (effect_row_cnt == 0) {
        WARN_LOG("del effect 0 row, del sql: %s, err str: %s", sql_buf, db->get_last_errstr());
    }
    else if (effect_row_cnt < 0) {
        ERROR_LOG("fail to execute sql:%s, err str: %s", sql_buf, db->get_last_errstr());
        return -1;
    }
    return 0;
}

//return successfully delete feed num
int feeds_database::dels(i_mysql_iface *db, feedid_list_t *fids)
{
    //TODO: complete this funtion perfectly, feedids belong to more than 1 mimi
    int success_num = 0;
    for (int i=0; i<fids->cur_used; i++) {
        if (del(db, &fids->list[i]) == 0)
            success_num ++;
    }

    return success_num;
}

int feeds_database::del2(i_mysql_iface *db, uint32_t mimi, uint16_t cmd_id, uint32_t app_id, uint64_t magic)
{
    int len = snprintf(sql_buf, sql_buf_max, "delete from %s "
            " where user_id=%u and cmd_id=%u and app_id=%u and magic=%lu;",
            gconfig::get_table_name(mimi), mimi, cmd_id, app_id, magic);
    if (len >= sql_buf_max || len < 0) {
        ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql_buf );
        return -1;
    }

DEBUG_SQL(sql_buf);
    int effect_row_cnt = db->execsql("%s", sql_buf);
    if (effect_row_cnt == 0 || effect_row_cnt > 1) {
        WARN_LOG("del effect %d row, del sql: %s, err str: %s", effect_row_cnt, sql_buf, db->get_last_errstr());
    }
    else if (effect_row_cnt < 0) {
        ERROR_LOG("fail to execute sql:%s, err str: %s", sql_buf, db->get_last_errstr());
        return -1;
    }
    return 0;
}

int feeds_database::get_pkg(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *pkg_len, feedid_t *fid)
{
    const char* presql = "select user_id, cmd_id, app_id, timestamp, magic, data from %s where "
        "(user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu)";
    int len = snprintf(sql_buf, sql_buf_max, presql, gconfig::get_table_name(fid->mimi),
            fid->mimi, fid->cmd_id, fid->app_id, fid->timestamp, fid->magic);

DEBUG_SQL(sql_buf);
    if (len >= sql_buf_max || len < 0) {
        ERROR_LOG("build sql meet error, sql len %d  max %d, sql: %s", len, sql_buf_max, sql_buf );
        return -1;
    }

    return get_feedspkg(db, buf, buflen, pkg_len, sql_buf, true);
}

//fetch feeds in same database and table, return feeds num (>=0) or error code(<0)
//feedid_t *fids, size_t fids_num/
int feeds_database::gets_pkg(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *pkgs_len, feedid_list_t * fidlist)
{
    std::map<std::string, std::vector<feedid_t*> > dbtable_vec;
    std::map<std::string, std::vector<feedid_t*> >::iterator dbtable_vec_it;
    for (int i=0 ; i<fidlist->cur_used; i++) {
        if (fidlist->flag[i] == 0) { //not fetched
            feedid_t *fid = &(fidlist->list[i]);
            std::string db_table_name(gconfig::get_table_name(fid->mimi));
            dbtable_vec_it = dbtable_vec.find(db_table_name);
            if (dbtable_vec_it == dbtable_vec.end()) {
                std::vector<feedid_t*> vec;
                vec.push_back(fid);
                dbtable_vec.insert(std::pair<std::string, std::vector<feedid_t*> >(db_table_name, vec));
            } else {
                dbtable_vec_it->second.push_back(fid);
            }
        }
    }
    int   total_rows = 0;
    int   buf_idx = 0;
    //fetch from each db table
    for (dbtable_vec_it = dbtable_vec.begin(); dbtable_vec_it != dbtable_vec.end(); dbtable_vec_it ++) {
        uint32_t mimi_token = (dbtable_vec_it->second)[0]->mimi;
        uint32_t dbid = get_dbid(mimi_token);
        i_mysql_iface *p_db = gconfig::get_bydbid(dbid);
        if (p_db == NULL) {
            ERROR_LOG("[store::get_pkgs] fail to get database connection for db id: %u", dbid);
            break;
        }

        std::vector<feedid_t*>  fids = dbtable_vec_it->second;
        if (fids.size() == 0) {
            continue;
        }
        const char* presql = "select user_id, cmd_id, app_id, timestamp, magic, data from %s where "
            "(user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu) ";
        int sql_idx = snprintf(sql_buf, sql_buf_max, presql, gconfig::get_table_name(fids[0]->mimi),
                fids[0]->mimi, fids[0]->cmd_id, fids[0]->app_id, fids[0]->timestamp, fids[0]->magic);

        const char* cond = " or (user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu)";
        for (size_t i = 1 ; i < fids.size() ; i++) {
            sql_idx += snprintf(sql_buf + sql_idx, sql_buf_max - sql_idx, cond,
                    fids[i]->mimi, fids[i]->cmd_id, fids[i]->app_id, fids[i]->timestamp, fids[i]->magic);
        }
DEBUG_SQL(sql_buf);
        if (sql_idx >= sql_buf_max || sql_idx < 0) {
            ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", sql_idx, sql_buf_max, sql_buf );
            return -1;
        }

        uint32_t once_len = 0;
        int db_rows = get_feedspkg(db, buf + buf_idx, buflen - buf_idx, &once_len, sql_buf, true);

        if (db_rows == 0) { //0 row
            WARN_LOG("fetch << 0 >> feed package from db table: %s", dbtable_vec_it->first.c_str());
            continue;
        }
        else if (db_rows < 0) { //database error
            ERROR_LOG("ERROR when fetch feed package from db table: %s", dbtable_vec_it->first.c_str());
            break;
        }
        else { //num > 0 rows feed

            total_rows += db_rows;
            //set buf offset for next
            buf_idx += once_len;
        } //end of db_rows
    } //end of fetch from db table

    *pkgs_len = buf_idx;
    return total_rows;
}

int feeds_database::gets_key_pkg(char *buf, size_t buflen, uint32_t *pkgs_len, feedid_list_t * fidlist)
{
    if (buf == 0 || pkgs_len == 0 || fidlist == 0) {
        ERROR_LOG("Null Value for gets2 feeds package");
        return -1;
    }

    int total_rows = 0;
    int buf_idx = 0;
    MYSQL_ROW row;
    char *endptr = NULL;
    for (int i=0 ; i < fidlist->cur_used ; i++) {
        if (fidlist->flag[i] != F_NONE) {
            continue;
        }

        feedid_t *fid = fidlist->list + i;
        uint32_t dis_id = fid->mimi;
        int db_id = get_dbid(dis_id);
        int t_id = get_tid(dis_id);
        snprintf(sql_buf, SQL_MAX_SZ -1, "select user_id,data from db_newsfeed_%d.t_newsfeed_%d "
                "where user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu;",
                db_id, t_id, dis_id, fid->cmd_id, fid->app_id, fid->timestamp, fid->magic);
    DEBUG_SQL(sql_buf);
        i_mysql_iface *db = gconfig::get_db_conn(dis_id);
        if (db == NULL) {
            ERROR_LOG("[get key feed package] fail to get database connection for mimi: %u", dis_id);
            break;
        }
        int row_cnt = db->select_first_row(&row, "%s", sql_buf);
        if (row_cnt > 0) {
            int datalen = strlen(row[1]);
            feed_key_pkg_t *kfeed =(feed_key_pkg_t *)(buf + buf_idx);
            kfeed->len = sizeof(feed_key_pkg_t) + datalen;
            buf_idx += kfeed->len;
            if ((size_t)buf_idx >= buflen) {
                ERROR_LOG("feed package build from database too long");
                buf_idx -= kfeed->len;
                break;
            }
            kfeed->key.dbid = db_id;
            kfeed->key.ftid = t_id;
            kfeed->key.pftid = 0xFF;
            kfeed->key.flag = 0;
            kfeed->key.id = strtoul(row[0], &endptr, 10);
            memcpy(&kfeed->id, fid, feedidsz);
            memcpy(kfeed->data, row[1], datalen);
            fidlist->flag[i] = F_DB;
            total_rows ++;
        }
        else if (row_cnt == 0) {
            WARN_LOG("get 0 row for gets key feed sql: %s", sql_buf);
        } else {
            ERROR_LOG("database error for get_feed_key sql: %s, err str: %s", sql_buf, db->get_last_errstr());
        }
    } /*-- end of for --*/

    *pkgs_len = buf_idx;
    return total_rows;
}

int feeds_database::get_key(feedid_t *fid, feedkey_t *fkey)
{
    if (fid == 0 || fkey == 0) {
        ERROR_LOG("NULL value for feed database get key");
        return -1;
    }

    uint32_t dis_id = fid->mimi;
    int db_id = get_dbid(dis_id);
    int t_id = get_tid(dis_id);
    snprintf(sql_buf, SQL_MAX_SZ -1, "select id from db_newsfeed_%d.t_newsfeed_%d where "
            "user_id=%u and cmd_id=%u and app_id=%u and timestamp=%u and magic=%lu;",
            db_id, t_id, dis_id, fid->cmd_id, fid->app_id, fid->timestamp, fid->magic);
DEBUG_SQL(sql_buf);

    MYSQL_ROW row;
    i_mysql_iface *db = gconfig::get_db_conn(dis_id);
    if (db == NULL) {
        ERROR_LOG("[get key feed package] fail to get database connection for mimi: %u", dis_id);
        return -1;
    }

    int row_cnt = db->select_first_row(&row, "%s", sql_buf);
    if (row_cnt > 0) {
        char *endptr = NULL;
        fkey->dbid = db_id;
        fkey->ftid = t_id;
        fkey->pftid = 0xFF;
        fkey->flag = 0;
        fkey->id = strtoul(row[0], &endptr, 10);
        return 0;
    }
    else if (row_cnt == 0) {
        WARN_LOG("get 0 row for get_feed_key sql: %s", sql_buf);
        return -1;
    } else {
        ERROR_LOG("database error for get_feed_key sql: %s, err str: %s", sql_buf, db->get_last_errstr());
        return -1;
    }
}


int feeds_database::gets2_pkg(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *pkgs_len, get2_pkg_t *pkg)
{
    if (db == 0 || buf == 0 || pkgs_len == 0 || pkg == 0) {
        ERROR_LOG("Null Value for gets2 feeds package");
        return -1;
    }

    snprintf(sql_buf, SQL_MAX_SZ - 1, "select user_id, cmd_id, app_id, timestamp, magic, data from %s "
            "where user_id=%u and cmd_id=%u and app_id=%u and magic=%lu order by timestamp desc limit %d;",
            gconfig::get_table_name(pkg->mimi), pkg->mimi, pkg->cmd_id, pkg->app_id, pkg->magic, GET2_LIMIT);

DEBUG_SQL(sql_buf);
    int db_rows = get_feedspkg(db, buf, buflen, pkgs_len, sql_buf, true);
    return db_rows;
}

inline char* feeds_database::get_nidx_presql(char *sqlbuf, size_t buflen, uint32_t mimi, uint16_t cmd_id, uint32_t app_id,
        bool use_cmd_id, bool use_app_id, bool with_data)
{
    const char * data_fld = with_data ? ", data" : " ";
    const char * cmd_fld = use_cmd_id ? "and cmd_id = %u" : " ";
    const char * app_fld = use_app_id ? "and app_id = %u" : " ";

    int sql_cnt = snprintf(sqlbuf, buflen, "select user_id, cmd_id, app_id, timestamp, magic %s from %s "
            "where user_id=%u %s %s ",
            data_fld, gconfig::get_table_name(mimi), mimi, cmd_fld, app_fld);

    if (use_cmd_id == false && use_app_id == false) {
        return sqlbuf;
    } else {
        char *newsql = sqlbuf + sql_cnt + 2;
        if (use_cmd_id == true && use_app_id == false)
            sql_cnt = snprintf(newsql, buflen - sql_cnt, sqlbuf, cmd_id);
        else if (use_cmd_id == false && use_app_id == true)
            sql_cnt = snprintf(newsql, buflen - sql_cnt, sqlbuf, app_id);
        else if (use_cmd_id == true && use_app_id == true)
            sql_cnt = snprintf(newsql, buflen - sql_cnt, sqlbuf, cmd_id, app_id);

        return newsql;
    }
}
//return feed pkg len(len| feedid | feeddata) or error code(<0)
inline int feeds_database::build_feed(MYSQL_ROW row, char *buf, size_t buflen, bool withdata)
{
    feed_pkg_t *pkg = (feed_pkg_t *)buf;
    char *endptr = NULL;
    pkg->feedid.mimi = strtoul(row[0], &endptr, 10);
    pkg->feedid.cmd_id = strtoul(row[1], &endptr, 10);
    pkg->feedid.app_id = strtoul(row[2], &endptr, 10);
    pkg->feedid.timestamp = strtoul(row[3], &endptr, 10);
    pkg->feedid.magic = (uint64_t)strtoull(row[4], &endptr, 10);
    uint32_t pkg_len = 0;
    if (withdata == false) {
        pkg_len = feed_hsz;
    } else {
        size_t vlen = strlen(row[5]);
        memcpy(pkg->data, row[5], vlen);
        pkg_len = feed_hsz + vlen;
    }
    pkg->len = pkg_len;

    return pkg_len;
}
//return feeds num (>=0) or error code(<0)
inline int feeds_database::get_feedspkg(i_mysql_iface *db, char *buf, size_t buflen,
        uint32_t *pkgs_len, char *sql, bool withdata)
{
    MYSQL_ROW row;
    int row_cnt = db->select_first_row(&row, "%s", sql);
    if (row_cnt > 0) {
        uint32_t idx = 0;
        for (int i = 0 ; i < row_cnt && row != NULL; i++) {
            idx += build_feed(row, buf + idx, buflen - idx, withdata);

            row = db->select_next_row(false);
        }

        *pkgs_len = idx;
    }
    else if (row_cnt == 0) {
        *pkgs_len = 0;
    } else {
        ERROR_LOG("database error for sql: %s, err str: %s", sql, db->get_last_errstr());
        *pkgs_len = 0;
    }
    return row_cnt;
}
//return feed pkg num (>=0) or error code(<0)
int feeds_database::get_nindexs(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *fids_len,
        uint32_t mimi, uint16_t flag, uint16_t cmd_id, uint32_t app_id,
        uint32_t starttime, uint16_t after, uint16_t before)
{
    bool use_cmd_id = flag & 0x1;
    bool use_app_id = flag & 0x2;
    bool with_data  = flag & 0x4;

    if (before > 0 && after == 0) {
        char *sql = get_nidx_presql(sql_buf, SQL_MAX_SZ, mimi, cmd_id, app_id, use_cmd_id, use_app_id, with_data);
        strcat(sql, " and timestamp <= %u order by timestamp desc limit %u;");

        int len = snprintf(sql2_buf, sql_buf_max, sql, starttime, before);
        if (len >= sql_buf_max || len < 0) {
            ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql2_buf);
            return -1;
        }
        DEBUG_SQL(sql2_buf);
        return get_feedspkg(db, buf, buflen, fids_len, sql2_buf, with_data);
    }
    else if (after > 0 && before == 0) {
        char *sql = get_nidx_presql(sql_buf, SQL_MAX_SZ, mimi, cmd_id, app_id, use_cmd_id, use_app_id, with_data);
        strcat(sql, " and timestamp >= %u order by timestamp limit %u) tmp order by timestamp desc;");
        snprintf(sql2_buf, sql_buf_max, sql, starttime, after);

        int len = snprintf(sql_buf, sql_buf_max, "select * from (%s", sql2_buf);
        if (len >= sql_buf_max || len < 0) {
            ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql_buf);
            return -1;
        }

DEBUG_SQL(sql_buf);
        return get_feedspkg(db, buf, buflen, fids_len, sql_buf, with_data);
    }
    else if (before > 0 && after > 0) {
        //(select * from t_newsfeed_index where timestamp >= 1302763166 order by timestamp asc limit 3)
        //union (select * from t_newsfeed_index where timestamp <= 1302763166 order by timestamp desc  limit 3)
        //order by timestamp desc;
        //build after feedids
        char *sql = get_nidx_presql(sql_buf, SQL_MAX_SZ, mimi, cmd_id, app_id, use_cmd_id, use_app_id, with_data);
        strcat(sql, " and timestamp <= %u order by timestamp desc limit %u");
        snprintf(sql2_buf, SQL_MAX_SZ, sql, starttime, before);
        //build before
        sql = get_nidx_presql(sql_buf, sql_buf_max, mimi, cmd_id, app_id, use_cmd_id, use_app_id, with_data);
        strcat(sql, " and timestamp >= %u order by timestamp asc limit %u");
        snprintf(sql3_buf, sql_buf_max, sql, starttime, after);

        int len = snprintf(sql_buf, sql_buf_max, "(%s) union (%s) order by timestamp desc;", sql2_buf, sql3_buf);
        if (len >= sql_buf_max || len < 0) {
            ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql_buf);
            return -1;
        }

DEBUG_SQL(sql_buf);
        return get_feedspkg(db, buf, buflen, fids_len, sql_buf, with_data);
    }
    else { //(before == 0 && after == 0)
        WARN_LOG("receive get_nindex package with before 0 and after 0, impossible");
        return 0;
    }
}

int feeds_database::get_spanindexs(i_mysql_iface *db, char *buf, size_t buflen, uint32_t *fids_len,
        uint32_t mimi, uint16_t flag, uint16_t cmd_id, uint32_t app_id,
        uint32_t starttime, uint32_t endtime)
{
    bool use_cmd_id = flag & 0x1;
    bool use_app_id = flag & 0x2;
    bool with_data  = flag & 0x4;
    char *sql = get_nidx_presql(sql_buf, SQL_MAX_SZ, mimi, cmd_id, app_id, use_cmd_id, use_app_id, with_data);
    strcat(sql, " and timestamp between %u and %u order by timestamp desc;");
    int len = snprintf(sql2_buf, sql_buf_max, sql, starttime, endtime);
    if (len >= sql_buf_max || len < 0) {
        ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql2_buf);
        return -1;
    }

DEBUG_SQL(sql2_buf);
    return get_feedspkg(db, buf, buflen, fids_len, sql2_buf, with_data);
}

bool feeds_database::check_existed(i_mysql_iface *db, feedid_t *fid)
{
    int len = snprintf(sql_buf, sql_buf_max, "select user_id from %s where user_id=%u and cmd_id=%u and app_id=%u "
            "and timestamp=%u and magic=%lu;",
            gconfig::get_table_name(fid->mimi), fid->mimi, fid->cmd_id, fid->app_id, fid->timestamp, fid->magic);
    if (len >= sql_buf_max || len < 0) {
        ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql_buf);
        return -1;
    }

DEBUG_SQL(sql_buf);
    MYSQL_ROW row;
    int effect_rows = db->select_first_row(&row, "%s", sql_buf);
    if (effect_rows < 0) {
        ERROR_LOG("[check_existed] database failure, sql:%s, err str: %s", sql_buf, db->get_last_errstr());
        return false;
    }

    return effect_rows == 0 ? false : true;
}

int feeds_database::get_feedids(i_mysql_iface *db, uint32_t mimi, uint16_t cmd_id, uint32_t app_id, uint64_t magic, feedid_list_t *list)
{
    if (list == NULL) {
        return -1;
    }
    int len = snprintf(sql_buf, sql_buf_max, "select user_id, cmd_id, app_id, timestamp, magic from %s "
            "where user_id=%u and cmd_id=%u and app_id=%u and magic=%lu;",
            gconfig::get_table_name(mimi), mimi, cmd_id, app_id, magic);
    if (len >= sql_buf_max || len < 0) {
        ERROR_LOG("build sql meet error, sql statement len %d > max %d, sql: %s", len, sql_buf_max, sql_buf);
        return -1;
    }

DEBUG_SQL(sql_buf);
    MYSQL_ROW row;
    int effect_rows = db->select_first_row(&row, "%s", sql_buf);
    if (effect_rows < 0) {
        ERROR_LOG("[get_feedids] database failure, sql:%s, err str: %s", sql_buf, db->get_last_errstr());
    }
    else if (effect_rows == 0) {
        WARN_LOG("get 0 feedid, sql: %s, last mysql err str: %s",
                sql_buf, db->get_last_errstr());
    }
    list->cur_used = 0;
    char *endptr = NULL;

    for (int i = 0; i < effect_rows; i++) {
        (*list)[i].mimi = strtoul(row[0], &endptr, 10);
        (*list)[i].cmd_id = strtoul(row[1], &endptr, 10);
        (*list)[i].app_id = strtoul(row[2], &endptr, 10);
        (*list)[i].timestamp = strtoul(row[3], &endptr, 10);
        (*list)[i].magic = strtoull(row[4], &endptr, 10);
        list->flag[i] = 0;
        row = db->select_next_row(false);
    }

    list->cur_used = effect_rows;
    return effect_rows;
}
