/**
 * @file poutbox.h
 * @brief  poutbox definition
 * @author Hansel
 * @version 
 * @date 2011-09-05
 */

#ifndef POUTBOX_H_2011_09_05
#define POUTBOX_H_2011_09_05

#include <libmemcached/memcached.h>

#include "i_poutbox.h"
#include "memc_key.h"

class c_poutbox : public i_poutbox
{
public:
	c_poutbox();
	virtual ~c_poutbox();
	virtual int init(i_ini_file *p_ini);
	virtual int update(const apfeedid_t *p_apfeedid);
	virtual int insert(const apfeedid_t *p_apfeedid);
	virtual int del(const apfeedid_t *p_apfeedid);
	virtual int get(const unsigned int *p_uid_list, int key_num, char *p_poutboxes, int *buf_data_len);
	virtual int status(int *hit, int *miss);
	virtual int uninit();
	virtual int release();

private:
	int get_memc_config();
	int get_stor_config();
	int get_key(mckey_t &p_key, unsigned int uid);
	int get_list_of_key(key_list_t &keys, int key_num, const unsigned int *p_uid);
	int local_poutbox_exists(const apfeedid_t &p_apfeedid, apfeedid_t *p_poutbox, int fid_num);
	int local_poutbox_insert(const apfeedid_t &p_apfeedid, apfeedid_t *p_poutbox, int *fid_num);
	int local_poutbox_remove(const apfeedid_t &p_apfeedid, apfeedid_t *p_poutbox, int *fid_num);
	int set_memc_poutbox(const mckey_t &keys, char *p_poutbox, int fid_num);
	int set_memc_poutbox(unsigned int mimi, char *p_poutbox, int fid_num);
	int del_memc_poutbox(const mckey_t &key);
	int get_memc_poutbox(const mckey_t &key, char **pp_poutbox);
	int get_memc_poutbox(key_list_t &keys, char *p_poutboxes);
	int get_memc_result(key_list_t &keys, char *p_poutboxes);
	int get_stor_poutbox(key_list_t &keys, char *p_poutbox);

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

#endif//POUTBOX_H_2011_09_05
