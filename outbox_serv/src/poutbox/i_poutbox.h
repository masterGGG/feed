/**
 * @file i_poutbox.h
 * @brief  interface for poutbox
 * @author Hansel
 * @version 
 * @date 2011-09-02
 */
#ifndef I_POUTBOX_H_2011_09_02
#define I_POUTBOX_H_2011_09_02

#include <stdint.h>

#include "outbox_proto.h"
#include "i_ini_file.h"

struct i_poutbox
{
	virtual int init(i_ini_file *p_ini) = 0;

	virtual int update(const apfeedid_t *p_apfeedid) = 0;

	virtual int insert(const apfeedid_t *p_apfeedid) = 0;

	virtual int del(const apfeedid_t *p_apfeedid) = 0;

	virtual int get(const unsigned int *p_uid_list, int key_num, char *p_pid_buf, int *buf_data_len) = 0;

	virtual int status(int *hit, int *miss) = 0;

	virtual int uninit() = 0;

	virtual int release() = 0;
};

int create_poutbox_instance(i_poutbox **pp_instance);

#endif//I_POUTBOX_H_2011_09_02
