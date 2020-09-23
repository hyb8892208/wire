#!/bin/sh

killps()
{
        ps | while read line;do
        NAME=`echo $line | awk '{print $3}'`
        if [ "$NAME" = "$1" ]; then
                 PID=`echo $line | awk '{print $1}'`
                 kill -9 $PID
        fi
        done
}

calllimit_start(){                                                 
        /my_tools/calllimit_server >/dev/null 2>&1 &                     
} 

calllimit_stop(){
	killps /my_tools/calllimit_server $$ 
}

case "$1" in
	start)
		calllimit_start
		;;
	stop)
		calllimit_stop 
		;;
	restart)
		calllimit_stop
		sleep 1
		calllimit_start
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 2
		;;
esac

exit 0
