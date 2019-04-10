#! /usr/bin/php5
<?php
require dirname(__FILE__) . DIRECTORY_SEPARATOR . '../config' . DIRECTORY_SEPARATOR .  'setup.inc.php';

$g_db_handler = new dbhandler(); 
echo "db_config_init\n";
if (!$g_db_handler->db_config_init(DB_HOST, DB_USER, DB_PASSWD, DB_PORT))
{
    echo("db_config_init  [fail]\n");
}
/*
echo "get_comfeed\n";
$user_id = 1;
$cmd_id = 7002;
$timestamp = strtotime('20110324010023');
if (!$g_db_handler->get_comfeed($user_id, $cmd_id, $timestamp, $feed))
{
    echo("get_comfeed  [fail]\n");
}
var_dump($feed);

echo "get_feed\n";
$user_id = 1;
$cmd_id = 7001;
$timestamp = strtotime('20110324020000');
if (!$g_db_handler->get_feed($user_id, $cmd_id, $timestamp, $feed))
{
    echo("get_comfeed  [fail]\n");
}
var_dump($feed);

echo "operate_feed_to_db\n";
$feed["user_id"] = 1;
$feed["nick_name"] = "测试2";
$feed["cmd_id"] = 7001;
$feed["timestamp"] = strtotime('20110324010022'); 
$feed["data"] = '{"level":888}';
if (!$g_db_handler->operate_feed_to_db($key, $feed))
{
    echo("operate_feed_to_db insert [fail]\n");
}
$key["user_id"] = 1;
$key["nick_name"] = "测试2";
$key["cmd_id"] = 7001;
$key["timestamp"] = strtotime('20110324010022'); 
$key["data"] = '{"level":888}';
$feed["user_id"] = 1;
$feed["nick_name"] = "测试xxx";
$feed["cmd_id"] = 7001;
$feed["timestamp"] = strtotime('20110324010022'); 
$feed["data"] = '{"level":2888}';
if (!$g_db_handler->operate_feed_to_db($key, $feed))
{
    echo("operate_feed_to_db update [fail]\n");
}


echo "get_confeed_for_news_reply\n";
$feed = "";
$user_id = 1;
$cmd_id = 7009;
$timestamp = strtotime('20110324020000');
$reply_key["target_appid"] = 1;
$reply_key["target_uid"] = 1;
$reply_key["target_id"] = 1;
if (!$g_db_handler->get_comfeed_for_news_reply($user_id, $cmd_id, $timestamp, $reply_key, $feed))
{
    echo("get_comfeed  [fail]\n");
}
var_dump($feed);
*/
 $feed["cmd_id"] = 7018;
 $feed["user_id"] = 50439;
 $feed["timestamp"] = strtotime("20110324020103");
 $feed["data"] = pack("L2",50439,1);
var_dump($feed);
feedhandle($feed,$outbox);
var_dump($outbox);

//news_friend($feed, $comfeed, $completefeed);
//var_dump($completefeed);
?>
