#!/bin/sh
total_channel=0
channel_type=unkown
SELECT_QDAI='AT+QDAI?'
QDAI_RESULT='1,0,0,1,0,0,1,1'
QDAI_CMD='AT+QDAI=1,0,0,1,0,0,1,1'
ATE_CMD='ATE0'
RESET_CMD='AT+CFUN=1,1'
SUCCESS='OK'
equal_at_command_run(){
	result=$(/my_tools/rri_cli at ${channels} $1 )
	if [[ $result =~ $2 ]]
	then
		#echo -e "$1:\\n$result\n"
		return 0
	else
		#echo -e "$1:\\n$result\n"
		return 1
	fi
	return 0
}

get_value(){
    SECTION=$1
    KEY=$2
    CONFILE=/tmp/hw_info.cfg
    eval $3=`awk -F '=' '/^\['"$SECTION"'\]/{a=1}a==1&&$1~/'"$KEY"'/{print $2;exit}' $CONFILE`
}

get_total_channel(){
	get_value sys total_chan_count total_channel
}


set_qdai(){
	get_total_channel
	for((channels=1;channels<=$total_channel;channels++));do
		get_value channel chan_${channels}_type channel_type
		if [ $channel_type != "EC20CE" ];then
			continue
		fi
		equal_at_command_run $ATE_CMD $SUCCESS
		equal_at_command_run $SELECT_QDAI $QDAI_RESULT
		if [ $? -eq 1 ];then
			equal_at_command_run $QDAI_CMD $SUCCESS
			if [ $? -eq 0 ];then
				equal_at_command_run $RESET_CMD $SUCCESS
				echo "Set chan[$channels] $QDAI_CMD success!"
			else
				echo "Set chan[$channels] $QDAI_CMD failed!"
			fi
		else
			echo "chan[$channels] $QDAI_CMD, normal exit"
		fi
	done
}

set_qdai



