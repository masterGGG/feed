<?php
/* vim: set expandtab tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file config.php
 * @author richard <richard@taomee.com>
 * @date 2011-06-10
 */

error_reporting(E_ALL);
ini_set('display_errors', '0');
ini_set('log_errors', '1');
ini_set('error_log', './log/mysys.log');
ini_set('memory_limit', '2048M');
set_time_limit(0);
date_default_timezone_set('Asia/Shanghai');

// 日志相关配置
define('LOG_DIR', './log/');                               //日志目录
define('LOG_PREFIX', 'view');                              //日志文件前缀

// 统计相关配置
define('STAT_LOG_PATHNAME', '/opt/taomee/stat/spool/inbox/newsfeed_view.statlog');
define('HIT_MEM_MSGID',     0x0D000201);
define('HIT_DB_MSGID',      0x0D000202);
define('HIT_NOTHING_MSGID', 0x0D000203);

// feedid相关配置
define('FEEDID_LEN', 22);

// outbox-server相关配置
define('OUTBOX_ADDR', '10.1.1.197:43321');
define('OUTBOX_OPCODE', 0xFFF4);
define('MAX_OUTBOX_REQUEST_COUNT', 100);
define('OUTBOX_RESPONSE_HEAD_LEN', 6);

// storage-server相关配置
define('STORAGE_ADDR', '10.1.1.197:2222');
define('STORAGE_OPCODE', 10);

define('ACTIVE_TIME_ADDR', '10.1.1.197:12004');
define('PASSIVE_TIME_ADDR', '10.1.1.197:12005');

define('ANALYSIS', false);

define('TIMEOUT', 5);

// relation-server相关配置
define('RELATION_ADDR', '127.0.0.1:21145');
