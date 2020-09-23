################################################################################
## gen_config.sh脚本
## 根据hw_info.cfg产生/etc/asterisk下的gw_gsm.conf跟extra-channels.conf文件；
## 同时修改define.inc文件的通道总数。
################################################################################
#!/bin/sh

CONF_PATH=/etc/asterisk
GW_GSM_FILE=${CONF_PATH}/gw_gsm.conf
EXTRA_CHANS_FILE=${CONF_PATH}/extra-channels.conf
EXT_ROUTE_FILE=${CONF_PATH}/extensions_routing.conf
DEFINE_INC_FILE=/www/cgi-bin/inc/define.inc
HWINFO_FILE=/tmp/hw_info.cfg
GEN_EXTRA_CHNL_TOOLS=/my_tools/gen_extra_channels
SET_CONFIG_TOOLS=/my_tools/set_config
DEV_NULL=/dev/null

CHAN_MAX_COUNT=44


add_gw_gsm()
{
	chan_id=$1
	chan_id_plus_one=$(($chan_id+1))
	save_file=$GW_GSM_FILE
	brd_mcu_id=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "channel" "chan_${chan_id_plus_one}_brd_id"`
	if [ ${brd_mcu_id} = -1 ]; then
		${SET_CONFIG_TOOLS} ${save_file} set option_value ${chan_id_plus_one} "brd_mcu_serial" ${DEV_NULL}
	else
		brd_mcu_serial=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "mod_brd" "mod_brd_${brd_mcu_id}"`
		${SET_CONFIG_TOOLS} ${save_file} set option_value ${chan_id_plus_one} "brd_mcu_serial" ${brd_mcu_serial}
	fi
	module_usb_com=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "channel" "chan_${chan_id_plus_one}"`
	chan_type=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "channel" "chan_${chan_id_plus_one}_type"`
	if [ "${chan_type}" == "EC20CE" ]; then
		at_timeout=100
	else
		at_timeout=10
	fi

	cat >>$save_file <<_EOF
[$chan_id_plus_one]
name=
tosip=
vol=50
mic=8
dacgain=-15
adcgain=-3
anonymouscall=off
call_waiting=off
band=
dialprefix=
needpin=
pin=
hw_port=$chan_id_plus_one
brd_mcu_serial=$brd_mcu_serial
module_usb_com=$module_usb_com
at_timeout=$at_timeout
dl_single_sw=off
dl_total_sw=off

_EOF
}

create_gw_gsm_config()
{
	echo -n >$GW_GSM_FILE

	totol_count=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "sys" "total_chan_count"`
	max_count=$CHAN_MAX_COUNT
	for((i=0; i<$max_count && i<$totol_count; i=i+1))
	{
		add_gw_gsm $i
	}

}

modify_gw_gsm()
{
	chan_id=$1
	chan_id_plus_one=$(($chan_id+1))
	save_file=$GW_GSM_FILE

	brd_mcu_id=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "channel" "chan_${chan_id_plus_one}_brd_id"`
	if [ ${brd_mcu_id} = -1 ]; then
		${SET_CONFIG_TOOLS} ${save_file} set option_value ${chan_id_plus_one} "brd_mcu_serial" ${DEV_NULL}
	else
		brd_mcu_serial=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "mod_brd" "mod_brd_${brd_mcu_id}"`
		${SET_CONFIG_TOOLS} ${save_file} set option_value ${chan_id_plus_one} "brd_mcu_serial" ${brd_mcu_serial}
	fi
	
	module_usb_com=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "channel" "chan_${chan_id_plus_one}"`
	${SET_CONFIG_TOOLS} ${save_file} set option_value ${chan_id_plus_one} "module_usb_com" ${module_usb_com}

}

modify_gw_gsm_config()
{
	totol_count=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "sys" "total_chan_count"`
	max_count=$CHAN_MAX_COUNT
	for((i=0; i<$max_count && i<$totol_count; i=i+1))
	{
		modify_gw_gsm $i
	}
}

create_extra_channels_config()
{
	${GEN_EXTRA_CHNL_TOOLS}
}

add_extension_route()
{
	chan_id=$1
	save_file=$EXT_ROUTE_FILE

	cat >>$save_file <<_EOF
[gsm-${chan_id}]
exten => s,1,NoOp(Nothing to do, Not setting out channel)
exten => s,n,Hangup()
exten => sms,1,NoOp(SMS In)

_EOF
}

add_other_externsion_route()
{
	save_file=$EXT_ROUTE_FILE

	cat >>$save_file <<_EOF
[nothingtodo]
exten => _[*#+0-9].,1,NoOp(Nothing to do, Not setting out channel)
exten => _[*#+0-9].,n,Hangup()

[sipinbound]
exten => _[*#+0-9].,1,NoOp(SIP Inbound)
exten => _[*#+0-9].,n,GotoIf(\${DIALPLAN_EXISTS(\${SIPROUTE},\${EXTEN},1)}?:nocdr)
exten => _[*#+0-9].,n,Goto(\${SIPROUTE},\${EXTEN},1)
exten => _[*#+0-9].,n(nocdr),Goto(nocdr,s,1)

[iaxinbound]
exten => _[*#+0-9].,1,NoOp(IAX Inbound)
exten => _[*#+0-9].,n,GotoIf(\${DIALPLAN_EXISTS(\${IAXROUTE},\${EXTEN},1)}?:nocdr)
exten => _[*#+0-9].,n,Goto(\${IAXROUTE},\${EXTEN},1)
exten => _[*#+0-9].,n(nocdr),Goto(nocdr,s,1)

[nocdr]
exten => s,1,ChannelHangup("\${CDR_TOCHAN}")
exten => s,n,Hangup()

[globals]
SIPROUTE=sipdefault

_EOF
}

create_extensions_route_config()
{
	echo -n >$EXT_ROUTE_FILE

	totol_count=`${SET_CONFIG_TOOLS} ${HWINFO_FILE} get option_value "sys" "total_chan_count"`
	max_count=$CHAN_MAX_COUNT
	sum=0
	for((i=0; i<$max_count && i<$totol_count; i=i+1))
	{
		add_extension_route $(($i+1))
	}

	add_other_externsion_route
}

modify_define_inc()
{
	define_file=$DEFINE_INC_FILE
	total_count=$(cat /tmp/.boardtype)

	sed -i "s/\$__GSM_SUM__=.*$/\$__GSM_SUM__=${total_count};/g" $define_file
}


main()
{
	TASK="";
	#Parse argument
	while [ $# -gt 0 ]; do
		case $1 in
			"create_gw_gsm")
				create_gw_gsm_config
				exit 0
				;;
			"modify_gw_gsm")
				modify_gw_gsm_config
				exit 0
				;;
			"create_extra_channels")
				create_extra_channels_config
				exit 0
				;;
			"create_extensions_route")
				create_extensions_route_config
				exit 0
				;;
			"modify_define_inc")
				modify_define_inc
				exit 0
				;;
			"*")
				;;
		esac
		shift
	done

	if [ ! -e ${GW_GSM_FILE} -o x"$(grep brd_mcu_serial ${GW_GSM_FILE})" = x ]; then
		create_gw_gsm_config
	else
		modify_gw_gsm_config
	fi
	if [ ! -e ${EXT_ROUTE_FILE} -o x"$(grep sipinbound ${EXT_ROUTE_FILE})" = x ]; then
		create_extensions_route_config
	fi
	create_extra_channels_config
	modify_define_inc
	
	exit 0
}

main $*
