目录说明
	SimEmuSvr是网关端仿真程序存放目录。
	common是公共资源代码目录，被SimEmuSvr使用。
	tools是工具目录，里面包含:
		emuUpdate是emu固件升级工具
		i2c*是i2c总线控制工具，用于控制sim卡电路开关
		
介绍
	SimEmuSvr
		仿真端主程序，用于处理emu和simbank的数据交互。如：登录simbank，上报通道，建立链路，sim数据交互处理，释放链路等。
	simemusvr.conf
		simemusvr程序的配置文件
	calleventhdl
		呼叫时间处理程序，从redis里面读取呼叫事件信息，如dial, connect, hangup数据，发送给simemusvr程序
		redis里面的呼叫事件信息的监听与入库由/my_tools/目录下的lua脚本负责。
	probe_emu
		emu串口搜索和记录程序，功能和/my_tools/probe_ttyUSBx一样
	emuUpdate
		emu板卡固件升级程序
	
代码目录合并及编译
	拷贝simemusvr.32目录的文件到/my_tools/simemusvr/目录下(可根据新版拷贝到相应目录，如/my_tools/app/simemusvr，下同)
	进入simemusvr/SimEmuSvr目录，执行build.sh脚本，即可编译SimEmuSvr,calleventhdl,probe_emu程序

部署：

	把以下程序拷贝到/my_tools目录下，赋予可执行权限，并设置开机自动运行
		simemusvr，calleventhdl
	把emuUpdate和prboe_emu拷贝到/my_tools目录下
	simemusvr.conf拷贝到/etc/asterisk/目录下
		