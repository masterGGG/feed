/**
 * =====================================================================================
 *       @file  feedid.h
 *      @brief  common struct feedid (mimi cmd_id timestamp magic), used in proto
 *
 *     Created  06/08/2011 05:54:29 PM 
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __NEWSFEEDID_H__
#define  __NEWSFEEDID_H__

/*-----------------------------------------------------------------------------
 *  followed by struct for feed id, consist of mimi, cmd_id, timestamp and magic
 *-----------------------------------------------------------------------------*/
typedef struct feedid_st {
    uint32_t mimi;
    uint16_t cmd_id;
    uint32_t app_id;
    uint32_t timestamp;
    uint64_t magic;        //for specified id or random id
}__attribute__((packed)) feedid_t; /* ----------  end of struct feed_id  ---------- */


/*-----------------------------------------------------------------------------
 *  id for passive feed
 *-----------------------------------------------------------------------------*/
typedef struct passive_feedid {
    feedid_t src_fid;           //source feed id related
    uint32_t sender_id;         //sender user
    uint32_t target_id;         //target user
    uint32_t p_magic;           //magic field for passive feed
}__attribute__((packed)) pfeedid_t; 

/*-----------------------------------------------------------------------------
 *  8 bytes feedkey, independent for feedid
 *-----------------------------------------------------------------------------*/
typedef struct feed_key_pkg {
    uint8_t dbid;              //0xFF is not valid
    uint8_t ftid;              //feed table id, 0xFF is not valid
    uint8_t pftid;             //passive feed table id, 0xFF is not valid
    uint8_t flag;              //keep for other usage
    uint32_t id;               //auto inc num in db/table
}__attribute__((packed)) feedkey_t; /* --  end of struct  -- */

#endif  /*__NEWSFEEDID_H__*/
