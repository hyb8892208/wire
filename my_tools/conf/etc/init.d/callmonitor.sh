#!/bin/sh

callmonitor_server_start()
{
	/my_tools/callmonitor_server >/dev/null 2>&1 &
	/my_tools/keeper "start:callmonitor_ser:/etc/init.d/callmonitor.sh start"
}


callmonitor_server_stop()
{
	/my_tools/keeper "stop:callmonitor_ser"
	kill -9 `pidof callmonitor_server`
}

callmonitor_server_reload()
{
	/my_tools/callmonitor_cli config reload > /dev/null
}


case "$1" in
	start)
		callmonitor_server_start
		;;
	stop)
		callmonitor_server_stop
		;;
	restart)
		callmonitor_server_stop
		callmonitor_server_start
		exit 0
		;;
	reload)
		callmonitor_server_reload
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 2
		;;
esac

exit 0 
