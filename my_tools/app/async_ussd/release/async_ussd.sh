#!/bin/sh

. /etc/functions.sh

ussd_start()
{
	/my_tools/async_ussd >/dev/null 2>&1 &
	/my_tools/keeper "start:async_ussd:/etc/init.d/async_ussd.sh start"
}

ussd_stop()
{
	/my_tools/keeper "stop:async_ussd"
	killps async_ussd $$
}

case "$1" in
	start)
		ussd_start
		;;
	stop)
		ussd_stop
		;;
	restart)
		ussd_stop
		ussd_start
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 2
		;;
esac

exit 0
