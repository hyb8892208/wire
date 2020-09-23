#!/bin/sh
 
# udhcpc script edited by Tim Riker <Tim@Rikers.org>
 
[ -z "$1" ] && echo "Error: should be called from udhcpc" && exit 1

NET_PATH=/etc/asterisk/gw/network
NET_CFG_PATH=/etc/cfg/gw/network
NET_CONFIG=lan.conf
RESOLV_CONF="/etc/resolv.conf"
[ -n "$broadcast" ] && BROADCAST="broadcast $broadcast"
[ -n "$subnet" ] && NETMASK="netmask $subnet"

case "$1" in
  deconfig)
    /sbin/ifconfig $interface 0.0.0.0
    ;;
 
  renew|bound)
    /sbin/ifconfig $interface $ip $BROADCAST $NETMASK
	if [ x"$interface" == x"eth1" ]; then
		NET_CONFIG=wan.conf
	fi
	/my_tools/set_config ${NET_PATH}/${NET_CONFIG} set option_value ipv4 ipaddr $ip
	/my_tools/set_config ${NET_PATH}/${NET_CONFIG} set option_value ipv4 netmask $NETMASK
	/my_tools/set_config ${NET_CFG_PATH}/${NET_CONFIG} set option_value ipv4 ipaddr $ip
	/my_tools/set_config ${NET_CFG_PATH}/${NET_CONFIG} set option_value ipv4 netmask $NETMASK
 
    if [ -n "$router" ] ; then
      echo "deleting routers"
      while route del default gw 0.0.0.0 dev $interface ; do
        :
      done
 
      for i in $router ; do
        route add default gw $i dev $interface
      done
    fi
 
    echo -n > $RESOLV_CONF
    [ -n "$domain" ] && echo search $domain >> $RESOLV_CONF
    for i in $dns ; do
      echo adding dns $i
      echo nameserver $i >> $RESOLV_CONF
    done
    ;;
esac
 
exit 0