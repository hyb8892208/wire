#!/bin/sh

timeout=5


if [ $# -eq 0 ];then
	pppd_pids=`ps |grep pppd.*chat |grep -v grep |awk '{print $1}'`
else
	CHAN_ID=$1
	pppd_pids=`ps |grep pppd.*modppp${CHAN_ID} |grep -v grep |awk '{print $1}'`
	gobinet_pids=`ps |grep quectel-CM.*gobinet_$CHAN_ID\$|grep -v grep |awk '{print $1}'`
	rm -rf /tmp/${CHAN_ID}_*

fi

if [ x"${pppd_pids}" = x"" -a x"${gobinet_pids}" = x"" ];then
	exit 0
fi

for pppd_pid  in ${pppd_pids}
do
	timeout=5
	kill -15 ${pppd_pid}
	sleep 1
	kill -0 ${pppd_pid}
	while [ $? -ne 0 ]
	do
		timeout=`expr $timeout - 1`
		if [ $timeout -eq 0 ]
		then
			break
		fi
		kill -0 ${pppd_pid}
	done
	
	if [ $? -ne 0 ]
	then
		kill -9 ${pppd_pid}
	fi
done

for gobinet_pid in ${gobinet_pids}
do
	kill  ${gobinet_pid}
done


