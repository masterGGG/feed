/* vim: set tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file i_uvalue.h
 * @author richard <richard@taomee.com>
 * @date 2010-09-06
 */

#ifndef I_UVALUE_H_2010_09_06
#define I_UVALUE_H_2010_09_06

#include <stdint.h>          // for uint32_t

/**
 * @brief i_uvalue 唯一值接口
 */
struct i_uvalue
{
public:
	/**
	 * @brief key_opcode_t 键的操作码
	 */
	typedef enum key_opcode {
		INTERSECT,             /**< 交集 */
		EXCEPT,                /**< 差集 */
		UNION                  /**< 合集 */
	} key_opcode_t;

	/**
	 * @brief 对value进行操作的回调函数
	 * @param p_value_data1
	 * @param p_value_size1
	 * @param p_value_data2
	 * @param p_value_size2
	 * @param p_result_value
	 * @param p_result_size
	 * @return 成功返回0，失败返回-1
	 */
	typedef int (*cb_value_opcode_t)(const void *p_value_data1, uint32_t value_size1,
				                     const void *p_value_data2, uint32_t value_size2,
									 void *p_result_value, uint32_t *p_result_value_size);

	/**
	 * @brief init_flag 初始化标记
	 */
	typedef enum init_flag {
		CREATE = 1 << 0,     /**< 文件不存在时创建文件，文件存在时打开文件 */
		EXCL   = 1 << 1,     /**< EXCL和CREATE一起使用时才有意义。当文件存在时指定EXCL则出错 */
		BTREE  = 1 << 2,     /**< BTRR存储方式 */
		HASH   = 1 << 3      /**< HASH存储方式 */
	} init_flag_t;

	/**
	 * @brief btree key比较函数
	 */
	typedef int (*cb_bt_compare_t)(const void *p_key_data1, uint32_t key_size1,
				                   const void *p_key_data2, uint32_t key_size2);

	/**
	 * @brief 初始化接口实例
	 * @param file 文件名
	 * @param flags 文件创建标记(CREATE、EXCL的组合)
     * @param mode 文件创建模式(创建新文件时有效)
	 * @param cb_bt_compare btree key比较函数(当flags参数指定BTREE时有效)，当cb_bt_compare为NULL时，
	                        使用默认的基于字典序的比较
	 * @return 成功返回0，失败返回-1
	 */
	virtual int init(const char *p_file, uint32_t flags, int mode, cb_bt_compare_t cb_bt_compare) = 0;

	/**
	 * @brief 插入key/value。如果key已存在，则操作失败
	 * @param p_key_data 指向key的指针
	 * @param key_size key的大小(字节)
	 * @param p_value_data 指向value的指针
	 * @param p_value_size value的大小(字节)
	 * @return 成功返回0，失败返回-1
	 */
	virtual int insert(const void *p_key_data, uint32_t key_size,
				       const void *p_value_data, uint32_t value_size) = 0;

	/**
	 * @brief 更新key/value。如果key不存在，则操作失败
	 * @param p_key_data 指向key的指针
	 * @param key_size key的大小(字节)
	 * @param p_value_data 指向value的指针
	 * @param p_value_size value的大小(字节)
	 * @param value_opcode 对value的操作的回调函数，如果value_opcode等于NULL，直接更新
	 * @return 成功返回0，失败返回-1
	 */
	virtual int update(const void *p_key_data, uint32_t key_size,
				       const void *p_value_data, uint32_t value_size,
				       cb_value_opcode_t value_opcode) = 0;

	/**
	 * @brief 置key/value。如果key不存在，则插入
	 * @param p_key_data 指向key的指针
	 * @param key_size key的大小(字节)
	 * @param p_value_data 指向value的指针
	 * @param p_value_size value的大小(字节)
	 * @param value_opcode 对value的操作的回调函数，如果value_opcode等于NULL，直接更新
	 * @return 成功返回0，失败返回-1
	 */
	virtual int set(const void *p_key_data, uint32_t key_size,
				    const void *p_value_data, uint32_t value_size,
				    cb_value_opcode_t value_opcode) = 0;

	/**
	 * @brief 获取key/value。如果key不存在，则失败
	                         如果p_buffer等于NULL并且p_buffer_size等于NULL，则只是查询key是否存在
							 如果p_buffer等于NULL并且p_buffer_size不等于NULL，则p_buffer_size返回value的大小
							 如果p_buffer不等于NULL并且p_buffer_size不等于NULL，则p_buffer_size输入p_buffer
							 的大小，p_buffer返回value的值，p_buffer_size返回value的大小
	 * @param p_key_data 指向key的指针
	 * @param key_size key的大小(字节)
	 * @param p_buffer 指向buffer的指针
	 * @param p_buffer_size buffer的大小(字节)(输入输出)
	 * @return 成功返回0，失败返回-1
	 */
	virtual int get(const void *p_key_data, uint32_t key_size, void *p_buffer, uint32_t *p_buffer_size) = 0;

	/**
	 * @brief 删除key/value。如果key不存在，则失败
	 * @param p_key_data 指向key的指针
	 * @param key_size key的大小(字节)
	 * @return 成功返回0，失败返回-1
	 */
	virtual int del(const void *p_key_data, uint32_t key_size) = 0;

	/**
	 * @brief 获得key的个数
	 * @param p_count 返回key的个数
	 * @param skip_zero 是否跳过value为0的key 1:跳过 0:不跳过
	 * @return 成功返回0，失败返回-1
	 */
	virtual int get_key_count(uint32_t *p_count, int skip_zero) = 0;

	/**
	 * @brief 遍历key/value时的回调函数
	 * @param p_key_data 指向key的指针
	 * @param p_key_size key的大小(字节)
	 * @param p_value_data 指向value的指针
	 * @param p_value_size value的大小(字节)
	 * @param p_user 用户数据
	 * @return 成功返回0，失败返回-1(当返回-1时，则停止遍历)
	 */
	typedef int (*cb_traverse_t)(const void *p_key_data, uint32_t key_size,
				                 const void *p_value_data, uint32_t value_size,
								 void *p_user);

	/**
	 * @brief 遍历key/value
	 * @param cb_traverse 回调函数
	 * @param p_user 用户数据
	 * @return 成功返回0，失败返回-1
	 */
	virtual int traverse(cb_traverse_t cb_traverse, void *p_user) = 0;

	/**
	 * @brief 合并  合并后的文件就是本身的文件
	 * @param file 要合并的文件
	 * @param key_opcode key操作码
	 * @param value_opcode value操作码
	 * @return 成功返回0，失败返回-1
	 */
	virtual int merge(const char *p_file, key_opcode_t key_opcode, cb_value_opcode_t value_opcode) = 0;

	/**
	 * @brief 合并  合并后的文件就是本身的文件
	 * @param p_uvalue 要合并的uvalue类
	 * @param key_opcode key操作码
	 * @param value_opcode value操作码
	 * @return 成功返回0，失败返回-1
	 */
    virtual int merge(const i_uvalue *p_uvalue, key_opcode_t key_opcode, cb_value_opcode_t value_opcode) = 0;

	/**
	 * @brief 获得最后一次错误的错误码
	 * @return 返回相应的错误码，没有错误时返回0
	 */
	virtual int get_last_errno() = 0;

	/**
	 * @brief 获得最后一次错误的错误描述
	 * @return 错误描述，没有错误时返回NULL
	 */

	virtual const char* get_last_errstr() = 0;
    /**
     * @brief 把内存中的数据写到硬盘里
     * @return 错误描述， 没有错误返回0，有错误返回-1
     */
    virtual int flush() = 0;

    /**
     * @brief 返回数据库名字
     * @return 数据库名字
     *
     */
    virtual const char* get_database_name() const = 0 ;

	/**
	 * @brief 反初始化实例
	 * @return 成功返回0，失败返回-1
	 */
	virtual int uninit() = 0;

	/**
	 * @brief 释放实例
	 * @return 成功返回0，失败返回-1
	 */
	virtual int release() = 0;
};

int create_uvalue_instance(i_uvalue **pp_instance);

#endif /* I_UVALUE_H_2010_09_06 */

