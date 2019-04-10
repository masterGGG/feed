#! /usr/bin/php5
<?php
 require_once "../config/setup.inc.php";

                    $feed['user_id'] = 20000;
                    $feed['cmd_id'] = 7004;
                    $feed['timestamp'] = 1304406323;
                    $feed['app_id'] = 134;
                    $feed_content['img_url'] = array('http://url.cn'); 

                    //if (init_connect_and_nonblock(FEED_IP, FEED_PORT, $feed_socket))
                    //{
                    //    log::write("init_feed_connect fail", "warn");
                    //    $feed_reconnect_flag = 1;
                    //}
                    //else
                    //{
                    //    $feed_reconnect_flag = 0;
                    //}
                    if (init_connect(STORAGE_IP, STORAGE_PORT, $storage_server_socket))
                    {
                        log::write("init_connect storage_server fail reason: connect to storage_server", "error");
                        return -1;        
                    }                     

                     $img_url_num = count($feed_content['img_url']);
                     if ($img_url_num >= 4)
                     {
                         $content = $feed_content['img_url'][0] . ';' . $feed_content['img_url'][1] . ';' .
 $feed_content['img_url'][2] . ';' . $feed_content['img_url'][3];
                     }
                     else
                     {
                         if (false == get_feed_according_time($storage_server_socket, $feed['user_id'], $feed['cmd_id'],
 $feed['timestamp'], $feed['app_id'], 4, 0, $result_feed))
                         {   
                             log::write("get_feed_according_time error","warn");
                             data_log_pre($feed,"dispatch_get_data");
                             return -1;
                         }  
                         $get_feed_num = count($result_feed); 
                             
                         $img_url_arr = $feed_content['img_url']; 
                         for ($i = 0; $i < min($get_feed_num, (4 - $img_url_num)); $i++)
                         {   
                             $str = json_decode($result_feed[$i]['data'],true);
                             $arr_str = $str['img_url'];
                             $img_url_arr = array_merge($img_url_arr, $arr_str);
                         }
                         $get_img_num = count($img_url_arr);
                         if ($get_img_num >= 4)
                         {
                             $content = $img_url_arr[0] . ';' . $img_url_arr[1] . ';' . $img_url_arr[2] . ';' .$img_url_arr[3];
                         }   
                         else
                         {
                             $content = implode(';', $img_url_arr);
                         } 
                     }
                     echo $content;
?>
