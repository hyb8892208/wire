#!/bin/sh
############################################--pre--####################################
slot_num=`cat /tmp/.slotnum 2> /dev/null`
slot_type=`cat /tmp/.slot_type 2> /dev/null`
board_type=`cat /tmp/.boardtype 2> /dev/null`

#span_chan=`grep -r "mod_[0-99]_total_chan" /tmp/hw_info.cfg | head -1 | awk -F '=' '{print $2}'`
#sys_type=`grep -r "^sys_type" /tmp/hw_info.cfg | awk -F '=' '{print $2}'`

lua_dir="/my_tools/lua"

kill -9 `pidof lua` > /dev/null 2>&1
kill -9 `ps -ef | grep sms_recv | grep -v grep | awk '{print $1}'` > /dev/null 2>&1
kill -9 `pidof sms_send` > /dev/null 2>&1

################################--MASTER--#############################
if [ $slot_type == 1 ]; then
	#/my_tools/redis-server /etc/asterisk/redis.conf > /dev/null 2>&1 

	cd ${lua_dir}/send/
	lua master.lua > /dev/null 2>&1 &
	#lua http_sms_out.lua > /dev/null 2>&1 &
	#lua listen_slave_http.lua > /dev/null 2>&1 &
	

	#read group info into redis
	cd ${lua_dir}/sms_routing/
	lua read_group.lua > /dev/null 2>&1 

	cd ${lua_dir}/sms_receive/
	./sms_recv > /dev/null 2>&1 &
	./sms_send > /dev/null 2>&1 &

	cd ${lua_dir}/sms_routing/
	lua sms_routing.lua > /dev/null 2>&1 &
	
	cd ${lua_dir}/my_lua_tools/
	lua conf_to_redis.lua init > /dev/null 2>&1 &

	cd ${lua_dir}/my_lua_tools/
	lua insert_port_redis.lua > /dev/null 2>&1 &

	cd ${lua_dir}/info_access/
	lua firewall_config.lua > /dev/null 2>&1 &
fi

#############################--EVENT--#############################
cd ${lua_dir}/event/
lua event.lua > /dev/null 2>&1 &

################################--SEND--#############################
cd ${lua_dir}/send/
n=0
if [ $slot_type -eq 1 ];then
	n=1
else
	n=$slot_num
fi

index=1
while [ $index -le $board_type ];do                             
	lua send.lua $n $index > /dev/null 2>&1 &                              
	let index=index+1           
done  

lua monitor.lua > /dev/null 2>&1 &
lua sms_reports_to_http.lua > /dev/null 2>&1 &
lua sms_results_to_http.lua > /dev/null 2>&1 &
lua ussd_results_to_http.lua > /dev/null 2>&1 &

################################--INFO_ACCESS--#############################
##cd ${lua_dir}/info_access/
##lua monitor_module.lua > /dev/null 2>&1 &
