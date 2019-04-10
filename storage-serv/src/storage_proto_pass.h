/**
 * =====================================================================================
 *       @file  storage_proto_pass.h
 *      @brief  header file for passive newsfeed operations
 *
 *     Created  09/01/2011 05:09:36 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __STORAGE_PROTO_PASS_H__
#define  __STORAGE_PROTO_PASS_H__
#include  <stdint.h>

#include "feedid.h"

//message type for passive newsfeed
#define     REQ_OP_PASS_INSERT       21
#define     REQ_OP_PASS_UPDATE       22
#define     REQ_OP_PASS_DELETE       23
#define     REQ_OP_PASS_GETS_PKG     24
#define     REQ_OP_PASS_GETS_IDX     25
#define     REQ_OP_PASS_GETS_KEYS    26

typedef struct passive_feed_pkg {
    uint32_t  len;              //denote length of passive feed package
    pfeedid_t fid;
    uint32_t  active_time;
    char      data[0];          //contain data content
}__attribute__((packed))  pfeed_pkg_t;

typedef struct passive_key_feed_pkg {
    uint32_t    len;              //length of entire package
    feedkey_t   key;
    pfeedid_t   id;
    uint32_t    active_time;
    char        data[0];          //contain data content
}__attribute__((packed))  pfeed_key_pkg_t;

//length of passive feed header
const int P_FEED_HEAD_SZ = sizeof(pfeed_pkg_t);

//MAX length of feed package
const int P_FEED_MAX_SZ = 80 * 1024;  

typedef struct insert_passive_feedpkg_t {
    pfeed_pkg_t feed;           //entire feed
}__attribute__((packed)) insert_p_pkg_t;

typedef struct update_passive_feedpkg_t {
    pfeedid_t fid;              //passive feed id for update
    pfeed_pkg_t   feed;         //entire feed
}__attribute__((packed)) update_p_pkg_t;

typedef struct delete_passive_feedpkg_t {
    pfeedid_t fid;
}__attribute__((packed)) del_p_pkg_t;

typedef struct get_passive_pkg {
   pfeedid_t fid;               //passive feed id
}__attribute__((packed)) get_p_pkg_t; 

typedef struct get_passive_key_pkg {
   pfeedid_t fid;               //passive feed id 
}__attribute__((packed)) get_p_key_pkg_t; 

typedef struct get_n_passive_indexs_pkg {
    uint32_t mimi;              //fetch passive feed index for user
    uint32_t starttime;         //start time of lastest n indexs, before starttime
                                //0 represent now in storage server
    uint32_t prev_num;          //fetch prev_num feedid, before starttime ( <= starttime)
                                //denote fetch n feeds, default is 30 (n==0, mean fetch 30 feeds)
}__attribute__((packed)) indexn_p_pkg_t;

#endif  /*__STORAGE_PROTO_PASS_H__*/
