#!/bin/sh

. /etc/functions.sh

license_start()
{
	/my_tools/anlslicense >/dev/null 2>&1 &
	/my_tools/keeper "start:anlslicense:/etc/init.d/license.sh start"
}

license_stop()
{
	/my_tools/keeper "stop:anlslicense"
	killall anlslicense
}

case "$1" in
	start)
		license_start
		;;
	stop)
		license_stop
		;;
	restart)
		license_stop
		license_start
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 2
		;;
esac

exit 0
