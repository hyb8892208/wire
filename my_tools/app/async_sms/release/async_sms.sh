#!/bin/sh

. /etc/functions.sh

sms_start()
{
	/my_tools/async_sms >/dev/null 2>&1 &
	/my_tools/keeper "start:async_sms:/etc/init.d/async_sms.sh start"
}

sms_stop()
{
	/my_tools/keeper "stop:async_sms"
	killps async_sms $$
}

case "$1" in
	start)
		sms_start
		;;
	stop)
		sms_stop
		;;
	restart)
		sms_stop
		sms_start
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 2
		;;
esac

exit 0
