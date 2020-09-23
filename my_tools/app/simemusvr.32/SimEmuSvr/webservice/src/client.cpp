#include "../gsoap/soapwebproxyProxy.h"
#include "../gsoap/webproxy.nsmap"
#include <stdio.h>
#include<string.h>
const char server[] = "http://127.0.0.1:8888";



int main(int argc,char **argv)
{
	printf("%s\n",argv[0]);
	webproxyProxy proxy;

	char *seri = "gateway001";
	char *num = "10086";
	char *num1 = "10010";
	char *msg = "bj";
	char *str = "phone";
	struct smssendreqinfoh buff;
	memset(&buff,0,sizeof(buff));
	buff.smstype = 1;
	buff.sbbanknbr=2;
	buff.sbslotnbr=3;
	buff.chnnbr=4;
	buff.gwbanknbr=5;
	buff.gwslotnbr=6;
	
	buff.dstnum=num;
	buff.srcnum=num1;
	buff.sendmsg=msg;
	buff.matchstr=str;
	buff.seri = seri;
	
	int result = 0;
	proxy.soap_endpoint = server;
	proxy.SBKSMSReq(buff,&result);
	printf("%d\n",result);
	return 0;

}



