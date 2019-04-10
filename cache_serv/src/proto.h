/**
 * @file proto.h
 * @brief cache-server服务端协议
 * @author Hansel
 * @version 
 * @date 2011-03-21
 */

#include <stdint.h>

#define SYSTEM_MSG			0xFFFF
#define CONN_REQUEST		1
#define FEED_REQUEST		2

/**
 * @brief feed缓存请求包。
 */
typedef struct {
	uint16_t length;			/**< 包长 */
	uint16_t cmd_id;			/**< 消息协议号(校巴定义的编号) */
	uint32_t user_id;			/**< 米米号 */
	uint8_t version;			/**< 通讯协议版本号(目前只用1) */
	uint32_t timestamp;			/**< feed产生时间戳 */
	char body[0];				/**< feed内容 */
}__attribute__((__packed__)) so_request_msg_t;

/**
 * @brief  feed缓存回复包。
 */
typedef struct {
	uint16_t length;			/**< 包长 */
	uint16_t result;			/**< 返回操作结果(0成功0xFFFF失败) */
}__attribute__((__packed__)) so_response_msg_t;

/**
 * @brief  task连接请求包。
 */
typedef struct {
	uint16_t length;			/**< 包长 */
	uint16_t cmd_id;			/**< 消息协议号(此处为0xFFFF) */
	uint16_t sub_cmd;			/**< 子协议号(此处为1) */
}__attribute__((__packed__)) task_conn_request_msg_t;

/**
 * @brief  task连接回复包。
 */
typedef struct {
	uint16_t length;			/**< 包长 */
	uint16_t task_id;			/**< 分配给task进程的唯一标识(0xFFFF表示没有空闲工作进程) */
}__attribute__((__packed__)) task_conn_response_msg_t;

/**
 * @brief  task请求feed包。
 */
typedef struct {
	uint16_t length;			/**< 包长 */
	uint16_t cmd_id;			/**< 消息协议号(此处为0xFFFF) */
	uint16_t sub_cmd;			/**< 子协议号(此处为2) */
	uint16_t task_id;			/**< 分配给task进程的唯一标识 */
}__attribute__((__packed__)) task_feed_request_msg_t;

/**
 * @brief	task请求feed回复包。
 */
typedef struct {
	uint16_t length;			/**< 包长 */
	uint16_t cmd_id;			/**< 消息协议号(校巴定义的编号，0xFFFF表示缓存中无消息) */
	uint32_t user_id;			/**< 米米号 */
	uint8_t version;			/**< 通讯协议版本号(目前只用1) */
	uint32_t timestamp;			/**< feed产生时间戳 */
	char body[0];				/**< feed内容 */
}__attribute__((__packed__)) task_feed_response_msg_t;
