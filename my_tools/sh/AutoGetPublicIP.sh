#!/bin/sh

PARAM="externaddr"
SIP_GENERAL_CONF="/etc/asterisk/sip_general.conf"
GW_GENERRAL_CONF="/etc/asterisk/gw_general.conf"
curl[1]="http://myip.dnsdynamic.org/"
curl[2]="myip.dnsomatic.com"
curl[3]="whatismyip.akamai.com"
curl[4]="ipecho.net/plain"
EXTERNADDR=''
EXTERNADDR_VALUE=''
PUBLIC_IP=''

get_public_ip(){
	for(( i=1; i<5; i++ ));
	do
		#echo "${curl[$i]}"
		PUBLIC_IP=`timeout -t 10 wget -q -O - "${curl[$i]}"`
		if [ x"${PUBLIC_IP}" != x ];then
			return 0
		fi

	done
	return 1
}

compare_ip(){
	EXTERNADDR_FLAG=`grep '^externaddr' ${SIP_GENERAL_CONF}`
	if [ x"$?" != x0 ]; then
		EXTERNADDR=""
	else
		EXTERNADDR="exist"
		EXTERNADDR_VALUE=`grep 'externaddr=' ${SIP_GENERAL_CONF} | awk -F '=' '{print $2}' | awk -F ':' '{print $1}'`
	fi

	if [ x"${EXTERNADDR_VALUE}" = x"${PUBLIC_IP}" ];then
		return 0
	fi
	return 1
}

update_gw_general_conf(){
	RESULT=$1
	sed -i '/^externaddr/d' ${GW_GENERRAL_CONF}
	echo "${PARAM}=${RESULT}" >> ${GW_GENERRAL_CONF}
}

apply_new_ip(){
	asterisk -rx "sip reload" > /dev/null 2>&1
}

update_sip_general_conf(){
	EXTERNADDR_PORT=`grep 'externaddr=' ${SIP_GENERAL_CONF} | awk -F ':' '{print $2}'`
	if [ x"${EXTERNADDR}" = x -o x"${EXTERNADDR_VALUE}" = x ]; then
		sed -i '/^externaddr/d' ${SIP_GENERAL_CONF} > /dev/null 2>&1
		if [ x"${EXTERNADDR_PORT}" = x ]; then
			echo "${PARAM}=${PUBLIC_IP}" >> ${SIP_GENERAL_CONF}
		else
			echo "${PARAM}=${PUBLIC_IP}:${EXTERNADDR_PORT}" >> ${SIP_GENERAL_CONF}
		fi
	else
		if [ x"${EXTERNADDR_VALUE}" != x"${PUBLIC_IP}" -a x"${EXTERNADDR_PORT}" != x ]; then
			sed -i 's/'externaddr=${EXTERNADDR_VALUE}:${EXTERNADDR_PORT}'/'externaddr=${PUBLIC_IP}:${EXTERNADDR_PORT}'/g' ${SIP_GENERAL_CONF}
		elif [ x"${EXTERNADDR_VALUE}" != x"${PUBLIC_IP}" -a x"${EXTERNADDR_PORT}" = x ];then
			sed -i 's/'externaddr=${EXTERNADDR_VALUE}'/'externaddr=${PUBLIC_IP}'/g' ${SIP_GENERAL_CONF}
		fi
	fi
}

auto_set_sip_general_ip(){

	get_public_ip
	if [ x"$?" != x0 ];then
		update_gw_general_conf 0
		return 1
	fi
	#echo "public ip:${PUBLIC_IP}"
	compare_ip
	if [ x"$?" = x0 ];then
		update_gw_general_conf 1
		#echo "compare ip ok,ip is equal"
		return 0
	fi

	#echo "get sip general ok"
	update_sip_general_conf

	update_gw_general_conf 1

	apply_new_ip
}

auto_set_sip_general_ip &

exit 0
