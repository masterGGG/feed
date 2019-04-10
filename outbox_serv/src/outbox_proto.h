/**
 * @file outbox_proto.h
 * @brief  outbox-server protocol
 * @author Hansel
 * @version 
 * @date 2011-06-02
 */

#ifndef OUTBOX_PROTO_H
#define OUTBOX_PROTO_H

#include <stdint.h>

#include "feedid.h"

/**
 * @brief  pfeedid, last active time mixture
 */
typedef struct {
	pfeedid_t pfeedid;
	uint32_t atime;							/**< last active time */
}__attribute__((__packed__)) apfeedid_t;

/**
 * @brief  request message
 */
typedef struct {
	uint32_t len;
	uint16_t op_code;						/**< operation code */
	union {
		uint32_t uid[0];					/**< view request: user id list */
		feedid_t feedid[0];					/**< process request: first for the new feed id, second for the old one, if any */
		apfeedid_t apfeedid[0];				/**< process request: first for the new feed id, second for the old one, if any */
	};
}__attribute__((__packed__)) request_outbox_t;

/**
 * @brief  response message
 */
typedef struct {
	uint32_t len;
	uint16_t result;						/**< 0 on success, -1 on failure */
	union {
		feedid_t feedid[0];					/**< used when response to view for the feed id list */
		apfeedid_t apfeedid[0];				/**< used when response to view for the feed id list */
	};
}__attribute__((__packed__)) response_outbox_t;

#define		FEED_ID_UPDATE		0xFFF1
#define		FEED_ID_INSERT		0xFFF2
#define		FEED_ID_DEL			0xFFF3
#define		FEED_ID_GET			0xFFF4

#define		PFEED_ID_UPDATE		0xFFF5
#define		PFEED_ID_INSERT		0xFFF6
#define		PFEED_ID_DEL		0xFFF7
#define		PFEED_ID_GET		0xFFF8

#endif//OUTBOX_PROTO_H
