
目录说明：

documentation 
    原厂EC20升级相关文档以及一份升级log。
    
drivers
    EC20升级接口需要Quectel修改后的驱动才能生成/dev/ttyUSBx升级接口。
    驱动修改查看 documentatio下的《Quectel_WCDMA&LTE_Linux_USB_Driver_User_Guide_V1.8.pdf》
    对于升级，完成文档《3.2. USB Serial Driver for UCxx/EC2x/EGxx/EP06/EM06/BG96/AG35》节即可。
    将drivers目录下的驱动拷贝到
    /lib/modules/3.16.7-ckt25/kernel/drivers/usb/serial/
    执行
    depmode
    modprobe option
    即可生成4个/dev/ttyUSBx接口，用于升级的只用第一个。注意，模块上电并且选择了升级通道才会
    生成升级接口。
    驱动编译安装查看
    drivers/readme.txt

EC20CEFDGR06A06M4G
    Quectel EC20F Revision EC20CEFDGR06A06M4G 升级固件包
    
src
    原厂EC20升级工具源码。执行make便可生成工具QFlash. 升级命令如下：
    ./QFlash -f EC20CEFDGR06A06M4G -p ttyUSB4 -m 0
