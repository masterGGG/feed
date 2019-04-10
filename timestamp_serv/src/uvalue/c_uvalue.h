/**
 * =====================================================================================
 *       @file  c_uvalue.h
 *      @brief  use bdb to store data for uvalue
 *
 *  Detailed description starts here.
 *
 *   @internal
 *     Created  09/07/2010 04:19:42 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2010, TaoMee.Inc, ShangHai.
 *
 *     @author  imane (曼曼), imane@taomee.com
 * This source code was wrote for TaoMee,Inc. ShangHai CN.
 * =====================================================================================
 */
#ifndef C_UVALUE_H_2010_09_07
#define C_UVALUE_H_2010_09_07

#include "db.h"
#include "../i_uvalue.h"

#define MAX_BUFFER (4096 * 1024) //4mb
#define MAX_DATABASE_NAME 1024

class c_uvalue: public i_uvalue
{
    public:
        c_uvalue();
        virtual int init(const char *p_file, uint32_t flags, int mode, cb_bt_compare_t cb_bt_compare);
        virtual int insert(const void *p_key_data, uint32_t key_size, const void *p_value_data, uint32_t value_size);
        virtual int update(const void *p_key_data, uint32_t key_size, const void *p_value_data, uint32_t value_size, cb_value_opcode_t value_opcode);
        virtual int set(const void *p_key_data, uint32_t key_size, const void *p_value_data, uint32_t value_size, cb_value_opcode_t value_opcode);
        virtual int get(const void *p_key_data, uint32_t key_size, void *p_buffer, uint32_t *p_buffer_size);
        virtual int del(const void *p_key_data, uint32_t key_size);
        virtual int get_key_count(uint32_t *p_count, int skip_zero);
        virtual int traverse(cb_traverse_t cb_traverse, void *p_user);
        virtual int merge(const char *p_file, key_opcode_t key_opcode, cb_value_opcode_t value_opcode);
        virtual int merge(const i_uvalue *p_uvalue, key_opcode_t key_opcode, cb_value_opcode_t value_opcode);
        virtual int get_last_errno();
        virtual const char* get_last_errstr();
        virtual int flush();
        virtual const char* get_database_name() const;
        virtual int uninit();
        virtual int release();
    private:
        DB *m_pdb;
        DB_ENV *m_penv;
        DBC *m_pcursor;
        int m_errno;
        int m_myerrno;
        char m_buffer[MAX_BUFFER];
        int m_init;
        char m_database_name[MAX_DATABASE_NAME];
};

#endif
