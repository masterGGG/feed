<?php
function send_data($socket, $pack)
{
    while(strlen($pack) > 0)
    {
        $len = socket_write($socket, $pack, strlen($pack));
        if ($len === false)
        {
            if (socket_last_error($socket) == SOCKET_EINTR)
            {
                continue;
            }
            else
            {
                log::write("[".__FILE__."]:[".__LINE__."]"."socket_write fail: reason: ".socket_strerror(socket_last_error($socket)). "\n", "error");
                return -1;
            }
        }
        $pack = substr($pack, $len); 
    }
    return 0;
}

function send_data_and_nonblock($socket, $pack, $timeout)
{
    $s_time = time();
    while(strlen($pack) > 0)
    {
        $len = socket_write($socket, $pack, strlen($pack));
        if ($len === false)
        {
            if (socket_last_error($socket) == SOCKET_EINTR || socket_last_error($socket) == SOCKET_EAGAIN)
            {
                usleep(1);
                continue;
            }
            else
            {
                log::write("[".__FILE__."]:[".__LINE__."]"."socket_write fail: reason: ".socket_strerror(socket_last_error($socket)), "warn");
		socket_close($socket);
                return -1;
            }
        }
        else
        {
            $pack = substr($pack, $len); 
            $s_time = time();
        }

        if (time() - $s_time > $timeout)
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."timeout send_data_nonblock","warn");            
            socket_close($socket);
            return -1;
        }
        usleep(100);
    }
    return 0;
}

function recv_data_and_nonblock($socket, $pack_size, &$pack, $timeout)
{
    $s_time = time();
    while(strlen($pack) < $pack_size)
    {
        $recv_data = socket_read($socket, 4096);
        if ($recv_data === false)
        {
            if (socket_last_error($socket) == SOCKET_EINTR || socket_last_error($socket) == SOCKET_EAGAIN)
            {
                usleep(1);
                continue;
            }
            else
            {
                log::write("[".__FILE__."]:[".__LINE__."]"."socket_read fail:reason: ".socket_strerror(socket_last_error($socket)),"warn");
                socket_close($socket);
                return -1;
            }
        } 
        else if ($recv_data == "")
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."socket_read zero bytes:reason:".socket_strerror(socket_last_error($socket)), "warn");
            socket_close($socket);
            return -1;
        }
        else
        {
            $pack .= $recv_data;
            $s_time = time();
        }

        if (time() - $s_time > $timeout)
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."timeout send_data_nonblock","warn");            
            socket_close($socket);
            return -1;
        }
        usleep(100);
    } 
    return 0;
}

function recv_data($socket, $pack_size, &$pack)
{
    while(strlen($pack) < $pack_size)
    {
        $recv_data = socket_read($socket, 4096);
        if ($recv_data === false)
        {
            if (socket_last_error($socket) == SOCKET_EINTR)
            {
                continue;
            }
            else
            {
                log::write("[".__FILE__."]:[".__LINE__."]"."socket_read fail:reason: ".socket_strerror(socket_last_error($socket)),"error");
                return -1;
            }
        } 
        if ($recv_data == "")
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."socket_read zero bytes:reason:".socket_strerror(socket_last_error($socket)), "error");
            return -1;
        }
        $pack .= $recv_data;
    } 
    return 0;
}

function init_net($ip, $port, &$socket, &$pack)
{
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if ($socket === false)
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."socket_create fail: reason: ".socket_strerror(socket_last_error()),"error"); 
        return -1;
    }
    $result = socket_connect($socket, $ip, $port);
    if ($result === false)
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."socket_connect() fail:reason: ".socket_strerror(socket_last_error($socket)),"error");
        return -1;
    } 

    //发送包 进行协商
    $pack = pack("S3", 6, 0xffff, 1);

    if (send_data($socket, $pack))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."socket_send fail in init_net: ".socket_strerror(socket_last_error($socket)),"error");
        return -1;
    }

    $pack = "";
    if (recv_data($socket, 4, $pack))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."socket_recv fail in init_net: ".socket_strerror(socket_last_error($socket)),"error");
        return -1;
    }

    return 0;
}

function init_connect_and_nonblock($ip, $port, &$socket)
{
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if ($socket === false)
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."socket_create fail: reason: ".socket_strerror(socket_last_error()),"warn"); 
        socket_close($socket);
        return -1;
    }
    $result = socket_connect($socket, $ip, $port);
    if ($result === false)
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."socket_connect() fail:reason: ".socket_strerror(socket_last_error($socket)),"warn");
        socket_close($socket);
        return -1;
    } 
    socket_set_nonblock($socket);
    return 0;
}

function init_connect($ip, $port, &$socket)
{
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if ($socket === false)
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."socket_create fail: reason: ".socket_strerror(socket_last_error()),"error"); 
        return -1;
    }
    $result = socket_connect($socket, $ip, $port);
    if ($result === false)
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."socket_connect() fail:reason: ".socket_strerror(socket_last_error($socket)),"error");
        return -1;
    } 
    return 0;
}

function get_feed_according_time($storage_server_socket, $mimi_id, $cmd_id, $timestamp, $app_id, $prev_num, $next_num, &$feeds)
{
    if (!isset($mimi_id,$cmd_id,$timestamp,$app_id)) 
    {
        log::write("[".__FILE__."]:[".__LINE__."]".__FUNCTION__." para unset mimi_id:{$mimi_id} cmd_id:{$cmd_id} timestamp:{$timestamp} app_id:{$app_id}","error");
        return false;
    }
    
    $request_pack_format = array("LS2LS2L2S2");
    $request_pack_content = array(
                            'len' => 28,
                            'op' => 11,
                            'units' => 1,
                            'mimi' => $mimi_id,
                            'flag' => 0x7,
                            'cmd_id' => $cmd_id,
                            'app_id' => $app_id,
                            'starttime' => $timestamp,
                            'prev_num' => $prev_num, 
                            'next_num' => $next_num 
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_feed_according_time function request storage error ret:{$response_pack_content['ret']}","error");
            return false;     
    }
    
    $units = $response_pack_content['units']; 
    $response = substr($response_pack, 8);
    for($i = 0; $i < $units; $i++)
    {
        $row = unpack("Llen/Lmimi/Scmd_id/Lapp_id/Ltimestamp/L2magic", $response);
        $row['data'] = substr($response, 26, $row['len'] - 26); 
        $feeds[$i] = array('id' => array($row['mimi'],
            $row['cmd_id'],
            $row['app_id'],
            $row['timestamp'],
            $row['magic1'],
            $row['magic2']
        ),
        'user_id'=>$row['mimi'],
        'cmd_id'=>$row['cmd_id'],
        'app_id'=>$row['app_id'],
        'timestamp'=>$row['timestamp'],
        'magic1' => $row['magic1'],
        'magic2' => $row['magic2'],
        'data'=>$row['data']);
        $response = substr($response, $row['len']); 
    }
    return true;
}

//选取用于合并的feed 
function get_comfeed($storage_server_socket, $mimi_id, $cmd_id, $timestamp, $app_id, &$feeds)
{
    if (!isset($mimi_id,$cmd_id,$timestamp,$app_id)) 
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed para unset mimi_id:{$mimi_id} cmd_id:{$cmd_id} timestamp:{$timestamp} app_id:{$app_id}","error");
        return false;
    }
    
    $request_pack_format = array("LS2LS2L3");
    $request_pack_content = array(
                            'len' => 28,
                            'op' => 12,
                            'units' => 1,
                            'mimi' => $mimi_id,
                            'flag' => 0x7,
                            'cmd_id' => $cmd_id,
                            'app_id' => $app_id,
                            'starttime' => $timestamp - 4 * 3600,
                            'endtime' => $timestamp + 600 
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    //DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
            return false;     
    }
    
    $units = $response_pack_content['units']; 
    $response = substr($response_pack, 8);
    for($i = 0; $i < $units; $i++)
    {
        $row = unpack("Llen/Lmimi/Scmd_id/Lapp_id/Ltimestamp/L2magic", $response);
        $row['data'] = substr($response, 26, $row['len'] - 26); 
        $feeds[$i] = array('id' => array($row['mimi'],
            $row['cmd_id'],
            $row['app_id'],
            $row['timestamp'],
            $row['magic1'],
            $row['magic2']
        ),
        'user_id'=>$row['mimi'],
        'cmd_id'=>$row['cmd_id'],
        'app_id'=>$row['app_id'],
        'timestamp'=>$row['timestamp'],
        'magic1' => $row['magic1'],
        'magic2' => $row['magic2'],
        'data'=>$row['data']);
        $response = substr($response, $row['len']); 
    }
    return true;
}

function get_comfeed_for_news_reply($storage_server_socket, $mimi_id, $cmd_id, $timestamp, $app_id, $reply_key, &$feeds)
{
    if (!isset($mimi_id,$cmd_id,$timestamp,$app_id, $reply_key["target_appid"],$reply_key["target_uid"],$reply_key["target_id"])) 
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed_for_news_reply para unset mimi_id:{$mimi_id} cmd_id:{$cmd_id} timestamp:{$timestamp} app_id:{$app_id} reply_key[target_appid,target_uid,target_id]".print_r($reply_key,true) ,"error");
        return false;
    }

    $request_pack_format = array("LS2LS2L3");
    $request_pack_content = array(
        'len' => 28,
        'op' => 12,
        'units' => 1,
        'mimi' => $mimi_id,
        'flag' => 0x7,
        'cmd_id' => $cmd_id,
        'app_id' => $app_id,
        'starttime' => $timestamp - 4 * 3600,
        'endtime' => $timestamp + 600 
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
        return false;     
    }

    $units = $response_pack_content['units']; 
    $response = substr($response_pack, 8);
    $ii = 0;
    for($i = 0; $i < $units; $i++)
    {
        $row = unpack("Llen/Lmimi/Scmd_id/Lapp_id/Ltimestamp/L2magic", $response);
        $row['data'] = substr($response, 26, $row['len'] - 26); 
        $temp_key = json_decode($row['data'],true);
        if ($temp_key["target_appid"] == $reply_key["target_appid"] && $temp_key["target_uid"] == $reply_key["target_uid"] && $temp_key["target_id"] == $reply_key["target_id"])
        {
            $feeds[$ii] = array('id' => array($row['mimi'],
                $row['cmd_id'],
                $row['app_id'],
                $row['timestamp'],
                $row['magic1'],
                $row['magic2']
            ),
            'user_id'=>$row['mimi'],
            'cmd_id'=>$row['cmd_id'],
            'app_id'=>$row['app_id'],
            'timestamp'=>$row['timestamp'],
            'magic1' => $row['magic1'],
            'magic2' => $row['magic2'],
            'data'=>$row['data']);
            $ii++;
        }
        $response = substr($response, $row['len']); 
    }
    return true;
}

function get_comfeed_time_span($storage_server_socket, $mimi_id, $cmd_id, $starttime, $endtime, $app_id, &$feeds)
{
    if (!isset($mimi_id,$cmd_id,$starttime,$endtime,$app_id)) 
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed para unset mimi_id:{$mimi_id} cmd_id:{$cmd_id} starttime:{$starttime} endtime:{$endtime} app_id:{$app_id}","error");
        return false;
    }
    
    $request_pack_format = array("LS2LS2L3");
    $request_pack_content = array(
                            'len' => 28,
                            'op' => 12,
                            'units' => 1,
                            'mimi' => $mimi_id,
                            'flag' => 0x7,
                            'cmd_id' => $cmd_id,
                            'app_id' => $app_id,
                            'starttime' => $starttime,
                            'endtime' => $endtime 
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
            return false;     
    }
    
    $units = $response_pack_content['units']; 
    $response = substr($response_pack, 8);
    for($i = 0; $i < $units; $i++)
    {
        $row = unpack("Llen/Lmimi/Scmd_id/Lapp_id/Ltimestamp/L2magic", $response);
        $row['data'] = substr($response, 26, $row['len'] - 26); 
        $feeds[$i] = array('id' => array($row['mimi'],
            $row['cmd_id'],
            $row['app_id'],
            $row['timestamp'],
            $row['magic1'],
            $row['magic2']
        ),
        'user_id'=>$row['mimi'],
        'cmd_id'=>$row['cmd_id'],
        'app_id'=>$row['app_id'],
        'timestamp'=>$row['timestamp'],
        'magic1' => $row['magic1'],
        'magic2'=> $row['magic2'],
        'data'=>$row['data']);
        $response = substr($response, $row['len']); 
    }
    return true;
}

function delete_feed_user_defined($storage_server_socket, $key)
{
    $mimi_id = $key['user_id'];
    $cmd_id = $key['cmd_id'];
    $app_id = $key['app_id'];
    $magic1 = $key['magic1'];
    $magic2 = $key['magic2'];
    if (!isset($mimi_id,$cmd_id,$app_id,$magic1,$magic2)) 
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."delete_feed para unset mimi_id:{$mimi_id} cmd_id:{$cmd_id} app_id:{$app_id} magic1:{$magic1} magic2:{$magic2}","error");
        return false;
    }
    
    $request_pack_format = array("LS2LSL3");
    $request_pack_content = array(
                            'len' => 26,
                            'op' => 5,
                            'units' => 1,
                            'mimi' => $mimi_id,
                            'cmd_id' => $cmd_id,
                            'app_id' => $app_id,
                            'magic1' => $magic1,
                            'magic2' => $magic2 
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."delete_feed function request storage error ret:{$response_pack_content['ret']}","error");
            return false;     
    }
    return true;
}

function delete_feed($storage_server_socket, $key)
{
    $mimi_id = $key['user_id'];
    $cmd_id = $key['cmd_id'];
    $app_id = $key['app_id'];
    $timestamp = $key['timestamp'];
    $magic1 = $key['magic1'];
    $magic2 = $key['magic2'];
    if (!isset($mimi_id,$cmd_id,$app_id,$magic1,$magic2)) 
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."delete_feed para unset mimi_id:{$mimi_id} cmd_id:{$cmd_id} app_id:{$app_id} magic1:{$magic1} magic2:{$magic2}","error");
        return false;
    }
    
    $request_pack_format = array("LS2LSL4");
    $request_pack_content = array(
                            'len' => 30,
                            'op' => 4,
                            'units' => 1,
                            'mimi' => $mimi_id,
                            'cmd_id' => $cmd_id,
                            'app_id' => $app_id,
                            'timestamp' => $timestamp,
                            'magic1' => $magic1,
                            'magic2' => $magic2 
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."delete_feed function request storage error ret:{$response_pack_content['ret']}","error");
            return false;     
    }
    return true;
}

//todo
function delete_passive_feed($storage_server_socket, $key)
{
    $mimi_id = $key['user_id'];
    $cmd_id = $key['cmd_id'];
    $app_id = $key['app_id'];
    $timestamp = $key['timestamp'];
    $magic1 = $key['magic1'];
    $magic2 = $key['magic2'];
    if (!isset($mimi_id,$cmd_id,$app_id,$magic1,$magic2)) 
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."delete_feed para unset mimi_id:{$mimi_id} cmd_id:{$cmd_id} app_id:{$app_id} magic1:{$magic1} magic2:{$magic2}","error");
        return false;
    }
    
    $request_pack_format = array("LS2LSL4L3");
    $request_pack_content = array(
                            'len' => 42,
                            'op' => 23,
                            'units' => 1,
                            'mimi' => $mimi_id,
                            'cmd_id' => $cmd_id,
                            'app_id' => $app_id,
                            'timestamp' => $timestamp,
                            'magic1' => $magic1,
                            'magic2' => $magic2,
                            'sender_uid' => $key['sender_uid'],
                            'target_uid' => $key['target_uid'],
                            'passive_magic' => $key['passive_magic']
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return false;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."delete_feed function request storage error ret:{$response_pack_content['ret']}","error");
            return false;     
    }
    return true;
}


function operate_passive_feed_to_db($storage_server_socket, $key, $feed)
{
    if (isset($key) && !is_null($key))  //update
    {
        $temp = strlen($feed['data']); 
        $request_pack_format = array("LS2LSL4L3LLSL7La{$temp}");
        $request_pack_content = array(
            'len' => 84 + $temp,
            'op' => 22,
            'units' => 1,
            'old_mimi' => $key['user_id'],
            'old_cmd_id' => $key['cmd_id'],
            'old_app_id' => $key['app_id'],
            'old_timestamp' => $key['timestamp'],
            'old_magic1' => $key['magic1'],
            'old_magic2' => $key['magic2'],
            'old_sender_uid' => $key['sender_uid'],
            'old_target_uid' => $key['target_uid'],
            'old_passive_magic' => $key['passive_magic'],
            'new_p_feed_len' => 42 + $temp,
            'new_mimi' => $feed['user_id'],
            'new_cmd_id' => $feed['cmd_id'],
            'new_app_id' => $feed['app_id'],
            'new_timestamp' => $feed['timestamp'],
            'new_magic1' => $feed['magic1'],
            'new_magic2' => $feed['magic2'],
            'new_sender_uid' => $feed['sender_uid'],
            'new_target_uid' => $feed['target_uid'],
            'new_passive_magic' => $feed['passive_magic'],
            'new_update_time' => $feed['update_timestamp'],
            'data' => $feed['data'] 
        );
        $para = array_merge($request_pack_format, $request_pack_content);
        $request_pack = call_user_func_array('pack',array_values($para));

        DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

        if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }
        $response_pack = "";
        if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }
        $temp = unpack("Llen", $response_pack);
        if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }

        $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 

        if ($response_pack_content['ret'] == 2)
        {
            return 1;
        }

        if ($response_pack_content['ret'] != 0)
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
            return -1;     
        }
    }
    else
    {
        $temp = strlen($feed['data']); 
        $request_pack_format = array("LS2LLSL7La{$temp}");
        $request_pack_content = array(
            'len' => 50 + $temp,
            'op' => 21,
            'units' => 1,
            'pkglen' => 42 + $temp,
            'mimi' => $feed['user_id'],
            'cmd_id' => $feed['cmd_id'],
            'app_id' => $feed['app_id'],
            'timestamp' => $feed['timestamp'],
            'magic1' => $feed['magic1'],
            'magic2' => $feed['magic2'],
            'sender_uid' => $feed['sender_uid'],
            'target_uid' => $feed['target_uid'],
            'passive_magic' => $feed['passive_magic'],
            'update_timestamp' => $feed['update_timestamp'],
            'data' => $feed['data'] 
        );
        $para = array_merge($request_pack_format, $request_pack_content);
        $request_pack = call_user_func_array('pack',array_values($para));

        //DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

        if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }
        $response_pack = "";
        if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }
        $temp = unpack("Llen", $response_pack);
        if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }

        $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 

        if ($response_pack_content['ret'] == 2)
        {
            return 1;
        }

        if ($response_pack_content['ret'] != 0)
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
            return -1;     
        }

    }
    return 0;
}

//$key 要更新的feed的关键字 
function operate_feed_to_db($storage_server_socket, $key, $feed)
{
    //if (!isset($feed["id"],$feed["img_url"],$feed["user_id"],$feed["nick_name"],$feed["cmd_id"],$feed["timestamp"],$feed["data"])) 
    //{
    //    log::write("[".__FILE__."]:[".__LINE__."]"."Mysql operate_feed_to_db para unset feed[id,img_url,user_id,nick_name,cmd_id,timestamp,data]:".print_r($feed,true)."","error");
    //    return false;
    //}

    if (isset($key) && !is_null($key))  //update
    {
        $temp = strlen($feed['data']); 
        $request_pack_format = array("LS2LLSL4LSL4a{$temp}");
        $request_pack_content = array(
            'len' => 56 + $temp,
            'op' => 3,
            'units' => 1,
            'pkdlen' => 48 + $temp,
            'old_mimi' => $key['user_id'],
            'old_cmd_id' => $key['cmd_id'],
            'old_app_id' => $key['app_id'],
            'old_timestamp' => $key['timestamp'],
            'old_magic1' => $key['magic1'],
            'old_magic2' => $key['magic2'],
            'new_mimi' => $feed['user_id'],
            'new_cmd_id' => $feed['cmd_id'],
            'new_app_id' => $feed['app_id'],
            'new_timestamp' => $feed['timestamp'],
            'new_magic1' => $feed['magic1'],
            'new_magic2' => $feed['magic2'],
            'data' => $feed['data'] 
        );
        $para = array_merge($request_pack_format, $request_pack_content);
        $request_pack = call_user_func_array('pack',array_values($para));

        //DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

        if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }
        $response_pack = "";
        if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }
        $temp = unpack("Llen", $response_pack);
        if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }

        $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 

        if ($response_pack_content['ret'] == 2)
        {
            return 1;
        }

        if ($response_pack_content['ret'] != 0)
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
            return -1;     
        }
    }
    else
    {
        $temp = strlen($feed['data']); 
        $request_pack_format = array("LS2LLSL4a{$temp}");
        $request_pack_content = array(
            'len' => 34 + $temp,
            'op' => 1,
            'units' => 1,
            'pkglen' => 26 + $temp,
            'mimi' => $feed['user_id'],
            'cmd_id' => $feed['cmd_id'],
            'app_id' => $feed['app_id'],
            'timestamp' => $feed['timestamp'],
            'magic1' => $feed['magic1'],
            'magic2' => $feed['magic2'],
            'data' => $feed['data'] 
        );
        $para = array_merge($request_pack_format, $request_pack_content);
        $request_pack = call_user_func_array('pack',array_values($para));

        //DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

        if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }
        $response_pack = "";
        if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }
        $temp = unpack("Llen", $response_pack);
        if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
            //socket_close($storage_server_socket);
            return -1;
        }

        $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 

        if ($response_pack_content['ret'] == 2)
        {
            return 1;
        }

        if ($response_pack_content['ret'] != 0)
        {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
            return -1;     
        }

    }
    return 0;
}

function get_feed($storage_server_socket, $key, &$result)
{
    global $g_sys_conf;
    if (array_key_exists($key['cmd_id'], $g_sys_conf["feed"]["user_defined_id"]))//用户自定义
    {
        $request_pack_format = array("LS2LSL3");
        $request_pack_content = array(
            'len' => 26,
            'op' => 13,
            'units' => 1,
            'mimi' => $key['user_id'],
            'cmd_id' => $key['cmd_id'],
            'app_id' => $key['app_id'],
            'magic1' => $key['magic1'],
            'magic2' => $key['magic2'],
        );
        $para = array_merge($request_pack_format, $request_pack_content);
        $request_pack = call_user_func_array('pack',array_values($para));
    }
    else
    {
        $request_pack_format = array("LS2LSL4");
        $request_pack_content = array(
            'len' => 30,
            'op' => 10,
            'units' => 1,
            'mimi' => $key['user_id'],
            'cmd_id' => $key['cmd_id'],
            'app_id' => $key['app_id'],
            'timestamp' => $key['timestamp'],
            'magic1' => $key['magic1'],
            'magic2' => $key['magic2'],
        );
        $para = array_merge($request_pack_format, $request_pack_content);
        $request_pack = call_user_func_array('pack',array_values($para));
    }

    DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return -1;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return -1;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return -1;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
            return -1;     
    }
    
    $units = $response_pack_content['units']; 
    if ($units == 0)
    {
        return 1;
    }
    else if ($units != 1)
    {
        log::write("[".__FILE__."]:[".__LINE__."]".'get record bigger than 1 value:'.$units ." package: ". print_r($request_pack_content, true), 'warn');
    }

    $response = substr($response_pack, 8);
    $row = unpack("Llen/Lmimi/Scmd_id/Lapp_id/Ltimestamp/L2magic", $response);
    $row['data'] = substr($response, 26, $row['len'] - 26); 
    $result = array(
        'user_id'=>$row['mimi'],
        'cmd_id'=>$row['cmd_id'],
        'app_id'=>$row['app_id'],
        'timestamp'=>$row['timestamp'],
        'magic1' => $row['magic1'],
        'magic2'=> $row['magic2'],
        'data'=>$row['data']);
    return 0;
}

function get_passive_feed($storage_server_socket, $key, &$result)
{
    $request_pack_format = array("LS2LSL7");
    $request_pack_content = array(
        'len' => 42,
        'op' => 24,
        'units' => 1,
        'mimi' => $key['user_id'],
        'cmd_id' => $key['cmd_id'],
        'app_id' => $key['app_id'],
        'timestamp' => $key['timestamp'],
        'magic1' => $key['magic1'],
        'magic2' => $key['magic2'],
        'sender_uid' => $key['sender_uid'],
        'target_uid' => $key['target_uid'],
        'passive_magic' => $key['passive_magic']
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    DEBUG && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return -1;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return -1;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        //socket_close($storage_server_socket);
        return -1;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_comfeed function request storage error ret:{$response_pack_content['ret']}","error");
            return -1;     
    }
    
    $units = $response_pack_content['units']; 
    if ($units == 0)
    {
        return 1;
    }
    else if ($units != 1)
    {
        log::write("[".__FILE__."]:[".__LINE__."]".'get record bigger than 1 value:'.$units ." package: ". print_r($request_pack_content, true), 'warn');
    }

    $response = substr($response_pack, 8);
    $row = unpack("Llen/Lmimi/Scmd_id/Lapp_id/Ltimestamp/L2magic/Lsender_uid/Ltarget_uid/Lpassive_magic/Lupdate_timestamp", $response);
    $row['data'] = substr($response, 42, $row['len'] - 42); 
    $result = array(
        'user_id'=>$row['mimi'],
        'cmd_id'=>$row['cmd_id'],
        'app_id'=>$row['app_id'],
        'timestamp'=>$row['timestamp'],
        'magic1' => $row['magic1'],
        'magic2'=> $row['magic2'],
        'sender_uid' => $row['sender_uid'],
        'target_uid' => $row['target_uid'],
        'passive_magic' => $row['passive_magic'],
        'update_timestamp' => $row['update_timestamp'],
        'data'=>$row['data']);
    return 0;
}

function operate_update_feed_id_to_db($redis_server_socket, $user_id, $timestamp) {
    if ($redis_server_socket == FALSE)
        return -1;

    $request = pack("LLsLLL", 22, 1001, 0xA101, 0, $user_id, $timestamp);
    
    if (send_data_and_nonblock($redis_server_socket, $request, TIMEOUT)) {
        log::write("send data to redis_server fail", "error");
        return -1;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($redis_server_socket, 18, $response_pack, TIMEOUT)) {
        log::write("recv data to redis_server fail", "error");
        return -1;
    }
    $temp = unpack("Llen/Lseq/scmd/Lcode/Lmid", $response_pack);
    if ( $temp["code"] != 0 ) {
        log::write("redis_server internal fail", "error");
        return -1;
    }
    return 0;
}

function check_pfeed_unique($storage_server_socket, $target_id, $sender_id, $cmd_id, $app_id) {
    if ($storage_server_socket === FALSE) {
        log::write("Can not connect to_server", "error");
        return FALSE;
    }

    $rqst_format = array('LS3LSL2');
    $rqst_content = array(
        'len' => 24,
        'op' => 27,
        'units' => 1,
        'flag' => 0x7,
        'sender_id' => $sender_id,
        'cmd_id' => $cmd_id + 15000,
        'app_id' => $app_id,
        'target_id' => $target_id
    );

    $rqst_para = array_merge($rqst_format, $rqst_content);
    $rqst_pack = call_user_func_array('pack', array_values($rqst_para));
    if (send_data_and_nonblock($storage_server_socket, $rqst_pack, TIMEOUT)) {
        log::write("send data to storage_server fail", "error");
        return FALSE;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT)) {
        log::write("recv data header from storage_server fail", "error");
        return FALSE;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT)) {
        log::write("recv data body from storage_server fail", "error");
        return FALSE;
    }

    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0) {
            log::write("get passive feed function request storage error ret:{$response_pack_content['ret']}","error");
            return FALSE;     
    }
    
    $units = $response_pack_content['units']; 
    if ($units <= 0) {
        log::write("get 0 feed from storage server", 'debug');
        return FALSE;
    }
    return substr($response_pack, 6);
}

function check_liker_pfeed_unique($storage_server_socket, $target_id, $sender_id, $cmd_id, $app_id, $article_id) {
    $rv =  check_pfeed_unique($storage_server_socket, $target_id, $sender_id, $cmd_id, $app_id);

    if ($rv == FALSE) {
        log::write('['.__LINE__."]recv no data from storage_server", "error");
        return FALSE;
    }
    $response_pack_content = unpack("Sunits", $rv); 
    $units = $response_pack_content['units']; 
    $response = substr($rv, 2); 
    for($i = 0; $i < $units; ++$i) {
//        $row = unpack("Llen/Lmimi/Scmd_id/Lapp_id/Ltimestamp/L2magic/Lsender_uid/Ltarget_uid/Lpassive_magic/Lupdate_timestamp", $response);
        $row = unpack('Llen', $response);
        $data = json_decode(substr($response, 42, $row['len'] - 42), true); 
        log::write("recv data to storage_server data:".print_r($data, true), "error");
        if ($data['article_id'] == $article_id)
            return TRUE;
        $response = substr($response, $row['len']);
    }
    return FALSE;
}
function check_fans_pfeed_unique($storage_server_socket, $target_id, $sender_id, $cmd_id, $app_id, $article_id) {
    $rv =  check_pfeed_unique($storage_server_socket, $target_id, $sender_id, $cmd_id, $app_id);

    if ($rv == FALSE) {
        log::write('['.__LINE__."]recv no data from storage_server", "error");
        return FALSE;
    }
    return TRUE;
}

function get_pfeed_cnt_according_cmd($storage_server_socket, $user_id, $cmd_id, $app_id, &$cnt) {
    if (!isset($user_id, $cmd_id, $app_id)){
        log::write("[".__FILE__."]:[".__LINE__."]".__FUNCTION__." para unset mimi_id:{$mimi_id} cmd_id:{$cmd_id} app_id:{$app_id}","error");
        return false;
    }
    if ($storage_server_socket === false) {
    }
    
    $request_pack_format = array("LS2LSL");
    $request_pack_content = array(
                            'len' => 18,
                            'op' => 28,
                            'units' => 1,
                            'mimi' => $user_id,
                            'cmd_id' => $cmd_id,
                            'app_id' => $app_id
    );
    $para = array_merge($request_pack_format, $request_pack_content);
    $request_pack = call_user_func_array('pack',array_values($para));

    DEBUG_0710 && log::write("[".__FILE__."]:[".__LINE__."]".print_r($request_pack_content, true), "debug");

    if (send_data_and_nonblock($storage_server_socket, $request_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."send data to storage_server fail", "error");
        return false;
    }
    $response_pack = "";
    if (recv_data_and_nonblock($storage_server_socket, 4, $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        return false;
    }
    $temp = unpack("Llen", $response_pack);
    if (recv_data_and_nonblock($storage_server_socket, $temp['len'], $response_pack, TIMEOUT))
    {
        log::write("[".__FILE__."]:[".__LINE__."]"."recv data to storage_server fail", "error");
        return false;
    }
    
    $response_pack_content = unpack("Llen/Sret/Sunits", $response_pack); 
    if ($response_pack_content['ret'] != 0)
    {
            log::write("[".__FILE__."]:[".__LINE__."]"."get_feed_according_time function request storage error ret:{$response_pack_content['ret']}","error");
            return false;     
    }
    
    $units = $response_pack_content['units']; 
    if ($units = 1) {
        $response = unpack("Lcnt", substr($response_pack, 8));
        $cnt = $response['cnt'];
        return true;
    }

    return false;
}
