#!/bin/sh

server_path='./bin/outbox_serv'
server_conf='./conf/bench.conf'

logid=`cat /etc/passwd | grep $LOGNAME | awk -F ':' '{print $3}'`

cwd=`pwd`
add_crontab()
{
    tmpfile=crontab-ori.tempXX
    item1='*/1 * * * * cd '${cwd}' && ./keep-alive.sh server >> keep-alive.log 2>&1'

    crontab -l >$tmpfile 2>/dev/null

    fgrep "${item1}" $tmpfile &>/dev/null
    if [ $? -ne 0 ]
    then
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

if [ "$1" = "state" ] ; then
    ps -eo user,pid,stat,pcpu,pmem,cmd | grep "$server_path" | grep -v grep
elif [ "$1" = "start" ] ; then
     export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./bin/:../newbench/:/usr/local/lib/ && $server_path $server_conf
     add_crontab
elif [ "$1" = "stop" ] ; then
    ps -ef | grep -E "^$log_name|^$log_id" | grep "$server_path" | grep -v grep | awk '{print "echo kill " $2 " " $8 "; kill " $2;}' | sh
    if [ $? -eq 0 ] ; then
        printf "$red_clr%50s$end_clr\n" "$server_path is not running"
        delete_crontab
    fi
else
    echo 'usage: server.sh state|start|stop'
fi
