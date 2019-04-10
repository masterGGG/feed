#!/bin/bash

red_clr="\033[31m"
grn_clr="\033[32m"
end_clr="\033[0m"

if ! test -e ./bin/daemon.pid; then
	printf "$red_clr%50s$end_clr\n" "CACHE_SERVER is not running"
	exit -1
fi

pid=`cat ./bin/daemon.pid`
result=`ps -p $pid | wc -l`

if test $result -le 1; then
	printf "$red_clr%50s$end_clr\n" "CACHE_SERVER is not running"
	exit -1
fi

result=`ps -p $pid | wc -l`
while ! test $result -le 1; do
	kill `cat ./bin/daemon.pid`
	sleep 1
	result=`ps -p $pid | wc -l`
done

printf "$grn_clr%50s$end_clr\n" "CACHE_SERVER has been stopped"

{
    cwd=`pwd`
    appname='cache_server'
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
    printf "CACHE_SERVER keepalive.sh has been stopped\n"
}
