#!/bin/sh


PROCESS=/my_tools/module_ec20_upgrade
#chan=$1


pid=`ps -ef |grep "$PROCESS" |grep -v 'grep' | awk -F ' ' '{print $1}'`

if [ ! -z "$pid" ]; then
        kill -9 $pid
fi


exit 0