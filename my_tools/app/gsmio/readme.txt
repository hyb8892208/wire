˵��

1. Ŀ¼���ļ�����
	czmq��zmq c���ļ�Ŀ¼��ʹ������־�������ļ�����
	gsoap��gsoap�����ļ�Ŀ¼
	lib���ͻ��˿��ļ�Ŀ¼�����û�ʹ��
	serial�����ڴ����ļ�Ŀ¼
	
	client.c���ͻ���Դ���ļ����������ɿ��ļ����ṩ�ӿ�
	gsmio.h��client.cͷ�ļ�,Ҳ���û�����libgsmio.so��ͷ�ļ�
	server.c������˳����ṩmcu��������
	gsmiosvr��������������ļ�
	gsmiocli��gsmio�ͻ��˲��Գ���
	test.c�����Դ����ļ�
	
	Makefile������ű�
	
	mod_seq�����治ͬģ�����µ硢���ػ��ͽ����ػ��Ĳ������С�

2. �ϲ�
	��Ŀ¼gsmio������my_tools/app
	
3. ����
��ϸ����ѡ�����Makefile�ļ���
3.1 �������˳���
	make server
	���������mcuhdlsvr����
	make clean_server
	�������˱���

3.2 ����ͻ��˽ӿڿ�
	make lib
	�������libĿ¼������libgsmio.so��libgsmio.a�ļ���ͬʱ���������ͷ�ļ�
	make clean_lib
	����ӿڿ����
3.3 ����ͻ��˲��Գ���
	make gsmiocli
	���������gsmiocli��ִ���ļ���

4. ��������
4.1 ��װzmq
4.2 ��������gsmiosvr��/my_tools/Ŀ¼�������ÿ����Զ����С�
	�༭/etc/inittab�ļ��������������ݣ�
	:3:respawn:/my_tools/gsmiosvr
4.3 �ӿڿ�ʹ��
	����libgsmio.so��/usr/libĿ¼
	�ӿڵ��ÿɲο����Դ����ļ�gsmiocli.c��test.c��ͷ�ļ�ֻ�����gsmio.h���ɡ�
4.4 gsmiocliʹ��
	����gsmiocli����/my_toolsĿ¼�²������ִ��Ȩ�޼���ʹ�á�
4.5 ���������ļ�
	����/etc/asterisk/gsmioĿ¼
	����gsmiosvr.conf��/etc/asterisk/gsmio/Ŀ¼��
	����mod_seq�ļ��м���������ļ���/etc/asterisk/gsmioĿ¼��
4.6 ���Դ���ʹ��
	��test.c������libĿ¼
	ִ�����gcc -o test test.c -L. -lgsmio -I../czmq/include -L../czmq/lib -lczmq
	����test(����test֮ǰ�Ȱ�gsmiosvr��������)���ɡ�
	