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

/* 2019-05-09 新增根据用户米米号和协议号查询feedid的接口 */
#define     REQ_OP_PASS_GET_KEY_BY_CMDID    27

typedef struct get_passive_feedid_by_cmdid_pkg {
    uint16_t flag;              //denote filter way of (cmd_id, app_id), have same meaning in following package
                                /* one bit represent one feature, support bitwise or sematic
                                * 0x1 => use cmd_id , the last bit
                                * 0x2 => use app_id , the next to last bit
                                * 0x4 => response with feed data, the third bit from the end
                                * 0x8 => search limit 1
                                */
   uint32_t mimi;               //发起者的米米号
   uint16_t cmd_id;              //协议号
   uint32_t app_id;
   uint32_t target_id;           //接收被动feed的米米号
}__attribute__((packed)) get_p_feedid_by_cmdid_pkg_t; 

#endif  /*__STORAGE_PROTO_PASS_H__*/
