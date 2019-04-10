#!/bin/sh

server_name='storage_server'
server_path='./storage_serv'
server_conf='./conf/bench.conf'
server_flag='storage_serv'
log_dir='./log'

if [[ "$1" = "stat_all" ]] ; then
    ps -u$(whoami) -o user,pid,ppid,sid,stat,pcpu,pmem,start,wchan:15,cmd | grep "$server_flag\|USER" | grep -v 'grep'
elif [[ "$1" =~ "stat" ]] ; then
    ps -u$(whoami) -o user,pid,ppid,sid,stat,pcpu,pmem,start,wchan:15,cmd | grep "$server_flag\|USER" | grep -v 'grep'
elif [ "$1" = "start" ] ; then
    echo "start $server_name ...";
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./:./libs/
    $server_path $server_conf
elif [ "$1" = "stop" ] ; then
    echo "stop $server_name ...";
    killall -9 storage_serv
#    p_list=$(lsof +d $log_dir | grep -v -E "lsof|bash|grep|PID" | awk -F "[ \t]+" '{print $2;}' | uniq)
#    for it in $p_list; do
#        echo "kill $it";
#        kill $it;
#    done;
elif [ "$1" = "reload" ] ; then
    echo "reload configuration for $server_name ...";
    p_list=$(lsof +d $log_dir | grep -v -E "lsof|bash|grep|PID" | awk -F "[ \t]+" '{print $2;}' | uniq)
    for it in $p_list; do
        echo "kill -HUP $it";
        kill -HUP $it;
    done;
else
    echo 'usage: storagectl start|stop|status|reload'
fi
