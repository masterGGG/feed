<?php
/* vim: set expandtab tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file function.php
 * @author richard <richard@taomee.com>
 * @date 2011-06-10
 */

require_once('config.php');
require_once('netclient_class.php');

function do_log($level, $log)
{
    static $arr_handle = array();

    if (!array_key_exists($level, $arr_handle) || $arr_handle[$level] == null) {
        $log_file = LOG_DIR . LOG_PREFIX . '_' . $level . '_' . date('Y_m_d') . '.log';
        $arr_handle[$level] = fopen($log_file, 'ab');
        if ($arr_handle[$level] === FALSE) {
            return FALSE;
        }
    }

    if (fwrite($arr_handle[$level], 
               '[' . date('Y-m-d H:i:s') . '] ' . print_r($log, true) . "\n") === FALSE) {
        $arr_handle[$level] = null;
        return FALSE;
    }

    return TRUE;
}

function feedid_to_binary($feedid)
{
    return pack('LSLL', $feedid['user_id'], $feedid['cmd_id'], $feedid['app_id'], $feedid['timestamp']) . base64_decode($feedid['magic']);
}

function binary_to_feedid($binary)
{
    $feedid = unpack("Luser_id/Scmd_id/Lapp_id/Ltimestamp/Ltags", $binary);
    $feedid['magic'] = base64_encode(substr($binary, 14, 8));
    return $feedid;
}

function passive_feedid_to_binary($feedid)
{
    return pack('LSL2', $feedid['user_id'], $feedid['cmd_id'], $feedid['app_id'], $feedid['timestamp']) . base64_decode($feedid['magic']) . pack('L3',$feedid['sender_uid'] , $feedid['target_uid'] , $feedid['passive_magic']);
}

function binary_to_passive_feed_index($binary)
{
    $feedid = unpack("Luser_id/Scmd_id/Lapp_id/Ltimestamp/Lm1/Lm2/Lsender_uid/Ltarget_uid/Lpassive_magic/Lupdate_timestamp", $binary);
    unset($feedid['m1']);
    unset($feedid['m2']);
    $feedid['magic'] = base64_encode(substr($binary, 14, 8));
    return $feedid;
}

function get_passive_feedid($uid)
{
    $arr_feedid = array();

    $arr_outbox_addr = explode(':', constant('OUTBOX_ADDR'));
    $outbox_client = new netclient($arr_outbox_addr[0], $arr_outbox_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        do_log('error', 'get_feedid: outbox_client->open_conn');
        return FALSE;
    }

    $outbox_rqst_len = 4 + 2;
    $outbox_rqst_body = '';
    $outbox_rqst_body .= pack('L', $uid);
    $outbox_rqst_len += 4;

        $outbox_rqst = pack('LS', $outbox_rqst_len, 0xFFF8) . $outbox_rqst_body;
        $outbox_resp = FALSE;
        if (($outbox_resp = $outbox_client->send_rqst($outbox_rqst, TIMEOUT)) === FALSE) {
            do_log('error', 'get_feedid: outbox_client->send_rqst');
            return FALSE;
        }


        $rv = unpack('Llen/Sresult', $outbox_resp);
        if ($rv['result'] != 0) {
            do_log('error', "get_feedid: len: {$rv['len']} result: {$rv['result']}");
            return FALSE;
        }
        
        $feedid_count = ($rv['len'] - 6) / 38;
        for ($j = 0; $j != $feedid_count; ++$j) {
            $binary = substr($outbox_resp, 6 + $j * 38, 38);
            $feedid = binary_to_passive_feed_index($binary);
            $arr_feedid[] = $feedid;
        }

    if ($outbox_client->close_conn() === FALSE) {
        do_log('error', 'get_feedid: outbox_client->close_conn');
        return FALSE;
    }

    return $arr_feedid;
}

function get_feedid_by_tags($mid, $tag, $begin_time, $end_time, $cnt)
{
    $arr_feedid = array();
    $arr_tag_cache_addr = explode(':', constant('TAG_CACHE_ADDR'));
    $outbox_client = new netclient($arr_tag_cache_addr[0], $arr_tag_cache_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        do_log('error', 'get_feedid: outbox_client->open_conn');
        return FALSE;
    }
    $rqst = pack("LLSLL4L", 18+16, 0, 0xA104, 0, $mid, $tag, $begin_time, $end_time, $cnt);
    
    if (($outbox_resp = $outbox_client->send_rqst($rqst, TIMEOUT)) === FALSE) {
            do_log('error', 'get_feedid: tag_server_client->send_rqst');
            return FALSE;
    }
    $rv = unpack('Llen/Lseq/Scmd/Lresult/Luser_id', $outbox_resp);
    if ($rv["result"] != 0) {
            do_log('error', 'get_feedid: tag_server_client->send_rqst code :'.$rv["result"]);
            return FALSE;
    }
    $feedid_count = unpack('Lcnt', substr($outbox_resp, 18));
    $pos = 22;
    for ($j = 0; $j != $feedid_count['cnt']; ++$j) {
        $detail = unpack('Llen', substr($outbox_resp, $pos));
        $pos += 4;
        $binary = base64_decode(substr($outbox_resp, $pos, $detail['len']));
        $pos += $detail['len'];
        $feedid = binary_to_feedid($binary);
        $arr_feedid[] = $feedid;
    }
    if ($outbox_client->close_conn() === FALSE) {
        do_log('error', 'get_feedid: outbox_client->close_conn');
        return FALSE;
    }

    return $arr_feedid;
}

function get_feedid($arr_uid, $is_latest)
{
    $arr_feedid = array();

    $arr_outbox_addr = explode(':', constant('OUTBOX_ADDR'));
    $outbox_client = new netclient($arr_outbox_addr[0], $arr_outbox_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        do_log('error', 'get_feedid: outbox_client->open_conn');
        return FALSE;
    }

    if ($is_latest) {
        if (get_update_mid($arr_uid) === FALSE)
            return FALSE;
    } else if (get_friend_id($arr_uid) === FALSE) {
        return FALSE;
    }
    $uid_count = count($arr_uid);
    for ($i = 0; $i < $uid_count; $i += MAX_OUTBOX_REQUEST_COUNT) {
        $arr_slice_uid = array_slice($arr_uid, $i, MAX_OUTBOX_REQUEST_COUNT);

        $outbox_rqst_len = 4 + 2;
        $outbox_rqst_body = '';
        foreach ($arr_slice_uid as $uid) {
            $outbox_rqst_body .= pack('L', $uid);
            $outbox_rqst_len += 4;
        }

        $outbox_rqst = pack('LS', $outbox_rqst_len, OUTBOX_OPCODE) . $outbox_rqst_body;
        $outbox_resp = FALSE;
        if (($outbox_resp = $outbox_client->send_rqst($outbox_rqst, TIMEOUT)) === FALSE) {
            do_log('error', 'get_feedid: outbox_client->send_rqst');
            return FALSE;
        }

        $rv = unpack('Llen/Sresult', $outbox_resp);
        if ($rv['result'] != 0) {
            do_log('error', "get_feedid: len: {$rv['len']} result: {$rv['result']}");
            return FALSE;
        }

        $feedid_count = ($rv['len'] - OUTBOX_RESPONSE_HEAD_LEN) / FEEDID_LEN;
        for ($j = 0; $j != $feedid_count; ++$j) {
            $binary = substr($outbox_resp, OUTBOX_RESPONSE_HEAD_LEN + $j * FEEDID_LEN, FEEDID_LEN);
            $feedid = binary_to_feedid($binary);
            $arr_feedid[] = $feedid;
        }
    }

    if ($outbox_client->close_conn() === FALSE) {
        do_log('error', 'get_feedid: outbox_client->close_conn');
        return FALSE;
    }

    return $arr_feedid;
}

function feedid_cmp($feedid_1, $feedid_2)
{
    if ($feedid_1['timestamp'] == $feedid_2['timestamp']) {
        return 0;
    }

    return ($feedid_1['timestamp'] < $feedid_2['timestamp']) ? 1 : -1;
}

function passive_feedid_cmp($feedid_1, $feedid_2)
{
    if ($feedid_1['update_timestamp'] == $feedid_2['update_timestamp']) {
        return 0;
    }

    return ($feedid_1['update_timestamp'] < $feedid_2['update_timestamp']) ? 1 : -1;
}

$g_arr_app_id;
$g_arr_cmd_id;
$g_uid;

function feedid_filter($feedid) 
{
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    global $g_uid;
    
    if ($g_arr_app_id !== array()) {
        if (FALSE == in_array($feedid['app_id'], $g_arr_app_id))
        {
            return FALSE;
        }
    }
    if ($g_arr_cmd_id !== array()) {
        if (FALSE == in_array($feedid['cmd_id'], $g_arr_cmd_id))
        {
            return FALSE;
        }
    }
    
    // 根据业务逻辑过滤
    static $last_user_id = 0;                                 // TODO
    static $allow_num_doudou = 0;
    static $allow_num_chongwu = 0;
    static $allow_num_hero = 0;
    static $allow_num_gongfu = 0;
    static $allow_num_seer = 0;
    static $allow_num_hua = 0;
    static $allow_num_mole = 0;
    if ($feedid['user_id'] != $last_user_id) {
        $last_user_id = $feedid['user_id'];

        $allow_num_doudou = 1;
        $allow_num_chongwu = 1;
        $allow_num_hero = 1;
        $allow_num_gongfu = 1;
        $allow_num_seer = 1;
        $allow_num_hua = 1;
        $allow_num_mole = 1;
    }

    if ($feedid['cmd_id'] == 7022 && $feedid['app_id'] == 10002) {
        if ($allow_num_doudou >= 1) {
            $allow_num_doudou--;
        } else {
            return FALSE;
        }
    } else if ($feedid['cmd_id'] == 7022 && $feedid['app_id'] == 10038) {
        if ($allow_num_chongwu >= 1) {
            $allow_num_chongwu--;
        } else {
            return FALSE;
        }
    } else if ($feedid['cmd_id'] == 5001 || $feedid['cmd_id'] == 5002 || 
               $feedid['cmd_id'] == 5003 || $feedid['cmd_id'] == 5004) {
        if ($allow_num_hero >= 1) {
            $allow_num_hero--;
        } else {
            return FALSE;
        }
    } else if ($feedid['cmd_id'] == 4001 || $feedid['cmd_id'] == 4002 || 
               $feedid['cmd_id'] == 4003 || $feedid['cmd_id'] == 4005) {
        if ($allow_num_gongfu >= 1) {
            $allow_num_gongfu--;
        } else {
            return FALSE;
        }
    } else if ($feedid['cmd_id'] == 3001 || $feedid['cmd_id'] == 3002 || 
               $feedid['cmd_id'] == 3003 || $feedid['cmd_id'] == 3004 || $feedid['cmd_id'] == 3005) {
        if ($allow_num_seer >= 1) {
            $allow_num_seer--;
        } else {
            return FALSE;
        }
    } else if ($feedid['cmd_id'] == 2001 || $feedid['cmd_id'] == 2002 || $feedid['cmd_id'] == 2003) {
        if ($allow_num_hua >= 1) {
            $allow_num_hua--;
        } else {
            return FALSE;
        }
    } else if ($feedid['cmd_id'] == 1004 || $feedid['cmd_id'] == 1005) {
        if ($allow_num_mole >= 1) {
            $allow_num_mole--;
        } else {
            return FALSE;
        }
    }
    
    if ($feedid['cmd_id'] == 7018 || $feedid['cmd_id'] == 7019)
    {
        if ($g_uid == $feedid['user_id']) 
        {
            return FALSE;
        }
    }

    return TRUE;
}

function get_passive_feeds($arr_feedid)
{
    $arr_feeds = array();
    
    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'ERROR: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2;
    $storage_rqst_body = '';
    foreach ($arr_feedid as $feedid) {
        $storage_rqst_body .= passive_feedid_to_binary($feedid);
        $storage_rqst_len += 34;
    }

    $storage_rqst = pack('LSS', $storage_rqst_len, 26, count($arr_feedid)) . $storage_rqst_body;
    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', __FUNCTION__ . 'ERROR: storage_client->send_rqst');
        return FALSE;
    }


    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', __FUNCTION__ ."----".__LINE__ ."ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }
    $storage_resp = substr($storage_resp, 4 + 2 + 2);
    
    for ($i = 0; $i != $rv_0['units']; ++$i) {
        $rv_1 = unpack('Llen', $storage_resp);

        $feed = binary_to_passive_feed_index(substr($storage_resp, 4 + 8));

        $temp = unpack('Lid1/Lid2', substr($storage_resp, 4)); 
        $feed['id1'] = $temp['id1'];
        $feed['id2'] = $temp['id2'];

        $feed_data = substr($storage_resp, 4 + 38 + 8, $rv_1['len'] - (4 + 38 + 8));
        $feed['data'] = json_decode($feed_data, TRUE);
        $arr_feeds[] = $feed;
        $storage_resp = substr($storage_resp, $rv_1['len']);
    }

    if ($storage_client->close_conn() === FALSE) {
        do_log('error', 'ERROR: storage_client->close_conn');
        return FALSE;
    }

    return $arr_feeds;
}

function get_feeds($arr_feedid)
{
    $arr_feeds = array();
    
    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'ERROR: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2;
    $storage_rqst_body = '';
    foreach ($arr_feedid as $feedid) {
        $storage_rqst_body .= feedid_to_binary($feedid);
        $storage_rqst_len += FEEDID_LEN;
    }

    $storage_rqst = pack('LSS', $storage_rqst_len, 14, count($arr_feedid)) . $storage_rqst_body;
    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'ERROR: storage_client->send_rqst');
        return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', __FUNCTION__ . "ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }
    $storage_resp = substr($storage_resp, 4 + 2 + 2);

    for ($i = 0; $i != $rv_0['units']; ++$i) {
        $rv_1 = unpack('Llen', $storage_resp);
        $feed = binary_to_feedid(substr($storage_resp, 4 + 8));
        $temp = unpack('Lid1/Lid2', substr($storage_resp, 4)); 
        $feed['id1'] = $temp['id1'];
        $feed['id2'] = $temp['id2'];
        $feed_data = substr($storage_resp, 4 + FEEDID_LEN + 8, $rv_1['len'] - (4 + FEEDID_LEN + 8));
        $feed['data'] = json_decode($feed_data, TRUE);
        $arr_feeds[] = $feed;
        $storage_resp = substr($storage_resp, $rv_1['len']);
    }

    if ($storage_client->close_conn() === FALSE) {
        do_log('error', 'ERROR: storage_client->close_conn');
        return FALSE;
    }

    return $arr_feeds;
}

function del_passive_feed($uid, $app_id, $cmd_id, $timestamp, $magic, $sender_uid, $target_uid, $passive_magic)
{
    $feedid = array('user_id' => $uid,
                    'app_id' => $app_id,
                    'cmd_id' => $cmd_id,
                    'timestamp' => $timestamp,
                    'magic' => $magic,
                    'sender_uid' => $sender_uid,
                    'target_uid' => $target_uid,
                    'passive_magic' => $passive_magic);

    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'del_feed: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2;
    $storage_rqst_body = passive_feedid_to_binary($feedid);
    $storage_rqst_len += strlen($storage_rqst_body);

    $storage_rqst = pack('LSS', $storage_rqst_len, 23, 1) . $storage_rqst_body;
    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'del_feed: storage_client->send_rqst');
        return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', "del_feed: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }

    return TRUE;
}

function del_feed($uid, $app_id, $cmd_id, $timestamp, $magic)
{
    $feedid = array('user_id' => $uid,
                    'app_id' => $app_id,
                    'cmd_id' => $cmd_id,
                    'timestamp' => $timestamp,
                    'magic' => $magic);

    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'del_feed: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2;
    $storage_rqst_body = feedid_to_binary($feedid);
    $storage_rqst_len += strlen($storage_rqst_body);

    $storage_rqst = pack('LSS', $storage_rqst_len, 4, 1) . $storage_rqst_body;
    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'del_feed: storage_client->send_rqst');
        return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', "del_feed: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }

    return TRUE;
}

function arr_cmp($arr1, $arr2)
{
    @sort($arr1);
    @sort($arr2);
    if ($arr1 == $arr2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

function get_timestamp($uid, $type)
{
    if ($type == 'haoyou')
    {
        $arr_time_addr = explode(':', constant('ACTIVE_TIME_ADDR'));
    }
    else if ($type = 'yuwo')
    {
        $arr_time_addr = explode(':', constant('PASSIVE_TIME_ADDR'));
    }
    else
    {
        return NULL;
    }


    $time_client = new netclient($arr_time_addr[0], $arr_time_addr[1]); 
    if ($time_client->open_conn(1) === FALSE) {
        do_log('error', 'del_feed: time_client->open_conn');
        return NULL;
    }

    $time_rqst = pack('L2SL2', 18, 0, 0x2001, 0, $uid);
    $time_resp = FALSE;
    if (($time_resp = $time_client->send_rqst($time_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'del_feed: time_client->send_rqst');
        return NULL;
    }

    $rv_0 = unpack('Llen/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $time_resp);
    if ($rv_0['status_code'] != 0) {
        do_log('error', __FUNCTION__ . "len: {$rv_0['len']} result: {$rv_0['status_code']}");
        return NULL;
    }
    $rv_0 = unpack('Llen/Lseq_num/Scmd_id/Lstatus_code/Luser_id/Ltimestamp', $time_resp);

    return $rv_0['timestamp'];
}

function set_timestamp($uid, $time, $type)
{
    if ($type == 'haoyou')
    {
        $arr_time_addr = explode(':', constant('ACTIVE_TIME_ADDR'));
    }
    else if ($type = 'yuwo')
    {
        $arr_time_addr = explode(':', constant('PASSIVE_TIME_ADDR'));
    }
    else
    {
        return false;
    }

    $time_client = new netclient($arr_time_addr[0], $arr_time_addr[1]); 
    if ($time_client->open_conn(1) === FALSE) {
        do_log('error', 'del_feed: time_client->open_conn');
        return false;
    }

    $time_rqst = pack('L2SL2L', 22, 0, 0x2002, 0, $uid, $time);
    $time_resp = FALSE;
    if (($time_resp = $time_client->send_rqst($time_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'del_feed: time_client->send_rqst');
        return false;
    }

    $rv_0 = unpack('Llen/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $time_resp);
    if ($rv_0['status_code'] != 0) {
        do_log('error', "del_feed: len: {$rv_0['len']} result: {$rv_0['status_code']}");
        return false;
    }

    return true;
}

function get_notice($arr_uid, $item)
{
    $type = explode(',',$item);
    foreach($type as $val)
    {
        if ($val == 'haoyou')
        {
            $arr_feedid = get_feedid($arr_uid, FALSE);
            if ($arr_feedid === FALSE) {
                do_log('error', 'get_newsfeed: get_feedid');
                return FALSE;
            }

            $current_time = get_timestamp($arr_uid[0], 'haoyou');      
            if ($current_time === NULL)
            {
                return false;
            }

             // 对feedid进行排序
            usort($arr_feedid, 'feedid_cmp');

            $uid = array();
            $num = 0;
            foreach($arr_feedid as $val)
            {
                if ($val['timestamp'] > $current_time)
                {
                    if (!in_array($val['user_id'],$uid))
                    {
                        $uid[] = $val['user_id'];
                    }
                    $num++;
                    
                }   
            }
            $rv['haoyou_uid'] = $uid;
            $rv['haoyou'] = $num;
            $rv['haoyou_timestamp'] = $current_time;
        }
        else if ($val == 'yuwo')
        {
            $arr_feedid = get_passive_feedid($arr_uid[0]);
            if ($arr_feedid === FALSE) {
                do_log('error', 'get_newsfeed: get_feedid');
                return FALSE;
            }

            $current_time = get_timestamp($arr_uid[0], 'yuwo');      
            if ($current_time === NULL)
            {
                return false;
            }

             // 对feedid进行排序
            usort($arr_feedid, 'passive_feedid_cmp');

            $uid = array();
            $num = 0;
            foreach($arr_feedid as $val)
            {
                if ($val['update_timestamp'] > $current_time)
                {
                    if (!in_array($val['user_id'],$uid))
                    {
                        $uid[] = $val['user_id'];
                    }
                    $num++;
                    
                }   
            }
            $rv['yuwo_uid'] = $uid;
            $rv['yuwo'] = $num;
            $rv['yuwo_timestamp'] = $current_time;
        }
    }

    if (isset($rv))
    {
        return json_encode($rv);
    }
    else
    {
        return false;
    }
}

function get_passive_newsfeed($uid, $offset, $count, $timestamp)
{
    // 从outbox-server获取所有的feedid
    $arr_feedid = get_passive_feedid($uid);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }

    if (count($arr_feedid) == 0)
    {
        $arr_result = array();
        $arr_result['current_page'] = array();
        $arr_result['have_next'] = 0;
        $arr_result['next_offset'] = $offset + $count;
        return json_encode($arr_result);
    } 
    //获得最后拉取的时间戳
    $up_time = gettimeofday(true);

    if ($offset == 0) {
        $ncount = 0;
        $last_time = get_timestamp($uid, 'yuwo');      
        if ($last_time === NULL) 
            $last_time = $up_time; 
        foreach($arr_feedid as $val) {
            if ($val['timestamp'] <= $last_time) 
                break;
            $ncount++;
        }
        $arr_feedid = array_slice($arr_feedid, $offset, $ncount);
        //do_log('debug', "lastTime:".$last_time." eraase count:".$ncount." reset count:".count($arr_feedid));
    }

    usort($arr_feedid, 'passive_feedid_cmp');
    // 对feedid进行分页
    $total_count = count($arr_feedid);
    if ($offset != -1) { 
        if ($count !== 0) {
            $arr_feedid = array_slice($arr_feedid, $offset, $count);
        } else {
            $arr_feedid = array_slice($arr_feedid, $offset);
        }
    }
    else {
        $arr_feedid = array_slice($arr_feedid, 0, $count);
    }
    // 对feedid进行排序
    if ($offset <= 0) 
        set_timestamp($uid, $up_time, 'yuwo');

    $have_next = 0;
    if ($count !== 0 && $total_count > $count) {
        $have_next = 1;
    }    

    // 从storage-server获取feed内容
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_passive_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    //do_log('debug', print_r($arr_feeds,true));

    usort($arr_feeds, 'passive_feedid_cmp');

    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;
    if ($offset == -1)    
        $arr_result['next_offset'] = $count;
    else
        $arr_result['next_offset'] = $offset + $count;
    return json_encode($arr_result);
}

function get_newsfeed_by_tags($my_id, $arr_app_id, $arr_cmd_id, $count, $begin_time, $end_time, $my_tag) {
    /* 为了支持拉取历史tag，需要记录两个时间，最后一次拉去的时间， 和第一次拉去的时间*/
    $arr_feedid = get_feedid_by_tags($my_id, $my_tag, $begin_time, $end_time, $count);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }
    //do_log('error', __LINE__.'xxx get_newsfeed: last_time: '.print_r($arr_feedid, true));

    // 根据arr_app_id、arr_cmd_id及业务逻辑对feedid进行过滤
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    global $g_uid;
    $g_arr_app_id = $arr_app_id;
    $g_arr_cmd_id = $arr_cmd_id;
    $g_uid = $my_id; 
    $arr_feedid = array_filter($arr_feedid, 'feedid_filter');
    
    // 对过滤后的feedid进行排序
    usort($arr_feedid, 'feedid_cmp');
    $offset = 0; 
    $total_count = count($arr_feedid);
    $arr_feedid = array_slice($arr_feedid, $offset);
    $have_next = 0;
    if ($count !== 0 && $total_count > $offset + $count) {
        $have_next = 1;
    }    
    // 从storage-server获取feed内容
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    usort($arr_feeds, 'feedid_cmp');
    // 判断是否需要折叠, 和一些其他限制
    $i = 0;
    $last_feed = FALSE;
    $arr_join_friend = array();
    
    // 判断是否需要合并
    foreach ($arr_feeds as $key_0 => &$feed_0) {
        unset($feed_0['tags']);
    }
    foreach ($arr_feeds as $key_0 => &$feed_0) {
        if (isset($feed_0['fold'])) {
            continue;
        }
    }

    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;
    $arr_result['next_offset'] = $offset + $total_count;

    return json_encode($arr_result);
}

function get_newsfeed_of_latest($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp) {
    $my_id = $arr_uid[0];
    $arr_feedid = get_feedid($arr_uid, TRUE);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }
//    do_log('error', __LINE__.'xxx get_newsfeed: last_time: '.print_r($arr_uid, true));

    return get_newsfeed_common($my_id, $arr_feedid, $arr_app_id, $arr_cmd_id, $offset, $count);
}

function get_newsfeed($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp) {
    //get mine & my friends' feed id
    $my_id = $arr_uid[0];
    // 从outbox-server获取所有的feedid
    $arr_feedid = get_feedid($arr_uid, FALSE);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }
    //get_newsfeed_filter_feedid();
    return get_newsfeed_common($my_id, $arr_feedid, $arr_app_id, $arr_cmd_id, $offset, $count);
}

function get_newsfeed_common($my_id, $arr_feedid, $arr_app_id, $arr_cmd_id, $offset, $count)
{
   //获得最后拉取的时间戳
    $up_time = gettimeofday(true); 
    if ($offset == -1) 
        $last_time = $up_time;
    else {
        $last_time = get_timestamp($my_id, 'haoyou');      
        if ($last_time === NULL) 
            $last_time = gettimeofday(true); 
    }

    $new_offset = $offset == -1 ? 0 : $offset; 
    if ($offset == 0) {
        $news_count = count($arr_feedid);
        for($i = 0; $i < $news_count; ++$i) {
            if ($arr_feedid[$i]['timestamp'] <= $last_time) 
                unset($arr_feedid[$i]);
        }
    }

    // 根据arr_app_id、arr_cmd_id及业务逻辑对feedid进行过滤
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    global $g_uid;
    $g_arr_app_id = $arr_app_id;
    $g_arr_cmd_id = $arr_cmd_id;
    $g_uid = $my_id; 
    $arr_feedid = array_filter($arr_feedid, 'feedid_filter');
    
    // 对过滤后的feedid进行排序
    usort($arr_feedid, 'feedid_cmp');
     
    // 按页切分
    $total_count = count($arr_feedid);
    if ($count !== 0) {
        $arr_feedid = array_slice($arr_feedid, $new_offset, $count);
    } else {
        $arr_feedid = array_slice($arr_feedid, $new_offset);
    }

    $next_offset = count($arr_feedid);
    if ($offset <= 1) {
        set_timestamp($my_id, $up_time, 'haoyou');
    } else
        $next_offset += $offset;

    $have_next = 0;
    if ($count !== 0 && $total_count > $new_offset + $count) {
        $have_next = 1;
    }    

    // 从storage-server获取feed内容
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    usort($arr_feeds, 'feedid_cmp');
    //do_log('storage-server', $arr_feeds);

    // 判断是否需要折叠, 和一些其他限制
    $i = 0;
    $last_feed = FALSE;
    $arr_join_friend = array();
    
    // 判断是否需要合并
    foreach ($arr_feeds as $key_0 => &$feed_0) {
        unset($feed_0['tags']);
    }
    foreach ($arr_feeds as $key_0 => &$feed_0) {
        if (isset($feed_0['fold'])) {
            continue;
        }
    }

    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;
    $arr_result['next_offset'] = $next_offset;

    return json_encode($arr_result);
}

function get_class_newsfeed($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp)
{
    // 从outbox-server获取所有的feedid

    
    $arr_feedid = get_feedid($arr_uid, FALSE);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }

    // 根据arr_app_id、arr_cmd_id及业务逻辑对feedid进行过滤
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    global $g_uid;
    $g_arr_app_id = $arr_app_id;
    $g_arr_cmd_id = $arr_cmd_id;
    $g_uid = ''; 
    $arr_feedid = array_filter($arr_feedid, 'feedid_filter');

    // 对过滤后的feedid进行排序
    usort($arr_feedid, 'feedid_cmp');
     
    // 按页切分
    $total_count = count($arr_feedid);
    if ($count !== 0) {
        $arr_feedid = array_slice($arr_feedid, $offset, $count);
    } else {
        $arr_feedid = array_slice($arr_feedid, $offset);
    }
    
    if ($offset != 0)
    {
        foreach($arr_feedid as $val)
        {
            if ($val['timestamp'] >= $timestamp)
            {
                array_shift($arr_feedid);
            }
        }
    }

    $have_next = 0;
    if ($count !== 0 && $total_count > $offset + $count) {
        $have_next = 1;
    }    

    // 从storage-server获取feed内容
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    usort($arr_feeds, 'feedid_cmp');

    //do_log('storage-server', $arr_feeds);

    // 判断是否需要折叠, 和一些其他限制
    $i = 0;
    $last_feed = FALSE;
    $arr_join_friend = array();
    foreach ($arr_feeds as $key => &$feed) {
        //加好友里的限制
        
        if ($feed['cmd_id'] == 7002 && $feed['app_id'] == 7002)
        {
            //do_log('bbb',print_r($feed,true));
            if (count($feed['data']['target_uid'] == 1)) //自己的两个好友相互加的情况
            {
                $flag_join_friend = 0;
                foreach ($arr_join_friend as $temp_key => $temp_feed)
                {
                    if ($feed['user_id'] == $temp_feed['data']['target_uid'][0] && $temp_feed['user_id'] == $feed['data']['target_uid'][0])
                    {
                        unset($arr_feeds[$key]);
                        $flag_join_friend = 1;
                        break;
                    }
                }
                if ($flag_join_friend == 0)
                {
                $arr_join_friend[] = $feed;
                }
            }

            unset($last_feed);
            $last_feed = FALSE;
            continue;
        }
        else if ($feed['cmd_id'] == 7017 && $feed['app_id'] == 141)
        {
            unset($last_feed);
            $last_feed = FALSE;
            continue;
        }

        if ($last_feed !== FALSE && 
            $feed['user_id'] == $last_feed['user_id'] &&
            $feed['app_id'] == $last_feed['app_id'] &&
            $feed['cmd_id'] == $last_feed['cmd_id']) {     // TODO
            $last_feed['fold'] = $i;
            $feed['fold'] = $i;
        } else {
            $i++;
        }

        unset($last_feed);
        $last_feed = &$feed;
    }

    // 判断是否需要合并
    foreach ($arr_feeds as $key_0 => &$feed_0) {
        if (isset($feed_0['fold'])) {
            continue;
        }

        if ($feed_0['app_id'] == 7002 && $feed_0['cmd_id'] == 7002) {          // 加好友
            // 在$key_0之前进行查找
            foreach ($arr_feeds as $key_1 => $feed_1) {
                if ($key_1 == $key_0) {
                    break;
                }

                if ($feed_1['app_id'] == $feed_0['app_id'] && 
                    $feed_1['cmd_id'] == $feed_0['cmd_id'] && 
                    arr_cmp($feed_1['data']['target_uid'],$feed_0['data']['target_uid'])) {
                    // 合并
                    if (is_array($feed_1['user_id'])) {
                        array_push($feed_1['user_id'], $feed_0['user_id']);
                    } else {
                        $feed_1['user_id'] = array($feed_1['user_id'], $feed_0['user_id']);
                    }
                    unset($arr_feeds[$key_0]);
                    break;
                }
            }
        }
        else if ($feed_0['app_id'] == 141 && $feed_0['cmd_id'] == 7017) {       //收集徽章
            // 在$key_0之前进行查找
            foreach ($arr_feeds as $key_1 => $feed_1) {
                if ($key_1 == $key_0) {
                    break;
                }

                if ($feed_1['app_id'] == $feed_0['app_id'] && 
                    $feed_1['cmd_id'] == $feed_0['cmd_id'] && 
                    arr_cmp($feed_1['data']['badge_id'],$feed_0['data']['badge_id'])) {
                    // 合并
                    if (is_array($feed_1['user_id'])) {
                        array_push($feed_1['user_id'], $feed_0['user_id']);
                    } else {
                        $feed_1['user_id'] = array($feed_1['user_id'], $feed_0['user_id']);
                    }
                    unset($arr_feeds[$key_0]);
                    break;
                }
            }
        }
    }

    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;
    $arr_result['next_offset'] = $offset + $count;

    return json_encode($arr_result);
}

function get_following_feedid($uid)
{
    $arr_feedid = array();

    $arr_relation_addr = explode(':', constant('RELATION_ADDR'));
    
    $relation_client = new netclient($arr_relation_addr[0], $arr_relation_addr[1]); 
    if ($relation_client->open_conn(1) === FALSE) {
        do_log('error', 'get_following_feed: time_client->open_conn');
        return NULL;
    }

    $relation_rqst_len = 14;
    $relation_rqst_body = '';
    $relation_rqst_body .= pack('L', $uid);
    $relation_rqst_len += 4;

    $relation_rqst = pack('LLSL', $relation_rqst_len, rand(), 0xB102, 0).$relation_rqst_body;
    $relation_resp = FALSE;

    if (($relation_resp = $relation_client->send_rqst($relation_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'get attention failed: relation_client->send_rqst');
        return FALSE;
    }

    $res_header = unpack('Llen/Lseq/Scmd_id/Lresult/Lmid', $relation_resp);
    if ($res_header['result'] != 0) {
        do_log('error', "get attention feedid: len: {$res_header['len']} result: {$res_header['result']}");
        return FALSE;
    }

    $feedid_count = ($res_header['len'] - 22) / 8;
    for ($j = 0; $j != $feedid_count; ++$j) {
        $binary = substr($relation_resp, 22 + $j * 8, 8);
        $feedid = unpack('Luser_id/Ltimestamp', $binary);
        $arr_feedid[] = $feedid;
    }
    
    if ($relation_client->close_conn() === FALSE) {
        do_log('error', 'get_attenion_feedid: relation_client->close_conn');
        return FALSE;
    }

//    do_log('error', 'get_attenion_feedid: relation_client->close_conn'.print_r($arr_feedid, true));
    return $arr_feedid;
}

function get_friend_id(&$arr_uid)
{
    $arr_redis_addr = explode(':', constant('RELATION_ADDR'));
    $redis_cli = new netclient($arr_redis_addr[0], $arr_redis_addr[1]);
    
    if ($redis_cli->open_conn(1) === FALSE){
        do_log('error', 'get_feedid: relation_client->open_conn');
        return FALSE;
    }
    
    $relation_rqst = pack("LLsLLLLL", 18+4+4+4, 0, 0xB102, 0, $arr_uid[0], $arr_uid[0], 0, 0);
    $relation_resp = FALSE;
    
    if (($relation_resp = $redis_cli->send_rqst($relation_rqst,TIMEOUT)) === FALSE) {
        do_log('error', 'get_friend_id: relation_client->send_rqst');
        return FALSE;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $relation_resp);
    if ($rv['code'] != 0) {
        do_log('error', 'get_friend_id: relation_client internal err');
        return FALSE;
    }
    $relation_resp = substr($relation_resp, 18);
    $rv = unpack("Lunits", $relation_resp);
    $id_list = substr($relation_resp, 4);
    //do_log('debug', '['.__LINE__.'] len: ' . strlen($relation_resp).'  count :'.$rv['units']);
    
    for ($i = 0; $i < $rv['units']; ++$i) {
        $fan = unpack('Lid/Ltime', $id_list);
        array_push($arr_uid, $fan['id']);
        $id_list = substr($id_list, 8);
    }
    return TRUE;
}

function get_update_mid(&$arr_uid) { //,$count) {
    $arr_redis_addr = explode(':', constant('TAG_CACHE_ADDR'));
    $redis_cli = new netclient($arr_redis_addr[0], $arr_redis_addr[1]);
    
    if ($redis_cli->open_conn(1) === FALSE){
        do_log('error', 'get_feedid: relation_client->open_conn');
        return FALSE;
    }
    
    $timestamp = gettimeofday(true);
    $relation_rqst = pack("LLsLLLLL", 18+4+4+4, 10086, 0xA102, 0, $arr_uid[0], 0, $timestamp, 10); //$count);
    $relation_resp = FALSE;
    
    if (($relation_resp = $redis_cli->send_rqst($relation_rqst,TIMEOUT)) === FALSE) {
        do_log('error', 'get_fans_id: relation_client->send_rqst');
        return FALSE;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $relation_resp);
    if ($rv['code'] != 0) {
        do_log('error', 'get_fans_id: relation_client internal err');
        return FALSE;
    }
    $relation_resp = substr($relation_resp, 18);
    $rv = unpack("Lunits", $relation_resp);
    $id_list = substr($relation_resp, 4);

    unset($arr_uid[0]);
    for ($i = 0; $i < $rv['units']; ++$i) {
        $fan = unpack('Lid', $id_list);
        array_push($arr_uid, $fan['id']);
        $id_list = substr($id_list, 4);
    }
    return TRUE;
}
