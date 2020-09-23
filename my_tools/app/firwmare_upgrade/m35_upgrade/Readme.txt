程序功能：
	本程序主要用于升级模块固件

运行环境：
	1.Asterisk已经正常关闭
	2.rri_server和bsp_server正常运行,并且rri_server中对应通道的升级标记位已经设置为1，即升级状态
	备注：本程序运行期间不能运行asterisk
	
使用说明：
	./module_m35_upgrade channel filename,
    channel为需要升级的通道,filename为升级的固件名称(M35模块需要在文件名中增加M35标识),升级成功返回Update Success
	为方便页面显示，已将升级进度重定向到/www/chn_$channel文件中
    
目录说明：
    include:头文件目录，包含chn_upgrade.h和debug.h
    src:源文件目录，main.c为程序入口，m35_upgrade.c为M35模块升级具体实现，debug.c为程序调试接口
	
升级脚本：
    upgrade_module.sh，这个脚本会为调用module_m35_upgrade程序做环境准备，包括关闭asterisk，设置rri_server和模块MCU为升级模式等
    脚本运行方法：./upgrade_module.sh channel filename，当升级多个通道时，多个通道之间用“,”分隔

一个简单的升级脚本
---------------------------------------------------------------------------
#!/bin/sh

close_asterisk(){
	/etc/init.d/asterisk stop
}

set_upgrade_flag(){
	/my_tools/rri_cli upgrade_chn $1 1
}

upgrade(){
    close_asterisk
    set_upgrade_flag $1
	./module_m35_upgrade $1 $2
}

upgrade 1 M35F1018A.bin
--------------------------------------------------------------------------	
	
