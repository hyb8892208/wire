#!/bin/sh
# switch system 

data_dir=/data/info
efi_dir=/data/info/efi
config_dir=/data/info/config

curr_version=`cat /version/version`

is_same_fs()
{
	fs1_type=`blkid -s TYPE /dev/mmcblk0p4 | awk -F= '{print $2}'`
	fs2_type=`blkid -s TYPE /dev/mmcblk0p5 | awk -F= '{print $2}'`
	
	if [ -z $fs1_type ]; then
		return 0
	elif [ x"$fs1_type" = x"$fs2_type" ]; then
		return 1
	else
		return 0
	fi
}

bak_config()
{
	config_ver=$curr_version
	if [ ! -d ${config_dir} ]; then
		mkdir -p ${config_dir}
	fi

	ver1=`echo $config_ver | awk -F"." '{print $1}'`
	ver2=`echo $config_ver | awk -F"." '{print $2}'`
	if [ $ver1 -eq 2 -a $ver2 -eq 1 ]; then
		config_ver=2.1.x
	fi

	if [ ! -f /tmp/.bak_config ]; then
		if [ -f ${config_dir}/ast_cfg_${config_ver}.tar.gz ]; then
			rm -rf ${config_dir}/ast_cfg_${config_ver}.tar.gz
		fi
		cd /etc/cfg
		tar -zcf ${config_dir}/ast_cfg_${config_ver}.tar.gz grubcfg/grubenv
		calc_md5=`md5sum ${config_dir}/ast_cfg_${config_ver}.tar.gz | awk '{print $1}'`
		echo $calc_md5 > ${config_dir}/ast_cfg_${config_ver}.tar.gz.md5
		cd -
		touch /tmp/.bak_config
	fi
}

extract_config()
{
	tag_ver=$1

	if [ ! -f ${config_dir}/ast_cfg_${tag_ver}.tar.gz ]; then
		echo "${config_dir}/ast_cfg_${tag_ver}.tar.gz is absent."
		exit 7
	fi
	calc_md5=`md5sum ${config_dir}/ast_cfg_${tag_ver}.tar.gz | awk '{print $1}'`
	md5_file=`cat ${config_dir}/ast_cfg_${tag_ver}.tar.gz.md5`
	if [ -z $md5_file ]; then
		echo "The file ${config_dir}/ast_cfg_${tag_ver}.tar.gz.md5 is NULL."
		exit 8
	elif [ x"$md5_file" = x"$calc_md5" ]; then
		tar -zxf ${config_dir}/ast_cfg_${tag_ver}.tar.gz -C /etc/cfg/
	else
		echo "config md5 is wrong. calc_md5=$calc_md5, md5_file=$md5_file ."
		exit 9
	fi
}

burn_efi()
{
	tag_ver=$1

	if [ ! -f ${efi_dir}/efi_${tag_ver}.img ]; then
		echo "${efi_dir}/efi_${tag_ver}.img is absent."
		exit 4
	fi
	calc_md5=`md5sum ${efi_dir}/efi_${tag_ver}.img | awk '{print $1}'`
	md5_file=`cat ${efi_dir}/efi_${tag_ver}.img.md5`
	if [ -z $md5_file ]; then
		echo "The file ${efi_dir}/efi_${tag_ver}.img.md5 is NULL."
		exit 5
	elif [ x"$md5_file" = x"$calc_md5" ]; then
		dd if=${efi_dir}/efi_${tag_ver}.img of=/dev/mmcblk0p1
	else
		echo "efi md5 is wrong. calc_md5=$calc_md5, md5_file=$md5_file ."
		exit 6
	fi
}

restore_file()
{
	restore_version=
	ver1=`echo $curr_version | awk -F"." '{print $1}'`
	ver2=`echo $curr_version | awk -F"." '{print $2}'`

	if [ $ver1 -eq 2 ]; then
		if [ $ver2 -eq 0 ]; then
			restore_version=`cat ${efi_dir}/version_squash`
			burn_efi $restore_version
			extract_config $restore_version
		elif [ $ver2 -eq 1 ]; then
			restore_version=`cat ${efi_dir}/version_ext4`
			burn_efi $restore_version
			extract_config $restore_version
		else
			echo "version is 2.${ver2}.xx invalid."
			exit 3
		fi
	elif [ $ver1 -eq 1 ];then
		if [ $ver2 -eq 5 ];then
			restore_version=`cat ${efi_dir}/version_squash`
			burn_efi $restore_version
			extract_config $restore_versio
		fi
	else
		echo "version is ${ver1}.xx.xx invalid."
		exit 2
	fi
}

get_other_fs_dev()
{
	currappfs=$1
	ret_otherappfs=

	if [ x"${currappfs}" = x"mmcblk0p5" ]; then
		ret_otherappfs="mmcblk0p4"
	else
		ret_otherappfs="mmcblk0p5"
	fi

	echo $ret_otherappfs
}

get_other_appfs_dev()
{
	appfs1_dev=`mount |grep "/dev/mmcblk0p4 "|awk '{print $3}'`
	appfs2_dev=`mount |grep "/dev/mmcblk0p5 "|awk '{print $3}'`
	mount_appfs_fail=0

	if [ x"${appfs1_dev}" = x"/gateway" -a  x"${appfs2_dev}" = x"/gateway" ]; then
		mount_appfs_fail=1
	elif [ x"${appfs1_dev}" = x"/gateway" ]; then
		other_appfs=mmcblk0p5
	elif [ x"${appfs2_dev}" = x"/gateway" ]; then
		other_appfs=mmcblk0p4
	else
		mount_appfs_fail=1
	fi
	curr_appfs_dev=`mount |grep " /gateway "|awk '{print $1}'`

	if [ ${mount_appfs_fail} -eq 1 ]; then
		ret_curr_appfs=
		if [ -n ${current_kernel} ]; then
			if [ x"$current_appfs" = x"mmcblk0p5" -o x"$current_appfs" = x"mmcblk0p4" ]; then
				ret_curr_appfs=$current_appfs
				other_appfs=`get_other_fs_dev $ret_curr_appfs`
			fi
		fi
		if [ -z $ret_curr_appfs ]; then
			if [ x"${curr_area_value}" = x1 ]; then
				other_appfs=mmcblk0p5
			else
				other_appfs=mmcblk0p4
			fi
		fi
	fi
	echo ${other_appfs}
}

get_other_boot()
{
	boot1_dev=`mount |grep "/dev/mmcblk0p2 "|awk '{print $3}'`
	boot2_dev=`mount |grep "/dev/mmcblk0p3 "|awk '{print $3}'`
	mount_boot_fail=0

	if [ x"${boot1_dev}" = x"/boot" -a  x"${boot2_dev}" = x"/boot" ]; then
		mount_boot_fail=1
	elif [ x"${boot1_dev}" = x"/boot" ]; then
		other_kernel=kernel2
	elif [ x"${boot2_dev}" = x"/boot" ]; then
		other_kernel=kernel1
	else
		mount_boot_fail=1
	fi

	if [ ${mount_boot_fail} -eq 1 ]; then
		if [ x"${current_kernel}" = x"kernel1" ]; then
			other_kernel=kernel2
		elif [ x"${current_kernel}" = x"kernel2" ]; then
			other_kernel=kernel1
		else
			if [ x"${curr_area}" != xcurr_area ]; then
				echo "File grubenv variable exception,it is recommended to reboot the system ..."
				exit 1
			elif [ x"${curr_area_value}" = x1 ]; then
				other_kernel=kernel2
			elif [ x"${curr_area_value}" = x2 ]; then
				other_kernel=kernel1
			else
				echo "Grubenv variable(curr_area=${curr_area_value})error..."
				echo "It is recommended to reboot the system..."
				exit 2
			fi
		fi
	fi
	echo ${other_kernel}
}

change_grubenv()
{
	other_kernel=`get_other_boot`
	other_appfs=`get_other_appfs_dev`
	grub-editenv /etc/cfg/grubcfg/grubenv set boot_stat=0
	grub-editenv /etc/cfg/grubcfg/grubenv set current_kernel=${other_kernel}
	grub-editenv /etc/cfg/grubcfg/grubenv set current_appfs=${other_appfs}
}

copy_file(){
	src_path=$1
	drc_path=$2

	if [ -f ${src_path} -a -d ${drc_path} ];then
		cp -af ${src_path} ${drc_path}
	fi
}

backup_tools(){
	tools_dir=/gateway/my_tools
	backup_dir=/data/info

	copy_file ${tools_dir}/switch_sys.sh  ${backup_dir}
	copy_file ${tools_dir}/appfs_to_ext4.sh ${backup_dir}
}

main()
{
	if [ -z $curr_version ]; then
		echo "current version is (${curr_version})."
		exit 1
	fi

	is_same_fs
	same_flag=$?
	if [ $same_flag -eq 1 ]; then
		change_grubenv
	else
		bak_config
		restore_file
		backup_tools
	fi
}

main $*

