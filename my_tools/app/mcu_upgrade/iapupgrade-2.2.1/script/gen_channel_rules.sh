#!/bin/sh

#udev生成的所有设备在/dev/$OPVX_DEV_ROOT目录下
OPVX_DEV_ROOT=opvx

#通道板设备目录/dev/$OPVX_DEV_ROOT/$CHAN_BRD_DIR
CHAN_BRD_DIR=chan_brd
UDEV_RULE_PATH=/etc/udev/rules.d
RULE_FILE=${UDEV_RULE_PATH}/swg_channel.rules

total_chans=`grep -rn total_chan_count /tmp/hw_info.cfg | awk -F = '{print $2}'`

gen_opvx_swg2016_chan_rules()
{
	RULE_FILE=${UDEV_RULE_PATH}/swg_channel.rules
	cat > $RULE_FILE << END_INPUT
#Create channel symbol links
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.1/1-1.2.1.1:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/0" 
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/1"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/2"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.4/1-1.2.1.4:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/3"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.5/1-1.2.1.5:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/4"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.6/1-1.2.1.6:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/5"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.7/1-1.2.1.7:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/6"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/7"
END_INPUT

}

gen_opvx_swg2032_chan_rules()
{
	RULE_FILE=${UDEV_RULE_PATH}/swg_channel.rules
	cat > $RULE_FILE << END_INPUT
#Create channel symbol links
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.1/1-1.2.1.1:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/0" 
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/1"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/2"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.4/1-1.2.1.4:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/3"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.5/1-1.2.1.5:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/4"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.6/1-1.2.1.6:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/5"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.7/1-1.2.1.7:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/6"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/7"

DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.1/1-1.2.5.4.1:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/8" 
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.2/1-1.2.5.4.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/9"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.3/1-1.2.5.4.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/10"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.4/1-1.2.5.4.4:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/11"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.5/1-1.2.5.4.5:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/12"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.6/1-1.2.5.4.6:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/13"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4.7/1-1.2.5.4.7:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/14"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.3/1-1.2.5.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/15"
END_INPUT

}

gen_opvx_swg2032_chan_rules()
{
	RULE_FILE=${UDEV_RULE_PATH}/swg_channel.rules
	cat > $RULE_FILE << END_INPUT
#Create channel symbol links
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/0"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/1"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.2/1-1.2.2.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/2"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/3"
END_INPUT

}

gen_opvx_vs_1u_chan_rules()
{
	RULE_FILE=${UDEV_RULE_PATH}/vs_usb_channel.rules
	cat > $RULE_FILE << END_INPUT
#Create channel symbol links
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.2/1-1.2.5.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/0"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.3/1-1.2.5.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/1"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/2"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/3"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.2/1-1.2.2.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/4"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/5"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/6"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.3/1-1.2.3.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/7"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/8"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.3/1-1.2.4.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/9"
END_INPUT

}

gen_opvx_vs_2u_chan_rules()
{
	RULE_FILE=${UDEV_RULE_PATH}/vs_usb_channel.rules
	cat > $RULE_FILE << END_INPUT
#Create channel symbol links
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.2/1-1.2.5.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/0"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.3/1-1.2.5.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/1"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.6/1-1.2.6.6.2/1-1.2.6.6.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/2"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.6/1-1.2.6.6.3/1-1.2.6.6.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/3"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.5/1-1.2.6.5.2/1-1.2.6.5.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/4"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.5/1-1.2.6.5.3/1-1.2.6.5.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/5"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.2/1-1.2.1.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/6"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.3/1-1.2.1.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/7"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.2/1-1.2.2.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/8"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.3/1-1.2.2.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/9"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.1/1-1.2.6.1.2/1-1.2.6.1.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/10"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.1/1-1.2.6.1.3/1-1.2.6.1.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/11"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.2/1-1.2.6.2.2/1-1.2.6.2.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/12"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.2/1-1.2.6.2.3/1-1.2.6.2.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/13"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.2/1-1.2.3.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/14"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.3/1-1.2.3.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/15"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.2/1-1.2.4.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/16"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.3/1-1.2.4.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/17"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.4/1-1.2.6.4.2/1-1.2.6.4.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/18"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.4/1-1.2.6.4.3/1-1.2.6.4.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/19"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.3/1-1.2.6.3.2/1-1.2.6.3.2:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/20"
DEVPATH=="*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.3/1-1.2.6.3.3/1-1.2.6.3.3:1.0*", SYMLINK+="$OPVX_DEV_ROOT/$CHAN_BRD_DIR/21"
END_INPUT

}

udev_rules_apply()
{
	udevadm control -R
}

main()
{
	total_chans=`grep -rn total_chan_count /tmp/hw_info.cfg | awk -F = '{print $2}'`
	sys_type=`grep -rn sys_type /tmp/hw_info.cfg | awk -F = '{print $2}'`
	
	if [ $sys_type -eq 1 ]; then
	#SWG XX
		if [ $total_chans -le 16 ];then
			gen_opvx_swg2016_chan_rules
		elif [ $total_chans -le 32 ];then
			gen_opvx_swg2032_chan_rules
		else
			echo "SWG_XX unknown total channel count[$total_chans], exit"
			exit 1
		fi
	elif [ $sys_type -eq 3 ]; then
	#VS_USB_XX
		if [ $total_chans -le 8 ];then
			gen_opvx_swg2008_chan_rules
		elif [ $total_chans -le 20 ];then
			gen_opvx_vs_1u_chan_rules
		elif [ $total_chans -le 44 ];then
			gen_opvx_vs_2u_chan_rules
		else
			echo "VS_USB_XX unknown total channel count[$total_chans], exit"
		fi
	else
		echo "unknown system type[$sys_type] exit."
		exit 1
	fi
	
	udev_rules_apply
}

main $*
exit 0
