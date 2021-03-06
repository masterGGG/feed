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

    $feedid = new feedid();
    $feedid->mimi = 38743981;
    $feedid->cmd_id = 7006;
    $feedid->version = 1;
    $feedid->timestamp = time();
    $feedid_binary = $feedid->to_binary();

    $aid = 600;
    $amid = 1227401110;

    $f_arr_1 = pack('S',25).$feedid_binary.pack('LLL', 7004, $aid, $amid);

    $rqst_msg = $f_arr_1;
    $resp_msg = FALSE;
    if (($resp_msg = $client->send_rqst($rqst_msg, 5)) === FALSE) {
        do_log('error', 'ERROR: client->send_rqst');
        return FALSE;
    }

if ($client->close_conn() === FALSE) {
    do_log('error', 'ERROR: client->close_conn');
    return FALSE;
}
