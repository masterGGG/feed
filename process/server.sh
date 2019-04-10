#########################################################################
# File Name: server.sh
# Author: ian
# mail: ian@taomee.com
# Created Time: Tue 02 Apr 2019 18:16:34 PM CST
#########################################################################
#!/bash

pidfile=process.pid

cwd=`pwd`
appname='process'

function add_crontab()
{
    tmpfile=crontab-ori.tempXX
    item1='*/1 * * * * cd '${cwd}' && ./keep-alive.sh '${appname}' >> keep-alive.log 2>&1'
    item2='*/10 * * * * cd '${cwd}' && python error_warning.py 10 >error_warning.log 2>&1'

    crontab -l >$tmpfile 2>/dev/null

    fgrep "${item1}" $tmpfile &>/dev/null
    if [ $? -ne 0 ]
    then
        echo "${item1}" >> $tmpfile
    fi

    fgrep "${item2}" $tmpfile &>/dev/null
    if [ $? -ne 0 ]
    then
        echo "${item2}" >> $tmpfile
    fi

    crontab $tmpfile
	rm -f $tmpfile
}
function delete_crontab()
{
    tmpfile1=crontab-ori.temp1XX
    tmpfile2=crontab-ori.temp2XX
    tmpfile3=crontab-ori.temp3XX
    item1='*/1 * * * * cd '${cwd}' && ./keep-alive.sh '${appname}' >> keep-alive.log 2>&1'
    item2='*/10 * * * * cd '${cwd}' && python error_warning.py 10 >error_warning.log 2>&1'

    crontab -l >$tmpfile1 2>/dev/null

    fgrep "${item1}" $tmpfile1 &>/dev/null
    if [ $? -eq 0 ]
    then
	    fgrep -v "${item1}" $tmpfile1 &> $tmpfile2
		    crontab $tmpfile2
    fi

    fgrep "${item2}" $tmpfile1 &>/dev/null
    if [ $? -eq 0 ]
    then
    	fgrep -v "${item2}" $tmpfile2 &> $tmpfile3
	    crontab $tmpfile3
    fi

    rm -f $tmpfile1
    rm -f $tmpfile2
    rm -f $tmpfile3
    printf "process keepalive.sh has been stopped\n"
}

function start()
{
	./check-single $pidfile

	if [ $? -eq 1 ]
	then
		printf "$red_clr%50s$end_clr\n" "${appname} is already running"
		exit 1
	fi
	
    php process.php
    ps aux | grep php | grep process | grep -v php-fpm  | awk '{ print $2}' | head -1 > $pidfile

	sleep 1
	./check-single $pidfile
	if [ $? -eq 0 ]
	then
		printf "$red_clr%50s$end_clr\n" "start ${appname} failed."
		exit 1
	fi
}

function stop()
{
    ./check-single $pidfile
    running=$?
    if [ $running -eq 0 ]
    then
        printf "$red_clr%50s$end_clr\n" "$appname is not running"
        exit 1
    fi

    while [ $running -eq 1 ]
    do
        kill `cat $pidfile`
        sleep 1
        ./check-single $pidfile
        running=$?
    done
    rm $pidfile
}

function restart()
{
	stop
	start
}

function state()
{
    ./check-single $pidfile
    running=$?
    if [ $running -eq 0 ]
    then
        printf "$red_clr%50s$end_clr\n" "$appname is not running"
        exit 1
    fi

    ps -fs `cat $pidfile`
}

function usage()
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
        start $2
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
