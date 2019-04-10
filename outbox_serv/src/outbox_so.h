/**
 * @file outbox_so.h
 * @brief  
 * @author Hansel
 * @version 
 * @date 2011-06-09
 */

#ifndef OUTBOX_SO_H_2011_06_09
#define OUTBOX_SO_H_2011_06_09

#include "newbench.h"

extern "C" int handle_init(int argc, char **argv, int proc_type);
extern "C" int handle_dispatch(const char *p_buf, int buf_len, int proc_num, int *p_key);
extern "C" int handle_input(const char *p_recv, int recv_len, char **pp_send, int *p_send_len, skinfo_t *p_skinfo);
extern "C" int handle_input_complete(const char *p_recv, int recv_len, char **pp_send, int *p_send_len, skinfo_t *p_skinfo, int flag);
extern "C" int handle_open(char **pp_send, int *p_send_len, skinfo_t *p_skinfo);
extern "C" int handle_close(const skinfo_t *p_sdkinfo);
extern "C" int handle_process(char *p_recv, int recv_len, char **pp_send, int *p_send_len, skinfo_t *p_sdkinfo);
extern "C" int handle_schedule();
extern "C" void handle_fini(int proc_type);

#endif//OUTBOX_SO_H_2011_06_09
