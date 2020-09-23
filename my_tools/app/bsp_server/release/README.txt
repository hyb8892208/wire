1.bsp_cli和reg_operation简介：
    bsp_cli主要用于测试bsp_server提供的操作硬件的接口,reg_operation是针对具体模块的串口工具
2.bsp_cli测试方法：
	bsp_cli 提供两种测试模式，交互式和命令行方式。交互式使用比较简单，直接运行应用程序即可，以下主要讲述命令行方式测试方法：
	bsp_cli主要由sim-card,brdmcu,module和upgrade四个部分组成
	1)sim-card:
	    a)sim-card 使能/失能测试
		./bsp_cli sim [enable|disable] [chn],其中chn为具体的通道号,-1时控制所有通道
		b)sim-card 使能/插入状态检测
		./bsp_cli sim_state [enable|insert] [chn],其中chn为具体的通道号,-1时控制所有通道
		c)sim 事件检测，包括INSERT、REMOVE、NO三种状态
		./bsp_cli sim event [chn],其中chn为需要检测的具体的通道号,-1时控制所有通道
	2)module:
		a)module 数量获取：
		./bsp_cli module num
		b)module uid获取
		./bsp_cli module uid [index],其中index为模块编号
		c)module 复位键状态检测
		./bsp_cli module reset [index],其中index为模块编号
		d)module 开/关测试
		./bsp_cli module turn [on|off] [chn],其中chn为通道号,-1时控制所有通道
		e)module 上电/下电测试
		./bsp_cli module power [on|off] [chn],其中chn为通道号,-1时控制所有通道
		f)module 电源状态检测
		./bsp_cli module_state power [chn],其中chn为通道号,-1时控制所有通道
		g)module 电源开关状态检测
		./bsp_cli module_state turn [chn],其中chn为通道号,-1时控制所有通道
		h)module 模块板uart调试开关
		./bsp_cli module uart [on|off] [index],其中chn为模块板号,-1时控制所有通道
	3)board mcu:
		a)获取设备信息名称及版本
	    ./bsp_cli bmcu info
		b)获取board mcu 模块版本信息
		./bsp_cli bmcu ver [idx],其中index为模块编号
		b)获取board mcu 读寄存器
		./bsp_cli bmcu reg read  [brd] [reg] [num], brd为模块编号，reg为起始地址，num为寄存器偏移地址
		b)获取board mcu 写寄存器
		./bsp_cli bmcu reg read  [brd] [reg] [value], brd为模块编号，reg为起始地址，value为寄存器值
	4)upgrade:
		a)选择升级通道
		./bsp_cli upgrade select [chn],其中chn为通道号,chn为0xFFFF(65535)时清除已选择升级通道
		b)通道升级状态查询
		./bsp_cli upgrade state [chn],其中chn为通道号
	5)led:
		./bsp_cli led [name] [status] [channel], name为sig或者work, status为red,red_flash,green,green_flash,off， channel指具体的通道号
		./bsp_cli led all [status], status为on或者off, 打开或关闭所有通道的sig和work灯

3.reg_operation使用方法
   reg_operation按照VS_USB_M35读写寄存器方法设计，同样提供命令行的方式和交互模式
    1)寄存器读操作
      ./reg_operation [device] read <reg>
    2)寄存器读位操作
      ./reg_operation [device] read <reg>.<bit>
    3)寄存器读连续寄存器操作
      ./reg_operation [device] read <reg>-<num>，其中num为2位的十六进制，不足补0，最后带h
    4.寄存器写操作
      ./reg_operation [device] read <reg>=<value>, 其中value为2位的十六进制，不足补0，最后带h
    5.寄存器写位操作
      ./reg_operation [device] read <reg>.<bit>=<value>，其中value为十进制
    6.获取版本信息
      ./reg_operation [device] ver

