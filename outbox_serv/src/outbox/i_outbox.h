/**
 * @file i_outbox.h
 * @brief  outbox service interface
 * @author Hansel
 * @version 
 * @date 2011-06-09
 */

#ifndef I_OUTBOX_H_2011_06_09
#define I_OUTBOX_H_2011_06_09

#include <stdint.h>

#include "feedid.h"
#include "i_ini_file.h"

struct i_outbox
{
	/**
	 * @brief outbox initialization 
	 *
	 * @param p_ini  ini file instance
	 *
	 * @returns  0 on success -1 on error 
	 */
	virtual int init(i_ini_file *p_ini) = 0;

	/**
	 * @brief  update memcached content, do nothing if user not in memcached
	 *
	 * @param p_feedid first being the new one second being the old
	 *
	 * @returns  0 on success -1 on error
	 */
	virtual int update(const feedid_t *p_feedid) = 0;

	/**
	 * @brief	insert into memcached, do nothing if user not in memcached
	 *
	 * @param p_feedid feed id to be inserted
	 *
	 * @returns  0 on success -1 on error 
	 */
	virtual int insert(const feedid_t *p_feedid) = 0;

	/**
	 * @brief  delete one feed in memcached, do nothing if user not in memcached 
	 *
	 * @param p_feedid feed id to be deleted
	 *
	 * @returns  0 on success -1 on error 
	 */
	virtual int del(const feedid_t *p_feedid) = 0;

	/**
	 * @brief  get feedid of the requested uid list
	 *
	 * @param p_uid_list requested user id list
	 * @param key_num number of uids
	 * @param p_id_buf send buffer offered by caller
	 * @param buf_data_len data length in the buffer
	 *
	 * @returns	0 on success -1 on error
	 */
	virtual int get(const unsigned int *p_uid_list, int key_num, char *p_id_buf, int *buf_data_len) = 0;

	/**
	 * @brief  get outbox status, reset the status on every query
	 *
	 * @param hit return number of times get from memcached
	 * @param miss return number of times get from storage-server
	 *
	 * @returns  0 on success -1 on error 
	 */
	virtual int status(int *hit, int *miss) = 0;

	/**
	 * @brief  uninit
	 *
	 * @returns 0 on success -1 on error
	 */
	virtual int uninit() = 0;

	/**
	 * @brief  deallocate resource 
	 *
	 * @returns  0 on success -1 on error 
	 */
	virtual int release() = 0;
};

int create_outbox_instance(i_outbox **pp_outbox);

#endif//I_OUTBOX_H_2011_06_09
