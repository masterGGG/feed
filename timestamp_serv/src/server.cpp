#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <vector>

#include "i_uvalue.h"
#include "i_ini_file.h"
#include "server.h"
#include "benchapi.h"
#include "proto.h"
#include "timestamp_cache.h"
#include "cmd_handler.h"

#define MAX_BUF_LEN 4096

const static char g_process_name[] = "timestamp_serv";
const static char g_vesion[] = "v1.0.0";

i_ini_file* g_p_ini_file = NULL;
char g_send_buf[MAX_BUF_LEN] = {0};
std::vector<i_uvalue*> g_db_vec;
tm_cache_head_t* g_p_tm_cache = NULL;

static int userid_compare_func(const void* p_key_data1, uint32_t key_size1,
        const void* p_key_data2, uint32_t key_size2)
{
    uint32_t* data1 = (uint32_t*)p_key_data1;
    uint32_t* data2 = (uint32_t*)p_key_data2;

    return *data1 - *data2;
}

int handle_init(int argc, char** argv, int pid_type)
{
    if(pid_type == PROC_WORK)
    {
        if(argc != 4)
        {
            ERROR_LOG("usage: %s %s %s config_file", argv[0], argv[1], argv[2]);
            return -1;
        }

        INFO_LOG("%s: %s build time: %s %s", g_process_name, g_vesion, __DATE__, __TIME__);

        /*初始化读取配置信息模块*/
        if(create_ini_file_instance(&g_p_ini_file))
        {
            ERROR_LOG("fail to create i_ini_file instance");
            return -1;
        }

        if(g_p_ini_file->init(argv[3]) < 0)
        {
            ERROR_LOG("fail to init i_ini_file instance: %s", g_p_ini_file->get_last_errstr());
            return -1;
        }

        /*初始化数据库*/
        char db_path[PATH_MAX] = {0};
        if (g_p_ini_file->read_string("db", "db_path", db_path, sizeof(db_path), NULL) != 0)
        {
            ERROR_LOG("fail to read db_path from config file");
            return -1;
        }

        int db_num = g_p_ini_file->read_int("db", "db_num", 5);

        for (int i = 0; i < db_num; i++)
        {
            char tmp_path[PATH_MAX] = {0};
            sprintf(tmp_path, "%s/userid_%d.db", db_path, i);

            i_uvalue* p_uvalue = NULL;
            if (create_uvalue_instance(&p_uvalue))
            {
                ERROR_LOG("fail to create i_uvalue instance");
                return -1;
            }
            if (p_uvalue->init(tmp_path, i_uvalue::CREATE | i_uvalue::BTREE, 0644, userid_compare_func))
            {
                ERROR_LOG("fail to init i_uvalue instance: %s", p_uvalue->get_last_errstr());
                return -1;
            }

            g_db_vec.push_back(p_uvalue);
        }

        /*创建key_cache缓存子系统*/
        g_p_tm_cache = tm_cache_create(300000);
        if (!g_p_tm_cache)
        {
            ERROR_LOG("tm_cache_create() failed");
            return -1;
        }
    }
    return 0;
}

int handle_input(const char *recv_buf, int recv_len, const skinfo_t *sk)
{
    if (recv_len < (int)sizeof(uint32_t))
    {
        return 0;
    }

    uint32_t pkg_len = *((uint32_t *)recv_buf);

    if (pkg_len > MAX_BUF_LEN)
    {
        ERROR_LOG("pkg_len %u is too large", pkg_len);
        return -1;
    }

    return pkg_len;
}

int handle_process(const char *recv_buf, int recv_len,
                char **send_buf, int *send_len, const skinfo_t *sk)
{
    pkg_header_t* req_head = (pkg_header_t*)recv_buf;
    pkg_header_t* ack_head = (pkg_header_t*)g_send_buf;
    memcpy(ack_head, req_head, sizeof(*ack_head));

    if(recv_len != (int)(req_head->pkg_len))
    {
        DEBUG_LOG("recv_len[%d] is not equal to required pkg_length[%d]", recv_len, req_head->pkg_len);
        ack_head->status_code = SYS_PKG_ILLEGAL;
        goto err;
    }

    switch(req_head->cmd_id)
    {
        case TIMESTAMP_GET_CMD_ID:
            if (req_head->pkg_len != sizeof(pkg_header_t))
            {
                DEBUG_LOG("recv pkg len[%u] is not equal to defined len[%u]",
                          req_head->pkg_len,
                          sizeof(pkg_header_t));
                ack_head->status_code = SYS_PKG_ILLEGAL;
                goto err;
            }
            get_timestamp(recv_buf, recv_len, send_buf, send_len, sk);
            return 0;
        case TIMESTAMP_SET_CMD_ID:
            if (req_head->pkg_len != sizeof(pkg_header_t) + sizeof(timestamp_set_req_t))
            {
                DEBUG_LOG("recv pkg len[%u] is not equal to defined len[%u]",
                          req_head->pkg_len,
                          sizeof(pkg_header_t) + sizeof(timestamp_set_req_t));
                ack_head->status_code = SYS_PKG_ILLEGAL;
                goto err;
            }
            set_timestamp(recv_buf, recv_len, send_buf, send_len, sk);
            return 0;
        default:
            ack_head->status_code = SYS_CMDID_NOFIND;
            goto err;
    }

err:
    ack_head->pkg_len = sizeof(*ack_head);
    *send_buf = g_send_buf;
    *send_len = ack_head->pkg_len;
    return 0;
}

int handle_fini(int pid_type)
{
    if(pid_type == PROC_WORK)
    {
        if(g_p_ini_file)
        {
            g_p_ini_file->uninit();
            g_p_ini_file->release();
            g_p_ini_file = NULL;
        }

        for (size_t i = 0; i < g_db_vec.size(); i++)
        {
            g_db_vec[i]->uninit();
            g_db_vec[i]->release();
            g_db_vec[i] = NULL;
        }

        if (g_p_tm_cache)
        {
            tm_cache_destroy(g_p_tm_cache);
            g_p_tm_cache = NULL;
        }
    }
    return 0;
}

