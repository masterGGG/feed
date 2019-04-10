/**
 * =====================================================================================
 *       @file  feeds_memcached.cpp
 *      @brief  impl of feeds operation related memcached
 *
 *     Created  06/09/2011 02:54:54 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#include  "feeds_memcached.h"
#include  "feeds_database.h"
#include  "storage_proto.h"

//memcached server allowed max key length
char feeds_memcached::feedkey[MEM_FEEDKEY_MAX] = { 0 };
memcached_return_t feeds_memcached::ret_memc = MEMCACHED_SUCCESS;

inline int feeds_memcached::generate_feedkey(feedid_t *f, char * fkey)
{
    int fsz = snprintf(fkey, MEM_FEEDKEY_MAX, "%x_%x_%x_%x_%lx",  \
                                f->mimi, f->cmd_id, f->app_id, f->timestamp, f->magic);

    if (fsz >= MEM_FEEDKEY_MAX || fsz < 0) {
        DEBUG_WARN("[feeds_memcached::generate_feedkey] snprintf return %d (truncated or meet error)", fsz);
        fkey[MEM_FEEDKEY_MAX - 1] = 0;
        fsz = MEM_FEEDKEY_MAX - 1;
    }
    return fsz;
}

inline int generate_keykey(feedid_t *f, char * fkey)
{
    return snprintf(fkey, MEM_FEEDKEY_MAX - 1, "key_%x_%x_%x_%x_%lx", f->mimi, f->cmd_id, f->app_id, f->timestamp, f->magic);
}

int feeds_memcached::add(memcached_st *memc, feedid_t *feedid, const char* escape_data, const size_t datalen)
{
    int keysz = generate_feedkey(feedid, feedkey);
    time_t expire = gconfig::is_startuser(feedid->mimi) ? 0 : EXPIRE_TIME;

    ret_memc = memcached_add(memc, feedkey, keysz, escape_data, datalen, expire, feedid->mimi); //set flag = mimi
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to add feedkey: %s to memcached server, error: %s", feedkey,
                memcached_strerror(memc, ret_memc));
        return -1;
    } else {
        return 0;
    }
}

int feeds_memcached::set(memcached_st *memc, feedid_t * feedid, const char* escape_data, const size_t datalen)
{
    int keysz = generate_feedkey(feedid, feedkey);
    time_t expire = gconfig::is_startuser(feedid->mimi) ? 0 : EXPIRE_TIME;

    ret_memc = memcached_set(memc, feedkey, keysz, escape_data, datalen, expire, feedid->mimi);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to set feedkey:%s to memcached server, error: %s", feedkey,
                memcached_strerror(memc, ret_memc));
        DEBUG_WARN("keylen:%d value:%s, valuelen:%d,expire:%u,mimi:%d", keysz, escape_data,
                datalen, expire, feedid->mimi);
        return -1;
    } else {
        return 0;
    }
}

int feeds_memcached::replace(memcached_st *memc, feedid_t *feedid, const char* escape_data, const size_t datalen)
{
    int keysz = generate_feedkey(feedid, feedkey);

    ret_memc = memcached_replace(memc, feedkey, keysz, escape_data, datalen,
            EXPIRE_TIME, feedid->mimi);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to replace feedkey:%s to memcached server, error: %s", feedkey,
                memcached_strerror(memc, ret_memc));
        return -1;
    } else {
        return 0;
    }

}

int feeds_memcached::gets(memcached_st *memc, char* gets_buf, size_t buflen, uint32_t *datalen, feedid_list_t * fids)
{
    for (int i=0; i<fids->cur_used; i++) {
        fids->fkeyslen[i] = generate_feedkey(&(fids->list[i]), (fids->fkeys[i]));
    }
    ret_memc = memcached_mget(memc, (char**)(fids->fkeys), fids->fkeyslen, fids->cur_used);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to memcached_mget, error: %s", memcached_strerror(memc, ret_memc));
        return -1;
    }
    static char feed_key[MEM_FEEDKEY_MAX];
    size_t key_len = 0;
    char *feed_value = NULL;
    size_t value_len = 0;
    uint32_t flags = 0;

    uint32_t pkgs_totalsz = 0;
    int pkgs_num = 0;
    char *feed_pkg_start = gets_buf;

    while((feed_value = memcached_fetch(memc, feed_key, &key_len, &value_len, &flags, &ret_memc))) {
//        DEBUG_SHOW("get key %s key-len: %u value-len:%u flags %u content: %s ", 
//                 feed_key, key_len, value_len, flags, feed_value );

        for (int i=0; i < fids->cur_used; i++) {
            if (/*key_len == fids->fkeyslen[i] &&*/ strncmp(feed_key, fids->fkeys[i], key_len) == 0) {
                uint32_t feed_pkg_len = feed_hsz + value_len;
                pkgs_totalsz += feed_pkg_len;
                //check buffer space is enough?
                if (pkgs_totalsz > buflen) {
                    ERROR_LOG("fail to copy gets feed, due to buflen:%lu < totalsz:%u", buflen, pkgs_totalsz);
                    return -1;
                }
                feed_pkg_t *feed = (feed_pkg_t*)feed_pkg_start;
                feed->len = feed_pkg_len;
                memcpy(&feed->feedid, &fids->list[i], feedidsz);
                memcpy(feed->data, feed_value, value_len);
                feed_pkg_start += feed_pkg_len;

                //set fetched flag
                fids->flag[i] = F_MEM;
                pkgs_num ++;

                break;
            }

            if (i == fids->cur_used - 1) {
                ERROR_LOG("fuck>>>>found unkown feed key: %s", feed_key);
            }
        } //end of check and build feed package fetched
        free(feed_value);
    }
    if (ret_memc != MEMCACHED_END) {
        DEBUG_WARN("feed memcached_mget not to end, error: %s", memcached_strerror(memc, ret_memc));
    }

    *datalen = pkgs_totalsz;
    return pkgs_num;
}

int feeds_memcached::gets_key_pkg(memcached_st *memc, char* gets_buf, size_t buflen, uint32_t *datalen, feedid_list_t *fids)
{
    if (memc == 0 || datalen == 0 || fids == 0) {
        ERROR_LOG("null value for feed memcache gets key packages");
        return -1;
    }

    for (int i=0; i<fids->cur_used; i++) {
        fids->fkeyslen[i] = generate_feedkey(&(fids->list[i]), (fids->fkeys[i]));
    }
    ret_memc = memcached_mget(memc, (char**)(fids->fkeys), fids->fkeyslen, fids->cur_used);
    if (ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("fail to memcached_mget, error: %s", memcached_strerror(memc, ret_memc));
        return -1;
    }
    static char feed_key[MEM_FEEDKEY_MAX];
    size_t key_len = 0;
    char *feed_value = NULL;
    size_t value_len = 0;
    uint32_t flags = 0;

    int pkgs_num = 0;
    uint32_t pkgs_totalsz = 0;
    char *feed_pkg_start = gets_buf;

    while((feed_value = memcached_fetch(memc, feed_key, &key_len, &value_len, &flags, &ret_memc))) {
        for (int i=0; i < fids->cur_used; i++) {
            if (strncmp(feed_key, fids->fkeys[i], key_len) == 0) { //find one
                uint32_t feed_pkg_len = sizeof(feed_key_pkg_t) + value_len;
                pkgs_totalsz += feed_pkg_len;
                //check buffer space is enough?
                if (pkgs_totalsz > buflen) {
                    ERROR_LOG("response too long, due to buflen:%lu < totalsz:%u", buflen, pkgs_totalsz);
                    pkgs_totalsz -= feed_pkg_len;
                    *datalen = pkgs_totalsz;
                    return pkgs_num;
                }
                feed_key_pkg_t *kfeed = (feed_key_pkg_t*)feed_pkg_start;
                feedid_t *fid = &(fids->list[i]);

                memcpy(&kfeed->id, fid, feedidsz);
                memcpy(kfeed->data, feed_value, value_len);
                kfeed->len = feed_pkg_len;

                feed_pkg_start += kfeed->len;
                pkgs_num ++;

                //set fetched flag
                fids->flag[i] = F_MEM;

                break;
            }

            if (i == fids->cur_used - 1) {
                ERROR_LOG("fuck>>>>found unkown feed key: %s", feed_key);
            }
        } //end of check and build feed package fetched
        free(feed_value);
    }
    if (ret_memc != MEMCACHED_END) {
        DEBUG_WARN("memcached_mget not to end, error: %s", memcached_strerror(memc, ret_memc));
    }

    //get feed key from memcache
    for (uint32_t kf_idx = 0; kf_idx < pkgs_totalsz  ; ) {
        feed_key_pkg_t *kfeed = (feed_key_pkg_t*)(gets_buf + kf_idx);
        feeds_memcached::get_key(memc, &kfeed->id, &(kfeed->key));
        kf_idx += kfeed->len;
    } /*-- end of for --*/
    
    *datalen = pkgs_totalsz;
    return pkgs_num;
}

int feeds_memcached::get_key(memcached_st *memc, feedid_t *fid, feedkey_t *fkey)
{
    if (memc == 0 || fid == 0) {
        return -1;
    }
    static char feed_key[MEM_FEEDKEY_MAX] = {0}; 
    size_t key_len= generate_keykey(fid, feed_key);
    size_t value_len = 0;
    uint32_t flags = 0;
    char *feed_key_value = memcached_get(memc, feed_key, key_len, &value_len, &flags, &ret_memc);
    
    if (feed_key_value == NULL || ret_memc != MEMCACHED_SUCCESS || value_len != sizeof(feedkey_t)) {
        int ret = feeds_database::get_key(fid, fkey);
        if (ret == 0) {
            feeds_memcached::set_key(memc, fid, fkey);
        }
        return ret;
    } else {
        memcpy(fkey, feed_key_value, sizeof(feedkey_t));
        free(feed_key_value);
        return 0;
    }
}

int feeds_memcached::set_key(memcached_st *memc, feedid_t *fid, feedkey_t *fkey)
{
    if (memc == 0 || fid == 0 || fkey == 0) {
        return -1;
    }
    static char feed_key[MEM_FEEDKEY_MAX] = {0}; 
    size_t key_len= generate_keykey(fid, feed_key);
    ret_memc = memcached_set(memc, feed_key, key_len, (char*)fkey, sizeof(feedkey_t), EXPIRE_TIME, fid->mimi);

    if (ret_memc != MEMCACHED_SUCCESS) {
        return -1;
    } else {
        return 0;
    }
}

int feeds_memcached::get(memcached_st *memc, char* get_buf, size_t buflen, uint32_t *datalen, feedid_t * fid)
{
    int fkeysz = generate_feedkey(fid, feedkey);
    size_t value_len = 0;
    uint32_t flags = -1;
    char *value = memcached_get(memc, feedkey, fkeysz, &value_len, &flags, &ret_memc);
    if (value == NULL || ret_memc != MEMCACHED_SUCCESS) {
        DEBUG_WARN("[memcached::get] fail to get feedkey:%s from memcached server, error: %s", feedkey,
                memcached_strerror(memc, ret_memc));
        return -1;
    } else {
        if (flags != fid->mimi) {
            ERROR_LOG("[memcached::get] feed flags %u != feedid mimi %u", flags, fid->mimi);
            return -1;
        }
        int pkglen = feed_hsz + value_len;

        *(uint32_t*)get_buf = pkglen;
        get_buf += sizeof(uint32_t);
        memcpy(get_buf, fid, feedidsz);
        get_buf += feedidsz;
        memcpy(get_buf, value, value_len);
        *datalen = pkglen;
        free(value);

        return 0;
    }
}

int feeds_memcached::dels(memcached_st *memc, feedid_list_t *feedids)
{
    int keysz = 0;
    for (int i=0; i< feedids->cur_used; i++) {
        keysz = generate_feedkey(&(feedids->list[i]), feedkey);
        ret_memc = memcached_delete(memc, feedkey, keysz, 0);
        if (ret_memc != MEMCACHED_SUCCESS) {
            WARN_LOG("fail to delete feedkey:%s in memcached server, error: %s", feedkey,
                    memcached_strerror(memc, ret_memc));
        }
    }
    return 0;
}

int feeds_memcached::del(memcached_st *memc, feedid_t *fid)
{
    int keysz = generate_feedkey(fid, feedkey);

    //expiretime = 0 , set flag = mimi
    ret_memc = memcached_delete(memc, feedkey, keysz, 0);
    if (ret_memc != MEMCACHED_SUCCESS) {
        WARN_LOG("fail to delete feedkey:%s in memcached server, error: %s", feedkey,
                memcached_strerror(memc, ret_memc));
    }
    return 0;
}
