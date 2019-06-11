#!/bin/sh

server_name='storage_server'
server_path='./storage_serv'
server_conf='./conf/bench.conf'
server_flag='storage_serv'
log_dir='./log'

cwd=`pwd`
add_crontab()
{
    tmpfile=crontab-ori.tempXX
    item1='*/1 * * * * cd '${cwd}' && ./keep-alive.sh server >> keep-alive.log 2>&1'

    crontab -l >$tmpfile 2>/dev/null

    if [ -s $tmpfile ] ; then
        fgrep "${item1}" $tmpfile &>/dev/null

        if [ $? -ne 0 ]
        then
            echo "${item1}" >> $tmpfile
        fi
    else
        echo "${item1}" >> $tmpfile
    fi
    crontab $tmpfile
    rm -f $tmpfile
}

delete_crontab()
{
    tmpfile1=crontab-ori.temp1XX
        tmpfile2=crontab-ori.temp2XX
        item1='*/1 * * * * cd '${cwd}' && ./keep-alive.sh server >> keep-alive.log 2>&1'

        crontab -l >$tmpfile1 2>/dev/null
        fgrep "${item1}" $tmpfile1 &>/dev/null
        if [ $? -eq 0 ]
            then
                fgrep -v "${item1}" $tmpfile1 &> $tmpfile2
                crontab $tmpfile2
                fi

                rm -f $tmpfile1
                rm -f $tmpfile2
}
if [ "$1" = "stat_all" ] ; then
ps -u$(whoami) -o user,pid,ppid,sid,stat,pcpu,pmem,start,wchan:15,cmd | grep "$server_flag\|USER" | grep -v 'grep'
elif [ $1 = "state" ] ; then
ps -u$(whoami) -o user,pid,ppid,sid,stat,pcpu,pmem,start,wchan:15,cmd | grep "$server_flag" | grep -v 'grep'
if [ $? -ne 0 ]; then
printf "$red_clr%50s$end_clr\n" "$server_name is not running"
exit 1
fi
elif [ "$1" = "start" ] ; then
echo "start $server_name ...";
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./:./libs/:../../newsfeed-lib/
$server_path $server_conf 
add_crontab
elif [ "$1" = "stop" ] ; then
echo "stop $server_name ...";
delete_crontab
killall -9 storage_serv
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

