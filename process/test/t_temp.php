#! /usr/bin/php5
<?php
require_once "../config/setup.inc.php";


if (init_connect_and_nonblock(FEED_IP, FEED_PORT, $feed_socket))
{   
    log::write("init_feed_connect fail", "warn"); 
    $feed_reconnect_flag = 1;
}   
else
{   
    $feed_reconnect_flag = 0;
}

if (init_connect(STORAGE_IP, STORAGE_PORT, $storage_server_socket))
{   
    log::write("init_connect fail reason: connect to storage_server", "error");
    return -1;
}   

//delete
$feed = array(
"len" => 59,
"opt_id" => 1,
"user_id" => 8880484,
"version" => 1,
"timestamp" => 1308102958,
"app_id" => 1234,
'magic1' => 0,
'magic2' => 1,
'cmd_id' => 7007,
);
$pack = pack("SSLCLLL2S",$feed['len'],$feed['opt_id'],$feed['user_id'],$feed['version'],$feed['timestamp'],$feed['app_id'],$feed['magic1'],$feed['magic2'],$feed['cmd_id']);
$verify = md5($pack.DELETE_SECRET_KEY);
//echo "*";
//echo $pack.DELETE_SECRET_KEY;
//echo "*";
$pack .= $verify;
//echo strlen($pack);

//echo $verify;
//echo "\n";

$feed = unpack("Slen/Scmd_id/Luser_id/Cversion/Ltimestamp", $pack); 
$feed["data"] = substr($pack, 13, $feed["len"] - 13);               
if ($feed["cmd_id"] == 0xffff)  // 表明无feed消息到达  
{                                                      
    sleep(2);                                          
    continue;                                          
}                                                      

//log::write(print_r($feed,true));                     
if ($feed['cmd_id'] == 1) //删除feed                   
{
     $input_arr = array("storage_server_socket" => $storage_server_socket
     );  
     if ($ret = feeddelete($pack, $input_arr))
     {   
         if (-1 == $ret)
         {   
             log::write("[feed delete]delete feed fail", "error");
             return -1; 
         }   
         else
         {   
             log::write("[feed delete]feed process error error:unvalid return value{$ret}", "error");
             return -1; 
         }   
     }   
}
echo $ret . "\n";
//normal feed input
/*
do
{
$feed = array(
"len" => 29,
"cmd_id" => 7007,
"user_id" => 8880484,
"version" => 1,
"timestamp" => 1308102958,
"app_id" => 1234,
);
$feed['data'] = pack("L2L",0,1,14);

if (is_feed_valid($feed))
{
    continue;
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
        return -1;
    }
    else if (-2 == $ret) //表明这个feed没有拉取到数据或者一些警告错误 可以直接跳过
    {
        continue;
    }
    else
    {
        log::write("[feed_handle]feed process error error:unvalid return value{$ret}", "error");
        return -1;
    }
}

log::write("process user_id:".$outbox["user_id"]);
}while(0);
*/
echo "program end\n";
