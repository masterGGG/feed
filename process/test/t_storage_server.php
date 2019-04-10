#! /usr/bin/php5
<?php
require_once "../config/setup.inc.php";

$storage_ip = '10.1.1.44';
$storage_port = '2222';


if (init_connect($storage_ip, $storage_port, $storage_server_socket))
{
    log::write("init_connect fail reason: connect to storage_server", "error");
    return -1;
}   

/*
*/
$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308239982,
'app_id' => 504,
'prev_num' => 1,
'next_num' => 0,
);
if (false == get_feed_according_time($storage_server_socket, $feed['user_id'], $feed['cmd_id'],$feed['timestamp'], $feed['app_id'], $feed['prev_num'], $feed['next_num'], $result_feed))
{    
    echo __FUNCTION__ . " error\n";
    return -1;
}    
var_dump($result_feed);


$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 504,
);
if (false == get_comfeed($storage_server_socket, $feed['user_id'], $feed['cmd_id'], $feed['timestamp'], $feed['app_id'], $feeds))
{    
    echo __FUNCTION__ . " error\n";
    return -1;
}    
var_dump($feeds);

$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 504,
);
$reply_key = array(
'target_appid' => 1001,
'target_uid' => 50439,
'target_id' => 1 
);
if (false == get_comfeed_for_news_reply($storage_server_socket, $feed["user_id"], $feed["cmd_id"], $feed["timestamp"], $feed['app_id'], $reply_key,  $comfeed))
{    
    echo __FUNCTION__ . " error\n";
    return -1;
}    
var_dump($comfeed);

$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'app_id' => 500,
);
$begin_time = 1308240000;
$end_time = 1308240000;
if (false == get_comfeed_time_span($storage_server_socket, $feed["user_id"], $feed["cmd_id"], $begin_time, $end_time, $feed['app_id'], $comfeed))
{    
    echo __FUNCTION__ . " error\n";
    return -1;
}    
var_dump($comfeed);

$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
);
if (false == delete_feed_user_defined($storage_server_socket, $feed))
{    
    echo __FUNCTION__ . " error\n";
    return -1;
}    
else
{
    echo "del success\n";
}

$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
);
if (false == delete_feed($storage_server_socket, $feed))
{    
    echo __FUNCTION__ . " error\n";
    return -1;
}    
else 
{
    echo "delete feed success\n";
}


//insert 

unset($comfeed);
$completefeed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'data' => '{"nick_name":"4399"}'
);
$ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed);
if ($ret == 1)
{
    echo $ret . " dup\n";
    return -2;
}
else if ($ret != 0)
{
    echo $ret . " error\n";
    return -1;
}
else
{
    echo $ret . " success\n";
}

//update
$completefeed = array(
'user_id' => 10134,
'cmd_id' => 1121,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'data' => '{"nick_name":"4399-\u6ce1\u6ce1"}'
);
$comfeed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
);

$ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed);
if ($ret == 1)
{
    echo $ret . "operate_feed_to_db dup\n";
    return -2;
}
else if ($ret != 0)
{
    echo $ret . "operate_feed_to_db error\n";
    return -1;
}
else
{
    echo $ret . "operate_feed_to_db success\n";
}

/*
//outbox_server
$outbox_ip = '10.1.1.44';
$outbox_port = '43321';
if (init_connect($outbox_ip, $outbox_port, $outbox_server_socket))
{   
    log::write("init_feed_connect fail", "warn");
    return -1;
}   

//update

$output_arr = array(
    'update_new_feedid' => array(
        'user_id' => 3333,
        'cmd_id' => 1025,
        'timestamp' => 1308240001,
        'app_id' => 2,
        'magic1' => 1,
        'magic2' => 1
    ),
    'update_old_feedid' => array(
        'user_id' => 3333,
        'cmd_id' => 1001,
        'timestamp' => 1308240000,
        'app_id' => 1,
        'magic1' => 1,
        'magic2' => 1
    )
);
$pack = pack("LSLSL4LSL4", 50, 0xfff1, $output_arr['update_new_feedid']['user_id'],    
    $output_arr['update_new_feedid']['cmd_id'],  $output_arr['update_new_feedid']['app_id'],    
    $output_arr['update_new_feedid']['timestamp'], $output_arr['update_new_feedid']['magic1'],    
    $output_arr['update_new_feedid']['magic2'], $output_arr['update_old_feedid']['user_id'],                   
    $output_arr['update_old_feedid']['cmd_id'],  $output_arr['update_old_feedid']['app_id'],    
    $output_arr['update_old_feedid']['timestamp'], $output_arr['update_old_feedid']['magic1'],                 
    $output_arr['update_old_feedid']['magic2']);

if (send_data_and_nonblock($outbox_server_socket, $pack, 2))
{   
    log::write("send data to outbox_server fail", "warn");
}   
$pack = "";
if (recv_data_and_nonblock($outbox_server_socket, 6, $pack, 2))
{   
    log::write("recv data from outbox_server fail", "warn");
}   

$temp = unpack("Llen/Sresult", $pack);
var_dump($temp);

//insert
$output_arr = array(
    'insert_feedid' => array(
        'user_id' => 1234,
        'cmd_id' => 1001,
        'timestamp' => 1308240000,
        'app_id' => 1,
        'magic1' => 1,
        'magic2' => 1
    )
);
$pack = pack("LSLSL4", 28, 0xfff2, $output_arr['insert_feedid']['user_id'],$output_arr['insert_feedid']['cmd_id'], $output_arr['insert_feedid']['app_id'], $output_arr['insert_feedid']['timestamp'],$output_arr['insert_feedid']['magic1'], $output_arr['insert_feedid']['magic2']);

if (send_data_and_nonblock($outbox_server_socket, $pack, 2))
{
    log::write("send data to outbox_server fail", "warn");
}
$pack = "";
if (recv_data_and_nonblock($outbox_server_socket, 6, $pack, 2))
{
    log::write("recv data from outbox_server fail", "warn");
}
$temp = unpack("Llen/Sresult", $pack);
var_dump($temp);
*/
?>
