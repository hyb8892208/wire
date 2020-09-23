#!/bin/sh

total_channel=`grep -rn total_chan_count /tmp/hw_info.cfg | awk -F = '{print $2}'`

gen_opvx_swg2016_internet_rules()
{
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3:1.3*\",SYMLINK+=\"opvx/internet/0\" > /etc/udev/rules.d/internet.rules
}

gen_opvx_swg2032_internet_rules()
{
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3:1.3*\",SYMLINK+=\"opvx/internet/0\" > /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.6/1-1.2.5.6:1.3*\",SYMLINK+=\"opvx/internet/1\" >> /etc/udev/rules.d/internet.rules
}

gen_opvx_swg2008_internet_rules()
{
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.4/1-1.2.1.4:1.3*\",SYMLINK+=\"opvx/internet/0\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.4/1-1.2.2.4:1.3*\",SYMLINK+=\"opvx/internet/1\" >> /etc/udev/rules.d/internet.rules
}

gen_opvx_1u_internet_rules()
{
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4:1.3*\",SYMLINK+=\"opvx/internet/0\" > /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.4/1-1.2.1.4:1.3*\",SYMLINK+=\"opvx/internet/1\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.4/1-1.2.2.4:1.3*\",SYMLINK+=\"opvx/internet/2\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.4/1-1.2.3.4:1.3*\",SYMLINK+=\"opvx/internet/3\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.4/1-1.2.4.4:1.3*\",SYMLINK+=\"opvx/internet/4\" >> /etc/udev/rules.d/internet.rules
}

gen_opvx_2u_internet_rules()
{
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.5/1-1.2.5.4/1-1.2.5.4:1.3*\",SYMLINK+=\"opvx/internet/0\" > /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.6/1-1.2.6.6.4/1-1.2.6.6.4:1.3*\",SYMLINK+=\"opvx/internet/1\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.5/1-1.2.6.5.4/1-1.2.6.5.4:1.3*\",SYMLINK+=\"opvx/internet/2\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.1/1-1.2.1.4/1-1.2.1.4:1.3*\",SYMLINK+=\"opvx/internet/3\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.2/1-1.2.2.4/1-1.2.2.4:1.3*\",SYMLINK+=\"opvx/internet/4\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.1/1-1.2.6.1.4/1-1.2.6.1.4:1.3*\",SYMLINK+=\"opvx/internet/5\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.2/1-1.2.6.2.4/1-1.2.6.2.4:1.3*\",SYMLINK+=\"opvx/internet/6\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.3/1-1.2.3.4/1-1.2.3.4:1.3*\",SYMLINK+=\"opvx/internet/7\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.4/1-1.2.4.4/1-1.2.4.4:1.3*\",SYMLINK+=\"opvx/internet/8\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.4/1-1.2.6.4.4/1-1.2.6.4.4:1.3*\",SYMLINK+=\"opvx/internet/9\" >> /etc/udev/rules.d/internet.rules
	echo DEVPATH==\"*devices/pci0000:00/0000:00:1d.0/usb1/1-1/1-1.2/1-1.2.6/1-1.2.6.3/1-1.2.6.3.4/1-1.2.6.3.4:1.3*\",SYMLINK+=\"opvx/internet/10\" >> /etc/udev/rules.d/internet.rules
}

gen_opvx_swg2016_gobinet_rules()
{
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.3:1.4*\",NAME+=\"gobinet0\" > /etc/udev/rules.d/gobinet.rules
}

gen_opvx_swg2032_gobinet_rules()
{
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.3:1.4*\",NAME+=\"gobinet0\" > /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.5.6:1.4*\",NAME+=\"gobinet1\" >> /etc/udev/rules.d/gobinet.rules
}

gen_opvx_swg2008_gobinet_rules()
{
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.1.4:1.4*\",NAME+=\"gobinet0\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.2.4:1.4*\",NAME+=\"gobinet1\" >> /etc/udev/rules.d/gobinet.rules
}

gen_opvx_1u_gobinet_rules()
{
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.5.4:1.4*\",NAME+=\"gobinet0\" > /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.1.4:1.4*\",NAME+=\"gobinet1\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.2.4:1.4*\",NAME+=\"gobinet2\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.3.4:1.4*\",NAME+=\"gobinet3\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.4.4:1.4*\",NAME+=\"gobinet4\" >> /etc/udev/rules.d/gobinet.rules
}

gen_opvx_2u_gobinet_rules()
{
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.5.4:1.4*\",NAME+=\"gobinet0\" > /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.6.6.4:1.4*\",NAME+=\"gobinet1\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.6.5.4:1.4*\",NAME+=\"gobinet2\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.1.4:1.4*\",NAME+=\"gobinet3\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.2.4:1.4*\",NAME+=\"gobinet4\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.6.1.4:1.4*\",NAME+=\"gobinet5\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.6.2.4:1.4*\",NAME+=\"gobinet6\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.3.4:1.4*\",NAME+=\"gobinet7\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.4.4:1.4*\",NAME+=\"gobnet8\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.6.4.4:1.4*\",NAME+=\"gobnet9\" >> /etc/udev/rules.d/gobinet.rules
	echo SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"GobiNet\",DEVPATH==\"*/1-1.2.6.3.4:1.4*\",NAME+=\"gobnet10\" >> /etc/udev/rules.d/gobinet.rules
}

udev_rules_apply()
{
	udevadm control -R
}

gen_internet_dev_again()
{
	/my_tools/bsp_cli upgrade sel -1
	sleep 1
	/my_tools/vs_usb_channel_set.sh
}

if [ $total_channel -eq 8 -o $total_channel -eq 4 ];then
	gen_opvx_swg2008_internet_rules
	gen_opvx_swg2008_gobinet_rules
elif [ $total_channel -eq 20 ];then
	gen_opvx_1u_internet_rules
	gen_opvx_1u_gobinet_rules
elif [ $total_channel -eq 44 ];then
	gen_opvx_2u_internet_rules
	gen_opvx_2u_gobinet_rules
elif [ $total_channel -eq 16 ];then
	gen_opvx_swg2016_internet_rules
	gen_opvx_swg2016_gobinet_rules
elif [ $total_channel -eq 32 ];then
	gen_opvx_swg2032_internet_rules
	gen_opvx_swg2032_gobinet_rules
else
	echo "unknown device, exit"
	exit 1
fi

udev_rules_apply

gen_internet_dev_again

