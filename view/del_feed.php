<?php
/* vim: set expandtab tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file del_feed.php
 * @author richard <richard@taomee.com>
 * @date 2011-06-23
 */

header("Content-Type: application/json; charset=utf8");
require_once('function.php');

if ($_REQUEST['type'] == 'active')
{

    $uid    = '';                                                 
    $app_id = 0;                                             
    $cmd_id = 0;                                             
    $timestamp = 0;
    $magic = '';

    if (isset($_REQUEST['uid'])) {
        $uid = $_REQUEST['uid'];
    }
    if (isset($_REQUEST['app_id'])) {
        $app_id = $_REQUEST['app_id'];
    }
    if (isset($_REQUEST['cmd_id'])) {
        $cmd_id = $_REQUEST['cmd_id'];
    }
    if (isset($_REQUEST['timestamp'])) {
        $timestamp = $_REQUEST['timestamp'];
    }
    if (isset($_REQUEST['magic'])) {
        $magic = $_REQUEST['magic'];
    }

    $rv = del_feed($uid, $app_id, $cmd_id, $timestamp, $magic);
    if ($rv === TRUE) {
        do_log('debug', "del_feed: uid: $uid app_id: $app_id cmd_id: $cmd_id timestamp: $timestamp magic: $magic");
        echo json_encode(array('result' => 0));
    } else {
        do_log('error', "del_feed: uid: $uid app_id: $app_id cmd_id: $cmd_id timestamp: $timestamp magic: $magic");
        echo json_encode(array('result' => -1));
    }
}
else if ($_REQUEST['type'] == 'passive')
{
    if (isset($_REQUEST['uid'])) {
        $uid = $_REQUEST['uid'];
    }
    if (isset($_REQUEST['app_id'])) {
        $app_id = $_REQUEST['app_id'];
    }
    if (isset($_REQUEST['cmd_id'])) {
        $cmd_id = $_REQUEST['cmd_id'];
    }
    if (isset($_REQUEST['timestamp'])) {
        $timestamp = $_REQUEST['timestamp'];
    }
    if (isset($_REQUEST['magic'])) {
        $magic = $_REQUEST['magic'];
    }
    if (isset($_REQUEST['sender_uid'])) {
        $sender_uid = $_REQUEST['sender_uid'];
    }
    if (isset($_REQUEST['target_uid'])) {
        $target_uid = $_REQUEST['target_uid'];
    }
    if (isset($_REQUEST['passive_magic'])) {
        $passive_magic = $_REQUEST['passive_magic'];
    }

    $rv = del_passive_feed($uid, $app_id, $cmd_id, $timestamp, $magic, $sender_uid, $target_uid, $passive_magic);
    if ($rv === TRUE) {
        do_log('debug', "del_feed: uid: $uid app_id: $app_id cmd_id: $cmd_id timestamp: $timestamp magic: $magic sender_uid: $sender_uid target_uid:$target_uid passive_magic:$passive_magic");
        echo json_encode(array('result' => 0));
    } else {
        do_log('error', "del_feed: uid: $uid app_id: $app_id cmd_id: $cmd_id timestamp: $timestamp magic: $magic sender_uid: $sender_uid target_uid:$target_uid passive_magic:$passive_magic");
        echo json_encode(array('result' => -1));
    }
}
