#!/bin/sh

echo "generate Symbolic links ......"

CFG_NUM=8
UPDATE_NUM=7
CFG_PATH=/etc/cfg
LIB_PATH=/usr/lib
BIN_PATH=/usr/bin
SBIN_PATH=/usr/sbin
MOUNT_PATH=/gateway
CFG_PARTITION=/dev/mmcblk0p$CFG_NUM
UPDATE_PARTITION=/dev/mmcblk0p$PARTITION_NUM

# Mount Gateway Partition
#mount $CFG_PARTITION $CFG_PATH
#mount $UPDATE_PARTITION $MOUNT_PATH

symbol_link()
{
	file=$1
	echo $file
	if [ -f "$file" ]; then
		#echo "$file is a file"
		rm $file -f
	elif [ -d "$file" ]; then
		#echo "$file is a directory"
		rm $file -rf
	elif [ -L "$file" ]; then
		#echo "$file is a symbolic link"
		rm $file -f
	fi
	ln -s $MOUNT_PATH$file $file
}

####################################################
# Symbolic links: include                          #
# configuration file 			           #
# shell script	 			           #
#--------------------------------------------------
# version infomation 
symbol_link /etc/.hostname
symbol_link /version
symbol_link /etc/oem_ver_ctl.conf
symbol_link /etc/motd

# init && start && stop
symbol_link /etc/start
symbol_link /etc/stop
symbol_link /etc/inittab
symbol_link /etc/preinit
symbol_link /etc/remain_cfg_list
#symbol_link /lib/systemd/system/boot-start.service
#ln -s /lib/systemd/system/boot-start.service /etc/systemd/system/multi-user.target.wants

# ssh tools dropbear
symbol_link /etc/passwd   ##login passwd
symbol_link /etc/dropbear
#symbol_link $BIN_PATH/dropbear
#symbol_link $BIN_PATH/dropbearkey

# time config
symbol_link /etc/TZ
symbol_link /etc/localtime
symbol_link /etc/resolv.conf
symbol_link /etc/crontabs

# app config file
symbol_link /etc/ppp
symbol_link $SBIN_PATH/pppoe-status

symbol_link /etc/dahdi
symbol_link /etc/openvox
symbol_link /etc/asterisk

symbol_link /etc/functions.sh
symbol_link /etc/init.d/asterisk
symbol_link /etc/init.d/autoimei
symbol_link /etc/init.d/cron
symbol_link /etc/init.d/dns
symbol_link /etc/init.d/gsm_special_funs
symbol_link /etc/init.d/krtpd
symbol_link /etc/init.d/lan
symbol_link /etc/init.d/logfile_monitor
symbol_link /etc/init.d/ssh
symbol_link /etc/init.d/time
symbol_link /etc/init.d/usbserial
symbol_link /etc/init.d/wan
symbol_link /etc/init.d/simemu.sh
symbol_link /etc/init.d/OPlink
symbol_link /etc/init.d/sim_query.sh
symbol_link /etc/init.d/callEventHdl
symbol_link /etc/init.d/mcuhdlsvr
symbol_link /etc/init.d/led
symbol_link /etc/init.d/vpn
symbol_link /etc/init.d/ipsec
symbol_link /etc/init.d/async_sms.sh
symbol_link /etc/init.d/async_ussd.sh
symbol_link /etc/init.d/handle_mms.sh
symbol_link /etc/init.d/callmonitor.sh
symbol_link /etc/init.d/call_limit.sh
symbol_link /etc/init.d/checkAutoUpdatePublicIP
symbol_link /etc/init.d/smsreports
symbol_link /etc/init.d/ussdresults 
symbol_link /etc/init.d/license.sh
symbol_link /sbin/hotplug-call
symbol_link /usr/share/udhcpc/default.script



####################################################

####################################################
# Symbolic links: include                          #
# binary file 				           #
# library file	 			           #
#  source library files are placed in the gateway  # 
#  partition,initfs contains only symbolic files.  #
#--------------------------------------------------
# asterisk
symbol_link $BIN_PATH/asterisk
symbol_link $LIB_PATH/asterisk

# web and php
symbol_link /www
symbol_link /usr/www
symbol_link /lib/php.ini
symbol_link /bin/php
symbol_link /bin/php-cgi
symbol_link /webservice

# DDNS
symbol_link $BIN_PATH/phddns
symbol_link $BIN_PATH/inadyn
symbol_link /etc/init.d/ddns
symbol_link /etc/inadyn.conf
symbol_link /etc/phlinux.conf

# lighttpd
symbol_link /etc/ssl
symbol_link /etc/lighttpd.conf
symbol_link /etc/lighttpd.ori.conf
symbol_link /etc/lighttpdpassword.ori
symbol_link /etc/init.d/lighttpd
symbol_link $SBIN_PATH/lighttpd
symbol_link $LIB_PATH/lighttpd
symbol_link $LIB_PATH/libpcre.so.1
symbol_link $LIB_PATH/libfcgi.so
symbol_link $LIB_PATH/libfcgi.so.0
symbol_link $LIB_PATH/libfcgi++.so
symbol_link $LIB_PATH/libfcgi++.so.0
symbol_link $LIB_PATH/libcgicc.so
symbol_link $LIB_PATH/libcgicc.so.5

# astmanproxy
symbol_link /etc/init.d/astmanproxy
symbol_link $LIB_PATH/astmanproxy
symbol_link $BIN_PATH/astmanproxy

# link library
symbol_link $LIB_PATH/libgsmat.so
symbol_link $LIB_PATH/libgsmat.so.2.0.9
symbol_link $LIB_PATH/libiconv.so
symbol_link $LIB_PATH/libiconv.so.2
symbol_link $LIB_PATH/libcrypto.so
symbol_link $LIB_PATH/libtonezone.so.2.0
symbol_link $LIB_PATH/libsqlite3.so.0
symbol_link $LIB_PATH/libhiredis.so.0.11
symbol_link $LIB_PATH/libhiredis.so.0.13
symbol_link $LIB_PATH/libcurl.so
symbol_link $LIB_PATH/libcurl.so.4
symbol_link $LIB_PATH/libxml2.so
symbol_link $LIB_PATH/libxml2.so.2
symbol_link $LIB_PATH/libasound.so
symbol_link $LIB_PATH/libasound.so.2
symbol_link $LIB_PATH/libmcuhdl.so
symbol_link $LIB_PATH/libzmq.so.5.0.0
symbol_link $LIB_PATH/libzmq.so
symbol_link $LIB_PATH/libzmq.so.5
symbol_link $LIB_PATH/libczmq.so.3.0.0
symbol_link $LIB_PATH/libczmq.so
symbol_link $LIB_PATH/libczmq.so.3
symbol_link $LIB_PATH/libbsp_api.so
symbol_link $LIB_PATH/librri_api.so
symbol_link $LIB_PATH/i386-linux-gnu/libpcap.so.0.8
symbol_link $LIB_PATH/libapr-1.so.0.7.0
symbol_link $LIB_PATH/libapr-1.so.0
symbol_link $LIB_PATH/libapr-1.so
symbol_link $LIB_PATH/libactivemq-cpp.so.19.0.5
symbol_link $LIB_PATH/libactivemq-cpp.so.19
symbol_link $LIB_PATH/libactivemq-cpp.so
symbol_link $LIB_PATH/libdaemon.so.0.5.0
symbol_link $LIB_PATH/libdaemon.so.0
symbol_link $LIB_PATH/libdaemon.so
symbol_link $LIB_PATH/libexpat.so.1.6.11
symbol_link $LIB_PATH/libexpat.so.1
symbol_link $LIB_PATH/libexpat.so

#avahi
symbol_link $LIB_PATH/libavahi-common.so.3
symbol_link $LIB_PATH/libavahi-core.so.7
symbol_link $LIB_PATH/libavahi-client.so.3
symbol_link $LIB_PATH/libdbus-1.so.3
symbol_link $LIB_PATH/libdbus-1.so.3.27.0
symbol_link /etc/avahi
symbol_link $SBIN_PATH/avahi-daemon
symbol_link $SBIN_PATH/avahi-autoipd
symbol_link $BIN_PATH/avahi-publish

#dbus
symbol_link $BIN_PATH/dbus-daemon
symbol_link /etc/dbus-1/system.conf
symbol_link /etc/dbus-1/system.d/avahi-dbus.conf

# ko
symbol_link /lib/modules/dahdi_timer.ko

# alsa
symbol_link /usr/share/alsa

# my_tools
symbol_link /my_tools
symbol_link $BIN_PATH/auto_update
symbol_link $BIN_PATH/lua
symbol_link /etc/redis.conf
symbol_link /usr/share/zoneinfo
symbol_link /sound

# shutdown_sh
symbol_link /sbin/reboot
symbol_link /sbin/halt
symbol_link /sbin/poweroff
symbol_link /sbin/shutdown

# cloud
symbol_link $BIN_PATH/libubox
symbol_link $BIN_PATH/jshn
symbol_link /sbin/uci
symbol_link $BIN_PATH/ubus
symbol_link $BIN_PATH/ubusd
symbol_link $BIN_PATH/autossh
symbol_link $BIN_PATH/sshpass    
symbol_link $BIN_PATH/ssh    

symbol_link $BIN_PATH/remotemonitor

symbol_link $SBIN_PATH/cloudMain
symbol_link $SBIN_PATH/cloudNat
symbol_link $SBIN_PATH/cloudExternal

symbol_link $LIB_PATH/libjson-c.so.3
symbol_link $LIB_PATH/libubox.so
symbol_link $LIB_PATH/libuci.so
symbol_link $LIB_PATH/libubus.so
symbol_link $LIB_PATH/libmicroxml.so.1
symbol_link $LIB_PATH/libexpect5.45.so
symbol_link $LIB_PATH/libblobmsg_json.so
symbol_link $LIB_PATH/libcryptopp.so.6.0
symbol_link $LIB_PATH/libjsoncpp.so
symbol_link $LIB_PATH/libgmp.so.10


symbol_link $BIN_PATH/cloud
symbol_link /etc/init.d/cloud
symbol_link /etc/init.d/remoteweb
symbol_link /etc/init.d/remotessh
symbol_link /etc/config
symbol_link /etc/led
symbol_link /etc/syslog
symbol_link /etc/init.d/sysklog
symbol_link /etc/init.d/bsp_server.sh
symbol_link /etc/init.d/rri_server.sh

# network
symbol_link $BIN_PATH/chat

# pptp-vpn
symbol_link /usr/sbin/pppd
symbol_link /usr/sbin/pptp

#openvpn
symbol_link /usr/sbin/openvpn
symbol_link /etc/openvpn

symbol_link $SBIN_PATH/edge

# sim_query
symbol_link /etc/sim_query.conf

#tls
symbol_link $BIN_PATH/expect
symbol_link /usr/sbin/ast_tls_cert
symbol_link $LIB_PATH/tcl8.4
symbol_link $LIB_PATH/libtcl8.4.so

# xl2tp
symbol_link /usr/sbin/xl2tpd
symbol_link /usr/sbin/xl2tpd-control
symbol_link /usr/libexec
symbol_link /usr/sbin/ipsec
symbol_link /usr/sbin/setkey
symbol_link /usr/lib/ipsec
symbol_link /etc/ipsec
symbol_link /etc/xl2tpd

#sqlite3
symbol_link /usr/bin/sqlite3

#Quectel-CM
symbol_link /etc/udhcpc
