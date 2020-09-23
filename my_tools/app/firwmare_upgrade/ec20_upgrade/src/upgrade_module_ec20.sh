#!/bin/sh

BSP_CLI=/my_tools/bsp_cli
PROCESS=/my_tools/module_ec20_upgrade
channel=$1
filename=$2
close_asterisk(){
        /etc/init.d/asterisk stop
}

power_check(){
        state_off="power  OFF"
        result=$( $BSP_CLI module_state power $channel )
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
        /my_tools/bsp_cli upgrade sel -1
        sleep 1
        /my_tools/bsp_cli upgrade sel $channel
        sleep 1
}


upgrade_process(){
        $PROCESS -f $filename -m 0 -c $channel> /www/upgrade_module_$channel.log &
}

upgrade(){
        close_asterisk
        power_check
        turn_check
        select_upgrade_channel
        upgrade_process
}

upgrade
