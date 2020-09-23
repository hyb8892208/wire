#!/bin/sh

stopname=tcpdump

# sleep 180s
#sleep 180 on capture_monitor

#echo "auto stop capture" >> /tmp/mylog
pidstr=`ps -ef |grep "${stopname}" | grep -v grep | awk '{print $1}'`
if [ -n "${pidstr}" ]; then
#	echo "kill pidof(${stopname})=${rootStr}" >> /tmp/mylog                                                          
	kill -9 ${pidstr}
fi
	rm -f /data/*.pcap
/etc/init.d/capture_monitor stop

exit 0
