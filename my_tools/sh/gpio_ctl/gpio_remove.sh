## stop gpio work
#!/bin/sh

gpio_reset_port=105
gpio_run_port=111
gpio_path=/sys/class/gpio

stop_gpio()
{
	echo ${gpio_reset_port} > ${gpio_path}/unexport
	echo ${gpio_run_port} > ${gpio_path}/unexport
}

if [ ! -e /sys/class/gpio/export ]; then
	echo no export file exist!
	exit
fi

stop_gpio

exit 0
