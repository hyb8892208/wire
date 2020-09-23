#!/bin/sh

killps()
{
	PID=`pidof bsp_server`
	for i in $PID
	do
		if [ $i != $$ ];then
			kill -9 $i
		fi
        done
}

bsp_server_start(){                                                 
        /my_tools/bsp_server >/dev/null 2>&1 &                     
	/my_tools/keeper "start:bsp_server:/etc/init.d/bsp_server.sh start"
} 

bsp_server_stop(){
	/my_tools/keeper "stop:bsp_server"
	killps
}

bsp_server_restart(){
	bsp_server_stop
	/etc/init.d/rri_server.sh stop
	bsp_server_start
	sleep 15
	/etc/init.d/rri_server.sh start
}

case "$1" in
	start)
		bsp_server_start
		;;
	stop)
		bsp_server_stop 
		;;
	restart)
		bsp_server_restart
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 2
		;;
esac

exit 0
