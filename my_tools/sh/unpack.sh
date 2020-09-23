#!/bin/sh
#---------------------------------------------------------------
# the script use for unpack the firmware and update images 
# 1.1 check file
# 1.2 stop programs and release memory
#	1.3 unzip firmware image
#	1.4 check md5 key 
#	1.5 Decryption
#	1.6 unpack the zipped file,then list its contents
# 1.7 call upgrade.sh
#----------------------------------------------------------------
	WORK_DIR="/tmp"
	PREFIX_DIR="/tmp"

	usage()
	{
		echo "Usage : $0 [filename]"
		echo "        $0 /path/firmware.bin"
	}
	if [ $# -ne 1 ]; then 
		echo "Are you sure that you have input source file ?"
		usage
		exit 1
	fi
	
	echo "------------- start unpack ---------------"
	date
	
	FIRMWARE_BIN=$1
		
	### 1.1 check file
	echo "check file..."
	auto_update -c ${FIRMWARE_BIN}
	if [ "$?" = "1" ]; then 
	  echo "check file head success!"
	else
	  echo "unknown file format!"
	  exit 1
	fi
	
	### 1.2 kill process and release memory
	echo "kill process and release memory..."
	echo "before release memory:"
	free
	/etc/init.d/asterisk stop
	/etc/init.d/logfile_monitor stop
	kill `pidof sms_recv`
	rm /tmp/log -rf
	sync && echo 3 > /proc/sys/vm/drop_caches
	echo "after release memory:"
	free
	### check upgrade space
	dir_size=`df -h ${WORK_DIR} | tail -1 | awk '{print $4}' | cut -d'M' -f1`
	dir_size=$(echo ${dir_size} | awk '{printf("%d", $1)}')
	if [ `expr ${dir_size} \> 45` = 1 ]; then
	    echo "${WORK_DIR} size: ${dir_size}M"
	else
	    echo "${WORK_DIR} size: ${dir_size}M, not enough for upgrade!"
	    exit 1
	fi

	### 1.3 unzip firmware image. firmware.bin at ${WORK_DIR}/ 
	if [ -d ${WORK_DIR}/firmware ]; then
		rm -rf ${WORK_DIR}/firmware/*
	else
		mkdir -p ${WORK_DIR}/firmware
	fi

	tail -c +33 ${FIRMWARE_BIN} > ${WORK_DIR}/img.tar.gz
	rm ${FIRMWARE_BIN} -f
	FIRMWARE_BIN=${WORK_DIR}/img.tar.gz
	tar -zxf ${FIRMWARE_BIN} -C ${WORK_DIR}/firmware
	if [ "$?" -ne "0" ]; then 
		logger "error code 3: failed to unpack the firmware image ****"; 
		echo "error code 3: failed to unpack the firmware image"; 
		exit 3
	fi
	cd ${WORK_DIR}/firmware
	
	#echo "src file : $STAT_SRCFILE"
	SRCFILE=`ls -l |grep -v "md5"| awk '{print $9}'`
	SRCMD5FILE=`ls -l |grep "md5"| awk '{print $9}'`
	[ ! -f ${SRCFILE} ] && (echo "error code 4: ${SRCFILE} is not a file"; exit 4)
	[ ! -f ${SRCMD5FILE} ] && (echo "error code 4: ${SRCMD5FILE} is not a file"; exit 4)
	### 1.4 check md5 key   ######
	TMP_SRCFILE_MD5=`md5sum ${SRCFILE}|awk '{print $1}'`
	TMP_SRCMD5FILE=`cat ${SRCMD5FILE}`
	if [ "${SRCFILE}" = "" ] || [ "${SRCMD5FILE}" = "" ]; then 
		echo "Are you sure there are two files to be check ?"
		exit 4
	else
		echo -ne "\nChecking ${SRCFILE} md5 key ..."
		if [ "${TMP_SRCFILE_MD5}" = "${TMP_SRCMD5FILE}" ]; then
			echo -ne "\t[\033[32m OK \033[0m]\n"
		else
			echo -ne "\t[\033[31m Failed \033[0m]\n"
			echo "error code 5 : firmware image mismatch standard md5 key!"
			cd ${WORK_DIR}
			rm ${FIRMWARE_BIN} -f
			rm -rf ${WORK_DIR}/firmware
			exit 5
		fi	
	fi

	echo "remove file $FIRMWARE_BIN"
	rm ${FIRMWARE_BIN} -f

	### 1.5 Decryption ###
	cd ${WORK_DIR}/firmware
	STRNAME=`echo $SRCFILE`	
	NEWSRCFILE=${STRNAME%.*}
	echo "decrypt ${SRCFILE} to ${NEWSRCFILE}.tar.gz ..."
	auto_update -o ${SRCFILE} -f ${NEWSRCFILE}.tar.gz
	if [ "$?" -eq "0" ]; then 
	  echo "decrypt done! remove ${SRCFILE} & ${SRCMD5FILE}"
		rm -f ${SRCFILE}
		rm -f ${SRCMD5FILE}
	else
		echo "error code 6 : fail to decrypt the file ${SRCFILE}"; 
		cd ${WORK_DIR}
		rm -rf ${WORK_DIR}/firmware
		exit 6
	fi
	### 1.6 unpack the zipped file,then list its contents 
	echo "tar ${NEWSRCFILE}.tar.gz ..."
	tar -zxf ${NEWSRCFILE}.tar.gz
	if [ $? = 0 ]; then 
		rm -rf ${NEWSRCFILE}.tar.gz
		echo -e "tar done! upgrade packages include files as follows :"
		ls ${WORK_DIR}/firmware
		### Copy ${WORK_DIR}/firmware/upgrade.sh to ${WORK_DIR}/../tmp
#		cp -f ${WORK_DIR}/firmware/upgrade.sh ${PREFIX_DIR}
	else
		rm -rf ${NEWSRCFILE}.tar.gz
		rm -rf ${WORK_DIR}/firmware
		echo "error code 7 : unzip ${NEWSRCFILE}.tar.gz failed!"
		exit 7
	fi
 
	LICENSE_SW=`ls /data/crt/license*.crt 2> /dev/null | wc -l`
	if [ "$LICENSE_SW" != "0" ];then
		if [ ! -f ${WORK_DIR}/firmware/version ];then
			echo "Are you sure firmware is later than 2.1.2?"
			exit 8
		fi
	fi

 ### 1.7 call upgrade.sh  ###
 sync && echo 3 > /proc/sys/vm/drop_caches
 . ${WORK_DIR}/firmware/upgrade.sh
