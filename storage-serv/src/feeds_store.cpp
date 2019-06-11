/**
 * =====================================================================================
 *       @file  feeds_store.cpp
 *      @brief  wrapper of impl of feeds storage (local/memcached/database)
 *
 *     Created  06/13/2011 10:26:56 AM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#include  "config.h"
#include  "feeds_database.h"
#include  "feeds_memcached.h"
#include  "pfeeds_database.h"
#include  "pfeeds_memcached.h"
#include  "feeds_store.h"
#include  "stat_analysis.h"

#define     MAX_FEEDDATA_SZ     (1024*1024)
bool feeds_store::inited = false;
char *feeds_store::esc_data_buf = NULL;
feedid_list_t feeds_store::fidlist = { 0, 0, {0}, {0}, {0}};
pfeedid_list_t feeds_store::pfidlist = { 0, 0, {0}, {0}, {0}};

bool feeds_store::init()
{
    if (inited == false) {
        //escape string may make string enlarge to double size
        esc_data_buf = (char*) malloc(MAX_FEEDDATA_SZ);
        fidlist.init();
        pfidlist.init();
        inited = true;
    }

    return inited;
}

void feeds_store::uninit()
{
    if (inited == true) {
        fidlist.uninit();
        pfidlist.uninit();
        inited = false;
    }
}

int feeds_store::insert(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, insert_pkg_t *pkg)
{
    if (units == 1) {
        feedid_t* feedid = &pkg->feedid;
        uint32_t mimi = feedid->mimi;
        i_mysql_iface *p_db = gconfig::get_db_conn(mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::insert] fail to get database connection for mimi: %u", mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }

        bool fid_existed = feeds_database::check_existed(p_db, feedid);
        if (fid_existed == true) {
            ERROR_LOG("feedid %s existed", get_feedidstr(feedid));
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_FEEDID_DUP);
            return -1;
        }

        unsigned long datalen = (pkg->len - sizeof(insert_pkg_t));
        int esc_datalen = mysql_real_escape_string(p_db->get_conn(), esc_data_buf, (char*)pkg->data, datalen);

        // insert into db
        int ret = feeds_database::insert(p_db, feedid, esc_data_buf, esc_datalen);
        if (ret != 0) {
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            //*res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
            ERROR_LOG("fail to insert feed %s into database ", get_feedidstr(feedid));
            return -1;
        }

//        memcached_st *memc = gconfig::get_mem_handle();
//        if (memc == NULL) {
//            ERROR_LOG("[store::insert] fail to get memcached server connection");
//        } else {
//            ret = feeds_memcached::add(memc, feedid, pkg->data, datalen);
//            if (ret != 0)
//                WARN_LOG("fail to add feed to memcached server, but insert into db success, log and pass");
//        }

        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
        return 0;
    } else { //TODO: impl mutil feeds
        WARN_LOG("insert request with multi units %u", units);
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return 0;
    }
}

int feeds_store::update(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, update_pkg_t *pkg)
{
    if (units == 1) { //update one feed
        feedid_t* oldfid = &pkg->feedid;
        feedid_t* newfid = &pkg->feedid;
        uint32_t mimi = newfid->mimi;

        i_mysql_iface *p_db = gconfig::get_db_conn(mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::update] fail to get database connection for mimi: %u", mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
        }

        bool fid_existed = feeds_database::check_existed(p_db, oldfid);
        if (fid_existed == false) {
            ERROR_LOG("[store::update] feedid %s not existed", get_feedidstr(oldfid));
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_FEEDID);
            return -1;
        }

        //update  database
        unsigned long datalen = (pkg->len - sizeof(update_pkg_t));
        int esc_datalen = mysql_real_escape_string(p_db->get_conn(), esc_data_buf, pkg->data, datalen);
        int ret = feeds_database::update2(p_db, oldfid, newfid, esc_data_buf, esc_datalen);
        if (ret != 0) {
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            ERROR_LOG("fail to update feed for (user:%u timestamp:%u) into database ", mimi, newfid->timestamp);
            return -1;
        }
        //update  memcached, do not affect process if failed
        memcached_st *memc = gconfig::get_mem_handle();
        if (memc == NULL) {
            ERROR_LOG("[store::update] fail to get memcached server connection");
        } else {
            ret = feeds_memcached::replace(memc, newfid, pkg->data, datalen);
#ifdef TRACE_DEBUG
            if (ret != 0) {
                DEBUG_WARN("[store::update] fail to update(replace) feed to memcached server, log and pass");
            }
#endif
        }

        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
        return 0;
    }
    else { //TODO: impl mutil
        WARN_LOG("update package with multi units %u", units);
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return 0;
    }
}

inline bool feeds_store::feedid_equal(feedid_t *lhs, feedid_t *rhs)
{
    if (lhs == rhs) {
        return true;
    } else {
        return ((lhs->mimi == rhs->mimi) && (lhs->cmd_id == rhs->cmd_id)  &&
                (lhs->app_id == rhs->app_id) && (lhs->timestamp == rhs->timestamp) && (lhs->magic == rhs->magic));
    }
}
int feeds_store::update2(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, update2_pkg_t *pkg)
{
    if (units == 1) { //update one feed
        feedid_t* oldfid = &pkg->old_feedid;
        feedid_t* newfid = &pkg->new_feedid;
        uint32_t old_mimi = oldfid->mimi;
        uint32_t new_mimi = newfid->mimi;

        i_mysql_iface *p_db = gconfig::get_db_conn(new_mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::update2] fail to get database connection for mimi: %u", new_mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
        }

        if (old_mimi != new_mimi) {
            ERROR_LOG("[store::update2] oldmimi %u != newmimi %u", old_mimi, new_mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_REQUEST);
            return -1;
        }

        bool is_fid_equal = feedid_equal(newfid, oldfid);
        bool fid_db_existed = feeds_database::check_existed(p_db, newfid);
        if ( !is_fid_equal  &&  fid_db_existed ) {
            WARN_LOG("fail to update2, old feedid != new feedid and new feedid existed in db"
                    "old feed id: %s new feed id: %s", get_feedidstr(oldfid), get_feedidstr(newfid));
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_FEEDID_DUP);
            return -1;
        }

        unsigned long datalen = (pkg->len - sizeof(update2_pkg_t));
        int esc_datalen = mysql_real_escape_string(p_db->get_conn(), esc_data_buf, pkg->data, datalen);
        //update  database
        int ret = feeds_database::update2(p_db, oldfid, newfid, esc_data_buf, esc_datalen);
        if (ret != 0) {
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            ERROR_LOG("fail to update2 feed [old %s new %s] into database ", get_feedidstr(newfid), get_feedidstr(newfid));
            return -1;
        }
        //update  memcached, do not affect process if failed
        memcached_st *memc = gconfig::get_mem_handle();
        if (memc == NULL) {
            ERROR_LOG("[store::update2] fail to get memcached server connection");
        } else {
            uint32_t len = 0;
            ret = feeds_memcached::get(memc, send_buf, buflen, &len, oldfid);
            if (ret == 0) { //existed in memc
                ret = feeds_memcached::del(memc, oldfid);
#ifdef TRACE_DEBUG
                if (ret != 0) {
                    DEBUG_WARN("[store::update2] fail to del oldfid %s, log and pass", get_feedidstr(oldfid));
                }
#endif
                ret = feeds_memcached::add(memc, newfid, pkg->data, datalen);
#ifdef TRACE_DEBUG
                if (ret != 0) {
                    DEBUG_WARN("[store::update2] fail to add newfid %s, log and pass", get_feedidstr(newfid));
                }
#endif
                //if (is_fid_equal) {  //set
                //    //ret = feeds_memcached::set(memc, newfid, pkg->data, datalen);
                //    ret = feeds_memcached::replace(memc, newfid, pkg->data, datalen);
                //} else { //del old, add new
                //    //ret = feeds_memcached::add(memc, newfid, pkg->data, datalen);
                //    ret = feeds_memcached::replace(memc, newfid, pkg->data, datalen);
                //}
                //if (ret != 0) {
                //    DEBUG_WARN("fail to update2 feed to memcached server, but update2 into db success, log and pass");
                //}
            }
#ifdef TRACE_DEBUG
            else {
                DEBUG_WARN("oldfeedid %s do not existed", get_feedidstr(oldfid));
            }
#endif
        }

        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
        return 0;
    }
    else { //TODO: impl mutil
        WARN_LOG("update2 package with multi units %u", units);

        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return 0;
    }
}

int feeds_store::set(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, set_pkg_t *pkg)
{
    if (units == 1) { //one feed
        feedid_t* oldfid = &pkg->old_feedid;
        feedid_t* newfid = &pkg->new_feedid;
        uint32_t old_mimi = oldfid->mimi;
        uint32_t new_mimi = newfid->mimi;

        i_mysql_iface *p_db = gconfig::get_db_conn(old_mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::set] fail to get database connection for mimi: %u", new_mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }

        if (old_mimi != new_mimi) {
            ERROR_LOG("[store::set] oldmimi %u != newmimi %u", old_mimi, new_mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_REQUEST);
            return -1;
        }

        unsigned long datalen = (pkg->len - sizeof(update2_pkg_t));
        int esc_datalen = mysql_real_escape_string(p_db->get_conn(), esc_data_buf, pkg->data, datalen);

        bool oldfid_existed = feeds_database::check_existed(p_db, oldfid);
        if (oldfid_existed == true) {  //update
            int ret = feeds_database::update2(p_db, oldfid, newfid, esc_data_buf, esc_datalen);
            if (ret != 0) {
                *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
                ERROR_LOG("fail to update2 feed into database in [store::set] operation");
                return -1;
            }
        } else { //insert
            int ret = feeds_database::insert(p_db, newfid, esc_data_buf, esc_datalen);
            if (ret != 0) {
                *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
                ERROR_LOG("fail to insert feed into database in [store::set] operation");
                return -1;
            }
        }

        //set  memcached, do not affect process if failed
        memcached_st *memc = gconfig::get_mem_handle();
        if (memc == NULL) {
            ERROR_LOG("[store::set] fail to get memcached server connection");
        } else {
            bool is_fid_equal = feedid_equal(newfid, oldfid);
            if (oldfid_existed == true && is_fid_equal == false) {
                feeds_memcached::del(memc, oldfid);
                int ret = feeds_memcached::add(memc, newfid, pkg->data, datalen);
                if (ret != 0) {
                    ERROR_LOG("fail to add feed to memcached, set db success, log and pass");
                }
            } else {
                int ret = feeds_memcached::set(memc, newfid, pkg->data, datalen);
                if (ret != 0) {
                    ERROR_LOG("fail to set feed to memcached, insert db success, log and pass");
                }
            }
        }

        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
        return 0;
    }
    else { //TODO: impl mutil
        WARN_LOG("set package with multi units %u", units);
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return 0;
    }
}

//return length of statu package (fixed size>0) or error code
inline int feeds_store::build_stat_pkg(char *buf, size_t buflen, feedid_t *fid, uint16_t status)
{
    feed_status_pkg_t *status_pkg = (feed_status_pkg_t*)(buf);
    memcpy(&status_pkg->feedid, fid, feedidsz);
    status_pkg->status = status;

    return stat_pkg_len;
}
int feeds_store::del(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, delete_pkg_t *pkg)
{
    if (units == 1) {
        feedid_t *fid = &pkg->feedid;
        uint32_t mimi = fid->mimi;

        int ret = 0;
        //del feed in memcached, do not affect process if failed
        memcached_st *memc = gconfig::get_mem_handle();
        if (memc == NULL) {
            ERROR_LOG("[store::del] fail to get memcached connection");
        } else {
            ret = feeds_memcached::del(memc, fid);
            if (ret != 0) {
                WARN_LOG("[store::del] fail to del feed in memcached, log and pass");
            }
        }

        //del feed in db
        i_mysql_iface *p_db = gconfig::get_db_conn(mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::del] fail to get database connection for mimi: %u", mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }
        ret = feeds_database::del(p_db, fid);
        if (ret != 0) {
            ERROR_LOG("[store::del] fail to del feed %s in database", get_feedidstr(fid));
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }

        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
        return 0;
    }
    else { //units > 1, batch
        response_pkg_t *send =  (response_pkg_t *)send_buf;
        uint32_t send_pkgslen = 0;
        uint32_t offset = res_hsz;
        bool anyfail = false;

        memcached_st *memc = gconfig::get_mem_handle();
        if (memc == NULL) {
            ERROR_LOG("[store::del] fail to get memcached server connection");
        }

        for (int i=0 ; i < units ; i++) {
            feedid_t *fid = &(pkg[i].feedid);
            offset += i*(stat_pkg_len);

            //del feed in memcached, do not affect process if failed
            int ret = feeds_memcached::del(memc, fid);
            if (ret != 0) {
                WARN_LOG("fail to del feed in memcached, log and pass");
            }

            //del feed in db
            uint32_t mimi = fid->mimi;
            i_mysql_iface *p_db = gconfig::get_db_conn(mimi);
            if (p_db == NULL) {
                ERROR_LOG("[store::del] fail to get database connection for mimi: %u", mimi);
                send_pkgslen += build_stat_pkg(send_buf + offset, buflen - offset, fid, RES_OP_ERROR);
                anyfail = true;
                continue;
            }
            ret = feeds_database::del(p_db, fid);
            if (ret != 0) {
                WARN_LOG("fail to del feed %s in database", get_feedidstr(fid));
                send_pkgslen += build_stat_pkg(send_buf + offset, buflen - offset, fid, RES_OP_ERROR);
                anyfail = true;
                continue;
            }

            send_pkgslen += build_stat_pkg(send_buf + offset, buflen - offset, fid, RES_OP_SUCCESS);
        } //end of for
        send->len = res_hsz + send_pkgslen;
        send->ret = anyfail ? RES_OP_ERROR : RES_OP_SUCCESS;
        send->units = units;
        *res_pkg_len = send->len;
        return 0;
    }
}

int feeds_store::del2(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, delete2_pkg_t *pkg)
{
    if (units == 1) {
        uint32_t mimi = pkg->mimi;
        i_mysql_iface *p_db = gconfig::get_db_conn(mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::del2] fail to get database connection for mimi: %u", mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }
        //del in memcached
        int ret = feeds_database::get_feedids(p_db, pkg->mimi, pkg->cmd_id, pkg->app_id, pkg->magic, &fidlist);
        if (ret < 0) {
            ERROR_LOG("fail to get feedids for delete2 request");
        }
        else if (ret == 0) {
            WARN_LOG("get << 0 >> feedids for delete2 request");
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
            return 0;
        } else {
            memcached_st *memc = gconfig::get_mem_handle();
            if (memc == NULL) {
                ERROR_LOG("[store::del2] fail to get memcached server connection");
            } else {
                for (int i = 0; i < fidlist.cur_used; i++) {
                    ret = feeds_memcached::del(memc, &(fidlist[i]));
                    if (ret != 0) {
                        WARN_LOG("fail to del feed in memcached, log and pass");
                    }
                }
            }
        }

        //del in db
        ret = feeds_database::del2(p_db, mimi, pkg->cmd_id, pkg->app_id, pkg->magic);
        if (ret != 0) {
            ERROR_LOG("fail to del2 feed (user:%u magic:%lu) in database", mimi, pkg->magic);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }

        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
        return 0;
    }
    else { //TODO: when needed
        WARN_LOG("del2 feeds with multi units %u", units);
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return 0;
    }
}

int feeds_store::get_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, get_pkg_t *pkg_start)
{
    fidlist.clear();
    memcpy(fidlist.list, pkg_start, units*sizeof(get_pkg_t));
    fidlist.cur_used = units;

    int total_rows = 0;
    uint32_t buf_idx = res_hsz;

#ifdef TRACE_DEBUG
    static c_time stat_timestart;
    static c_time stat_timeend;
#endif
    //gets from memcached
    memcached_st *memc = gconfig::get_mem_handle();
    uint32_t mem_datalen = 0;
    int mem_rows = 0;
    if (memc == NULL) {
        ERROR_LOG("[store::get_pkgs] fail to get memcached server connection");
    } else {
    STAT_START();
        mem_rows = feeds_memcached::gets(memc, send_buf + buf_idx, buflen - buf_idx, &mem_datalen, &fidlist);
        if (mem_rows < 0) {
            ERROR_LOG("[store::get_pkgs] fail to get feed from memcached server, << row < 0 >> pass");
        } else {
            total_rows += mem_rows;
            buf_idx += mem_datalen;
        }
    STAT_ENDTIME(&pkg_start->feedid, "get_mem_pkgs");
    }

    //gets from database , not memcached feeds
    if (mem_rows < units)
    {
    STAT_START();
        for (int i=0 ; i < fidlist.cur_used ; i++) {
            if(fidlist.flag[i] != 0) {
                continue;
            }
            i_mysql_iface *p_db = gconfig::get_db_conn(fidlist[i].mimi);
            if (p_db == NULL) {
                ERROR_LOG("[feeds_store::get_pkgs] get db conn handle is NULL for mimi %u", fidlist[i].mimi);
                continue;
            }
            uint32_t once_len = 0;
            int ret = feeds_database::get_pkg(p_db, send_buf + buf_idx, buflen - buf_idx, &once_len, &(fidlist[i]));
            if (ret < 0) {
                ERROR_LOG("[feeds_store::get_pkgs] fail to fetch from database for feed %s", get_feedidstr(&(fidlist[i])));
            } else if (ret == 0) {
                WARN_LOG("[feeds_store::get_pkgs] feed %s do not in database", get_feedidstr(&(fidlist[i])));
            } else { //fetch one
                feed_pkg_t *feed = (feed_pkg_t *)(send_buf + buf_idx);

                int memc_ret = feeds_memcached::set(memc, &feed->feedid, feed->data, feed->len - feed_hsz);
                if (memc_ret != 0) {
                    WARN_LOG("[feeds_store::get_pkgs] fail to set feed ( %s ) to memcached, log and pass",
                            get_feedidstr(&feed->feedid));
                }

                fidlist.flag[i] = 4; //set flag from db
                total_rows += ret;
                buf_idx += once_len;

        STAT_PKGLEN(&feed->feedid, feed->len - feed_hsz);
            }
        } //end of for
    STAT_ENDTIME(&pkg_start->feedid, "get_db_pkgs");
    }

    //feeds hit statistic msglog
    static storage_hits  hit = {0, 0};
    static time_t timer = time(0);
    hit.mem_num += mem_rows > 0 ? mem_rows : 0;
    hit.db_num += total_rows - mem_rows;
    storage_hits_log(0x0D000411, &timer, &hit);

    //time_t now_timer = time(0);
    //if ((timer + 60) <= now_timer) {
        //if (!(hit.mem_num == 0 && hit.db_num == 0)) {
            //msglog(gconfig::get_msg_file_path(), 0x0D000411, now_timer, &hit, storage_hits_size);
        //}
        //timer = now_timer;
        //hit.mem_num = 0;
        //hit.db_num = 0;
    //}
#ifdef TRACE_DEBUG
    DEBUG_SHOW("[FETCH_STAT] expect %d feeds, fetch from memcached %d, total fetched %d", units, mem_rows, total_rows);
#endif

    response_pkg_t *send =  (response_pkg_t *)send_buf;
    send->len = buf_idx;
    send->ret = RES_OP_SUCCESS;
    send->units = total_rows;

    *res_pkg_len = send->len;
    return 0;
}

int feeds_store::get2_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, get2_pkg_t *pkg)
{
    if (send_buf == NULL || res_pkg_len == NULL || pkg == NULL) {
        ERROR_LOG("NULL value for get2 feeds pkgs");
        return -1;
    }

    int  total_rows = 0;
    uint32_t total_len = res_hsz;
    uint32_t buf_idx = res_hsz;


    for (int i = 0 ; i < units ; i ++ ) {
        get2_pkg_t *cur_pkg = pkg + i;
        i_mysql_iface *p_db = gconfig::get_db_conn(cur_pkg->mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::get2_pkgs] fail to get database connection for mimi: %u", cur_pkg->mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }

        //del in memcached
        uint32_t g_len = 0;
        int g_cnt = feeds_database::gets2_pkg(p_db, send_buf + buf_idx, buflen - buf_idx , &g_len, cur_pkg);
        total_rows += g_cnt;
        total_len += g_len;
        if (g_cnt < 0 || total_len > buflen) {
            ERROR_LOG("fail to get feedids for get2 package request");
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }
        else if (g_cnt == 0) {
            WARN_LOG("get << 0 >> feedids for get2 package request");
        }
        else if (g_cnt > 1) {
            WARN_LOG("get %d feeds for one get2_pkg %u %u %u %lu", g_cnt, cur_pkg->mimi, cur_pkg->cmd_id, cur_pkg->app_id, cur_pkg->magic);
        }

        memcached_st *memc = gconfig::get_mem_handle();
        if (memc == NULL) {
            ERROR_LOG("[store::get2_pkgs] fail to get memcached server connection");
        } else {
            for (int i = 0 ; i < g_cnt ; i++) {
                feed_pkg_t *feed = (feed_pkg_t *)(send_buf + buf_idx);
                int memc_ret = feeds_memcached::set(memc, &feed->feedid, feed->data, feed->len - feed_hsz);
                if (memc_ret != 0) {
                    WARN_LOG("[feeds_store::get_pkgs] fail to set feed ( %s ) to memcached, log and pass",
                            get_feedidstr(&feed->feedid));
                }
                buf_idx += feed->len;
            }
        }
    } //end of for
    response_pkg_t *send =  (response_pkg_t *)send_buf;
    send->len = total_len;
    send->ret = RES_OP_SUCCESS;
    send->units = total_rows;

    *res_pkg_len = send->len;
    return 0;
}

int feeds_store::get_key_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, get_key_pkg_t *pkg)
{
    if (send_buf == 0 || res_pkg_len == 0 || pkg == 0 || units < 0) {
        ERROR_LOG("[feeds_store::get_key_pkgs] null or invalid value");
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
        return -1;
    }

    fidlist.clear();
    memcpy(fidlist.list, pkg, units*sizeof(get_key_pkg_t));
    fidlist.cur_used = units;

    uint32_t buf_idx = res_hsz;
    int total_rows = 0;

    uint32_t mem_len = 0;
    int mem_rows = 0;
    memcached_st *memc = gconfig::get_mem_handle();
    if (memc == NULL) {
        ERROR_LOG("[store::get_key_pkgs] fail to get memcached server connection");
    } else {
        mem_rows = feeds_memcached::gets_key_pkg(memc, send_buf + buf_idx, buflen - buf_idx, &mem_len, &fidlist);
        if (mem_rows < 0) {
            ERROR_LOG("[store::get_key_pkgs] fail to get from memcached, << row < 0 >> pass");
        }
        else if (mem_rows == 0) {
            WARN_LOG("[store::get_key_pkgs] get 0 row from memcached");
        }
        else {
            total_rows += mem_rows;
            buf_idx += mem_len;
        }
    }

    if (mem_rows != units) {
        uint32_t db_len = 0;
        int db_rows = feeds_database::gets_key_pkg(send_buf + buf_idx, buflen - buf_idx, &db_len, &fidlist);
        if (db_rows < 0) {
            ERROR_LOG("[store::get_key_pkgs] fail to get from database");
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }
        else if (db_rows == 0) {
            WARN_LOG("[store::get_key_pkgs] get 0 row from database");
        }
        else {
            if (memc != NULL) {
                char *kfeed_start = send_buf + buf_idx;
                for (int i = 0 ; i < db_rows ; i++) {
                    feed_key_pkg_t *kfeed =  (feed_key_pkg_t *)kfeed_start;
                    size_t datalen = kfeed->len - sizeof(feed_key_pkg_t);
                    int memc_ret = feeds_memcached::set(memc, &kfeed->id, kfeed->data, datalen);
                    memc_ret = feeds_memcached::set_key(memc, &kfeed->id, &kfeed->key);
                    if (memc_ret != 0) {
                        WARN_LOG("[store::get_key_pkgs] fail to set feed key %s to memcached, log and pass",
                                get_feedidstr(&kfeed->id));
                    }
                    kfeed_start += kfeed->len;
                }
            } /*-- end of for --*/
            total_rows += db_rows;
            buf_idx += db_len;
        }
    }
    //feeds hit statistic msglog
    static storage_hits  hit = {0, 0};
    static time_t timer = time(0);
    hit.mem_num += mem_rows > 0 ? mem_rows : 0;
    hit.db_num += total_rows - mem_rows;
    storage_hits_log(0x0D000411, &timer, &hit);
#ifdef TRACE_DEBUG
    DEBUG_SHOW("[FETCH_KEY_STAT] expect %d feeds, from memcached %d, total fetched %d", units, mem_rows, total_rows);
#endif
    response_pkg_t *send =  (response_pkg_t *)send_buf;
    send->len = buf_idx;
    send->ret = RES_OP_SUCCESS;
    send->units = total_rows;

    *res_pkg_len = send->len;
    return 0;
}

int feeds_store::get_nindexs(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, indexn_pkg_t *pkg_start)
{
    int total_rows = 0;
    uint32_t idxs_len = res_hsz;
    indexn_pkg_t *pkg = NULL;
    for (int i=0 ; i<units ; i++) {
        pkg = &pkg_start[i];
        uint32_t mimi = pkg->mimi;
        i_mysql_iface *p_db = gconfig::get_db_conn(mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::get_nindexs] fail to get database connection for mimi: %u", mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }

        verify_time(&pkg->starttime);
        uint32_t once_len = 0;
        int ret = feeds_database::get_nindexs(p_db, send_buf + idxs_len, buflen - idxs_len, &once_len,
                mimi, pkg->flag, pkg->cmd_id, pkg->app_id, pkg->starttime, pkg->next_num, pkg->prev_num);
        if (ret == 0) { //0 row
            WARN_LOG("fetch 0 feed index for user:%u start:%u before: %u after: %u",
                    mimi, pkg->starttime, pkg->prev_num, pkg->next_num);
            continue;
        }
        else if (ret < 0) { //database error
            ERROR_LOG("database ERROR when fetch N index for user:%u start:%u before: %u after: %u",
                    mimi, pkg->starttime, pkg->prev_num, pkg->next_num);

            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }
        else { // ret feeds index
            total_rows += ret;
            idxs_len += once_len;
        }
    }
    response_pkg_t *send =  (response_pkg_t *)send_buf;
    send->len = idxs_len;
    send->ret = RES_OP_SUCCESS;
    send->units = total_rows;

    *res_pkg_len = send->len;
    return 0;
}

int feeds_store::get_spanindexs(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, indexspan_pkg_t *pkg_start)
{
    uint32_t idxs_len = res_hsz;
    int total_rows = 0;
    indexspan_pkg_t *pkg = NULL;
    for (int i=0 ; i<units ; i++) {
        pkg = &pkg_start[i];
        uint32_t mimi = pkg->mimi;
        i_mysql_iface *p_db = gconfig::get_db_conn(mimi);
        if (p_db == NULL) {
            ERROR_LOG("[store::get_spanindexs] fail to get database connection for mimi: %u", mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }

        verify_time(&pkg->endtime);
        if (pkg->starttime > pkg->endtime) {
            ERROR_LOG("[store::get_spanindexs] fail due to starttime %u > endtime %u mimi: %u",
                    pkg->starttime, pkg->endtime, mimi);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_REQUEST);
            return -1;
        }

        uint32_t once_len = 0;
        int ret = feeds_database::get_spanindexs(p_db, send_buf + idxs_len, buflen - idxs_len, &once_len,
                mimi, pkg->flag, pkg->cmd_id, pkg->app_id, pkg->starttime, pkg->endtime);
        if (ret == 0) { //0 row
            WARN_LOG("fetch 0 rows span feed index for user:%u start:%u end: %u",
                    mimi, pkg->starttime, pkg->endtime);
        }
        else if (ret < 0) { //database error
            ERROR_LOG("database ERROR when fetch SPAN index for user:%u start:%u end: %u",
                    mimi, pkg->starttime, pkg->endtime);
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }
        else { // ret feeds index
            total_rows += ret;
            idxs_len += once_len;
        }
    } //end of fetch span indexs
    response_pkg_t *send =  (response_pkg_t *)send_buf;
    send->len = idxs_len;
    send->ret = RES_OP_SUCCESS;
    send->units = total_rows;

    *res_pkg_len = send->len;
    return 0;
}

int feeds_store::insert_p(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, insert_p_pkg_t *pkg)
{
    if (send_buf == NULL || res_pkg_len == NULL || pkg == NULL) {
        ERROR_LOG("null value for pfeed insert");
        goto RES_ERROR;
    }
    if (units != 1) {
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return -1;
    }
{
    uint32_t dis_id = pkg->feed.fid.target_id;
    i_mysql_iface *db = gconfig::get_db_conn(dis_id);
    if (db == NULL) {
        ERROR_LOG("[store::insert_p] fail to get database connection for mimi: %u", dis_id);
        goto RES_ERROR;
    }

    if (pfeeds_database::check_existed(db, &pkg->feed.fid)) {
        DEBUG_WARN("pfeed id %s existed", get_pfeedid_str(&pkg->feed.fid));
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_FEEDID_DUP);
        return -1;
    }

    int ret = pfeeds_database::insert(db, &pkg->feed);
    if (ret != 0) {
        ERROR_LOG("[store::insert_p] fail to insert passive feed key: %s", get_pfeedid_str(&pkg->feed.fid));
        goto RES_ERROR;
    } else {
        goto RES_SUCCESS;
    }
}
RES_SUCCESS:
    *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
    return 0;
RES_ERROR:
    *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
    return -1;
}

int feeds_store::update_p(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, update_p_pkg_t *pkg)
{
    if (send_buf == NULL || res_pkg_len == NULL || pkg == NULL) {
        ERROR_LOG("null value for pfeed update");
        goto RES_ERROR;
    }
    if (units != 1) {
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return -1;
    }
{
    uint32_t dis_id = pkg->feed.fid.target_id;
    i_mysql_iface *db = gconfig::get_db_conn(dis_id);
    if (db == NULL) {
        ERROR_LOG("[store::update_p] fail to get database connection for mimi: %u", dis_id);
        goto RES_ERROR;
    }
    if (!pfeedid_equal(&pkg->fid, &pkg->feed.fid)) {
        ERROR_LOG("[store::update_p] fail to update passive feed key not match: old %s, new %s",
                get_pfeedid_str(&pkg->fid), get_pfeedid_str(&pkg->feed.fid));
        goto RES_ERROR;
    }

    //end of checking, update db
    int ret = pfeeds_database::update(db, &pkg->fid, &pkg->feed);
    if (ret != 0) {
        ERROR_LOG("[store::update_p] fail to update passive feed key: %s %s",
                get_pfeedid_str(&pkg->fid), get_pfeedid_str(&pkg->feed.fid));
        goto RES_ERROR;
    } else {
        //update mem
        memcached_st *memc = gconfig::get_mem_handle();
        if (memc == NULL) {
            ERROR_LOG("[store::update_p] fail to get memcached server connection");
        } else {
            ret = pfeeds_memcached::replace(memc, &pkg->fid, &pkg->feed);
        #ifdef TRACE_DEBUG
            if (ret != 0) {
                DEBUG_WARN("[store::update_p] fail to update(replace) feed to memcached server, log and pass");
            }
        #endif
        }

        goto RES_SUCCESS;
    }
}
RES_SUCCESS:
    *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
    return 0;
RES_ERROR:
    *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
    return -1;
}
int feeds_store::del_p(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, del_p_pkg_t *pkg)
{
    if (send_buf == NULL || res_pkg_len == NULL || pkg == NULL) {
        ERROR_LOG("null value for pfeed delete");
        goto RES_ERROR;
    }
    if (units != 1) {
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return -1;
    }
{
    uint32_t dis_id = pkg->fid.target_id;
    i_mysql_iface *db = gconfig::get_db_conn(dis_id);
    if (db == NULL) {
        ERROR_LOG("[store::update_p] fail to get database connection for mimi: %u", dis_id);
        goto RES_ERROR;
    }

    int ret = pfeeds_database::del(db, &pkg->fid);
    if (ret != 0) {
        ERROR_LOG("[store::del_p] fail to delete passive feed key: %s", get_pfeedid_str(&pkg->fid));
        goto RES_ERROR;
    } else {
        memcached_st *memc = gconfig::get_mem_handle();
        if (memc == NULL) {
            ERROR_LOG("[store::del_p_pkg] fail to get memcached server connection");
        } else {
            pfeeds_memcached::del(memc, &pkg->fid);
        }

        goto RES_SUCCESS;
    }
}
RES_SUCCESS:
    *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_SUCCESS);
    return 0;
RES_ERROR:
    *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
    return -1;
}

int feeds_store::get_p_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, get_p_pkg_t *pkg)
{
    if (send_buf == NULL || res_pkg_len == NULL || pkg == NULL || units < 0) {
        ERROR_LOG("null value for pfeed get npkgs");
        return -1;
    }
    pfidlist.clear();
    memcpy(&pfidlist[0], pkg, units*sizeof(get_p_pkg_t));
    pfidlist.cur_used = units;

    int total_len = res_hsz;
    int total_cnt = 0;

    memcached_st *memc = gconfig::get_mem_handle();
    int mem_body_len = 0;
    int mem_cnt = 0;
    if (memc == NULL) {
        ERROR_LOG("[store::get_p_pkgs] fail to get memcached server connection");
    } else {
        mem_cnt = pfeeds_memcached::get_pkgs(memc, send_buf + total_len, buflen - total_len, &mem_body_len, &pfidlist);
        if (mem_cnt < 0) {
            ERROR_LOG("[store::get_p_pkgs] fetch from memcached server error, << row < 0 >> pass");
            pfidlist.reset();
        }
        else {
            total_len += mem_body_len;
            total_cnt += mem_cnt;
        }
    }

    if (total_cnt != units) {
        int db_body_len = 0;
        int db_cnt = pfeeds_database::get_pkgs(send_buf + total_len, buflen - total_len, &db_body_len, &pfidlist);
        if (db_cnt < 0) {
            ERROR_LOG("[store::get_p_pkgs] fetch from mysql server error");
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }
        else {
            if (memc != 0) {
                //set passive feed to memcached server
                char *pfeed_idx = send_buf + res_hsz + mem_body_len;
                for (int j=0 ; j < db_cnt ; j++) {
                    pfeed_pkg_t *pfeed = (pfeed_pkg_t *)pfeed_idx;
                    size_t datalen = pfeed->len - sizeof(pfeed_pkg_t) + sizeof(uint32_t);
                    pfeeds_memcached::set(memc, &pfeed->fid, (char*)(&pfeed->active_time), datalen);
                    pfeed_idx += pfeed->len;
                }
            }
            total_len += db_body_len;
            total_cnt += db_cnt;
        }
    }
    //passive feeds hit statistic msglog
    static storage_hits  hit = {0, 0};
    static time_t timer = time(0);
    hit.mem_num += mem_cnt > 0 ? mem_cnt : 0;
    hit.db_num += total_cnt - mem_cnt;
    storage_hits_log(0x0D000412, &timer, &hit);
#ifdef TRACE_DEBUG
    DEBUG_SHOW("[FETCH_P_STAT] expect %d pfeeds, from memcached %d, total fetched %d", units, mem_cnt, total_cnt);
#endif

    response_pkg_t *res = (response_pkg_t *)send_buf;
    res->len = total_len;
    res->ret = RES_OP_SUCCESS;
    res->units = total_cnt;

    *res_pkg_len = res->len;

    return 0;
}

int feeds_store::get_p_key_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, get_p_key_pkg_t *pkg)
{
    if (send_buf == NULL || res_pkg_len == NULL || pkg == NULL || units < 0) {
        ERROR_LOG("null value for pfeed get key pkgs");
        return -1;
    }
    pfidlist.clear();
    memcpy(&pfidlist[0], pkg, units*sizeof(get_p_key_pkg_t));
    pfidlist.cur_used = units;

    int total_len = res_hsz;
    int total_cnt = 0;

    memcached_st *memc = gconfig::get_mem_handle();
    int mem_body_len = 0;
    int mem_cnt = 0;
    if (memc == NULL) {
        ERROR_LOG("[store::get_p_pkgs] fail to get memcached server connection, << row < 0 >> pass");
    } else {
        mem_cnt = pfeeds_memcached::get_key_pkgs(memc, send_buf + total_len, buflen - total_len, &mem_body_len, &pfidlist);
        if (mem_cnt < 0) {
            ERROR_LOG("[store::get_p_pkgs] fetch from memcached server error");
            pfidlist.reset();
        }
        else {
            total_len += mem_body_len;
            total_cnt += mem_cnt;
        }
    }

    if (total_cnt != units) {
        int db_body_len = 0;
        int db_cnt = pfeeds_database::get_key_pkgs(send_buf + total_len, buflen - total_len, &db_body_len, &pfidlist);
        if (db_cnt < 0) {
            ERROR_LOG("[store::get_p_pkgs] fetch from mysql server error");
            *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
            return -1;
        }
        else {
            if (memc != NULL) {
                //set passive feed and key to memcached server
                char *pfeed_idx = send_buf + res_hsz + mem_body_len;
                for (int j=0 ; j < db_cnt ; j++) {
                    pfeed_key_pkg_t *pkfeed = (pfeed_key_pkg_t *)pfeed_idx;
                    size_t datalen = pkfeed->len - sizeof(pfeed_key_pkg_t) + sizeof(uint32_t);
                    pfeeds_memcached::set(memc, &pkfeed->id, (char*)(&pkfeed->active_time), datalen);
                    pfeeds_memcached::set_key(memc, &pkfeed->id, &pkfeed->key);
                    pfeed_idx += pkfeed->len;
                }
            }
            total_len += db_body_len;
            total_cnt += db_cnt;
        }
    }
    //passive feeds hit statistic msglog
    static storage_hits  hit = {0, 0};
    static time_t timer = time(0);
    hit.mem_num += mem_cnt > 0 ? mem_cnt : 0;
    hit.db_num += total_cnt - mem_cnt;
    storage_hits_log(0x0D000412, &timer, &hit);
#ifdef TRACE_DEBUG
    DEBUG_SHOW("[FETCH_P_STAT] expect %d pfeeds, from memcached %d, total fetched %d", units, mem_cnt, total_cnt);
#endif

    response_pkg_t *res = (response_pkg_t *)send_buf;
    res->len = total_len;
    res->ret = RES_OP_SUCCESS;
    res->units = total_cnt;

    *res_pkg_len = res->len;

    return 0;
}

int feeds_store::get_p_nindexs(char *send_buf, const size_t buflen, int *res_pkg_len, const int units, indexn_p_pkg_t *pkg)
{
    if (send_buf == NULL || res_pkg_len == NULL || pkg == NULL) {
        ERROR_LOG("null value for pfeed get nindexs");
        return -1;
    }

    int body_len = 0;
    int ret = pfeeds_database::get_indexs(send_buf + res_hsz, buflen - res_hsz, &body_len , units, pkg);
    if (ret < 0) {
        ERROR_LOG("[pfeed get indexs] get passive feed index error");
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
        return -1;
    } else {
        response_pkg_t *res = (response_pkg_t *)send_buf;
        res->len = res_hsz + body_len;
        res->ret = RES_OP_SUCCESS;
        res->units = ret;
        *res_pkg_len = res->len;
        return 0;
    }

}

int feeds_store::get_p_feedid_by_cmdid_pkgs(char *send_buf, const size_t buflen, int *res_pkg_len,
        const int units, get_p_feedid_by_cmdid_pkg_t *pkg)
{
    if (send_buf == NULL || res_pkg_len == NULL || pkg == NULL || units < 0) {
        ERROR_LOG("null value for pfeed get pfeedid by cmdid pkgs");
        return -1;
    }
    
    if (units != 1) {
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERR_UNITS);
        return -1;
    }
    
    int body_len = 0;
    int ret = pfeeds_database::get_feedid_by_cmdid(send_buf + res_hsz, buflen - res_hsz, body_len, units, pkg);
    if (ret < 0) {
        ERROR_LOG("[store::del_p] fail to get passive feed id by mimi:[%u] target_id:[%u] cmdid:[%u]", pkg->mimi, pkg->target_id, pkg->cmd_id);
        *res_pkg_len = build_simple_msg(send_buf, buflen, RES_OP_ERROR);
        return -1;
    }

    response_pkg_t *res = (response_pkg_t *)send_buf;
    res->len = res_hsz + body_len;
    res->ret = RES_OP_SUCCESS;
    res->units = ret;

    *res_pkg_len = res->len;
    return 0;
}
