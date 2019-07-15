#!/usr/bin/php
<?php

error_reporting(E_ALL);

require_once('../function.php');
require_once('./feedid_class.php');
require_once('../netclient_class.php');

$client = new netclient('10.1.1.197', 58810);

if ($client->open_conn(1) === FALSE) {
    do_log('error', 'ERROR=> client->connect');
    return FALSE;
}

$aList = array(
    array("aid"=>654, "mimi"=>188888888, "url"=>"http://img.mifan.61.com/g3/M00/00/10/Ch5kAlzVNuaAC3WBAALBtYQ_9V8763_256x256.png", "time"=>1557477094, "tag"=>1),
    array("aid"=>655, "mimi"=>188888888, "url"=>"http://img.mifan.61.com/g3/M00/00/10/Ch5kAlzVOuCAJqaAAADFnnyFzXg123_192x256.jpg", "time"=>1557478112, "tag"=>1),
    array("aid"=>656, "mimi"=>188888888, "url"=>"http://img.mifan.61.com/g3/M00/00/10/Ch5kAlzVPXWAEfwIAABjah-b0bc417_209x256.jpg", "time"=>1557478773, "tag"=>1),
    array("aid"=>657, "mimi"=>188888888, "url"=>"http://img.mifan.61.com/g3/M00/00/10/Ch5kAlzVP-WAfDcSAADH_BRNuWo532_256x143.jpg", "time"=>1557479398, "tag"=>1),
    array("aid"=>658, "mimi"=>38743982, "url"=>"http://img.mifan.61.com/g3/M00/00/10/Ch5kAlzVS4iATvYJAAZHsx0fc7Y090_231x256.png", "time"=>1557482740, "tag"=>1),
    array("aid"=>659, "mimi"=>188888888, "url"=>"http://img.mifan.61.com/g3/M00/00/10/Ch5kAlzVTPOAO45UAABboohhQmI068_209x256.jpg", "time"=>1557482386, "tag"=>3),
    array("aid"=>660, "mimi"=>908892223, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzYzWCAQZ83AAQQKxEz_mU941_193x256.png", "time"=>1557712224, "tag"=>1),
    array("aid"=>661, "mimi"=>188888888, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzZG_6AIlmeAAiQGhV5oys196_192x256.png", "time"=>1557732350, "tag"=>1),
    array("aid"=>662, "mimi"=>908892223, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzaJ0-APcABAAY2jnDaYbk491_256x192.png", "time"=>1557800783, "tag"=>3),
    array("aid"=>663, "mimi"=>188888888, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzaKiGAOlXCAABPuZlhZ3o867_206x256.jpg", "time"=>1557801505, "tag"=>1),
    array("aid"=>664, "mimi"=>188888888, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzaK9SAMn0vAAA6ceI0EyA654_103x115.jpg", "time"=>1557801940, "tag"=>1),
    array("aid"=>665, "mimi"=>38743982, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzb69qAWy8AAAM9Ojz0LwQ773_144x256.png", "time"=>1557916635, "tag"=>2),
    array("aid"=>666, "mimi"=>83764149, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzdFeSAUGzFAA_U9IEI5k0552_120x120.gif", "time"=>1557992935, "tag"=>4),
    array("aid"=>667, "mimi"=>1206205, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzdQ66AHs8jAASgBKqIL2U045_256x149.png", "time"=>1558004656, "tag"=>1),
    array("aid"=>668, "mimi"=>38743982, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzdR0SAJfViAAeRcAjGur4500_125x70.gif", "time"=>1558005572, "tag"=>2),
    array("aid"=>669, "mimi"=>31102528, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzdYQeAeVdbAARcS_uhUqo966_256x144.png", "time"=>1558012167, "tag"=>1),
    array("aid"=>670, "mimi"=>31102528, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzdYX6APsu3AAPWB4GXNQg913_256x144.png", "time"=>1558012286, "tag"=>1),
    array("aid"=>671, "mimi"=>31102528, "url"=>"http://img.mifan.61.com/g3/M00/00/11/Ch5kAlzdgnqAT_07AAPyFepVDsM572_144x256.png", "time"=>1558020736, "tag"=>1),
    array("aid"=>672, "mimi"=>31102528, "url"=>"http://img.mifan.61.com/g3/M00/00/12/Ch5kAlzdhkmAPXRZAACY-gLgzp8085_144x256.png", "time"=>1558021705, "tag"=>3),
    array("aid"=>673, "mimi"=>890118, "url"=>"http://img.mifan.61.com/g3/M00/00/12/Ch5kAlzeVh6AfXoMAArRQQKt6FY219_192x256.png", "time"=>1558074911, "tag"=>5),
    array("aid"=>674, "mimi"=>382854116, "url"=>"http://img.mifan.61.com/g3/M00/00/12/Ch5kAlzeW_GAGDG1AAOLRiZUwr8433_256x147.png", "time"=>1558076402, "tag"=>5),
    array("aid"=>675, "mimi"=>83764149, "url"=>"http://img.mifan.61.com/g3/M00/00/12/Ch5kAlzeX7iAFGOrAAEykczFwB8903_256x242.png", "time"=>1558077369, "tag"=>1),
    array("aid"=>676, "mimi"=>38743982, "url"=>"http://img.mifan.61.com/g3/M00/00/12/Ch5kAlzeZDiAJpIwAAXCYHx_vN0773_200x256.png", "time"=>1558078520, "tag"=>2),
    array("aid"=>677, "mimi"=>14415033, "url"=>"http://img.mifan.61.com/g3/M00/00/12/Ch5kAlzegMKANl14AAP6F4YF9bI476_256x197.png", "time"=>1558085827, "tag"=>1),
    array("aid"=>678, "mimi"=>382854116, "url"=>"http://img.mifan.61.com/g7/M00/00/00/Ch6qB1ziSs-AF6AzAACz1gQC9Io109_256x192.jpg", "time"=>1558334159, "tag"=>4),
    array("aid"=>679, "mimi"=>38743982, "url"=>"http://img.mifan.61.com/g14/M00/00/00/Ch6qDlziTk6AXxrbAAKQlqCaAdQ839_256x170.jpg", "time"=>1558335054, "tag"=>5)
);

for ($i = 0; $i != count($aList); ++$i) {
    $feedid = new feedid();
    $feedid->mimi = $aList[$i]['mimi'];
    $feedid->cmd_id = 7003;
    $feedid->version = 1;
    $feedid->timestamp = $aList[$i]['time'];
    $feedid_binary = $feedid->to_binary();

    $aid = $aList[$i]['aid'];
    $pic = $aList[$i]['url'];
    $len = 2 + 2 + 4 + 1 + 4 + 4  + 4  + 1 + strlen($pic) + 4 + 4;
    
    $f_arr_1 = pack('S',$len).$feedid_binary.pack('LLc', 1, $aid, strlen($pic)).$pic.pack('LL', 1, $aList[$i]['tag']);
    $f_unpack_ret = unpack('Slen/Scmd/Lmid/Cversion/Ltime/Lapp_id/La_id/cilen',$f_arr_1);
    $f_unpack_ret['url'] = substr($f_arr_1, 22, $f_unpack_ret['ilen']);

    /*
    var_dump($f_unpack_ret);
    */
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

?>
