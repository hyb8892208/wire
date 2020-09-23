
#include <pthread.h>

#include "public.h"
#include "soapH.h"
#include "ns.nsmap"
#include "rri_api.h"


/* ����֧�ֵ����ͨ���� */
#define BOARD_MAX_CHN			        (32)

pthread_mutex_t g_rri_api_lock;

#if T_DESC("rri api init")

/* soap handle */
struct soap *g_soap_rri_hdl = NULL;
char g_soap_rri_url[256];

/*
desc      : api init, connect to soap server
param in  : host -- soap server ip address
            port -- soap server port
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int rri_api_init(char *host, unsigned short port)
{
    char dst_host[256] = {0};
    unsigned short dst_port = 8810;
    
	if (host == NULL)
		strcpy(dst_host, "127.0.0.1");
    else
        strcpy(dst_host, host);

	if (port != 0)
		dst_port = port;

    memset(g_soap_rri_url, 0, sizeof(g_soap_rri_url));
	sprintf(g_soap_rri_url, "http://%s:%d", dst_host, dst_port);

    if ( g_soap_rri_hdl != NULL )
    {
        soap_destroy(g_soap_rri_hdl);
        soap_end(g_soap_rri_hdl);
        soap_free(g_soap_rri_hdl);
        g_soap_rri_hdl = NULL;
    }

	g_soap_rri_hdl = soap_malloc(g_soap_rri_hdl, sizeof(struct soap));
	if (g_soap_rri_hdl == NULL)
		return RET_ERROR;
	
	soap_init1(g_soap_rri_hdl, SOAP_XML_INDENT);

    pthread_mutex_init(&g_rri_api_lock, NULL);

	return RET_OK;
}

/*
desc      : rri deinit, disconnect from soap server
param in  : 
param out : 
return    : 
history   : create by zhongwei.peng/2017.11.24
*/
void rri_api_deinit(void)
{
    pthread_mutex_destroy(&g_rri_api_lock);
    
	if ( g_soap_rri_hdl != NULL )
    {
        soap_destroy(g_soap_rri_hdl);
        soap_end(g_soap_rri_hdl);
        soap_free(g_soap_rri_hdl);
        g_soap_rri_hdl = NULL;
    }
}

#endif

/*
    ��ȡServer�İ汾��
*/
int GetServerVersion(struct rri_server_version_t *sv)
{
    int ret = 0;
    int len;
    struct server_version_t ver;

    if ( sv == NULL )
        return RET_ERROR;

    memset(&ver, 0, sizeof(ver));
    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetServerVersion(g_soap_rri_hdl, g_soap_rri_url, "", &ver);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) && (ver.nName != NULL) )
    {
        len = strlen(ver.nName);
        if ( (len > 0) && (len < sizeof(sv->nName)) )
            strcpy(sv->nName, ver.nName);

        sv->bugfixNumber = ver.bugfixNumber;
        sv->buildNumber = ver.buildNumber;
        sv->majorVersion = ver.majorVersion;
        sv->minorVersion = ver.minorVersion;
    }

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock); 

    return ret;
}

/*
    ��ȡͨ������
*/
int GetChannelCount(int* nChannels)
{
    int ret = 0;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetChannelCount(g_soap_rri_hdl, g_soap_rri_url, "", nChannels);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ��ȡͨ��������Ϣ
*/
int GetChannelInfo(int nCh, struct rri_channel_info_t *info)
{
    int ret = RET_OK;
    struct channel_info_t cinfo;

    memset(&cinfo, 0, sizeof(cinfo));
    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetChannelInfo(g_soap_rri_hdl, g_soap_rri_url, "", nCh, &cinfo);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) && (cinfo.haveDebugPort == 1) )
    {
        if ( cinfo.rf_module_model_name != NULL )
            strcpy(info->rf_module_model_name, cinfo.rf_module_model_name);
        if ( cinfo.rf_module_hw_version != NULL )
            strcpy(info->rf_module_hw_version, cinfo.rf_module_hw_version);
        if ( cinfo.rf_module_fw_version != NULL )
            strcpy(info->rf_module_fw_version, cinfo.rf_module_fw_version);
        if ( cinfo.rf_module_manufacturer != NULL )
            strcpy(info->rf_module_manufacturer, cinfo.rf_module_manufacturer);
        if ( cinfo.audioEndpointName_r != NULL )
            strcpy(info->audioEndpointName_r, cinfo.audioEndpointName_r);
        if ( cinfo.audioEndpointName_w != NULL )
            strcpy(info->audioEndpointName_w, cinfo.audioEndpointName_w);
        if ( cinfo.atEndpointName_r != NULL )
            strcpy(info->atEndpointName_r, cinfo.atEndpointName_r);
        if ( cinfo.atEndpointName_w != NULL )
            strcpy(info->atEndpointName_w, cinfo.atEndpointName_w);
        if ( cinfo.DebugEndpointName_r != NULL )
            strcpy(info->DebugEndpointName_r, cinfo.DebugEndpointName_r);
        if ( cinfo.DebugEndpointName_w != NULL )
            strcpy(info->DebugEndpointName_w, cinfo.DebugEndpointName_w);
        if ( cinfo.UpgradeEndpointName_r != NULL )
            strcpy(info->UpgradeEndpointName_r, cinfo.UpgradeEndpointName_r);
        if ( cinfo.UpgradeEndpointName_w != NULL )
            strcpy(info->UpgradeEndpointName_w, cinfo.UpgradeEndpointName_w);

        info->haveDebugPort = cinfo.haveDebugPort;
        info->haveUpgradePort = cinfo.haveUpgradePort;            
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ��ȡͨ����Ƶ����
*/
int GetChannelAudioFromat(int nCh, struct rri_voice_attri_t *va)
{
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetChannelAudioFromat(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct voice_attri_t *)va);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ��ȡatͨ������
    baudrateΪ�����ʣ����������server�Ĳ����ʣ����Է���0
    XON_XOFF��ʾ�Ƿ���Ҫ��XON_XOFF���أ�0��ʾ����Ҫ�� 1��ʾ��Ҫ
*/
int GetAtPortInfo(int nCh, struct rri_at_attri_t *attri)
{
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetAtPortInfo(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct at_attri_t *)attri);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}
    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ��ȡDebugͨ������
*/
int GetDebugPortInfo(int nCh, struct rri_at_attri_t *attri)
{
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetDebugPortInfo(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct at_attri_t *)attri);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ��ȡUpgradeͨ������
*/
int GetUpgradePortInfo(int nCh, struct rri_at_attri_t *attri)
{
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetUpgradePortInfo(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct at_attri_t *)attri);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ��ȡͨ������״̬
    ����ֵ0��ʾͨ��Ϊfree, 1��ʾͨ���Ѿ������ӡ������Ҫ����״̬��������չ
*/
int GetChannelConnectionState(int nCh, struct rri_chn_conn_state_t* state)
{
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetChannelConnectionState(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct chn_conn_state_t *)state);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ��ʼ/ֹͣ��������
*/
int SetAudioTransmitState(int nCh, int newState)
{
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__SetAudioTransmitState(g_soap_rri_hdl, g_soap_rri_url, "", nCh, newState, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ���� server debug ����
*/
int SetServerDebug(int nVal)
{
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__SetServerDebug(g_soap_rri_hdl, g_soap_rri_url, "", nVal, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}

/*
    ���� ͨ�� debug ����
*/
int SetChannelDebug(int nCh, int nVal)
{
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__SetChannelDebug(g_soap_rri_hdl, g_soap_rri_url, "", nCh, nVal, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}
/*
    ���� ͨ�� ����debug ����
*/
int SetChannelSndDebug(int nCh, int nVal){
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__SetChannelSndDebug(g_soap_rri_hdl, g_soap_rri_url, "", nCh, nVal, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}
/*
    ���´򿪴���
*/
int ReopenSerial(int nCh)
{
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__ReopenSerial(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}

    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);

    return ret;
}
/*����ͨ����������*/
int SetChannelUpgrade(int nCh, int state){
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__SetChannelUpgrade(g_soap_rri_hdl, g_soap_rri_url, "", nCh, state, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}
    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);
    return rsp.result;
	
}


int SetChannelTxSndSpeed(int nCh, int speed){
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__SetChannelTxSndSpeed(g_soap_rri_hdl, g_soap_rri_url, "", nCh, speed, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}else
	    ret = rsp.result;
    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);
    return rsp.result;
	
}

int GetChannelTxSndSpeed(int nCh, int *speed){
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetChannelTxSndSpeed(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}
    if(rsp.result == 0 && rsp.value != NULL){
        *speed = atoi(rsp.value);
        ret = RET_OK;
     }else
        ret = rsp.result;
    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);
    return ret;
	
}


int SetChannelTxSndBufSize(int nCh, int bufsize){
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__SetChannelTxSndBufSize(g_soap_rri_hdl, g_soap_rri_url, "", nCh, bufsize, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}else
	    ret = rsp.result;
    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);
    return ret;
}

int GetChannelTxSndBufSize(int nCh, int *bufsize){
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetChannelTxSndBufSize(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}
    if(rsp.result == 0 && rsp.value != NULL){
        *bufsize = atoi(rsp.value);
        ret = RET_OK;
     }else
        ret = rsp.result;
    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);
    return ret;
	
}

int SetChannelTxSndDelay(int nCh, int delay){
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__SetChannelTxSndDelay(g_soap_rri_hdl, g_soap_rri_url, "", nCh, delay, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}else
	    ret = rsp.result;
    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);
    return ret;
}

int GetChannelTxSndDelay(int nCh, int *delay){
    struct rri_gsoap_api_rsp_t rsp;
    int ret = RET_OK;

    pthread_mutex_lock(&g_rri_api_lock);
    ret = soap_call_ns__GetChannelTxSndDelay(g_soap_rri_hdl, g_soap_rri_url, "", nCh, (struct ns__gsoap_api_rsp_t *)&rsp);
    if ( g_soap_rri_hdl->error )
	{
		soap_print_fault(g_soap_rri_hdl, stderr);
		ret = RET_ERROR;
	}
    if(rsp.result == 0 && rsp.value != NULL){
        *delay = atoi(rsp.value);
        ret = RET_OK;
     }else
        ret = rsp.result;
    soap_destroy(g_soap_rri_hdl);
    soap_end(g_soap_rri_hdl);
    pthread_mutex_unlock(&g_rri_api_lock);
    return ret;
	
}


