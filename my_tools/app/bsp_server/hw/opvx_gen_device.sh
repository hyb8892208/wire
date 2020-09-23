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

#参数 $1 为模块板MUC设备, 从此设备中获取版本信息
MB_MCU_DEV=

APP_GET_DEV="/my_tools/get_dev"
APP_GET_VER="/my_tools/get_version"
APP_GET_VER_WAIT_TIME_MS=100
APP_GET_DEV_WAIT_TIME_MS=200
UDEV_RULE_FILE="/etc/udev/rules.d/20_opvx.rules"

HW_VER_1="V1.0"
HW_VER_2="V2.0"
HW_VER_TITLE="HW version"
MB_HW_VER="V"

USB_TYPE="SWG1032_BASED"
USB_TYPE_SWG="SWG1032_BASED"
USB_TYPE_VS_M35="VS_USB_BASED"
DEV_TYPE="2U"
DEV_TYPE_TITLE="HW DevType"
SLOT_ID=1

LOG_FILE="/tmp/opvx_gen_device.log"

#udev生成的所有设备在/dev/$OPVX_DEV_ROOT目录下
OPVX_DEV_ROOT=opvx

#通道板设备目录/dev/$OPVX_DEV_ROOT/$CHAN_BRD_DIR
CHAN_BRD_DIR=chan_brd

#Debug通道目录/dev/$OPVX_DEV_ROOT/$DEBUG_DIR
DEBUG_DIR=debug

#模块板MCU /dev/$OPVX_DEV_ROOT/$MOD_BOARD_DIR
MOD_BOARD_DIR=mod_brd

#SWG设备lcd 目录 /dev/$OPVX_DEV_ROOT/$MOD_BOARD_DIR
LCD_DIR=lcd

#SWG设备lcd 目录 /dev/$OPVX_DEV_ROOT/$EMU_DIR
EMU_DIR=emu


# $1 - 指定生成的udev规则文件 
# $2 - 模块板硬件版本信息 
function generate_udev_rules_hw_ver_swg_10xx()
{
	mkdir -p /dev/opvx/emu
	mkdir -p /dev/opvx/lcd
#USB symbol links
	local USB_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.4/1-1.2.4.4.2/1-1.2.4.4.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2.3/1-1.2.4.2.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.4/1-1.2.3.4.2/1-1.2.3.4.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2.3/1-1.2.3.2.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2.2/1-1.2.3.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2.2/1-1.2.4.2.2:1.0)

#ACM symbol links
	local ACM_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.1/1-1.2.3.1.1/1-1.2.3.1.1.3/1-1.2.3.1.1.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.1/1-1.2.3.1.2/1-1.2.3.1.2.2/1-1.2.3.1.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2.1/1-1.2.3.2.1.1/1-1.2.3.2.1.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2.1/1-1.2.3.2.1.4/1-1.2.3.2.1.4:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.1/1-1.2.3.1.4/1-1.2.3.1.4.3/1-1.2.3.1.4.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.1/1-1.2.3.1.3/1-1.2.3.1.3.3/1-1.2.3.1.3.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.3/1-1.2.3.3.2/1-1.2.3.3.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.4/1-1.2.3.4.1/1-1.2.3.4.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.1/1-1.2.4.1.1/1-1.2.4.1.1.3/1-1.2.4.1.1.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.1/1-1.2.4.1.2/1-1.2.4.1.2.2/1-1.2.4.1.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2.1/1-1.2.4.2.1.1/1-1.2.4.2.1.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2.1/1-1.2.4.2.1.4/1-1.2.4.2.1.4:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.1/1-1.2.4.1.4/1-1.2.4.1.4.3/1-1.2.4.1.4.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.1/1-1.2.4.1.3/1-1.2.4.1.3.3/1-1.2.4.1.3.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.3/1-1.2.4.3.2/1-1.2.4.3.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.4/1-1.2.4.4.1/1-1.2.4.4.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.4/1-1.2.3.4.4/1-1.2.3.4.4:1.0)

	for dev in `ls /dev/ttyACM*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<17;i++))
		do
			if [[ $path =~ ${ACM_dev_path[$i]} ]] ; then
				if [ x"$i" != x"16" ]; then
					echo "find $i channle $path"
					ln -sf $dev /dev/opvx/chan_brd/$i
				else
					echo "find led $path"
					ln -sf $dev /dev/opvx/lcd/0
				fi
				break
			fi
		done
	done
	
	for dev in `ls /dev/ttyUSB*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<6;i++))
		do
			if [[ $path =~ ${USB_dev_path[$i]} ]] ; then
				if [ $i -lt 4  ]; then
					echo "find $i EMU $path"
					ln -sf $dev /dev/opvx/emu/$i
				else
					index=$[$i-4]  
					echo "find $index board $path"
					ln -sf $dev /dev/opvx/mod_brd/$index
				fi
				break
			fi
		done
	done
}

# $1 - 指定生成的udev规则文件 
# $2 - 模块板硬件版本信息 
function generate_udev_rules_hw_ver_swg_20xx()
{
	mkdir -p /dev/opvx/emu
	mkdir -p /dev/opvx/lcd
#USB symbol links
	local USB_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.5/1-1.2.2.5:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.7/1-1.2.2.7:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.5/1-1.2.5.5:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.7/1-1.2.5.7:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.2/1-1.2.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.2/1-1.2.5.2:1.0)

#ACM symbol links
	local ACM_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.1/1-1.2.1.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.4/1-1.2.1.4:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.5/1-1.2.1.5:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.6/1-1.2.1.6:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.7/1-1.2.1.7:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.1/1-1.2.5.4.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.2/1-1.2.5.4.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.3/1-1.2.5.4.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.4/1-1.2.5.4.4:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.5/1-1.2.5.4.5:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.6/1-1.2.5.4.6:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.7/1-1.2.5.4.7:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.3/1-1.2.5.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.4/1-1.2.4.4:1.0)

	for dev in `ls /dev/ttyACM*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<17;i++))
		do
			if [[ $path =~ ${ACM_dev_path[$i]} ]] ; then
				if [ x"$i" != x"16" ]; then
					echo "find $i channle $path"
					ln -sf $dev /dev/opvx/chan_brd/$i
				else
					echo "find led $path"
					ln -sf $dev /dev/opvx/lcd/0
				fi
				break
			fi
		done
	done
	
	for dev in `ls /dev/ttyUSB*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<6;i++))
		do
			if [[ $path =~ ${USB_dev_path[$i]} ]] ; then
				if [ $i -lt 4  ]; then
					echo "find $i EMU $path"
					ln -sf $dev /dev/opvx/emu/$i
				else
					index=$[$i-4]  
					echo "find $index board $path"
					ln -sf $dev /dev/opvx/mod_brd/$index
				fi
				break
			fi
		done
	done
}


function generate_udev_rules_hw_VS_M35_1U()
{
	local ACM_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.2/1-1.2.5.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.3/1-1.2.5.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.2/1-1.2.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.3/1-1.2.3.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.3/1-1.2.4.3:1.0)

   local USB_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.1/1-1.2.5.1:1.0 \
 /devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.1/1-1.2.1.1:1.0 \
 /devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.1/1-1.2.2.1:1.0 \
 /devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.1/1-1.2.3.1:1.0 \
 /devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.1/1-1.2.4.1:1.0)

	
	for dev in `ls /dev/ttyACM*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<10;i++))
		do
			if [[ $path =~ ${ACM_dev_path[$i]} ]] ; then
				echo "find $i channle $path"
				ln -sf $dev /dev/opvx/chan_brd/$i
				break
			fi
		done
	done
	
	for dev in `ls /dev/ttyUSB*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<5;i++))
		do
			if [[ $path =~ ${USB_dev_path[$i]} ]] ; then
				echo "find $i board $path"	
				ln -sf $dev /dev/opvx/mod_brd/$i
				break
			fi
		done
	done
}

function generate_udev_rules_hw_VS_M35_2U()
{
	local ACM_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.2/1-1.2.5.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.3/1-1.2.5.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.6/1-1.2.6.6.2/1-1.2.6.6.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.6/1-1.2.6.6.3/1-1.2.6.6.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.5/1-1.2.6.5.2/1-1.2.6.5.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.5/1-1.2.6.5.3/1-1.2.6.5.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.2/1-1.2.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.1/1-1.2.6.1.2/1-1.2.6.1.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.1/1-1.2.6.1.3/1-1.2.6.1.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.2/1-1.2.6.2.2/1-1.2.6.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.2/1-1.2.6.2.3/1-1.2.6.2.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.3/1-1.2.3.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.3/1-1.2.4.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.4/1-1.2.6.4.2/1-1.2.6.4.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.4/1-1.2.6.4.3/1-1.2.6.4.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.3/1-1.2.6.3.2/1-1.2.6.3.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.3/1-1.2.6.3.3/1-1.2.6.3.3:1.0)

	local USB_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.1/1-1.2.5.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.6/1-1.2.6.6.1/1-1.2.6.6.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.5/1-1.2.6.5.1/1-1.2.6.5.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.1/1-1.2.1.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.1/1-1.2.2.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.1/1-1.2.6.1.1/1-1.2.6.1.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.2/1-1.2.6.2.1/1-1.2.6.2.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.1/1-1.2.3.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.1/1-1.2.4.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.4/1-1.2.6.4.1/1-1.2.6.4.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.3/1-1.2.6.3.1/1-1.2.6.3.1:1.0)

	for dev in `ls /dev/ttyACM*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<22;i++))
		do
			if [[ $path =~ ${ACM_dev_path[$i]} ]] ; then
				echo "find $i channle $path"
				ln -sf $dev /dev/opvx/chan_brd/$i
				break
			fi
		done
	done
	
	for dev in `ls /dev/ttyUSB*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<11;i++))
		do
			if [[ $path =~ ${USB_dev_path[$i]} ]] ; then
				echo "find $i board $path"	
				ln -sf $dev /dev/opvx/mod_brd/$i
				break
			fi
		done
	done
}

function generate_udev_rules_hw_VS_1008()
{
	local ACM_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.2/1-1.2.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0)

   local USB_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.1/1-1.2.1.1:1.0 \
 /devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.1/1-1.2.2.1:1.0)

	
	for dev in `ls /dev/ttyACM*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<4;i++))
		do
			if [[ $path =~ ${ACM_dev_path[$i]} ]] ; then
				echo "find $i channle $path"
				ln -sf $dev /dev/opvx/chan_brd/$i
				break
			fi
		done
	done
	
	for dev in `ls /dev/ttyUSB*`; do
		path=`udevadm info -q path -n $dev`
		for ((i=0;i<2;i++))
		do
			if [[ $path =~ ${USB_dev_path[$i]} ]] ; then
				echo "find $i board $path"	
				ln -sf $dev /dev/opvx/mod_brd/$i
				break
			fi
		done
	done
}


function generate_udev_rules()
{
    MB_MCU_DEV=$1
    MB_HW_VER=$2
    MB_USB_TYPE=$3

	mkdir -p /dev/opvx/chan_brd
	mkdir -p /dev/opvx/mod_brd
	
    rule_name=`echo "$MB_MCU_DEV""_""$MB_HW_VER" |tr '/' '_'` #将'/' 替换为 '_'
    time_normal=`date "+%Y-%m-%d %H:%M:%S"`
    time_name=`echo $time_normal | sed 's/:/-/g' | sed 's/ /-/g'`
    tmp_udev_rule=/tmp/"$rule_name"_"$time_name".rule

    if [ x"$USB_TYPE_SWG" = x"$MB_USB_TYPE" ]; then	
    		if [ x"$MB_HW_VER" = x"$HW_VER_1" ]; then
    				generate_udev_rules_hw_ver_swg_10xx
    		else
		        generate_udev_rules_hw_ver_swg_20xx  
		    fi
    elif [ x"$USB_TYPE_VS_M35" = x"$MB_USB_TYPE" -o x"UNDEF_TYPE" = x"$MB_USB_TYPE" ]; then
        if [ x"$DEV_TYPE" = x"1U" ]; then
            generate_udev_rules_hw_VS_M35_1U
        elif [ x"$DEV_TYPE" = x"2U" ]; then
            generate_udev_rules_hw_VS_M35_2U
        elif [ x"$DEV_TYPE" = x"VS1008" ]; then
            generate_udev_rules_hw_VS_1008
        else
            echo "$0 : Unkown hardware dev type : $DEV_TYPE" >> $LOG_FILE
            exit 1
        fi
    else
        echo "$0 : Unkown hardware usb type : $MB_USB_TYPE" >> $LOG_FILE
        exit 1
    fi
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
	VS_USB_TYPE=`$APP_GET_VER $MB_MCU_DEV $APP_GET_VER_WAIT_TIME_MS 2>&1 | sed -n "1p"`
	sleep 1
	MB_HW_VER=`$APP_GET_VER $MB_MCU_DEV $APP_GET_VER_WAIT_TIME_MS 2>&1 |\
		grep "HwVer" |\
		awk -F":" '{print $2}' |\
		sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//'` #去掉首尾空格

#	echo "VS_USB_TYPE=$VS_USB_TYPE, MB_HW_VER=$MB_HW_VER"
	if [ x"$VS_USB_TYPE" = x"$USB_TYPE_SWG"  -o x"$VS_USB_TYPE" = x"$USB_TYPE_VS_M35" ]; then
		USB_TYPE="$VS_USB_TYPE"
	else
		VS_USB_TYPE=`$APP_GET_VER $MB_MCU_DEV $APP_GET_VER_WAIT_TIME_MS 2>&1 | sed -n "2p"`
		if [ x"$VS_USB_TYPE" = x"$USB_TYPE_SWG"  -o x"$VS_USB_TYPE" = x"$USB_TYPE_VS_M35" ]; then
			USB_TYPE="$VS_USB_TYPE"
		else
			USB_TYPE="UNDEF_TYPE"
		fi
	fi
}

#获取设备类型：1U还是2U机箱，
function get_dev_type()
{
	DEV_TYPE=`$APP_GET_DEV $MB_MCU_DEV $APP_GET_DEV_WAIT_TIME_MS 2>&1 |\
		grep "dev_type" |\
		awk -F":" '{print $2}' |\
		sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//'` #去掉首尾空格
	sleep 1
	SLOT_ID=`$APP_GET_DEV $MB_MCU_DEV $APP_GET_DEV_WAIT_TIME_MS 2>&1 |\
		grep "slot_id" |\
		awk -F":" '{print $2}' |\
		sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//'` #去掉首尾空格
}

function main()
{
local USB_dev_path=(/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.2/1-1.2.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.2/1-1.2.5.2:1.0 \

/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2.2/1-1.2.3.2.2:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2.2/1-1.2.4.2.2:1.0 \

/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.1/1-1.2.2.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.1/1-1.2.1.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.1/1-1.2.2.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.1/1-1.2.3.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.1/1-1.2.4.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.6/1-1.2.6.6.1/1-1.2.6.6.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.5/1-1.2.6.5.1/1-1.2.6.5.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.1/1-1.2.6.1.1/1-1.2.6.1.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.2/1-1.2.6.2.1/1-1.2.6.2.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.4/1-1.2.6.4.1/1-1.2.6.4.1:1.0 \
/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.3/1-1.2.6.3.1/1-1.2.6.3.1:1.0)

	j=0
	for dev in `ls /dev/ttyUSB*`; do
		tmp=`udevadm info -q path -n $dev`
		path[$j]=$tmp
		devpath[$j]=$dev
		j=$[j+1]
	done
	
	for((i=0;i<"${#USB_dev_path[*]}";i++))
	do
		for((j=0;j < "${#path[*]}";j++)) 
		do
			if [[ ${path[$j]} =~ ${USB_dev_path[$i]}  ]] ; then
				echo "find mcu device ${path[$j]}"
				MB_MCU_DEV=${devpath[$j]}
				break
			fi
		done
		if [ -n "$MB_MCU_DEV" ]; then 
			break 
		fi
	done

	#如果没有发现除第一块MCU设备的其他board，最后才使用该设备ID来探测
	if [ -z "$MB_MCU_DEV" ]; then
		local first_mcu_path="/devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.1/1-1.2.5.1:1.0"
		for((j=0;j < "${#path[*]}";j++)) 
		do
			if [[ ${path[$j]} =~ $first_mcu_path  ]] ; then
				echo "find mcu device ${path[$j]}"
				MB_MCU_DEV=${devpath[$j]}
				break
			fi
		done
	fi
	get_dev_type
	sleep 1
	get_brd_mcu_key_info
	
	generate_udev_rules $MB_MCU_DEV $MB_HW_VER $USB_TYPE
}

if [ ! -e $LOG_FILE ]; then                                                                                                                                                 
	    touch $LOG_FILE
fi

#通过/etc/start启动脚本执行，因此对于可插拔设备重新插拔后需要再次调用
rm -rf /dev/opvx
main $*
exit 0

