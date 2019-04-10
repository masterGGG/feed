<?php
function data_log($feed)
{
        clearstatcache();
        $str_log_file = './log/data'.posix_getpid().'.log';
        $str_log = pack("SSLCLL",$feed["len"],$feed["cmd_id"],$feed["user_id"],$feed["version"],$feed["timestamp"],$feed["app_id"]);
        $str_log .= $feed["data"];
        $handle = fopen($str_log_file, 'ab');
        if ($handle) {
                fwrite($handle,$str_log);
                fclose($handle) ;
                return true;
        }
        return false;
}

function data_log_pre($feed,$pre)
{
        clearstatcache();
        $str_log_file = './log/'.$pre.'_data'.posix_getpid().'.log';
        $str_log = pack("SSLCLL",$feed["len"],$feed["cmd_id"],$feed["user_id"],$feed["version"],$feed["timestamp"],$feed['app_id']);
        $str_log .= $feed["data"];
        $handle = fopen($str_log_file, 'ab');
        if ($handle) {
                fwrite($handle,$str_log);
                fclose($handle) ;
                return true;
        }
        return false;
}

function data_log_binary($pack, $pre)
{
        clearstatcache();
        $str_log_file = './log/'.$pre.'_data'.posix_getpid().'.log';
        $handle = fopen($str_log_file, 'ab');
        if ($handle) {
                fwrite($handle,$pack);
                fclose($handle) ;
                return true;
        }
        return false;
}

function is_feed_valid($feed)
{
    global $g_sys_conf;
    if (!array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["operator"]))
    {
        log::write("the feed's cmd_id[{$feed["cmd_id"]}] doesn't Configure in operator array","warn");
        return -1; 
    }
    if (!array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["valid_len"]))
    {
        log::write("the feed's cmd_id[{$feed["cmd_id"]}] doesn't Configure in valid_len array","warn");
        return -1; 
    }
    if ($g_sys_conf["feed"]["valid_len"][$feed["cmd_id"]] != $feed["len"] && $g_sys_conf['feed']['valid_len'][$feed['cmd_id']] != 0)
    {
        log::write("the feed's len is unvalid cmd_id {$feed['cmd_id']}:{$feed["len"]}, correct len is {$g_sys_conf["feed"]["valid_len"][$feed["cmd_id"]]}","warn");
        data_log_pre($feed,'unvalid_'.date('Ymd'));
        return -1; 
    }

    $now_time = time() + 3600;
    if ($feed['timestamp'] >= $now_time || $feed['timestamp'] < $now_time -3600 -60)
    {
        log::write("the feed's time > now + 3600, it is impossible. feed timestamp:{$feed['timestamp']} now timestamp + 3600:{$now_time}", 'warn');
        return -1;
    }

    return 0; 
}

function passive_feed_handler($storage_server_socket, $feed, $is_dest, &$retval)
{
    global $g_sys_conf;
    if ($feed['sender_uid'] == $feed['target_uid'])
    {
        log::write("sender_uid == target_uid",'warn');
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
                log::write("feed['flag'] is unvalid", "warn");
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
                        log::write(__FUNCTION__ ."get feed none", "warn");
                        return -2;
                    }
                    else
                    {
                        log::write(__FUNCTION__ .'get feed fail ', "error");
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
                    log::write('unvalid cmd_id', 'warn');
                    return -2;
                }
            }
            else
            {
                log::write('unvalid cmd_id', 'warn');
                return -2;
            }
        }
    }
    else
    {
        log::write(__FUNCTION__ .'get feed fail ', "error");
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
        log::write("operate_feed_to_db update id duplicate dont happen", "warn");
        return -2;
    }
    else if ($ret != 0)
    {
        log::write(__FUNCTION__."[".__LINE__."]"."update to storage_server fail", "error");
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
            log::write(__FUNCTION__ .'pack len is unvalid '.strlen($pack), 'warn');
            return -2;
        }
        $storage_server_socket = $input_arr['storage_server_socket'];
        $feed = unpack('Slen/Sopt_id/Lsender_id/Cversion/Ltimestamp/Lapp_id/Lmagic1/Lmagic2/Scmd_id/Luser_id/Cevaluate', $pack);

        if (!in_array($feed['cmd_id'], $allow_ce))
        {
            log::write(__FUNCTION__ .'the cmd_id cannot evaluate '. $feed['cmd_id'], 'warn');
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
            log::write(__FUNCTION__ ."get feed none", "warn");
            return -2;
        }
        else
        {
            log::write(__FUNCTION__ .'get feed fail ', "error");
            //data_log_binary($pack, __FUNCTION__);
            return -1;
        }

        $ret = operate_feed_to_db($storage_server_socket, $feed, $feed_content); 
        if ($ret == 1)
        {
            log::write(__FUNCTION__."[".__LINE__."]"."operate_feed_to_db update id duplicate dont happen", "warn");
            //data_log_binary($pack,"feed_update_sync_warn");
            return -2;
        }
        else if ($ret != 0)
        {
            log::write(__FUNCTION__."[".__LINE__."]"."update to storage_server fail", "error");
            //data_log_binary($pack, __FUNCTION__);
            return -1;
        }
        return 0;
    }
    else if($feed['opt_id'] == 202)
    {
        $cmd_arr_only_passive = array(7023,7024);
        if (912 != strlen($pack))
        {
            log::write(__FUNCTION__."[".__LINE__."]".'pack len is unvalid '.strlen($pack), 'warn');
            return -2;
        }
        $storage_server_socket = $input_arr['storage_server_socket'];
        $feed = unpack('Slen/Sopt_id/Lsender_uid/Cversion/Ltimestamp/Lapp_id/Lmagic1/Lmagic2/Scmd_id/Luser_id/Cflag/a430source_content/Lsource_id/Lsource_timestamp/a430comment_content/Lcomment_timestamp/Lcomment_id/Ltarget_uid', $pack);

        if (!in_array($feed['cmd_id'], $allow_ce))
        {
            log::write(__FUNCTION__."[".__LINE__."]" .'the cmd_id cannot comment '. $feed['cmd_id'], 'warn');
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
                log::write(__FUNCTION__."[".__LINE__."]"."operate_feed_to_db update id duplicate dont happen", "warn");
                //data_log_binary($pack,"feed_update_sync_warn");
                return -2;
            }
            else if ($ret != 0)
            {
                log::write(__FUNCTION__."[".__LINE__."]"."update to storage_server fail", "error");
                //data_log_binary($pack, __FUNCTION__);
                return -1;
            }
        }
        else if ($ret == 1)
        {
            if (!in_array($feed['cmd_id'], $cmd_arr_only_passive))
            {
                log::write(__FUNCTION__ ."get feed none", "warn");
            }
        }
        else
        {
            log::write(__FUNCTION__ .'get feed fail ', "error");
            //data_log_binary($pack, __FUNCTION__);
            return -1;
        }

        //**************************
        //*****更新或生产passive_feed******
        //更新对方的被动
        $ret = passive_feed_handler($storage_server_socket, $feed, true, $ret_val);
        if ($ret == -2)
        {
            log::write(__FUNCTION__."[".__LINE__."]"."passive_feed_handler desc parameter problem", "warn");
            //data_log_binary($pack,"feed_update_sync_warn");
            return -2;
        }
        else if ($ret != 0)
        {
            log::write(__FUNCTION__."[".__LINE__."]".'passive_feed_handler process fail ', "error");
            //data_log_binary($pack, __FUNCTION__);
            return -1;
        }

        //更新自己的被动
        $ret = passive_feed_handler($storage_server_socket, $feed, false, $ret_val1);
        if ($ret == -2)
        {
            log::write(__FUNCTION__."[".__LINE__."]"."passive_feed_handler my parameter problem", "warn");
            //data_log_binary($pack,"feed_update_sync_warn");
            return -2;
        }
        else if ($ret != 0)
        {
            log::write(__FUNCTION__."[".__LINE__."]".'passive_feed_handler process fail ', "error");
            data_log_binary($pack, __FUNCTION__);
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
        log::write(__FUNCTION__ . " cmd_id is unvalid ". $feed['opt_id'], "warn");
        return -2;
    }
    return 0;
}

function feeddelete($pack, $input_arr, &$output_arr)
{
    global $g_sys_conf;
    $feed = unpack('Slen/Sopt_id', $pack);
    if ($feed['opt_id'] == 1)
    {
        if (strlen($pack) != 59)
        {
            log::write('pack len is unvalid '.strlen($pack), 'warn');
            return -2;
        }

        $feed = unpack('Slen/Sopt_id/Luser_id/Cversion/Ltimestamp/Lapp_id/Lmagic1/Lmagic2/Scmd_id/a32verify',$pack);
        $str = substr($pack, 0, 27);
        $str .= DELETE_SECRET_KEY;
        $verify = md5($str);
        if ($verify != $feed['verify'])
        {
            log::write("The packet doesn't pass verify","warn");
            return -2;
        }

        $storage_server_socket = $input_arr['storage_server_socket'];    
        if (array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["user_defined_id"]))
        {
            if (false == delete_feed_user_defined($storage_server_socket, $feed))
            {
                log::write(__FUNCTION__."[".__LINE__."]"."delete feed fail from storage_server", "error");
                data_log_binary($pack, __FUNCTION__);
                return -1;
            }
        }
        else
        {
            if (false == delete_feed($storage_server_socket, $feed))
            {
                log::write(__FUNCTION__."[".__LINE__."]"."delete feed fail from storage_server", "error");
                data_log_binary($pack, __FUNCTION__);
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
            log::write('pack len is unvalid '.strlen($pack), 'warn');
            return -2;
        }

        $feed = unpack('Slen/Sopt_id/Lsender_uid/Cversion/Ltimestamp/Lapp_id/Lmagic1/Lmagic2/Scmd_id/Luser_id/Ltarget_uid/a32verify',$pack);
        $feed['passive_magic'] = 0;
        $str = substr($pack, 0, 35);
        $str .= DELETE_SECRET_KEY;
        $verify = md5($str);
        if ($verify != $feed['verify'])
        {
            log::write("The packet doesn't pass verify","warn");
            return -2;
        }

        $storage_server_socket = $input_arr['storage_server_socket'];    

            if (false == delete_passive_feed($storage_server_socket, $feed))
            {
                log::write(__FUNCTION__."[".__LINE__."]"."delete feed fail from storage_server", "error");
                data_log_binary($pack, __FUNCTION__);
                return -1;
            }

        $output_arr = array(array(
                                   'feed_op' => 'delete',
                                   'type' => 'passive',
                                  'delete_feedid' => $feed ));
    }
    else
    {
        log::write(__FUNCTION__ . " cmd_id is unvalid ". $feed['opt_id'], "warn");
        return -2;
    }
    return 0;
}

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
    $redis_server_socket = $input_arr['redis_server_socket'];

    if (!array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["operator"]))
    {
        log::write("the feed's cmd_id[{$feed["cmd_id"]}] doesn't Configure","warn");
        return -2; 
    }
 
    if (array_key_exists($feed["cmd_id"], $g_sys_conf["feed"]["iscombine"]))
    {
            if (false == get_comfeed($storage_server_socket, $feed["user_id"], $feed["cmd_id"], $feed["timestamp"], $feed['app_id'], $comfeed))
            {
                log::write("get_comfeed get combine feed fail from storage server", "error");
                data_log($feed);
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
            log::write("feed operator error:".print_r($feed, true), "error");
            data_log($feed);
            return -1;
        }
        else if(-2 == $ret)
        {
            log::write("feed operator http request get data is null or feed data is invalid error:".print_r($feed, true), "warn");
            data_log_pre($feed,"get_null");
            return -2;
        }
        else
        {
            log::write("feed operator return unvalid value[{$ret}]".print_r($feed, true), "error");
            data_log($feed);
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
                    data_log_pre($feed,"user_defined_id_duplicate");
                    return -2;
                } 
                else if ($ret != 0)
                {
                    log::write(__FUNCTION__."[".__LINE__."]"."update completefeed to storage_server fail", "error");
                    //data_log($feed);
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
                    data_log_pre($feed,"user_defined_id_duplicate");
                    return -2;
                }
                else if ($ret != 0)
                {
                    log::write(__FUNCTION__."[".__LINE__."]"."insert completefeed to storage_server fail", "error");
                    data_log($feed);
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
                    log::write(__FUNCTION__."[".__LINE__."]"."update completefeed to storage_server fail", "error");
                    data_log($feed);
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
                    log::write(__FUNCTION__."[".__LINE__."]"."insert completefeed to storage_server fail", "error");
                    data_log($feed);
                    return -1;
                }
                array_push($output_arr,array(
                        'feed_op' => 'insert',
                        'type' => 'active',
                        'insert_feedid' => $completefeed
                            ));
            }
        }
        $ret = operate_update_feed_id_to_db($redis_server_socket, $feed["user_id"], $feed["timestamp"]);
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
                log::write(__FUNCTION__."[".__LINE__."]"."update to storage_server fail" . print_r($val, true), "error");
                return -1;
            }
            $output_arr[] = array(
                'feed_op' => 'insert',
                'type' => 'passive',
                'insert_feedid' => $val);
        }
    } 
        
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
       log::write($query_url, "error"); 
    }
    DEBUG && log::write("url:".$query_url."content:".$ret,"debug");
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

function news_article($feed, &$comfeed, &$completefeed, &$passive_feed)
{ 
    $completefeed["user_id"] = $feed["user_id"];
    
    $completefeed["cmd_id"] = $feed["cmd_id"];
    $completefeed["timestamp"] = $feed["timestamp"];
   
    $src_data = unpack("Larticle_id", $feed["data"]);
    $feed["data"] = substr($feed["data"], 4);
    /*
    //1. 解析icon Url的长度并根据长度解析出url
    $parse = unpack("Clen", $feed["data"]);
    $src_data["icon"] = substr($feed["data"], 1, $parse["len"]);
    $feed["data"] = substr($feed["data"], 1 + $parse["len"]);
    
    //2. 先解析昵称长度，再根据昵称长度获取昵称
    $parse = unpack("Clen", $feed["data"]);
    $src_data["nickName"] = substr($feed["data"], 1, $parse["len"]);
    $feed["data"] = substr($feed["data"], 1 + $parse["len"]);
    */
    //3. 解析首张图片的长度并根据长度获取url
    $parse = unpack("Clen", $feed["data"]);
    $src_data["pic"] = substr($feed["data"], 1, $parse["len"]);
    /*
    $feed["data"] = substr($feed["data"], 1 + $parse["len"]);
    
    //4. 解析帖子简介
    $parse = unpack("Clen", $feed["data"]);
    $src_data["text"] = substr($feed["data"], 1, $parse["len"]);
    */
    $completefeed["data"] = json_encode($src_data);
    return 0;
}

function news_liker($feed, &$comfeed, &$completefeed, &$passive_feed)
{
    $completefeed["user_id"] = $feed["user_id"];
    
    $completefeed["cmd_id"] = $feed["cmd_id"];
    $completefeed["timestamp"] = $feed["timestamp"];
    
    $src_data = unpack("Larticle_id/Lauthor_id", $feed["data"]);
    $src_data['liked_id'] = $feed["user_id"];
    $json_src_data = json_encode($src_data);
    // 通知博主有人点赞了我的帖子
    $passive_feed[] = produce_passive_feed($feed, $feed['user_id'], $src_data['author_id'], $json_src_data);
    // 通知博主的粉丝有人点赞了好友的帖子
    if (init_connect_and_nonblock(RELATION_IP, RELATION_PORT, $relation_socket))
    {
        log::write("init_connect relation_server fail reason: connect to relation_server", "error");
        return -1;
    }

    $relation_rqst = pack("LLsLL", 18, 0, 0xB104, 0, $src_data['author_id']);
    if (send_data_and_nonblock($relation_socket, $relation_rqst, TIMEOUT)) {
        log::write('get_fans_id: relation_client->send_rqst', 'error');
        socket_close($relation_socket);
        return -1;
    }

    $relation_resp = "";
    if (recv_data_and_nonblock($relation_socket, 18, $relation_resp, TIMEOUT))
    {
        log::write("recv data to relation_server fail", "error");
        socket_close($relation_socket);
        return -1;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $relation_resp);
    if ($rv['code'] != 0) {
        log::write('relation server interal fail', 'error');
        socket_close($relation_socket);
        return -1;
    }

    if (recv_data_and_nonblock($relation_socket, $rv['len'], $relation_resp, TIMEOUT))
    {
        log::write("recv data to relation_server fail", "error");
        socket_close($relation_socket);
        return -1;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi/Lunits', $relation_resp);
    $id_list = substr($relation_resp, 18 + 4);
    for ($i = 0; $i < $rv['units']; ++$i) {
        $fan = unpack('Lid/Ltime', $id_list);
        $id_list = substr($id_list, 8);
        if ($fan['id'] === $feed['user_id'])
            continue;
        $passive_feed[] = produce_passive_feed($feed, $feed['user_id'], $fan['id'], $json_src_data);
    }    
    $completefeed['data'] = json_encode($src_data);
    return 0;
}
function news_comment($feed, &$comfeed, &$completefeed, &$passive_feed)
{
    $completefeed["user_id"] = $feed["user_id"];
    
    $completefeed["cmd_id"] = $feed["cmd_id"];
    $completefeed["timestamp"] = $feed["timestamp"];
    
    $src_data = unpack("Ltype/Larticle_id/Lauthor_id/Lcomment_mid/Lcomment_id", $feed["data"]);
    $src["reply_id"] = $feed["user_id"];
    $json_src_data = json_encode($src_data);
    //通知被评论者，谁评论了我(被动feed)
    if ($src_data["comment_id"] != $feed["user_id"])
        $passive_feed[] = produce_passive_feed($feed, $feed['user_id'], $src_data['comment_mid'], $json_src_data);
    
    //通知博主，有新评论(被动feed)
    if ($src_data["comment_id"] != $src_data["author_id"] && $src_data["author_id"] != $feed["user_id"])
        $passive_feed[] = produce_passive_feed($feed, $feed['user_id'], $src_data['author_id'], $json_src_data);

    $completefeed['data'] = json_encode($src_data);
    return 0;
}
/*10
function news_($feed, &$comfeed, &$completefeed, &$passive_feed)
{
    $completefeed["user_id"] = $feed["user_id"];
    
    $completefeed["cmd_id"] = $feed["cmd_id"];
    $completefeed["timestamp"] = $feed["timestamp"];
    
    $completefeed["data"] = "{}";
    return 0;
}
*/
