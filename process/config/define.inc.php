<?php
//基础文件路径
if (!defined('DS'))  		define('DS', 			DIRECTORY_SEPARATOR) ;
if (!defined('ROOT')) 		define('ROOT', 			dirname(dirname(__FILE__)) . DS);
if (!defined('LIB_DIR')) 	define('LIB_DIR',			ROOT. 'lib'. DS);
if (!defined('HANDLER_DIR'))	define('HANDLER_DIR',	ROOT. 'handler' . DS);
if (!defined('CONFIG_DIR')) 	define('CONFIG_DIR',	ROOT. 'config' . DS);

// 日志相关的常量设置
if (!defined('LOG_PATH')) define('LOG_PATH', ROOT . 'log'. DS );
if (!defined('LOG_FILE_SIZE')) define('LOG_FILE_SIZE', 1048576 * 5); // 5M

// 进程数量
if (!defined('PROCESS_NUM')) define('PROCESS_NUM', '4');

// cache server 的地址
if (!defined('CACHE_IP')) define('CACHE_IP', '0.0.0.0');
if (!defined('CACHE_PORT')) define('CACHE_PORT', '58810');

if (!defined('FEED_IP')) define('FEED_IP', '0.0.0.0');
if (!defined('FEED_PORT')) define('FEED_PORT', '58802');

define('RELATION_IP', '0.0.0.0');
define('RELATION_PORT', '21145');

define('TAG_CACHE_IP', '0.0.0.0');
define('TAG_CACHE_PORT', '22080');

define('STORAGE_IP', '0.0.0.0');
define('STORAGE_PORT', '2222');

define('TIMEOUT', 5);

define('OUTBOX_IP', '0.0.0.0');
define('OUTBOX_PORT', '43321');

define('DEBUG', true);

define('DELETE_SECRET_KEY', 'xiaoba7564'); //删除所需要密钥

/**
 * 以下为消息队列相关配置
 */
define('KAFKA_DEBUG_FILE', '../log/kafka.debug');
define('KAFKA_ERROR_FILE', '../log/kafka.error');
define('KAFKA_BROKER_IP', '10.1.1.187');
define('KAFKA_TOPIC_ACK', 'request.required.acks');
define('KAFKA_TOPIC_ACK_VALUE', 0);

define('KAFKA_NOTE_ARTICLE_OPTION', 'protobuf');
define('KAFKA_NOTE_UPDATE_SCORE', 'mifan-note-update-score');
/**
 * 以下为消息格式，默认都包含发起者 uid 和时间戳：
 */
// 米饭
define('NEWS_ARTICLE',                   7003);          // array($target_uid)
define('NEWS_LIKER',                   7004);          // array($target_uid)
define('NEWS_COMMENT',                   7005);
define('NEWS_FANS',                      7006);          // 关注协议

//修改feed协议
define('CLICK_ARTICLE',                 101);          // 点击阅读文章协议
define('PASSIVE_MESSAGE', 22024);

$g_sys_conf["feed"]["operator"] = array(
     NEWS_ARTICLE                => 'news_article',
     NEWS_LIKER                  => 'news_liker',
     NEWS_COMMENT                => 'news_comment',
     NEWS_FANS                   => 'news_fans',

     CLICK_ARTICLE              => 'click_article',
 );

//用于验证feed的是否合法 可变长度配0
$g_sys_conf["feed"]["valid_len"] = array(
     NEWS_ARTICLE                => 0,
     NEWS_LIKER                  => 25,
     NEWS_COMMENT                => 37,
     NEWS_FANS                   => 21,
     CLICK_ARTICLE               => 25,     //17+3*4
 );

//为了和老的协议兼容
$g_sys_conf['feed']['app_id'] = array(
);

$g_sys_conf["feed"]["iscombine"] = array(
);

$g_sys_conf["feed"]["user_defined_id"] = array(
    NEWS_ARTICLE   => true,
);

//被动feed流注册
$g_sys_conf["feed"]["ispassive"] = array(
    NEWS_LIKER   => true,
    NEWS_COMMENT    => true,
    NEWS_FANS       => true,
);

$g_sys_conf["feed"]["support_delete"] = array(
    NEWS_ARTICLE      => 'delete_article',
);

$g_sys_cong['feed']['no_feed'] = array(
    CLICK_ARTICLE    => true,
);
?>
