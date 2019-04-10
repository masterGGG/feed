#! /usr/bin/php5
<?php
 require_once "../config/setup.inc.php";

 if (init_connect_and_nonblock(STORAGE_IP, STORAGE_PORT, $storage_server_socket))
 {
     log::write("init_connect storage_server fail reason: connect to storage_server", "error");
     return -1;
 }

 if (init_connect_and_nonblock(OUTBOX_IP, OUTBOX_PORT, $outbox_server_socket))
 {
     log::write("init_feed_connect outbox_server fail", "warn");
     $outbox_reconnect_flag = 1;
 }
 else
 {
     $outbox_reconnect_flag = 0;
 }
 $output_arr = array();                                                   
 $input_arr = array("storage_server_socket" => $storage_server_socket
 );                                                                       

// $completefeed = array(
//     'user_id' => 10134,
//     'cmd_id' => 1111,
//     'timestamp' => 1308240000,
//     'app_id' => 500,
//     'magic1' => 1,
//     'magic2' => 1,
//     'sender_uid' => 35,
//     'target_uid' => 36,
//     'passive_magic' => 0,
//     'update_timestamp' => 1308240002,
//     'data' => '{"nick_name":"4399"}'
// );
// $ret = operate_passive_feed_to_db($storage_server_socket, NULL, $completefeed);
//
// unset($comfeed);                                                               
// $completefeed = array(                                                         
//     'user_id' => 10134,                                                            
//     'cmd_id' => 1111,                                                              
//     'timestamp' => 1308240000,                                                     
//     'app_id' => 500,                                                               
//     'magic1' => 1,                                                                 
//     'magic2' => 1,                                                                 
//     'data' => '{"nick_name":"4399"}'                                               
// );                                                                             
// $ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed);    
//
// $output_arr = array();
// $input_arr = array("storage_server_socket" => $storage_server_socket
// );
//
//$pack_body = pack("SSLCL4S", 59, 1, 10134, 1, 1308240000, 500, 1,1, 1111);
//$verify = md5($pack_body . DELETE_SECRET_KEY); 
//$pack = $pack_body . $verify;
//$ret = feeddelete($pack, $input_arr, &$output_arr);
//echo $ret;
//var_dump($output_arr);
//
//$pack_body = pack("SSLCL4SLL", 67, 2, 35, 1, 1308240000, 500, 1,1, 1111, 10134, 36); 
//$verify = md5($pack_body . DELETE_SECRET_KEY); 
//$pack = $pack_body . $verify;
//$ret = feeddelete($pack, $input_arr, &$output_arr);
//echo $ret;
//var_dump($output_arr);
/*

unset($comfeed);                                                               
$completefeed = array(                                                         
    'user_id' => 10134,                                                            
    'cmd_id' => 7007,                                                              
    'timestamp' => 1308240000,                                                     
    'app_id' => 500,                                                               
    'magic1' => 1,                                                                 
    'magic2' => 1,                                                                 
    'data' => '{"nick_name":"4399"}'                                               
);                                                                             
$ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed);    
echo "**************evaluate*******************";
$pack = pack("SSLCL4SLC", 32, 201, 35, 1, 1308240000, 500, 1,1, 7007, 10134, 130); 
$ret = feed_update_sync($pack, $input_arr, &$output_arr);
echo $ret;
var_dump($output_arr);

$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'app_id' => 500,
'timestamp' => 1308240000,
'magic1' => 1,
'magic2' => 1,
);
$ret = get_feed($storage_server_socket, $feed, $feeds);
var_dump($feeds);
echo "****************************************";

echo "**************comment*******************";
$pack = pack("SSLCL4SLCa430L2a430L3", 912, 202, 8880161, 1, 0, 500, 1,1, 7007, 10134, 0, '', 44, 0, '评论下',1308250000, 55, 50439); 
$ret = feed_update_sync($pack, $input_arr, &$output_arr);
echo $ret . "\n";
var_dump($output_arr);
$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'app_id' => 500,
'timestamp' => 0,
'magic1' => 1,
'magic2' => 1,
);
$ret = get_feed($storage_server_socket, $feed, $feeds);
echo $ret;
var_dump($feeds);

$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 8880161,
'target_uid' => 50439,
'passive_magic' => 0,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
$ret = get_passive_feed($storage_server_socket, $feed, $feeds);
echo $ret;
var_dump($feeds);
echo "****************************************";
*/
//$pack = pack("SSLCL4SLCa430L2a430L3", 912, 202, 123, 1, 0, 500, 1,1, 7007, 10134, 1, '评论下', 55, 1308250000, '回复下',1308260003, 56, 321); 
// $feed = unpack('Slen/Sopt_id/Lsender_uid/Cversion/Ltimestamp/Lapp_id/Lmagic1/Lmagic2/Scmd_id/Luser_id/Cflag/a430source_content/Lsource_id/Lsource_timestamp/a430comment_content/Lcomment_timestamp/Lcomment_id/Ltarget_uid', $pack);
//$ret = passive_feed_handler($storage_server_socket, $feed, false, $ret_val);
//echo "*****\n";
//echo $ret;
//var_dump($ret_val);

/*
$pack = pack("SSLCL4SLCa430L2a430L3", 912, 202, 50439, 1, 0, 500, 1,1, 7007, 10134, 1, '评论下', 55, 1308250000, '回复下',1308260000, 56, 8880161); 
$ret = feed_update_sync($pack, $input_arr, &$output_arr);
echo $ret;
var_dump($output_arr);

$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'app_id' => 500,
'timestamp' => 0,
'magic1' => 1,
'magic2' => 1,
);
$ret = get_feed($storage_server_socket, $feed, $feeds);
echo $ret;
var_dump($feeds);

$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 8880161,
'target_uid' => 50439,
'passive_magic' => 0,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
$ret = get_passive_feed($storage_server_socket, $feed, $feeds);
echo $ret;
var_dump($feeds);

$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 50439,
'target_uid' => 8880161,
'passive_magic' => 0,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
$ret = get_passive_feed($storage_server_socket, $feed, $feeds);
echo $ret;
var_dump($feeds);

$pack = pack("SSLCL4SLCa430L2a430L3", 912, 202, 8880161, 1, 0, 500, 1,1, 7007, 10134, 0, '', 0, 0, '评论下2',1308270000, 57, 50439); 
$ret = feed_update_sync($pack, $input_arr, &$output_arr);
echo $ret;
var_dump($output_arr);

$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'app_id' => 500,
'timestamp' => 0,
'magic1' => 1,
'magic2' => 1,
);
$ret = get_feed($storage_server_socket, $feed, $feeds);
echo $ret;
var_dump($feeds);

$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 8880161,
'target_uid' => 50439,
'passive_magic' => 0,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
$ret = get_passive_feed($storage_server_socket, $feed, $feeds);
echo $ret;
var_dump($feeds);

$feed = array(
'user_id' => 10134,
'cmd_id' => 7007,
'timestamp' => 1308240000,
'app_id' => 500,
'magic1' => 1,
'magic2' => 1,
'sender_uid' => 50439,
'target_uid' => 8880161,
'passive_magic' => 0,
'update_timestamp' => 1308240002,
'data' => '{"nick_name":"4399"}'
);
$ret = get_passive_feed($storage_server_socket, $feed, $feeds);
echo $ret;
var_dump($feeds);
*/

?>
