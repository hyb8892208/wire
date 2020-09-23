#!/bin/sh

source /etc/profile

#opvx-pppd devname apn user password
echo "opvx-pppd options in effect:"
OPVX_PORT=/dev/opvx/internet/0
OPVX_APN=3gnet
OPVX_USER=user
OPVX_PASSWORD=passwd
OPVX_PORT=1
OPVX_PPP="ppp0"
OPVX_PING="ping"
BSP_CLI=/my_tools/bsp_cli
PING_TIME=5
URL="www.baidu.com"
INTERNET_APP=/my_tools/curl
KILL_APP=/my_tools/opvx-ppp-kill
TMP_PORT=1
SECTION=port1

#config
internet_sw=on
username=
passwd=
apn=3gnet
flow_total_size=1
flow_use_size=0
domain=www.baidu.com
url=www.baidu.com

total_channel=0
size=$[1*1024*1024]
use_size=0
one_size=0

total_channel=`grep -rn total_chan_count /tmp/hw_info.cfg | awk -F = '{print $2}'`
#check the network is connected?
test_internet()
{
	echo "$OPVX_PING -I $NET_DEV -w $PING_TIME $URL"
	$OPVX_PING -I $NET_DEV -w $PING_TIME $domain
	if [ $? -ne 0 ]; then
		echo " No network, exit "
		return 1
	fi
	return 0
}

get_one_size()
{

	tmp=`ls -al /tmp/internet1_log |awk '{print $5}'`
	one_size=$[tmp*3]
}

#get ppp0 rx size and tx size
get_use_size()
{
	use_size=`cat /proc/net/dev|grep $NET_DEV|awk '{print $2+$10}'`
}

#surf the internet
go_to_internet()
{
	count=5
	while [ $count -gt 0 ]
	do
		$INTERNET_APP --interface $NET_DEV --insecure $URL -o /tmp/internet${OPVX_PORT}_log
		if [ $? -eq 0 ];then
			echo "get $URL success"
			return 0
		fi
		sleep 1
		count=$[count-1]
	done
	return 1
}

get_value()
{
	SECTION=$1
	KEY=$2
	CONFILE=/etc/asterisk/gw_internet.conf
	eval $2=`awk -F '=' '/\['"$SECTION"'\]/{a=1}a==1&&$1~/'"$KEY"'/{print $2;exit}' $CONFILE`
}


get_config()
{
	get_value $1 internet_sw
	get_value $1 user
	get_value $1 passwd
	get_value $1 apn
	get_value $1 flow_total_size
	get_value $1 flow_use_size
	get_value $1 domain
	get_value $1 url
	get_value $1 dail_type
	get_value $1 pdp_type

	use_size=$flow_use_size
	size=$[flow_total_size * 1024 * 1024]
	echo "$size"
	OPVX_APN=$apn
	OPVX_USER=$user
	OPVX_PASSWORD=$passwd
	URL=$url

	if [ x"$pdp_type" = x"" ];then
		pdp_type="IPV4V6"
	fi

	if [ x"$dail_type" = x"" ];then
		dail_type="ppp"
	fi

	if [ x"$internet_sw" == x"on" ];then
		return 0
	fi

	return 1    
}

#pppd
ppp_and_internet()
{
	NET_DEV=`/my_tools/module_connect_net.sh -t $dail_type -v $pdp_type -a $OPVX_APN -u $OPVX_USER -p $OPVX_PASSWORD -c $OPVX_PORT`
	if [ x"$NET_DEV" = x"" ];then
		return 1
	fi
	test_internet
	if [ $? != 0 ];then
		$KILL_APP
		return 1
	fi

	go_to_internet
	get_use_size
	one_size=$use_size
	while [ `expr $use_size + $one_size` -le $size ]
	do
		if [ $? != 0 ];then
			return 1
		fi
		go_to_internet
		
		/my_tools/set_config /etc/asterisk/gw_internet.conf set option_value $SECTION flow_use_size $use_size
		
		get_use_size
	done

	$KILL_APP

	return 0
}


everyone_port_internet()
{
	test_count=3
	for ((OPVX_PORT=1; OPVX_PORT<=$total_channel;OPVX_PORT++))
	do
		get_config port$OPVX_PORT
		if [ $? -eq 1 ];then
			continue
		fi
		ppp_and_internet
		for((i=0;i<$test_count && $?!=0;i++))
		do
			ppp_and_internet
		done
	done
}

everyone_port_internet
