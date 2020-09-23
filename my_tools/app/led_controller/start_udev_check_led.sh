#!/bin/bash

udev_script=/etc/udev/rules.d/10_check_led.rules

cp -af /gateway$udev_script $udev_script
udevadm control -R
udevadm trigger --sysname-match=tty[A]*

exit 0

