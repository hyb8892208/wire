/*
    gsoap server 主入口
    api 实现文件
    bsp 状态维护
*/
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "soapH.h"
#include "ns.nsmap"

#include "public.h"
#include "czmq.h"

//#include "gsoap_api.h"
#include "rri_server.h"

#if T_DESC("soap server")

typedef struct queue_handle_param_s
{
	int nbr;
    int thd_flag;      // 线程运行标志，=1运行中，=0已退出
    pthread_attr_t attr;  // 线程参数
	struct soap *psoap;
}queue_handle_param_t;

pthread_mutex_t queue_lock;		// 队列锁
pthread_cond_t  queue_cond;		// 队列条件变量
SOAP_SOCKET queue[MAX_QUEUE];	// 数组队列
int g_queue_head = 0;			// 队列头
int g_queue_tail = 0;			// 队列尾

struct soap *server_soap = NULL;
zpoller_t *poller = NULL;
int       g_soap_server_run_flag; // server 线程运行标志位
pthread_attr_t g_soap_server_attr; // 线程参数
pthread_t g_soap_server_tid;      // server 线程id
pthread_t g_queue_tid[MAX_THR];   // 处理调用队列线程id
queue_handle_param_t hdl_params[MAX_THR];
struct soap *soap_thr[MAX_THR];

#define HOST_IP "127.0.0.1"

int soap_enqueue(SOAP_SOCKET sock)
{
	int status = SOAP_OK;
	int next = 0;
	//zsys_debug("enqueue: Starting[head:%03d-tail:%03d] ...", g_queue_head, g_queue_tail);
	pthread_mutex_lock(&queue_lock);
	next = g_queue_tail + 1;
	if (next >= MAX_QUEUE)
	{
		next = 0;
	}
	if (next == g_queue_head)
	{
		status = SOAP_EOM;
	}
	else
	{
		queue[g_queue_tail] = sock;
		g_queue_tail = next;
	}
	pthread_cond_signal(&queue_cond);
	pthread_mutex_unlock(&queue_lock);
	//zsys_debug("enqueue: done [head:%03d-tail:%03d]status:%d", g_queue_head, g_queue_tail, status);
	return status;
}

SOAP_SOCKET soap_dequeue(void)
{
	SOAP_SOCKET sock;
	//zsys_debug("dequeue: begin[head:%03d-tail:%03d]", g_queue_head, g_queue_tail);
	pthread_mutex_lock(&queue_lock);
	while (g_queue_head == g_queue_tail)
	{
		pthread_cond_wait(&queue_cond, &queue_lock);
	}
	sock = queue[g_queue_head++];
	if (g_queue_head >= MAX_QUEUE)
	{
		g_queue_head = 0;
	}
	pthread_mutex_unlock(&queue_lock);
	
	//zsys_debug("dequeue: done [head:%03d-tail:%03d]sock:%d", g_queue_head, g_queue_tail, sock);
	return sock;
}

/*
desc      : get server port
param in  : 
param out : 
return    : server port
history   : create by zhongwei.peng/2017.11.24
*/
int soap_get_port(void)
{
	int val = 8810;
	return val;
}

void * soap_queue_handle(void *arg)
{
	queue_handle_param_t *param = (queue_handle_param_t *)arg;

	zsys_info("queue_handle[%d]: Starting ...", param->nbr);

	while ( param->thd_flag )
	{
		param->psoap->socket = soap_dequeue();
		if (!soap_valid_socket(param->psoap->socket))
		{
			zsys_error("queue_handle[%d]: socket(%d) from queue not valid!!!", param->nbr, param->psoap->socket);
			break;
		}

		soap_serve(param->psoap);
		soap_destroy(param->psoap);
		soap_end(param->psoap);
	}
	zsys_info("queue_handle[%d]: End", param->nbr);
	return NULL;
}

/*
desc      : soap server
param in  : 
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
void * soap_server_run(void *param)
{
//    int i = 0;
    void *res = NULL;
    SOAP_SOCKET s = 0;

    g_soap_server_run_flag = 1;
	while ( g_soap_server_run_flag )
	{
		res = zpoller_wait(poller, 2000);
		if (res != &server_soap->master)
		{
			continue;
		}

		s = soap_accept(server_soap);
		if (!soap_valid_socket(s))
		{
			if (server_soap->errnum)
			{
				soap_print_fault(server_soap, stderr);
				continue;
			}
			else
			{
				zsys_error("Server time out!!!");
				break;
			}
		}
		/*zsys_info("Accept connection from IP = %d.%d.%d.%d, socket = %d", \
			((server_soap.ip)>>24)&0xFF, ((server_soap.ip)>>16)&0xFF, ((server_soap.ip)>>8)&0xFF, \
			(server_soap.ip)&0xFF, server_soap.socket);*/

		while (soap_enqueue(s) == SOAP_EOM)
		{
			sleep(1);
		}
	}
    
	soap_server_stop();

    return (void *)0;
}

/*
desc      : start soap server, it will create some threads
param in  : 
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int soap_server_start(void)
{
	int ret = 0;
    int i = 0;
	SOAP_SOCKET m = 0;

    if ( ((server_soap = (struct soap*)malloc(sizeof(struct soap))) == NULL) )
        return RET_ERROR;

	soap_init(server_soap);
	server_soap->bind_flags |= SO_REUSEADDR;

    poller = zpoller_new(NULL);
	if ( poller == NULL )
    {
        soap_done(server_soap);
        free(server_soap);
		return -1;
    }
    
	ret = zpoller_add(poller, (void *)&(server_soap->master));
	if ( ret < 0 )
    {
        zpoller_destroy (&poller);
        soap_done(server_soap);
        free(server_soap);
		return -1;
    }
	m = soap_bind(server_soap, HOST_IP, soap_get_port(), BACKLOG);
	if (!soap_valid_socket(m))
	{
	    zpoller_remove(poller, (void *)server_soap->master);
	    zpoller_destroy(&poller);
		soap_print_fault(server_soap, stderr);
        soap_done(server_soap);
        free(server_soap);
		return -1;
	}
	zsys_info("Socket connection successful: master socket = %d", m);

    pthread_mutex_init(&queue_lock, NULL);
	pthread_cond_init(&queue_cond, NULL);
	for (i = 0; i < MAX_THR; i++)
	{
	    pthread_attr_init(&(hdl_params[i].attr));
        pthread_attr_setinheritsched(&(hdl_params[i].attr), PTHREAD_INHERIT_SCHED);
        pthread_attr_setstacksize(&(hdl_params[i].attr), 262144); /* 堆栈大小256k */
    
		soap_thr[i] = soap_copy(server_soap);
        hdl_params[i].thd_flag = 1;
		hdl_params[i].nbr = i;
		hdl_params[i].psoap = soap_thr[i];
		pthread_create(&g_queue_tid[i], &(hdl_params[i].attr), soap_queue_handle, (void *)&hdl_params[i]);
	}

    pthread_attr_init(&g_soap_server_attr);
    pthread_attr_setinheritsched(&g_soap_server_attr, PTHREAD_INHERIT_SCHED);
    pthread_attr_setstacksize(&g_soap_server_attr, 262144); /* 堆栈大小256k */

    pthread_create(&g_soap_server_tid, &g_soap_server_attr, soap_server_run, NULL);
    
    return 0;
}

/*
desc      : stop soap server, it will kill all threads which create by soap_server_start
param in  : 
param out : 
return    : 
history   : create by zhongwei.peng/2017.11.24
*/
void soap_server_stop(void)
{
    int i;

    if ( server_soap == NULL )
        return;

    zpoller_remove(poller, (void *)server_soap->master);
	zpoller_destroy (&poller);

    for (i = 0; i < MAX_THR; i++)
        //pthread_cancel(g_queue_tid[i]);
        hdl_params[i].thd_flag = 0;

	for (i = 0; i < MAX_THR; i++)
	{
		zsys_info("Waiting for thread %d to terminate ...", i);
		pthread_join(g_queue_tid[i], NULL);
		zsys_info("Thread %d terminate", i);
		soap_done(soap_thr[i]);
		soap_free(soap_thr[i]);
	}
	pthread_mutex_destroy(&queue_lock);
	pthread_cond_destroy(&queue_cond);

	soap_done(server_soap);
    free(server_soap);
    server_soap = NULL;
}

#endif

#if T_DESC("api")

/*
    获取Server的版本号
*/
int ns__GetServerVersion(struct soap *soap, struct server_version_t *sv)
{

    if ( sv->nName == NULL )
    {
        sv->nName = soap_malloc(soap, 64);
        if ( sv->nName == NULL )
            return SOAP_ERR;
    }

    strcpy(sv->nName, "server_name_rri 2.0");
    sv->buildNumber = 1;
    sv->bugfixNumber = 2;
    sv->majorVersion = 3;
    sv->minorVersion = 4;

    return RET_OK;
}

/*
    获取通道总数
*/
int ns__GetChannelCount(struct soap *soap, int* nChannels)
{
    *nChannels = svr_get_hwport_num();
    return RET_OK;
}

/*
    获取通道基本信息
*/
int ns__GetChannelInfo(struct soap *soap, int nCh, struct channel_info_t *info)
{
    int ret;

    if ( info->rf_module_model_name == NULL )
    {
        info->rf_module_model_name = soap_malloc(soap, 128);
        info->rf_module_hw_version = soap_malloc(soap, 128);
        info->rf_module_fw_version = soap_malloc(soap, 128);
        info->rf_module_manufacturer = soap_malloc(soap, 128);
        info->audioEndpointName_r = soap_malloc(soap, 128);
        info->audioEndpointName_w = soap_malloc(soap, 128);
        info->atEndpointName_r = soap_malloc(soap, 128);
        info->atEndpointName_w = soap_malloc(soap, 128);
        info->DebugEndpointName_r = soap_malloc(soap, 128);
        info->DebugEndpointName_w = soap_malloc(soap, 128);
        info->UpgradeEndpointName_r = soap_malloc(soap, 128);
        info->UpgradeEndpointName_w = soap_malloc(soap, 128);
    }

    strcpy(info->rf_module_model_name, "model name");
    strcpy(info->rf_module_hw_version, "hw_version");
    strcpy(info->rf_module_fw_version, "fw_version");
    strcpy(info->rf_module_manufacturer, "manufacturer");

    ret = svr_get_pipe_names(nCh, 
        info->audioEndpointName_r, 
        info->audioEndpointName_w, 
        info->atEndpointName_r, 
        info->atEndpointName_w, 
        info->DebugEndpointName_r, 
        info->DebugEndpointName_w);

    /* 这个参数目前没什么意义，用作返回值 */
    if ( ret != 0 )
        info->haveDebugPort = 0;
    else
        info->haveDebugPort = 1;

    return RET_OK;
}

/*
    获取通道音频特性
*/
int ns__GetChannelAudioFromat(struct soap *soap, int nCh, struct voice_attri_t *va)
{
    va->sampleRate = 8000;
    va->samples_per_block = 16;

    return RET_OK;
}

/*
    获取at通道特性
    baudrate为波特率，如果不关心server的波特率，可以返回0
    XON_XOFF表示是否需要用XON_XOFF流控，0表示不需要， 1表示需要
*/
int ns__GetAtPortInfo(struct soap *soap, int nCh, struct at_attri_t *attri)
{
    attri->result = svr_get_at_port_info(nCh, &attri->baudrate, &attri->XON_XOFF);
    if(attri->result != 0){
        zsys_error("[%s:%d]%d is not find, result = %d\n", __FILE__, __LINE__, nCh, attri->result);
    }
    return RET_OK;
}

/*
    获取Debug通道特性
*/
int ns__GetDebugPortInfo(struct soap *soap, int nCh, struct at_attri_t *attri)
{
    attri->result = svr_get_debug_port_info(nCh, &attri->baudrate, &attri->XON_XOFF);
    if(attri->result != 0){
        zsys_error("[%s:%d]%d is not find or is not debug state, result = %d\n", __FILE__, __LINE__, nCh, attri->result);
    }
    return RET_OK;
}

/*
    获取Upgrade通道特性
*/
int ns__GetUpgradePortInfo(struct soap *soap, int nCh, struct at_attri_t *attri)
{
    attri->result = svr_get_upgrade_port_info(nCh, &attri->baudrate, &attri->XON_XOFF);
    if(attri->result < 0){
        zsys_error("[%s:%d]%d is not find, reuslt = %d\n", __FILE__, __LINE__, nCh, attri->result);
    }
    return RET_OK;
}

/*
    获取通道连接状态
    返回值0表示通道为free, 1表示通道已经被连接。如果需要其他状态，可以扩展
*/
int ns__GetChannelConnectionState(struct soap *soap, int nCh, struct chn_conn_state_t* state)
{
    state->result = svr_get_channel_connect_state(nCh, &state->audioStatus, &state->atStatus, &state->debugStatus, &state->upgradeStatus);
    if(state->result < 0){
        zsys_error("[%s:%d]%d is not find, result = %d\n", __FILE__, __LINE__, nCh, state->result);
    }
    return RET_OK;
}

/*
    开始/停止语音传输
*/
int ns__SetAudioTransmitState(struct soap *soap, int nCh, int newState, struct ns__gsoap_api_rsp_t *rsp)
{
    rsp->result = RET_OK;

    if ( newState )
        svr_chan_voice_start(nCh);
    else
        svr_chan_voice_stop(nCh);
    return RET_OK;
}

/*
    设置 server debug 开关
*/
int ns__SetServerDebug(struct soap *soap, int nVal, struct ns__gsoap_api_rsp_t *rsp)
{
    rsp->result = RET_OK;

    svr_global_debug_set(nVal);
    return RET_OK;
}

/*
    设置 通道 at debug 开关
*/
int ns__SetChannelDebug(struct soap *soap, int nCh, int nVal, struct ns__gsoap_api_rsp_t *rsp)
{
    rsp->result = RET_OK;

    if(nCh == -1){//如果是-1，就设置所有通道
    	int chn_count = svr_get_hwport_num();
    	int i = 0;
	    for(i = 0; i < chn_count; ++i){
		    svr_chan_debug_set(i, nVal);
	    }
    }else
    	svr_chan_debug_set(nCh, nVal);
    return RET_OK;
}

/*
    设置 通道 语音 debug 开关
*/
int ns__SetChannelSndDebug(struct soap *soap, int nCh, int nVal, struct ns__gsoap_api_rsp_t *rsp){
    rsp->result = RET_OK;
    if(nCh == -1){//如果是-1，就设置所有通道
    	int chn_count = svr_get_hwport_num();
    	int i = 0;
	    for(i = 0; i < chn_count; ++i){
		    svr_chan_debug_snd_set(i, nVal);
	    }
    }else
    	svr_chan_debug_snd_set(nCh, nVal);
    return RET_OK;
}

/*
    重新打开串口
*/
int ns__ReopenSerial(struct soap *soap, int nCh, struct ns__gsoap_api_rsp_t *rsp)
{
    rsp->result = RET_OK;

    module_group_com_reopen(nCh);
    return RET_OK;
}

int ns__SetChannelUpgrade(struct soap *soap, int nCh, int state, struct ns__gsoap_api_rsp_t *rsp){
    rsp->result = svr_chan_upgrade_set(nCh, state);
    return RET_OK;
}
#endif

int ns__SetChannelTxSndBufSize(struct soap *soap, int nCh, int bufsize, struct ns__gsoap_api_rsp_t *rsp){
    if(nCh == -1){
        int chn_count = svr_get_hwport_num();
        int i = 0;
        for(i = 1; i <= chn_count; ++i){
	    svr_set_channel_txsnd_buf_size(i, bufsize);
        }
    }else
        rsp->result = svr_set_channel_txsnd_buf_size(nCh, bufsize);
    
    return SOAP_OK;
}


int ns__GetChannelTxSndBufSize(struct soap *soap, int nCh, struct ns__gsoap_api_rsp_t *rsp){
    int bufsize=0;
     char tmp_buf[32];
     rsp->result = svr_get_channel_txsnd_buf_size(nCh, &bufsize);
     if(rsp->result >= 0){
        sprintf(tmp_buf, "%d", bufsize);
        rsp->value = soap_strdup(soap, tmp_buf);
     }
     
    return SOAP_OK;
}

int ns__SetChannelTxSndSpeed(struct soap *soap, int nCh, int speed, struct ns__gsoap_api_rsp_t *rsp){
    if(nCh == -1){
        int chn_count = svr_get_hwport_num();
        int i = 0;
        for(i = 1; i <=chn_count; ++i){
	    svr_set_channel_txsnd_speed(nCh, speed);
        }
    }else
        rsp->result = svr_set_channel_txsnd_speed(nCh, speed);
    
    return SOAP_OK;
}


int ns__GetChannelTxSndSpeed(struct soap *soap, int nCh, struct ns__gsoap_api_rsp_t *rsp){
     char tmp_buf[32];
     int speed = 0;
     rsp->result = svr_get_channel_txsnd_speed(nCh, &speed);
     if(rsp->result >= 0){
        sprintf(tmp_buf, "%d", speed);
        rsp->value = soap_strdup(soap, tmp_buf);
     }
    return SOAP_OK;
}

int ns__SetChannelTxSndDelay(struct soap *soap, int nCh, int delay, struct ns__gsoap_api_rsp_t *rsp){
    if(nCh == -1){
        int chn_count = svr_get_hwport_num();
        int i = 0;
        for(i = 1; i <=chn_count; ++i){
	    svr_set_channel_txsnd_delay(nCh, delay);
        }
    }else
        rsp->result = svr_set_channel_txsnd_delay(nCh, delay);
    
    return SOAP_OK;
}


int ns__GetChannelTxSndDelay(struct soap *soap, int nCh, struct ns__gsoap_api_rsp_t *rsp){
     char tmp_buf[32];
     int delay = 0;
     rsp->result = svr_get_channel_txsnd_delay(nCh, &delay);
     if(rsp->result >= 0){
        sprintf(tmp_buf, "%d", delay);
        rsp->value = soap_strdup(soap, tmp_buf);
     }
    return SOAP_OK;
}
