#!/bin/sh

CHAN_ID=$1
TYPE=$2
URL=$3
SIZE=$4

HW_INFO_CFG=/tmp/hw_info.cfg
total_chan=`/my_tools/set_config /tmp/hw_info.cfg get option_value sys total_chan_count`

NET_DEV="modppp0"
INTERNET_LOG=/tmp/${CHAN_ID}_internet_log

clean_log(){
	if [ -f /tmp/${CHAN_ID}_surf_internet.log ];then
		rm -rf /tmp/${CHAN_ID}_surf_internet.log
	fi

	if [ -f ${INTERNET_LOG}  ];then
		rm -rf  ${INTERNET_LOG}
	fi
}

record_log(){
	echo $1 > /tmp/${CHAN_ID}_surf_internet.log
}

check_dialup_limit_state(){
	limitstat=`/my_tools/redis-cli hget app.module.dialup.limitstat.channel $CHAN_ID`
	if [ $? -eq 0 ];then
		if [ x"$limitstat" = x"1" ];then
			record_log "error: channel $CHAN_ID dialup has been limited"
			exit 
		fi
	fi
}

check_net_device(){
	if [ ! -d /sys/class/net/$NET_DEV ];then
		record_log "error: Channel have not dialup"
		exit 1
	fi
	result=0
}

check_dev_id(){

	if [ $CHAN_ID -lt 1 ];then
		record_log "error: Channel id is invaild"
		exit 2
	elif [ $CHAN_ID -gt $total_chan ];then
		record_log "error: Channel id is invaild"
		exit 2
	fi
}

get_net_devname(){
	NET_DEV=modppp$CHAN_ID
}

get_usesize(){
	cat /proc/net/dev|grep ${NET_DEV}|awk '{print $2+$10}'
}

surf_internet(){
	/my_tools/curl --connect-timeout 5 --interface $NET_DEV --insecure $URL -o $INTERNET_LOG
	return $?
}

must_internet(){
	surf_internet
	if [ $? -eq 0 ];then
		record_log "info: Success, filename is $INTERNET_LOG"
		exit 0
	fi
	record_log "error: Requst timeout"
	exit 4
}

none_internet(){
	if [ "$URL" = "" ];then
		print_usage
		exit 1
	fi

	if [ "$SIZE" = "" ];then
		SIZE=0
	fi

	if [ $SIZE -le 0 ];then
		while [ true ]
		do
			surf_internet
			if [ $? -eq 45 ];then
				record_log "dialup disconnect"
				exit 3
			fi
		done
	else
		last_size=`get_usesize`
		surf_internet
		cur_size=`get_usesize`
		one_size=$(( $cur_size - $last_size ))
		while [ $(( $cur_size - $last_size + $one_size )) -le $total_size ]
		do
			last_size=$cur_size
			surf_internet
			if [ $? -eq 45 ];then # disconnect pppoe
				record_log "dialup disconnect"
				exit 3
			fi
			cur_size=`get_usesize`
		done
	fi
}

size_internet(){
	if [ "$URL" = "" ];then
		use_size=`get_usesize`
		record_log $use_size
	else
		last_size=`get_usesize`
		surf_internet
		cur_size=`get_usesize`
		use_size=$(( $cur_size - $last_size ))
		record_log $use_size
	fi
}

print_usage(){
	echo "usage: $0 \$channel \$type \$url"
}

main(){

	clean_log
	check_dialup_limit_state
	get_net_devname
	check_net_device

	case $TYPE in
		"must")
			must_internet
			;;
		"none")
			none_internet
			;;
		"size")
			size_internet
			;;
		"")
			print_usage
			;;
	esac
}

main

