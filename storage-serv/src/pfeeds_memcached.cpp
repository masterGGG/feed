/**
 * =====================================================================================
 *       @file  pfeeds_memcached.cpp
 *      @brief  impl of passive feed memcache operations
 *
 *     Created  09/02/2011 01:30:49 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */

#include "config.h"
#include "storage_proto.h"
#include "pfeeds_memcached.h"
#include "pfeeds_database.h"

static memcached_return_t ret_memc = MEMCACHED_SUCCESS;
static const int TIMESTAMP_SIZE = sizeof(uint32_t);

static char memkey_buf[MEM_FEEDKEY_MAX] = {0};
static const int max_membuf_size = MEM_FEEDKEY_MAX - 1;

inline int gen_pfid_memkey(pfeedid_t *fid, char *buf, int buf_size)
{
    return  snprintf(buf, buf_size, "%x_%x_%x_%x_%lx_%x_%x_%x",
            fid->src_fid.mimi, fid->src_fid.cmd_id, fid->src_fid.app_id, fid->src_fid.timestamp, fid->src_fid.magic,
            fid->sender_id, fid->target_id, fid->p_magic);
}
inline int generate_keykey(pfeedid_t *fid, char *buf, int buf_size)
{
    return  snprintf(buf, buf_size, "key_%x_%x_%x_%x_%lx_%x_%x_%x",
            fid->src_fid.mimi, fid->src_fid.cmd_id, fid->src_fid.app_id, fid->src_fid.timestamp, fid->src_fid.magic,
            fid->sender_id, fid->target_id, fid->p_magic);
}

int pfeeds_memcached::add(memcached_st *memc, pfeed_pkg_t *pkg)
{
    if (memc == NULL || pkg == NULL) {
        ERROR_LOG("null value, for pfeed mem add");
    }

    int expire = EXPIRE_TIME;
    size_t datalen = pkg->len - P_FEED_HEAD_SZ + TIMESTAMP_SIZE;
    size_t keysz = gen_pfid_memkey(&pkg->fid, memkey_buf, max_membuf_size);
    //set flag = tagert_id, and add TIMESTAMP_SIZE bytes timestamp before pfeed data
    ret_memc = memcached_add(memc, memkey_buf, keysz, (char*)(&pkg->active_time), datalen, expire, pkg->fid.target_id);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to add pfeed: %s to memcached server, error: %s", memkey_buf,
                memcached_strerror(memc, ret_memc));
        return -1;
    } else {
        return 0;
    }
}

int pfeeds_memcached::set(memcached_st *memc, pfeedid_t *pfid, char *pkg_data, size_t datalen)
{
    if (memc == NULL || pfid == NULL || pkg_data == NULL) {
        ERROR_LOG("null value, for pfeed mem set");
    }

    int expire = EXPIRE_TIME;
    size_t keysz = gen_pfid_memkey(pfid, memkey_buf, max_membuf_size);
    //set flag = tagert_id, and add TIMESTAMP_SIZE bytes timestamp before pfeed data
    ret_memc = memcached_set(memc, memkey_buf, keysz, pkg_data, datalen, expire, pfid->target_id);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to set pfeed: %s to memcached server, error: %s", memkey_buf,
                memcached_strerror(memc, ret_memc));
        return -1;
    } else {
        return 0;
    }
}

int pfeeds_memcached::replace(memcached_st *memc, pfeedid_t *fid, pfeed_pkg_t *pkg)
{
    if (memc == NULL || pkg == NULL || fid == NULL) {
        ERROR_LOG("null value, for pfeed mem update");
    }
    size_t keysz = 0;
    if (!pfeedid_equal(fid, &pkg->fid)) {
        keysz = gen_pfid_memkey(fid, memkey_buf, max_membuf_size);
        ret_memc = memcached_delete(memc, memkey_buf, keysz, 0);
        if (ret_memc != MEMCACHED_SUCCESS) {
            DEBUG_WARN("fail to del old pfeed: %s,  memcached server error: %s",
                    memkey_buf, memcached_strerror(memc, ret_memc));
            return -1;
        }
    }

    //handle:  old pfid(fid) == new pfid(pkg->fid)
    int expire = EXPIRE_TIME;
    size_t datalen = pkg->len - P_FEED_HEAD_SZ + TIMESTAMP_SIZE;
    keysz = gen_pfid_memkey(&pkg->fid, memkey_buf, max_membuf_size);
    //place active_time in first TIMESTAMP_SIZE bytes of memcached data value
    //set flag = tagert_id, and add TIMESTAMP_SIZE bytes timestamp before pfeed data
    ret_memc = memcached_replace(memc, memkey_buf, keysz, (char*)(&pkg->active_time), datalen, expire, pkg->fid.target_id);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to replace pfeed: %s to memcached server, error: %s", memkey_buf,
                memcached_strerror(memc, ret_memc));
        return -1;
    } else {
        return 0;
    }
}

int pfeeds_memcached::del(memcached_st *memc, pfeedid_t *fid)
{
    if (memc == NULL || fid == NULL) {
        ERROR_LOG("null value, for pfeed mem del");
    }

    size_t keysz = gen_pfid_memkey(fid, memkey_buf, max_membuf_size);
    int expire = 0;
    ret_memc = memcached_delete(memc, memkey_buf, keysz, expire);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to del pfeed: %s,  memcached server error: %s",
                memkey_buf, memcached_strerror(memc, ret_memc));
        return -1;
    } else {
        return 0;
    }
}

bool pfeeds_memcached::check_existed(memcached_st *memc, pfeedid_t *fid)
{
    if (memc == NULL || fid == NULL) {
        ERROR_LOG("null value, for pfeed mem check existed");
    }
    size_t keysz = gen_pfid_memkey(fid, memkey_buf, max_membuf_size);
    char * buf = NULL;
    size_t value_length = 0;
    uint32_t flags = 0;

    buf = memcached_get(memc, memkey_buf, keysz , &value_length, &flags, &ret_memc);
    if (buf == NULL || ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("pfeed %s not in memcached server", memkey_buf);
        return false;
    } else {
        free(buf);
        return true;
    }
}

int pfeeds_memcached::get_pkgs(memcached_st *memc, char *resbuf, const int buflen, int *reslen, pfeedid_list_t *pfids)
{
    if (memc == NULL || resbuf == NULL|| reslen == NULL || pfids == NULL) {
        ERROR_LOG("null value, for pfeed mem get packages");
    }
    for (int i=0 ; i < pfids->cur_used ; i++) {
        pfids->flag[i] = 0;
        pfids->memkeyslen[i] = gen_pfid_memkey(&(*pfids)[i], pfids->memkeys[i], max_membuf_size);
    }

    ret_memc = memcached_mget(memc, (pfids->memkeys), pfids->memkeyslen, pfids->cur_used);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("[pfeed gets]fail to memcached_mget, error: %s", memcached_strerror(memc, ret_memc));
        return -1;
    }
    size_t key_len = 0;
    char *feed_value = NULL;
    size_t value_len = 0;
    uint32_t flags = 0;

    int total_cnt = 0;
    char *buf_idx = resbuf;
    while ((feed_value = memcached_fetch(memc, memkey_buf, &key_len, &value_len, &flags, &ret_memc))) {
        for (int i= 0; i < pfids->cur_used ; i++) { //loop check get which pfeed
            //first to check get one expected feed ?
            if(key_len == pfids->memkeyslen[i] && strncmp(memkey_buf, pfids->memkeys[i], key_len) == 0) { 
                pfeed_pkg_t *p_pkg = (pfeed_pkg_t *)buf_idx;
                pfeedid_t *pid_t = &(*pfids)[i];
                memcpy(&p_pkg->fid, pid_t, sizeof(pfeedid_t));
                //first TIMESTAMP_SIZE bytes is active timestamp
                //following passive feed data
                p_pkg->active_time = *(uint32_t*)feed_value;
                //value_len = sizeof(active_time + feed_data)
                size_t feed_datalen = value_len - TIMESTAMP_SIZE;
                memcpy(p_pkg->data, feed_value + TIMESTAMP_SIZE, feed_datalen);
                free(feed_value);
                p_pkg->len = P_FEED_HEAD_SZ + feed_datalen;

                pfids->flag[i] = F_MEM; //set flag get from memcached

                total_cnt++ ;

                buf_idx += p_pkg->len;
                if (buf_idx - resbuf >= buflen) {
                    ERROR_LOG("[mem gets pfeeds] response package too long %ld bytes", buf_idx - resbuf);
                    return -1;
                }
                break;
            }
            if (i == pfids->cur_used - 1) {
                ERROR_LOG("<<fuck>> get unkown pfeed from mem %s", memkey_buf);
            }
        } //end of for
    } //end of while

    *reslen = buf_idx - resbuf;
    return total_cnt;
}

int pfeeds_memcached::get_key_pkgs(memcached_st *memc, char *resbuf, const int buflen, int *reslen, pfeedid_list_t *pfids)
{
    if (memc == NULL || resbuf == NULL|| reslen == NULL || pfids == NULL) {
        ERROR_LOG("null value, for pfeed mem get packages");
    }
    for (int i=0 ; i < pfids->cur_used ; i++) {
        pfids->flag[i] = 0;
        pfids->memkeyslen[i] = gen_pfid_memkey(&(*pfids)[i], pfids->memkeys[i], max_membuf_size);
    }

    ret_memc = memcached_mget(memc, (pfids->memkeys), pfids->memkeyslen, pfids->cur_used);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("[pfeed gets]fail to memcached_mget, error: %s", memcached_strerror(memc, ret_memc));
        *reslen = 0;
        return -1;
    }
    size_t key_len = 0;
    char *feed_value = NULL;
    size_t value_len = 0;
    uint32_t flags = 0;

    int total_cnt = 0;
    char *buf_idx = resbuf;
    while ((feed_value = memcached_fetch(memc, memkey_buf, &key_len, &value_len, &flags, &ret_memc))) {
        for (int i= 0; i < pfids->cur_used ; i++) { 
            //first to check get one expected feed ?
            if(key_len == pfids->memkeyslen[i] && strncmp(memkey_buf, pfids->memkeys[i], key_len) == 0) { 
                int feed_datalen = value_len - TIMESTAMP_SIZE;
                int pkg_len = sizeof(pfeed_key_pkg_t) + feed_datalen;
                if (buf_idx - resbuf + pkg_len >= buflen) {
                    ERROR_LOG("[mem gets pfeeds] response package too long %ld bytes", buf_idx - resbuf + pkg_len);
                }
                pfeed_key_pkg_t *p_pkg = (pfeed_key_pkg_t *)buf_idx;

                pfeedid_t *pid_t = &(*pfids)[i];
                memcpy(&p_pkg->id, pid_t, sizeof(pfeedid_t));

                //first TIMESTAMP_SIZE bytes is active timestamp, following passive feed data
                p_pkg->active_time = *(uint32_t*)feed_value;
                //value_len = sizeof(active_time + feed_data)
                memcpy(p_pkg->data, feed_value + TIMESTAMP_SIZE, feed_datalen);

                free(feed_value);

                p_pkg->len = pkg_len;
                buf_idx += p_pkg->len;

                pfids->flag[i] = F_MEM; //set flag get from memcached

                total_cnt++ ;
                break;
            }
            if (i == pfids->cur_used - 1) {
                ERROR_LOG("<<fuck>> get unkown pfeed from mem %s", memkey_buf);
            }
        } //end of for
    } //end of while
    if (ret_memc != MEMCACHED_END) {
        DEBUG_WARN("pfeed memcached_mget not to end, error: %s", memcached_strerror(memc, ret_memc));
    }

    //get feed key from memcache
    int pkgs_totalsz = buf_idx - resbuf;
    for (int kf_idx = 0; kf_idx <  pkgs_totalsz; ) {
        pfeed_key_pkg_t *kfeed = (pfeed_key_pkg_t*)(resbuf + kf_idx);
        pfeeds_memcached::get_key(memc, &(kfeed->id), &(kfeed->key));
        kf_idx += kfeed->len;
    } /*-- end of for --*/

    *reslen = pkgs_totalsz;
    return total_cnt;
}

int pfeeds_memcached::get_key(memcached_st *memc, pfeedid_t *fid, feedkey_t *fkey)
{
    if (memc == 0 || fid == 0 || fkey == 0) {
        ERROR_LOG("null value for passive feed get key");
        return -1;
    }

    static char feed_key[MEM_FEEDKEY_MAX] = {0};
    size_t key_len= generate_keykey(fid, feed_key, MEM_FEEDKEY_MAX - 1);
    size_t value_len = 0;
    uint32_t flags = 0;
    char *feed_key_value = memcached_get(memc, feed_key, key_len, &value_len, &flags, &ret_memc);

    if (feed_key_value == NULL || ret_memc != MEMCACHED_SUCCESS || value_len != sizeof(feedkey_t)) {
        int ret = pfeeds_database::get_key(fid, fkey);
        if (ret == 0) {
            pfeeds_memcached::set_key(memc, fid, fkey);
        }
        return ret;
    } else {
        memcpy(fkey, feed_key_value, sizeof(feedkey_t));
        free(feed_key_value);
        return -1;
    }

}
int pfeeds_memcached::set_key(memcached_st *memc, pfeedid_t *fid, feedkey_t *fkey)
{
    if (memc == 0 || fid == 0 || fkey == 0) {
        return -1;
    }
    static char feed_key[MEM_FEEDKEY_MAX] = {0};
    size_t key_len= generate_keykey(fid, feed_key, MEM_FEEDKEY_MAX - 1);
    ret_memc = memcached_set(memc, feed_key, key_len, (char*)fkey, sizeof(feedkey_t), EXPIRE_TIME, fid->target_id);

    if (ret_memc != MEMCACHED_SUCCESS) {
        return -1;
    } else {
        return 0;
    }
}
