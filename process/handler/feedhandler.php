<?php
function data_log($feed)
{
//        clearstatcache();
//        $str_log_file = './log/data'.posix_getpid().'.log';
//        $str_log = pack("SSLCLL",$feed["len"],$feed["cmd_id"],$feed["user_id"],$feed["version"],$feed["timestamp"],$feed["app_id"]);
//        $str_log .= $feed["data"];
//        $handle = fopen($str_log_file, 'ab');
//        if ($handle) {
//                fwrite($handle,$str_log);
//                fclose($handle) ;
//                return true;
//        }
        return false;
}

function data_log_pre($feed,$pre)
{
//        clearstatcache();
//        $str_log_file = './log/'.$pre.'_data'.posix_getpid().'.log';
//        $str_log = pack("SSLCLL",$feed["len"],$feed["cmd_id"],$feed["user_id"],$feed["version"],$feed["timestamp"],$feed['app_id']);
//        $str_log .= $feed["data"];
//        $handle = fopen($str_log_file, 'ab');
//        if ($handle) {
//                fwrite($handle,$str_log);
//                fclose($handle) ;
//                return true;
//        }
        return false;
}

function data_log_binary($pack, $pre)
{
//        clearstatcache();
//        $str_log_file = './log/'.$pre.'_data'.posix_getpid().'.log';
//        $handle = fopen($str_log_file, 'ab');
//        if ($handle) {
//                fwrite($handle,$pack);
//                fclose($handle) ;
//                return true;
//        }
        return false;
}

function is_feed_valid($feed)
{
    global $g_sys_conf;
    if (!array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["operator"]))
    {
        log::write("[".__LINE__."]".__FUNCTION__."he feed's cmd_id[{$feed["cmd_id"]}] doesn't Configure in operator array","warn");
        return -1; 
    }
    if (!array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["valid_len"]))
    {
        log::write("[".__LINE__."]".__FUNCTION__."he feed's cmd_id[{$feed["cmd_id"]}] doesn't Configure in valid_len array","warn");
        return -1; 
    }
    if ($g_sys_conf["feed"]["valid_len"][$feed["cmd_id"]] != $feed["len"] && $g_sys_conf['feed']['valid_len'][$feed['cmd_id']] != 0)
    {
        log::write("[".__LINE__."]".__FUNCTION__."he feed's len is unvalid cmd_id {$feed['cmd_id']}:{$feed["len"]}, correct len is {$g_sys_conf["feed"]["valid_len"][$feed["cmd_id"]]}","warn");
        //data_log_pre($feed,'unvalid_'.date('Ymd'));
        return -1; 
    }
//
//    $now_time = time();
//    if ($feed['timestamp'] >= $now_time || $feed['timestamp'] < $now_time -3600 -60)
//    {
//        log::write("[".__LINE__."]".__FUNCTION__."he feed's time > now + 3600, it is impossible. feed timestamp:{$feed['timestamp']} now timestamp + 3600:{$now_time}", 'warn');
//        return -1;
//    }
//
    return 0; 
}

function passive_feed_handler($storage_server_socket, $feed, $is_dest, &$retval)
{
    global $g_sys_conf;
    if ($feed['sender_uid'] == $feed['target_uid'])
    {
        log::write("[".__LINE__."]".__FUNCTION__."ender_uid == target_uid",'warn');
        return -2;
    }

    $feed['passive_magic'] = 0;

    $feed['cmd_id'] += 15000;

    if (!$is_dest)
    {
        $temp['sender_uid'] = $feed['sender_uid'];       
        $temp['target_uid'] = $feed['target_uid'];       
        $feed['sender_uid'] = $temp['target_uid'];
        $feed['target_uid'] = $temp['sender_uid'];
    }
    $ret = get_passive_feed($storage_server_socket, $feed, $result);
    if (!$is_dest)
    {
        $feed['sender_uid'] = $temp['sender_uid'];
        $feed['target_uid'] = $temp['target_uid'];
    }
    if ($ret == 0) //表明要更新
    {
        $flag = 1;
        $feed_content = $result;
        $feed_content['data'] = json_decode($result['data'], true);
        $feed_content['data']['comment_num']++;
        $comment = array(
            'sender_uid' => $feed['sender_uid'], 
            'sender_name' => $feed['sender_uid'],         //'sender_name' => $ret['data']['name'],
            'sender_img_url' => $feed['sender_uid'],      //'sender_img_url' => $ret['data']['logo_m'],
            'timestamp' => $feed['comment_timestamp'],
            'content' => $feed['comment_content'],
            'id' => $feed['comment_id']
        );
        $i = 0;
        foreach($feed_content['data']['comment'] as $val)
        {
            if ($comment['timestamp'] > $val['timestamp'])
            {
                break;
            }
            $i++;
        }
        array_splice($feed_content['data']['comment'], $i, 0, 0);
        $feed_content['data']['comment'][$i] = $comment;

        if ($feed_content['data']['comment_num'] > 100)
        {
            $feed_content['data']['comment_num']--;
            array_pop($feed_content['data']['comment']);
        }
        if ($is_dest)
        {
            if ($feed['comment_timestamp'] > $feed_content['update_timestamp'])
            {
                $feed_content['update_timestamp'] = $feed['comment_timestamp'];
                $is_update = 1;
            }
        }
        $feed_content['data'] = json_encode($feed_content['data']);
    }
    else if ($ret == 1) //表明要插入
    {
        if (!$is_dest)
        {
            return 0; 
        }

        $flag = 0;
        $feed_content['app_id'] = $feed['app_id'];
        $feed_content['cmd_id'] = $feed['cmd_id'];
        $feed_content['user_id'] = $feed['user_id'];
        $feed_content['magic1'] = $feed['magic1'];
        $feed_content['magic2'] = $feed['magic2'];
        $feed_content['timestamp'] = $feed['timestamp'];
        $feed_content['sender_uid'] = $feed['sender_uid'];
        $feed_content['target_uid'] = $feed['target_uid'];
        $feed_content['passive_magic'] = 0;
        $feed_content['update_timestamp'] = $feed['comment_timestamp'];
        switch($feed['cmd_id'])
        {
        case PASSIVE_MESSAGE:
            $comment = array(
                'sender_uid' => $feed['sender_uid'], 
                'sender_name' => $feed['sender_uid'],         //'sender_name' => $ret['data']['name'],
                'sender_img_url' => $feed['sender_uid'],      //'sender_img_url' => $ret['data']['logo_m'],
                'timestamp' => $feed['comment_timestamp'],
                'content' => $feed['comment_content'],
                'id' => $feed['comment_id']
            );
            if ($feed['flag'] == 0)
            {
                $feed_content['data'] = array(
                    'flag' => $feed['flag'],
                    'source' => '',
                    'source_id' => '',
                    'source_timestamp' => '',
                    'comment_num' => 1,
                    'comment' => array($comment)
                );
                $feed_content['data'] = json_encode($feed_content['data']);
            }
            else if ($feed['flag'] == 1)
            {
                $comment = array(
                    'sender_uid' => $feed['sender_uid'], 
                    'sender_name' => $feed['sender_uid'],         //'sender_name' => $ret['data']['name'],
                    'sender_img_url' => $feed['sender_uid'],      //'sender_img_url' => $ret['data']['logo_m'],
                    'timestamp' => $feed['comment_timestamp'],
                    'content' => $feed['comment_content'],
                    'id' => $feed['comment_id']
                );
                $feed_content['data'] = array(
                    'flag' => $feed['flag'],
                    'source' => $feed['source_content'],
                    'source_id' => $feed['source_id'],
                    'source_timestamp' => $feed['source_timestamp'],
                    'comment_num' => 1,
                    'comment' => array($comment) 
                );
                $feed_content['data'] = json_encode($feed_content['data']);
            }
            else
            {
                log::write("[".__LINE__."]".__FUNCTION__."eed['flag'] is unvalid", "warn");
                return -2;
            }
            break;
        default:
            if (($feed['cmd_id'] >= PASSIVE_START_GAME && $feed['cmd_id'] <= PASSIVE_END_GAME) || $feed['cmd_id'] == PASSIVE_YY || $feed['cmd_id'] == PASSIVE_DIARY || $feed['cmd_id'] == PASSIVE_ALBUM || $feed['cmd_id'] == PASSIVE_PHOTO)
            {
                if (array_key_exists(($feed['cmd_id'] - 15000), $g_sys_conf['feed']['operator']))
                {
                    $temp_feed = $feed;
                    $temp_feed['cmd_id'] -= 15000;
                    if (($ret = get_feed($storage_server_socket, $temp_feed, $result)) == 0)
                    {
                        $temp_source = $result;
                        $temp_source['data'] = json_decode($result['data'], true);
                        unset($temp_source['data']['comment']);  // 去除评论部分
                    }
                    else if ($ret == 1)
                    {
                        log::write("[".__LINE__."]".__FUNCTION__."get feed none", "warn");
                        return -2;
                    }
                    else
                    {
                        log::write("[".__LINE__."]".__FUNCTION__."get feed fail ", "error");
                        return -1;
                    }
                    
                    $comment = array(
                        'sender_uid' => $feed['sender_uid'], 
                        'sender_name' => $feed['sender_uid'],         //'sender_name' => $ret['data']['name'],
                        'sender_img_url' => $feed['sender_uid'],      //'sender_img_url' => $ret['data']['logo_m'],
                        'timestamp' => $feed['comment_timestamp'],
                        'content' => $feed['comment_content'],
                        'id' => $feed['comment_id']
                    );

                    if ($feed['flag'] == 0)
                    {
                        $feed_content['data'] = array(
                            'flag' => $feed['flag'],
                            'source' => $temp_source,
                            'reply_content' => $feed['source_content'],
                            'reply_id' => $feed['source_id'],
                            'reply_timestamp' => $feed['source_timestamp'],
                            'comment_num' => 1,
                            'comment' => array($comment) 
                        );
                        $feed_content['data'] = json_encode($feed_content['data']);
                    }
                    else if ($feed['flag'] == 1)
                    {
                        $feed_content['data'] = array(
                            'flag' => $feed['flag'],
                            'source' => $temp_source,
                            'reply_content' => $feed['source_content'],
                            'reply_id' => $feed['source_id'],
                            'reply_timestamp' => $feed['source_timestamp'],
                            'comment_num' => 1,
                            'comment' => array($comment)
                        );
                        $feed_content['data'] = json_encode($feed_content['data']);
                    }
                }
                else
                {
                    log::write("[".__LINE__."]".__FUNCTION__."nvalid cmd_id", 'warn');
                    return -2;
                }
            }
            else
            {
                log::write("[".__LINE__."]".__FUNCTION__."nvalid cmd_id", 'warn');
                return -2;
            }
        }
    }
    else
    {
        log::write("[".__LINE__."]".__FUNCTION__."et feed fail ", "error");
        return -1;
    }


    if ($flag == 1)
    {
        $key = $feed; 
        if (!$is_dest)
        {
            $temp['sender_uid'] = $feed['sender_uid'];       
            $temp['target_uid'] = $feed['target_uid'];       
            $key['sender_uid'] = $temp['target_uid'];
            $key['target_uid'] = $temp['sender_uid'];
        }
    }
    else
    {
    }
   
    $ret = operate_passive_feed_to_db($storage_server_socket, $key, $feed_content); 
    if ($ret == 1)
    {
        log::write("[".__LINE__."]".__FUNCTION__."perate_feed_to_db update id duplicate dont happen", "warn");
        return -2;
    }
    else if ($ret != 0)
    {
        log::write("[".__LINE__."]".__FUNCTION__."pdate to storage_server fail", "error");
        return -1;
    }
    
    if ($flag == 1)
    {
        if ($is_update == 1)
        {
            $retval['feed_op'] = 'update';
            $retval['type'] = 'passive';
            $retval['update_old_feedid'] = $key;
            $retval['update_new_feedid'] = $feed_content;
        }
    }
    else
    {
            $retval['feed_op'] = 'insert';
            $retval['type'] = 'passive';
            $retval['insert_feedid'] = $feed_content;
    }    

    return 0;
}

function feed_update_sync($pack, $input_arr, &$output_arr)
{
    $allow_ce = array(
        NEWS_STATUS,
        NEWS_DIARY,
        NEWS_PHOTO,
        7023,
        7024         
    );

    $feed = unpack('Slen/Sopt_id', $pack);
    if ($feed['opt_id'] == 201)
    {
        if (32 != strlen($pack))
        {
            log::write("[".__LINE__."]".__FUNCTION__."ack len is unvalid ".strlen($pack), 'warn');
            return -2;
        }
        $storage_server_socket = $input_arr['storage_server_socket'];
        $feed = unpack('Slen/Sopt_id/Lsender_id/Cversion/Ltimestamp/Lapp_id/Lmagic1/Lmagic2/Scmd_id/Luser_id/Cevaluate', $pack);

        if (!in_array($feed['cmd_id'], $allow_ce))
        {
            log::write("[".__LINE__."]".__FUNCTION__."he cmd_id cannot evaluate ". $feed['cmd_id'], 'warn');
            return -2;
        }

        if (($ret = get_feed($storage_server_socket, $feed, $result)) == 0)
        {
            $feed_content = $result;
            $feed_content['data'] = json_decode($result['data'], true);
            if (!isset($feed_content['data']['evaluate']))
            {
                $feed_content['data']['evaluate'] = 0;
            }
            $feed_content['data']['evaluate'] += $feed['evaluate'] - 128;
            $feed_content['data'] = json_encode($feed_content['data']);
        }
        else if ($ret == 1)
        {
            log::write("[".__LINE__."]".__FUNCTION__."et feed none", "warn");
            return -2;
        }
        else
        {
            log::write("[".__LINE__."]".__FUNCTION__."et feed fail ", "error");
            //data_log_binary($pack, ;
            return -1;
        }

        $ret = operate_feed_to_db($storage_server_socket, $feed, $feed_content); 
        if ($ret == 1)
        {
            log::write("[".__LINE__."]".__FUNCTION__."perate_feed_to_db update id duplicate dont happen", "warn");
            //data_log_binary($pack,"feed_update_sync_warn");
            return -2;
        }
        else if ($ret != 0)
        {
            log::write("[".__LINE__."]".__FUNCTION__."pdate to storage_server fail", "error");
            //data_log_binary($pack, ;
            return -1;
        }
        return 0;
    }
    else if($feed['opt_id'] == 202)
    {
        $cmd_arr_only_passive = array(7023,7024);
        if (912 != strlen($pack))
        {
            log::write("[".__LINE__."]".__FUNCTION__."pack len is unvalid ".strlen($pack), 'warn');
            return -2;
        }
        $storage_server_socket = $input_arr['storage_server_socket'];
        $feed = unpack('Slen/Sopt_id/Lsender_uid/Cversion/Ltimestamp/Lapp_id/Lmagic1/Lmagic2/Scmd_id/Luser_id/Cflag/a430source_content/Lsource_id/Lsource_timestamp/a430comment_content/Lcomment_timestamp/Lcomment_id/Ltarget_uid', $pack);

        if (!in_array($feed['cmd_id'], $allow_ce))
        {
            log::write("[".__LINE__."]".__FUNCTION__."the cmd_id cannot comment ". $feed['cmd_id'], 'warn');
            return -2;
        }
        
        //*****更新主动feed*********
        if (($ret = get_feed($storage_server_socket, $feed, $result)) == 0)
        {
            $feed['timestamp'] = $result['timestamp'];
            $feed_content = $result;
            $feed_content['data'] = json_decode($result['data'], true);
            $feed_content['data']['comment_num']++;
            $comment = array(
            'sender_uid' => $feed['sender_uid'], 
            'sender_name' => $feed['sender_uid'],         //'sender_name' => $ret['data']['name'],
            'sender_img_url' => $feed['sender_uid'],      //'sender_img_url' => $ret['data']['logo_m'],
            'timestamp' => $feed['comment_timestamp'],
            'content' => $feed['comment_content'],
            'id' => $feed['comment_id']
            );
            $i = 0;
            if (!isset($feed_content['data']['comment']))
            {
                $feed_content['data']['comment'] = array();
            }
            foreach($feed_content['data']['comment'] as $val)
            {
                if ($comment['timestamp'] > $val['timestamp'])
                {
                    break;
                }
                $i++;
            }
            array_splice($feed_content['data']['comment'], $i, 0, 0);
            $feed_content['data']['comment'][$i] = $comment;
            if ($feed_content['data']['comment_num'] > 10)
            {
                //$feed_content['data']['comment_num']--;
                array_pop($feed_content['data']['comment']);
            }
            $feed_content['data'] = json_encode($feed_content['data']);

            $ret = operate_feed_to_db($storage_server_socket, $feed, $feed_content); 
            if ($ret == 1)
            {
                log::write("[".__LINE__."]".__FUNCTION__."perate_feed_to_db update id duplicate dont happen", "warn");
                //data_log_binary($pack,"feed_update_sync_warn");
                return -2;
            }
            else if ($ret != 0)
            {
                log::write("[".__LINE__."]".__FUNCTION__."pdate to storage_server fail", "error");
                //data_log_binary($pack, ;
                return -1;
            }
        }
        else if ($ret == 1)
        {
            if (!in_array($feed['cmd_id'], $cmd_arr_only_passive))
            {
                log::write("[".__LINE__."]".__FUNCTION__."get feed none", "warn");
            }
        }
        else
        {
            log::write("[".__LINE__."]".__FUNCTION__."get feed fail ", "error");
            //data_log_binary($pack, ;
            return -1;
        }

        //**************************
        //*****更新或生产passive_feed******
        //更新对方的被动
        $ret = passive_feed_handler($storage_server_socket, $feed, true, $ret_val);
        if ($ret == -2)
        {
            log::write("[".__LINE__."]".__FUNCTION__."assive_feed_handler desc parameter problem", "warn");
            //data_log_binary($pack,"feed_update_sync_warn");
            return -2;
        }
        else if ($ret != 0)
        {
            log::write("[".__LINE__."]".__FUNCTION__."passive_feed_handler process fail ", "error");
            //data_log_binary($pack, ;
            return -1;
        }

        //更新自己的被动
        $ret = passive_feed_handler($storage_server_socket, $feed, false, $ret_val1);
        if ($ret == -2)
        {
            log::write("[".__LINE__."]".__FUNCTION__."assive_feed_handler my parameter problem", "warn");
            //data_log_binary($pack,"feed_update_sync_warn");
            return -2;
        }
        else if ($ret != 0)
        {
            log::write("[".__LINE__."]".__FUNCTION__."passive_feed_handler process fail ", "error");
            //data_log_binary($pack,;
            return -1;
        }

        if (isset($ret_val))
        {
            $output_arr[] = $ret_val;
        }
        if (isset($ret_val1))
        {
            $output_arr[] = $ret_val1;
        }

        return 0;
        //*********************************
    }
    else
    {
        log::write("[".__LINE__."]".__FUNCTION__." cmd_id is unvalid ". $feed['opt_id'], "warn");
        return -2;
    }
    return 0;
}

function feeddelete($pack, $input_arr, &$output_arr)
{
    global $g_sys_conf;
    $feed = unpack('Slen/Sopt_id', $pack);
    if ($feed['opt_id'] == 1)    {
        $feed = unpack('Slen/Sopt_id/Luser_id/Cversion/Ltimestamp/Lapp_id/Scmd_id',$pack);
        if (!array_key_exists($feed['cmd_id'], $g_sys_conf['feed']['support_delete']))        {
            log::write('request feed do not support delete function '.strlen($pack), 'warn');
            return -2;
        }

        $storage_server_socket = $input_arr['storage_server_socket'];    
    global $g_tag_server_socket;
    $g_tag_server_socket = $input_arr['tag_server_socket'];
        if ($ret = $g_sys_conf['feed']['support_delete'][$feed['cmd_id']]($feed, $pack, $storage_server_socket)) {
            if (-1 == $ret)        {
                log::write(__LINE__.":feed operator error:".print_r($feed, true), "error");
                return -2;
            }  else if(-2 == $ret)    {
                log::write("feed operator http request get data is null or feed data is invalid error:".print_r($feed, true), "warn");
                return -2;
            }        else        {
                log::write("feed operator return unvalid value[{$ret}]".print_r($feed, true), "error");
                return -2;
            }
        }
        {
            if (false == delete_feed($storage_server_socket, $feed))
            {
                log::write("[".__LINE__."]".__FUNCTION__."elete feed fail from storage_server", "error");
                //data_log_binary($pack, ;
                return -1;
            }
        }
        $output_arr = array(array(
                                   'feed_op' => 'delete',
                                   'type' => 'active',
                                  'delete_feedid' => $feed ));
    }
    else if ($feed['opt_id'] == 2)
    {
        if (strlen($pack) != 67 )
        {
            log::write("[".__LINE__."]".__FUNCTION__."ack len is unvalid ".strlen($pack), 'warn');
            return -2;
        }

        $feed = unpack('Slen/Sopt_id/Lsender_uid/Cversion/Ltimestamp/Lapp_id/Lmagic1/Lmagic2/Scmd_id/Luser_id/Ltarget_uid/a32verify',$pack);
        $feed['passive_magic'] = 0;
        $str = substr($pack, 0, 35);
        $str .= DELETE_SECRET_KEY;
        $verify = md5($str);
        if ($verify != $feed['verify'])
        {
            log::write("[".__LINE__."]".__FUNCTION__."he packet doesn't pass verify","warn");
            return -2;
        }

        $storage_server_socket = $input_arr['storage_server_socket'];    

            if (false == delete_passive_feed($storage_server_socket, $feed))
            {
                log::write("[".__LINE__."]".__FUNCTION__."elete feed fail from storage_server", "error");
                //data_log_binary($pack, ;
                return -1;
            }

        $output_arr = array(array(
                                   'feed_op' => 'delete',
                                   'type' => 'passive',
                                  'delete_feedid' => $feed ));
    }
    else
    {
        log::write("[".__LINE__."]".__FUNCTION__." cmd_id is unvalid ". $feed['opt_id'], "warn");
        return -2;
    }
    return 0;
}

$g_redis_server_socket;
$g_tag_server_socket;
$g_storage_server_socket;
// $feed 一条feed的内容
// array("len","cmd_id","user_id","version","timestamp","app_id","data")
// $input_arr("feed_socket"=>val1,"feed_reconnect_flag"=>val2,"storage_server_socket" => val3):feed_socket为同步feed的服务器连接socket，feed_reconnect_flag为feed服务器重连标志,storage_server_socket为存储服务器的socket
// $output_arr为输出数组 
// return 0 成功 -1严重的错误 -2 一些警告，非严重错误，可以忽略
function feedhandle($feed, $input_arr, &$output_arr)
{
    global $g_sys_conf;
    
    $feed_socket = $input_arr['feed_socket'];
    $feed_reconnect_flag = $input_arr['feed_reconnect_flag'];
    $storage_server_socket = $input_arr['storage_server_socket'];
    global $g_storage_server_socket;
    $g_storage_server_socket = $storage_server_socket;
    global $g_redis_server_socket;
    $g_redis_server_socket = $input_arr['redis_server_socket'];
    global $g_tag_server_socket;
    $g_tag_server_socket = $input_arr['tag_server_socket'];

    if (!array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["operator"]))
    {
        log::write("[".__LINE__."]".__FUNCTION__."he feed's cmd_id[{$feed["cmd_id"]}] doesn't Configure","warn");
        return -2; 
    }
 
    if (array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["iscombine"]))
    {
            if (false == get_comfeed($storage_server_socket, $feed["user_id"], $feed["cmd_id"], $feed["timestamp"], $feed['app_id'], $comfeed))
            {
                log::write("[".__LINE__."]".__FUNCTION__."et_comfeed get combine feed fail from storage server", "error");
                //data_log_pre($feed, ;
                return -1; 
            }
    }     
   
    //防止合并的feed过多。超过字段限制 
    if ($comfeed['data'] > 60000)
    {
        unset($comfeed);
    }  

    //$comfeed只对于需要合并的有意义。对于不需要合并的忽略它，在需要合并的动态下，如果没有设置表明，原来数据库里没有合并相关项.如果设置了，该值传入的是一个数组，数组的内容就是一条feed，最后要修改为你要更新的feed
    //$completefeed feed需要合并的动态是合并好并且装配好后的，不需要合并的动态是装配好后的。
    //$feed是原始的格式，$comfeed从数据库查出来的合并好后的，$completefeed这个是前两个合并好后生成的
    //对于magic字段有些feed是用户指定的 在data字段里。有些是系统生成的就没有feed里就没有, 这就影响到$completefeed，用户生成的有magic1,magic2,而系统指定就没有。
    $completefeed['app_id'] = $feed['app_id'];
    if ($ret = $g_sys_conf["feed"]["operator"][$feed["cmd_id"]]($feed, $comfeed, $completefeed, $passive_feed))
    {
        if (-1 == $ret)
        {
            log::write("[".__LINE__."]".__FUNCTION__."eed operator error:".print_r($feed, true), "error");
            //data_log_pre($feed, ;
            return -1;
        }
        else if(-2 == $ret)
        {
            log::write("[".__LINE__."]".__FUNCTION__."eed operator http request get data is null or feed data is invalid error:".print_r($feed, true), "warn");
            //data_log_pre($feed,"get_null");
            return -2;
        }
        else
        {
            log::write("[".__LINE__."]".__FUNCTION__."eed operator return unvalid value[{$ret}]".print_r($feed, true), "error");
            //data_log_pre($feed, ;
            return -1;
        }
    }
    
    $output_arr = array(); 
    if (!array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["ispassive"]))    //不需要生成主动feed流
    {
        if (array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["user_defined_id"]))
        {
            if (isset($comfeed) && !is_null($comfeed)) //update
            {
                $ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed); 
                if ($ret == 1)
                {
                    log::write("operate_feed_to_db update user defined id duplicate error:".print_r($feed, true), "warn");
                    //data_log_pre($feed,"user_defined_id_duplicate");
                    return -2;
                } 
                else if ($ret != 0)
                {
                    log::write(__FUNCTION__."".__LINE__."]"."update completefeed to storage_server fail", "error");
                    ////data_log($feed);
                    return -1;
                }
                array_push($output_arr, array(
                            'feed_op' => 'update',
                            'type' => 'active',
                            'update_new_feedid' => $completefeed,
                            'update_old_feedid' => $comfeed));
            }
            else //insert
            {
                $ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed); 
                if ($ret == 1)
                {
                    log::write("operate_feed_to_db insert user defined id duplicate error:".print_r($feed, true), "warn");
                    //data_log_pre($feed,"user_defined_id_duplicate");
                    return -2;
                }
                else if ($ret != 0)
                {
                    log::write(__FUNCTION__."".__LINE__."]"."insert completefeed to storage_server fail", "error");
                    //data_log($feed);
                    return -1;
                }
                array_push($output_arr,array(
                        'feed_op' => 'insert',
                        'type' => 'active',
                        'insert_feedid' => $completefeed));
            }
        }
        else
        {
            $completefeed["magic1"] = rand();
            $completefeed["magic2"] = 0; 
            if (isset($comfeed) && !is_null($comfeed)) //update 
            {   
                $ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed);
                while ($ret == 1)  //保证id唯一
                {
                    $completefeed["magic1"] = rand();
                    $completefeed["magic2"] = 0; 
                    $ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed); 
                }
                if (0 != $ret)
                {
                    log::write(__FUNCTION__."".__LINE__."]"."update completefeed to storage_server fail", "error");
                    //data_log($feed);
                    return -1;
                } 
                array_push($output_arr,array(
                        'feed_op' => 'update',
                        'type' => 'active',
                        'update_new_feedid' => $completefeed,
                        'update_old_feedid' => $comfeed
                    ));
            }
            else //insert
            {
                $ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed); 
                while ($ret == 1)  //保证id唯一
                {
                    $completefeed["magic1"] = rand();
                    $completefeed["magic2"] = 0; 
                    $ret = operate_feed_to_db($storage_server_socket, $comfeed, $completefeed); 
                }
                if ($ret != 0)
                {
                    log::write(__FUNCTION__."".__LINE__."]"."insert completefeed to storage_server fail", "error");
                    //data_log($feed);
                    return -1;
                }
                array_push($output_arr,array(
                        'feed_op' => 'insert',
                        'type' => 'active',
                        'insert_feedid' => $completefeed
                            ));
            }
        }
        $ret = operate_update_feed_id_to_db($g_tag_server_socket, $feed["user_id"], $feed["timestamp"]);
        if ($ret != 0)
        {
            log::write(__FUNCTION__."[".__LINE__."]"."update feed mimiid to redis_server fail".print_r($feed, true), "error");
            return -1;
        }
  
    }
 
    if (isset($passive_feed))
    {
        foreach($passive_feed as $val)
        {
            $val['magic1'] = $completefeed['magic1'];
            $val['magic2'] = $completefeed['magic2'];
            $ret = operate_passive_feed_to_db($storage_server_socket, NULL, $val); 
            while ($ret == 1)  //保证id唯一
            {
                $val['passive_magic'] = rand();
                $ret = operate_passive_feed_to_db($storage_server_socket, NULL, $val); 
                //return -2;
            }
            if ($ret != 0)
            {
                log::write("[".__LINE__."]".__FUNCTION__."pdate to storage_server fail" . print_r($val, true), "error");
                return -1;
            }
            $output_arr[] = array(
                'feed_op' => 'insert',
                'type' => 'passive',
                'insert_feedid' => $val);
        }
    } 
        
//    DEBUG && log::write("XXXX [".__LINE__."] storage".print_r($completefeed, true),"debug");
//    DEBUG && log::write("XXXX [".__LINE__."] outbox".print_r($output_arr, true),"debug");
    //分发到feed_server上去的**************** 
    //*****************************************
    
    return 0;
}

function produce_passive_feed($feed, $sender_uid, $target_uid, $data)
{
    $passive_feed['app_id'] = $feed['app_id'];
    $passive_feed['cmd_id'] = $feed['cmd_id'] + 15000;
    $passive_feed['user_id'] = $feed['user_id'];
    $passive_feed['timestamp'] = $feed['timestamp'];
    $passive_feed['sender_uid'] = $sender_uid;
    $passive_feed['target_uid'] = $target_uid;
    $passive_feed['passive_magic'] = 0;
    $passive_feed['update_timestamp'] = $feed['timestamp'];
    $passive_feed['data'] = $data;
    return $passive_feed;
}


function http_post($arr_para,$server_url)
{
    $para = http_build_query($arr_para);
    $query_url = implode('?', array($server_url,$para));
    $ret = file_get_contents($query_url);
    if ($ret == false)
    {
       log::write("[".__LINE__."]".__FUNCTION__.$query_url, "error"); 
    }
    //DEBUG && log::write("[".__LINE__."]".__FUNCTION__."rl:".$query_url."content:".$ret,"debug");
    return $ret;
}

function is_valid(&$result_arr)
{
    $result_arr = json_decode($result_arr, true); 
    if(is_array($result_arr))
    {
        if($result_arr["success"] == true)
        {
            //if($result_arr["code"] == 0)
            //{
                return true;
            //}
        }
    }
    return false;
}

function get_friend_id(&$arr_uid, $mimi) {
    global $g_redis_server_socket;
    if ($g_redis_server_socket == false) {
        if (init_connect_and_nonblock(RELATION_IP, RELATION_PORT, $g_redis_server_socket)) {
            log::write("[".__LINE__."]:init_connect friend_server fail reason: connect to relation_server", "error");
            return -1;
        }
    }

    $relation_rqst = pack("LLsLLLLL", 18+4+4+4, 0, 0xB104, 0, $mimi, $mimi, 0, 0);
    if (send_data_and_nonblock($g_redis_server_socket, $relation_rqst, TIMEOUT)) {
            log::write("[".__LINE__."]:friend_server fail reason send data error", "error");
            return -1;
    }

    $resp_head = "";
    if (recv_data_and_nonblock($g_redis_server_socket, 18, $resp_head, TIMEOUT)) {
        log::write("[".__LINE__."]:recv data to friend server fail", "error");
        return -1;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $resp_head);
    if ($rv['code'] != 0) {
        log::write("[".__LINE__."]:tag friend server interal fail :".$rv['code'], "error");
        return -1;
    }
    
//    DEBUG && log::write("[".__LINE__."]: friend server recv len:".strlen($resp_head), "error");
    $resp_body = substr($resp_head, 18);
    $rv = unpack("Lunits", $resp_body);
    
//    DEBUG && log::write("[".__LINE__."]: friend server cnt:".$rv['units'], "error");
    $id_list = substr($resp_body, 4);
    for ($i = 0; $i < $rv['units']; ++$i) {
        $fan = unpack('Lid/Ltime', $id_list);
        array_push($arr_uid, $fan['id']);
        $id_list = substr($id_list, 8);
    }
    return 0;
}

function __init_completefeed_magic(&$completefeed) {
    $completefeed["magic1"] = rand();
    $completefeed["magic2"] = 0;
}
function __init_completefeed_common($feed, &$completefeed) {
    $completefeed["user_id"] = $feed["user_id"];
    $completefeed["cmd_id"] = $feed["cmd_id"];
    $completefeed["timestamp"] = $feed["timestamp"];
}
function __init_completefeed($feed, &$completefeed) {
    __init_completefeed_common($feed, $completefeed);
    __init_completefeed_magic($completefeed);
}

function news_article($feed, &$comfeed, &$completefeed, &$passive_feed)
{ 
    __init_completefeed($feed, $completefeed);
    $src_data = unpack("Larticle_id", $feed["data"]);
    $feed["data"] = substr($feed["data"], 4);
    
    //解析首张图片的长度并根据长度获取url
    $parse = unpack("Clen", $feed["data"]);
    $src_data["pic"] = substr($feed["data"], 1, $parse["len"]);
    $feed["data"] = substr($feed["data"], 1 + $parse["len"]);
    $rv = unpack("Ltag_cnt", $feed["data"]);
    $feed["data"] = substr($feed["data"], 4);
    $tag = array();
    for ($i = 0; $i < $rv['tag_cnt']; $i++) {
        $cur = unpack('Ltag', substr($feed["data"], 4 * $i));
        $tag[] = $cur['tag'];
    }
    $src_data["tags"] = $tag;
    
    //用于向tagList插入数据时的数据打包参数
    $feed['tag_cnt'] = $rv['tag_cnt'];
    $feed['tag_data'] = substr($feed['data'], 0, 4 * $rv['tag_cnt']);

    $completefeed["data"] = json_encode($src_data);
    $code =  __fill_article_to_tag_list($feed, $completefeed);
//    DEBUG && log::write("XXXX [".__LINE__."] constructor feedid:".print_r($completefeed, true),"debug");
    return $code;

    //推送模式模块
    if(news_article_add_to_friend($feed['new_feedid'], $feed["user_id"], $feed["timestamp"]))
        return -1;

    return 0;
}

function __fill_article_to_tag_list(&$feed, $completefeed) {
    //将帖子添加到指定归类的集合中
    global $g_tag_server_socket;
    if ($g_tag_server_socket == false) {
        if (init_connect_and_nonblock(TAG_CACHE_IP, TAG_CACHE_PORT, $g_tag_server_socket))
       {   
           log::write("init_connect tag_server fail reason: connect to relation_server", "error");
           return -1;
       }
        log::write("init_connect tag_server fail reason: connect to relation_server", "error");
    }
    $feed['new_feedid'] = base64_encode(pack("LSLLLL", $feed['user_id'], $feed["cmd_id"], $feed["app_id"], $feed["timestamp"], $completefeed["magic1"], $completefeed["magic2"]));
//    DEBUG && log::write("XXXX [".__LINE__."] constructor feedid:<".$feed['new_feedid']."> ","debug");

    $relation_rqst = pack("LLsLLLLL", 18 + 12 + strlen($feed['new_feedid'])+strlen($feed["tag_data"]), 0, 0xA103, 0, $feed['user_id'],  $feed["timestamp"], $feed['tag_cnt'], strlen($feed['new_feedid'])).$feed['new_feedid'].$feed["tag_data"];
    
    if (send_data_and_nonblock($g_tag_server_socket, $relation_rqst, TIMEOUT)) {
        log::write(__LINE__.'get_fans_id: relation_client->send_rqst', 'error');
        return -1;
    }

    $relation_resp = "";
    if (recv_data_and_nonblock($g_tag_server_socket, 18, $relation_resp, TIMEOUT))
    {
        log::write(__LINE__."recv data to tag cache server fail", "error");
        return -1;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $relation_resp);
    if ($rv['code'] != 0) {
        log::write(__LINE__.'tag cache server interal fail', 'error');
        return -1;
    }

    return 0;
}

function news_article_add_to_friend($feedid, $mimi, $time) {
    global $g_tag_server_socket;

    $friendId = array();//[] = $mimi;
    if (get_friend_id($friendId, $mimi)) {
        log::write("[".__LINE__."]:Can not get friend mimi from relation server", "error");
        return -1;
    }

    $protobuf = new \Mifan\feedidToFriend();
    $protobuf->setFeedid($feedid);
    foreach ($friendId as $fid) {
        $protobuf->appendMimi($fid);
    }
    $protobuf->setTime($time);
    $seri_buf = $protobuf->serializeToString();
    $feed_cache_rqst = pack("LLSLL", 18 + strlen($seri_buf), 0, 0xA105, 0, $mimi).$seri_buf;
    if (send_data_and_nonblock($g_tag_server_socket, $feed_cache_rqst, TIMEOUT)) {
        log::write(__LINE__.'get_fans_id: relation_client->send_rqst', 'error');
        return -1;
    }

    $feed_cache_resp = "";
    if (recv_data_and_nonblock($g_tag_server_socket, 18, $feed_cache_resp, TIMEOUT)) {
        log::write(__LINE__."recv data to tag cache server fail", "error");
        return -1;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $feed_cache_resp);
//    DEBUG && log::write('['.__LINE__.']: get response from cache server'.print_r($rv, true), "error");
    if ($rv['code'] != 0) {
        log::write(__LINE__.'tag cache server interal fail :'.$rv['code'], 'error');
        return -1;
    }
    
    return 0;
}
function update_pfeeds_statistic($passive_feed) {
//    DEBUG && log::write('['.__LINE__.']: passive:'.print_r($passive_feed, true), "error");
    
    global $g_tag_server_socket;
    if ($g_tag_server_socket == false) {
        if (init_connect_and_nonblock(TAG_CACHE_IP, TAG_CACHE_PORT, $g_tag_server_socket)) {
            log::write("[".__LINE__."]:init_connect friend_server fail reason: connect to tag_server", "error");
            return -1;
        }
    }

    $protobuf = new \Mifan\pUpdateStat();
    $protobuf->setCmd($passive_feed[0]['cmd_id']);
    foreach ($passive_feed as $pfeed) { 
        $protobuf->appendMimi($pfeed['target_uid']);
    }
    //DEBUG && log::write('['.__LINE__.']: cmd:'.$protobuf->getCmd(), "error");
    $body = $protobuf->serializeToString();
    $rqst = pack('L2SL2', 18 + strlen($body), 0, 0xA201, 0, $passive_feed[0]['user_id']).$body;
    if (send_data_and_nonblock($g_tag_server_socket, $rqst, TIMEOUT)) {
        log::write(__LINE__.'get_fans_id: relation_client->send_rqst', 'error');
        return -1;
    }
    
    $resp = "";
    if (recv_data_and_nonblock($g_tag_server_socket, 18, $resp, TIMEOUT)) {
        log::write(__LINE__."recv data to tag cache server fail", "error");
        return -1;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $resp);
//    DEBUG && log::write('['.__LINE__.']: get response from cache server'.print_r($rv, true), "error");
    if ($rv['code'] != 0) {
        log::write(__LINE__.'tag cache server interal fail :'.$rv['code'], 'error');
        return -1;
    }
    return 0;
}

function news_liker($feed, &$comfeed, &$completefeed, &$passive_feed)
{
    $src_data = unpack("Larticle_id/Lauthor_id", $feed["data"]);
    if ($src_data['author_id'] == $feed['user_id']) {
        log::write("Do not support liker myself".print_r($feed, true), "warn");
        return 0;
    }
   
    global $g_storage_server_socket;
    if (check_liker_pfeed_unique($g_storage_server_socket, $src_data['author_id'], $feed['user_id'], $feed['cmd_id'], $feed['app_id'], $src_data['article_id']) == TRUE) {
        log::write('<'.$feed['user_id'].'> Reliker article<'.$src_data['article_id'].'> again', "warn");
        return 0;
    }
    log::write('<'.$feed['user_id'].'> Newliker article<'.$src_data['article_id'].'> again', "warn");
    $src_data['liked_id'] = $feed["user_id"];
    $json_src_data = json_encode($src_data);
    // 通知博主有人点赞了我的帖子
    $passive_feed[] = produce_passive_feed($feed, $feed['user_id'], $src_data['author_id'], $json_src_data);
    
    __init_completefeed_common($feed, $completefeed);
    $completefeed['data'] = json_encode($src_data);
    update_pfeeds_statistic($passive_feed);
    notify_kafka_update_score(__notify_gen_message($feed['user_id'], $src_data['article_id'], $feed['cmd_id'], $src_data['author_id']));
    return 0;
}
function news_comment($feed, &$comfeed, &$completefeed, &$passive_feed)
{
    __init_completefeed_common($feed, $completefeed);
    
    $src_data = unpack("Ltype/Larticle_id/Lauthor_id/Lcomment_mid/Lcomment_id", $feed["data"]);

    $src_data["reply_id"] = $feed["user_id"];
    $json_src_data = json_encode($src_data);
    //通知被评论者，谁评论了我(被动feed)
    if ($src_data["comment_mid"] != $feed["user_id"])
        $passive_feed[] = produce_passive_feed($feed, $feed['user_id'], $src_data['comment_mid'], $json_src_data);
    
    //通知博主，有新评论(被动feed)
    if ($src_data["comment_mid"] != $src_data["author_id"] && $src_data["author_id"] != $feed["user_id"])
        $passive_feed[] = produce_passive_feed($feed, $feed['user_id'], $src_data['author_id'], $json_src_data);

    $completefeed['data'] = json_encode($src_data);
    update_pfeeds_statistic($passive_feed);
    notify_kafka_update_score(__notify_gen_message($feed['user_id'], $src_data['article_id'], $feed['cmd_id'], $src_data['author_id']));
    return 0;
}
       
function delete_article_by_tag($feed, $tags, $storage_server_socket) {
    //将帖子添加到指定归类的集合中
    global $g_tag_server_socket;
    if ($g_tag_server_socket == false) {
        if (init_connect_and_nonblock(TAG_CACHE_IP, TAG_CACHE_PORT, $g_tag_server_socket))
       {   
           log::write("init_connect tag_server fail reason: connect to relation_server", "error");
            return -1;
       }
        log::write("init_connect tag_server fail reason: connect to relation_server", "error");
    }

    $feedid = base64_encode(pack("LSLLLL", $feed['user_id'], $feed["cmd_id"], $feed["app_id"], $feed["timestamp"], $feed["magic1"], $feed["magic2"]));

    $protobuf = new \Mifan\dropFeedidFromTag();
    $protobuf->setFeedid($feedid);
//    DEBUG && log::write('['.__LINE__.']: feedid r'.$feedid, "error");
    foreach ($tags as $tag) {
        $protobuf->appendTag($tag);
//    DEBUG && log::write('['.__LINE__.']: feedid tag:<'.$tag.'>', "debug");
    }
    $seri_buf = $protobuf->serializeToString();
    $feed_cache_rqst = pack("LLSLL", 18 + strlen($seri_buf), 0, 0xD103, 0, $feed['user_id']).$seri_buf;
    if (send_data_and_nonblock($g_tag_server_socket, $feed_cache_rqst, TIMEOUT)) {
        log::write(__LINE__.'get_fans_id: relation_client->send_rqst', 'error');
        return -1;
    }

    $feed_cache_resp = "";
    if (recv_data_and_nonblock($g_tag_server_socket, 18, $feed_cache_resp, TIMEOUT)) {
        log::write(__LINE__."recv data to tag cache server fail", "error");
        return -1;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $feed_cache_resp);
//    DEBUG && log::write('['.__LINE__.']: get response from cache server'.print_r($rv, true), "error");
    if ($rv['code'] != 0) {
        log::write(__LINE__.'tag cache server interal fail :'.$rv['code'], 'error');
        return -1;
    }
    
    return 0;
}

function delete_article(&$feed, $rqst, $storage_server_socket) {
//2    len
//2    opt_id
//4    user_id
//1    version
//4    timestamp
//4    app_id
//2    cmd_id
//4    article
    if ($feed['len'] != 23) {
        log::write('['.__LINE__.'] Invalid len for delete article(23) :'.$feed['len'], 'error');
        return -1;
    }

    $article = unpack('Lid', substr($rqst, 19));
    $feed_list = array();
//    DEBUG && log::write('['.__LINE__.']: article id is : '.$article['id'], "error");

    if (false == get_comfeed_time_span($storage_server_socket, 
            $feed['user_id'],
            $feed['cmd_id'],
            $feed['timestamp'],
            $feed['timestamp'],
            $feed['app_id'],
            $feed_list)) {
        log::write('['.__LINE__.'] Can not find matched feed :'.print_r($feed, true), 'error');
        return -1;
    }
   
//    DEBUG && log::write('['.__LINE__.']: match feeds is : '.print_r($feed_list, true), "error");
    foreach ($feed_list as $var) {
        $tmp = json_decode($var['data'], true);
//    DEBUG && log::write('['.__LINE__.']: feed data is : '.print_r($tmp, true), "error");
        if ($article['id'] == $tmp['article_id']) {
            $feed['magic1'] = $var['magic1'];
            $feed['magic2'] = $var['magic2'];
            $feed['data'] = $var['data'];
//            DEBUG && log::write('['.__LINE__.']: Got feed: '.print_r($feed, true), "debug");
            delete_article_by_tag($feed, $tmp['tags'], $storage_server_socket);
//            $feedid = base64_encode(pack("LSLLLL", $feed['user_id'], $feed["cmd_id"], $feed["app_id"], $feed["timestamp"], $feed["magic1"], $feed["magic2"]));
                
            return 0;
        }
    }
    return -2;
}

function news_fans($feed, &$comfeed, &$completefeed, &$passive_feed) {
    $following = unpack('Lid', $feed['data']);
    if ($following['id'] == $feed['user_id']) {
        log::write("Do not support following myself".print_r($feed, true), "warn");
        return 0;
    }
    
    global $g_storage_server_socket;
    if (check_fans_pfeed_unique($g_storage_server_socket, $following['id'], $feed['user_id'], $feed['cmd_id'], $feed['app_id'], -1) == TRUE) {
        log::write('<'.$feed['user_id'].'> Refollowing<'.$following['id'].'> again', "warn");
        return 0;
    }
    log::write('<'.$feed['user_id'].'> following <'.$following['id'].'> first time', "warn");
    $src_data['fans'] = $feed['user_id'];
    $json_src_data = json_encode($src_data);
//        log::write("Fans ".print_r($src_data, true), "error");
    $passive_feed[] = produce_passive_feed($feed, $src_data['fans'], $following['id'], $json_src_data);

    __init_completefeed_common($feed, $completefeed);
    $completefeed['data'] = $json_src_data;
//        log::write("Fans ".print_r($completefeed_data, true), "error");
    update_pfeeds_statistic($passive_feed);
    return 0;
}

global $kCnf;
global $kProducerCnf;
global $kTopicCnf;

function __notify_gen_message($uid, $article, $cmd_id, $author) {
    $protobuf_ = new \Mifan\noteScore();
    $protobuf_->setUser($uid);
    $protobuf_->setArticle($article);
    $protobuf_->setCmd($cmd_id);
    $protobuf_->setAuthor($author);
    return $protobuf_->serializeToString();
}
function __check_kafka_cnf_exist_and_init(&$kCnf, &$kProducerCnf, &$kTopicCnf) {
    if (!isset($kCnf)) {
        log::write('['.__FUNCTION__.']['.__LINE__.'] Kafka ctx never init before, initing...');

        $kCnf = new RdKafka\Conf();
        $kCnf->setDrMsgCb(function ($kafka, $message) {
           file_put_contents(KAFKA_DEBUG_FILE, var_export($message, true).PHP_EOL, FILE_APPEND);
           });

        $kCnf->setErrorCb(function ($kafka, $err, $reason) {
           file_put_contents(KAFKA_ERROR_FILE, var_export($message, true).PHP_EOL, FILE_APPEND); 
        });
    }

    if (!isset($kProducerCnf)) {
        log::write('['.__FUNCTION__.']['.__LINE__.'] Kafka Producer ctx never init before, initing...');
        $kProducerCnf = new RdKafka\Producer($kCnf);
        $kProducerCnf->setLogLevel(LOG_DEBUG);
        $kProducerCnf->addBrokers(KAFKA_BROKER_IP);
    }
    
    if (!isset($kTopicCnf)) {
        log::write('['.__FUNCTION__.']['.__LINE__.'] Kafka Topic Configuration never init before, initing...');
        $kTopicCnf = new RdKafka\TopicConf();
// -1必须等所有brokers同步完成的确认 1当前服务器确认 0不确认，这里如果是0回调里的offset无返回，如果是1和-1会返回offset
// 我们可以利用该机制做消息生产的确认，不过还不是100%，因为有可能会中途kafka服务器挂掉
        $kTopicCnf->set(KAFKA_TOPIC_ACK, KAFKA_TOPIC_ACK_VALUE);
    }
}

function notify_kafka_update_score($msg) {
    global $kCnf;
    global $kProducerCnf;
    global $kTopicCnf;

    __check_kafka_cnf_exist_and_init($kCnf, $kProducerCnf, $kTopicCnf);

    $kTopic = $kProducerCnf->newTopic(KAFKA_NOTE_UPDATE_SCORE, $kTopicCnf);
    $option = KAFKA_NOTE_ARTICLE_OPTION;

    $kTopic->produce(RD_KAFKA_PARTITION_UA, 0, $msg, $option);
}


function click_article($feed){
    $src_data = unpack("Larticle_id/Lauthor_id", $feed["data"]);
    if ($src_data['author_id'] == $feed['user_id']) {
        //log::write("Do not support click myself".print_r($feed, true), "warn");
        return 0;
    }
    
    notify_kafka_update_score(__notify_gen_message($feed['user_id'], $src_data['article_id'], $feed['cmd_id'], $src_data['author_id']));
}
