#include "../gsoap/soapwebproxyService.h"
#include "../gsoap/webproxy.nsmap"

#include "../../SimEmuSvr.h"
#include "../../../common/zprint.h"

extern int apdu_switch;
extern int log_class;
extern ttyUSBx_t g_ttyUSBx;
extern register_stat_t gRegisterStat;

int http_get(struct soap *soap)  
{      
	extern unsigned char gsoap_webproxy_wsdl[];
	extern unsigned int gsoap_webproxy_wsdl_len;

	soap->http_content = "text/xml";  
	soap_response(soap,SOAP_FILE);	
	soap_send_raw(soap, (const char *)gsoap_webproxy_wsdl, gsoap_webproxy_wsdl_len);
    soap_end_send(soap);  
	
    return SOAP_OK;  
}  

void *emu_soapservice(void * param)
{
	webproxyService service;
	int *port = (int *)param;
	service.fget = http_get;
	soap_set_mode(&service,SOAP_C_UTFSTRING); //设置UTF-8格式，不然无法支持中文内容
	service.bind_flags |= SO_REUSEADDR;
	zprintf(INFO,"emu_soapservice create success");
	
	while(1)
	{
		if(service.run(*port))
		{
			zprintf(ERROR,"%s",strerror(errno));
			sleep(10);
		}
	}

	return NULL;
}


int webproxyService::EMULogSetting(int logclass,int comlogswitch,int *result)
{
	apdu_switch=comlogswitch;
	log_class = logclass;
	*result = logclass;
	zprintf(INFO,"Log class = %d,apdu switch to %d",logclass,comlogswitch);
	return 0;
}

extern commhdltask_param_t gCommHdlTaskParam[MAX_BOARD];

int webproxyService::EMUGetVersion(int EMU_No,char **version)
{
	*version =  (char*)soap_malloc(this,1024);
	getSTM32Version(gCommHdlTaskParam[EMU_No].handle,EMU_No,*version,1024);
	zprintf(INFO,"%s",*version);
	return SOAP_OK;
}

int webproxyService::EMUResetSTM32(int EMU_No,int *result)
{
	zprintf(INFO,"EMU_No = %x,handle=%x",EMU_No,gCommHdlTaskParam);
	*result = resetSTM32(gCommHdlTaskParam[EMU_No].handle,EMU_No);
	return SOAP_OK;
}


int webproxyService::EMUGetMini52Version(int EMU_No,char **version)
{
	*version =  (char*)soap_malloc(this,1024);
	memset(*version,0,1024);
	getAllMini52Version(gCommHdlTaskParam[EMU_No].handle,EMU_No,*version);
	return SOAP_OK;
}


int webproxyService::EMUResetMini52(int EMU_No,int *result)
{
	*result = resetAllMini52(gCommHdlTaskParam[EMU_No].handle,EMU_No);
	return SOAP_OK;
}

int webproxyService::EMUGetDeviceStatus(int EMU_No,int *result)
{
	if(g_ttyUSBx.grp[EMU_No].ttyUSB_Emu.dev[0] == '\0'){
		*result = 1;
	}else{
		*result = 0;
	}
	return SOAP_OK;
}

int webproxyService::EMUGetNetStatus(int EMU_No,int *result)
{
	*result = gRegisterStat.stat;
	return SOAP_OK;
}


