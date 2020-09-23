#!/bin/sh

vpn_server=$1

if [ -z $vpn_server ]; then
	echo "vpn server ip is NULL."
	exit 0
fi

ping -q -c1 $vpn_server > /dev/null
if [ $? -eq 0 ]; then
	echo "VPN is OK!"
else
	/etc/init.d/vpn restart
fi
