#!/bin/sh

BSP_CLI=/my_tools/bsp_cli
PROCESS=/my_tools/module_sim6320c_upgrade
channle=-1
all_channels=-1
filename=./
mode=0
close_asterisk(){
        /etc/init.d/asterisk stop
}

module_reset(){
	$BSP_CLI module power off $channel
	sleep 1
	$BSP_CLI module power on $channel	
}

power_check(){
        state_off="power  OFF"
        result=$( $BSP_CLI module_state power $1 )
        if [[ "$result" =~ "$state_off" ]];then
                $BSP_CLI module power on $channel
        fi
}

turn_check(){
        state_off="turn  OFF"
        result=$( $BSP_CLI module_state turn $channel )
        echo "$result"
        if [[ "$result" =~ "$state_off" ]];then
                $BSP_CLI module turn on $channel
                sleep 1
        fi
}

select_upgrade_channel(){
        $BSP_CLI upgrade sel $all_channels
        sleep 1
	$BSP_CLI upgrade sel $channel
        sleep 1
}


upgrade_process(){
        $PROCESS -c $channel -m $mode -f $filename> /www/upgrade_module_$channel.log &
# 	 echo "$PROCESS -f $filename -c $channel"
#        $PROCESS -f $filename -c $channel
}

upgrade(){
        close_asterisk
		if [ $mode -eq 1 ];then
			power_check
		else
			module_reset
		fi
        turn_check
        select_upgrade_channel
        upgrade_process
}

if [ $# -lt 2 ];then
	echo "param error"
	echo "usage:$0 channel FirmwareDir"
	exit 1
fi

channel=$1
filename=$2

if [ $# -gt 2 ];then
	mode=$3
fi

upgrade
