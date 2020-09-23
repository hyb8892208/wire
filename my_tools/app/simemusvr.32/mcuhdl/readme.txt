说明

1. 目录及文件简述
	czmq：zmq c库文件目录，使用其日志和配置文件功能
	gsoap：gsoap代码文件目录
	lib：客户端库文件目录，供用户使用
	serial：串口代码文件目录
	
	client.c：客户端源码文件，用于生成库文件，提供接口
	mcuhdl.h：client.c头文件
	server.c：服务端程序，提供mcu操作功能
	mcuhdlsvr：服务程序配置文件
	test.c：测试代码文件
	
	Makefile：编译脚本

2. 合并
	把目录mcuhdl拷贝到my_tools/app
	
3. 编译
3.1 编译服务端程序
	make server
	编译后生成mcuhdlsvr程序
	make clean_server
	清除服务端编译

3.2 编译客户端接口库
	make lib
	编译后在lib目录下生成libmcuhdl.so和libmcuhdl.a文件，同时拷贝必需的头文件
	make clean_lib
	清除接口库编译

4. 部署运行
4.1 安装zmq
4.2 拷贝程序mcuhdlsvr到/my_tools/目录，并设置开机自动运行。
	编辑/etc/inittab文件，加入如下内容：
	:3:respawn:/my_tools/mcuhdlsvr
4.3 接口库使用
	拷贝libmcuhdl.so到/usr/lib目录
	接口调用可参考测试代码文件test.c，头文件只需包含mcuhdl.h即可。

	编译命令： gcc -o test test.c -L. -lmcuhdl -I../czmq/include -L../czmq/lib -lczmq



