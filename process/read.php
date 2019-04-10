#!/usr/bin/php5
<?php
require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'config' . DIRECTORY_SEPARATOR .  'setup.inc.php';

declare(ticks=1);

sleep(1);
$g_pid_file = "./outbox_pid";

ini_set("display_errors", "0");
ini_set("log_errors", "1");
ini_set("error_log", "log/mysys.log");
date_default_timezone_set("Asia/Shanghai");

if (init_connect_and_nonblock(FEED_IP, FEED_PORT, $feed_socket))
{
    log::write("init_feed_connect feed_server fail", "warn");
    $feed_reconnect_flag = 1;
}
else
{
    $feed_reconnect_flag = 0;
}

if (init_connect(STORAGE_IP, STORAGE_PORT, $storage_server_socket))
{
    log::write("init_connect storage_server fail reason: connect to storage_server", "error");
    return -1;
}


$fd = fopen($argv[1],"r");
$pack = fread($fd, filesize($argv[1]));
$feed = unpack("Slen/Scmd_id/Luser_id/Cversion/Ltimestamp/Lapp_id",$pack);
var_dump($feed);
$feed["data"] = substr($pack, 13, $feed["len"] - 13);
if (array_key_exists($feed["cmd_id"], $g_sys_conf['feed']['app_id'])) //判断是否是老版本不带app_id的协议包
{
    $feed["data"] = substr($pack, 17, $feed["len"] - 13);
}
else
{
    $feed["data"] = substr($pack, 17, $feed["len"] - 17);
}    

$input_arr = array("feed_socket" => $feed_socket,
    "feed_reconnect_flag" => $feed_reconnect_flag,
    "storage_server_socket" => $storage_server_socket
);
$output_arr = array();
if ($ret = feedhandle($feed, $input_arr, &$output_arr))
{
    if (-1 == $ret)
    {
        log::write("[feed_handle]feed process error fatal error", "error");
        //return -1;
    }
    else if (-2 == $ret) //表明这个feed没有拉取到数据或者一些警告错误 可以直接跳过
    {
       // continue;
    }
    else
    {
        log::write("[feed_handle]feed process error error:unvalid return value{$ret}", "error");
        //return -1;
    }
}    
//sleep(600);
?>
