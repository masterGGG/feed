#########################################################################
# File Name: stop.sh
# Author: ian
# mail: ian@taomee.com
# Created Time: Tue 05 Mar 2019 11:00:34 AM CST
#########################################################################
#!/bin/bash

ps aux | grep php | grep process | grep -v php-fpm  | awk '{ print $2}' | xargs kill -9

