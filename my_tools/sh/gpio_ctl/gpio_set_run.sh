## set gpio run lamp
#!/bin/sh

gpio_type=$1
gpio_run_port=367
gpio_path=/sys/class/gpio

set_sys_run()
{
	run_port=$1
	if [ -d ${gpio_path}/gpio${run_port} ]; then
		echo out > ${gpio_path}/gpio${run_port}/direction
		dir=`cat ${gpio_path}/gpio${run_port}/direction`
		if [ $dir == "out" ]; then
			if [ x"$gpio_type" == x"off" ]; then
				echo 1 > ${gpio_path}/gpio${run_port}/value
			else
				echo 0 > ${gpio_path}/gpio${run_port}/value
			fi
		fi
	fi
}

if [ ! -e /sys/class/gpio/export ]; then
	echo no export file exist!
	exit
fi
if [ ! -d ${gpio_path}/gpio${gpio_run_port} ]; then
	echo ${gpio_run_port} > ${gpio_path}/export
fi

set_sys_run ${gpio_run_port}

exit 0
