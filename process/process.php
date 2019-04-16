#! /usr/bin/php
<?php

require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'config' . DIRECTORY_SEPARATOR .  'setup.inc.php';

declare(ticks=1);

$g_pid_arr = array();
$g_pid_file = "./outbox_pid";
$g_stop = 0;

function daemon()
{
    $pid = pcntl_fork();
    if ($pid == -1)
    {
        die("could not fork\n");
    }
    else if ($pid)
    {
        exit();
    }
    else
    {
        if (posix_setsid() == -1)
        {
            die("could not detach from terminal\n");
        }
    }
}

function parent_signal_handler($signo)
{
    global $g_pid_arr;
    switch($signo)
    {
        case SIGCHLD:
            break;
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
            foreach($g_pid_arr as $key => $pid)
            {
                posix_kill($g_pid_arr[$key], SIGUSR1);
            }
            break;
        default:
            break;
    }
}

function child_signal_handler($signo)
{
        global $g_stop;
        switch($signo)
        {
                case SIGPIPE:
                        break;
                case SIGUSR1:
                        $g_stop++;
                        break;
                default:
                        break;
        }
}

function process_control()
{
    global $g_pid_arr;
    global $g_pid_file;
    daemon();
    $fdlock = fopen("./lock","w+");
    if (!flock($fdlock, LOCK_EX | LOCK_NB))
    {
        die("program have run!!!\n");
    }
    
    $fdpid = fopen($g_pid_file,"wb");                 
    if($fdpid)                                        
    {                                                  
        fwrite($fdpid, posix_getpid());           
        fclose($fdpid);                           
    }                                                  
    else                                               
    {                                                  
        die("file ".$g_pid_file." open failed.\n");
    }                                                   
   
    log::write("start run"); 

    pcntl_signal(SIGINT, "parent_signal_handler", false); 
    pcntl_signal(SIGTERM, "parent_signal_handler", false); 
    pcntl_signal(SIGQUIT, "parent_signal_handler", false); 
    pcntl_signal(SIGCHLD, "parent_signal_handler", false); 
    
    for($i = 0; $i < PROCESS_NUM; $i++)
    {
        $pid = pcntl_fork();
        if ($pid == -1)
        {
            die("fork error\n");
        }
        else if ($pid)
        {
            $g_pid_arr[$i] = $pid;
        }
        else
        {
            pcntl_signal(SIGINT, "child_signal_handler", false);
            pcntl_signal(SIGTERM, "child_signal_handler", false);
            pcntl_signal(SIGQUIT, "child_signal_handler", false);
            pcntl_signal(SIGPIPE, "child_signal_handler", false);
            pcntl_signal(SIGUSR1, "child_signal_handler", false);
            exit(children_process());
        }
    }

    while(count($g_pid_arr) > 0)
    {
        $myId = pcntl_waitpid(-1, $status, 0);
        foreach($g_pid_arr as $key => $pid) 
        {
            if($myId == $pid)
            {
                unset($g_pid_arr[$key]);
                $children_ret = pcntl_wexitstatus($status);
                log::write("return code " . $children_ret);
                if ($children_ret == -1)
                {
                    sleep(2);
                    log::write("reboot children");
                    reboot_children_process($key);
                }
                else if ($children_ret == 255)
                {
                    sleep(2);
                    log::write("reboot children");
                    reboot_children_process($key);
                }
            }
        }
        //usleep(100);
    }
    flock($fdlock, LOCK_UN);
    fclose($fdlock);

    log::write("program end");
    exit(0);
}

function reboot_children_process($key)
{
        global $g_pid_arr;
        $pid = pcntl_fork();
        if ($pid == -1)
        {
            log::write("fork error\n","error");
        }
        else if ($pid)
        {
            $g_pid_arr[$key] = $pid;
        }
        else
        {
            pcntl_signal(SIGINT, "child_signal_handler", false);
            pcntl_signal(SIGTERM, "child_signal_handler", false);
            pcntl_signal(SIGQUIT, "child_signal_handler", false);
            pcntl_signal(SIGPIPE, "child_signal_handler", false);
            pcntl_signal(SIGUSR1, "child_signal_handler", false);
            exit(children_process());
        }
}

function outbox_handle($outbox_server_socket, $outbox_reconnect_flag, $output_arr)
{
    foreach($output_arr as $val)
    {
        $output = $val; 
        if ($outbox_reconnect_flag)
        {
            if (init_connect_and_nonblock(OUTBOX_IP, OUTBOX_PORT, $outbox_server_socket))
            {
                log::write("init_feed_connect outbox_server fail", "warn");
                $outbox_reconnect_flag = 1;
            } 
            else
            {
                $outbox_reconnect_flag = 0;
            }
        }
        else
        {
            if ($output['feed_op'] == "update")
            {
                //feedid 一样 对于outbox来说就没有更新
                if ($output['type'] == 'active')
                {
                    if ($output['update_new_feedid']['user_id'] == $output['update_old_feedid']['user_id'] && $output['update_new_feedid']['cmd_id'] == $output['update_old_feedid']['cmd_id'] && $output['update_new_feedid']['app_id'] == $output['update_old_feedid']['app_id'] && $output['update_new_feedid']['timestamp'] == $output['update_old_feedid']['timestamp'] && $output['update_new_feedid']['magic1'] == $output['update_old_feedid']['magic1'] && $output['update_new_feedid']['magic2'] == $output['update_old_feedid']['magic2'])
                    {
                        return 0;
                    }

                    $pack = pack("LSLSL4LSL4", 50, 0xfff1, $output['update_new_feedid']['user_id'], $output['update_new_feedid']['cmd_id'],  $output['update_new_feedid']['app_id'], $output['update_new_feedid']['timestamp'], $output['update_new_feedid']['magic1'], $output['update_new_feedid']['magic2'], $output['update_old_feedid']['user_id'], $output['update_old_feedid']['cmd_id'],  $output['update_old_feedid']['app_id'], $output['update_old_feedid']['timestamp'], $output['update_old_feedid']['magic1'], $output['update_old_feedid']['magic2']);
                }
                else if ($output['type'] == 'passive')
                {
                    $pack = pack("LSLSL7LLSL7L", 82, 0xfff5, $output['update_new_feedid']['user_id'], $output['update_new_feedid']['cmd_id'],  $output['update_new_feedid']['app_id'], $output['update_new_feedid']['timestamp'], $output['update_new_feedid']['magic1'], $output['update_new_feedid']['magic2'], $output['update_new_feedid']['sender_uid'], $output['update_new_feedid']['target_uid'], $output['update_new_feedid']['passive_magic'], $output['update_new_feedid']['update_timestamp'],  $output['update_old_feedid']['user_id'], $output['update_old_feedid']['cmd_id'],  $output['update_old_feedid']['app_id'], $output['update_old_feedid']['timestamp'], $output['update_old_feedid']['magic1'], $output['update_old_feedid']['magic2'], $output['update_old_feedid']['sender_uid'], $output['update_old_feedid']['target_uid'], $output['update_old_feedid']['passive_magic'], $output['update_old_feedid']['update_timestamp']);
                }
                else
                {
                    log::write(__FUNCTION__ . 'unvalid ouput type ' . $output['type'], 'warn');
                    return 0;
                }

                if (send_data_and_nonblock($outbox_server_socket, $pack, TIMEOUT))
                {
                    log::write("send data to outbox_server fail", "warn");
                    $outbox_reconnect_flag = 1;
                }
                else
                {
                    $pack = "";
                    if (recv_data_and_nonblock($outbox_server_socket, 6, $pack, TIMEOUT))
                    {
                        log::write("recv data from outbox_server fail", "warn");
                        $outbox_reconnect_flag = 1;
                    }
                    else
                    {
                        $temp = unpack("Llen/Sresult", $pack);
                        if ($temp['result'] != 0)
                        {
                            log::write("recv data from outbox_server fail, result is {$temp['result']}", "warn");
                        }
                    }
                }
            }
            else if ($output['feed_op'] == "insert")
            {
                if ($output['type'] == 'active')
                {
                    $pack = pack("LSLSL4", 28, 0xfff2, $output['insert_feedid']['user_id'], $output['insert_feedid']['cmd_id'], $output['insert_feedid']['app_id'], $output['insert_feedid']['timestamp'], $output['insert_feedid']['magic1'], $output['insert_feedid']['magic2']);
                }
                else if ($output['type'] == 'passive')
                {
                    $pack = pack("LSLSL7L", 44, 0xFFF6, $output['insert_feedid']['user_id'], $output['insert_feedid']['cmd_id'], $output['insert_feedid']['app_id'], $output['insert_feedid']['timestamp'], $output['insert_feedid']['magic1'], $output['insert_feedid']['magic2'], $output['insert_feedid']['sender_uid'], $output['insert_feedid']['target_uid'], $output['insert_feedid']['passive_magic'], $output['insert_feedid']['update_timestamp']);
                }
                else
                {
                    log::write(__FUNCTION__ . 'unvalid ouput type ' . $output['type'], 'warn');
                    return 0;
                }

                if (send_data_and_nonblock($outbox_server_socket, $pack, TIMEOUT))
                {
                    log::write("send data to outbox_server fail", "warn");
                    $outbox_reconnect_flag = 1;
                }
                else
                {
                    $pack = "";
                    if (recv_data_and_nonblock($outbox_server_socket, 6, $pack, TIMEOUT))
                    {
                        log::write("recv data from outbox_server fail", "warn");
                        $outbox_reconnect_flag = 1;
                    }
                    else
                    {
                        $temp = unpack("Llen/Sresult", $pack);
                        if ($temp['result'] != 0)
                        {
                            log::write("recv data from outbox_server fail, result is {$temp['result']}", "warn");
                        }
                    }
                }
            }
            else if ($output['feed_op'] == 'delete')
            {
                if ($output['type'] == 'active')
                {
                    $pack = pack("LSLSL4", 28, 0xfff3, $output['delete_feedid']['user_id'], $output['delete_feedid']['cmd_id'], $output['delete_feedid']['app_id'], $output['delete_feedid']['timestamp'], $output['delete_feedid']['magic1'], $output['delete_feedid']['magic2']);
                    if (send_data_and_nonblock($outbox_server_socket, $pack, TIMEOUT))
                    {
                        log::write("send data to outbox_server fail", "warn");
                        $outbox_reconnect_flag = 1;
                    }
                    else
                    {
                        $pack = "";
                        if (recv_data_and_nonblock($outbox_server_socket, 6, $pack, TIMEOUT))
                        {
                            log::write("recv data from outbox_server fail", "warn");
                            $outbox_reconnect_flag = 1;
                        }
                        else
                        {
                            $temp = unpack("Llen/Sresult", $pack);
                            if ($temp['result'] != 0)
                            {
                                log::write("recv data from outbox_server fail, result is {$temp['result']}", "warn");
                            }
                        }
                    }
                }
                else if ($output['type'] == 'passive')
                {
                    $pack = pack("LSLSL4L3L", 44, 0xfff7, $output['delete_feedid']['user_id'], $output['delete_feedid']['cmd_id'], $output['delete_feedid']['app_id'], $output['delete_feedid']['timestamp'], $output['delete_feedid']['magic1'], $output['delete_feedid']['magic2'], $output['delete_feedid']['sender_uid'], $output['delete_feedid']['target_uid'], $output['delete_feedid']['passive_magic'], 0);
                    if (send_data_and_nonblock($outbox_server_socket, $pack, TIMEOUT))
                    {
                        log::write("send data to outbox_server fail", "warn");
                        $outbox_reconnect_flag = 1;
                    }
                    else
                    {
                        $pack = "";
                        if (recv_data_and_nonblock($outbox_server_socket, 6, $pack, TIMEOUT))
                        {
                            log::write("recv data from outbox_server fail", "warn");
                            $outbox_reconnect_flag = 1;
                        }
                        else
                        {
                            $temp = unpack("Llen/Sresult", $pack);
                            if ($temp['result'] != 0)
                            {
                                log::write("recv data from outbox_server fail, result is {$temp['result']}", "warn");
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

function children_process()
{
    global $g_stop;
    global $g_sys_conf;

    if (init_net(CACHE_IP, CACHE_PORT, $socket, $taskpack))
    {
        log::write("init_net cache_server fail","error");
        return -1;
    }        
  
/* 
    if (init_connect_and_nonblock(FEED_IP, FEED_PORT, $feed_socket))
    {
        log::write("init_feed_connect feed_server fail", "warn");
        $feed_reconnect_flag = 1;
    } 
    else
    {
        $feed_reconnect_flag = 0;
    }
*/
    
    if (init_connect_and_nonblock(STORAGE_IP, STORAGE_PORT, $storage_server_socket))
    {
        log::write("init_connect storage_server fail reason: connect to storage_server", "error");
        return -1;
    } 

    if (init_connect_and_nonblock(OUTBOX_IP, OUTBOX_PORT, $outbox_server_socket))
    {
        log::write("init_feed_connect outbox_server fail", "warn");
        $outbox_reconnect_flag = 1;
    } 
    else
    {
        $outbox_reconnect_flag = 0;
    }
    if (init_connect_and_nonblock(RELATION_IP, RELATION_PORT, $redis_server_socket))
    {
        log::write("init_connect redis_server fail reason: connect to redis_server", "error");
        return -1;
    }

    $task = unpack("Slen/Stask_id",$taskpack);
    while(!$g_stop)
    {
        //*********请求包**************
        $pack = pack("S4", 8, 0xffff, 2, $task["task_id"]);
        //****************************
        if (send_data($socket, $pack))
        {
            log::write("[request handler package]send_data fail", "error");
            return -1;
        }
        $pack = "";
        if (recv_data($socket, 2, $pack))
        {
            log::write("[response handler package]recv_data 2 bytes fail", "error");
            return -1;
        } 

        $temp = unpack("Slen",$pack);

        if ($temp['len'] < 13)
        {
            log::write('recv data length is '. $temp['len'] .' < 13', 'warn');
            continue;
        }       
        
        if (recv_data($socket, $temp["len"], $pack))
        {
            log::write("[response handler package]recv_data full data fail", "error");
            return -1;
        }
        
        $feed = unpack("Slen/Scmd_id/Luser_id/Cversion/Ltimestamp", $pack); 
        $feed["data"] = substr($pack, 13, $feed["len"] - 13);
       
         
        if ($feed["cmd_id"] == 0xffff)  // 表明无feed消息到达
        {
            sleep(2);
            continue;
        }        

        if ($feed['cmd_id'] <= 1000)
        {
            $output_arr = array(); 
            $input_arr = array("storage_server_socket" => $storage_server_socket
            );
            if ($feed['cmd_id'] == 1 || $feed['cmd_id'] == 2) //删除feed
            {
                if ($ret = feeddelete($pack, $input_arr, $output_arr))
                {
                    if (-1 == $ret)
                    {
                        log::write("[feed delete]delete feed fail", "error");
                        return -1;
                    }
                    else if (-2 == $ret)
                    {
                        continue;
                    }
                    else
                    {
                        log::write("[feed delete]feed process error error:unvalid return value{$ret}", "error");
                        return -1;
                    }
                }
            }
            else if ($feed['cmd_id'] == 201 || $feed['cmd_id'] == 202)
            {
                if ($ret = feed_update_sync($pack, $input_arr, $output_arr))
                {
                    if (-1 == $ret)
                    {
                        log::write("[feed_handle]feed process error fatal error", "error");
                        return -1;
                    }
                    else if (-2 == $ret) //表明这个feed没有拉取到数据或者一些警告错误 可以直接跳过
                    {
                        continue;
                    }
                    else 
                    {
                        log::write("[feed_handle]feed process error error:unvalid return value{$ret}", "error");
                        return -1;
                    }
                }
            }
            //同步outbox-server
            //木有配置的feed feedhandle会打印日志 返回0 
            if (is_null($output_arr))
            {
                continue;
            } 

            outbox_handle($outbox_server_socket, $outbox_reconnect_flag, $output_arr);

            continue;
        }
        else if ($feed['cmd_id'] >= 1001 && $feed['cmd_id'] <= 15000) //正常的feed消息
        {
            if (array_key_exists($feed["cmd_id"], $g_sys_conf['feed']['app_id'])) //判断是否是老版本不带app_id的协议包
            {
                $feed = unpack("Slen/Scmd_id/Luser_id/Cversion/Ltimestamp", $pack); 
                $feed['app_id'] = $g_sys_conf['feed']['app_id'][$feed['cmd_id']];
                $feed["data"] = substr($pack, 13, $feed["len"] - 13);
            }
            else
            {
                //$feed = unpack("Slen/Scmd_id/Luser_id/Ltimestamp/Cversion/Lapp_id", $pack); 
                $feed = unpack("Slen/Scmd_id/Luser_id/Cversion/Ltimestamp/Lapp_id", $pack); 
                $feed["data"] = substr($pack, 17, $feed["len"] - 17);
            }

            if (is_feed_valid($feed))
            {
                continue;
            }

            $input_arr = array(
                //"feed_socket" => $feed_socket, 
                //"feed_reconnect_flag" => $feed_reconnect_flag,                
                "storage_server_socket" => $storage_server_socket,
                "redis_server_socket" => $redis_server_socket
            );
            $output_arr = array(); 
            if ($ret = feedhandle($feed, $input_arr, $output_arr))
            {
                if (-1 == $ret)
                {
                    log::write("[feed_handle]feed process error fatal error", "error");
                    return -1;
                }
                else if (-2 == $ret) //表明这个feed没有拉取到数据或者一些警告错误 可以直接跳过
                {
                    continue;
                }
                else 
                {
                    log::write("[feed_handle]feed process error error:unvalid return value{$ret}", "error");
                    return -1;
                }
            }

            //if (isset($output['feed_reconnect_flag']))
            //{
            //    $feed_reconnect_flag = $output['feed_reconnect_flag'];
            //}

            //木有配置的feed feedhandle会打印日志 返回0 
            if (is_null($output_arr))
            {
                continue;
            } 

            outbox_handle($outbox_server_socket, $outbox_reconnect_flag, $output_arr);

            continue;
        }
        else
        {
            log::write(__LINE__."[process.php] unsupport cmd id".print_r($feed, true), "error");
        }

    }

    return 0;
}

ini_set("display_errors", "0");
ini_set("log_errors", "1");
ini_set("error_log", "log/mysys.log");
ini_set('memory_limit', '1024M');
error_reporting(E_ALL & ~E_NOTICE);
date_default_timezone_set("Asia/Shanghai");

process_control();

?>
