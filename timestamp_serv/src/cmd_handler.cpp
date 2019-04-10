/*
 * =====================================================================================
 *
 *       Filename:  cmd_handler.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2011年04月11日 12时00分06秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jerryshao (邵赛赛), jerryshao@taomee.com
 *        Company:  TaoMee.Inc, ShangHai
 *
 * =====================================================================================
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include <vector>

#include "cmd_handler.h"
#include "server.h"
#include "proto.h"
#include "i_uvalue.h"
#include "timestamp_cache.h"

#define MAX_BUF_LEN 4096

extern char g_send_buf[MAX_BUF_LEN];
extern std::vector<i_uvalue*> g_db_vec;
extern tm_cache_head_t* g_p_tm_cache;

int get_timestamp(const char* recv_buf, int recv_len,
                char **send_buf, int *send_len, const skinfo_t* sk)
{
    if(!recv_buf || recv_len <= 0)
    {
        return -1;
    }

    pkg_header_t* req_head = (pkg_header_t*)recv_buf;
    pkg_header_t* ack_head = (pkg_header_t*)g_send_buf;
    memcpy(ack_head, req_head, sizeof(*req_head));
    timestamp_get_ack_t* ack_body = (timestamp_get_ack_t*)(ack_head + 1);

    i_uvalue* p_uvalue = NULL;
    uint32_t value = 0;
    uint32_t value_length = sizeof(value);
    int rt = 0;

    ///查找缓冲区中是否有数据
    tm_cache_node_t* node = tm_cache_search(g_p_tm_cache, req_head->userid);
    if (node) {
        ack_body->timestamp = node->timestamp;
        INFO_LOG("get node from cache, timestamp[%u], userid[%u]", ack_body->timestamp, req_head->userid);
        goto out;
    }

    ///查找数据库中是否有记录
    p_uvalue = g_db_vec[req_head->userid % g_db_vec.size()];
    if (!p_uvalue)
    {
        ERROR_LOG("i_uvalue get point failed");
        ack_head->status_code = SYS_DB_ERR;
        goto end;
    }

    if ((rt = p_uvalue->get(&req_head->userid, sizeof(req_head->userid), &value, &value_length)) == -1)
    {
        ERROR_LOG("fail to get timestamp for user_id[%u]", req_head->userid);
        ack_head->status_code = SYS_DB_ERR;
        goto end;
    }
    else if (rt == 1)
    {
        DEBUG_LOG("db cannot found timestamp for user_id[%u]", req_head->userid);
        ack_head->status_code = SYS_USERID_NOFIND;
        goto end;
    }

    ///更新缓冲区
    tm_cache_insert(g_p_tm_cache, req_head->userid, value);

    ack_body->timestamp = value;

out:
    ack_head->pkg_len = sizeof(*ack_head) + sizeof(*ack_body);
    *send_buf = g_send_buf;
    *send_len = ack_head->pkg_len;
    return 0;

end:
    ack_head->pkg_len = sizeof(*ack_head);
    *send_buf = g_send_buf;
    *send_len = ack_head->pkg_len;
    return 0;
}

int set_timestamp(const char* recv_buf, int recv_len,
                char **send_buf, int *send_len, const skinfo_t* sk)
{
    if(!recv_buf || recv_len <= 0)
    {
        return -1;
    }

    pkg_header_t* req_head = (pkg_header_t*)recv_buf;
    pkg_header_t* ack_head = (pkg_header_t*)g_send_buf;
    memcpy(ack_head, req_head, sizeof(*req_head));
    timestamp_set_req_t* req_body = (timestamp_set_req_t*) (req_head + 1);

    i_uvalue* p_uvalue = NULL;
    int rt = 0;

    ///插入或更新数据库
    p_uvalue = g_db_vec[req_head->userid % g_db_vec.size()];
    if (!p_uvalue)
    {
        ERROR_LOG("i_uvalue get point failed");
        ack_head->status_code = SYS_DB_ERR;
        goto end;
    }

    if ((rt = p_uvalue->set(&req_head->userid, sizeof(req_head->userid), &req_body->timestamp, sizeof(req_body->timestamp), NULL)) == -1)
    {
        ERROR_LOG("fail to set timestamp[%u] for user_id[%u]", req_body->timestamp, req_head->userid);
        ack_head->status_code = SYS_DB_ERR;
        goto end;
    }

    ///插入或更新缓冲区
    tm_cache_insert(g_p_tm_cache, req_head->userid, req_body->timestamp);

end:
    ack_head->pkg_len = sizeof(*ack_head);
    *send_buf = g_send_buf;
    *send_len = ack_head->pkg_len;
    return 0;
}

