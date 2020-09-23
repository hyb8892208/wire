#!/bin/bash

query_start()
{
	/my_tools/sim_query 1 &
	/my_tools/keeper "start:sim_query:/my_tools/sim_query 1 &"
}


query_stop()
{
	/my_tools/keeper "stop:sim_query"
	kill -9 `pidof sim_query`
}



case "$1" in
	start)
		query_start
		;;
	stop)
		query_stop
		;;
	restart)
		query_stop
		query_start
		exit 0
		;;
	*)
		echo "Usage: $0 {start|stop|restart|switch}"
		exit 2
		;;
esac

exit 0 
