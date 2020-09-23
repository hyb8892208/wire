#!/bin/sh

handle_mms_start(){
	mms_pid=`pidof handle_mms`
	if [ x"$mms_pid" = x"" ];then
		/my_tools/handle_mms >/dev/null 2>&1 &
		/my_tools/keeper "start:handle_mms:/etc/init.d/handle_mms.sh start"
	fi
}

handle_mms_stop(){
	mms_pid=`pidof handle_mms`
	/my_tools/keeper "stop:handle_mms"
	if [ x"$mms_pid" != x"" ];then
		#SIGINT
		kill -2 $mms_pid
	fi
}

handle_mms_reload(){
	mms_pid=`pidof handle_mms`
	if [ x"$mms_pid" = x"" ];then
		handle_mms_start
	else
		#SIGUSER1
		kill -10 $mms_pid
	fi
}

handle_mms_restart(){
	stop
	start
}

case "$1" in
	start)
		handle_mms_start
		;;  
	stop)
		handle_mms_stop
		;;
	restart)
		handle_mms_stop
		handle_mms_start
		;;
	reload)
		handle_mms_reload
		;;
	*)
		echo "Usage: $0 {start|stop|restart|reload}"
		exit 2
	;;
esac


