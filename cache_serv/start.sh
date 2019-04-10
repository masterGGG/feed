#!/bin/bash

red_clr="\033[31m"
grn_clr="\033[32m"
end_clr="\033[0m"
cwd=`pwd`
appname='cache_server'

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

if test -e ./bin/daemon.pid; then
	pid=`cat ./bin/daemon.pid`
	result=`ps -p $pid | wc -l`
	if test $result -gt 1; then
		printf "$red_clr%50s$end_clr\n" "STAT_FRAME is running"
		exit -1
	fi
fi

export LD_LIBRARY_PATH=../../newsfeed-lib/:$LD_LIBRARY_PATH && 
cd ./bin && ./cache_server
add_crontab
# if test $? -ne 0; then
# 	printf "$red_clr%50s$end_clr\n" "failed to start STAT_SERVICE"
# else 
# 	printf "$grn_clr%50s$end_clr\n" "STAT_SERVICE has been started"
# fi

