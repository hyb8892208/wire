#!/bin/sh


PROCESS=/my_tools/module_m35_upgrade
chan=$1


pid=`ps -ef |grep "$PROCESS $chan" |grep -v 'grep' | awk -F ' ' '{print $1}'`

if [ ! -z "$pid" ]; then
        kill -9 $pid
fi


exit 0