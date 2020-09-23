1.rri_cli简介：
    rri_cli主要用于测试模块基本功能测试
2.rri_cli测试方法：
	rri_cli 提供两种测试模式，交互式和命令行方式。交互式使用比较简单，直接运行应用程序即可，以下主要讲述命令行方式测试方法：
	rri_cli主要由四个部分组成
	1)服务端信息获取和设置:
	    a)rri_server版本获取
		./rri_cli server ver
		b)rri_server调试模式设置
        ./rri_cli server debug [n],其中n为0时关闭调试模式，大于0开启调试模式
	2)通道相关:
		a)通道数量获取：
		./rri_cli chn_num get
		b)通道管道名称等信息获取
		./rri_cli chn info [chn],其中[chn]为通道编号
		c)通道调试at/cmd模式设置
		./rri_cli chn_at debug [chn] [value],其中[chn]为通道标号,[value]为0时关闭调试模式，大于0开启调试模式
        d)通道调试语音模式设置
		./rri_cli chn_snd debug [chn] [value],其中[chn]为通道标号,[value]为0时关闭调试模式，大于0开启调试模式
		e)通道串口重新打开
		./rri_cli chn com reopen [chn],其中chn为通道号
		f)通道at端口信息获取
		./rri_cli at_port info [chn],其中chn为通道号
		g)通道调试信息获取
		./rri_cli debug_port info [chn],其中chn为通道号
		h)升级通道信息获取
		./rri_cli upgrade_port info [chn],其中chn为通道号
        i)连接状态获取，主要是语音状态，短信状态，调试状态，升级状态获取
        ./rri_cli conn state [chn],其中chn为通道号
        j)通道升级模式设置
        ./rri_cli upgrade_chn [chn] [value],其中[chn]为通道号，[value]为0关闭升级模式，为1时打开升级模式
		k)通道版本信息获取
		./rri_cli module version [chn], chn为通道号
		l)单片机buf状态获取
		./rri_cli module track [chn], chn为通道号
	3)音频相关
		a)获取音频传输格式
	    ./rri_cli audio format [chn],其中chn为通道号
		b)开启/关闭音频传输
		./rri_cli audio transmit [chn] [start|stop],其中chn为通道号
		c)设置音频发送缓冲区大小
		./rri_cli chn_snd bufsize set [chn] [bufsize],chn为通道号，bufsize语音发送缓冲区大小，必须为2的n次方
		d)获取音频发送缓冲区大小
		./rri_cli chn_snd bufsize get [chn],chn为通道号
		e)设置音频传输包间隔
		./rri_cli chn_snd speed set [chn] [speed],chn为通道号，speed为发送时间间隔，速度为[speed]*8/[speed]ms
		f)获取音频传输包间隔
		./rri_cli chn_snd speed get [chn],chn为通道号
		g)设置第一个包发送延时
		./rri_cli chn_snd delay set [chn] [delay],chn为通道号，delay为延时时间
		h)获取第一个包发送延时
		./rri_cli chn_snd delay get [chn]，chn为通道号

	4)at指令:
		a)向模块发起at指令
	    ./rri_cli at [chn] [command],其中chn为通道号,command为具体的AT指令
    5)gsoap性能测试
        ./rri_cli gsoap test [cnt],其中cnt为具体的测试次数
    6)模块mcu版本信息获取
        ./rri_cli module version [chn],[chn]为具体的通道号
		
