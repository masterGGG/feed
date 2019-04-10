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

?>
