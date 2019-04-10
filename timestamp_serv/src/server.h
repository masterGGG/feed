#ifndef _SERVER_H
#define _SERVER_H

#include "benchapi.h"

extern "C" int handle_init(int argc, char **argv, int pid_type);

extern "C" int handle_input(const char *recv_buf, int recv_len, const skinfo_t *sk);

extern "C" int handle_process(const char *recv_buf, int recv_len,
                char **send_buf, int *send_len, const skinfo_t *sk);

extern "C" int handle_fini(int pid_type);

#endif
