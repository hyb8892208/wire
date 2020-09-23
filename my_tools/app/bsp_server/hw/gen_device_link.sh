#!/bin/bash

#==========================================================
#功能：根据硬件版本生成STM8,STM32 单片机相应的软链接文件.
#      生成符号链接。这样就可以根据通道(STM32)或底板MCU(STM8)，
#      生成固定文件。即此脚本主要功能是生成固定设备名，并完成
#      固定设备名与实现设备之间的映射关系(创建符号链接)。
#
#      固定设备名格式如下：
#      /dev/opvx/
#      ├── chan_brd
#      │   ├── 0 -> ../../ttyACM0
#      │   ├── 1 -> ../../ttyACM1
#      │   ├── 10 -> ../../ttyACM12
#      │   ├── 11 -> ../../ttyACM13
#      │   ├── 12 -> ../../ttyACM14
#      │   ├── 13 -> ../../ttyACM15
#      │   ├── 14 -> ../../ttyACM16
#      │   ├── 15 -> ../../ttyACM5
#      │   ├── 2 -> ../../ttyACM2
#      │   ├── 3 -> ../../ttyACM4
#      │   ├── 4 -> ../../ttyACM6
#      │   ├── 5 -> ../../ttyACM8
#      │   ├── 6 -> ../../ttyACM10
#      │   ├── 7 -> ../../ttyACM3
#      │   ├── 8 -> ../../ttyACM9
#      │   └── 9 -> ../../ttyACM11
#      ├── emu
#      │   ├── 0 -> ../../ttyUSB4
#      │   ├── 1 -> ../../ttyUSB5
#      │   ├── 2 -> ../../ttyUSB6
#      │   └── 3 -> ../../ttyUSB8
#      ├── lcd
#      │   └── 0 -> ../../ttyACM7
#      └── mod_brd
#          ├── 0 -> ../../ttyUSB2
#          └── 1 -> ../../ttyUSB3

#========================================================

#参数 $1 为模块板MCU设备, 从此设备中获取版本信息
MB_MCU_DEV=

APP_GET_DEV="/my_tools/get_dev"
APP_GET_VER="/my_tools/get_version"
APP_GET_VER_WAIT_TIME_MS=100
APP_GET_DEV_WAIT_TIME_MS=200

HW_VER_1="V1.0"
HW_VER_2="V2.0"
MB_HW_VER="V"

USB_TYPE="SWG1032_BASED"
USB_TYPE_SWG="SWG1032_BASED"
USB_TYPE_VS2_8X="VS2_8XEC20_DOWN"
USB_TYPE_VS_USB="VS_USB_BASED"
DEV_TYPE="2U"

LOG_FILE="/tmp/gen_device_link.log"


DEV_NULL=null

DEV_LINK_DIR=/dev/opvx
USB_BUS_DIR=/sys/bus/usb/devices
DEV_DIR=/dev
single_vs2_8x=0

usb_brd_nodeid_swg20xx=(1-1.2.2.2:1.0		1-1.2.5.2:1.0)
usb_brd_nodeid_swg10xx=(1-1.2.3.2.2:1.0 1-1.2.4.2.2:1.0)
usb_brd_nodeid_2u=(1-1.2.5.1:1.0 \
	1-1.2.6.6.1:1.0 \
	1-1.2.6.5.1:1.0 \
	1-1.2.1.1:1.0 \
	1-1.2.2.1:1.0 \
	1-1.2.6.1.1:1.0 \
	1-1.2.6.2.1:1.0 \
	1-1.2.3.1:1.0 \
	1-1.2.4.1:1.0 \
	1-1.2.6.4.1:1.0 \
	1-1.2.6.3.1:1.0)
usb_brd_nodeid_1u=(1-1.2.5.1:1.0 \
	1-1.2.1.1:1.0 \
	1-1.2.2.1:1.0 \
	1-1.2.3.1:1.0 \
	1-1.2.4.1:1.0)
usb_brd_nodeid_swg2008=(1-1.2.1.1:1.0 1-1.2.2.1:1.0)
usb_brd_nodeid_vs2_8x=(1-1.1.1.6:1.0 \
	1-1.1.1.5.6:1.0 \
	1-1.1.2.6:1.0 \
	1-1.1.2.5.6:1.0 \
	1-1.1.3.6:1.0 \
	1-1.1.3.5.6:1.0 \
	1-1.1.4.6:1.0 \
	1-1.1.4.5.6:1.0)
usb_brd_nodeid_single_vs2_8x=(1-1.1.6:1.0 \
	1-1.1.5.6:1.0)


usb_emu_nodeid_swg20xx=(1-1.2.2.5:1.0 \
	1-1.2.2.7:1.0 \
	1-1.2.5.5:1.0 \
	1-1.2.5.7:1.0)
usb_emu_nodeid_swg10xx=(1-1.2.4.4.2:1.0 \
	1-1.2.4.2.3:1.0 \
	1-1.2.3.4.2:1.0 \
	1-1.2.3.2.3:1.0)

usb_lcd_nodeid_swg20xx=1-1.2.4.4:1.0
usb_lcd_nodeid_swg10xx=1-1.2.3.4.4:1.0

usb_simswitch_nodeid_swg24xx=(1-1.2.4.5:1.0 \
	1-1.2.5.1:1.0)

usb_chan_nodeid_swg20xx=(1-1.2.1.1:1.0 \
	1-1.2.1.2:1.0 \
	1-1.2.1.3:1.0 \
	1-1.2.1.4:1.0 \
	1-1.2.1.5:1.0 \
	1-1.2.1.6:1.0 \
	1-1.2.1.7:1.0 \
	1-1.2.2.3:1.0 \
	1-1.2.5.4.1:1.0 \
	1-1.2.5.4.2:1.0 \
	1-1.2.5.4.3:1.0 \
	1-1.2.5.4.4:1.0 \
	1-1.2.5.4.5:1.0 \
	1-1.2.5.4.6:1.0 \
	1-1.2.5.4.7:1.0 \
	1-1.2.5.3:1.0)
usb_chan_nodeid_swg10xx=(1-1.2.3.1.1.3:1.0 \
	1-1.2.3.1.2.2:1.0 \
	1-1.2.3.2.1.1:1.0 \
	1-1.2.3.2.1.4:1.0 \
	1-1.2.3.1.4.3:1.0 \
	1-1.2.3.1.3.3:1.0 \
	1-1.2.3.3.2:1.0 \
	1-1.2.3.4.1:1.0 \
	1-1.2.4.1.1.3:1.0 \
	1-1.2.4.1.2.2:1.0 \
	1-1.2.4.2.1.1:1.0 \
	1-1.2.4.2.1.4:1.0 \
	1-1.2.4.1.4.3:1.0 \
	1-1.2.4.1.3.3:1.0 \
	1-1.2.4.3.2:1.0 \
	1-1.2.4.4.1:1.0)
usb_chan_nodeid_2u=(1-1.2.5.2:1.0 \
	1-1.2.5.3:1.0 \
	1-1.2.6.6.2:1.0 \
	1-1.2.6.6.3:1.0 \
	1-1.2.6.5.2:1.0 \
	1-1.2.6.5.3:1.0 \
	1-1.2.1.2:1.0 \
	1-1.2.1.3:1.0 \
	1-1.2.2.2:1.0 \
	1-1.2.2.3:1.0 \
	1-1.2.6.1.2:1.0 \
	1-1.2.6.1.3:1.0 \
	1-1.2.6.2.2:1.0 \
	1-1.2.6.2.3:1.0 \
	1-1.2.3.2:1.0 \
	1-1.2.3.3:1.0 \
	1-1.2.4.2:1.0 \
	1-1.2.4.3:1.0 \
	1-1.2.6.4.2:1.0 \
	1-1.2.6.4.3:1.0 \
	1-1.2.6.3.2:1.0 \
	1-1.2.6.3.3:1.0)
usb_chan_nodeid_1u=(1-1.2.5.2:1.0 \
	1-1.2.5.3:1.0 \
	1-1.2.1.2:1.0 \
	1-1.2.1.3:1.0 \
	1-1.2.2.2:1.0 \
	1-1.2.2.3:1.0 \
	1-1.2.3.2:1.0 \
	1-1.2.3.3:1.0 \
	1-1.2.4.2:1.0 \
	1-1.2.4.3:1.0)
usb_chan_nodeid_swg2008=(1-1.2.1.2:1.0 \
	1-1.2.1.3:1.0 \
	1-1.2.2.2:1.0 \
	1-1.2.2.3:1.0)
usb_chan_nodeid_vs2_8x=(1-1.1.1.7:1.0 \
	1-1.1.1.3:1.0 \
	1-1.1.1.2:1.0 \
	1-1.1.1.1:1.0 \
	1-1.1.1.5.7:1.0 \
	1-1.1.1.5.3:1.0 \
	1-1.1.1.5.2:1.0 \
	1-1.1.1.5.1:1.0 \
	1-1.1.2.7:1.0 \
	1-1.1.2.3:1.0 \
	1-1.1.2.2:1.0 \
	1-1.1.2.1:1.0 \
	1-1.1.2.5.7:1.0 \
	1-1.1.2.5.3:1.0 \
	1-1.1.2.5.2:1.0 \
	1-1.1.2.5.1:1.0 \
	1-1.1.3.7:1.0 \
	1-1.1.3.3:1.0 \
	1-1.1.3.2:1.0 \
	1-1.1.3.1:1.0 \
	1-1.1.3.5.7:1.0 \
	1-1.1.3.5.3:1.0 \
	1-1.1.3.5.2:1.0 \
	1-1.1.3.5.1:1.0 \
	1-1.1.4.7:1.0 \
	1-1.1.4.3:1.0 \
	1-1.1.4.2:1.0 \
	1-1.1.4.1:1.0 \
	1-1.1.4.5.7:1.0 \
	1-1.1.4.5.3:1.0 \
	1-1.1.4.5.2:1.0 \
	1-1.1.4.5.1:1.0)
usb_chan_nodeid_single_vs2_8x=(1-1.1.7:1.0 \
	1-1.1.3:1.0 \
	1-1.1.2:1.0 \
	1-1.1.1:1.0 \
	1-1.1.5.7:1.0 \
	1-1.1.5.3:1.0 \
	1-1.1.5.2:1.0 \
	1-1.1.5.1:1.0)


usb_brd_dev_swg20xx=($DEV_NULL $DEV_NULL)
usb_brd_dev_swg10xx=($DEV_NULL $DEV_NULL)
usb_brd_dev_2u=($DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL)
usb_brd_dev_vs2_8x=($DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL \
	$DEV_NULL)


function get_brd_swg20xx_dev()
{
	nodeid_count="${#usb_brd_nodeid_swg20xx[*]}"
	for((i=0;i<$nodeid_count && i<2;i++))
	do
		if [ -d ${USB_BUS_DIR}/${usb_brd_nodeid_swg20xx[$i]} ]; then
			tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_brd_nodeid_swg20xx[$i]} | grep ttyUSB`
			if [ ! -z "$tmp_dev_name" ]; then
				usb_brd_dev_swg20xx[$i]=${DEV_DIR}/${tmp_dev_name}
				if [ -z "$MB_MCU_DEV" ]; then 
					MB_MCU_DEV=${DEV_DIR}/${tmp_dev_name}
					echo "MB_MCU_DEV=$MB_MCU_DEV" >> $LOG_FILE
				fi
			fi
		fi
	done
}

function get_brd_swg10xx_dev()
{
	nodeid_count="${#usb_brd_nodeid_swg10xx[*]}"
	for((i=0;i<$nodeid_count && i<2;i++))
	do
		if [ -d ${USB_BUS_DIR}/${usb_brd_nodeid_swg10xx[$i]} ]; then
			tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_brd_nodeid_swg10xx[$i]} | grep ttyUSB`
			if [ ! -z "$tmp_dev_name" ]; then
				usb_brd_dev_swg10xx[$i]=${DEV_DIR}/${tmp_dev_name}
				if [ -z "$MB_MCU_DEV" ]; then 
					MB_MCU_DEV=${DEV_DIR}/${tmp_dev_name}
					echo "MB_MCU_DEV=$MB_MCU_DEV" >> $LOG_FILE
				fi
			fi
		fi
	done
}

function get_brd_2u_dev()
{
	nodeid_count="${#usb_brd_nodeid_2u[*]}"
	for((i=1;i<$nodeid_count && i<11;i++))
	do
		if [ -d ${USB_BUS_DIR}/${usb_brd_nodeid_2u[$i]} ]; then
			tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_brd_nodeid_2u[$i]} | grep ttyUSB`
			if [ ! -z "$tmp_dev_name" ]; then
				usb_brd_dev_2u[$i]=${DEV_DIR}/${tmp_dev_name}
				if [ -z "$MB_MCU_DEV" ]; then 
					MB_MCU_DEV=${DEV_DIR}/${tmp_dev_name}
					echo "MB_MCU_DEV=$MB_MCU_DEV" >> $LOG_FILE
				fi
			fi
		fi
	done
	if [ -d ${USB_BUS_DIR}/${usb_brd_nodeid_2u[0]} ]; then
		tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_brd_nodeid_2u[0]} | grep ttyUSB`
		if [ ! -z "$tmp_dev_name" ]; then
			usb_brd_dev_2u[0]=${DEV_DIR}/${tmp_dev_name}
			if [ -z "$MB_MCU_DEV" ]; then 
				MB_MCU_DEV=${DEV_DIR}/${tmp_dev_name}
				echo "MB_MCU_DEV=$MB_MCU_DEV" >> $LOG_FILE
			fi
		fi
	fi
}

function get_brd_vs2_8x_dev()
{
	single_vs2_8x=1
	nodeid_count="${#usb_brd_nodeid_vs2_8x[*]}"
	for((i=0;i<$nodeid_count && i<8;i++))
	do
		if [ -d ${USB_BUS_DIR}/${usb_brd_nodeid_vs2_8x[$i]} ]; then
			tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_brd_nodeid_vs2_8x[$i]} | grep ttyUSB`
			if [ ! -z "$tmp_dev_name" ]; then
				single_vs2_8x=0
				usb_brd_dev_vs2_8x[$i]=${DEV_DIR}/${tmp_dev_name}
				if [ -z "$MB_MCU_DEV" ]; then 
					MB_MCU_DEV=${DEV_DIR}/${tmp_dev_name}
					echo "MB_MCU_DEV=$MB_MCU_DEV" >> $LOG_FILE
				fi
			fi
		fi
	done
	
	if [ $single_vs2_8x -eq 1 ]; then
		nodeid_count="${#usb_brd_nodeid_single_vs2_8x[*]}"
		for((i=0;i<$nodeid_count && i<2;i++))
		do
			if [ -d ${USB_BUS_DIR}/${usb_brd_nodeid_single_vs2_8x[$i]} ]; then
				tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_brd_nodeid_single_vs2_8x[$i]} | grep ttyUSB`
				if [ ! -z "$tmp_dev_name" ]; then
					single_vs2_8x=1
					usb_brd_dev_vs2_8x[$i]=${DEV_DIR}/${tmp_dev_name}
					if [ -z "$MB_MCU_DEV" ]; then 
						MB_MCU_DEV=${DEV_DIR}/${tmp_dev_name}
						echo "MB_MCU_DEV=$MB_MCU_DEV" >> $LOG_FILE
					fi
				fi
			fi
		done
	fi
}

function get_brd_all_dev()
{
	ls /dev/ttyUSB* >> $LOG_FILE
	sleep 1
	get_brd_swg20xx_dev
	get_brd_swg10xx_dev
	get_brd_2u_dev
	get_brd_vs2_8x_dev
	if [ -z $MB_MCU_DEV ]; then
		exit 1
	fi
	echo "finish get_brd_all_dev" >> $LOG_FILE
}

function gen_brd_dev_link_from_dev()
{
	brd_dev_arr=($(echo "$@"))
	dev_count="${#brd_dev_arr[*]}"
	for((i=0;i<$dev_count && i<11;i++))
	do
		if [ x"${DEV_NULL}" != x"${brd_dev_arr[$i]}" ]; then
			ln -sf ${brd_dev_arr[$i]} ${DEV_LINK_DIR}/mod_brd/$i
		fi
	done
}
function gen_brd_dev_link_from_nodeid()
{
	usb_brd_nodeid_arr=($(echo "$@"))
	nodeid_count="${#usb_brd_nodeid_arr[*]}"
	for((i=0;i<$nodeid_count && i<11;i++))
	do
		if [ -d ${USB_BUS_DIR}/${usb_brd_nodeid_arr[$i]} ]; then
			tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_brd_nodeid_arr[$i]} | grep ttyUSB`
			if [ ! -z "$tmp_dev_name" ]; then
				ln -sf ${DEV_DIR}/${tmp_dev_name} ${DEV_LINK_DIR}/mod_brd/$i
			fi
		fi
	done
}

function gen_emu_dev_link()
{
	usb_emu_nodeid_arr=($(echo "$@"))
	mkdir -p ${DEV_LINK_DIR}/emu
	nodeid_count="${#usb_emu_nodeid_arr[*]}"
	for((i=0;i<$nodeid_count && i<4;i++))
	do
		if [ -d ${USB_BUS_DIR}/${usb_emu_nodeid_arr[$i]} ]; then
			tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_emu_nodeid_arr[$i]} | grep ttyUSB`
			if [ ! -z "$tmp_dev_name" ]; then
				ln -sf ${DEV_DIR}/${tmp_dev_name} ${DEV_LINK_DIR}/emu/$i
			fi
		fi
	done
}

function gen_lcd_dev_link()
{
	usb_lcd_nodeid=$1
	mkdir -p ${DEV_LINK_DIR}/lcd
	if [ -d ${USB_BUS_DIR}/${usb_lcd_nodeid}/tty ]; then
		tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_lcd_nodeid}/tty | grep ttyACM`
		if [ ! -z "$tmp_dev_name" ]; then
			ln -sf ${DEV_DIR}/${tmp_dev_name} ${DEV_LINK_DIR}/lcd/0
		fi
	fi
}

function gen_simswitch_dev_link()
{
	usb_simswitch_nodeid_arr=($(echo "$@"))
	mkdir -p ${DEV_LINK_DIR}/sim_switch
	nodeid_count="${#usb_simswitch_nodeid_arr[*]}"
	for((i=0;i<$nodeid_count && i<2;i++))
	do
		if [ -d ${USB_BUS_DIR}/${usb_simswitch_nodeid_arr[i]} ]; then
			tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_simswitch_nodeid_arr[i]} | grep ttyUSB`
			if [ ! -z "$tmp_dev_name" ]; then
				ln -sf ${DEV_DIR}/${tmp_dev_name} ${DEV_LINK_DIR}/sim_switch/$i
			fi
		fi
	done
}

function gen_chan_dev_link()
{
	usb_chan_nodeid_arr=($(echo "$@"))
	nodeid_count="${#usb_chan_nodeid_arr[*]}"
	for((i=0;i<$nodeid_count && i<22;i++))
	do
		if [ -d ${USB_BUS_DIR}/${usb_chan_nodeid_arr[$i]}/tty ]; then
			tmp_dev_name=`ls ${USB_BUS_DIR}/${usb_chan_nodeid_arr[$i]}/tty | grep ttyACM`
			if [ ! -z "$tmp_dev_name" ]; then
				ln -sf ${DEV_DIR}/${tmp_dev_name} ${DEV_LINK_DIR}/chan_brd/$i
			fi
		fi
	done
}

function gen_swg20xx_dev_link()
{
	nodeid_arr=$(echo ${usb_brd_dev_swg20xx[*]})
	gen_brd_dev_link_from_dev $nodeid_arr

	nodeid_arr=$(echo ${usb_emu_nodeid_swg20xx[*]})
	gen_emu_dev_link $nodeid_arr

	nodeid_arr=$(echo ${usb_lcd_nodeid_swg20xx[*]})
	gen_lcd_dev_link $nodeid_arr

	nodeid_arr=$(echo ${usb_chan_nodeid_swg20xx[*]})
	gen_chan_dev_link $nodeid_arr

	nodeid_arr=$(echo ${usb_simswitch_nodeid_swg24xx[*]})
	gen_simswitch_dev_link $nodeid_arr

	echo "finish gen_swg20xx_dev_link" >> $LOG_FILE
}

function gen_swg10xx_dev_link()
{
	nodeid_arr=$(echo ${usb_brd_dev_swg10xx[*]})
	gen_brd_dev_link_from_dev $nodeid_arr

	nodeid_arr=$(echo ${usb_emu_nodeid_swg10xx[*]})
	gen_emu_dev_link $nodeid_arr

	nodeid_arr=$(echo ${usb_lcd_nodeid_swg10xx[*]})
	gen_lcd_dev_link $nodeid_arr

	nodeid_arr=$(echo ${usb_chan_nodeid_swg10xx[*]})
	gen_chan_dev_link $nodeid_arr
	echo "finish gen_swg10xx_dev_link" >> $LOG_FILE
}

function gen_2u_dev_link()
{
	nodeid_arr=$(echo ${usb_brd_dev_2u[*]})
	gen_brd_dev_link_from_dev $nodeid_arr

	nodeid_arr=$(echo ${usb_chan_nodeid_2u[*]})
	gen_chan_dev_link $nodeid_arr
	echo "finish gen_2u_dev_link" >> $LOG_FILE
}

function gen_1u_dev_link()
{
	nodeid_arr=$(echo ${usb_brd_nodeid_1u[*]})
	gen_brd_dev_link_from_nodeid $nodeid_arr

	nodeid_arr=$(echo ${usb_chan_nodeid_1u[*]})	
	gen_chan_dev_link $nodeid_arr
	echo "finish gen_1u_dev_link" >> $LOG_FILE
}

function gen_swg2008_dev_link()
{
	nodeid_arr=$(echo ${usb_brd_nodeid_swg2008[*]})
	gen_brd_dev_link_from_nodeid $nodeid_arr

	nodeid_arr=$(echo ${usb_chan_nodeid_swg2008[*]})
	gen_chan_dev_link $nodeid_arr
	echo "finish gen_swg2008_dev_link" >> $LOG_FILE
}

function gen_vs2_8x_dev_link()
{
	nodeid_arr=$(echo ${usb_brd_dev_vs2_8x[*]})
	gen_brd_dev_link_from_dev $nodeid_arr

	nodeid_arr=$(echo ${usb_chan_nodeid_vs2_8x[*]})
	gen_chan_dev_link $nodeid_arr
	echo "finish gen_vs2_8x_dev_link" >> $LOG_FILE
}

function gen_single_vs2_8x_dev_link()
{
	nodeid_arr=$(echo ${usb_brd_dev_single_vs2_8x[*]})
	gen_brd_dev_link_from_dev $nodeid_arr

	nodeid_arr=$(echo ${usb_chan_nodeid_single_vs2_8x[*]})
	gen_chan_dev_link $nodeid_arr
	echo "finish gen_single_vs2_8x_dev_link" >> $LOG_FILE
}

function gen_dev_link()
{
	mb_brd_hw_ver=$1
	mb_brd_hw_type=$2
	mkdir -p ${DEV_LINK_DIR}/chan_brd
	mkdir -p ${DEV_LINK_DIR}/mod_brd

	if [ x"$USB_TYPE_SWG" = x"$mb_brd_hw_type" ]; then
		if [ x"$HW_VER_1" = x"$mb_brd_hw_ver" ]; then
			gen_swg10xx_dev_link
		else
			gen_swg20xx_dev_link
		fi
	elif [ x"$USB_TYPE_VS_USB" = x"$mb_brd_hw_type" -o x"UNDEF_TYPE" = x"$mb_brd_hw_type" ]; then
		if [ x"$DEV_TYPE" = x"1U" ]; then
			gen_1u_dev_link
		elif [ x"$DEV_TYPE" = x"2U" ]; then
			gen_2u_dev_link
		elif [ x"$DEV_TYPE" = x"VS1008" ]; then
			gen_swg2008_dev_link
		else
			echo "$0 : Unkown hardware dev type : $DEV_TYPE" >> $LOG_FILE
			exit 1
		fi
	elif [ x"$USB_TYPE_VS2_8X" = x"$mb_brd_hw_type" ]; then
		if [ $single_vs2_8x -eq 1 ]; then
			gen_single_vs2_8x_dev_link
		else
			gen_vs2_8x_dev_link
		fi
	else
		echo "$0 : Unkown hardware usb type : $mb_brd_hw_type" >> $LOG_FILE
		exit 1
	fi
	echo "finish gen_dev_link" >> $LOG_FILE
}


#获取硬件版本号
#====='APP_GET_VER'输出版本信息格式如下=======
#   SWG1032_BASED
#   HwVer : V2.0
#   SwVer : V2.0
#   Jan  8 2018
#   17:32:28
#=============================================
function get_brd_mcu_key_info()
{
	tmp_file=/tmp/ver.cfg
	$APP_GET_VER $MB_MCU_DEV $APP_GET_VER_WAIT_TIME_MS > $tmp_file
	VS_USB_TYPE=`cat $tmp_file | sed -n "1p"`
	MB_HW_VER=`cat $tmp_file |\
		grep "HwVer" |\
		awk -F":" '{print $2}' |\
		sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//'` #去掉首尾空格

#	echo "VS_USB_TYPE=$VS_USB_TYPE, MB_HW_VER=$MB_HW_VER"
	if [ x"$VS_USB_TYPE" = x"$USB_TYPE_SWG"  -o x"$VS_USB_TYPE" = x"$USB_TYPE_VS_USB" -o x"$VS_USB_TYPE" = x"$USB_TYPE_VS2_8X" ]; then
		USB_TYPE="$VS_USB_TYPE"
	else
		VS_USB_TYPE=`$APP_GET_VER $MB_MCU_DEV $APP_GET_VER_WAIT_TIME_MS 2>&1 | sed -n "2p"`
		if [ x"$VS_USB_TYPE" = x"$USB_TYPE_SWG"  -o x"$VS_USB_TYPE" = x"$USB_TYPE_VS_USB" -o x"$VS_USB_TYPE" = x"$USB_TYPE_VS2_8X" ]; then
			USB_TYPE="$VS_USB_TYPE"
		else
			USB_TYPE="UNDEF_TYPE"
		fi
	fi
	rm $tmp_file
}

#获取设备类型：1U还是2U机箱，
function get_dev_type()
{
	DEV_TYPE=`$APP_GET_DEV $MB_MCU_DEV $APP_GET_DEV_WAIT_TIME_MS 2>&1 |\
		grep "dev_type" |\
		awk -F":" '{print $2}' |\
		sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//'` #去掉首尾空格
}

main()
{
	get_brd_all_dev
	get_dev_type
	sleep 1
	get_brd_mcu_key_info
	
	gen_dev_link $MB_HW_VER $USB_TYPE
}

if [ ! -e $LOG_FILE ]; then                                                                                                                                                 
	    touch $LOG_FILE
fi

#通过/etc/start启动脚本执行，因此对于可插拔设备重新插拔后需要再次调用
rm -rf ${DEV_LINK_DIR}
main $*
exit 0
