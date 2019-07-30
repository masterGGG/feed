<?php
/* vim: set expandtab tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file news_feed.php
 * @author richard <richard@taomee.com>
 * @date 2011-06-14
 */

header("Content-Type: application/json; charset=utf8");
require_once('function.php');

$action = '';
if (isset($_REQUEST['action']) && $_REQUEST['action'] != '') {
    $action = $_REQUEST['action'];
}
if ($action == 'normal')
{
    $type = '';
    if (isset($_REQUEST['type']) && $_REQUEST['type'] != '') {
        $type = $_REQUEST['type'];
    }
    
    if ($type == 'active')
    {
        $begin_time = gettimeofday(true) * 1000;
        $analysis_time['type'] = 'time'; 
        $analysis_time['timestamp'] = time(); 

        $uid    = '';                                                 
        $app_id = '';                                             
        $cmd_id = '';                                             
        $offset = 0;
        $count  = 0;
        $timestamp = 0;

        if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
            $uid = $_REQUEST['uid'];
        }
        if (isset($_REQUEST['app_id']) && $_REQUEST['app_id'] != '') {
            $app_id = $_REQUEST['app_id'];
        }
        if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') {
            $cmd_id = $_REQUEST['cmd_id'];
        }
        if (isset($_REQUEST['offset']) && $_REQUEST['offset'] != '') {
            $offset = $_REQUEST['offset'];
        }
        if (isset($_REQUEST['count']) && $_REQUEST['count'] != '') {
            $count = $_REQUEST['count'];
        }
        if (isset($_REQUEST['timestamp']) && $_REQUEST['timestamp'] != '') {
            $timestamp = $_REQUEST['timestamp'];
        }

        $rv = FALSE;

        if ($uid === '') {
            echo json_encode(array('result' => -1));
            do_log('error', "ERROR: uid: $uid app_id: $app_id cmd_id: $cmd_id offset: $offset count: $count");
            exit();
        }
        $arr_uid = explode(',', $uid);
        if ($app_id === '')
        {
            $arr_app_id = array();
        }
        else
        {
            $arr_app_id = explode(',', $app_id);
        }
        if ($cmd_id === '')
        {
            $arr_cmd_id = array();
        }
        else
        {
            $arr_cmd_id = explode(',', $cmd_id);
        }

        $rv = get_newsfeed($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp);

        if ($rv === FALSE) {
            echo json_encode(array('result' => -1));
            do_log('error', "ERROR: uid: $uid app_id: $app_id cmd_id: $cmd_id offset: $offset count: $count");
            exit();
        }

        echo $rv;

        $end_time = gettimeofday(true) * 1000;
        $used_time = $end_time - $begin_time;
        $analysis_time["total"] = array("key" => $arr_uid[0]."_".$app_id."_".$cmd_id."_".$offset."_".$count,
            "time" => $used_time);
        $analysis_time = json_encode($analysis_time);

        if (ANALYSIS)
        {
            analysis_log("time",$analysis_time);
        }
    }
    else if ($type == 'passive')
    {
        $uid  = '';                                                 
        $offset = 0;
        $count  = 0;
        $timestamp = 0;
        $cmd_id = ''; 

        if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
            $uid = $_REQUEST['uid'];
        }
        else
        {
            echo json_encode(array('result' => -1));
            exit();
        }

        if (isset($_REQUEST['offset']) && $_REQUEST['offset'] != '') {
            $offset = $_REQUEST['offset'];
        }
        if (isset($_REQUEST['count']) && $_REQUEST['count'] != '') {
            $count = $_REQUEST['count'];
        }
        if (isset($_REQUEST['timestamp']) && $_REQUEST['timestamp'] != '') {
            $timestamp = $_REQUEST['timestamp'];
        }
        if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') {
            $cmd_id = $_REQUEST['cmd_id'];
        }

        $rv = get_passive_newsfeed($uid, $offset, $count, $timestamp, $cmd_id);
        if ($rv === FALSE) {
            echo json_encode(array('result' => -1));
        }
        echo $rv;
    } 
    else if ($type == 'recommend')
    {
        $uid  = '';                                   
        $app_id = '';
        $cmd_id = '';                                 
        $offset = 0;
        $count  = 0;
        $timestamp = 0;

        if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
            $uid = $_REQUEST['uid'];
        }
        else
        {
            echo json_encode(array('result' => -1));
            exit();
        }

        if (isset($_REQUEST['app_id']) && $_REQUEST['app_id'] != '') {
            $app_id = $_REQUEST['app_id'];
        }
        if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') {
            $cmd_id = $_REQUEST['cmd_id'];
        }
        if (isset($_REQUEST['offset']) && $_REQUEST['offset'] != '') {
            $offset = $_REQUEST['offset'];
        }
        if (isset($_REQUEST['count']) && $_REQUEST['count'] != '') {
            $count = $_REQUEST['count'];
        }
        if (isset($_REQUEST['timestamp']) && $_REQUEST['timestamp'] != '') {
            $timestamp = $_REQUEST['timestamp'];
        }
        /*
        if (isset($_REQUEST['tags_id']) && $_REQUEST['tags_id'] != '') {
            $tags_id = $_REQUEST['tags_id'];
        }
        */

        $arr_uid = explode(',', $uid);
        if ($app_id === '')
        {
            $arr_app_id = array();
        }
        else
        {
            $arr_app_id = explode(',', $app_id);
        }
        if ($cmd_id === '')
        {
            $arr_cmd_id = array();
        }
        else
        {
            $arr_cmd_id = explode(',', $cmd_id);
        }
        $rv = get_newsfeed_of_recommend($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp/*, $tags_id*/);
        if ($rv === FALSE) {
            echo json_encode(array('result' => -1));
        }
        echo $rv;
    }
    else if ($type == 'latest')
    {
        $uid  = '';                                   
        $app_id = '';
        $cmd_id = '';                                 
        $offset = 0;
        $count  = 0;
        $timestamp = 0;

        if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
            $uid = $_REQUEST['uid'];
        }
        else
        {
            echo json_encode(array('result' => -1));
            exit();
        }

        if (isset($_REQUEST['app_id']) && $_REQUEST['app_id'] != '') {
            $app_id = $_REQUEST['app_id'];
        }
        if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') {
            $cmd_id = $_REQUEST['cmd_id'];
        }
        if (isset($_REQUEST['offset']) && $_REQUEST['offset'] != '') {
            $offset = $_REQUEST['offset'];
        }
        if (isset($_REQUEST['count']) && $_REQUEST['count'] != '') {
            $count = $_REQUEST['count'];
        }
        if (isset($_REQUEST['timestamp']) && $_REQUEST['timestamp'] != '') {
            $timestamp = $_REQUEST['timestamp'];
        }
        /*
        if (isset($_REQUEST['tags_id']) && $_REQUEST['tags_id'] != '') {
            $tags_id = $_REQUEST['tags_id'];
        }
        */

        $arr_uid = explode(',', $uid);
        if ($app_id === '')
        {
            $arr_app_id = array();
        }
        else
        {
            $arr_app_id = explode(',', $app_id);
        }
        if ($cmd_id === '')
        {
            $arr_cmd_id = array();
        }
        else
        {
            $arr_cmd_id = explode(',', $cmd_id);
        }
        $rv = get_newsfeed_of_latest($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp/*, $tags_id*/);
        if ($rv === FALSE) {
            echo json_encode(array('result' => -1));
        }
        echo $rv;
    }
    else if ($type == 'tag')
    {
        $uid  = '';                                   
        $app_id = '';
        $cmd_id = '';                                 
        $count  = 0;
        $begin = 0;
        $end = 0;
        $tags_id = 0;

        if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
            $uid = $_REQUEST['uid'];
        }
        else
        {
            echo json_encode(array('result' => -1));
            exit();
        }

        if (isset($_REQUEST['app_id']) && $_REQUEST['app_id'] != '') {
            $app_id = $_REQUEST['app_id'];
        }
        if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') {
            $cmd_id = $_REQUEST['cmd_id'];
        }
        if (isset($_REQUEST['count']) && $_REQUEST['count'] != '') {
            $count = $_REQUEST['count'];
        }
        if (isset($_REQUEST['start_time']) && $_REQUEST['start_time'] != '') {
            $begin = $_REQUEST['start_time'];
        }
        if (isset($_REQUEST['end_time']) && $_REQUEST['end_time'] != '') {
            $end = $_REQUEST['end_time'];
        }
        if (isset($_REQUEST['tags_id']) && $_REQUEST['tags_id'] != '') {
            $tags_id = $_REQUEST['tags_id'];
        }
        $arr_uid = explode(',', $uid);
        if ($app_id === '')
        {
            $arr_app_id = array();
        }
        else
        {
            $arr_app_id = explode(',', $app_id);
        }
        if ($cmd_id === '')
        {
            $arr_cmd_id = array();
        }
        else
        {
            $arr_cmd_id = explode(',', $cmd_id);
        }
        $rv = get_newsfeed_by_tags($arr_uid[0], $arr_app_id, $arr_cmd_id, $count, $begin, $end, $tags_id);
        if ($rv === FALSE) {
            echo json_encode(array('result' => -1));
        }
        echo $rv;
    }
    else if ($type == 'home') {
        $uid    = '';                                                 
        $app_id = '';                                             
        $cmd_id = '';                                             
        $offset = 0;
        $count  = 0;
        $timestamp = 0;

        if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
            $uid = $_REQUEST['uid'];
        }
        if (isset($_REQUEST['app_id']) && $_REQUEST['app_id'] != '') {
            $app_id = $_REQUEST['app_id'];
        }
        if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') {
            $cmd_id = $_REQUEST['cmd_id'];
        }
        if (isset($_REQUEST['offset']) && $_REQUEST['offset'] != '') {
            $offset = $_REQUEST['offset'];
        }
        if (isset($_REQUEST['count']) && $_REQUEST['count'] != '') {
            $count = $_REQUEST['count'];
        }
        if (isset($_REQUEST['timestamp']) && $_REQUEST['timestamp'] != '') {
            $timestamp = $_REQUEST['timestamp'];
        }

        $rv = FALSE;

        if ($uid === '') {
            echo json_encode(array('result' => -1));
            do_log('error', "ERROR: uid: $uid app_id: $app_id cmd_id: $cmd_id offset: $offset count: $count");
            exit();
        }
        $arr_uid = explode(',', $uid);
        if ($app_id === '')
        {
            $arr_app_id = array();
        }
        else
        {
            $arr_app_id = explode(',', $app_id);
        }
        if ($cmd_id === '')
        {
            $arr_cmd_id = array();
        }
        else
        {
            $arr_cmd_id = explode(',', $cmd_id);
        }

        $rv = get_homepage_newsfeed($uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp);

        if ($rv === FALSE) {
            echo json_encode(array('result' => -1));
            do_log('error', "ERROR: uid: $uid app_id: $app_id cmd_id: $cmd_id offset: $offset count: $count");
            exit();
        }

        echo $rv;
    }
    else
    {
        echo json_encode(array('result' => -1));
        exit();
    }
}
else if ($action == 'class')
{
        $uid  = '';                                                 
        $offset = 0;
        $count  = 0;
        $app_id = '';
        $cmd_id = '';
        $timestamp = 0;

        if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
            $uid = $_REQUEST['uid'];
        }
        else
        {
            echo json_encode(array('result' => -1));
            exit();
        }

        if (isset($_REQUEST['app_id']) && $_REQUEST['app_id'] != '') {
            $app_id = $_REQUEST['app_id'];
        }
        if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') {
            $cmd_id = $_REQUEST['cmd_id'];
        }
        if (isset($_REQUEST['offset']) && $_REQUEST['offset'] != '') {
            $offset = $_REQUEST['offset'];
        }
        if (isset($_REQUEST['count']) && $_REQUEST['count'] != '') {
            $count = $_REQUEST['count'];
        }
        if (isset($_REQUEST['timestamp']) && $_REQUEST['timestamp'] != '') {
            $timestamp = $_REQUEST['timestamp'];
        }

        $rv = FALSE;

        if ($uid === '') {
            echo json_encode(array('result' => -1));
            do_log('error', "ERROR: uid: $uid app_id: $app_id cmd_id: $cmd_id offset: $offset count: $count");
            exit();
        }

        $arr_uid = explode(',', $uid);
        if ($app_id === '')
        {
            $arr_app_id = array();
        }
        else
        {
            $arr_app_id = explode(',', $app_id);
        }
        if ($cmd_id === '')
        {
            $arr_cmd_id = array();
        }
        else
        {
            $arr_cmd_id = explode(',', $cmd_id);
        }

        $rv = get_class_newsfeed($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp);

        if ($rv === FALSE) {
            echo json_encode(array('result' => -1));
        }
        echo $rv;
}
else if ($action == 'notice')
{
    $uid = '';
    $type = '';
    $cmd_id = '';
    if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
        $uid = $_REQUEST['uid'];
    }
    if (isset($_REQUEST['type']) && $_REQUEST['type'] != '') {
        $type = $_REQUEST['type'];
    }
            
    if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') 
        $cmd_id = $_REQUEST['cmd_id'];

    if ($cmd_id === '') {
        $arr_cmd_id = array();
    } else {
        $arr_cmd_id = explode(',', $cmd_id);
    }
    
    if ($uid == '' || $type == '')
    {
        echo json_encode(array('result' => -1));
        do_log('error', "ERROR: uid: $uid cmd_id: $cmd_id type: $type");
        exit();
    }
    $arr_uid = explode(',', $uid);
    $rv = get_notice($arr_uid, $type, $arr_cmd_id);

    if ($rv === FALSE) {
        echo json_encode(array('result' => -1));
    }
    echo $rv; 
} else if ($action == 'pull') {
do_log('error', '['.__LINE__.']: >>>>>>>>>>>>>> BEGIN <<<<<<<<<<<<:');
    $uid = '';
    $begintime = 0;
    $endtime = gettimeofday(true);
    $cnt = 0;
    if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '')
        $uid = $_REQUEST['uid'];
    else {
        echo json_encode(array('result' => -1));
    }

    if (isset($_REQUEST['start_time']) && $_REQUEST['start_time'] != '')
        $begintime = $_REQUEST['start_time'];
    if (isset($_REQUEST['end_time']) && $_REQUEST['end_time'] != '')
        $endtime = $_REQUEST['end_time'];
    if (isset($_REQUEST['count']) && $_REQUEST['count'] != '')
        $cnt = $_REQUEST['count'];
    $rv = get_pull_feedlist($uid,  $begintime, $endtime, $cnt);
    if ($rv === FALSE) {
        echo json_encode(array('result' => -1));
    }
    do_log('error', '['.__LINE__.']: >>>>>>>>>>>>>> END <<<<<<<<<<<<:');
    echo $rv; 
} else if ($action == 'stat') {
    $cmd_id = '';
    $uid = '';
    $type = '';

    if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
        $uid = $_REQUEST['uid'];
    }
    if (isset($_REQUEST['type']) && $_REQUEST['type'] != '') {
        $type = $_REQUEST['type'];
    }
            
    if (isset($_REQUEST['cmd_id']) && $_REQUEST['cmd_id'] != '') 
        $cmd_id = $_REQUEST['cmd_id'];
    
    if ($uid == '' || $type == '' || $cmd_id == '')
    {
        echo json_encode(array('result' => -1));
        do_log('error', "ERROR: uid: $uid cmd_id: $cmd_id type: $type");
        exit();
    }
    
    $rv = get_pfeed_stat($uid, $type, $cmd_id);

    if ($rv === FALSE) {
        echo json_encode(array('result' => -1));
    }
    echo $rv; 
} else if ($action == 'recommend') {
    $uid = '';
    $type = '';
    $offset = 0;
    $count  = 0;
    if (isset($_REQUEST['uid']) && $_REQUEST['uid'] != '') {
        $uid = $_REQUEST['uid'];
    }
    if (isset($_REQUEST['type']) && $_REQUEST['type'] != '') {
        $type = $_REQUEST['type'];
    }
    if (isset($_REQUEST['offset']) && $_REQUEST['offset'] != '') {
        $offset = $_REQUEST['offset'];
    }
    if (isset($_REQUEST['count']) && $_REQUEST['count'] != '') {
        $count = $_REQUEST['count'];
    }
            
    $rv = get_recommend_user($uid, $type, $offset, $count);

    if ($rv === FALSE) {
        echo json_encode(array('result' => -1));
    }
    echo $rv; 
} else {
    echo json_encode(array('result' => -1));
    exit();
}
