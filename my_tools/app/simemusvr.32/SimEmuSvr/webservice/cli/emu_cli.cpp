#include "../gsoap/soapwebproxyProxy.h"
#include "../gsoap/webproxy.nsmap"

#include <iostream>

using namespace std;

void emu_cli_usage(char *cli_name)
{
	cout << cli_name << " [device]" << " [option]" <<" [emu number]" << endl;
	cout <<"device:stm32,mini52,net\n" << "option:reset,getversion,status\n" << "emu number:0-3"<< endl; 
}

void emu_operator(int argc,char **argv)
{
	const char server[] = "http://127.0.0.1:8868";
	webproxyProxy proxy;
	proxy.soap_endpoint = server;
	int result = -1;
	char version[1024];
	char *p = version;
	int ret = 0;
	int emu_no = atoi(argv[3]);

	if(0 > emu_no || 3 < emu_no){
		cout << "invalid emu number:"<<emu_no << endl;
		return ;
	}
	
	if(0 == strcasecmp(argv[1],"stm32") )
	{
		
		if(0 == strcasecmp(argv[2],"reset")){
			ret = proxy.EMUResetSTM32(emu_no,&result);
			cout << "reset stm32:" << result << endl;
		}else if(0 == strcasecmp(argv[2],"getversion")){
			ret = proxy.EMUGetVersion(emu_no,&p);
			cout << "stm32 version:"<<p << endl;
		}else if(0 == strcasecmp(argv[2],"status")){
			ret = proxy.EMUGetDeviceStatus(emu_no,&result);
			cout << "emu status:" << result << endl;	// 0 found device; 1: not found
		}
		else{
			cout << "unknown command:" << argv[2] << endl;
		}
	}
	else if( 0 == strcasecmp(argv[1],"mini52"))
	{
		if(0 == strcasecmp(argv[2],"reset"))
		{
			ret = proxy.EMUResetMini52(emu_no,&result);
			cout << "reset mini52:" << result << endl;
		}
		else if( 0 == strcasecmp(argv[2],"getversion")){
			ret = proxy.EMUGetMini52Version(emu_no,&p);
			cout << "mini52 version:"<<endl <<p << endl;
		}else{
			cout << "unknown command:" << argv[2] << endl;
		}
	}
	else if( 0 == strcasecmp(argv[1],"net"))
	{
		if(0 == strcasecmp(argv[2],"status")){
					ret = proxy.EMUGetNetStatus(emu_no,&result);
					cout << "server connect status:" << result << endl; // 0:registed; 1:unregister
		}else{
			cout << "unknown command:" << argv[2] << endl;
		}
	}

	if(SOAP_OK != ret){
		cout << "connect " <<server<<" fail !" << endl;
	}
	
}

int main(int argc,char **argv)
{
	if(argc != 4)
	{
		emu_cli_usage(argv[0]);
		return -1;
	}

	emu_operator(argc,argv);
}

