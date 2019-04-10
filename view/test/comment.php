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

//for ($i = 1; $i != 10; ++$i) {
    $feedid = new feedid();
    $feedid->mimi = 1227401114;
    $feedid->cmd_id = 7005;
    $feedid->version = 1;
    $feedid->timestamp = time();
    $feedid_binary = $feedid->to_binary();

    $aid = 1022222;
    $amid = 1227401112;
    $cmid = 1227401113;
    $cid = 10;
    
    $f_arr_1 = pack('S',37).$feedid_binary.pack('LLLLLL', 7005, 0, $aid, $amid, $cmid, $cid);

    print("total buffer:{$f_arr_1}\n");
    $f_unpack_ret = unpack('Slen/Scmd/Lmid/Cversion/Ltime/Lappid/Ltype/Larticle_id/Lauthor_id/LcommentMimi/LcommentID',$f_arr_1);
    var_dump($f_unpack_ret);
    $rqst_msg = $f_arr_1;
    $resp_msg = FALSE;
   if (($resp_msg = $client->send_rqst($rqst_msg, 5)) === FALSE) {
        do_log('error', 'ERROR: client->send_rqst');
        return FALSE;
    }
//}

if ($client->close_conn() === FALSE) {
    do_log('error', 'ERROR: client->close_conn');
    return FALSE;
}
