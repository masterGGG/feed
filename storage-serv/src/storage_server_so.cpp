/**
 * =====================================================================================
 *       @file  fetch_key.cpp
 *      @brief  demo so for newbench
 *
 *     Created  05/27/2011 02:01:43 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#include  <unistd.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <stdint.h>
#include  <signal.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <libmemcached/memcached.h>

#include  "newbench.h"
#include  "i_mysql_iface.h"

#include  "util.h"
#include  "config.h"
#include  "storage_proto.h"
#include  "feeds_database.h"
#include  "feeds_memcached.h"
#include  "feeds_store.h"

#include  "stat_analysis.h"

//use global configuration in newbench framework
extern bench_config_t g_bench_conf;
//used in one request process
static char op_buf[MAX_PACKAGE_LEN];
static int process_type = PROC_WORK;

void p_signal_handler(int sig)
{
    switch (sig)
    {
        case SIGHUP: //SIGHUP
            if (process_type != PROC_WORK) {
                return;
            }

            WARN_LOG("get reload signal, to reload configuration information");
            gconfig::uninit();
            if (gconfig::init(g_bench_conf.saved_argv[1])) {
                WARN_LOG("success to init gconfig with %s", g_bench_conf.saved_argv[1]);
                //init config success and dump cfg info
                gconfig::dump_cfg_info();
            } else {
                ERROR_LOG("fail to init gconfig with %s", g_bench_conf.saved_argv[1]);
            }

            feeds_store::uninit();
            if (!feeds_store::init()) {
                ERROR_LOG("fail to init feeds_store, may be out of memory !!! ");   
            } else {
                WARN_LOG("success to init feeds storage");
            }

#ifdef TRACE_DEBUG 
            stat_uninit();
            if (!stat_init(gconfig::get_stat_path(), "stat")) {
                ERROR_LOG("fail to init stat log !!! ");   
            }
#endif
            WARN_LOG("end of reload configuration information");
            break;

        default :
            DEBUG_LOG("get signal: no %d, str %s",sig, strsignal(sig));
    }  ///* end of switch */
}

extern "C" int handle_input(const char *p_recv,  int recv_len,
                            char **pp_send,      int *p_send_len, skinfo_t *p_skinfo)
{
    if (recv_len < 4) { //bytes < len field size
        DEBUG_SHOW("len < 4 continue to wait");
        return 0;
    }
    request_pkg_t *recv = (request_pkg_t*)p_recv;
    int pkg_len = (int)recv->len;
    //request have common header (len|op|units 8bytes)
    if (pkg_len < 8 || pkg_len > MAX_PACKAGE_LEN) {
        ERROR_LOG("receive package len: %d bytes larger than MAX ALLOW %d bytes", pkg_len, MAX_PACKAGE_LEN);
        return -1;
    }
    if (pkg_len <= recv_len) {
        DEBUG_SHOW("get one package, len:%d", pkg_len);
        return pkg_len;
    }
    //continue to wait for more bytes
    DEBUG_SHOW("continue to wait , pkglen:%d recvlen:%d", pkg_len, recv_len);
    return 0;
}
extern "C" int handle_dispatch(const char *p_recv, int recv_len, int proc, int *p_key)
{
    return 0;
}

extern "C" int handle_process(const char *p_recv, int recv_len,
                              char **pp_send,     int *p_send_len,   skinfo_t * p_skinfo)
{
    request_pkg_t *recv = (request_pkg_t *)p_recv;
    if (recv_len != (int)recv->len) {
        ERROR_LOG("handle_process recv_len: %d != request_pkg_t:len: %u", recv_len, recv->len);
        return -1;
    }
#define     REMOTE_IP           (inet_ntoa(*(struct in_addr*)&p_skinfo->remote_ip))
#define     REMOTE_PORT         (p_skinfo->remote_port)
    int sendlen = 0;
    int op_ret = 0;
    int r_units = recv->units;

    if (r_units <= 0 || r_units > MAX_ALLOW_UNITS) {
        ERROR_LOG("request units == %d, exceed allowed range [1, %d]", r_units, MAX_ALLOW_UNITS);
        sendlen = build_simple_msg(op_buf, sizeof(op_buf),  RES_OP_ERR_UNITS);
    } else {
#ifdef TRACE_DEBUG 
        static c_time stat_timestart;
        static c_time stat_timeend;
#endif
        switch (recv->op)
        {
        case REQ_OP_INSERT : // 1 => insert_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::insert(op_buf, sizeof(op_buf), &sendlen, r_units, recv->insert_items);
                STAT_ENDTIME(&recv->insert_items->feedid, "insert");
                break;
            }
        case REQ_OP_UPDATE : // 2 => update_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::update(op_buf, sizeof(op_buf), &sendlen, r_units, recv->update_items);
                STAT_ENDTIME(&recv->update_items->feedid, "update");
                break;
            }
        case REQ_OP_UPDATE2: // 3 => update2_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::update2(op_buf, sizeof(op_buf), &sendlen, r_units, recv->update2_items);
                STAT_ENDTIME(&recv->update2_items->old_feedid, "update2");
                break;
            }
        case REQ_OP_DELETE : // 4 => delete_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::del(op_buf, sizeof(op_buf), &sendlen, r_units, recv->delete_items);
                STAT_ENDTIME(&recv->delete_items->feedid, "delete");
                break;
            }
        case REQ_OP_DELETE2: // 5 => delete2_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::del2(op_buf, sizeof(op_buf), &sendlen, r_units, recv->delete2_items);
                STAT_END2TIME("delete2");
                break;
            }
        case REQ_OP_SET:      // 6 => set_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::set(op_buf, sizeof(op_buf), &sendlen, r_units, recv->set_items);
                STAT_ENDTIME(&recv->set_items->old_feedid, "set");
                break;
            }
        case REQ_OP_GETS:             // 10 => get_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::get_pkgs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->get_items);
                STAT_ENDTIME(&recv->get_items->feedid, "getpkgs");
                break;
            }
        case REQ_OP_GET_N_INDEXS:     // 11 => indexn_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::get_nindexs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->indexns);
                STAT_END2TIME("getnidx");
                break;
            }
        case REQ_OP_GET_SPAN_INDEXS:  // 12 => indexspan_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::get_spanindexs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->indexspans);
                STAT_END2TIME("getspanidx");
                break;
            }
        case REQ_OP_GETS2:  // 13 => indexspan_pkg_t
            {
                STAT_START();
                op_ret = feeds_store::get2_pkgs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->get2_items);
                STAT_END2TIME("get2");
                break;
            }
        case REQ_OP_GETS_KEYS:  // 14 => 
            {
                STAT_START();
                op_ret = feeds_store::get_key_pkgs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->get_key_items);
                STAT_END2TIME("get_keypkgs");
                break;
            }
        case REQ_OP_PASS_INSERT:  // 21 => 
            {
                STAT_START();
                op_ret = feeds_store::insert_p(op_buf, sizeof(op_buf), &sendlen, r_units, recv->insert_p_items);
                STAT_END2TIME("get_p_insert");
                break;
            }
        case REQ_OP_PASS_UPDATE:  // 22 => 
            {
                STAT_START();
                op_ret = feeds_store::update_p(op_buf, sizeof(op_buf), &sendlen, r_units, recv->update_p_items);
                STAT_END2TIME("get_p_update");
                break;
            }
        case REQ_OP_PASS_DELETE:  // 23 => 
            {
                STAT_START();
                op_ret = feeds_store::del_p(op_buf, sizeof(op_buf), &sendlen, r_units, recv->del_p_items);
                STAT_END2TIME("get_p_del");
                break;
            }
        case REQ_OP_PASS_GETS_PKG:  // 24 => 
            {
                STAT_START();
                op_ret = feeds_store::get_p_pkgs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->get_p_items);
                STAT_END2TIME("get_p_pkgs");
                break;
            }
        case REQ_OP_PASS_GETS_IDX:  // 25 => 
            {
                STAT_START();
                op_ret = feeds_store::get_p_nindexs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->indexns_p);
                STAT_END2TIME("get_p_idxs");
                break;
            }
        case REQ_OP_PASS_GETS_KEYS:  // 26 => 
            {
                STAT_START();
                op_ret = feeds_store::get_p_key_pkgs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->get_p_key_items);
                STAT_END2TIME("get_p_key_pkgs");
                break;
            }
        case REQ_OP_PASS_GET_KEY_BY_CMDID:  // 27 => 根据用户米米号加协议号查找对应的feedid
            {
                STAT_START();
                op_ret = feeds_store::get_p_feedid_by_cmdid_pkgs(op_buf, sizeof(op_buf), &sendlen, r_units, recv->get_pfeedid_items);
                STAT_END2TIME("get_pfeed_by_cmdid_pkgs");
                break;
            }
        case REQ_OP_PASS_GET_CNT_BY_CMDID:  // 28 =>2019-07-10 新增根据用户米米号和协议号查询被动feed的条数
            {
                STAT_START();
                op_ret = feeds_store::get_p_feedcnt(op_buf, sizeof(op_buf), &sendlen, r_units, recv->get_p_feedcnt_items);
                STAT_END2TIME("get_pfeed_count_by_cmdid");
                break;
            }
        default:           //error: can not handle op type
            {
                STAT_START();
                sendlen = build_simple_msg(op_buf, sizeof(op_buf), RES_OP_ERR_REQUEST);
                ERROR_LOG("unknown request type:%u from connection id: %d (%s:%u)",
                        recv->op, p_skinfo->connection_id, REMOTE_IP, REMOTE_PORT);
                STAT_END2TIME("no_op");
            }
        }  ///* end of switch */
    }

#ifdef TRACE_DEBUG 
    if (op_ret != 0 || sendlen < 0) {
        ERROR_LOG("fail to process request from {%s:%u}, request: %u|%u|%u ... ", 
                REMOTE_IP, REMOTE_PORT, recv_len, recv->op, recv->units);
    } else {
        DEBUG_LOG("success to process request from {%s:%u}, request: %u|%u|%u ... ", 
                REMOTE_IP, REMOTE_PORT, recv_len, recv->op, recv->units);
    } 
    response_pkg_t *res = (response_pkg_t *)op_buf;                               
    DEBUG_LOG("op_ret: %u, response: %u|%u|%u", op_ret, res->len, res->ret, res->units); 
    //dump_send_buf(op_buf);
#endif

    *pp_send = op_buf;
    *p_send_len = sendlen;
    return 0;
}

extern "C" int handle_init(int argc, char **argv, int ptype)
{
    process_type = ptype;

    if (ptype == PROC_MAIN) {
        DEBUG_LOG("[MAIN] global configuration max_pkg_len: %u", g_bench_conf.max_pkg_len);
        //if (signal(SIGHUP, p_signal_handler)) {  //? get error: no such file or directory
            //DEBUG_LOG("sucess to set signal handler for SIGHUP");
        //} else {
            //ERROR_LOG("fail to set signal handler for SIGHUP, err str: %s", strerror(errno));
        //}
    } else if (ptype == PROC_CONN) {
        if (signal(SIGHUP, p_signal_handler)) {
            DEBUG_LOG("sucess to set signal handler for SIGHUP");
        } else {
            ERROR_LOG("fail to set signal handler for SIGHUP, err str: %s", strerror(errno));
        }

        DEBUG_LOG("[CONN] global configuration max_pkg_len %u", g_bench_conf.max_pkg_len);
        DEBUG_LOG("[CONN] global configuration use_barrier: %s", g_bench_conf.use_barrier ? "on":"off");
    } if (ptype == PROC_WORK) {
        if (signal(SIGHUP, p_signal_handler)) {
            DEBUG_LOG("sucess to set signal handler for SIGHUP");
        } else {
            ERROR_LOG("fail to set signal handler for SIGHUP, err str: %s", strerror(errno));
        }

        //struct sigaction new_act;
        //struct sigaction old_act;
        //new_act.sa_handler = p_signal_handler;
        //if (sigaction(SIGHUP, &new_act, &old_act)) {
            //DEBUG_LOG("sucess to set signal handler for SIGHUP");
        //} else {
            //ERROR_LOG("fail to set signal handler for SIGHUP, err str: %s", strerror(errno));
        //}

        DEBUG_LOG("[WORK] global configuration max_pkg_len %u", g_bench_conf.max_pkg_len);
        if (gconfig::init(g_bench_conf.saved_argv[1])) {
            DEBUG_LOG("success to init gconfig with %s", g_bench_conf.saved_argv[1]);
            //init config success and dump cfg info
            gconfig::dump_cfg_info();
        } else {
            ERROR_LOG("fail to init gconfig with %s", g_bench_conf.saved_argv[1]);
            return -1;
        }

        if (!feeds_database::init()) {
            ERROR_LOG("fail to init feeds_database, may be out of memory !!! ");   
            return -1;
        }

        if (!feeds_store::init()) {
            ERROR_LOG("fail to init feeds_store, may be out of memory !!! ");   
            return -1;
        }
#ifdef TRACE_DEBUG 
        if (!stat_init(gconfig::get_stat_path(), "stat")) {
            ERROR_LOG("fail to init stat log !!! ");   
            return -1;
        }
#endif
    }

    return 0;
}

extern "C" int handle_fini(int ptype)
{
    if (PROC_MAIN == ptype) {
        DEBUG_LOG("handle_finit from process main[%d]", getpid());
    }
    else if (PROC_WORK == ptype) {
        DEBUG_LOG("handle_finit from process work[%d]", getpid());
        gconfig::uninit();
        feeds_store::uninit();
        feeds_database::uninit();
#ifdef TRACE_DEBUG 
        stat_uninit();
#endif
    }
    else if (PROC_CONN == ptype) {
        DEBUG_LOG("handle_finit from process conn[%d]", getpid());
    } 
    else {
        DEBUG_LOG("who[%d] am i? impossible !!!", getpid());
    }
    return 0;
}

extern "C" int handle_open(char **pp_send, int *p_send_len, skinfo_t *p_skinfo)
{
    struct in_addr aa = { s_addr:(in_addr_t)p_skinfo->remote_ip };
    DEBUG_LOG("[open] conn id:%d, from ip:%s, port:%u, sockfd:%d",
            p_skinfo->connection_id, inet_ntoa(aa), p_skinfo->remote_port, p_skinfo->sockfd);
    return 0;
}

extern "C" int handle_close(skinfo_t *p_skinfo)
{
    struct in_addr aa = { s_addr:(in_addr_t)p_skinfo->remote_ip };
    DEBUG_LOG("[close] conn id:%d, from ip:%s, port:%u, sockfd:%d\n",
            p_skinfo->connection_id, inet_ntoa(aa), p_skinfo->remote_port, p_skinfo->sockfd);
    return 0;
}

extern "C" int handle_schedule()
{
    return 0;
}

extern "C" int handle_input_complete(const char *p_recv, int recv_len, char **pp_send, int *p_send_len, skinfo_t *p_skinfo, int flag)
{
//    DEBUG_SHOW("handle input complete, flag = %d", flag);
    return 0;
}
