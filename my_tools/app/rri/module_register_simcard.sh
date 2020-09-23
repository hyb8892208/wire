#!/bin/bash
AT='AT'
CSQ='AT+CSQ'
ATH='ATH0'
ATE='ATE0'
CMEE='AT+CMEE=2'
CGMM='AT+CGMM'
CGMI='AT+CGMI'
CGMR='AT+CGMR'
CGSN='AT+CGSN'
CPIN='AT+CPIN?'
CIMI='AT+CIMI'
CLIP='AT+CLIP=1'
CLIR='AT+CLIR=2'
CCWA='AT+CCWA=1'
CREG='AT+CREG=1'
CREG1='AT+CREG?'
CNMI='AT+CNMI=2,2,0,0,0'
ATX='ATX4'
CSCS='AT+CSCS="GSM"'
CSCA='AT+CSCA?'
COPS='AT+COPS=3,0'
ATW='AT&W'
COPS1='AT+COPS?'
QINDCFG1='AT+QINDCFG="csq",1'
QINDCFG2='AT+QINDCFG="CCINFO",1,1'
QINDCFG3='AT+QURCCFG="urcport","uart1"'


channel=15
try_count=10


BSP_CLI=/my_tools/bsp_cli

if [ $# ==  1 ]
then
    channel=$1
else
    channel=15
fi

module_reset(){
	state=1
	state_on="module[$channel] is turn  ON"
	state_off="module[$channel] is turn  OFF"

#if module is turn on, will reset
	result=$( $BSP_CLI module_state turn ${channel} )
	if [[ "$result" =~ "$state_on" ]];then
	    result=$( $BSP_CLI module turn off ${channel} )
	    if [[ "$result" =~ "trun off ok" ]]
	    then
	    	echo $result
	    else
	    	echo -e $result
	    	exit 1
    	fi
	
	    while [ $state == 1 ]
	    do
		    result=$( $BSP_CLI module_state turn ${channel} )
		    if [[ "$result" =~ "$state_off" ]];then
			    state=0
		    	break
	    	fi
		    sleep 2
	    done
	fi
	
	result=$( $BSP_CLI module turn on ${channel} )
	if [[ "$result" =~ "trun on ok" ]]
	then
		echo $result
	else
		echo -e $result
		exit 1
	fi
	
	state=1
	while [ $state == 1 ]
	do
		result=$( $BSP_CLI module_state turn ${channel} )
		if [[ "$result" =~ "$state_on" ]]
		then
			state=0
			break
		fi
		sleep 2
	done
	echo -e "module reset ok"
}

#result is equal
equal_at_command_run(){
	result=$(/my_tools/rri_cli at ${channel} $1 )
	if [[ $result =~ $2 ]]
	then
		echo -e "$1:\\n$result\n"
		return 0
	else
		echo -e "$1:\\n$result\n"
		return 1
	fi
	return 0
}

#result is not equal
no_equal_at_command_run(){
	result=$(/my_tools/rri_cli at ${channel} $1 )
	if [[ $result =~ *$2* ]]
	then
		echo  -e "$1:\n$result\n"
		return 1
	else
		echo -e "$1:\n$result\n"
		return 0
	fi
	return 0
}

#result is not equal, will try again
no_equal_at_command_run_again(){
	for j in $( seq 1 $try_count )
	do
		equal_at_command_run $1 $2
		result=$?
		if [ $result == 0 ]
		then
			return 0
		fi
		echo "register network failed, try again"
		sleep 1
	done
	exit 1
}

#result is equal, will try again
equal_at_command_run_again(){
	for j in $( seq 1 $try_count )
	do
		no_equal_at_command_run $1 $2
		result=$?
		if [ $result == 0 ]
		then
			return 0
		fi
		echo "register network failed, try again"
		sleep 1
	done
	exit 1
}

register_network_sleep(){
	sleep 1
}
register_network(){
	no_equal_at_command_run_again $AT OK
	register_network_sleep

	no_equal_at_command_run_again $ATH OK
	register_network_sleep

	no_equal_at_command_run_again $ATE OK
	register_network_sleep

	no_equal_at_command_run_again $CMEE OK
	register_network_sleep

	no_equal_at_command_run_again $CGMM OK
	register_network_sleep

	no_equal_at_command_run_again $CGMI OK
	register_network_sleep

	no_equal_at_command_run_again $CGMR OK
	register_network_sleep

	no_equal_at_command_run_again $CGSN OK
	register_network_sleep

	no_equal_at_command_run_again $CPIN '\+CPIN: READY'
	register_network_sleep

	no_equal_at_command_run_again $CIMI OK
	register_network_sleep

	no_equal_at_command_run_again $CLIP OK
	register_network_sleep

	no_equal_at_command_run_again $CLIR OK
	register_network_sleep

	no_equal_at_command_run_again $CCWA OK
	register_network_sleep

	no_equal_at_command_run_again $CREG OK
	register_network_sleep

	no_equal_at_command_run_again $CREG1 '\+CREG: 1,1'
	register_network_sleep

	no_equal_at_command_run_again $CNMI OK
	register_network_sleep

	no_equal_at_command_run_again $ATX OK
	register_network_sleep

	no_equal_at_command_run_again $CSCS OK
	register_network_sleep

	no_equal_at_command_run_again $CSCA OK
	register_network_sleep

	no_equal_at_command_run_again $COPS OK
	register_network_sleep

	no_equal_at_command_run_again $ATW OK
	register_network_sleep

	no_equal_at_command_run_again $QINDCFG1 OK
	register_network_sleep

	no_equal_at_command_run_again $QINDCFG2 OK
	register_network_sleep

	no_equal_at_command_run_again $QINDCFG3 OK
	register_network_sleep

	equal_at_command_run_again $CSQ 'CSQ: 99,99'
}

module_reset

register_network

echo "register network success!"

