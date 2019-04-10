#! /usr/bin/php5
<?php
require_once "../config/setup.inc.php";
require_once "../process.php";

$storage_ip = '10.1.1.44';
$storage_port = '2222';



if (init_connect($storage_ip, $storage_port, $storage_server_socket))
{
    log::write("init_connect fail reason: connect to storage_server", "error");
    return -1;
}   

/*
//insert 
unset($comfeed);
$completefeed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 35,
'target_uid' => 36,
'passive_magic' => 2,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
$ret = operate_passive_feed_to_db($storage_server_socket, NULL, $completefeed);
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


////update
$completefeed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 35,
'target_uid' => 36,
'passive_magic' => 2,
'update_timestamp' => 1309240002,
'data' => '{"nick_name":"7899"}'
);
$comfeed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 35,
'target_uid' => 36,
'passive_magic' => 2,
'update_timestamp' => 1308240025,
'data' => '{"nick_name":"4399","sdfsdf":"334w34"}'
);

$ret = operate_passive_feed_to_db($storage_server_socket, $comfeed, $completefeed);
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
    echo $ret . "1.operate_feed_to_db success\n";
}


$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 35,
'target_uid' => 36,
'passive_magic' => 2,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
if (false == delete_passive_feed($storage_server_socket, $feed))
{    
    echo 'delete ' . " error\n";
    return -1;
}    
else 
{
    echo "2. delete feed success\n";
}

$feed = array(
'user_id' => 4830036,
'cmd_id' => 7007,
'app_id' => 139,
'timestamp' => 1302600403,
'magic1' => 0,
'magic2' => 0,
);
if (0 != ($ret = get_feed($storage_server_socket, $feed, $feeds)))
{    
    echo $ret. " error\n";
    return -1;
}    
echo "3. ";
var_dump($feeds);

$feed = array(
'user_id' => 222500036,
'cmd_id' => 3003,
'app_id' => 146,
'timestamp' => 1302582121,
'magic1' => 0,
'magic2' => 0,
);
if (0 != get_feed($storage_server_socket, $feed, $feeds))
{    
    echo __FUNCTION__ . " error\n";
    return -1;
}    
echo "4. ";
var_dump($feeds);

$completefeed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 35,
'target_uid' => 36,
'passive_magic' => 2,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
$ret = operate_passive_feed_to_db($storage_server_socket, NULL, $completefeed);
*/
$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 35,
'target_uid' => 36,
'passive_magic' => 2,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
if (0 != ($ret = get_passive_feed($storage_server_socket, $feed, $feeds)))
{    
    echo $ret . " error\n";
    return -1;
}    
var_dump($feeds);

/*
$feed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 35,
'target_uid' => 36,
'passive_magic' => 2,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
delete_passive_feed($storage_server_socket, $feed);
echo "5. ";
var_dump($feeds);
*/

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
    array(
        'feed_op' => 'insert',
        'type' => 'passive',
        'insert_feedid' => array(
            'user_id' => 10134,
            'cmd_id' => 1111,
            'timestamp' => 1308240000,
            'app_id' => 500,
            'magic1' => 1,
            'magic2' => 1,
            'sender_uid' => 35,
            'target_uid' => 36,
            'passive_magic' => 2,
            'update_timestamp' => 1308240000
        ) 
    ),
    array(
        'feed_op' => 'update',
        'type' => 'passive',
        'update_new_feedid' => array(
            'user_id' => 10134,
            'cmd_id' => 1111,
            'timestamp' => 1308240000,
            'app_id' => 500,
            'magic1' => 1,
            'magic2' => 1,
            'sender_uid' => 35,
            'target_uid' => 36,
            'passive_magic' => 2,
            'update_timestamp' => 0
        ),
        'update_old_feedid' => array(
            'user_id' => 10134,
            'cmd_id' => 1111,
            'timestamp' => 1308240000,
            'app_id' => 500,
            'magic1' => 1,
            'magic2' => 1,
            'sender_uid' => 35,
            'target_uid' => 36,
            'passive_magic' => 3,
            'update_timestamp' => 1308240011
        )
    ),
    array(
        'feed_op' => 'delete',
        'type' => 'passive',
        'delete_feedid' => array(
            'user_id' => 10134,
            'cmd_id' => 1111,
            'timestamp' => 1308240000,
            'app_id' => 500,
            'magic1' => 1,
            'magic2' => 1,
            'sender_uid' => 35,
            'target_uid' => 36,
            'passive_magic' => 3,
            'update_timestamp' => 1308240011
        ) 
    )
);

if (init_connect_and_nonblock(OUTBOX_IP, OUTBOX_PORT, $outbox_server_socket))
{
    log::write("init_feed_connect outbox_server fail", "warn");
    $outbox_reconnect_flag = 1;
}
else
{
    $outbox_reconnect_flag = 0;
}

$ret = outbox_handle($outbox_server_socket,$outbox_reconnect_flag,$output_arr);
echo $ret;

?>
