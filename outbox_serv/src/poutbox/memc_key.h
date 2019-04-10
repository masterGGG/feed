/**
 * @file memc_key.h
 * @brief  
 * @author Hansel
 * @version 
 * @date 2011-06-14
 */

#ifndef MEMC_KEY_H
#define MEMC_KEY_H

#include <string>
#include <list>

#include "feedid.h"

typedef struct {
	uint32_t key;
	std::string key_str;
} mckey_t;

struct key_eq_t {
	explicit key_eq_t(int key)
	{
		m_key = key;
	}
	bool operator()(const mckey_t &mckey)
	{
		return m_key == (int)mckey.key;
	}
	int m_key;
};

typedef std::list<mckey_t> key_list_t;

#endif//MEMC_KEY_H
