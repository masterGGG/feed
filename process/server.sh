#########################################################################
# File Name: server.sh
# Author: ian
# mail: ian@taomee.com
# Created Time: Tue 02 Apr 2019 18:16:34 PM CST
#########################################################################
#!/bash

pidfile=outbox_pid

cwd=`pwd`
appname='process'    
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
delete_crontab()
{
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

start()
{
    if [ -e $pidfile ] 
    then
        ps -fs `cat $pidfile` | grep process > /dev/null 2>&1
    	if [ $? -eq 0 ]
	    then
		    printf "$red_clr%50s$end_clr\n" "${appname} is already running"
    		exit 1
	    fi
    fi
        
    php process.php

	sleep 1

    ps -fs `cat $pidfile` | grep process > /dev/null 2>&1
	if [ $? -ne 0 ]
	then
		printf "$red_clr%50s$end_clr\n" "start ${appname} failed."
		exit 1
	fi
}

stop()
{
    ps aux | grep php | grep process | grep -v php-fpm  | awk '{ print $2}' | xargs kill -9
}

restart()
{
        stop
		delete_crontab
        start $2
		add_crontab
}

state()
{
    ps -fs `cat $pidfile` | grep "process"
#    ps -fs `cat $pidfile` | grep "process\|^UID"
    running=$?
    if [ $running -ne 0 ]
    then
        printf "$red_clr%50s$end_clr\n" "$appname is not running"
        exit 1
    fi
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
		delete_crontab
        ;;
    restart)
        restart 
        ;;
    state)
        state 
        ;;
    *)
        usage 
        ;;
    esac

exit 0
