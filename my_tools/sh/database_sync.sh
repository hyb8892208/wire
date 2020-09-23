#!/bin/sh

sync()
{
	php -r "include_once(\"/www/cgi-bin/inc/smsinboxdb.php\");\$db=new SMSINBOXDB();\$db->sync_cache();"
	php -r "include_once(\"/www/cgi-bin/inc/smsoutboxdb.php\");\$db=new SMSOUTBOXDB();\$db->sync_cache();"
}

check_sync()
{
	PROCESS="sync_cache"
	pid=`ps -ef |grep "$PROCESS" |grep -v 'grep' | awk -F ' ' '{print $1}'`
	while true
	do
		if [ -z "$pid" ]; then
			break;
		fi
		sleep 1
		pid=`ps -ef |grep "$PROCESS" |grep -v 'grep' | awk -F ' ' '{print $1}'`
	done
}

## 没参数就代表守护进程，有参数
if [ $# -ne 0 ]; then
	echo "start to sync..."
	sync
	echo "sync complete"
	exit 0
fi

while true
do
	sync
	sleep 30
	check_sync
done

