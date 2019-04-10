/**
 * =====================================================================================
 *       @file  c_uvalue.cpp
 *      @brief
 *
 *  Detailed description starts here.
 *
 *   @internal
 *     Created  09/07/2010 04:20:11 PM
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2010, TaoMee.Inc, ShangHai.
 *
 *     @author  imane (曼曼), imane@taomee.com
 * This source code was wrote for TaoMee,Inc. ShangHai CN.
 * =====================================================================================
 */
#include <new>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "c_uvalue.h"

static i_uvalue::cb_bt_compare_t g_compare = NULL;
int user_compare(DB *db, const DBT *key1, const DBT *key2, size_t *len)
{
    return g_compare(key1->data, key1->size, key2->data, key2->size);
}

int create_uvalue_instance(i_uvalue **pp_instance)
{
    if (pp_instance == NULL)
    {
        return -1;
    }
    i_uvalue *p_uvalue = new (std::nothrow)c_uvalue();
    if (p_uvalue == NULL)
    {
        return -1;
    }
    else
    {
        *pp_instance = dynamic_cast<i_uvalue *>(p_uvalue);
        return 0;
    }
}

c_uvalue::c_uvalue():m_pdb(NULL),m_penv(NULL),m_pcursor(NULL),m_errno(0),m_myerrno(0),m_init(0)
{
    memset(m_buffer, 0, MAX_BUFFER);
    memset(m_database_name, 0, MAX_DATABASE_NAME);
}

int c_uvalue::init(const char *p_file, uint32_t flags, int mode, cb_bt_compare_t cb_bt_compare)
{
    if (m_init != 0)
    {
        m_myerrno = 7;
        m_errno = 0;
        return 1;
    }

    memcpy(m_database_name, p_file, strlen(p_file));
    m_database_name[strlen(p_file)] = '\0';

    int ret;
    u_int32_t db_flags = 0;
    //u_int32_t env_flags;
    DBTYPE type;

    if (BTREE == (flags & BTREE))
    {
        type = DB_BTREE;
    }
    if (HASH == (flags & HASH))
    {
        type = DB_HASH;
    }
    if (CREATE == (flags & CREATE))
    {
        db_flags |= DB_CREATE;
    }
    if (EXCL == (flags & EXCL))
    {
        db_flags |= DB_EXCL;
    }


    //create an environment object and init it
    //ret = db_env_create(&m_penv,0);
    //if (ret != 0)
    //{
    //    m_myerrno = 0;
    //    m_errno = ret;
    //    uninit();
    //    return -1;
    //}
    //env_flags = DB_CREATE | DB_INIT_MPOOL;
    //ret = m_penv->open(m_penv,NULL,env_flags,0);
    //if (ret != 0)
    //{
    //    m_myerrno = 0;
    //    m_errno = ret;
    //    uninit();
    //    return -1;
    //}
    //init db structure and open database
    //ret = db_create(&m_pdb, m_penv, 0);
    ret = db_create(&m_pdb, 0, 0);
    if (ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        uninit();
        return -1;
    }

    if (cb_bt_compare != NULL)
    {
        g_compare = cb_bt_compare;
        if (DB_BTREE == type)
        {
            m_pdb->set_bt_compare(m_pdb, user_compare);
        }
        if (DB_HASH == type)
        {
            m_pdb->set_h_compare(m_pdb, user_compare);
        }
    }

    //db_flags = DB_CREATE;
    ret = m_pdb->open(m_pdb, NULL, p_file, NULL, type, db_flags, mode);

    if (ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        uninit();
        return -1;
    }

    // get a cursor
    m_pdb->cursor(m_pdb, NULL, &m_pcursor, 0);

    m_init = 1;
    return 0;
}

int c_uvalue::insert(const void *p_key_data, uint32_t key_size, const void *p_value_data, uint32_t value_size)
{
    if (m_init == 0)
    {
        m_myerrno = 8;
        m_errno = 0;
        return -1;
    }
    int ret;
    DBT key,data;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    key.data = (void *)p_key_data;
    key.size = key_size;
    data.data = (void *)p_value_data;
    data.size = value_size;

    ret = m_pdb->put(m_pdb, NULL, &key, &data, DB_NOOVERWRITE);
    if (ret == DB_KEYEXIST)
    {
        m_myerrno = 0;
        m_errno = ret;
        return -1;
    }
    else if (ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        return -1;
    }
    return 0;
}

int c_uvalue::update(const void *p_key_data, uint32_t key_size, const void *p_value_data, uint32_t value_size, cb_value_opcode_t value_opcode)
{
    if (m_init == 0)
    {
        m_myerrno = 8;
        m_errno = 0;
        return -1;
    }
    int ret;
    DBT key,srcdata,descdata;
    memset(&key, 0, sizeof(DBT));
    memset(&srcdata, 0, sizeof(DBT));
    memset(&descdata, 0, sizeof(DBT));

    key.data = (void *)p_key_data;
    key.size = key_size;

    ret = m_pdb->get(m_pdb, NULL, &key, &srcdata, 0);
    if (ret == DB_NOTFOUND || ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        return -1;
    }

    if (value_opcode == NULL)// 直接更新
    {
        descdata.data = (void *)p_value_data;
        descdata.size = value_size;

        if ((ret = m_pdb->put(m_pdb, NULL, &key, &descdata, 0)))
        {
            m_myerrno = 0;
            m_errno = ret;
            return -1;
        }
    }
    else  //调用回调value_opcode函数更新
    {
        uint32_t bufferlen = MAX_BUFFER;
        if (0 != value_opcode(srcdata.data, srcdata.size, p_value_data, value_size, (void *)m_buffer, &bufferlen))
        {
            m_errno = 0;
            m_myerrno = 1;
            return -1;
        }
        descdata.data = m_buffer;
        descdata.size = bufferlen;
        if (0 != (ret = m_pdb->put(m_pdb, NULL, &key, &descdata, 0)))
        {
            m_myerrno = 0;
            m_errno = ret;
            return -1;
        }
    }
    return 0;
}

int c_uvalue::set(const void *p_key_data, uint32_t key_size, const void *p_value_data, uint32_t value_size, cb_value_opcode_t value_opcode)
{
    if (m_init == 0)
    {
        m_myerrno = 8;
        m_errno = 0;
        return -1;
    }
    int ret;
    DBT key,srcdata,descdata;
    memset(&key, 0, sizeof(DBT));
    memset(&srcdata, 0, sizeof(DBT));
    memset(&descdata, 0, sizeof(DBT));

    key.data = (void *)p_key_data;
    key.size = key_size;

    ret = m_pdb->get(m_pdb, NULL, &key, &srcdata, 0);
    if (ret == DB_NOTFOUND) // 未发现记录
    {
        descdata.data = (void *)p_value_data;
        descdata.size = value_size;

        if ((ret = m_pdb->put(m_pdb, NULL, &key, &descdata, 0)))
        {
            m_myerrno = 0;
            m_errno = ret;
            return -1;
        }
        return 0;
    }
    else if (ret != 0) //发生错误
    {
        m_myerrno = 0;
        m_errno = ret;
        return -1;
    }

    if (value_opcode == NULL)// 直接更新
    {
        descdata.data = (void *)p_value_data;
        descdata.size = value_size;

        if ((ret = m_pdb->put(m_pdb, NULL, &key, &descdata, 0)))
        {
            m_myerrno = 0;
            m_errno = ret;
            return -1;
        }
    }
    else  //调用回调value_opcode函数更新
    {
        uint32_t bufferlen = MAX_BUFFER;
        if (0 != value_opcode(srcdata.data, srcdata.size, (void *)p_value_data, value_size, (void *)m_buffer, &bufferlen))
        {
            m_errno = 0;
            m_myerrno = 1;
            return -1;
        }
        descdata.data = m_buffer;
        descdata.size = bufferlen;
        if (0 != (ret = m_pdb->put(m_pdb, NULL, &key, &descdata, 0)))
        {
            m_myerrno = 0;
            m_errno = ret;
            return -1;
        }
    }
    return 0;
}

int c_uvalue::get(const void *p_key_data, uint32_t key_size, void *p_buffer, uint32_t *p_buffer_size)
{
    if (m_init == 0)
    {
        m_myerrno = 8;
        m_errno = 0;
        return -1;
    }
    int ret;
    DBT key,data;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = (void *)p_key_data;
    key.size = key_size;

    ret = m_pdb->get(m_pdb, NULL, &key, &data, 0);
    if (ret == DB_NOTFOUND)
    {
        return 1;
    }
    else if (ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        return -1;
    }

    if (p_buffer_size != NULL)
    {
        if (p_buffer != NULL)
        {
            //if (data.size > *p_buffer_size)//用户提供的缓存不够
            //{
            //    m_myerrno = 2;
            //    m_errno = 0;
            //    return -1;
            //}
            memcpy(p_buffer, data.data, (data.size > *p_buffer_size ? *p_buffer_size : data.size));
        }
        *p_buffer_size = data.size;
    }
    return 0;
}

int c_uvalue::del(const void *p_key_data, uint32_t key_size)
{
    if (m_init == 0)
    {
        m_myerrno = 8;
        m_errno = 0;
        return -1;
    }
    int ret;
    DBT key;
    memset(&key, 0, sizeof(DBT));

    key.data = (char *)p_key_data;
    key.size = key_size;

    ret = m_pdb->del(m_pdb, NULL, &key, 0);
    if (ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        return -1;
    }
    return 0;
}

int is_zero(void *src, int len)
{
    int i = 0;
    for (; i < len; i++)
    {
        if(((char *)src)[i] != 0)
        {
            return 0;
        }
    }
    return 1;
}

int c_uvalue::get_key_count(uint32_t *p_count, int skip_zero)
{
    if (m_init == 0)
    {
        m_myerrno = 8;
        m_errno = 0;
        return -1;
    }
    int ret;
    int count=0; // 暂存计数
    DBT key,data;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    ret = m_pcursor -> get(m_pcursor, &key, &data, DB_FIRST);
    if (ret == DB_NOTFOUND)
    {
        return 0;
    }

    if (ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        return -1;
    }

    if (is_zero(data.data, data.size))
    {
        if (!skip_zero)
        {
            count++;
        }
    }
    else
    {
        count++;
    }

    while ((ret = m_pcursor->get(m_pcursor, &key, &data, DB_NEXT)) == 0 )
    {
        if (skip_zero)
        {
            if (is_zero(data.data, data.size))
            {
                continue;
            }
        }
        count++;
    }
    if (ret != DB_NOTFOUND)
    {
        m_myerrno = 0;
        m_errno = ret;
        return -1;
    }
    *p_count = count;
    return 0;
}

int c_uvalue::traverse(cb_traverse_t cb_traverse, void *p_user)
{
    if (m_init == 0)
    {
        m_myerrno = 8;
        m_errno = 0;
        return -1;
    }
    int ret;
    DBT key,data;
    DBC *pcursor = NULL;
    void *p;
    size_t klen,dlen;
    void *pkey,*pdata;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    char buffer[5*1024*1024] = {0};
    data.data = buffer;
    data.ulen = 5*1024*1024;
    data.flags = DB_DBT_USERMEM;

    if ((ret = m_pdb->cursor(m_pdb, NULL, &pcursor, 0)) != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        if (pcursor != NULL)
        {
            pcursor->close(pcursor);
        }
        return -1;
    }

    for (;;)
    {
        if ((ret = pcursor->get(pcursor, &key, &data, DB_MULTIPLE_KEY | DB_NEXT)) != 0)
        {
            if (ret != DB_NOTFOUND)
            {
                m_myerrno = 0;
                m_errno = ret;
                if (pcursor != NULL)
                {
                    pcursor->close(pcursor);
                }
                return -1;
            }
            break;
        }

        for (DB_MULTIPLE_INIT(p, &data);;)
        {
            DB_MULTIPLE_KEY_NEXT(p, &data, pkey, klen, pdata, dlen);
            if (p == NULL)
                break;
            if (0 != cb_traverse(pkey, klen, pdata, dlen, p_user))
            {
                m_myerrno = 3;
                m_errno = 0;
                if (pcursor != NULL)
                {
                    pcursor->close(pcursor);
                }
                return -1;
            }
        }
    }

    //if ((ret = m_pcursor -> get(m_pcursor, &key, &data, DB_FIRST)) == 0)
    //{
    //    if (0 != cb_traverse(key.data, key.size, data.data, data.size, p_user))
    //    {
    //        m_myerrno = 3;
    //        m_errno = 0;
    //        return -1;
    //    }
    //}
    //else if (ret == DB_NOTFOUND) //该数据库为空
    //{
    //    return 0;
    //}
    //else //查询出错
    //{
    //    m_myerrno = 0;
    //    m_errno = ret;
    //    return -1;
    //}
    //
    //while ((ret = m_pcursor->get(m_pcursor, &key, &data, DB_NEXT)) == 0)
    //{
    //    if (0 != cb_traverse(key.data, key.size, data.data, data.size, p_user))
    //    {
    //        m_myerrno = 3;
    //        m_errno = 0;
    //        return -1;
    //    }
    //}
    //if (ret != DB_NOTFOUND)
    //{
    //    m_myerrno = 0;
    //    m_errno = ret;
    //    return -1;
    //}

    if (pcursor != NULL)
    {
        pcursor->close(pcursor);
    }

    return 0;
}

int c_uvalue::merge(const char *p_file, key_opcode_t key_opcode, cb_value_opcode_t value_opcode)
{
    if (m_init == 0)
    {
        m_myerrno = 8;
        m_errno = 0;
        return -1;
    }
    int ret;
    DB *p_desdb = NULL;
    DBC *p_cursor = NULL,*p_srccursor = NULL;
    DBT key,srcdata,desdata,updata;
    memset(&key, 0, sizeof(DBT));
    memset(&srcdata, 0, sizeof(DBT));
    memset(&desdata, 0, sizeof(DBT));
    memset(&updata, 0, sizeof(DBT));

    ret = db_create(&p_desdb, m_penv, 0);
    if (ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        goto ERR;
    }

    ret = p_desdb->open(p_desdb, NULL, p_file, NULL, DB_UNKNOWN, 0, 0);
    if (ret != 0)
    {
        m_myerrno = 0;
        m_errno = ret;
        goto ERR;
    }
    m_pdb->cursor(m_pdb, NULL, &p_srccursor, 0);
    p_desdb->cursor(p_desdb, NULL, &p_cursor, 0);

    //处理交集，并集，差集
    if (key_opcode == INTERSECT)//交集
    {
        while ((ret = p_srccursor->get(p_srccursor, &key, &srcdata, DB_NEXT)) == 0) //遍历本类对象说指向的数据库
        {
            ret = p_desdb->get(p_desdb, NULL, &key, &desdata, 0);
            if (ret == 0)//存在
            {
                uint32_t bufferlen = MAX_BUFFER;
                if (0 != value_opcode(srcdata.data, srcdata.size, desdata.data, desdata.size, (void *)m_buffer, &bufferlen))
                {
                    m_errno = 0;
                    m_myerrno = 5;
                    goto ERR;
                }
                updata.data = m_buffer;
                updata.size = bufferlen;
                if (0 != (ret = p_srccursor->put(p_srccursor, &key, &updata, DB_CURRENT)))
                {
                    m_myerrno = 0;
                    m_errno = ret;
                    goto ERR;
                }
            }
            else if (ret == DB_NOTFOUND) //不存在
            {
                ret = p_srccursor->del(p_srccursor, 0);
                if (ret != 0)
                {
                    m_myerrno = 0;
                    m_errno = ret;
                    goto ERR;
                }
            }
            else
            {
                m_myerrno = 0;
                m_errno = ret;
                goto ERR;
            }
        }
        if (ret != DB_NOTFOUND)
        {
            m_myerrno = 0;
            m_errno = ret;
            goto ERR;
        }
    }
    else if (key_opcode == EXCEPT)//差集
    {
        while ((ret = p_srccursor->get(p_srccursor, &key, &srcdata, DB_NEXT)) == 0)//遍历本类对象所指向的数据库
        {
            ret = p_desdb->get(p_desdb, NULL, &key, &desdata, 0);
            if (ret == 0)//存在
            {
                ret = p_srccursor->del(p_srccursor, 0);
                if (ret != 0)
                {
                    m_myerrno = 0;
                    m_errno = ret;
                    goto ERR;
                }
            }
            else if (ret == DB_NOTFOUND) //不存在
            {
                continue;
            }
            else
            {
                m_myerrno = 0;
                m_errno = ret;
                goto ERR;
            }
        }
        if (ret != DB_NOTFOUND)
        {
            m_myerrno = 0;
            m_errno = ret;
            goto ERR;
        }
    }
    else if (key_opcode == UNION)//并集
    {
        while ((ret = p_cursor->get(p_cursor, &key, &desdata, DB_NEXT)) == 0)//遍历p_file所指向的数据库
        {
            ret = m_pdb->get(m_pdb, NULL, &key, &srcdata, 0);
            if (ret == 0)//存在
            {
                uint32_t bufferlen = MAX_BUFFER;
                if (0 != value_opcode(srcdata.data, srcdata.size, desdata.data, desdata.size, (void *)m_buffer, &bufferlen))
                {
                    m_errno = 0;
                    m_myerrno = 6;
                    goto ERR;
                }
                updata.data = m_buffer;
                updata.size = bufferlen;
                if (0 != (ret = m_pdb->put(m_pdb, NULL, &key, &updata, 0)))
                {
                    m_myerrno = 0;
                    m_errno = ret;
                    goto ERR;
                }
            }
            else if (ret == DB_NOTFOUND) //不存在
            {
                if (0 != (ret = m_pdb->put(m_pdb, NULL, &key, &desdata, 0)))
                {
                    m_myerrno = 0;
                    m_errno = ret;
                    goto ERR;
                }
            }
            else
            {
                m_myerrno = 0;
                m_errno = ret;
                goto ERR;
            }
        }
        if (ret != DB_NOTFOUND)
        {
            m_myerrno = 0;
            m_errno = ret;
            goto ERR;
        }

    }
    else
    {
        m_myerrno = 4;
        m_errno = 0;
        goto ERR;
    }

    //释放资源 用于正确的
    if (p_srccursor != NULL)
    {
        p_srccursor->close(p_srccursor);
    }
    if (p_cursor != NULL)
    {
        p_cursor->close(p_cursor);
    }
    if (p_desdb != NULL)
    {
        p_desdb->close(p_desdb,0);
    }
    return 0;

    //释放资源 用于错误处理
ERR:
    if (p_srccursor != NULL)
    {
        p_srccursor->close(p_srccursor);
    }
    if (p_cursor != NULL)
    {
        p_cursor->close(p_cursor);
    }
    if (p_desdb != NULL)
    {
        p_desdb->close(p_desdb,0);
    }
    return -1;
}

int c_uvalue::merge(const i_uvalue *p_uvalue, key_opcode_t key_opcode, cb_value_opcode_t value_opcode)
{
    return merge(p_uvalue->get_database_name(), key_opcode, value_opcode);
}

//int c_uvalue::merge(i_ucount *p_ucount, key_opcode_t key_opcode)
//{
//    if (m_init == 0)
//    {
//        m_myerrno = 8;
//        m_errno = 0;
//        return -1;
//    }
//    int ret;
//    DBC *p_srccursor = NULL;
//    DBT key,srcdata;
//    memset(&key, 0, sizeof(DBT));
//    memset(&srcdata, 0, sizeof(DBT));
//
//    m_pdb->cursor(m_pdb, NULL, &p_srccursor, 0);
//
//    uint32_t mimiid = 0;
//
//    //处理交集，并集，差集
//    if (key_opcode == INTERSECT)//交集
//    {
//        while ((ret = p_srccursor->get(p_srccursor, &key, &srcdata, DB_NEXT)) == 0) //遍历本类对象说指向的数据库
//        {
//            memcpy(&mimiid, key.data, 4);
//            ret = p_ucount->get(mimiid);
//            if (ret == 1)//存在
//            {
//            }
//            else if (ret == 0) //不存在
//            {
//                ret = p_srccursor->del(p_srccursor, 0);
//                if (ret != 0)
//                {
//                    m_myerrno = 0;
//                    m_errno = ret;
//                    goto ERR;
//                }
//            }
//            else
//            {
//                m_myerrno = 10;
//                m_errno = 0;
//                goto ERR;
//            }
//        }
//        if (ret != DB_NOTFOUND)
//        {
//            m_myerrno = 0;
//            m_errno = ret;
//            goto ERR;
//        }
//    }
//    else if (key_opcode == EXCEPT)//差集
//    {
//        while ((ret = p_srccursor->get(p_srccursor, &key, &srcdata, DB_NEXT)) == 0)//遍历本类对象所指向的数据库
//        {
//            memcpy(&mimiid, key.data, 4);
//            ret = p_ucount->get(mimiid);
//            if (ret == 1)//存在
//            {
//                ret = p_srccursor->del(p_srccursor, 0);
//                if (ret != 0)
//                {
//                    m_myerrno = 0;
//                    m_errno = ret;
//                    goto ERR;
//                }
//            }
//            else if (ret == 0) //不存在
//            {
//                continue;
//            }
//            else
//            {
//                m_myerrno = 10;
//                m_errno = 0;
//                goto ERR;
//            }
//        }
//        if (ret != DB_NOTFOUND)
//        {
//            m_myerrno = 0;
//            m_errno = ret;
//            goto ERR;
//        }
//    }
//    else
//    {
//        m_myerrno = 4;
//        m_errno = 0;
//        goto ERR;
//    }
//
//    //释放资源 用于正确的
//    if (p_srccursor != NULL)
//    {
//        p_srccursor->close(p_srccursor);
//    }
//    return 0;
//
//    //释放资源 用于错误处理
//ERR:
//    if (p_srccursor != NULL)
//    {
//        p_srccursor->close(p_srccursor);
//    }
//    return -1;
//}
int c_uvalue::get_last_errno()
{
    return m_errno;
}

const char* c_uvalue::get_last_errstr()
{
    if (m_myerrno == 1)
    {
        return "set function: value_opcode error";
    }
    //if (m_myerrno == 2)
    //    return "get function: user support buffer is small";
    else if (m_myerrno == 3)
    {
        return "traverse function: cb_traverse error";
    }
    else if (m_myerrno == 4)
    {
        return "merge function: opcode is invalid";
    }
    else if (m_myerrno == 5)
    {
        return "merge function: [INTERSECT] value_opcode error";
    }
    else if (m_myerrno == 6)
    {
        return "merge function: [UNION] value_opcode error";
    }
    else if (m_myerrno == 7)
    {
        return "init have run";
    }
    else if (m_myerrno == 8)
    {
        return "uninit before call";
    }
    else if (m_myerrno == 10)
    {
        return "ucount get_ucount return error";
    }
    return db_strerror(m_errno);
}

int c_uvalue::flush()
{
    int ret;
    if (0 != (ret = m_pdb->sync(m_pdb, 0)))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

const char* c_uvalue::get_database_name() const
{
    return m_database_name;
}

int c_uvalue::uninit()
{
    if (m_pcursor != NULL)
    {
        m_pcursor->close(m_pcursor);
        m_pcursor = NULL;
    }
    if (m_pdb != NULL)
    {
        m_pdb->close(m_pdb, 0);
        m_pdb = NULL;
    }
    if (m_penv != NULL)
    {
        m_penv->close(m_penv, 0);
        m_penv = NULL;
    }
    m_init = 0;
    return 0;
}

int c_uvalue::release()
{
    delete this;
    return 0;
}

