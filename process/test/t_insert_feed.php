#! /usr/bin/php5
<?php
//insert 
require_once "../config/setup.inc.php";

$storage_ip = '10.1.1.44';
$storage_port = '2222';


if (init_connect($storage_ip, $storage_port, $storage_server_socket))
{
    log::write("init_connect fail reason: connect to storage_server", "error");
    return -1;
}

/*
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
*/
unset($comfeed);
$completefeed = array(
'user_id' => 10134,
'cmd_id' => 1111,
'timestamp' => 1308240000,
'app_id' => 504,
'magic1' => 1,
'magic2' => 1,
'data' => '{"target_appid":1001,"target_uid":50439,"target_id":1,"comment_id":[111111],"timestamp":['.     strtotime("20110324020103").'],"comment_content":["\u56de\u590d\u5b59\u5f20\u60a6\uff1a\u544a\u8bc9\u6211\u554a\uff0c\u5feb\u544a\u8bc9\u6211\uff0c\u4ec0\u4e48\u7384\u673a\u554a\uff1f"],"target":{"type":"diary","timestamp":1302235394}}' 
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
?>
