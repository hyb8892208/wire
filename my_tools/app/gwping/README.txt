�汾�����
0.03�汾��ʹ��stl vector/string�����tinystl��cbstring,���������Ҫlibstdc++��libsupc++;
          ɾ����ԭ��Դ�����е�TinySTLĿ¼���Լ���Ŀʹ�õ�CB��ͷ��CBString���Դ��
          ������ini parserԴ�룬�����glib��gkeyfile����������glib
          �µ�ini parser�Ƚϼ򵥣�:/#����Ϊ�����ַ������ܶ��룬�����������ļ�����������������Ŀ���ֱ�������ʾ�Ƿ��;/#��Ϊ������
          blackslash=1            ��ʾ/�ں�������
		  blacksemicolon=1		  ��ʾ;�ں�������
		  blackpound=1			  ��ʾ#�ں�������
          ���gwping���͵�����Ϸ�����᷵�����ݰ���ֹgwping���ڵȴ�����ǰ�İ汾�������κζ���
          makefile���ǲ��ܹ��Զ����������ϵ����һ�汾�޸ġ�

ethservice ��Ҫ����

������ԣ�c++
������ TINYSTL: ���������׼C++��STL,��С��� ʹ�������е�vector���������ϵͳʹ����libstdc++���������STL��vector���
                Դ�����Ѿ���������Ŀ��
������ libuuid: ��Ҫ��װlibuuid-devel
������ CBString: һ������C++ STL ��string�࣬�����Ѿ���������Ŀ��

���ӣ�ʹ��gcc������g++���룬��Ҫ����libsupc++��������Ҫlibstdc++

���뷽���� make clean
			make
			Makefileд�ıȽ��ã�ÿ�ζ�Ҫmake clean֮��make�������Զ����������ϵ�������п���д��

����ԭ��ʹ��������̫��֡����0xd01d��Ϊ��������
		  client �㲥�ض���ʽ��֡��mac ff:ff:ff:ff:ff:ff��
		  server��鵽��֡��ֱ�ӻظ������ļ��ڵ�serverinfo�ε�����
		  �ͻ����ض���server(ָ��mac��ַ�����ǹ㲥)���Է�����[general]��allow���е����ͷ������֮һ��
		  serverִ�д�������ؽ��������һ֡���ȵķ���ֵ����������ʵ�ʿɷ��ص���󳤶ȴ����1450byte.
		  ��������server����ִ��uname �Լ�ls��ͷ�������˿ͻ��˿�����ô��:
		  gwping eth0 "uname -r" aabbccddeeff 
		  ����Mac��ַaabbccddeeff��server��ִ��uname -r���
		  ͬһ̨��������ͬʱ����client/server����server����mac��ַ�Ƿ����Ա��������Ա����Ĳ���ظ�

		  д��blacklist������ַ����ܹ������������������֣�����ᱻֱ�Ӷ������κη��ء�

ʹ�÷�����tx_test, rx_test���ڴ�����ԣ��������շ�����Ʒ
		  gwpingd : Server����פ��Gateway���ڴ棬��Ӧ�ͻ��˵�����
		  gwping  : client�����ڲ�ѯserver�����ض�server��������
		  
		  ֱ������������������Կ�������˵��
		  eth_server.conf: Server�������ļ���˵�����ļ��ڣ�������һ������
        
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
