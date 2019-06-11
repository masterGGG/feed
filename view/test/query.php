#!/usr/bin/php
<?php

 error_reporting(E_ALL);
 ini_set('display_errors', '0');
 ini_set('log_errors', '1');
 ini_set('error_log', './log/mysys.log');
 ini_set('memory_limit', '2048M');

set_time_limit(0);
//sleep(5);
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
//'type' => 'tag',
'type' => 'active',
//'type' => 'passive',
//'type' => 'latest',
'action' => 'normal',
'offset' => -1,
'start_time' => 1,
'end_time' => 1955656634,
'count' => 50,
'tags_id' => 1,
//'uid'=>'382854116');
'uid'=>'188888888');
//'uid'=>'1227401112');

var_dump(Post('http://10.1.1.197:10086/news_feed.php', $data));
//var_dump(Post('http://10.1.1.197:8080/news_feed.php', $data));





