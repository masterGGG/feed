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

$article = array(
    array("mimi"=>1227401110, "icon"=>"spring.jpg", 'name'=>'春天', 'text'=>'播种','pic'=>'aaaaaaaaaa.png'),
    array("mimi"=>1227401111, "icon"=>"summer.png", 'name'=>'夏天', 'text'=>'避暑','pic'=>'bbbbbbbbbbbbbbbbbbbbbbbbbbbbb.png'),
    array("mimi"=>1227401112, "icon"=>"autumn.gif", 'name'=>'秋天', 'text'=>'收获','pic'=>'ccccccccccc.gif'),
    array("mimi"=>1227401113, "icon"=>"winter.jpg", 'name'=>'冬天', 'text'=>'吃饺子','pic'=>'dddddddddddddddddddddddddddddddddddddddddddddd.jpg')
);
for ($i = 0; $i != 1; ++$i) {
    $feedid = new feedid();
    $feedid->mimi = 1227401110 + $i;
    $feedid->cmd_id = 7003;
    $feedid->version = 1;
    $feedid->timestamp = time();
    $feedid_binary = $feedid->to_binary();

    $aid = 150001 + $i;
    $pic = $article[$i]['icon'];
    $len = 2 + 2 + 4 + 1 + 4 + 4  + 4  + 1 + strlen($pic) + 4 + 4*3;
    
    $f_arr_1 = pack('S',$len).$feedid_binary.pack('LLc', 7003, $aid, strlen($pic)).$pic.pack('LLLL', 3, $i, 10+$i, 100+$i);
    $f_unpack_ret = unpack('Slen/Scmd/Lmid/Cversion/Ltime/Lapp_id/La_id/cilen',$f_arr_1);

    var_dump($f_unpack_ret);
    $rqst_msg = $f_arr_1;
    $resp_msg = FALSE;
   if (($resp_msg = $client->send_rqst($rqst_msg, 5)) === FALSE) {
        do_log('error', 'ERROR=> client->send_rqst');
        return FALSE;
    }
}

if ($client->close_conn() === FALSE) {
    do_log('error', 'ERROR=> cliet->close_conn');
    return FALSE;
}
