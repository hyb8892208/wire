#!/bin/sh

DEV_ID=
CHAN_ID=
NET_APN=
NET_USER=
NET_PWD=

source /etc/profile

HW_INFO_CFG=/tmp/hw_info.cfg
#sys_type=`/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type`
total_chan=`/my_tools/set_config /tmp/hw_info.cfg get option_value sys total_chan_count`
NET_DEV="modppp0"

record_log(){
	result=$1
	echo "$result" > /tmp/${CHAN_ID}_dialup.log
}

usage()
{
	echo "usage:"
	echo "$0 -ctavuph"
	echo "-c specfic channel num"
	echo "-t dial type, ppp or gobinet"
	echo "-v ip protol, IPV4 or IPV6 or IPV4V6"
	echo "-a apn"
	echo "-u apn username"
	echo "-p apn passowrd"
	echo "-h print this usage."
	exit 0
}
get_optargs()
{
	while getopts "c:t:v:ahup" opt; do
		case $opt in
			c)
				CHAN_ID=$OPTARG
				;;
			t)
				NET_TYPE=$OPTARG
				;;
			a)
				NET_APN=$OPTARG
				;;
			v)
				NET_PROTOL=$OPTARG
				;;
			u)
				NET_USER=$OPTARG
				;;
			p)
				NET_PWD=$OPTARG
				;;
			h)
				usage $*
				;;
		esac
	
	done
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
	
	if [ -e /sys/class/net/$PPP_NET_DEV ];then
		ppp_busy_chan=`ps |grep pppd.*${PPP_DEV} |grep -v grep |awk -F 'ppp' '{print $NF}'`
		record_log "error: Device busy, used by channel $ppp_busy_chan"
		exit 1
	fi
	#ipv6
	global_link=`ifconfig $GOBINET_NET_DEV 2>/dev/null|grep "Scope:Global"`
	ipv4_link=`ifconfig $GOBINET_NET_DEV 2>/dev/null|grep "inet addr"`
	if [ $? -ne 0 ];then
		return 1
	elif [ x"$ipv4_link" != x"" -o x"$global_link" != x"" ];then
		gobinet_busy_chan=`ps |grep quectel-CM |grep " $GOBINET_NET_DEV " |grep -v grep|awk -F '_' '{print $NF}'`
		record_log "error: Device busy, used by channel $gobinet_busy_chan"
		exit 1
	fi

	return 0
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
	PPP_NET_DEV=modppp$CHAN_ID
	GOBINET_NET_DEV=gobinet${DEV_ID}
	if [ x"$NET_TYPE" = x"ppp" ];then
		NET_DEV=$PPP_NET_DEV
	else
		NET_DEV=$GOBINET_NET_DEV
	fi
}

get_ppp_dev(){
	PPP_DEV="/dev/opvx/internet/${DEV_ID}"
}

set_channel_to_upgrade(){
	/my_tools/bsp_cli upgrade sel -1 > /dev/null
	sleep 1
	/my_tools/bsp_cli upgrade sel $CHAN_ID > /dev/null
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

get_gobinet_protol_args()
{
	if [ x"$NET_PROTOL" = x"IPV4" ];then
		protol_args="-4"
	elif [ x"$NET_PROTOL" = x"IPV6" ];then
		protol_args="-6"
	else
		protol_args="-4 -6"
	fi


}

gobinet_conn_internet()
{
	get_gobinet_protol_args
	/my_tools/quectel-CM ${protol_args} -s $NET_APN $NET_USER $NET_PWD -i ${GOBINET_NET_DEV} -f /tmp/gobinet_$CHAN_ID > /dev/null &
}

dial_conn_internet()
{
	if [ x"${NET_TYPE}" = x"ppp" ];then
		ppp_conn_internet
	else
		gobinet_conn_internet
	fi	
}

check_ppp_conn_result()
{
	count=10
	while [ $count -gt 0 ]
	do
		sleep 1
		ipv6_result=`ifconfig $NET_DEV 2>/dev/null|grep "Scope:Global"`
		ipv4_result=`ifconfig $NET_DEV 2>/dev/null|grep "inet addr"`
		if [ x"$ipv6_result" != x"" -o x"$ipv4_result" != x"" ];then
			echo $NET_DEV
			return 0
		fi
		count=$(( $count - 1 ))
	done
	record_log "error: dialup fail"
	return 4
}

check_gobinet_conn_result()
{
	count=10
	while [ $count -gt 0 ]
	do
		sleep 1
		ipv6_result=`ifconfig $NET_DEV 2>/dev/null|grep "Scope:Global"`
		ipv4_result=`ifconfig $NET_DEV 2>/dev/null|grep "inet addr"`
		if [ x"$ipv6_result" != x"" -o x"$ipv4_result" != x"" ];then
			record_log "dialup success"
			echo $NET_DEV
			return 0
		fi
		count=$(( $count - 1 ))
	done
	record_log "error: dialup fail"
	return 4
}

check_dial_conn_result()
{
	if [ x"${NET_TYPE}" = x"ppp" ];then
		check_ppp_conn_result
	else
		check_gobinet_conn_result
	fi	
}

get_value()
{
	SECTION=$1
	KEY=$2
	CONFILE=/etc/asterisk/gw_internet_apn.conf
	eval $2=`awk -F '=' '/\['"$SECTION"'\]/{a=1}a==1&&$1~/'"$KEY"'/{print $2;exit}' $CONFILE`
}

get_operator_num()
{
	i=0
	while [ $i -lt 5 ]
	do
		operator_num=`cat /tmp/gsm/${CHAN_ID} 2>/dev/null|grep "Network Num"|awk '{print $3}'`
		if [ x$operator_num != x ];then
			break
		fi
		sleep 1
		i=$((i+1))
	done
}

get_default_apn()
{
	get_operator_num
	if [ x"$operator_num" = x"" ];then
		return
	fi
	get_value  $operator_num apn
	get_value  $operator_num username
	get_value  $operator_num password

	NET_APN=$apn
	NET_USER=$username
	NET_PWD=$password
}

get_apn_info()
{
	if [ x$NET_APN != x ];then
		return
	fi
	get_default_apn
}

main()
{
	get_optargs $*
	check_dialup_limit_state
	get_apn_info
	get_dev_id
	get_net_devname
	get_ppp_dev
	check_net_device
	set_channel_to_upgrade
	dial_conn_internet
	check_dial_conn_result
}

main $*
