#!/bin/bash

RRI_CLI=/my_tools/rri_cli
UPGRADE_PROCESS=/my_tools/module_m35_upgrade
#彻底关闭asterisk
close_asterisk(){
    /etc/init.d/asterisk stop
}

#设置rri_server和模块mcu升级标记位
set_upgrade_flag(){
   /my_tools/rri_cli upgrade_chn $1 1 > /www/upgrade_module_$1.log
}

#获取模块版本信息
get_module_version(){
   /my_tools/rri_cli at $1 ati >/www/upgrade_module_$1.log
}

#升级模块
upgrade(){
    close_asterisk
    temp=${1//,/ }
    for i in $temp
    do
        set_upgrade_flag $i
        /my_tools/module_m35_upgrade $i $2 >> /www/upgrade_module_$i.log &
    done
}

if [ $# != 2 ]
then
    echo "param error"
    exit 1
fi
upgrade $1 $2
