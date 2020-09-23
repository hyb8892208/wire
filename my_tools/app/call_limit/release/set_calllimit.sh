#!/bin/sh

LIMIT_CLI=/my_tools/calllimit_cli
RET_RESULT="error"

cmd=$1
channel=$2
sim_idx=$3

#echo "cmd=$cmd, channel=$channel, sim_idx=$sim_idx"
case $cmd in
	"reload")
	$LIMIT_CLI set chn reload
	;;
	"unlock")
    $LIMIT_CLI set chn unlock $channel
	;;
	"unmark")
	$LIMIT_CLI set chn unmark $channel
	;;
	"unlimited")
    $LIMIT_CLI clean chn limited $channel
    ;;
	"status")
    $LIMIT_CLI show chn status $channel
    ;;
	"filewrite")
    $LIMIT_CLI set filewrite switch $2 
	;;
	"resetcalltime")
	$LIMIT_CLI set chn calltimecnt $channel 0
	;;
	"allstatus")
    $LIMIT_CLI show allstatus
	;;
	"sim_unlimited")
    $LIMIT_CLI set sim unlimit $channel $sim_idx
    ;;
    "sim_unlock")
    $LIMIT_CLI set sim unlock $channel $sim_idx  
    ;;
    "sim_unmark")
    $LIMIT_CLI set sim unmark $channel $sim_idx  
    ;;
	"allsim_unmark")
    $LIMIT_CLI set allsim unmark
    ;;
    "allsim_unlock")
    $LIMIT_CLI set allsim unlock
    ;;	
    "allsim_unlimited")
    $LIMIT_CLI set allsim unlimit
    ;;	
    "allsim_unsmslimited")
    $LIMIT_CLI set allsim unsmslimit
    ;;	
    "sim_resetcalltime")
   	$LIMIT_CLI set sim calltimecnt $channel $sim_idx 0
    ;;
    "sim_setdaysmscnt")
    $LIMIT_CLI set sim daysmscnt $channel $sim_idx 0
    ;;
    "sim_setmonsmscnt")
    $LIMIT_CLI set sim monsmscnt $channel $sim_idx 0
    ;;
	"sim_unsmslimit")  
	$LIMIT_CLI set sim daysmscnt $channel $sim_idx 0                             
    $LIMIT_CLI set sim monsmscnt $channel $sim_idx 0
    ;;
    "sim_getpincode")
    $LIMIT_CLI get sim pincode $channel $sim_idx 
	;;
	"sim_setpincode")
    $LIMIT_CLI set sim pincode $channel $sim_idx $4
    ;;
	"sim_switch")
    $LIMIT_CLI switch chan simcard $channel $sim_idx
    ;;	
	*)
	;;
esac
RET_RESULT=$?
#echo "$RET_RESULT"

