目录文件说明：
	script               -- 包含上网拨号脚本opvx-ppp.sh,拨号退出脚本opvx-ppp-kill，udev规则脚本gen_internet_rules.sh
	chat                 -- 网络连接工具chat
	curl-curl-7_38_0     -- 解压curl-curl-7_38_0.zip生成，开源上网工具
	edit_value           -- 编辑文件工具edit_key_value_tool(统计并更新已使用流量)，在libacfa.a的基础编译。
	Makefile             -- 编译makefile
	release              -- 安装路径，编译时生成

上网方式说明：
	本程序采用ppp拨号方式上网
	相关路径说明：opvx-ppp.sh中，opvx-ppp-kill 和 curl默认路径为/my_tools,chat为shell调用(放入到/usr/bin目录)
	步骤说明：1.运行gen_internet_rules.sh脚本，生成上网udev规则
			  2.修改/etc/asterisk/internet.conf配置文件，配置上网功能
	          3.运行opvx-ppp.sh脚本，获取到的信息保存到/tmp/internet$channel.log中
	环境说明：bsp_server正常运行，SIM卡能够正常注册

