说明

1. 目录及文件简述
	czmq：zmq c库文件目录，使用其日志和配置文件功能
	gsoap：gsoap代码文件目录
	lib：客户端库文件目录，供用户使用
	serial：串口代码文件目录
	
	client.c：客户端源码文件，用于生成库文件，提供接口
	gsmio.h：client.c头文件,也是用户调用libgsmio.so的头文件
	server.c：服务端程序，提供mcu操作功能
	gsmiosvr：服务程序配置文件
	gsmiocli：gsmio客户端测试程序
	test.c：测试代码文件
	
	Makefile：编译脚本
	
	mod_seq：保存不同模块上下电、开关机和紧急关机的操作序列。

2. 合并
	把目录gsmio拷贝到my_tools/app
	
3. 编译
纤细编译选项参照Makefile文件。
3.1 编译服务端程序
	make server
	编译后生成mcuhdlsvr程序
	make clean_server
	清除服务端编译

3.2 编译客户端接口库
	make lib
	编译后在lib目录下生成libgsmio.so和libgsmio.a文件，同时拷贝必需的头文件
	make clean_lib
	清除接口库编译
3.3 编译客户端测试程序
	make gsmiocli
	编译后生成gsmiocli可执行文件。

4. 部署运行
4.1 安装zmq
4.2 拷贝程序gsmiosvr到/my_tools/目录，并设置开机自动运行。
	编辑/etc/inittab文件，加入如下内容：
	:3:respawn:/my_tools/gsmiosvr
4.3 接口库使用
	拷贝libgsmio.so到/usr/lib目录
	接口调用可参考测试代码文件gsmiocli.c或test.c，头文件只需包含gsmio.h即可。
4.4 gsmiocli使用
	拷贝gsmiocli程序到/my_tools目录下并赋予可执行权限即可使用。
4.5 拷贝配置文件
	创建/etc/asterisk/gsmio目录
	拷贝gsmiosvr.conf到/etc/asterisk/gsmio/目录下
	拷贝mod_seq文件夹及其里面的文件到/etc/asterisk/gsmio目录下
4.6 测试代码使用
	把test.c拷贝到lib目录
	执行命令：gcc -o test test.c -L. -lgsmio -I../czmq/include -L../czmq/lib -lczmq
	运行test(运行test之前先把gsmiosvr运行起来)即可。
	