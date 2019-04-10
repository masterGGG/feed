/**
 * @file proto.h
 * @brief
 * @author jerryshao jerryshao@taomee.com
 * @version
 * @date 2011-08-30
 */

#ifndef _H_PROTO_H_
#define _H_PROTO_H_

#include <stdint.h>

#define TIMESTAMP_GET_CMD_ID 0x2001
#define TIMESTAMP_SET_CMD_ID 0x2002

#define SYS_DB_ERR 1002
#define SYS_PKG_ILLEGAL 1006
#define SYS_CMDID_NOFIND 1007
#define SYS_USERID_NOFIND 1014

typedef struct {
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t userid;
} __attribute__((packed)) pkg_header_t;

typedef struct {
    uint32_t timestamp;
} __attribute__((packed)) timestamp_set_req_t;

typedef struct {
    uint32_t timestamp;
} __attribute__((packed)) timestamp_get_ack_t;

#endif
