#!/bin/sh

curr_area_value=`cat /etc/cfg/grubcfg/grubenv | grep curr_area |  cut -d "=" -f 2`
upgrade_appfs=

check_mount(){
	dev_mmc=$1
	mount_point=$2
	if mount | grep ${dev_mmc} ; then
		MNT_POINT=`mount | grep ${dev_mmc} | awk '{print $3}'`
		if [ x"${MNT_POINT}" = x"${mount_point}" ];then
			echo "parition $dev_mmc mount at ${mount_point}, error)..."
			exit 0
		else
			umount ${MNT_POINT}
		fi
	fi
}

get_upgrade_appfs_dev()
{

	if [ x"${curr_area_value}" = x1 ]; then
		upgrade_appfs=mmcblk0p5
	else
		upgrade_appfs=mmcblk0p4
	fi
	check_mount /dev/${upgrade_appfs} /gateway
}

make_appfs_to_ext4(){
	update_fs_dev=${upgrade_appfs}
	echo -e "y\n" | mkfs.ext4 /dev/${update_fs_dev}
}

get_upgrade_appfs_dev

make_appfs_to_ext4
