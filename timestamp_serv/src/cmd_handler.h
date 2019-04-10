#ifndef _H_CMD_HANDLER_20110407_H_
#define _H_CMD_HANDLER_20110407_H_
#include "benchapi.h"

extern int get_timestamp(const char *recv_buf, int recv_len,
                char **send_buf, int *send_len, const skinfo_t *sk);
extern int set_timestamp(const char *recv_buf, int recv_len,
                char **send_buf, int *send_len, const skinfo_t *sk);
#endif
