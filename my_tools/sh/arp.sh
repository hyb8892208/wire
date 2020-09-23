#!/bin/sh
CONFILE=/etc/asterisk/gw/arp.conf

ip=
mac=
#CONFILE=/etc/asterisk/gw_arp.conf

get_value()
{
    SECTION=$1
    KEY=$2
    eval $2=`awk -F '=' '/\['"$SECTION"'\]/{a=1}a==1&&$1~/'"$KEY"'/{print $2;exit}' $CONFILE`
}
                                                                                             
add_arp()
{
	arp -s $ip $mac
}

del_arp()
{
	arp -d $ip
}
                  
main(){
	if ! [ -f $CONFILE ];then
		exit 1
	fi

	SECTION_ARR=`grep -rn "^\[" $CONFILE|awk -F ':' '{print $2}'`
	if [ x"${SECTION_ARR}" = x ]; then
		exit 1
	fi

	for ((i=0;i<${#SECTION_ARR[@]};i++)) do
		for section in ${SECTION_ARR[${i}]};do
			get_value ${section:1:-1} ip
			get_value ${section:1:-1} mac
			add_arp                 
		done                        
	done
}

main

