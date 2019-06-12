#!/usr/bin/php
<?php
/* vim=> set expandtab tabstop=4 softtabstop=4 shiftwidth=4=> */
/**
 * @file test.php
 * @author richard <richard@taomee.com>
 * @date 2011-06-10
class feedid
{
	public $mimi;
	public $cmd_id;
	public $app_id;
	public $version;
	public $timestamp;

	public function to_binary()
	{
		$ret = pack('SLCL', $this->cmd_id, $this->mimi, $this->version, $this->timestamp);
		return $ret;
	}
}
 */
error_reporting(E_ALL);

require_once('../function.php');
require_once('./feedid_class.php');
require_once('../netclient_class.php');

$client = new netclient('10.1.1.197', 58810);

if ($client->open_conn(1) === FALSE) {
    do_log('error', 'ERROR=> client->connect');
    return FALSE;
}

    $feedid = new feedid();
    $feedid->mimi = 1227401110; //188888888;
    $feedid->cmd_id = 1;
    $feedid->version = 1;
    $feedid->timestamp = 1560221101;
    $feedid_binary = $feedid->to_binary();

    $aid = 610;
    $len = 23;
    
    $f_arr_1 = pack('S',$len).$feedid_binary.pack('LSL', 1, 7003, $aid);
    $f_unpack_ret = unpack('Slen/Scmd/Lmid/Cversion/Ltime/Lapp_id/Scmd_id/Larticle_id',$f_arr_1);

    var_dump($f_unpack_ret);
    var_dump(strlen($f_arr_1));
    $rqst_msg = $f_arr_1;
    $resp_msg = FALSE;
   if (($resp_msg = $client->send_rqst($rqst_msg, 5)) === FALSE) {
        do_log('error', 'ERROR=> client->send_rqst');
        return FALSE;
    }

if ($client->close_conn() === FALSE) {
    do_log('error', 'ERROR=> cliet->close_conn');
    return FALSE;
}
