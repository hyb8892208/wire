#!/bin/bash

channel=16
PROC_NAME=./bsp_cli
#PROC_NAME=./release/btool
sim_card_enable_and_status_test(){
    for i in $( seq 1 $channel )
    do
        ${PROC_NAME} sim enable $i
        ${PROC_NAME} sim_state enable $i
    done
}

sim_card_disable_and_status_test(){
    for i in $( seq 1 $channel )
    do
        ${PROC_NAME} sim disable $i
#        ./bsp_cli sim_state disable $i
    done
}

sim_card_insert_status(){
    for i in $( seq 1 $channel )
    do
        ${PROC_NAME} sim_state insert $i
    done
}

sim_card_event(){
    for i in $( seq 1 $channel )
    do
        ${PROC_NAME} sim event $i
    done
}


module_num_get_test(){
    ${PROC_NAME} module num
}

module_uid_get_test(){
    ${PROC_NAME} module uid 1
}

module_reset_key_test(){
    ${PROC_NAME} module reset 1
}

module_turn_on_test(){
    for i in $( seq 1 $channel )
    do
        ${PROC_NAME} module turn on $i
        ${PROC_NAME} module_state turn $i
    done
}

module_turn_off_test(){
    for i in $( seq 1 $channel )
    do
        ${PROC_NAME} module turn off $i
        ${PROC_NAME} module_state turn $i
    done
}

module_power_on_test(){
    for i in $( seq 1 $channel )
    do
        ${PROC_NAME} module power on $i
        ${PROC_NAME} module_state power $i
    done
}

module_power_off_test(){
    for i in $( seq 1 $channel )
    do
        ${PROC_NAME} module power off $i
        ${PROC_NAME} module_state power $i
    done
}

bmcu_ver_test(){
    ${PROC_NAME} bmcu ver
}

bmcu_info_test(){
    ${PROC_NAME} bmcu info
}


bmcu_read_reg_test(){
    ${PROC_NAME} bmcu reg read 1 
}

bmcu_write_reg_test(){
    ${PROC_NAME} bmcu reg write 1
}

upgrade_select_and_status_test(){
    for i in $( seq 1 $channel )
    do 
        ${PROC_NAME} upgrade sel $i
        ${PROC_NAME} upgrade state $i
    done

    ${PROC_NAME} upgrade sel 0xFFFF
}

upgrade_test(){
    upgrade_select_and_status_test
}





sim_card_test(){   
    sim_card_enable_and_status_test
    sim_card_disable_and_status_test
    sim_card_enable_and_status_test
    sim_card_insert_status
    sim_card_event
}

module_test(){
    
    module_num_get_test
    module_uid_get_test
    module_reset_key_test
    module_power_off_test
    module_power_on_test
    module_turn_off_test
    module_turn_on_test
}

bmcu_test(){
    bmcu_ver_test
    bmcu_info_test
#    bmcu_read_reg_test
#    bmcu_write_reg_test
}

bsp_cli_test(){
    sim_card_test
    module_test
    bmcu_test
    upgrade_test
}

while [ true ]
do
    bsp_cli_test
    sleep 10
done

