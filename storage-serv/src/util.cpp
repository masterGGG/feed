/**
 * =====================================================================================
 *       @file  util.cpp
 *      @brief
 *
 *     Created  06/01/2011 04:23:33 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#include  "util.h"
#include  "log.h"

bool feedid_list::init()
{
    cur_used = 0;
    for (int i=0; i< MAX_FEEDID_NUM; i++) {
        fkeys[i] = (char*)malloc(MEM_FEEDKEY_MAX);
        if (fkeys[i] == 0) {
            return false;
        }

        fkeyslen[i] = 0;
        flag[i] = 0;
    }
    list = (feedid_t*)malloc(sizeof(feedid_t)*MAX_FEEDID_NUM);
    //keys = (feedkey_t*)malloc(sizeof(feedkey_t)*MAX_FEEDID_NUM);
    if (list == 0) {
        return false;
    }

    return true;
}
void feedid_list::uninit()
{
    for (int i=0; i< MAX_FEEDID_NUM; i++) {
        free(fkeys[i]);
    }

    free(list);
}

void feedid_list::clear()
{
    cur_used = 0;
    memset(flag, 0, MAX_FEEDID_NUM*sizeof(int));
}

//void feedid_list::clear_keys()
//{
    //memset(keys, 0, MAX_FEEDID_NUM*sizeof(feedkey_t));
//}

int feedid_list::append(feedid_t *id)
{
    if (cur_used == MAX_FEEDID_NUM || id == NULL)
        return -1;
    memcpy(list + cur_used, id, sizeof(feedid_t));
    cur_used++;
    return 0;
}

feedid_t &feedid_list::operator[](int idx)
{
    return list[idx];
}


/*-----------------------------------------passive feed id list-------------------------------------------*/
bool pfeedid_list::init()
{
    for (int i=0 ; i<MAX_FEEDID_NUM ; ++i) {
        memkeys[i] = (char*) malloc(MEM_FEEDKEY_MAX);
        if (memkeys[i] == 0) {
            return false;
        }

        memkeyslen[i] = 0;
        flag[i] = 0;
    }

    list = (pfeedid_t *)malloc(sizeof(pfeedid_t) * MAX_FEEDID_NUM) ;
    if (list == 0 ) {
        return false;
    }
    cur_used = 0;

    return true;
}

void pfeedid_list::uninit()
{
    for (int i=0 ; i<MAX_FEEDID_NUM ; ++i) {
        free(memkeys[i]);
    }

    free(list);
}

void pfeedid_list::clear()
{
    cur_used = 0;
    memset(flag, 0, sizeof(flag));
}

void pfeedid_list::reset()
{
    for (int i=0 ; i < cur_used ; i++) {
        flag[i] = 0;
    }
}

pfeedid_t &pfeedid_list::operator[](int idx)
{
    //if(idx < 0 || idx > MAX_FEEDID_NUM) {
        //return NULL;
    //}
    return list[idx];
}


/*********************************** utility function **************************************/


char *get_feedkey_str(feedkey_t *key)
{
    static const int BUF_MAX = 1024;
    static char buf[BUF_MAX] = {0};
    snprintf(buf, BUF_MAX - 1, "FKEY: %u %u %u %u %u",
            key->dbid, key->ftid, key->pftid, key->flag, key->id);
    return buf;
}

char *get_feedidstr(const feedid_t *t)
{
    static char feedid_str[MEM_FEEDKEY_MAX];
    snprintf(feedid_str, MEM_FEEDKEY_MAX - 1, "FID %u %u %u %u %lu ",
            t->mimi, t->cmd_id, t->app_id, t->timestamp, t->magic);
    return feedid_str;
}


//return simply message length(>=0) or error code
int build_simple_msg(char *msgbuf, const int buflen, uint16_t ret_code)
{
    response_pkg_t *res_msg = (response_pkg_t*)msgbuf;
    res_msg->len = res_hsz;
    res_msg->ret = ret_code;
    res_msg->units = 0;

    return (int)res_hsz;
}

bool strtoul_wrapper(const char* str, int base, uint32_t *value)
{
    if (value == NULL) {
        ERROR_LOG("fail to parse %s, value is NULL", str);
        return false;
    }
    char *endptr;
    *value = strtoul(str, &endptr, base);
    if ((errno == ERANGE && (*value == LONG_MAX || *value == LONG_MIN))
            || (errno != 0 && value == 0)) {
        ERROR_LOG("parse error! value %u err %s", *value, strerror(errno));
        return false;
    }
    if (endptr == str) {
        ERROR_LOG("No digits were found");
        return false;
    }
    if (*endptr != '\0') {
        ERROR_LOG("Further characters after number: %s", endptr);
        return false;
    }
    return true;
}

bool pfeedid_equal(const pfeedid_t *fid1, const pfeedid_t *fid2)
{
    if (fid1 == fid2) {
        return true;
    }

    if (fid1->src_fid.mimi == fid2->src_fid.mimi && fid1->src_fid.cmd_id == fid2->src_fid.cmd_id && fid1->src_fid.app_id == fid2->src_fid.app_id
            && fid1->src_fid.timestamp == fid2->src_fid.timestamp && fid1->src_fid.magic == fid2->src_fid.magic
            && fid1->sender_id == fid2->sender_id && fid1->target_id == fid2->target_id && fid1->p_magic == fid2->p_magic) {
        return true;
    } else {
        return false;
    }
}

char *get_pfeedid_str(const pfeedid_t *fid)
{
    static char buf[MEM_FEEDKEY_MAX] = {0};
    snprintf(buf, MEM_FEEDKEY_MAX - 1, "PFID %u %u %u %u %lu %u %u %u ",
            fid->src_fid.mimi, fid->src_fid.cmd_id, fid->src_fid.app_id, fid->src_fid.timestamp, fid->src_fid.magic,
            fid->sender_id, fid->target_id, fid->p_magic);
    return buf;
}

inline void print_feedid(feedid_t *fid)
{
    DEBUG_LOG("feed ID: |%u|%u|%u|%u|%lu|", fid->mimi, fid->cmd_id, fid->app_id, fid->timestamp, fid->magic);
}
void dump_send_buf(char *buf)
{
    response_pkg_t *res = (response_pkg_t *)buf;
    feed_pkg_t *feed = (feed_pkg_t *)(res->feeds) ;
    DEBUG_LOG("res pkg len:%u ret:%u units:%u", res->len, res->ret, res->units);
    for (int i=0 ; i<res->units ; i++) {
        if (feed->len == feed_hsz) {
            print_feedid(&feed->feedid);
        } else {
            DEBUG_LOG("feed pkg len: %u", feed->len);
            print_feedid(&feed->feedid);
            *((char*)feed + feed->len - 1) = 0;
            DEBUG_LOG("feed DATA:%s", feed->data);
        }
        feed = (feed_pkg_t *)(((char*)feed) + feed->len);
    }
}

inline void print_p_feed(pfeed_pkg_t *pkg)
{
    DEBUG_LOG("feed pkg len:       %u", pkg->len);
    DEBUG_LOG("passive feed id :   %s", get_pfeedid_str(&pkg->fid));
    DEBUG_LOG("passive feed active time: %u", pkg->active_time);
    if (pkg->len > (uint32_t)P_FEED_HEAD_SZ) {
        DEBUG_LOG("feed DATA length: %d", pkg->len - P_FEED_HEAD_SZ);
        *((char*)pkg + pkg->len - 1) = 0;
        DEBUG_LOG("feed DATA:%s", pkg->data);
    }
}
void dump_p_send_buf(char *buf)
{
    response_pkg_t *res = (response_pkg_t *)buf;
    DEBUG_LOG("res pkg len:%u ret:%u units:%u", res->len, res->ret, res->units);
    char *buf_idx = (char*)res->pfeeds;;
    for (int i=0 ; i<res->units; i++) {
        pfeed_pkg_t *pfeed = (pfeed_pkg_t *)buf_idx;
        buf_idx += pfeed->len;
        print_p_feed(pfeed);
    }
}

void dump_send_keybuf(char *buf)
{
    response_pkg_t *res = (response_pkg_t*)buf;
    DEBUG_LOG("res pkg len:%u ret:%u units:%u", res->len, res->ret, res->units);

    char *body = (char*)res->kfeeds;
    for(int i=0; i<res->units; i++) {
        feed_key_pkg_t *kf = (feed_key_pkg_t *)body;
        DEBUG_LOG("Feed id: %s", get_feedidstr(&kf->id));
        DEBUG_LOG("Feed key: %s", get_feedkey_str(&kf->key));
        body[kf->len - 1] = 0;
        DEBUG_LOG("Feed data: %s", kf->data);
        body += kf->len;
    }
}

void dump_send_p_keybuf(char *buf)
{
    response_pkg_t *res = (response_pkg_t*)buf;
    DEBUG_LOG("res pkg len:%u ret:%u units:%u", res->len, res->ret, res->units);

    char *body = (char*)res->pkfeeds;
    for(int i=0; i<res->units; i++) {
        pfeed_key_pkg_t *kf = (pfeed_key_pkg_t *)body;
        DEBUG_LOG("Feed id: %s", get_pfeedid_str(&kf->id));
        DEBUG_LOG("Feed key: %s", get_feedkey_str(&kf->key));
        DEBUG_LOG("Feed active tiem: %u", kf->active_time);
        body[kf->len - 1] = 0;
        DEBUG_LOG("Feed data: %s", kf->data);
        body += kf->len;
    }
}
