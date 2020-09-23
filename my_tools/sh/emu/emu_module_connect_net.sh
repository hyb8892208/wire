#!/bin/sh

DEV_ID=
CHAN_ID=$1
NET_APN=$2
NET_USER=$3
NET_PWD=$4

HW_INFO_CFG=/tmp/hw_info.cfg
#sys_type=`/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type`
total_chan=`/my_tools/set_config /tmp/hw_info.cfg get option_value sys total_chan_count`
NET_DEV="modppp0"

record_log(){
	result=$1
	echo "$result" > /tmp/${CHAN_ID}_dialup.log
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
	if [ ! -e /sys/class/net/$NET_DEV ];then
		return 0
	fi
	busy_chan=`ps |grep pppd.*${PPP_DEV} |grep -v grep |awk -F 'ppp' '{print $NF}'`
	record_log "error: Device busy, used by channel $busy_chan"
	exit 1
}

get_dev_id(){

	if [ $CHAN_ID -le 0 ];then
		record_log "error: invaild channel id"
		exit 2
	elif [ $CHAN_ID -gt $total_chan ];then
		record_log "error: invaild channel id"
		exit 2
	fi

	if [ $total_chan -le 4 ];then
		DEV_ID=$(( $CHAN_ID - 1 )) 
	elif [ $total_chan -le 8 ];then
		DEV_ID=$(( $(( $CHAN_ID - 1 )) / 4 ))
	elif [ $total_chan -le 16 ];then
		DEV_ID=$(( $(( $CHAN_ID - 1 )) / 16 ))
	elif [ $total_chan -le 20 ];then
		DEV_ID=$(( $(( $CHAN_ID - 1 )) / 4 ))
	elif [ $total_chan -le 32 ];then
		DEV_ID=$(( $(( $CHAN_ID - 1 )) / 16 ))
	elif [ $total_chan -le 44 ];then
		DEV_ID=$(( $(( $CHAN_ID - 1 )) / 4 ))
	fi 
}

get_net_devname(){
	NET_DEV=modppp$CHAN_ID
}

get_ppp_dev(){
	PPP_DEV="/dev/opvx/internet/${DEV_ID}"
}

set_channel_to_upgrade(){
	/my_tools/bsp_cli upgrade sel $CHAN_ID
	sleep 1
}

ppp_conn_internet()
{
	CONNECT="'chat -s -v ABORT BUSY ABORT \"NO CARRIER\" ABORT \"NO DIALTONE\" ABORT ERROR ABORT \"NO ANSWER\" TIMEOUT 30 \
        \"\" AT OK ATE0 OK ATI\;+CSUB\;+CSQ\;+CPIN?\;+COPS?\;+CGREG?\;\&D2 \
        OK AT+CGDCONT=1,\\\"IP\\\",\\\"$NET_APN\\\",,0,0 OK ATD*99# CONNECT'"

	pppd $PPP_DEV 115200 user "$NET_USER" password "$NET_PWD" \
        connect "'$CONNECT'" \
        disconnect 'chat -s -v ABORT ERROR ABORT "NO DIALTONE" SAY "\nSending break to the modem\n" "" +++ "" +++ "" +++ SAY "\nGood bay\n"' \
        noauth debug defaultroute noipdefault novj novjccomp noccp ipcp-accept-local ipcp-accept-remote ipcp-max-configure 30 local lock modem \
        dump nodetach nocrtscts usepeerdns ifname $NET_DEV > /dev/null &
	if [ $? -ne 0 ];then
		record_log "error: call dailup fail"
		exit 3
    	fi
	return 0
}

check_ppp_conn_result(){
	count=10
	while [ $count -gt 0 ]
	do
		sleep 1
		if [ -e /sys/class/net/$NET_DEV ];then
			record_log "dialup success"
			return 0
		fi
		count=$(( $count - 1 ))
	done
	record_log "error: dialup fail"
	return 4
}

main()
{
	check_dialup_limit_state
	get_dev_id
	get_net_devname
	get_ppp_dev
	check_net_device
	set_channel_to_upgrade
	ppp_conn_internet
	check_ppp_conn_result
}

main $*
