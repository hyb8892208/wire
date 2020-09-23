#!/bin/sh

HW_INFO_CFG=/tmp/hw_info.cfg
LAN_CONF=/etc/asterisk/gw/network/lan.conf
WAN_CONF=/etc/asterisk/gw/network/wan.conf
## According to the configure file of /tmp/hw_info.cfg to generate the Array of $module_type_arr,
## the $module_type_arr is used to save the channel's index and type
eval $(grep -r "^chan_.*_type" $HW_INFO_CFG | awk -F "=" '{split($1, tmp_arr, "_"); print "module_type_arr["tmp_arr[2]"]="$2}')

REDIS_CLI=/my_tools/redis-cli  
CALL_COUNT_KEY="app.asterisk.callcount.channel"

print_usage()
{
	echo "The Usage of lcd_info.sh: "
	echo "$0 system_status"
	echo "$0 network_wan_info"
	echo "$0 network_lan_info"
	echo "$0 device_info"
	echo "$0 network_wan_setting [dhcp|static|disable]"
	echo "$0 network_lan_setting [dhcp|static|factory]"
	echo "$0 web_access"
	echo "$0 ssh_access [stop|start]"
	echo "$0 reboot_setting"
	echo "$0 factory_reset"
	echo "$0 language_type"
	echo "$0 language_setting [EN|CH]"
}

get_active_chan_sum()
{
	active_chan_sum=0
	for((index=1; index<=${#module_type_arr[@]}; index++))
	do
		if [ -e /tmp/gsm/${index} ]; then
		    UP_FLAG=0
			## Getting the value of channel's Singnal Quality
		    span_signal_quality=`cat /tmp/gsm/${index} | grep "Signal Quality" | awk -F ":" '{print $2}'`
		    ## Removing the space character in $span_signal_quality
		    span_signal_quality=`echo $span_signal_quality | sed s/[[:space:]]//g`
            span_status=`cat /tmp/gsm/${index} | grep "^Status" | awk -F ":" '{print $2}'`
			while (($UP_FLAG != 1))
			do
				echo $span_status | grep "Power on, Provisioned, Up" > /dev/null 2>&1
				if [ "$?" = 0 ]; then
					UP_FLAG=1
					break
				fi
				echo $span_status | grep "Power on, Provisioned, Block" > /dev/null 2>&1
				if [ "$?" = 0 ]; then
					UP_FLAG=1
					break
				fi 
				echo $span_status | grep "Power on, Provisioned, In Alarm, Up" > /dev/null 2>&1
				if [ "$?" = 0 ]; then
					 UP_FLAG=1
					 break
				fi
				break
			done

		    if [ "$UP_FLAG" = "1" ]; then
		        ## the channel is considered to active when the $span_signale_quality is greater than 1
		        if [ "${span_signal_quality}" -gt "1" ]; then
		            let active_chan_sum=active_chan_sum+1
		        fi
		    fi
    	fi
	done
	echo $active_chan_sum
}
get_current_call_sum()
{
	call_chan_sum=0
	for((index=1; index<=${#module_type_arr[@]}; index++))
	do
		## query the span call status
		span_call_status=`$REDIS_CLI hget app.asterisk.callcount.channel ${index}`
		if [ x"${span_call_status}" = x1 ];then
			let call_chan_sum=call_chan_sum+1
		fi
	done
	echo $call_chan_sum
}

get_memory_usage()
{
	memory_used=`cat /proc/meminfo 2>/dev/null  | awk '{if(NR==1){A=$2}if(NR==2){B=$2}}END{print (A-B)/A*100}' 2>/dev/null`

	echo "${memory_used}%"	
}

get_storage_useage()
{

	storge_usage=`df -m |grep data |awk '{print $5}'`
	storge_total_size=`df -h | grep data | awk '{print $2}'`
	storge_used_size=`df -h | grep data | awk '{print $3}'`
	echo "${storge_used_size}/${storge_total_size}(${storge_usage})"
}

get_model_name()
{
	mode_type=`/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type`
	chan_count=`/my_tools/set_config /tmp/hw_info.cfg get option_value sys total_chan_count`

	if [ x"$mode_type" = x"1" ]; then
		if [ x"$chan_count" = x"16" ]; then
		model_name="SWG-2016"
		echo $model_name
		elif [ x"$chan_count" = x"32" ]; then
			model_name="SWG-2032"
			echo $model_name
		fi
	
		elif [ x"$mode_type" = x"3" ]; then
			if [ x"$chan_count" = x"20" ]; then
			model_name="VS_USB-1020"
			echo $model_name
		elif [ x"$chan_count" = x"44" ]; then
			model_name="VS_USB-1044"
			echo $model_name
		fi
	fi
}
get_model_description()
{
	cdma_flag=0
	for((index=1; index<=${#module_type_arr[@]}; index++))
    do
        if [ x"${module_type_arr[$index]}" = xSIMCOM_SIM6320C ]; then
        	cdma_flag=1 
        	break                                                
        fi
    done  

    if [ x"${cdma_flag}" != x0 ]; then
        echo "800MHz@CDMA 2000"
    else
        echo "850/900/1800/1900MHz@GSM"
    fi
}

get_system_uptime()
{
	total_time=`awk -F '.' '{print $1}' /proc/uptime`

	local days=0
	local hours=0
	local mins=0
	local secs=0

	let days=total_time/3600/24
	let hours=total_time/3600%24
	let mins=total_time%3600/60
	let secs=total_time%3600%60

	echo "${days}days ${hours}:${mins}:${secs}"
}

gsm_system_status()
{
	active_span_sum=`get_active_chan_sum`
	current_call_sum=`get_current_call_sum`
	storge_usage=`get_storage_useage`
	memory_usage=`get_memory_usage`

	if [ x"${active_span_sum}" = x ]; then
		active_span_sum=0
	fi

	if [ x"${current_call_sum}" = x ]; then
		current_call_sum=0
	fi

	if [ x"${storge_usage}" = x ]; then
		storge_usage=" "
	fi

	if [ x"${memory_usage}" = x ]; then
		memory_usage=" "
	fi

	echo "${active_span_sum},${current_call_sum},${storge_usage},${memory_usage}"
}

gsm_network_lan_info()
{
	network_lan_info_arr=($(/my_tools/net_tool eth0 | awk '{print $1}'))
	lan_mode=`/my_tools/set_config /etc/asterisk/gw/network/lan.conf get option_value general type`
	lan_mac=${network_lan_info_arr[2]}
	lan_ip=`/my_tools/net_tool eth0 | sed -n "4p"`
	lan_netmask=`/my_tools/net_tool eth0 | sed -n "6p"`
	dns=`/my_tools/set_config /etc/asterisk/gw/network/dns.conf get option_value general dns1`

	if [ x"${lan_mode}" = x ]; then
		lan_mode=" "
	fi

	if [ x"${lan_mac}" = x ]; then
		lan_mac=" "
	fi

	if [ x"${lan_ip}" = x ]; then
		lan_ip=" "
	fi

	if [ x"${lan_netmask}" = x ]; then
		lan_netmask=" "
	fi

	if [ x"${dns}" = x ]; then
		dns=" "
	fi

	echo "${lan_mode},${lan_mac},${lan_ip},${lan_netmask},${dns}"
}

gsm_network_wan_info()
{
	network_wan_info_arr=($(/my_tools/net_tool eth1 | awk '{print $1}'))
	wan_state="Disable"                                                                                                    
	wan_state=${network_wan_info_arr[1]}
	wan_mode=`/my_tools/set_config /etc/asterisk/gw/network/wan.conf get option_value general type`
	if [ x"${wan_state}" != xDisable ]; then                                                                               
		wan_mac=${network_wan_info_arr[2]}                                                                             
		wan_ip=`/my_tools/net_tool eth1 | sed -n "4p"`                                                                
		wan_netmask=`/my_tools/net_tool eth1 | sed -n "6p"`                           
		dns=`/my_tools/set_config /etc/asterisk/gw/network/dns.conf get option_value general dns1`                     
	else                                                                                                                   
		wan_mac=                                                                                                       
		wan_ip=                                                                                                        
		wan_netmask=                                                                                                   
		dns=                                                                                                           
	fi

	if [ x"${wan_mode}" = x ]; then
		wan_mode=" "
	fi

	if [ x"${wan_mac}" = x ]; then
		wan_mac=" "
	fi

	if [ x"${wan_ip}" = x ]; then
		wan_ip=" "
	fi

	if [ x"${wan_netmask}" = x ]; then
		wan_netmask=" "
	fi

	if [ x"${dns}" = x ]; then
		dns=" "
	fi
	echo "${wan_mode},${wan_mac},${wan_ip},${wan_netmask},${dns}"
}

gsm_device_info()
{
	mode_name=`get_model_name`
	model_description=`get_model_description`
	software_version=`cat /version/version`
	hardware_version=`$REDIS_CLI get local.product.board.version`
	system_time=`date "+%Y-%m-%d %H:%M:%S"`
	up_time=`get_system_uptime`

	if [ x"${mode_name}" = x ]; then
		mode_name=" "
	fi

	if [ x"{model_description}" = x ]; then
		model_description=" "
	fi

	if [ x"{software_version}" = x ]; then
		software_version=" "
	fi

	if [ x"${hardware_version}" = x ]; then
		hardware_version=" "
	fi

	if [ x"${system_time}" = x ]; then
		system_time=" "
	fi

	if [ x"${up_time}" = x ]; then
		up_time=" "
	fi

	echo "${mode_name},${model_description},${software_version},${hardware_version},${system_time},${up_time}"
}

gsm_web_access()
{
	protocol=`/my_tools/set_config /etc/asterisk/gw/web_server.conf get option_value general login_mode`
	port=`/my_tools/set_config /etc/asterisk/gw/web_server.conf get option_value general port`

	if [ x"${protocol}" = x ]; then
		protocol=" "
	fi

	if [ x"${port}" = x ]; then
		port=" "
	fi

	echo "${protocol},${port}"
}

gsm_ssh_access()
{
	action=$1
	if [ x"${action}" = x"start" ]; then
		/my_tools/set_config /etc/asterisk/gw/ssh.conf set option_value ssh sw on
		/etc/init.d/ssh start> /dev/null 2>&1
	elif [ x"${action}" = x"stop" ]; then
		/my_tools/set_config /etc/asterisk/gw/ssh.conf set option_value ssh sw off
		/etc/init.d/ssh stop > /dev/null 2>&1
	fi

	echo $?
}

gsm_network_wan_setting()
{
	mode=$1
	if [ x"${mode}" = xdhcp ]; then
		/my_tools/set_config $WAN_CONF set option_value general type dhcp
	elif [ x"${mode}" = xstatic ]; then
		/my_tools/set_config $WAN_CONF set option_value general type static
	elif [ x"${mode}" = xdisable ]; then
		/my_tools/set_config $WAN_CONF set option_value general type disable
	fi
	/etc/init.d/wan restart > /dev/null 2>&1

	echo $?
}

gsm_network_lan_setting()
{
	mode=$1
	if [ x"${mode}" = xdhcp ]; then
		/my_tools/set_config $LAN_CONF set option_value general type dhcp
		/etc/init.d/lan restart > /dev/null 2>&1
	elif [ x"${mode}" = xstatic ]; then
		/my_tools/set_config $LAN_CONF set option_value general type static
		/etc/init.d/lan restart > /dev/null 2>&1
	elif [ x"${mode}" = xfactory ]; then
		/my_tools/set_config $LAN_CONF set option_value general type factory
		/etc/init.d/lan restart > /dev/null 2>&1
	elif [ x"${mode}" = xdisable ]; then
		ifconfig eth1 down
	fi

	echo $?
}

gsm_language_type()
{
	language_type="EN"
	local language=`/my_tools/set_config /etc/asterisk/gw/web_language.conf get option_value general language`
	if [ x"${language}" = xchinese ]; then
		language_type="CH"
	elif [ x"${language}" = xenglish ]; then
		language_type="EN"
	else
		exit 255
	fi
	
	echo "$language_type"
}

gsm_language_setting()
{
	language_type=$1
	if [ x"${language_type}" = xCH ]; then
		/my_tools/set_config /etc/asterisk/gw/web_language.conf set option_value general language chinese
		/my_tools/web_language_init > /dev/null 2>&1 &
	elif [ x"${language_type}" = xEN ]; then
		/my_tools/set_config /etc/asterisk/gw/web_language.conf set option_value general language english
		/my_tools/web_language_init > /dev/null 2>&1 &
	else
		exit 255
	fi
}

gsm_reset_lighttpd_login()
{
	/my_tools/set_config /etc/asterisk/gw/web_server.conf set option_value general username admin
	/my_tools/set_config /etc/cfg/gw/web_server.conf set option_value general username admin
	/my_tools/set_config /etc/asterisk/gw/web_server.conf set option_value general password admin
	/my_tools/set_config /etc/cfg/gw/web_server.conf set option_value general password admin
	php -r "include_once('/www/cgi-bin/inc/wrcfg.inc');save_webserver_to_lighttpd();" 
	cp /etc/asterisk/gw/lighttpdpassword_digest /etc/cfg/gw/lighttpdpassword_digest
	/etc/init.d/lighttpd restart > /dev/null 2>&1 &

}
gsm_reboot_setting()
{
	/sbin/reboot
}

gsm_factory_reset()
{
	/my_tools/add_syslog "Factory reset from LCD."
	/my_tools/restore_cfg_file
}

main()
{
	#parse argument
	if [ $# -gt 0 ]; then
		case $1 in
			"system_status")
				TASK="gsm_system_status"
				;;
			"device_info")
				TASK="gsm_device_info"
				;;
			"network_wan_info")
				TASK="gsm_network_wan_info"
				;;
			"network_lan_info")
				TASK="gsm_network_lan_info"
				;;
			"web_access")
				TASK="gsm_web_access"
				;;
			"ssh_access")
				TASK="gsm_ssh_access $2"
				;;
			"network_lan_setting")
				TASK="gsm_network_lan_setting $2"
				;;
			"network_wan_setting")
				TASK="gsm_network_wan_setting $2"
				;;
			"reboot_setting")
				TASK="gsm_reboot_setting"
				;;
			"factory_reset")
				TASK="gsm_factory_reset"
				;;
			"language_type")
				TASK="gsm_language_type"
				;;
			"language_setting")
				TASK="gsm_language_setting $2"
				;;
			"reset_lighttpd_login")
				TASK="gsm_reset_lighttpd_login"
				;;
			*)
				echo "no input param"
				exit 255
				;;
		esac
		if [ x"${TASK}"  != x ];then
			$TASK
			exit 0
		fi
	else
		print_usage
	fi
}

main $*
