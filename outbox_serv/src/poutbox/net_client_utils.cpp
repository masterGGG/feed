/* vim: set tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file net_client_utils.cpp
 * @author richard <richard@taomee.com>
 * @date 2011-01-13
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "log.h"
#include "net_client_utils.h"

int open_conn(const char *p_ip, int port, int timeout)
{
	if (p_ip == NULL || port < 0 || port > 65535) {
		ERROR_LOG("ERROR: open_conn: p_ip == NULL || port < 0 || port > 65535");
		return -1;
	}
	
	int fd = -1;
	int rv = -1;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		ERROR_LOG("ERROR: socket: %s", strerror(errno));
		return -1;
	}

	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) {
		ERROR_LOG("ERROR: fcntl: %s", strerror(errno));
		close(fd);
		fd = -1;
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		ERROR_LOG("ERROR: fcntl: %s", strerror(errno));
		close(fd);
		fd = -1;
		return -1;
	}

	struct sockaddr_in serv_addr = {0};
	serv_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, p_ip, &serv_addr.sin_addr) != 1) {
		ERROR_LOG("ERROR: inet_pton %s: %s", p_ip, strerror(errno));
		close(fd);
		fd = -1;
		return -1;
	}
	serv_addr.sin_port = htons(port);
	if (serv_addr.sin_port < 0 || serv_addr.sin_port > 65535) {
		ERROR_LOG("ERROR: port: %d illegal", port);
		close(fd);
		fd = -1;
		return -1;
	}

	rv = connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (rv == 0) {
		return fd;
	} else if (rv == -1 && errno != EINPROGRESS) {
		ERROR_LOG("ERROR: connect: %s", strerror(errno));
		close(fd);
		fd = -1;
		return -1;
	}

	struct timeval tv = {0};
	tv.tv_sec = timeout / 1000000;
	tv.tv_usec = timeout % 1000000;

	fd_set write_set = {{0}}; 
	fd_set error_set = {{0}}; 

	FD_SET(fd, &write_set);
	FD_SET(fd, &error_set);

	rv = select(fd + 1, NULL, &write_set, &error_set, &tv);
	if (rv <= 0) {
		ERROR_LOG("ERROR: select: %s", strerror(errno));
		close(fd);
		fd = -1;
		return -1;
	}
	
	if (FD_ISSET(fd, &error_set)) {
		ERROR_LOG("ERROR: select error_set: %s", strerror(errno));
		close(fd);
		fd = -1;
		return -1;
	}
	if (FD_ISSET(fd, &write_set)) {
		int opt = -1;
		socklen_t opt_len = sizeof(opt);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &opt, &opt_len) == -1) {
			ERROR_LOG("ERROR: getsockopt: %s", strerror(errno));
			close(fd);
			fd = -1;
			return -1;
		}
		if (opt != 0) {
			ERROR_LOG("ERROR: connect: %s", strerror(opt));
			close(fd);
			fd = -1;
			return -1;
		}
	}

	return fd;
}

int send_rqst(int fd, const char *p_rqst_msg, int rqst_len, char *p_resp_buf, int resp_len, int timeout)
{
	int rv = -1;
	int resp_buf_used_len = 0;

	struct timeval tv = {0};
	tv.tv_sec = timeout / 1000000;
	tv.tv_usec = timeout % 1000000;
	
	for (;;) {
		fd_set read_set = {{0}}; 
		fd_set write_set = {{0}}; 
		fd_set error_set = {{0}}; 

		if (resp_buf_used_len >= resp_len) {
			ERROR_LOG("ERROR: resp_buf_used_len: %d >= resp_len: %d", resp_buf_used_len, resp_len);
			return-1;
		}
		FD_SET(fd, &read_set);
		if (rqst_len > 0) {
			FD_SET(fd, &write_set);
		}
		FD_SET(fd, &error_set);

		rv = select(fd + 1, &read_set, &write_set, &error_set, &tv);
		if (rv < 0) {
			ERROR_LOG("ERROR: select: %s", strerror(errno));
			return -1;
		} else if (rv == 0) {
			ERROR_LOG("ERROR: select timeout");
			return -1;
		} else {
			if (FD_ISSET(fd, &write_set)) {
				if (rqst_len > 0) {
					rv = write(fd, p_rqst_msg, rqst_len);
					if (rv == -1 && errno != EAGAIN) {
						ERROR_LOG("ERROR: write: %s", strerror(errno));
						return -1;
					}
					
					p_rqst_msg += rv;
					rqst_len -= rv;
				}
			}
			if (FD_ISSET(fd, &read_set)) {
				rv = read(fd, p_resp_buf + resp_buf_used_len, resp_len - resp_buf_used_len);
				if (rv == -1 && errno != EAGAIN ) {
					ERROR_LOG("ERROR: read: %s", strerror(errno));
					return -1;
				} else if (rv == -1 && errno == EAGAIN) {
					// do nothing
				} else if (rv == 0) {
					ERROR_LOG("ERROR: connection closed by other party");
					return -1;
				} else {
					resp_buf_used_len += rv;
					if (resp_buf_used_len >= (int)sizeof(uint32_t) && 
						resp_buf_used_len >= (int)*(uint32_t *)p_resp_buf) {
						break;                   // 收到完整的回复消息，退出for循环 
					}
				}
			}
			if (FD_ISSET(fd, &error_set)) {
				ERROR_LOG("ERROR: select error_set: %s", strerror(errno));
				return -1;
			}
		}
	}

	return 0;
}

int close_conn(int fd)
{
	if (fd != -1) {
		close(fd);
	}

	return 0;
}

