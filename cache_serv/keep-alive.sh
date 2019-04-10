#!/bin/bash

cd `dirname $0`
appname=$1

./state.sh > /dev/null 2>&1

if [ $? -ne 0 ];then
    echo "${appname} stopped abnormally, try to start it."
    ./stop.sh
    sleep 5
    echo "restart ${appname} at $(date)"
    ./start.sh start
fi

find . -name "core*" | xargs rm -vf 

exit 0
