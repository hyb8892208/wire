documentation:
	原厂提供的升级说明文档和升级log

src：
	原厂提供的升级工具QDL_LINUX_SIM6320_V2.tar.gz解压后得到
	直接在src目录下执行make，就可以生成QDL升级程序

使用说明：
	1.安装ec20模块的usb驱动补丁。
	2.通过bsp_server切换升级通道到指定模块
	3.修改os_linux.cpp 167行中tmp_port变量的值为我们的升级端口号，重新编译代码
	4.执行命令为./QDL --path FirmawrePath

