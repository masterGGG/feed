/**
 * =====================================================================================
 *       @file  util.h
 *      @brief
 *
 *     Created  05/27/2011 03:59:08 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __STORAGE_SRV_UTIL_H__
#define  __STORAGE_SRV_UTIL_H__
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <time.h>
#include  <errno.h>
#include  <limits.h>

#include  "newbench.h"
#include  "msglog.h"

#include  "config.h"
#include  "storage_proto.h"

typedef struct bench_config {
///基本信息
    char *prog_name;
    char *current_dir;
    char **saved_argv;
///日志信息
    char log_dir[MAX_CONF_STR_LEN];
    int log_level;
    u_int log_size;
    u_int log_maxfiles;
    char log_prefix[MAX_CONF_STR_LEN];
///绑定信息
    char bind_file[MAX_CONF_STR_LEN];
    struct bind_config *bind_list;
    bool use_barrier;
///工作进程信息
    char so_file[MAX_CONF_STR_LEN];
    u_int shmq_length;
    u_int pkg_timeout;
    u_int worker_num;
    u_int max_pkg_len;
    u_int avg_pkg_len;
///运行模式
    char run_mode[MAX_CONF_STR_LEN];
} bench_config_t;

#define     TRACE_DEBUG
#ifdef TRACE_DEBUG
    #define     DEBUG_SHOW(fmt, args...)        DEBUG_LOG(fmt, ##args)
    #define     DEBUG_SQL(sql)                  DEBUG_LOG("[DEBUG] SQL string: %s", sql)
    #define     DEBUG_TRACE()                   DEBUG_LOG("call %s:%s(L%d)", __FILE__, __FUNCTION__, __LINE__)
    #define     DEBUG_WARN(fmt, args...)        WARN_LOG(fmt, ##args)
#else
    #define     DEBUG_SHOW(fmt, args...)
    #define     DEBUG_SQL(sql)
    #define     DEBUG_TRACE()
    #define     DEBUG_WARN(fmt, args...)
#endif

//define get2 response feed limit
#define     GET2_LIMIT          10
//define SQL buffer max size
#define     SQL_MAX_SZ          (1024*1024)

//fixed size feedid list
#define     MAX_FEEDID_NUM      100
#define     MEM_FEEDKEY_MAX     256

#define F_NONE      0
#define F_MEM       2
#define F_DB        4
typedef struct feedid_list{
    bool init();
    void uninit();
    void clear();
    /*void clear_keys();*/
    int append(feedid_t *id);
    feedid_t &operator[](int idx);

    feedid_t    *list;
    /*feedkey_t   *keys;*/
    int         cur_used;
    char        *fkeys[MAX_FEEDID_NUM];
    size_t      fkeyslen[MAX_FEEDID_NUM];
    int         flag[MAX_FEEDID_NUM];  /* flag = 0 F_NONE => not fetched
                                        * flag = 2 F_MEM => fetched from memcached
                                        * flag = 4 F_DB => fetched from database
                                        */
} feedid_list_t;

typedef struct pfeedid_list{
    bool init();
    void uninit();
    void clear();
    void reset();
    pfeedid_t &operator[](int idx);

    pfeedid_t   *list;
    int         cur_used;
    int         flag[MAX_FEEDID_NUM];  /* flag = 0 => F_NONE not fetched
                                        * flag = 2 => F_MEM  fetched from memcached
                                        * flag = 4 => F_DB   fetched from database
                                        */
    char        *memkeys[MAX_FEEDID_NUM];
    size_t      memkeyslen[MAX_FEEDID_NUM];
} pfeedid_list_t;

/*-----------------------------------------------------------------------------
 *  const variable about request and response package
 *-----------------------------------------------------------------------------*/
//record request and response header length
const unsigned int req_hsz = (unsigned int)sizeof(request_pkg_t);
const unsigned int res_hsz = (unsigned int)sizeof(response_pkg_t);
//record size of feed package header
const unsigned int feed_hsz = (unsigned int)sizeof(feed_pkg_t);
//record size of feedid
const unsigned int feedidsz = (unsigned int)sizeof(feedid_t);
//memcached server expiration time
const unsigned int EXPIRE_TIME = 7*24*60*60;

/*-----------------------------------------------------------------------------
 *  utility function
 *-----------------------------------------------------------------------------*/
inline void verify_time(uint32_t *timestamp)
{
    if( *timestamp == 0 ) {
        *timestamp = (uint32_t)time(NULL);
    }
}

bool strtoul_wrapper(const char* str, int base);

//build error response with ret_code(set uints to 0)
int build_simple_msg(char *msgbuf, const int buflen, uint16_t ret_code);

//return feedkey readable string
char *get_feedkey_str(feedkey_t *key);

//return feedid readable string
char *get_feedidstr(const feedid_t *fid);

//judge passive feedid equal or not
bool pfeedid_equal(const pfeedid_t *fid1, const pfeedid_t *fid2);
//get passive feedid string representation
char *get_pfeedid_str(const pfeedid_t *fid);

//dump response package
void dump_send_buf(char *buf);
void dump_p_send_buf(char *buf);
void dump_send_keybuf(char *buf);
void dump_send_p_keybuf(char *buf);

#endif  /*__STORAGE_SRV_UTIL_H__*/
