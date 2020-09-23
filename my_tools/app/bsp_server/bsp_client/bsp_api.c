
#include <pthread.h>

#include "public.h"
#include "soapH.h"
#include "soapStub.h"
#include "ns.nsmap"
#include "bsp_api.h"
#include "int_api.h"

/*
 *Gsoap 返回结构体( struct ns__gsoap_api_rsp_t *rsp)
 *.value 成员为 .xsd:string, 因此其值必须为可打印字符
 *否则传输过程中会出错，因此需要将char型转为打印字符
 *再传输，转换方法：
 *1，对于返回状态值，由于状态值较小，因此加编码值即可，如下，
 *  注意：当编码后staus为不可打印字符，传输可能出错。
 *     rep->value = status + ENCODE_VALUE;
 * 2，对于其它值,一个字符分高4和低4位，如下，
 *       rsp->value[j++] = (tx_value[i] >> 4) + ENCODE_VALUE;	
 *       rsp->value[j++] = (tx_value[i] & 0x0F) + ENCODE_VALUE;
 *当前返回值简单，复杂的返回值可以用base64编译
 *Base64 is a group of similar binary-to-text encoding schemes 
 *that represent binary data in an ASCII string format 
 *by translating it into a radix-64 representation. 
 *编解码接口参考：
 *https://www.codeproject.com/Tips/813146/Fast-base-functions-for-encode-decode
 */
#define CODEC_VALUE     ('0')

/* 单板支持的最大通道数 */
#define BOARD_MAX_CHN			        (128)

pthread_mutex_t g_bsp_api_lock;

#if T_DESC("bsp api init")

/* soap handle */
struct soap *g_soap_hdl = NULL;
char g_soap_url[256];

/*
desc      : api init, connect to soap server
param in  : host -- soap server ip address
            port -- soap server port
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int bsp_api_init(char *host, unsigned short port)
{
    char dst_host[256] = {0};
    unsigned short dst_port = 8809;
    
	if (host == NULL)
		strcpy(dst_host, "127.0.0.1");
    else
        strcpy(dst_host, host);

	if (port != 0)
		dst_port = port;

    memset(g_soap_url, 0, sizeof(g_soap_url));
	sprintf(g_soap_url, "http://%s:%d", dst_host, dst_port);

    if ( g_soap_hdl != NULL )
    {
        soap_destroy(g_soap_hdl);
        soap_end(g_soap_hdl);
        soap_done(g_soap_hdl);
        soap_free(g_soap_hdl);
        g_soap_hdl = NULL;
    }

	g_soap_hdl = soap_malloc(g_soap_hdl, sizeof(struct soap));
	if (g_soap_hdl == NULL)
		return RET_ERROR;
	
	soap_init1(g_soap_hdl, SOAP_XML_INDENT);

    pthread_mutex_init(&g_bsp_api_lock, NULL);

	return RET_OK;
}

/*
desc      : api deinit, disconnect from soap server
param in  : 
param out : 
return    : 
history   : create by zhongwei.peng/2017.11.24
*/
void bsp_api_deinit(void)
{
    pthread_mutex_destroy(&g_bsp_api_lock);
    
	if ( g_soap_hdl != NULL )
    {
        soap_destroy(g_soap_hdl);
        soap_end(g_soap_hdl);
        soap_done(g_soap_hdl);
        soap_free(g_soap_hdl);
        g_soap_hdl = NULL;
    }
}

#endif

#if T_DESC("simcard")
/*
desc      : enable simcard by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_enable(int chn)
{
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__simcard_enable(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : disable simcard by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_disable(int chn)
{
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__simcard_disable(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : get simcard state(disable/enable) by channel
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : states -- 1 means enable, 0 means disable
                      single channel use states[0], 
                      multi channel use states[chn]
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_enable_state_get(int chn, unsigned char *states)
{
    int i;
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    if ( states == NULL )
    	return RET_PARAM;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__simcard_enable_state_get(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        for ( i = 0; i < rsp.cnt; i++ )
        {
            states[i] = rsp.value[i] - CODEC_VALUE; /* 传的字符串 */
        }
            
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : get simcard state(insert/remove) by channel
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : states -- 0 means insert, 1 means remove
                      single channel use states[0], 
                      multi channel use states[chn]
return    : ok return 0, or return error num(<0)
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_insert_state_get(int chn, unsigned char *states)
{
    int i;
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    if ( states == NULL )
    	return RET_PARAM;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__simcard_insert_state_get(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        for ( i = 0; i < rsp.cnt; i++ )
            states[i] = rsp.value[i] - CODEC_VALUE; /* 传的字符串 */
            
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : get simcard event(insert/remove) by channel
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : events -- 0 means no event, 1 means insert, 2 means remove
                      single channel use events[0], 
                      multi channel use events[chn]
return    : ok return 0, or return error num(<0)
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_insert_event_get(int chn, unsigned char *events)
{
    int i;
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    if ( events == NULL )
    	return RET_PARAM;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__simcard_insert_event_get(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        for ( i = 0; i < rsp.cnt; i++ )
            events[i] = rsp.value[i] - CODEC_VALUE; /* 传的字符串 */
            
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}
#endif

#if T_DESC("module")
/*
desc      : turn on specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int module_turn_on(int chn)
{
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_turn_on(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : turn off specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int module_turn_off(int chn)
{
    //struct soap *tmp_soap_hdl = NULL;
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));

    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_turn_off(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}
/*
desc      : reset specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.10.26
*/
int module_reset(int chn){
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));

    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_reset(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;

}

/*
desc      : reset specified module by channel ON TIMER
param in  : chn -- channel
param out : 
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.11.01
*/
int module_turn_on_timer(int chn, int timer){
    int ret = RET_OK;
	
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));

    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_turn_on_timer(g_soap_hdl, g_soap_url, "", chn, timer, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;

}


/*
desc      : get module state(turn on/turn off)
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : states -- 1 means on, 0 means off
                      single channel use states[0], 
                      multi channel use states[chn]
return    : ok return 0, or return error num(<0)
history   : create by zhongwei.peng/2017.11.24
*/
int module_turn_on_state_get(int chn, unsigned char *states)
{
    int i;
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    if ( states == NULL )
    	return RET_PARAM;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_turn_on_state_get(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        for ( i = 0; i < rsp.cnt; i++ )
            states[i] = rsp.value[i] - CODEC_VALUE; /* 传的字符串 */
            
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : power on specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int module_power_on(int chn)
{
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_power_on(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : power off specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int module_power_off(int chn)
{
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_power_off(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : get module state(power on/power off)
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : states -- 1 means on, 0 means off
                      single channel use states[0], 
                      multi channel use states[chn]
return    : ok return 0, or return error num(<0)
history   : create by zhongwei.peng/2017.11.24
*/
int module_power_state_get(int chn, unsigned char *states)
{
    int i;
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    if ( states == NULL )
    	return RET_PARAM;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_power_state_get(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        for ( i = 0; i < rsp.cnt; i++ )
            states[i] = rsp.value[i] - CODEC_VALUE; /* 传的字符串 */
            
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}


/*
desc      : get module num
param in  : 
param out : 
return    : success return module num, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_num_get(void){
	int ret = 0;
    int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_num_get(g_soap_hdl, g_soap_url, "", &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        num = 0;
        for(i = 0; i < rsp.cnt; i++)
        {
            num = (rsp.value[i] & 0xF) + (num << 4);
        }
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    if ( ret < RET_OK )
        return ret;
    else
        return num;
}

/*
desc      : get module uid
param in  : idx--module index
param out : uidbuf--return uid
                 len--uidbuf length
return    : success 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_uid_get(int idx, char *uidbuf, int len){
	int ret = 0;
    //int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_uid_get(g_soap_hdl, g_soap_url, "", idx, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}
    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        strncpy(uidbuf, rsp.value , len);
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

	return ret;
}

/*
desc      : get module reset key status
param in  : idx--module index
param out : status--0:means reset key press, 1:means reset key not press
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_reset_status_get(int index , int *status){
	int ret = 0;
    //int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_reset_status_get(g_soap_hdl, g_soap_url, "", index, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result >= RET_OK) && rsp.value != NULL)
    {
        *status = atoi(rsp.value);
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
	return ret;
}


/*
desc      : set powerkey level
param in  : chn--channel num
            press --0:means low level, 1:means hign level
            id --"upgrade" is valid, other will return failed
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_powerkey_hign_low(int chn , int level, char *id){
	int ret = 0;
    //int num, i;
    int channel = 0xFFFF & chn;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    if(id == NULL)
      return -1;
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__module_powerkey_hign_low(g_soap_hdl, g_soap_url, "", channel, level, id, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result >= RET_OK))
    {
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
	return ret;
}



/*
desc      : get module reset key status
param in  : idx--module index
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_debug_uart_set(int idx , int enable ){
	int ret = 0;
    //int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__brd_set_debug_uart(g_soap_hdl, g_soap_url, "", idx, enable, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}
    
    if ( (ret == RET_OK)&& (rsp.result == RET_OK))
    {
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return ret;
}
#endif

#if T_DESC("board mcu")
/*
desc      : get board mcu version info
param in  : 
param out : ver_info
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_version(unsigned char *ver_info)
{
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    if ( ver_info == NULL )
    	return RET_PARAM;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__brdmcu_version(g_soap_hdl, g_soap_url, "", &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        memcpy(ver_info, rsp.value, rsp.cnt);            
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : get board mcu version info
param in  : idx -- mcu number
param out : ver_info -- version information buffer
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_int_version(unsigned int idx, unsigned char *ver_info)
{
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    if ( ver_info == NULL )
    	return RET_PARAM;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__brdmcu_int_version(g_soap_hdl, g_soap_url, "", idx, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        memcpy(ver_info, rsp.value, rsp.cnt);
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : read board mcu reg
param in  : brd -- board num(0,1)
            reg -- reg addr
            num -- reg num to read
param out : values -- reg value in string
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_reg_read(int brd, int reg, int num, unsigned char *values)
{
    int i, j;
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    if ( values == NULL )
    	return RET_PARAM;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__brdmcu_reg_read(g_soap_hdl, g_soap_url, "", brd, reg, num, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        /* 字符串转数值 */
        for ( i = 0, j = 0; i < rsp.cnt; i += 2 )
        {
            values[j] = (rsp.value[i] - CODEC_VALUE)<< 4;
            values[j++] += (rsp.value[i+1]) - CODEC_VALUE;
        }
            
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}

/*
desc      : read board mcu reg
param in  : brd -- board num(0,1)
            reg -- reg addr
            value -- value to write
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_reg_write(int brd, int reg, unsigned char value)
{
    int ret = RET_OK;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__brdmcu_reg_write(g_soap_hdl, g_soap_url, "", brd, reg, value, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret != RET_OK) || (rsp.result != RET_OK) )
        ret = RET_ERROR;
    else
        ret = RET_OK;

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    return ret;
}
#endif

#if T_DESC("board info")

/*
desc      : get board channel number
param in  : 
param out : 
return    : channel number
history   : create by zhongwei.peng/2017.12.20
*/
unsigned char brdinfo_chn_num_get(void)
{
    int ret = 0;
    int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__brdinfo_chn_num_get(g_soap_hdl, g_soap_url, "", &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        num = 0;
        for(i = 0; i < rsp.cnt; i++)
        {
         //num = (rsp.value[i] & 0xF) + (num << (i * 4));
		num = (rsp.value[i] & 0xF) + (num << (4));	
        }
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);

    if ( ret != RET_OK )
        return BOARD_MAX_CHN;
    else
        return (unsigned char)num;
}

/*
desc      : get board name and version
param in  : 
param out : version--name and version
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/

int brdinfo_version(char *version){
 	int ret = 0;
   // int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__brdinfo_version(g_soap_hdl, g_soap_url, "",  &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK)
        && (rsp.cnt > 0) 
        && (rsp.value != NULL) )
    {
        strcpy(version,  rsp.value);
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
	return ret;
}

#endif 

#if T_DESC("upgrade")

/*
desc      : select upgrade channel
param in  : chn-channels, if(chn == 0xFFFF), it means all channels unselect
param out :
return    : succes return 0 , or error return(<0)
history   : create by wengang.mu/2018.3.15
*/

int upgrade_chan_select(int chn){
	int ret = 0;
   // int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__upgrade_chan_select(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result == RET_OK))
    {

        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
	return ret;
	
}

/*
desc      : select upgrade channel
param in  : chn-channels,
param out : status--0:chn is not upgrade channel, 1:chn is upgrade channel, 2:channel is not support upgrade
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/

int upgrade_chan_status(int chn, int *status){
	int ret = 0;
//    int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__upgrade_chan_status(g_soap_hdl, g_soap_url, "", chn, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result >= RET_OK))
    {
        *status = rsp.result;
        ret = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
	return ret;
}

int chan_upgrade_status_set(int chn, int flag, char *id, int *status){
	int ret = 0;
//    int num, i;
    struct ns__gsoap_api_rsp_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__chan_upgrade_status_set(g_soap_hdl, g_soap_url, "", chn, flag, id, &rsp);
    if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

    if ( (ret == RET_OK) 
        && (rsp.result >= RET_OK))
    {
        *status = rsp.result;
        ret = RET_OK;
    }
    else
    {
        *status = RET_ERROR;
        ret = RET_ERROR;
    }

    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
	return ret;
}
#endif

#if T_DESC("debug")

/*
desc      : set sever debug level
param in  : debug value
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.4.8
*/
int bsp_server_debug_level(int value){
    int ret;
    struct ns__gsoap_api_rsp_t rsp;
	pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__bsp_server_debug_level(g_soap_hdl, g_soap_url, "", value, &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
        ret = RET_ERROR;
    }
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return RET_OK;
}
#endif


#if T_DESC("led")
/*
desc      : set signal led or work led status
param in  : chan -- channel
			status -- led status
                      CHAN_LAMP_STATUS_OFF(0):         set sig led off
                      CHAN_LAMP_STATUS_RED_ON(1):      set sig led red on
                      CHAN_LAMP_STATUS_RED_FLASH(2):   set sig led red flash
                      CHAN_LAMP_STATUS_GREEN_ON(3):       set sig led green
                      CHAN_LAMP_STATUS_GREEN_FLASH(4): set sig led green flash
                      CHAN_LAMP_STATUS_YELLOW_ON(5):       set sig led yellow
                      CHAN_LAMP_STATUS_YELLOW_FLASH(6): set sig led yellow flash
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.8.3
*/

int chan_led_set_status_sig(int chan, int status){
    int ret;
    struct ns__gsoap_api_rsp_t rsp;
	pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__chan_led_set_sig(g_soap_hdl, g_soap_url, "", chan, status, &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
        ret = RET_ERROR;
    }
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return RET_OK;
}

/*
desc      : set signal led or work led status
param in  : chan -- channel
			status -- led status
                      CHAN_LAMP_STATUS_OFF(0):         set work led off
                      CHAN_LAMP_STATUS_GREEN(3):       set work led green
                      CHAN_LAMP_STATUS_GREEN_FLASH(4): set work led green flash
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.8.3
*/

int chan_led_set_status_work(int chan, int status){
    int ret;
    struct ns__gsoap_api_rsp_t rsp;
	pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__chan_led_set_work(g_soap_hdl, g_soap_url, "", chan, status, &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
        ret = RET_ERROR;
    }
	ret = rsp.result;
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return ret;
}


/*
desc      : set signal led or work led status
param in  : status -- led status, 0--turn off led
                                  1 -turn on led
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.8.3
*/

int chan_led_set_all(int status){
    int ret;
    struct ns__gsoap_api_rsp_t rsp;
	pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__chan_led_set_all(g_soap_hdl, g_soap_url, "", status, &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
        ret = RET_ERROR;
    }
	ret = rsp.result;
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return ret;
}


/*
desc      : set mod brd system led status
param in  : status -- led status, 0--turn off led
                                  1 -turn on led
            index  -- mod brd index
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.9.22
*/

int mod_brd_led_set_sys(int index, int status){
    int ret;
    struct ns__gsoap_api_rsp_t rsp;
	pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__mod_brd_led_set_sys(g_soap_hdl, g_soap_url, "", index, status, &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
        ret = RET_ERROR;
    }
	ret = rsp.result;
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return ret;
}



/*
desc      : get system type
return    : system type
			0 --SYS_TYPE_UNKOWN
			1 --SYS_TYPE_SWG20XX
			2 --SYS_TYPE_1CHAN4SIMS
			3 --SYS_TYPE_VS_USB
			-1 --error
history   : create by wengang.mu/2018.8.3
*/


SYS_TYPE_E bsp_get_sys_type(void){
	int ret;
	int systype;
    struct ns__gsoap_api_rsp_t rsp;
	pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__bsp_get_sys_type(g_soap_hdl, g_soap_url, "", &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
//        ret = RET_ERROR;
		systype = -1;
    }
	if(rsp.result >= 0){
		systype = atoi(rsp.value);
	}else
		systype = -1;
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return systype;
}

int select_simcard_from_chan(int chan, int slot){
   int ret;
    struct ns__gsoap_api_rsp_t rsp;
	pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__set_sim_card_slot(g_soap_hdl, g_soap_url, "", chan, slot, &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
        ret = RET_ERROR;
    }
	ret = rsp.result;
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return ret;
}

int get_chan_all_sim_state(int chan, int state[][4]){
   int ret;
   if(state == NULL)
   	return -1;
    struct ns__gsoap_api_rsp_t rsp;
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__get_sim_state_all(g_soap_hdl, g_soap_url, "", chan, &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
        ret = RET_ERROR;
    }
	ret = rsp.result;
    int i = 0, j = 0;
    int cnt = rsp.cnt/4;
    int count  = 0;
    for(i = 0; i < cnt; i++){
		for(j = 0; j < 4; j++){
			state[i][j] = rsp.value[count] - CODEC_VALUE;
			count++;
		}
    }
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return ret;
}


int get_chan_one_sim_state(int chan, int card,int *state){
   int ret;
   if(state == NULL)
   	return -1;
    struct ns__gsoap_api_rsp_t rsp;
    pthread_mutex_lock(&g_bsp_api_lock);
    ret = soap_call_ns__get_sim_state_one(g_soap_hdl, g_soap_url, "", chan, card, &rsp);
    if ( g_soap_hdl->error )
    {
        soap_print_fault(g_soap_hdl, stderr);
        ret = RET_ERROR;
    }
	ret = rsp.result;
    int i = 0, j = 0;
    int cnt = rsp.cnt;
    int count  = 0;
    for(i = 0; i < cnt; i++){
	state[i] = rsp.value[count] - CODEC_VALUE;
	count++;
    }
    soap_destroy(g_soap_hdl);
    soap_end(g_soap_hdl);
    pthread_mutex_unlock(&g_bsp_api_lock);
    return ret;
}

int get_sim_version(unsigned int idx, unsigned char *ver_info)
{
	int ret = RET_OK;
	struct ns__gsoap_api_rsp_t rsp;

	if ( ver_info == NULL ){
		return RET_PARAM;
	}

	memset(&rsp, 0, sizeof(rsp));
	pthread_mutex_lock(&g_bsp_api_lock);
	ret = soap_call_ns__get_sim_version(g_soap_hdl, g_soap_url, "", idx, &rsp);
	if ( g_soap_hdl->error )
	{
		soap_print_fault(g_soap_hdl, stderr);
		ret = RET_ERROR;
	}

	if ( (ret == RET_OK) 
			&& (rsp.result == RET_OK)
			&& (rsp.cnt > 0) 
			&& (rsp.value != NULL) )
	{
		memcpy(ver_info, rsp.value, rsp.cnt);
		ret = RET_OK;
	}
	else
	{
		ret = RET_ERROR;
	}

	soap_destroy(g_soap_hdl);
	soap_end(g_soap_hdl);
	pthread_mutex_unlock(&g_bsp_api_lock);

	return ret;
}

#endif
