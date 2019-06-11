#!/bin/bash

red_clr="\033[31m"
grn_clr="\033[32m"
end_clr="\033[0m"
cwd=`pwd`
appname='cache_server'
pidfile='./bin/daemon.pid'
    item1='*/1 * * * * cd '${cwd}' && ./keep-alive.sh server >> keep-alive.log 2>&1'
add_crontab()
{
    tmpfile=crontab-ori.tempXX

    crontab -l >$tmpfile 2>/dev/null

    fgrep "${item1}" $tmpfile &>/dev/null
    if [ $? -ne 0 ]
    then
        echo "${item1}" >> $tmpfile
    fi

    crontab $tmpfile
	rm -f $tmpfile
}

start()
{
    if [ -e $pidfile ] 
    then    
	    ps -f `cat $pidfile` | grep CACHE_SERVER > /dev/null 2>&1
    	if [ $? -eq 0 ] 
        then
		    printf "$red_clr%50s$end_clr\n" "STAT_FRAME is running"
		    exit -1
	    fi
    fi

    export LD_LIBRARY_PATH=../../newsfeed-lib/:$LD_LIBRARY_PATH && 
    cd ./bin && ./cache_server
    add_crontab
}

state()
{
    pid=`cat $pidfile`
    ps -f $pid | grep "CACHE_SERVER"
    if [ $? -ne 0 ]
    then
        printf "$red_clr%50s$end_clr\n" " cache-serv is not running"
        exit 1
    fi
}

stop() {
    ps -f `cat $pidfile` | grep CACHE_SERVER > /dev/null 2>&1
    if [ $? -ne 0 ]
    then
	    printf "$red_clr%50s$end_clr\n" "CACHE_SERVER is not running"
	    exit -1
    fi

    while [ $? -eq 0 ]
    do
	    kill `cat $pidfile`
    	sleep 1
        ps -f `cat $pidfile` | grep $appname > /dev/null 2>&1
    done

    printf "$grn_clr%50s$end_clr\n" "CACHE_SERVER has been stopped"

    tmpfile1=crontab-ori.temp1XX
    tmpfile2=crontab-ori.temp2XX

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

usage()
{
	echo "$0 <start|stop|restart|state|>"
}

if [ $# -ne 1 ]; then
    usage
    echo "count : $#"
    exit 1
fi

case $1 in
    start)
        start
		add_crontab
        ;;
    stop)
        stop
        ;;
    state)
        state 
        ;;
    *)
        usage 
        ;;
    esac

exit 0
