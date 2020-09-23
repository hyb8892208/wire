#!/bin/sh

start_svr()
{
	switch=`/my_tools/set_config /etc/asterisk/simemusvr.conf get option_value SimEmuSvr simemusvr_switch`
	if [ x"$switch" = x"yes" ]; then
		/my_tools/SimEmuSvr > /dev/null 2>&1 &
		/my_tools/keeper "start:SimEmuSvr:/my_tools/SimEmuSvr >/dev/null 2>&1"	
	fi
}

stop_svr()
{
	kill -9 `pidof SimEmuSvr`
	/my_tools/keeper "stop:SimEmuSvr"
}

emu_switch()
{
	switch=`/my_tools/set_config /etc/asterisk/simemusvr.conf get option_value SimEmuSvr simemusvr_switch`
	if [ x"$switch" = x"yes" ]; then
		stop_svr
		while (true); do
			sleep 1
			result=`asterisk -rx "gsm set sim disable"`
			echo $result
			if  [ x"$result" = x"Set simcard disable successful" ]; then
				   break
			fi
		done
		
		cd /my_tools/lua/emu_Autocheck/ && lua emu_udp_remote.lua > /tmp/log/emu.log
		start_svr
	elif [ x"$switch" = x"no" ]; then
		stop_svr
		while (true); do
			sleep 1
			result=`asterisk -rx "gsm set sim enable"`
			echo $result
			if  [ x"$result" = x"Set simcard enable successful" ]; then
				   break
			fi
		done

		cd /my_tools/lua/emu_Autocheck/ && lua emu_udp_local.lua > /tmp/log/emu.log
	else
		echo "read simemusvr.cof error.........."
	fi
}	

case "$1" in
	start)
		start_svr
		;;
	stop)
		stop_svr
		;;
	switch)
		emu_switch
		;;
	restart)
		stop_svr
		start_svr
		exit 0
		;;
	*)
		echo "Usage: $0 {start|stop|restart|switch}"
		exit 2
		;;
esac

exit 0
