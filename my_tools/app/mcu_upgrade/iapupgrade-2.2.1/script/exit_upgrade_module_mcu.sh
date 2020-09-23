#!/bin/sh


PROCESS=/my_tools/iapupgrade
SCRIPT=/my_tools/upgrade_module_mcu.sh
mcu_id=$1
channel=$((mcu_id*2-1))

pid=`ps -ef |grep "$PROCESS" |grep -v 'grep' | awk -F ' ' '{print $1}'`
if [ ! -z "$pid" ]; then
        kill -9 $pid
fi

pid=`ps -ef |grep "$SCRIPT" |grep -v 'grep' | awk -F ' ' '{print $1}'`
if [ ! -z "$pid" ]; then
        kill -9 $pid
fi


/my_tools/rri_cli chn com reopen $channel

exit 0
