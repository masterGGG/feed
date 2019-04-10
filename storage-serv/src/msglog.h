/**
 * =====================================================================================
 *       @file  msglog.h
 *      @brief  
 *
 *  Detailed description starts here.
 *
 *   @internal
 *     Created  11/13/2009 04:26:22 PM 
 *    Revision  3.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2009, TaoMee.Inc, ShangHai.
 *
 *     @author  taomee (淘米), taomee@taomee.com
 * This source code was wrote for TaoMee,Inc. ShangHai CN.
 * =====================================================================================
 */
#ifndef  __MSGLOG_H__
#define  __MSGLOG_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    /**
     * @struct message_header 
     * @brief message_header 
     */
    struct message_header {
        uint16_t len;
        unsigned char  hlen;
        unsigned char  flag0;
        uint32_t  flag;
        uint32_t  saddr;
        uint32_t  seqno;
        uint32_t  type;
        uint32_t  timestamp;
    };

    typedef struct message_header message_header_t;

    int  msglog(const char *logfile, uint32_t type, uint32_t timestamp, const void *data, int len); 
#ifdef __cplusplus
}
#endif

#endif  /*__MSGLOG_H__*/
