#!/bin/sh

RRI_CLI=/my_tools/rri_cli
PROCESS=/my_tools/iapupgrade
RESET_DEV=/my_tools/gen_device_link.sh
PRE_DEV_NAME=/dev/opvx/chan_brd
RET_RESULT="error"

mcu_id=$1
filename=$2
channel=$((mcu_id*2-1))

#echo "mcu_id=$mcu_id, channel=$channel."

close_asterisk(){
	/etc/init.d/asterisk stop > /dev/null 2>&1
}

switch_upgrade(){
	$RRI_CLI module upgrade $channel > /dev/null 2>&1
	sleep 15
}

gen_dev() {
	$RESET_DEV > /dev/null 2>&1
	sleep 10
}

reopen_dev() {
	$RRI_CLI chn com reopen $channel > /dev/null 2>&1 &
}

upgrade_process(){
	dev_id=$((mcu_id-1))
	dev_id_name="${PRE_DEV_NAME}/${dev_id}"

#	echo "$PROCESS -p $dev_id_name -i $filename"	
	$PROCESS -p $dev_id_name -i $filename > /www/upgrade_module_mcu_${mcu_id}.log 
	sleep 2
}

calc_percent() {
#	if [ `grep "Transfert complete" /www/upgrade_module_mcu_${mcu_id}.log > /dev/null 2>&1` ]; then
	if ( cat /www/upgrade_module_mcu_${mcu_id}.log 2> /dev/null | grep "Transfert complete" > /dev/null 2>&1 ) ; then
		RET_RESULT="success"
	fi 
}

close_bsp(){
	/etc/init.d/bsp_server.sh stop
}

upgrade(){
	close_asterisk
	close_bsp
	switch_upgrade
	gen_dev
	upgrade_process
	calc_percent
#	gen_dev
	reopen_dev
}

upgrade

echo "$RET_RESULT"
