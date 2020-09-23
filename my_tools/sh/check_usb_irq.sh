#!/bin/sh

usb_irq_id=$(cat /proc/interrupts |grep usb1|awk -F ':' '{sub(/^[[:blank:]]*/,"",$1);print $1}')
echo ${usb_irq_id}
