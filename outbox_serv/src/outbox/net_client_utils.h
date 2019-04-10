/* vim: set tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file net_client_utils.h
 * @author richard <richard@taomee.com>
 * @date 2011-01-13
 */

#ifndef NET_CLIENT_UTILS_H_2010_01_13
#define NET_CLIENT_UTILS_H_2010_01_13

/**
 * @brief 打开到服务端的连接
 * @param p_ip 服务端的IP地址
 * @param port 服务端的端口号
 * @timeout 超时时间(单位：微秒 1秒= 1000,000微秒)
 * @return 成功返回连接套接字fd，失败返回-1
 */
int open_conn(const char *p_ip, int port, int timeout);

/**
 * @brief 向服务端发送请求消息，并返回服务端的回复消息
 * @param fd 和服务端连接的套接字fd
 * @param p_rqst_msg 请求消息
 * @param rqst_len 请求消息长度
 * @param p_resp_buf 存放服务端回复消息的缓存
 * @param buf_len 缓存的长度
 * @param timeout 超时时间(单位：微秒 1秒= 1000,000微秒)
 * @return 成功返回0，失败返回-1
 * @note 请求消息和回复消息的头两个字节必须表示整个消息的长度
 */
int send_rqst(int fd, const char *p_rqst_msg, int rqst_len, char *p_resp_buf, int resp_len, int timeout);

/**
 * @brief 关闭和服务端的连接
 * @param fd 和服务端连接的套接字fd 
 * @return 成功返回0，失败返回-1
 */
int close_conn(int fd);

#endif /* NET_CLIENT_UTILS_H_2010_01_13 */

