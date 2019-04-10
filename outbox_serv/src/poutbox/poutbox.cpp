/**
 * @file poutbox.cpp
 * @brief  poutbox implementation
 * @author Hansel
 * @version 
 * @date 2011-09-05
 */

#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <new>
#include <vector>
#include <algorithm>

using namespace std;

#include <errno.h>
#include <time.h>

#include "log.h"
#include "define.h"
#include "poutbox.h"
#include "storage_proto.h"
#include "net_client_utils.h"

//----------------------------------------------------------------------
int create_poutbox_instance(i_poutbox **pp_instance)
{
	if (pp_instance == NULL) {
		return -1;
	}

	c_poutbox *p_instance = new (nothrow)c_poutbox();
	if (p_instance == NULL) {
		return -1;
	}
	else {
		*pp_instance = dynamic_cast<i_poutbox *>(p_instance);
		return 0;
	}
}

//----------------------------------------------------------------------
c_poutbox::c_poutbox()
 :  m_p_memc(NULL),
	m_p_ini(NULL),
	m_hit(0),
	m_miss(0),
	m_size(0),
	m_fd(0),
	m_feed_buffer_len(0),
	m_send_buffer_len(0),
	m_recv_buffer_len(0),
	m_port(0),
	m_feed_buffer(NULL),
	m_send_buffer(NULL),
	m_recv_buffer(NULL)
{
}

//----------------------------------------------------------------------
c_poutbox::~c_poutbox()
{
}

//----------------------------------------------------------------------
int c_poutbox::init(i_ini_file *p_ini)
{
	if (p_ini == NULL)
	{
		return -1;
	}

	m_p_ini = p_ini;
	m_size = m_p_ini->read_int("feed", "poutbox_size", 30);
	DEBUG_LOG("poutbox size: %d", m_size);
	DEBUG_LOG("accept at most %d outbox requests one package", MAX_UID_NUM);

	m_p_memc = memcached_create(NULL);
	if (get_memc_config() == -1) {
		return -1;
	}

	if (get_stor_config() == -1) {
		return -1;
	}

	m_feed_buffer_len = MAX_UID_NUM * m_size * sizeof(apfeedid_t);
	m_feed_buffer = (char *)calloc(m_feed_buffer_len, 1);
	m_send_buffer_len = MAX_UID_NUM * sizeof(indexn_p_pkg_t) + sizeof(request_pkg_t);
	m_send_buffer = (char *)calloc(m_send_buffer_len, 1);
	m_recv_buffer_len = MAX_UID_NUM * m_size * sizeof(pfeed_pkg_t) + sizeof(response_pkg_t);
	m_recv_buffer = (char *)calloc(m_recv_buffer_len, 1);

	if (m_feed_buffer == NULL ||
			m_send_buffer == NULL ||
			m_recv_buffer == NULL) {
		return -1;
	}

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::update(const apfeedid_t *p_apfeedid)
{
	if (p_apfeedid == NULL) {
		return -1;
	}

	DEBUG_LOG(">>poutbox update new<<mimi:%u sender_id:%u target_id:%u atime:%u",
			p_apfeedid[0].pfeedid.src_fid.mimi,
			p_apfeedid[0].pfeedid.sender_id,
			p_apfeedid[0].pfeedid.target_id,
			p_apfeedid[0].atime);
	DEBUG_LOG(">>poutbox update old<<mimi:%u sender_id:%u target_id:%u atime:%u",
			p_apfeedid[1].pfeedid.src_fid.mimi,
			p_apfeedid[1].pfeedid.sender_id,
			p_apfeedid[1].pfeedid.target_id,
			p_apfeedid[1].atime);

	if (p_apfeedid[0].atime < p_apfeedid[1].atime) {
		ERROR_LOG("time new:%u old:%u", p_apfeedid[0].atime, p_apfeedid[1].atime);
		return -1;
	}

	if (p_apfeedid[0].pfeedid.target_id != p_apfeedid[1].pfeedid.target_id) {
		ERROR_LOG("inconsistent target_id new: %u old: %u", 
				p_apfeedid[0].pfeedid.target_id, p_apfeedid[1].pfeedid.target_id);
		return -1;
	}

	mckey_t key;
	if (get_key(key, p_apfeedid[0].pfeedid.target_id) == -1) {
		return -1;
	}

	char *p_poutbox = NULL;
	int ret_num = get_memc_poutbox(key, &p_poutbox);
	if (ret_num <= 0) {
		return ret_num;
	}
	else {
	}

	int rv = local_poutbox_exists(p_apfeedid[0], (apfeedid_t *)p_poutbox, ret_num);
	if (rv == 0) {
		local_poutbox_remove(p_apfeedid[1], (apfeedid_t *)p_poutbox, &ret_num);
		local_poutbox_insert(p_apfeedid[0], (apfeedid_t *)p_poutbox, &ret_num);
	}
	else if (rv == 1){
		local_poutbox_remove(p_apfeedid[1], (apfeedid_t *)p_poutbox, &ret_num);
		local_poutbox_remove(p_apfeedid[0], (apfeedid_t *)p_poutbox, &ret_num);
		local_poutbox_insert(p_apfeedid[0], (apfeedid_t *)p_poutbox, &ret_num);
	}
	else {
		ERROR_LOG("local outbox process error");
		return -1;
	}

	ret_num = ret_num > m_size ? m_size : ret_num;
	if (set_memc_poutbox(key, p_poutbox, ret_num) == -1) {
		ERROR_LOG("set_memc_poutbox error");
		return -1;
	}

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::insert(const apfeedid_t *p_apfeedid)
{
	if (p_apfeedid == NULL) {
		return -1;
	}

	mckey_t key;
	if (get_key(key, p_apfeedid[0].pfeedid.target_id) == -1) {
		return -1;
	}

	DEBUG_LOG(">>poutbox insert<<mimi:%u sender_id:%u target_id:%u atime:%u",
			p_apfeedid[0].pfeedid.src_fid.mimi,
			p_apfeedid[0].pfeedid.sender_id,
			p_apfeedid[0].pfeedid.target_id,
			p_apfeedid[0].atime);

	char *p_poutbox = NULL;
	int ret_num = get_memc_poutbox(key, &p_poutbox);
	if (ret_num < 0) {
		return -1;
	}
	else if (ret_num == 0) {
		return ret_num;
	}
	else {
		/// do nothing
	}

	int rv = local_poutbox_exists(p_apfeedid[0], (apfeedid_t *)p_poutbox, ret_num);
	if (rv == 0) {
		local_poutbox_insert(p_apfeedid[0], (apfeedid_t *)p_poutbox, &ret_num);
	}
	else if (rv == 1) {
		local_poutbox_remove(p_apfeedid[0], (apfeedid_t *)p_poutbox, &ret_num);
		local_poutbox_insert(p_apfeedid[0], (apfeedid_t *)p_poutbox, &ret_num);
		/// nothing to do
	}
	else {
		ERROR_LOG("local poutbox process error");
		return -1;
	}

	ret_num = ret_num > m_size ? m_size : ret_num;
	if (set_memc_poutbox(key, p_poutbox, ret_num) == -1) {
		ERROR_LOG("set_memc_poutbox error");
		return -1;
	}

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::del(const apfeedid_t *p_apfeedid)
{
	if (p_apfeedid == NULL) {
		return -1;
	}

	DEBUG_LOG(">>poutbox del<<mimi:%u sender_id:%u target_id:%u atime:%u",
			p_apfeedid[0].pfeedid.src_fid.mimi,
			p_apfeedid[0].pfeedid.sender_id,
			p_apfeedid[0].pfeedid.target_id,
			p_apfeedid[0].atime);

	mckey_t key;
	if (get_key(key, p_apfeedid[0].pfeedid.target_id) == -1) {
		return -1;
	}

	char *p_poutbox = NULL;
	int ret_num = get_memc_poutbox(key, &p_poutbox);
	if (ret_num < 0) {
		return -1;
	}
	else if (ret_num == 0) {
		/// no data for the specified key
		return 0;
	}
	else {
		/// do nothing
	}

	local_poutbox_remove(p_apfeedid[0], (apfeedid_t *)p_poutbox, &ret_num);

	ret_num = ret_num > m_size ? m_size : ret_num;
	if (ret_num == 0) {
		if (del_memc_poutbox(key) == -1) {
			ERROR_LOG("del_memc_poutbox error");
			return -1;
		}
	}
	else if (ret_num > 0) {
		if (set_memc_poutbox(key, p_poutbox, ret_num) == -1) {
			ERROR_LOG("set_memc_poutbox error");
			return -1;
		}
	}
	else {
		/// do nothing
	}

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::get(const unsigned int *p_uid_list, int key_num, char *p_poutboxes, int *buf_data_len)
{
	if (p_uid_list == NULL || p_poutboxes == NULL || buf_data_len == NULL) {
		return -1;
	}

	key_list_t keys;
	if (get_list_of_key(keys, key_num, p_uid_list) == -1) {
		ERROR_LOG("get_list_of_key().");
		return -1;
	}

	int ret_num_memc = get_memc_poutbox(keys, p_poutboxes);
	if (ret_num_memc < 0) {
		ERROR_LOG("get_memc_poutbox().");
		return -1;
	}

	int ret_num_stor = get_stor_poutbox(keys, p_poutboxes + ret_num_memc * sizeof(apfeedid_t));
	if (ret_num_stor < 0) {
		ERROR_LOG("get_stor_poutbox().");
		return -1;
	}

	*buf_data_len = (ret_num_memc + ret_num_stor) * sizeof(apfeedid_t);

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::status(int *hit, int *miss)
{
	if (hit == NULL || miss == NULL) {
		return -1;
	}

	*hit = m_hit;
	*miss = m_miss;

	m_hit = 0;
	m_miss = 0;

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::uninit()
{
	memcached_free(m_p_memc);
	close_conn(m_fd);

	free(m_feed_buffer);
	free(m_send_buffer);
	free(m_recv_buffer);

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::release()
{
	delete this;

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::get_memc_config()
{
	char key_buf[64] = {0};
	char val_buf[64] = {0};
	char *pcolon = NULL;
	int eno = 0;
	int port = 0;
	int index = 0;

	while (1) {
		/// generate key
		if (snprintf(key_buf, sizeof(key_buf), "memcached_server_%d", index) == sizeof(key_buf)) {
			return -1;
		}
		m_p_ini->read_string("memcached", key_buf, val_buf, sizeof(val_buf), NULL);

		/// get errno
		eno = m_p_ini->get_last_errno();
		if (eno == 6) {
			/// no more memcached server so quit
			return 0;
		}
		else if (eno == 0) {
			/// find one value for key
			pcolon = strchr(val_buf, ':');
			sscanf(pcolon + 1, "%d", &port);
			memset(pcolon, 0, sizeof(val_buf) - (pcolon - val_buf));

			memcached_server_add(m_p_memc, val_buf, port);
			DEBUG_LOG("memcached server add %s:%d", val_buf, port);
		}
		else {
			/// error in reading ini file
			ERROR_LOG("get_memc_config error: %s", m_p_ini->get_last_errstr());
			return -1;
		}
		memset(key_buf, 0, sizeof(key_buf));
		memset(val_buf, 0, sizeof(key_buf));

		++index;
	}
}

//----------------------------------------------------------------------
int c_poutbox::get_stor_config()
{
	char *pcolon = NULL;
	int rv = 0;

	memset(m_ip, 0, sizeof(m_ip));
	rv =m_p_ini->read_string("storage", "storage_server", m_ip, sizeof(m_ip), NULL);
	if (rv == -1) {
		ERROR_LOG("get_stor_config error: %s", m_p_ini->get_last_errstr());
		return -1;  
	}                   

	pcolon = strchr(m_ip, ':');
	sscanf(pcolon + 1, "%d", &m_port);
	memset(pcolon, 0, sizeof(m_ip) - (pcolon - m_ip));

	m_fd = open_conn(m_ip, m_port, G_CONN_TIMEOUT);
	if (m_fd == -1) {
		ERROR_LOG("unable to connect storage server %s:%d", m_ip, m_port);
		return -1;
	}
	DEBUG_LOG("connected to storage server %s:%d", m_ip, m_port);

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::get_key(mckey_t &key, unsigned int uid)
{
	char key_str[16] = {0};
	int rv = 0;

	rv = snprintf(key_str, sizeof(key_str), "%u_p", uid);
	if (rv == (int)sizeof(key_str)) {
		return -1;
	}
	key.key = uid;
	key.key_str = string(key_str);

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::get_list_of_key(key_list_t &keys, int key_num, const unsigned int *p_uid)
{
	if (p_uid == NULL) {
		return -1;
	}

	for (int index = 0; index != key_num; ++index) {
		mckey_t key;
		if (get_key(key, p_uid[index]) == -1) {
			return -1;
		}
		keys.push_back(key);
	}

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::local_poutbox_exists(const apfeedid_t &apfeedid, apfeedid_t *p_poutbox, int fid_num)
{
	if (p_poutbox == NULL) { 
		return -1; 
	} 

	for (int index = 0; index != fid_num; ++index) { 
		if (p_poutbox[index].pfeedid.src_fid.mimi == apfeedid.pfeedid.src_fid.mimi &&
				p_poutbox[index].pfeedid.src_fid.cmd_id == apfeedid.pfeedid.src_fid.cmd_id &&
				p_poutbox[index].pfeedid.src_fid.app_id == apfeedid.pfeedid.src_fid.app_id &&
				p_poutbox[index].pfeedid.src_fid.timestamp == apfeedid.pfeedid.src_fid.timestamp &&
				p_poutbox[index].pfeedid.src_fid.magic == apfeedid.pfeedid.src_fid.magic &&
				p_poutbox[index].pfeedid.sender_id == apfeedid.pfeedid.sender_id &&
				p_poutbox[index].pfeedid.target_id == apfeedid.pfeedid.target_id &&
				p_poutbox[index].pfeedid.p_magic == apfeedid.pfeedid.p_magic) {
			return 1;
		}
	} 
	return 0; 
}

//----------------------------------------------------------------------
int c_poutbox::local_poutbox_insert(const apfeedid_t &apfeedid, apfeedid_t *p_poutbox, int *fid_num)
{
	if (p_poutbox == NULL) { 
		return -1; 
	} 

	for (int index = 0; index != *fid_num; ++index) {
		if (p_poutbox[index].atime <= apfeedid.atime) {
			memmove(&p_poutbox[index + 1], &p_poutbox[index], (*fid_num - index) * sizeof(apfeedid_t));
			memcpy(&p_poutbox[index], &apfeedid, sizeof(apfeedid_t));
			++(*fid_num);
			break;
		}
		if (index == (*fid_num) - 1) {
			memcpy(&p_poutbox[index + 1], &apfeedid, sizeof(apfeedid_t));
			++(*fid_num);
			break;
		}
	}
	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::local_poutbox_remove(const apfeedid_t &apfeedid, apfeedid_t *p_poutbox, int *fid_num)
{
	if (p_poutbox == NULL) {
		return -1;
	}

	for (int index = 0; index != *fid_num; ++index) {
		if (p_poutbox[index].pfeedid.src_fid.mimi == apfeedid.pfeedid.src_fid.mimi &&
				p_poutbox[index].pfeedid.src_fid.cmd_id == apfeedid.pfeedid.src_fid.cmd_id &&
				p_poutbox[index].pfeedid.src_fid.app_id == apfeedid.pfeedid.src_fid.app_id &&
				p_poutbox[index].pfeedid.src_fid.timestamp == apfeedid.pfeedid.src_fid.timestamp &&
				p_poutbox[index].pfeedid.src_fid.magic == apfeedid.pfeedid.src_fid.magic &&
				p_poutbox[index].pfeedid.sender_id == apfeedid.pfeedid.sender_id &&
				p_poutbox[index].pfeedid.target_id == apfeedid.pfeedid.target_id &&
				p_poutbox[index].pfeedid.p_magic == apfeedid.pfeedid.p_magic) {
			memmove(&p_poutbox[index], &p_poutbox[index + 1], ((*fid_num) - index - 1) * sizeof(apfeedid_t));
			--(*fid_num);
			break;
		}
	}
	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::set_memc_poutbox(const mckey_t &key, char *p_poutbox, int fid_num)
{
	if (p_poutbox == NULL) {
		return -1;
	}

	memcached_return_t memc_ret = memcached_set(m_p_memc, key.key_str.c_str(), 
			key.key_str.size(), p_poutbox, fid_num * sizeof(apfeedid_t), (time_t)0, (uint32_t)0);

	if (memc_ret != MEMCACHED_SUCCESS) {
		ERROR_LOG("memcached_set error: %s", memcached_strerror(NULL, memc_ret));
		return -1;
	}

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::set_memc_poutbox(unsigned int mimi, char *p_poutbox, int fid_num)
{
	if (p_poutbox == NULL) {
		return -1;
	}
	mckey_t key;
	if (get_key(key, mimi) == -1) {
		return -1;
	}
	return set_memc_poutbox(key, p_poutbox, fid_num);
}

//----------------------------------------------------------------------
int c_poutbox::del_memc_poutbox(const mckey_t &key)
{
	memcached_return_t memc_ret = memcached_delete(m_p_memc, key.key_str.c_str(), key.key_str.size(), (time_t)0);

	if (memc_ret != MEMCACHED_SUCCESS) {
		ERROR_LOG("memcached_delete error: %s", memcached_strerror(NULL, memc_ret));
		return -1;
	}                                                                                    

	return 0;
}

//----------------------------------------------------------------------
int c_poutbox::get_memc_poutbox(const mckey_t &key, char **pp_poutbox)
{
	size_t ret_len = 0;
	uint32_t flags = 0;
	memcached_return_t memc_ret;
	*pp_poutbox = memcached_get(m_p_memc, key.key_str.c_str(), key.key_str.size(), &ret_len, &flags, &memc_ret);

	if (*pp_poutbox == NULL && memc_ret == MEMCACHED_NOTFOUND) {
		/// the requested feed id is not in memcached
		return 0;
	}   
	else if (*pp_poutbox == NULL && memc_ret != MEMCACHED_NOTFOUND) {
		ERROR_LOG("memcached_get error: %s", memcached_strerror(NULL, memc_ret));
		return -1; 
	}   
	else {
		/// nothing to do
	}   

	int ret_num = ret_len / sizeof(apfeedid_t);
	ret_num = ret_num > m_size ? m_size : ret_num;
	memset(m_feed_buffer, 0, m_feed_buffer_len);
	memcpy(m_feed_buffer, *pp_poutbox, ret_num * sizeof(apfeedid_t));
	free(*pp_poutbox);
	*pp_poutbox = m_feed_buffer;

	return ret_num;
}

//----------------------------------------------------------------------
int c_poutbox::get_memc_poutbox(key_list_t &keys, char *p_poutboxes)
{
	if (keys.size() < 1 || keys.size() > MAX_UID_NUM) {
		return -1;
	}

	vector<const char *> real_keys;
	vector<size_t> key_len;
	key_list_t::iterator it = keys.begin();

	while (it != keys.end()) {
		real_keys.push_back((*it).key_str.c_str());
		key_len.push_back((*it).key_str.length());
		DEBUG_LOG("get_memc_poutbox key: %u", it->key);
		++it;
	}

	if (!real_keys.empty()) {
		memcached_return_t memc_ret = memcached_mget(m_p_memc, &real_keys[0], &key_len[0], real_keys.size());
		if (memc_ret != MEMCACHED_SUCCESS) {
			ERROR_LOG("memcached_mget error: %s", memcached_strerror(NULL, memc_ret));
			return 0;
		}
	}

	return get_memc_result(keys, p_poutboxes);
}

//----------------------------------------------------------------------
int c_poutbox::get_memc_result(key_list_t &keys, char *p_poutboxes)
{
	if (p_poutboxes == NULL) {
		return -1;
	}

	char *ret_val = NULL;
	char *ret_key = NULL;
	size_t val_len = 0;
	int ret_num = 0;
	int tot_num = 0;

	key_list_t::iterator it;
	memcached_result_st *memc_rs;
	memcached_return_t memc_ret;

	key_list_t::size_type init_size = keys.size();
	while ((memc_rs = memcached_fetch_result(m_p_memc, NULL, &memc_ret))) {
		ret_key = (char *)memcached_result_key_value(memc_rs);
		ret_val = (char *)memcached_result_value(memc_rs);
		val_len = memcached_result_length(memc_rs);

		ret_num = (int)(val_len / sizeof(apfeedid_t));
		ret_num = ret_num > m_size ? m_size : ret_num;
		memcpy(p_poutboxes + tot_num * sizeof(apfeedid_t), ret_val, ret_num * sizeof(apfeedid_t));

		it = find_if(keys.begin(), keys.end(), key_eq_t(atoi(ret_key)));
		keys.erase(it);
		tot_num += ret_num;
		memcached_result_free(memc_rs);
	}
	key_list_t::size_type miss_size = keys.size();

	m_hit += (init_size - miss_size);

	return tot_num;
}

//----------------------------------------------------------------------
int c_poutbox::get_stor_poutbox(key_list_t &keys, char *p_poutboxes)
{
	if (p_poutboxes == NULL) {
		return -1; 
	}

	if (keys.empty()) {
		return 0;
	}   

	m_miss += keys.size();

	memset(m_send_buffer, 0, m_send_buffer_len);
	memset(m_recv_buffer, 0, m_recv_buffer_len);

	/// pack the request package.
	request_pkg_t *p_request = (request_pkg_t *)m_send_buffer;
	key_list_t::iterator it = keys.begin();
	for (int i = 0; it != keys.end(); ++it, ++i) {
		p_request->indexns_p[i].mimi = it->key;
		p_request->indexns_p[i].starttime = time(NULL);
		p_request->indexns_p[i].prev_num = m_size;
		DEBUG_LOG("get_stor_poutbox: %u", it->key);
	}
	p_request->len = sizeof(request_pkg_t) + keys.size() * sizeof(indexn_p_pkg_t);
	p_request->op = REQ_OP_PASS_GETS_IDX;
	p_request->units = keys.size();

	if (m_fd == -1) {
		m_fd = open_conn(m_ip, m_port, G_CONN_TIMEOUT);
		if (m_fd == -1) {
			ERROR_LOG("unable to connect to storage server %s:%d", m_ip, m_port);
			return -1;
		}
		DEBUG_LOG("connect to storage server %s:%d", m_ip, m_port);
	}

	if (send_rqst(m_fd, m_send_buffer, p_request->len, m_recv_buffer, m_recv_buffer_len, G_SEND_TIMEOUT) == -1) {
		close_conn(m_fd);
		ERROR_LOG("send_rqst to storage server %s:%d", m_ip, m_port);
		m_fd = -1;
		return -1;
	}

	response_pkg_t *p_response = (response_pkg_t *)m_recv_buffer;
	if (p_response->ret != RES_OP_SUCCESS) {
		ERROR_LOG("storage_server return failed: %hu len: %u", p_response->ret, p_response->len);
		return -1;
	}

	if (p_response->units == 0) {
		WARN_LOG("request for apfeedid but nothing found.");
		return 0;
	}

	int last_pos = 0;
	char *p_idx = NULL;
	for (int i = 0; i != p_response->units; ++i) {
		p_idx = ((char *)(&(p_response->pfeeds[i]))) + 4;
		memcpy(p_poutboxes + i * sizeof(apfeedid_t), p_idx, sizeof(apfeedid_t));
		if (p_response->pfeeds[i].fid.target_id != p_response->pfeeds[i - 1].fid.target_id && i != 0) {
			if (set_memc_poutbox(p_response->pfeeds[last_pos].fid.target_id,
						p_poutboxes + last_pos * sizeof(apfeedid_t), i - last_pos) == -1) {
				ERROR_LOG("set_memc_poutbox error");
			}
			last_pos = i;
		}
	}
	if (set_memc_poutbox(p_response->pfeeds[last_pos].fid.target_id,
				p_poutboxes + last_pos * sizeof(apfeedid_t), p_response->units - last_pos) == -1) {
		ERROR_LOG("set_memc_poutbox error");
	}

	return p_response->units;
}
