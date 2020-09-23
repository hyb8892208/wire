#!/bin/sh


PROCESS=/my_tools/module_sim6320c_upgrade
chan=$1


pid=`ps -ef |grep "$PROCESS -c $chan" |grep -v 'grep' | awk -F ' ' '{print $1}'`

if [ ! -z "$pid" ]; then
        kill -9 $pid
fi


exit 0
