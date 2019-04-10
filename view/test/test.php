#!/usr/bin/php
<?php
/* vim: set expandtab tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file test.php
 * @author richard <richard@taomee.com>
 * @date 2011-06-10
 */
error_reporting(E_ALL);

require_once('../function.php');
require_once('./feedid_class.php');
require_once('../netclient_class.php');

$client = new netclient('10.1.1.197', 58810);

if ($client->open_conn(1) === FALSE) {
    do_log('error', 'ERROR: client->connect');
    return FALSE;
}

//for ($i = 0; $i != 10000; ++$i) {
    $feedid = new feedid();
    $feedid->mimi = 123451;
    //$feedid->cmd_id = 7000;
    $feedid->cmd_id = 7001;
    $feedid->version = 1;
    $feedid->timestamp = time();
    $feedid_binary = $feedid->to_binary();
    //$attentionid = get_following_feedid($feedid->mimi);
    var_dump($attentionid);
    //$f_arr_1 = pack('S',29).$feedid_binary.pack('LLLL',111,1234,5678,1);
    $f_arr_1 = pack('S',17).$feedid_binary.pack('L', 15000);
    //$f_arr_1 = pack('S',13).$feedid_binary.pack('L', 1);
    //$f_arr_1 = pack('S',17).$feedid_binary.pack('L',1);
    //$f_arr_1 = pack('S',13).$feedid_binary.pack('L',1);
    $f_unpack_ret = unpack('Sret0/Sret1/Lret2/Cret3/Lret4/Lret5',$f_arr_1);
    var_dump($f_unpack_ret);
    $rqst_msg = $f_arr_1;
    $resp_msg = FALSE;
    if (($resp_msg = $client->send_rqst($rqst_msg, 5)) === FALSE) {
        do_log('error', 'ERROR: client->send_rqst');
        return FALSE;
    }
    echo 1111;
    var_dump($resp_msg);
    $feedid->from_binary(substr($resp_msg, 4));
    var_dump($feedid);
//}

if ($client->close_conn() === FALSE) {
    do_log('error', 'ERROR: client->close_conn');
    return FALSE;
}
