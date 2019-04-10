/**
 * @file outbox_so.cpp
 * @brief  outbox server shared object
 * @author Hansel
 * @version 
 * @date 2011-06-09
 */

#include "outbox_so.h"
#include "i_outbox.h"
#include "i_poutbox.h"
#include "i_ini_file.h"
#include "log.h"
#include "define.h"
#include "outbox_proto.h"
#include "msglog.h"

#include <cstring>
#include <cstdio>

using namespace std;

#include <netinet/ip.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

static i_ini_file *g_p_ini_file = NULL;
static i_outbox *g_p_outbox = NULL;
static i_poutbox *g_p_poutbox = NULL;
static int g_buffer_len = 0;
static char *g_response_buf = NULL;
static char g_logfile[1024];
static unsigned int g_msgid = 0;
static time_t g_last_log_time = 0;
static const char *g_p_ini_filename = "./conf/outbox_server.conf";

///---------------------------------------------------------------------
int handle_init(int argc, char **argv, int proc_type)
{
	if (create_ini_file_instance(&g_p_ini_file) == -1) {
		ERROR_LOG("create_ini_file_instance()");
		return -1;
	}
	if (g_p_ini_file->init(g_p_ini_filename) == -1) {
		ERROR_LOG("g_p_ini_file->init(%s)", g_p_ini_filename);
		return -1;
	}

	if (PROC_WORK == proc_type) {
		if (create_outbox_instance(&g_p_outbox) == -1) {
			ERROR_LOG("create_outbox_instance().");
			return -1;
		}
		if (g_p_outbox->init(g_p_ini_file)) {
			ERROR_LOG("g_p_outbox->init(%p)", g_p_ini_file);
			return -1;
		}
		if (create_poutbox_instance(&g_p_poutbox) == -1) {
			ERROR_LOG("create_poutbox_instance().");
			return -1;
		}
		if (g_p_poutbox->init(g_p_ini_file)) {
			ERROR_LOG("g_p_poutbox->init(%p)", g_p_ini_file);
			return -1;
		}
		int osize = g_p_ini_file->read_int("feed", "outbox_size", 30);
		if (osize == -1) {
			ERROR_LOG("get outbox size");
			return -1;
		}
		int psize = g_p_ini_file->read_int("feed", "poutbox_size", 30);
		if (psize == -1) {
			ERROR_LOG("get poutbox size");
			return -1;
		}
		int size = psize >= osize ? psize : osize;
		g_buffer_len = MAX_UID_NUM * size * sizeof(apfeedid_t) + sizeof(response_outbox_t);
		g_response_buf = (char *)malloc(g_buffer_len);

		char id_buf[16] = {0};
		if (g_p_ini_file->read_string("stat", "msgid", id_buf, sizeof(id_buf), NULL) == -1) {
			ERROR_LOG("get stat msgid");
			return -1;
		}
		sscanf(id_buf, "%x", &g_msgid);

		if (g_p_ini_file->read_string("stat", "logfile", g_logfile, sizeof(g_logfile), NULL) == -1) {
			ERROR_LOG("get logfile path");
			return -1;
		}

		DEBUG_LOG("stat %s:%#x", g_logfile, g_msgid);
	}

	return 0;
}

///---------------------------------------------------------------------
int handle_dispatch(const char *p_buf, int buf_len, int proc_num, int *p_key)
{
	return 0;
}

///---------------------------------------------------------------------
int handle_input(const char *p_recv, int recv_len, char **pp_send, int *p_send_len, skinfo_t *p_skinfo)
{
	if (p_recv == NULL) {
		return -1;
	}

	request_outbox_t *p_request = (request_outbox_t *)p_recv;

	if (recv_len < (int)sizeof(p_request->len)) {
		return 0;
	}

	if ((int)p_request->len > recv_len) {
		return 0;
	}

	switch (p_request->op_code) {
		case FEED_ID_INSERT:
			if (p_request->len != sizeof(request_outbox_t) + sizeof(feedid_t)) {
				ERROR_LOG("pack length error for insert: %u", p_request->len);
				return -1;
			}
			break;
		case FEED_ID_DEL:
			if (p_request->len != sizeof(request_outbox_t) + sizeof(feedid_t)) {
				ERROR_LOG("pack length error for delete: %u", p_request->len);
				return -1;
			}
			break;
		case FEED_ID_UPDATE:
			if (p_request->len != sizeof(request_outbox_t) + sizeof(feedid_t) * 2) {
				ERROR_LOG("pack length error for update: %u", p_request->len);
				return -1;
			}
			break;
		case PFEED_ID_INSERT:
			if (p_request->len != sizeof(request_outbox_t) + sizeof(apfeedid_t)) {
				ERROR_LOG("pack length error for insert passive: %u", p_request->len);
				return -1;
			}
			break;
		case PFEED_ID_DEL:
			if (p_request->len != sizeof(request_outbox_t) + sizeof(apfeedid_t)) {
				ERROR_LOG("pack length error for delete passive: %u", p_request->len);
				return -1;
			}
			break;
		case PFEED_ID_UPDATE:
			if (p_request->len != sizeof(request_outbox_t) + sizeof(apfeedid_t) * 2) {
				ERROR_LOG("pack length error for update passive: %u", p_request->len);
				return -1;
			}
			break;
		case FEED_ID_GET:
		case PFEED_ID_GET:
			break;
		default:
			ERROR_LOG("op_code %#x is not supported", p_request->op_code);
			return -1;
	}

	return p_request->len;
}

///---------------------------------------------------------------------
int handle_input_complete(const char *p_recv, int recv_len, char **pp_send, int *p_send_len, skinfo_t *p_skinfo, int flag)
{

	return 0;
}

///---------------------------------------------------------------------
int handle_open(char **pp_send, int *p_send_len, skinfo_t *p_skinfo)
{
	if (p_skinfo == NULL) {
		return -1;
	}

	struct in_addr peer_in;
	peer_in.s_addr = (in_addr_t)p_skinfo->remote_ip;

	DEBUG_LOG("connected by client, %s:%d", inet_ntoa(peer_in), p_skinfo->remote_port);

	return 0;
}

///---------------------------------------------------------------------
int handle_close(const skinfo_t *p_skinfo)
{
	if (p_skinfo == NULL) {
		return -1;
	}

	struct in_addr peer_in;
	peer_in.s_addr = (in_addr_t)p_skinfo->remote_ip;

	DEBUG_LOG("disconnected with client, %s:%d", inet_ntoa(peer_in), p_skinfo->remote_port);

	return 0;
}

///---------------------------------------------------------------------
int handle_process(char *p_recv, int recv_len, char **pp_send, int *p_send_len, skinfo_t *p_skinfo)
{
	if (p_recv == NULL || pp_send == NULL || p_send_len == NULL || p_skinfo == NULL) {
		return -1;
	}

	request_outbox_t *p_request = (request_outbox_t *)p_recv;
	response_outbox_t *p_response = (response_outbox_t *)g_response_buf;
	memset(p_response, 0, g_buffer_len);

	switch (p_request->op_code) {
		case FEED_ID_INSERT:
			p_response->len = sizeof(response_outbox_t);
			p_response->result = g_p_outbox->insert(p_request->feedid);
			if (p_response->result == -1) {
				ERROR_LOG("outbox feedid insert failed");
			}
			break;
		case FEED_ID_UPDATE:
			p_response->len = sizeof(response_outbox_t);
			p_response->result = g_p_outbox->update(p_request->feedid);
			if (p_response->result == -1) {
				ERROR_LOG("outbox feedid update failed");
			}
			break;
		case FEED_ID_DEL:
			p_response->len = sizeof(response_outbox_t);
			p_response->result = g_p_outbox->del(p_request->feedid);
			if (p_response->result == -1) {
				ERROR_LOG("outbox feedid delete failed");
			}
			break;
		case FEED_ID_GET: {
			int data_len = 0;
			int key_num = ((int)p_request->len - sizeof(request_outbox_t)) / (int)sizeof(p_request->uid[0]);
			if (key_num > MAX_UID_NUM) {
				ERROR_LOG("too many user id");
				return -1;
			}
			p_response->result = g_p_outbox->get(p_request->uid, key_num, ((char *)p_response) + sizeof(response_outbox_t), &data_len);
			if (p_response->result == -1) {
				ERROR_LOG("get outboxex failed");
			}
			p_response->len = sizeof(response_outbox_t) + data_len;
			break;
		}
		case PFEED_ID_INSERT:
			p_response->len = sizeof(response_outbox_t);
			p_response->result = g_p_poutbox->insert(p_request->apfeedid);
			if (p_response->result == -1) {
				ERROR_LOG("poutbox feedid insert failed");
			}
			break;
		case PFEED_ID_UPDATE:
			p_response->len = sizeof(response_outbox_t);
			p_response->result = g_p_poutbox->update(p_request->apfeedid);
			if (p_response->result == -1) {
				ERROR_LOG("poutbox feedid update failed");
			}
			break;
		case PFEED_ID_DEL:
			p_response->len = sizeof(response_outbox_t);
			p_response->result = g_p_poutbox->del(p_request->apfeedid);
			if (p_response->result == -1) {
				ERROR_LOG("poutbox feedid delete failed");
			}
			break;
		case PFEED_ID_GET: {
			int data_len = 0;
			int key_num = ((int)p_request->len - sizeof(request_outbox_t)) / (int)sizeof(p_request->uid[0]);
			if (key_num > MAX_UID_NUM) {
				ERROR_LOG("too many user id");
				return -1;
			}
			p_response->result = g_p_poutbox->get(p_request->uid, key_num, ((char *)p_response) + sizeof(response_outbox_t), &data_len);
			if (p_response->result == -1) {
				ERROR_LOG("get poutboxex failed");
			}
			p_response->len = sizeof(response_outbox_t) + data_len;
			break;
		}
		default:
			ERROR_LOG("it should never come here");
			return -1;
	}
	*pp_send = (char *)p_response;
	*p_send_len = (int)p_response->len;

	return 0;
}

///---------------------------------------------------------------------
void handle_fini(int proc_type)
{
	if (PROC_MAIN == proc_type) {
		DEBUG_LOG("handle_fini in PROC_MAIN");
	}
	else if (PROC_CONN == proc_type) {
		DEBUG_LOG("handle_fini in PROC_CONN");
	}
	else if (PROC_WORK == proc_type) {
		DEBUG_LOG("handle_fini in PROC_WORK");
		g_p_outbox->uninit();
		g_p_outbox->release();
		g_p_poutbox->uninit();
		g_p_poutbox->release();
		free(g_response_buf);
	}
	else {
		DEBUG_LOG("unknown process.");
	}
	g_p_ini_file->uninit();
	g_p_ini_file->release();
	return;
}

///---------------------------------------------------------------------
int handle_schedule()
{
	if (g_p_outbox == NULL || g_logfile == NULL) {
		return -1;
	}
	
	if (time(NULL) - g_last_log_time < LOG_INTERVAL) {
		return 0;
	}

	int stat[4] = {0};

	if (g_p_outbox->status(&stat[0], &stat[1]) == -1) {
		ERROR_LOG("g_p_outbox->status()");
		return -1;
	}
	if (g_p_poutbox->status(&stat[2], &stat[3]) == -1) {
		ERROR_LOG("g_p_poutbox->status()");
		return -1;
	}

	g_last_log_time = time(NULL);
	if (msglog(g_logfile, g_msgid, g_last_log_time, stat, sizeof(stat)) == -1) {
		ERROR_LOG("msglog error path:%s msgid:%#x time:%u hit:%d miss:%d phit:%d pmiss:%d", 
		g_logfile, g_msgid, (unsigned int)time(NULL), stat[0], stat[1], stat[2], stat[3]);
	}
	
	return 0;
}
