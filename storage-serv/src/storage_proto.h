/**
 * =====================================================================================
 *       @file  proto.h
 *      @brief  storage server package header definition
 *
 *     Created  06/02/2011 04:44:27 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __NEWSFEED_STORAGE_PROTO_H__
#define  __NEWSFEED_STORAGE_PROTO_H__
#include  <stdint.h>
#include  "feedid.h"
#include  "storage_proto_pass.h"

//feedid key in memcached: ("%x_%x_%x_%x_%lx", mimi, cmd_id, app_id, timestamp, magic)

/*-----------------------------------------------------------------------------
 *  followed by package struct used for request
 *-----------------------------------------------------------------------------*/
typedef struct insert_pkg {
    uint32_t len;          //len for one insert package
    feedid_t feedid;       //identification for one feed
    char data[0];       //data for this feed
}__attribute__((packed)) insert_pkg_t; /* ----------  end of struct insert_pkg  ---------- */

typedef struct update_pkg {
   uint32_t len;                //len for one insert package
   feedid_t feedid;            //identification for one feed
   char data[0];             //data for this feed
}__attribute__((packed)) update_pkg_t; /* ----------  end of struct update_pkg  ---------- */

typedef struct update2_pkg {    //update old feed to (new feedid, data), used in feed merge
   uint32_t len;                //len for one insert2 package
   feedid_t old_feedid;         //identification for old feed
   feedid_t new_feedid;         //identification for new feed
   char data[0];             //data for this feed
}__attribute__((packed)) update2_pkg_t; /* ----------  end of struct update_pkg  ---------- */

typedef struct delete_pkg {
   feedid_t feedid;            //identification for one feed
}__attribute__((packed)) delete_pkg_t;  /* ----------  end of struct delete_pkg  ---------- */

typedef struct delete2_pkg {  /* delete only by mimi, cmd_id, magic */
    uint32_t mimi;
    uint16_t cmd_id;
    uint32_t app_id;
    uint64_t magic;
}__attribute__((packed)) delete2_pkg_t;  /* ----------  end of struct delete_pkg  ---------- */

typedef struct set_pkg {        //set new feed to db, do not care old feed existed or not
    uint32_t len;               //len for one set package
    feedid_t old_feedid;        //identification for old feed
    feedid_t new_feedid;        //identification for new feed
    char data[0];             //data for this feed
}__attribute__((packed)) set_pkg_t; /* ----------  end of struct update_pkg  ---------- */

typedef struct get_pkg {
   feedid_t feedid;             //identification for one feed
}__attribute__((packed)) get_pkg_t;  /* ----------  end of struct gets_pkg  ---------- */

typedef struct get_n_indexs_pkg {
    uint32_t mimi;
    uint16_t flag;              //denote filter way of (cmd_id, app_id), have same meaning in following package
                                /* one bit represent one feature, support bitwise or sematic
                                 * 0x1 => use cmd_id , the last bit
                                 * 0x2 => use app_id , the next to last bit
                                 * 0x4 => response with feed data, the third bit from the end
                                 */
    uint16_t cmd_id;
    uint32_t app_id;
    uint32_t starttime;         //start time of lastest n indexs
                                //(0 represent now in storage server, and endtime interrept as releative time)
    uint16_t prev_num;          //fetch prev_num feedid, before starttime ( <= starttime)
    uint16_t next_num;          //fetch next_num feedid, after starttime ( >= starttime)
}__attribute__((packed)) indexn_pkg_t; /* ----------  end of struct get_n_indexs_pkg  ---------- */

typedef struct get_span_indexs_pkg {
    uint32_t mimi;
    uint16_t flag;              //TODO: see intepretation in indexn_pkg_t
    uint16_t cmd_id;
    uint32_t app_id;
    uint32_t starttime;         //start time
                                //(0 represent now in storage server, and endtime interrept as releative time)
    uint32_t endtime;           //end time of fetched feedid
}__attribute__((packed)) indexspan_pkg_t; /* ----------  end of struct get_ndays_indexs_pkg  ---------- */

typedef struct get2_pkg {  /* get feed only by mimi, cmd_id, magic */
    uint32_t mimi;
    uint16_t cmd_id;
    uint32_t app_id;
    uint64_t magic;
}__attribute__((packed)) get2_pkg_t;  /* ----------  end of struct get2_pkg  ---------- */

typedef struct get_key_data_pkg {
    feedid_t feedid;             //feed identification
}__attribute__((packed)) get_key_pkg_t;  /* ----------  end of struct gets_pkg  ---------- */

#define     REQ_OP_INSERT       1
#define     REQ_OP_UPDATE       2
#define     REQ_OP_UPDATE2      3
#define     REQ_OP_DELETE       4
#define     REQ_OP_DELETE2      5
#define     REQ_OP_SET          6

#define     REQ_OP_GETS                10
#define     REQ_OP_GET_N_INDEXS        11
#define     REQ_OP_GET_SPAN_INDEXS     12
#define     REQ_OP_GETS2               13
//gets packages data and keys(8bytes) according to feed key
#define     REQ_OP_GETS_KEYS           14

//request pkg units have max-limit
#define     MAX_ALLOW_UNITS             100
typedef struct request_pkg {
    uint32_t len;        //length for one request package
    uint16_t op;         //operation type(reference forward macro define): insert update delete gets ...
    uint16_t units;      //batch operation num for one request ( >=1 )
    union {
        insert_pkg_t            insert_items[0];
        update_pkg_t            update_items[0];
        update2_pkg_t           update2_items[0];
        delete_pkg_t            delete_items[0];
        delete2_pkg_t           delete2_items[0];
        set_pkg_t               set_items[0];

        get_pkg_t               get_items[0];
        indexn_pkg_t            indexns[0];
        indexspan_pkg_t         indexspans[0];
        get2_pkg_t              get2_items[0];
        get_key_pkg_t           get_key_items[0];

        insert_p_pkg_t          insert_p_items[0];
        update_p_pkg_t          update_p_items[0];
        del_p_pkg_t             del_p_items[0];
        get_p_pkg_t             get_p_items[0];
        indexn_p_pkg_t          indexns_p[0];
        get_p_key_pkg_t         get_p_key_items[0];
    };
}__attribute__((packed)) request_pkg_t;

/*-----------------------------------------------------------------------------
 *  followed by package struct used for response
 *-----------------------------------------------------------------------------*/
typedef struct feed_pkg {       //len == (sizeof(feedid_t) + sizeof(uint32_t)) => contain only feedid in one package
    uint32_t len;               //length for one feed
    feedid_t feedid;            //identification for one feed
    char data[0];               //start of feed data
}__attribute__((packed)) feed_pkg_t;

typedef struct feed_key_data_pkg {
    uint32_t    len;            //length for one entire feed, contain len, feedid, data
    feedkey_t   key;            //8 bytes feed key
    feedid_t    id;             //identification for one feed
    char        data[0];        //start of feed data
}__attribute__((packed)) feed_key_pkg_t;

typedef struct feed_operation_status_pkg {    //fixed package denote feed operated status
    feedid_t feedid;            //identification for one feed
    uint16_t status;            //status, such as units error, feedid error, request op error, etc.
}__attribute__((packed)) feed_status_pkg_t;

/* response for request package
 * for:
 * insert_pkg_t, update_pkg_t, update2_pkg_t, delete_pkg_t, delete2_pkg_t
 *       request           =>    respone
 *       (units == 1)          |len|ret|0|
 *       (units >= 1)          |len|ret|units|status(feed_status_pkg_t)
 * for:
 * get_pkg_t, indexn_pkg_t, indexspan_pkg_t
 *       request           =>    respone
 *   (units == n (<=100))      |len|ret|num|feeds(feed_pkg_t)
 */

/* response for passive feed operation
 * for:
 * insert_p_pkg_t   update_p_pkg_t    del_p_pkg_t
 *       request           =>    respone
 *       (units == 1)          |len|ret|0|
 *       units > 1 not support, now
 * for:
 * get_p_pkg_t     indexn_p_pkg_t
 *       request           =>    respone
 *   (units == n (<=100))      |len|ret|num|(feed_pkg_t|pfeed_pkg_t)
 *
 * for:
 * get_key_pkg_t get_p_key_pkg_t
 *       request           =>      respone
 * (units == n (<=100))    => |len|ret|num|(feed_key_pkg_t|pfeed_key_pkg_t)
 *
 */

#define     RES_OP_INVALID             -1
#define     RES_OP_SUCCESS              0
#define     RES_OP_ERROR                1
#define     RES_OP_FEEDID_DUP           2
#define     RES_OP_ERR_UNITS            3
#define     RES_OP_ERR_FEEDID           4
#define     RES_OP_ERR_REQUEST          5

static struct err_item {
    int eno;
    const char *str;
}storage_errstr[] = {
    {RES_OP_SUCCESS, "package process success"},
    {RES_OP_ERROR, "storage system error, maybe database failure"},
    {RES_OP_FEEDID_DUP, "feedid duplicates in database"},
    {RES_OP_ERR_UNITS, "batch request num too large or units equal to 0"},
    {RES_OP_ERR_FEEDID, "error feedid, maybe one or more fields is invalid (for update not match)"},
    {RES_OP_ERR_REQUEST, "can not handled request operation type"},
    {-1, "not invalid errno"}
};

inline const char* getstorage_errstr(int errno) {
    int i = 0;
    while (storage_errstr[i].eno != RES_OP_INVALID) {
        if (storage_errstr[i].eno == errno) {
            return storage_errstr[errno].str;
        }
        i++;
    }
    return storage_errstr[i].str;
}

typedef struct response_pkg {
    uint32_t len;        //length for response package
    uint16_t ret;        //operation result (refer to forward macro define)
    uint16_t units;      //batch num for one response
                         //( >= 0, eg. for get_pkg reqeust denote response feeds num )
    union {
        feed_status_pkg_t       status[0];//handle for feeds status
        feed_pkg_t              feeds[0]; //feeds
        pfeed_pkg_t             pfeeds[0];//passive feeds
        feed_key_pkg_t          kfeeds[0];//key feeds
        pfeed_key_pkg_t         pkfeeds[0];//passive key feeds
    };
}__attribute__((packed)) response_pkg_t; /* ----------  end of struct response_pkg  ---------- */

#endif  /*__NEWSFEED_STORAGE_PROTO_H__*/
