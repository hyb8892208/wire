#!/bin/sh

gpio_reset_port=361
gpio_path=/sys/class/gpio
low_level_count=0
reset_cmd=/my_tools/restore_cfg_file
syslog_cmd=/my_tools/add_syslog


if [ ! -e /sys/class/gpio/export ]; then
	echo no export file exist!
	exit
fi

echo ${gpio_reset_port} > ${gpio_path}/export

if [ -d ${gpio_path}/gpio${gpio_reset_port} ]; then
	while :
	do
		dir=`cat ${gpio_path}/gpio${gpio_reset_port}/direction`
		if [ $dir == "in" ]; then
			val=`cat ${gpio_path}/gpio${gpio_reset_port}/value`
			if [ $val == "0" ]; then
				low_level_count=$(($low_level_count+1))
			else
				low_level_count=0
			fi
		else
			low_level_count=0
		fi
		if [ $low_level_count -ge 5 ]; then
			$syslog_cmd "Factory reset from RST button [gpio]."
			$reset_cmd > /dev/null 2>&1 &
			echo "Factory reset from RST button [gpio]."
			break
		fi
		sleep 1
	done
fi

exit 0
