#!/bin/sh

killps()
{
	PID=`pidof rri_server`
	for i in $PID
	do
		if [ $i != $$ ];then
			kill -9 $i
		fi
        done
}

rri_server_start(){                                                 
        /my_tools/rri_server >/dev/null 2>&1 &                     
	/my_tools/keeper "start:rri_server:/etc/init.d/rri_server.sh start"
} 

rri_server_stop(){
	/my_tools/keeper "stop:rri_server"
	killps
}

case "$1" in
	start)
		rri_server_start
		;;
	stop)
		rri_server_stop 
		;;
	restart)
		rri_server_stop
		sleep 1
		rri_server_start
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 2
		;;
esac

exit 0
