#!/usr/bin/php
<?php

 error_reporting(E_ALL);
 ini_set('display_errors', '0');
 ini_set('log_errors', '1');
 ini_set('error_log', './log/mysys.log');
 ini_set('memory_limit', '2048M');

set_time_limit(0);

function Post($url, $post = null)  
{  
    $context = array();  

    if (is_array($post))  
    {  
        ksort($post);  

        $context['http'] = array  
            (  
             'method' => 'POST',  
             'content' => http_build_query($post, '', '&'),  
            );  
    }  

    return file_get_contents($url, false, stream_context_create($context));  
}  

$data = array  
(  
'action' => 'class',
'app_id' => 7003,
'cmd_id' => 7003,
'timestamp' => 1552027000,
'offset' => 0,
'count' => 20,
'uid'=>'1227401110');

var_dump(Post('http://10.1.1.197:10086/news_feed.php', $data));
//var_dump(Post('http://10.1.1.197:8080/news_feed.php', $data));





