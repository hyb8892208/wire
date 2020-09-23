#!/bin/sh
killps()
{
	#kill -9 `pgrep $1 | grep -v $2` > /dev/null 2>&1
	pgrep $1 | while read line;do
		PID=$line
		if [ "$PID" != "$2" ]; then
			kill -9 $PID
		fi
	done
}
