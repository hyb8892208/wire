#!/bin/bash

while [ 1 ] 
do
	echo 0 > /sys/class/graphics/fb0/blank
	sleep 30
done
