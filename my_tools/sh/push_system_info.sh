#!/bin/sh
#-------------------------------------------------------------------------
# Post system info into redis
#-------------------------------------------------------------------------
#PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
#SOURCE_BOOT_CONFILE="/factory/grub/boot_config"
#CURRENT_FW_VERSION_TXT=/usr/local/fw_release
#CURRENT_FW_BUILD_TXT=/usr/local/fw_build
#DEVICE_INFO="/var/device.info" #/tmp/hwinfo
OUTPUT="/var/log/redis.list"
#BOARD_INFO=/proc/board/info

LOGFILE_INTERVAL=20

#print  all system info into var/log/redis.txt
echo -n "" > ${OUTPUT}
## need to change MOUNT7,PARTITION7
#-------------------------------------------------------------------------
# product info 
#-------------------------------------------------------------------------
	PRODUCT_NUMBER=0
	PRODUCT_HW_VERSION=0
	PRODUCT_HW_TYPE=0
	PRODUCT_SW_VERSION=`cat /version/version`
	PRODUCT_SW_BUILD=`cat /version/build_time`
	PRODUCT_SW_CUSTOMID=0
	PRODUCT_SERIALNUMBER=0
	PRODUCT_HWCLOCK=0  #'1' stands for system has RTC, while '0' is not
	PRODUCT_BOARD_TYPE=0
	PRODUCT_BOARD_SPAN=0
	PRODUCT_BOARD_VERSION=0
	PRODUCT_BOARD_FALC_VERSION=0
	PRODUCT_HWEC_MODE=0
	PRODUCT_HARDEC_TYPE=0
	PRODUCT_HARDCODEC_VERSION=0
	PRODUCT_HARDCODEC_OCT_VERSION=0
	PRODUCT_HARDCODEC_CHANNELS=0

#-------------------------------------------------------------------------
# 2. collect the system info
#-------------------------------------------------------------------------

	SYSTEM_CPU=0
	SYSTEM_CPU_BIT=32
	SYSTEM_MEMORY=0
	SYSTEM_KERNEL=0
	SYSTEM_KERNEL_BUILD_TIME=0
	SYSTEM_BIOS_VENDOR="unknown"
	SYSTEM_BIOS_VERSION="unknown"
	SYSTEM_NET_IFACE="eth0"
	SYSTEM_NET_ETH0_IP=0
	SYSTEM_NET_IP=0
	SYSTEM_NET_MAC=0
	SYSTEM_NET_MASK=0
	SYSTEM_GRUB_BOOT=0
	SYSTEM_GRUB_BFS=0
	SYSTEM_BOOT_CHECK=0
	SYSTEM_BOOT_ERROR=0
	SYSTEM_BOOT_COUNT=0
	SYSTEM_HDD_MODEL=0
	SYSTEM_HDD_SIZE=0
	SYSTEM_HDD_PARTITION=0
	SYSTEM_FIRMWARE_VERSION=0
	SYSTEM_FIRMWARE_BUILD=0
	
	QUOTA_MARK="\""	

# 2.1 hardware info
	SYSTEM_CPU=`cat /proc/cpuinfo | grep 'model name'| head -1 | awk '{print $4 " " $5 " " $6 " " $7 " " $8 " " $9}'`

get_meminfo()
{
		MEM_INT=`cat /proc/meminfo |grep 'MemTotal' |awk -F : '{print $2}' |sed 's/^[ \t]*//g'|sed 's/[a-z][A-Z]//g'`;

		if [ "$MEM_INT" -gt "4000000" ]; then
			MEM="4096";
		elif [ "$MEM_INT" -gt "3000000" ]; then
			MEM="3072";
		elif [ "$MEM_INT" -gt "2000000" ]; then
			MEM="2048";
		elif [ "$MEM_INT" -gt "1000000" ]; then	
			MEM="1024";
		else
			MEM=`expr $MEM_INT / 1024`;
		fi
		return $MEM;	
}

	get_meminfo	
	SYSTEM_MEMORY=`echo "$MEM (MB)"`;
	SYSTEM_KERNEL=`uname -a |awk '{printf("%s-%s\n",$1,$3)}'`	
	SYSTEM_KERNEL_BUILD_TIME=`uname -a |awk '{ printf("%s-%s-%s-%s-%s",$6,$7,$8,$11,$9)}'`	
	
	SYSTEM_NET_IFACE="`ifconfig -a |grep "eth" |awk '{print $1}'|head -1`"                                                                     
	SYSTEM_NET_IP="`ifconfig ${SYSTEM_NET_IFACE} | grep "inet addr" | awk '{print($2)}' | cut -c 6-`"                                          
	SYSTEM_NET_MAC="`ifconfig ${SYSTEM_NET_IFACE} | grep "HWaddr" | awk '{print($5)}'`"                                                        
	SYSTEM_NET_MASK="`ifconfig ${SYSTEM_NET_IFACE} | grep "Mask" | awk '{print($4)}' | cut -c 6-`" 

	var=`cat /etc/cfg/grubcfg/grubenv | grep curr_area |  cut -d "=" -f 2`
	SYSTEM_GRUB_BOOT="`echo ${var#*curr_area:}`"
	SYSTEM_GRUB_BFS="`mount | grep "/dev/mmcblk" | head -1 | awk '{print $5}'`"

	SYSTEM_BOOT_CHECK=0
	SYSTEM_BOOT_ERROR="no"
	SYSTEM_BOOT_COUNT="`cat /data/info/rtimes`"
	
#-----------------------------------------------------------------------o-
# 3. collect the application info
#------------------------------------------------------------------------
	APP_UPGRADE_ERROR=0
	APP_LOGFILE_INTERVAL=${LOGFILE_INTERVAL}
	APP_CALLSTAT_ANSWERED=0
	APP_CALLSTAT_UNKNOWN=0
	APP_CALLSTAT_CONGESION=0
	APP_CALLSTAT_BUSY=0
	APP_CALLSTAT_FAILED=0
	APP_CALLSTAT_NOANSWER=0
	APP_CALLSTAT_CURRENTCALLS=0
	APP_CALLSTAT_ACCCALLS=0
	APP_CALLSTAT_ACCDURATION=0
	APP_GENERAL_TONEZONE=0
#-------------------------------------------------------------------------
# 4. push all these info into Redis database
#-------------------------------------------------------------------------
	
#-------------------------------------------------------------------------
# 4. Add advanced company information,2015-11-09 10:43
#-------------------------------------------------------------------------
	ADV_COMPY_NAME="openvox"
	
	if [ x"${ADV_COMPY_NAME}" = "xopenvox" ]; then 
		ADV_COMPY_ADDRESS="10/F, Building 6-A, Baoneng Science and Technology Industrial Park, Longhua New District, Shenzhen, Guangdong,China"
		ADV_COMPY_ADDRESS_CN="广东省深圳市龙华新区宝能科技园6栋A座10楼"
		ADV_COMPY_TEL="+86-755-82535461"
		ADV_COMPY_FAX="+86-755-83823074"
		ADV_COMPY_EMAIL="support@openvox.cn"
		ADV_COMPY_WEBSITE="http://www.openvox.cn"
		ADV_COMPY_TITLE="Openvox T1/E1 Gateway Administration Console"
		ADV_COMPY_ONLINE_UPGRADE="yes"
		ADV_COMPY_SIP_USERAGENT="VoxStack T1/E1 Gateway"
		ADV_COMPY_SIP_SDPSESSION="VoxStack T1/E1 Gateway"
		ADV_COMPY_COPYRIGHTS="Copyright © 2015 OpenVox All Rights Reserved."
	elif [ x"${ADV_COMPY_NAME}" = "xy-free" ]; then 
		ADV_COMPY_ADDRESS="1Rm.1012,No.1 Building,No.2875,YangGao Rd.,Shanghai"
		ADV_COMPY_ADDRESS_CN="上海市浦东新区杨高南路2875号康琳创意园1号楼1012室"
		ADV_COMPY_TEL="+86-21-33191992-801"
		ADV_COMPY_FAX="+86-21-33191992-860"
		ADV_COMPY_EMAIL="shenyj@h-shen.com"
		ADV_COMPY_WEBSITE="www.h-shen.com"
		ADV_COMPY_TITLE="Y-Free T1/E1 Gateway Administration Console"
		ADV_COMPY_ONLINE_UPGRADE="no"
		ADV_COMPY_SIP_USERAGENT="Y-Free T1/E1 Gateway"
		ADV_COMPY_SIP_SDPSESSION="Y-Free T1/E1 Gateway"
		ADV_COMPY_COPYRIGHTS="Copyright © Shanghai HaoShen Information Technology Co.,Ltd All Rights Reserved. 沪ICP备14010026号"
		PREFIX_TYPE="TG-100"
		PRODUCT_BOARD_TYPE="${PREFIX_TYPE}${PRODUCT_BOARD_SPAN}"
	else
		ADV_COMPY_ADDRESS=""
		ADV_COMPY_TEL=""
		ADV_COMPY_FAX=""
		ADV_COMPY_EMAIL=""
		ADV_COMPY_WEBSITE=""
		ADV_COMPY_TITLE="Digital T1/E1 Gateway Administration Console"
		ADV_COMPY_ONLINE_UPGRADE="no"
		ADV_COMPY_SIP_USERAGENT="Digital T1/E1 Gateway"
		ADV_COMPY_SIP_SDPSESSION="Digital T1/E1 Gateway"
		ADV_COMPY_COPYRIGHTS="Digital T1/E1 Gateway"
		PREFIX_TYPE="TG-100"
		PRODUCT_BOARD_TYPE="${PREFIX_TYPE}${PRODUCT_BOARD_SPAN}"

	fi


#echo "pushing all system info into Redis"
#/usr/bin/redis-cli -a O7r8yGv0cdlNM4lQ & <<EOF
/my_tools/redis-cli >/dev/null 2>&1 <<EOF
set product.number	"${PRODUCT_NUMBER}"
set product.hw.version	"${PRODUCT_HW_VERSION}"
set product.hw.type	"${PRODUCT_HW_TYPE}"
set product.sw.version	"${PRODUCT_SW_VERSION}"
set product.sw.build	"${PRODUCT_SW_BUILD}"
set product.sw.customid	"${PRODUCT_SW_CUSTOMID}"
set product.serialnumber	"${PRODUCT_SERIALNUMBER}"
set product.hwclock	"${PRODUCT_HWCLOCK}"
set product.board.type "${PRODUCT_BOARD_TYPE}"
set product.board.span "${PRODUCT_BOARD_SPAN}"
set product.board.version "${PRODUCT_BOARD_VERSION}"
set product.board.falc.version "${PRODUCT_BOARD_FALC_VERSION}"
set product.hardec.type "${PRODUCT_HWEC_MODE}"
set product.hardcodec.version "${PRODUCT_HARDCODEC_VERSION}"
set product.hardcodec.oct.version "${PRODUCT_HARDCODEC_OCT_VERSION}"
set product.hardcodec.channels "${PRODUCT_HARDCODEC_CHANNELS}"
set system.cpu		"${SYSTEM_CPU}"
set system.cpu.bit	"${SYSTEM_CPU_BIT}"
set system.memory	"${SYSTEM_MEMORY}"
set system.kernel	"${SYSTEM_KERNEL}"
set system.kernel.build.time	"${SYSTEM_KERNEL_BUILD_TIME}"
set system.bios.vendor	"${SYSTEM_BIOS_VENDOR}"
set system.bios.version	"${SYSTEM_BIOS_VERSION}"
set system.net.iface	"${SYSTEM_NET_IFACE}"
set system.net.eth0.ip	"${SYSTEM_NET_ETH0_IP}"
set system.net.ip	"${SYSTEM_NET_IP}"
set system.net.mac	"${SYSTEM_NET_MAC}"
set system.net.mask	"${SYSTEM_NET_MASK}"
set system.grub.boot	"${SYSTEM_GRUB_BOOT}"
set system.grub.bfs	"${SYSTEM_GRUB_BFS}"
set system.boot.check	"${SYSTEM_BOOT_CHECK}"
set system.boot.error	"${SYSTEM_BOOT_ERROR}"
set system.hdd.model	"${SYSTEM_HDD_MODEL}"
set system.hdd.size	"${SYSTEM_HDD_SIZE}"
set system.hdd.partition	"${SYSTEM_HDD_PARTITION}"
set system.firmware.version "${SYSTEM_FIRMWARE_VERSION}"
set system.firmware.build "${SYSTEM_FIRMWARE_BUILD}"
set app.logfile.interval "${APP_LOGFILE_INTERVAL}"
set adv.com.name "${ADV_COMPY_NAME}"
set adv.com.address "${ADV_COMPY_ADDRESS}"
set adv.com.address.cn "${ADV_COMPY_ADDRESS_CN}"
set adv.com.tel "${ADV_COMPY_TEL}"
set adv.com.fax "${ADV_COMPY_FAX}"
set adv.com.email "${ADV_COMPY_EMAIL}"
set adv.com.website "${ADV_COMPY_WEBSITE}"
set adv.com.title "${ADV_COMPY_TITLE}"
set adv.com.online.upgrade "${ADV_COMPY_ONLINE_UPGRADE}"
set adv.com.sip.useragent "${ADV_COMPY_SIP_USERAGENT}"
set adv.com.sip.sdpsession "${ADV_COMPY_SIP_SDPSESSION}"
set adv.com.copyrights "${ADV_COMPY_COPYRIGHTS}"
EOF

echo -ne "product number        : ${PRODUCT_NUMBER}\n" >> ${OUTPUT}
echo -ne "product hw version    : ${PRODUCT_HW_VERSION}\n" >>  ${OUTPUT}
echo -ne "product hw type       : ${PRODUCT_HW_TYPE}\n" >>  ${OUTPUT}
echo -ne "product sw version    : ${PRODUCT_SW_VERSION}\n" >>  ${OUTPUT}
echo -ne "product sw build      : ${PRODUCT_SW_BUILD}\n" >>  ${OUTPUT}
echo -ne "product sw customid   : ${PRODUCT_SW_CUSTOMID}\n" >>  ${OUTPUT}
echo -ne "product serialnumber  : ${PRODUCT_SERIALNUMBER}\n" >>  ${OUTPUT}
echo -ne "product hwclock       : ${PRODUCT_HWCLOCK}\n" >>  ${OUTPUT}
echo -ne "E1 board mode         : ${PRODUCT_BOARD_TYPE}\n" >> ${OUTPUT}
echo -ne "E1 board span numbers : ${PRODUCT_BOARD_SPAN}\n" >> ${OUTPUT}
echo -ne "E1 board version      : ${PRODUCT_BOARD_VERSION}\n" >> ${OUTPUT}
echo -ne "E1 board falc version : ${PRODUCT_BOARD_FALC_VERSION}\n" >> ${OUTPUT}
echo -ne "Hard echocan          : ${PRODUCT_HWEC_MODE}\n" >> ${OUTPUT}
echo -ne "Hard codec fw version : ${PRODUCT_HARDCODEC_VERSION}\n" >> ${OUTPUT}
echo -ne "Hard codec OCT version: ${PRODUCT_HARDCODEC_OCT_VERSION}\n" >> ${OUTPUT}
echo -ne "Hard codec channels   : ${PRODUCT_HARDCODEC_CHANNELS}\n" >> ${OUTPUT}
echo -ne "CPU                   : ${SYSTEM_CPU}\n" >> ${OUTPUT}
echo -ne "CPU bit               : ${SYSTEM_CPU_BIT}\n" >> ${OUTPUT}
echo -ne "memory size           : ${SYSTEM_MEMORY}\n"	>> ${OUTPUT}
echo -ne "kernel                : ${SYSTEM_KERNEL}\n" >>  ${OUTPUT}
echo -ne "kernel build time     : ${SYSTEM_KERNEL_BUILD_TIME}\n"   >>  ${OUTPUT}
echo -ne "bios vendor           : ${SYSTEM_BIOS_VENDOR}\n"	>>  ${OUTPUT}
echo -ne "bios version          : $SYSTEM_BIOS_VERSION\n"	>>  ${OUTPUT}
echo -ne "net_iface             : $SYSTEM_NET_IFACE\n" >>  ${OUTPUT}
echo -ne "eth0                  : $SYSTEM_NET_ETH0_IP\n" >>  ${OUTPUT}
echo -ne "ip                    : $SYSTEM_NET_IP\n" >>  ${OUTPUT}
echo -ne "mac                   : $SYSTEM_NET_MAC\n" >>  ${OUTPUT}
echo -ne "mask                  : $SYSTEM_NET_MASK\n" >>  ${OUTPUT}
echo -ne "basefs                : $SYSTEM_GRUB_BFS\n" >>  ${OUTPUT}
echo -ne "kernel                : $SYSTEM_GRUB_BOOT\n" >>  ${OUTPUT}
echo -ne "hdd model             : $SYSTEM_HDD_MODEL\n" >>  ${OUTPUT}
echo -ne "hdd size              : $SYSTEM_HDD_SIZE\n" >>  ${OUTPUT}
echo -ne "hdd partition         : $SYSTEM_HDD_PARTITION\n" >> ${OUTPUT}
echo -ne "firmware version      : ${SYSTEM_FIRMWARE_VERSION}\n" >> ${OUTPUT}
echo -ne "firmware build number : ${SYSTEM_FIRMWARE_BUILD}\n" >> ${OUTPUT}
echo -ne "logfile interval      : ${APP_LOGFILE_INTERVAL}\n" >> ${OUTPUT}
echo -ne "company name          : ${ADV_COMPY_NAME}\n" >> ${OUTPUT}
echo -ne "company address       : ${ADV_COMPY_ADDRESS}\n" >> ${OUTPUT}
echo -ne "公司地址              : ${ADV_COMPY_ADDRESS_CN}\n" >> ${OUTPUT}
echo -ne "company tel           : ${ADV_COMPY_TEL}\n" >> ${OUTPUT}
echo -ne "company fax           : ${ADV_COMPY_FAX}\n" >> ${OUTPUT}
echo -ne "company email         : ${ADV_COMPY_EMAIL}\n" >> ${OUTPUT}
echo -ne "company website       : ${ADV_COMPY_WEBSITE}\n" >> ${OUTPUT}
echo -ne "company title         : ${ADV_COMPY_TITLE}\n" >> ${OUTPUT}
echo -ne "company online upgrade: ${ADV_COMPY_ONLINE_UPGRADE}\n" >> ${OUTPUT}
echo -ne "company sip useragent : ${ADV_COMPY_SIP_USERAGENT}\n" >> ${OUTPUT}
echo -ne "company sip sdpsession: ${ADV_COMPY_SIP_SDPSESSION}\n" >> ${OUTPUT}
echo -ne "company copyrights    : ${ADV_COMPY_COPYRIGHTS}\n" >> ${OUTPUT}
