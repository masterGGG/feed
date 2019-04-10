/**
 * @file outbox.h
 * @brief  outbox class definition
 * @author Hansel
 * @version 
 * @date 2011-06-10
 */

#ifndef OUT_BOX_H_2011_06_10
#define OUT_BOX_H_2011_06_10

#include <libmemcached/memcached.h>

#include "i_outbox.h"
#include "memc_key.h"

class c_outbox : public i_outbox
{
public:
	c_outbox();
	virtual ~c_outbox();
	virtual int init(i_ini_file *p_ini);
	virtual int update(const feedid_t *p_feedid);
	virtual int insert(const feedid_t *p_feedid);
	virtual int del(const feedid_t *p_feedid);
	virtual int get(const unsigned int *p_uid_list, int key_num, char *p_outboxes, int *buf_data_len);
	virtual int status(int *hit, int *miss);
	virtual int uninit();
	virtual int release();

private:
	int get_memc_config();
	int get_stor_config();
	int get_key(mckey_t &p_key, unsigned int uid);
	int get_list_of_key(key_list_t &keys, int key_num, const unsigned int *p_uid_list);
	int local_outbox_exists(const feedid_t &p_feedid, feedid_t *p_outbox, int fid_num);
	int local_outbox_insert(const feedid_t &p_feedid, feedid_t *p_outbox, int *fid_num);
	int local_outbox_remove(const feedid_t &p_feedid, feedid_t *p_outbox, int *fid_num);
	int set_memc_outbox(const mckey_t &keys, char *p_outbox, int fid_num);
	int del_memc_outbox(const mckey_t &keys);
	int set_memc_outbox(unsigned int mimi, char *p_outbox, int fid_num);
	int get_memc_outbox(const mckey_t &key, char **pp_outbox);
	int get_memc_outbox(key_list_t &keys, char *p_outboxes);
	int get_memc_result(key_list_t &keys, char *p_outboxes);
	int get_stor_outbox(key_list_t &keys, char *p_outbox);

private:
	memcached_st *m_p_memc;

	i_ini_file *m_p_ini;

	int m_hit;
	int m_miss;
	int m_size;
	int m_fd;
	int m_feed_buffer_len;
	int m_send_buffer_len;
	int m_recv_buffer_len;
	int m_port;

	char *m_feed_buffer;
	char *m_send_buffer;
	char *m_recv_buffer;

	char m_ip[1024];
};

#endif//OUT_BOX_H_2011_06_10
