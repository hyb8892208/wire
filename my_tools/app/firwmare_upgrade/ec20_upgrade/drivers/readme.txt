编译、加载驱动方法
1, 编译：
解压内核压缩包
wg3200/modules/linux-source-3.16.tar.bz2 
到
wg3200/modules/linux-source-3.16 
进入
# cd wg3200/modules
打补丁,  将Quectel驱动修改打补丁到内核源码上
# patch  -p0 < linux-3.16_usb_dirver_with_quectel.path
说明：补丁linux-3.16_usb_dirver_with_quectel.path 原文件是
commit d86a57b9c1387b8f037066699059224090373127
wg3200/modules/linux-source-3.16.tar.bz2 解压后的内核源码.

编译驱动
进入
#cd wg3200/modules/linux-source-3.16 
执行make.
编译内核, 驱动自然也就编译了。
2，加载
将
linux-source-3.16/drivers/usb/serial/option.ko
linux-source-3.16/drivers/usb/serial/qcserial.ko
linux-source-3.16/drivers/usb/serial/usb_wwan.ko
拷贝到网关
/lib/modules/3.16.7-ckt25/kernel/drivers/usb/serial/
执行
depmode
modprobe option
加载驱动

