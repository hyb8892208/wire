版本变更：
0.03版本：使用stl vector/string替代了tinystl和cbstring,因此链接需要libstdc++和libsupc++;
          删除了原来源代码中的TinySTL目录，以及项目使用的CB开头的CBString类库源码
          加入了ini parser源码，替代了glib的gkeyfile，不再依赖glib
          新的ini parser比较简单，:/#均作为特殊字符，不能读入，所以在配制文件里面增加了三个项目，分别用来表示是否把;/#作为黑名单
          blackslash=1            表示/在黑名单内
		  blacksemicolon=1		  表示;在黑名单内
		  blackpound=1			  表示#在黑名单内
          如果gwping发送的命令不合法，则会返回数据包防止gwping长期等待，以前的版本不返回任何东西
          makefile还是不能够自动检查依赖关系，下一版本修改。

ethservice 主要功能

编程语言：c++
依赖库 TINYSTL: 用来替代标准C++的STL,减小体积 使用了其中的vector。如果将来系统使用了libstdc++，则可以用STL的vector替代
                源代码已经包含在项目中
依赖库 libuuid: 需要安装libuuid-devel
依赖库 CBString: 一个类似C++ STL 的string类，代码已经包含在项目中

链接：使用gcc而不是g++编译，需要链接libsupc++，但不需要libstdc++

编译方法： make clean
			make
			Makefile写的比较烂，每次都要make clean之后make，不会自动检查依赖关系，你们有空重写。

基本原理：使用特殊以太网帧类型0xd01d作为网络包类别。
		  client 广播特定格式的帧到mac ff:ff:ff:ff:ff:ff。
		  server检查到此帧后，直接回复配置文件内的serverinfo段的内容
		  客户向特定的server(指定mac地址，不是广播)可以发送以[general]中allow所列的命令开头的命令之一，
		  server执行此命令并返回结果，超过一帧长度的返回值将被丢弃。实际可返回的最大长度大概在1450byte.
		  如下例，server允许执行uname 以及ls打头的命令，因此客户端可以这么做:
		  gwping eth0 "uname -r" aabbccddeeff 
		  将在Mac地址aabbccddeeff的server上执行uname -r命令。
		  同一台机器可以同时运行client/server，但server会检查mac地址是否来自本机，来自本机的不会回复

		  写在blacklist里面的字符不能够出现在命令里，如果出现，命令会被直接丢弃无任何返回。

使用方法：tx_test, rx_test用于代码测试，不是最终发布产品
		  gwpingd : Server，常驻于Gateway的内存，相应客户端的请求
		  gwping  : client，用于查询server，向特定server发送命令
		  
		  直接运行这两个软件可以看到参数说明
		  eth_server.conf: Server端配置文件，说明在文件内，下面是一个例子
        
# comments
#
[general]
# list all the service allowed
# the server will check command if started with following string
allow=uname;ls;
#print verbose information
verbose=1
#print debug information
debug=1
#dump package received
dump=1
#dump raw package binary
dumpbin=1
#blacklist 
blacklist=*?[]-{},=$><|&();!'"
blackslash=1
blacksemicolon=1
blackpound=1


#serverinfo is the information returned to client in find stage
#each line is returned to client directly if start with $
#or a script cmd if not start with $
[serverinfo]
#eth_server name
servername=$eth_server_var OPVX GW EthServer
#eth_server version in serverver
serverver=$1.0.0
#ip address of this product
ip=ifconfig | grep "inet addr:"
#model of this product
productmodel=$VS_GSU_400
#name description of this product
productname=$OpenVox 4 port GSM Gateway
#firmware version
firmwarever=$1.2.0
#or anything else, for example boot model etc.
#for test only, a empty error item
testerror=
~
~
~
~
~
~
