#!/bin/bash

PROCESS=/my_tools/led_server
pid=`ps -ef |grep "$PROCESS $chan" |grep -v 'grep' | awk -F ' ' '{print $1}'`
if [ ! -z "$pid" ]; then
	kill -9 $pid
	echo "kill $pid" >> /tmp/mylog
fi

#sleep 2
#/my_tools/led_server start >/dev/null 2>&1 &
#echo "led restart" >> /tmp/mylog

exit 0

