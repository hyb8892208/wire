/*
    gsoap api ÊµÏÖ
*/
#include "public.h"
#include "bsp_api.h"
#include "soapH.h"
//#include "ns.nsmap"
#include "czmq.h"
#include "swg_intf.h"
#include "bsp_tools.h"

#define GET_VERSION_MAJOR(VERSION)          (((VERSION)>> 16) & 0xFF)
#define GET_VERSION_MINOR(VERSION)          (((VERSION)>> 8) & 0xFF)
#define GET_VERSION_BUGFIX(VERSION)         (((VERSION)) & 0xFF)
#define     MAX_CHAN_NUM        64

typedef enum mod_brd_name_e
{
	MB_NAME_UNKOWN = 0,
	MB_SWG_1004_BASE,   /* 4¿¿¿¿, ¿¿¿¿¿¿ */
	MB_SWG_1008_BASE,   /* 8¿¿¿¿, ¿¿¿¿¿¿ */
	MB_SWG_1016_BASE,   /* 16¿¿¿¿,¿¿¿¿¿16¿¿32¿¿¿, ¿¿MB_SWG_1032_BASE, ¿¿¿¿ */
	MB_SWG_1032_BASE,   /* 32¿¿¿¿¿¿¿¿¿¿1032,¿¿¿¿¿¿16¿¿32¿¿2¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿ */ 
	MB_SWG_VS_USB_BASE, /* ¿¿¿4¿USB¿¿*/
	MB_SWG_VS2_USB_DOWN,
	MB_SWG_VS_USB_EMU,
}MOD_BRD_NAME_E;

typedef enum swg_name_e{
	SWG_DEV_UNKOWN = 0,
	SWG_DEV_1001,       /* 1¿ÚÎÞÏßÍø¹Ø */
	SWG_DEV_1002,       /* 2¿ÚÎÞÏßÍø¹Ø */ 
	SWG_DEV_1004,       /* 4¿ÚÎÞÏßÍø¹Ø */
	SWG_DEV_1008,       /* 8¿ÚÎÞÏßÍø¹Ø */
	SWG_DEV_1016,       /* 16¿ÚÎÞÏßÍø¹Ø */
	SWG_DEV_1032,       /* 32¿ÚÎÞÏßÍø¹Ø */
	SWG_DEV_1064,       /* 64¿ÚÎÞÏßÍø¹Ø */
	SWG_DEV_VS_USB_1020,/* 20¿Ú¿É²å°ÎUSBÄ£¿éÎÞÏßÍø¹Ø 1U»úÏä*/
	SWG_DEV_VS_USB_1044,/* 44¿Ú¿É²å°ÎUSBÄ£¿éÎÞÏßÍø¹Ø 2U»úÏä*/
}SWG_NAME_E;
static inline char *name_to_str(int name){
	switch (name){
		case SWG_DEV_UNKOWN:
			return "SWG_DEV_UNKOWN";
			break;
		case SWG_DEV_1001:
			return "SWG_DEV_1001";
			break;
		case SWG_DEV_1002:
			return "SWG_DEV_1002";
			break;
		case SWG_DEV_1004:
			return "SWG_DEV_1004";
			break;
		case SWG_DEV_1008:
			return "SWG_DEV_1008";
			break;
		/*case SWG_DEV_1016:
			return "SWG_DEV_1016";
			break;
		*/case SWG_DEV_1032:
			return "SWG_DEV_1032";
			break;
		case SWG_DEV_1064:
			return "SWG_DEV_1064";
			break;
		case SWG_DEV_VS_USB_1020:
			return "SWG_DEV_VS_USB_1020";
			break;
		case SWG_DEV_VS_USB_1044:
			return "SWG_DEV_VS_USB_1044";
			break;
		default:
			return "SWG_DEV_UNKOWN";
			break;

	}
}

static inline char *module_brd_name_to_str(int name)
{
	switch (name)
	{
		case MB_NAME_UNKOWN:
			return "HW_NAME_UNKOWN";
			break;

		case MB_SWG_1004_BASE:
			return "SWG_1004_BASE";
			break;

		case MB_SWG_1008_BASE:
			return "SWG_1008_BASE";
			break;

			/*
			 *[>¿¿16¿¿32¿¿¿, ¿¿HW_SWG_1032_BASE <]
			 *case MB_SWG_1016_BASE:
			 *    return "SWG_1016_BASE";
			 *    break;
			 */
		case MB_SWG_1032_BASE:
			return "SWG_1032_BASE";
			break;
		case MB_SWG_VS2_USB_DOWN:
			return "VS2_8XEC20_DOWN";
			break;

		case MB_SWG_VS_USB_BASE:
			return "VS_USB_BASE";
			break;

		case MB_SWG_VS_USB_EMU:
			return "VS_USB_EMU";
			break;

		default:
			return "NAME_UNKOWN";
			break;
	}
}
/*
 *Gsoap ·µ»Ø½á¹¹Ìå( struct ns__gsoap_api_rsp_t *rsp)
 *.value ³ÉÔ±Îª .xsd:string, Òò´ËÆäÖµ±ØÐëÎª¿É´òÓ¡×Ö·û
 *·ñÔò´«Êä¹ý³ÌÖÐ»á³ö´í£¬Òò´ËÐèÒª½«charÐÍ×ªÎª´òÓ¡×Ö·û
 *ÔÙ´«Êä£¬×ª»»·½·¨£º
 *1£¬¶ÔÓÚ·µ»Ø×´Ì¬Öµ£¬ÓÉÓÚ×´Ì¬Öµ½ÏÐ¡£¬Òò´Ë¼Ó±àÂëÖµ¼´¿É£¬ÈçÏÂ£¬
 *  ×¢Òâ£ºµ±±àÂëºóstausÎª²»¿É´òÓ¡×Ö·û£¬´«Êä¿ÉÄÜ³ö´í¡£
 *     rep->value = status + ENCODE_VALUE;
 * 2£¬¶ÔÓÚÆäËüÖµ,Ò»¸ö×Ö·û·Ö¸ß4ºÍµÍ4Î»£¬ÈçÏÂ£¬
 *       rsp->value[j++] = (tx_value[i] >> 4) + ENCODE_VALUE;	
 *       rsp->value[j++] = (tx_value[i] & 0x0F) + ENCODE_VALUE;
 *µ±Ç°·µ»ØÖµ¼òµ¥£¬¸´ÔÓµÄ·µ»ØÖµ¿ÉÒÔÓÃbase64±àÒë
 *Base64 is a group of similar binary-to-text encoding schemes 
 *that represent binary data in an ASCII string format 
 *by translating it into a radix-64 representation. 
 *±à½âÂë½Ó¿Ú²Î¿¼£º
 *https://www.codeproject.com/Tips/813146/Fast-base-functions-for-encode-decode
 */
#define CODEC_VALUE     ('0')

enum
{
	OFF = 0,
	ON = 1,
	RESET = 4,
};

enum
{
	DISABLE = 0,
	ENABLE = 1,
	
};

extern unsigned int bsp_prt_unmask;
#if T_DESC("simcard")
/*
desc      : enable simcard by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_enable(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
	int channel = 0xFFFF & chn;

       rsp->result = swg_sim_card_enable(channel,ENABLE);
       if(rsp->result != RET_OK){
         BSP_PRT( ERR,"[%s %d] swg_sim_card_enable failed,chn = %d,res = %d\n", __FILE__, __LINE__, chn, rsp->result);
	     return SOAP_OK;
    	}
	BSP_PRT( INFO,"[%s:%s:%d] enable chn[%d] ok!\n", __FILE__,__func__ ,__LINE__, chn);
	return RET_OK;
}

/*
desc      : disable simcard by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_disable(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
	int channel = 0xFFFF & chn;
    rsp->result = swg_sim_card_enable(channel, DISABLE);
    if(rsp->result != RET_OK){
        BSP_PRT( ERR,"[%s %d] swg_sim_card_enable failed,chn = %d,res = %d\n", __FILE__, __LINE__, chn,rsp->result);
        return SOAP_OK;
    }
	BSP_PRT( INFO,"[%s:%s:%d] disable chn[%d] ok!\n", __FILE__,__func__ ,__LINE__, chn);
	return RET_OK;
}

/*
desc      : get simcard state(disable/enable) by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_enable_state_get(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
    int i;
    int cnt = 0;
    int states[MAX_CHAN_NUM] = {0};
    int channel = 0xFFFF & chn;
    cnt = swg_get_total_chan_num();
    if(cnt < 0){
        rsp->result = RET_ERROR;
        return RET_OK;
    }
    rsp->result = swg_get_sim_card_enable_status(channel,states);
    if ( rsp->result != RET_OK ){
        BSP_PRT( ERR,"[%s %d] swg_get_sim_card_enable_status failed,chn = %d, res = %d\n", __FILE__, __LINE__, chn, rsp->result);
        return RET_OK; 
    }
    if ( rsp->value == NULL )
    {
        rsp->value = soap_malloc(soap, cnt+1);
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }
	
	memset(rsp->value, 0, cnt + 1);

    for ( i = 0; i < cnt; i++ )
    {
        rsp->value[i] = (char)(states[i] + CODEC_VALUE);
    }
    
    rsp->cnt = cnt;
	if(channel == 0xFFFF)
		BSP_PRT( DBG,"[%s:%s:%d] get all channels enable state ok, the state is %s(1 means enable)!\n", __FILE__,__func__ ,__LINE__, rsp->value);
	else
		BSP_PRT( DBG,"[%s:%s:%d] get chn[%d] enable state ok, the state is %s!\n", __FILE__,__func__ ,__LINE__, chn, states==0?"disable":"enable");

    return RET_OK;
}

/*
desc      : get simcard state(insert/remove) by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_insert_state_get(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
    int i;
    int cnt = 0;
    int states[128] = {0};
	int channel = 0xFFFF & chn;

	cnt = swg_get_total_chan_num();
	
	if(cnt < 0){
		rsp->result = RET_ERROR;
		return RET_OK;
	}

	rsp->result = swg_get_sim_card_insert_status(channel,states);
    if ( rsp->result != RET_OK ){
        BSP_PRT( ERR,"[%s %d] swg_get_sim_card_insert_status failed,chn = %d, res = %d\n", __FILE__, __LINE__, chn, rsp->result);
        return RET_OK; 
    }

    if ( rsp->value == NULL )
    {
        rsp->value = soap_malloc(soap, cnt+1);
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }
	memset(rsp->value, 0, cnt + 1);
    for ( i = 0; i < cnt; i++ )
    {
        rsp->value[i] = (char)(!(states[i]) + CODEC_VALUE); /* bsp_api ÖÐ0±íÊ¾ÒÑ²åÈë£¬1±íÊ¾Î´²åÈë */
    }
    rsp->cnt = cnt;
	if(channel == 0xFFFF){
		BSP_PRT( DBG,"[%s:%s:%d] get all channel insert state ok, sim card is %s(1 means insert)!\n", __FILE__,__func__ ,__LINE__, rsp->value);
    }else
		BSP_PRT( DBG,"[%s:%s:%d] get chn[%d] insert state ok, sim card is %s!\n", __FILE__,__func__ ,__LINE__, chn, states[0]==1?"insert":"remove");
    return RET_OK;
}

/*
desc      : get simcard event(insert/remove) by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_insert_event_get(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
    int i;
    int cnt = 0;
    int states[MAX_CHAN_NUM] = {0};
	int channel = 0xFFFF & chn;

	cnt = swg_get_total_chan_num();
	if(cnt < 0){
        BSP_PRT( ERR,"[%s %d] swg_get_total_chan_num failed,res = %d\n", __FILE__, __LINE__, rsp->result);
		rsp->result = RET_ERROR;
		return RET_OK;
	}
	rsp->result = swg_get_sim_card_insert_event(channel,states);
    if ( rsp->result != RET_OK ){
        BSP_PRT( ERR ,"[%s %d] swg_get_sim_card_insert_event failed, chn =%d, res = %d\n", __FILE__, __LINE__, chn, rsp->result);
        return RET_OK;
    }

    if ( rsp->value == NULL )
    {
        rsp->value = soap_malloc(soap, cnt+1);
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }
	memset(rsp->value, 0, cnt + 1);
	
    for ( i = 0; i < cnt; i++ )
        rsp->value[i] = (char)(states[i] + CODEC_VALUE);
    
    
    rsp->cnt = cnt;

	if(channel == 0xFFFF){
		BSP_PRT( DBG,"[%s:%s:%d] get all channels insert envent ok,event is %s(1 means insert)!\n", __FILE__,__func__ ,__LINE__, rsp->value);
    }else
		BSP_PRT( DBG,"[%s:%s:%d] get chn[%d] insert envent ok,event is %s!\n", __FILE__,__func__ ,__LINE__, chn, states[0]==1 ? "insert":"remove");
    return RET_OK;
}
#endif

#if T_DESC("module")
/*
desc      : turn on specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_turn_on(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
	int channel = 0xFFFF & chn;
	rsp->result = swg_chan_mod_power_on_off(channel,ON);
    if(rsp->result != RET_OK){
        BSP_PRT( ERR,"[%s %d] swg_chan_mod_power_on_off failed, ,res = %d\n", __FILE__, __LINE__, rsp->result);
	    return SOAP_OK;
	}
    //rsp->result = brd_mcu_module_turn_on(chn);
	BSP_PRT( INFO,"[%s:%s:%d] turn on chn[%d] ok!\n", __FILE__,__func__ ,__LINE__, chn);
    return RET_OK;
}

/*
desc      : turn off specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_turn_off(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
	int channel = 0xFFFF & chn;
	rsp->result = swg_chan_mod_power_on_off(channel,OFF);
	if(rsp->result != RET_OK){
		BSP_PRT( ERR,"[%s %d] swg_chan_mod_power_on_off failed,chn = %d, res = %d\n", __FILE__, __LINE__, chn, rsp->result);
		return SOAP_OK;
	}
    //rsp->result = brd_mcu_module_turn_off(chn);
	BSP_PRT( INFO,"[%s:%s:%d] turn off chn[%d] ok!\n", __FILE__,__func__ ,__LINE__, chn);
    return RET_OK;
}

/*
desc      : reset specified module by channel
param in  : chn -- channel,  it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.10.26
*/
int ns__module_reset(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
	int channel = 0xFFFF & chn;
	if(channel == 0xFFFF){
		rsp->result = -1;
		return SOAP_OK;
	}
	rsp->result = swg_chan_mod_power_on_off(channel,RESET);
	if(rsp->result != RET_OK){
		BSP_PRT( ERR,"[%s %d] power reset failed,chn = %d, res = %d, reset=%d\n", __FILE__, __LINE__, chn, rsp->result, RESET);
		return SOAP_OK;
	}
    //rsp->result = brd_mcu_module_turn_off(chn);
	BSP_PRT( INFO,"[%s:%s:%d] power reset chn[%d] ok!\n", __FILE__,__func__ ,__LINE__, chn);
    return RET_OK;
}


/*
desc      : power off specified module by channel ON TIMER
param in  : chn -- channel
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.11.07
*/
int ns__module_turn_on_timer(struct soap *soap, int chn, int timer, struct ns__gsoap_api_rsp_t *rsp)
{
	int channel = 0xFFFF & chn;

	if(channel == 0xFFFF){
		BSP_PRT( ERR,"[%s:%s:%d] channle is out of range, chan = %d.\n", __FILE__,__func__ ,__LINE__, chn);
		rsp->result = -1;
		return RET_OK;
	}

	if(timer <= 0 ){
		BSP_PRT( ERR,"[%s:%s:%d] timer is out of range(1-300), chan = %d, timer = %d.\n", __FILE__,__func__ ,__LINE__, chn, timer);
		rsp->result = -2;
		return RET_OK;
	}else if(timer > 300){
		BSP_PRT( ERR,"[%s:%s:%d] channle is out of range(1-300), chan = %d, timer = %d.\n", __FILE__,__func__ ,__LINE__, chn, timer);
		rsp->result = -2;
		return SOAP_OK;
	}
	
	rsp->result = swg_chan_mod_power_on_module_timer(channel,timer);
	if(rsp->result != RET_OK){
		BSP_PRT( ERR ,"[%s %d] swg_chan_mod_vbat_supply failed, chn = %d,res = %d\n", __FILE__, __LINE__,channel, rsp->result);
		return SOAP_OK;
	}
	BSP_PRT( INFO,"[%s:%s:%d] set chn[%d] power on timer ok, timer = %d .\n", __FILE__,__func__ ,__LINE__, chn, timer);
	
    return RET_OK;
}


/*
desc      : get module state(turn on/turn off)
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_turn_on_state_get(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
    int i;
    int cnt = 0;
    int states[MAX_CHAN_NUM] = {0};

	int channel = 0xFFFF & chn;
	cnt = swg_get_total_chan_num();
	if(cnt < 0){
		rsp->result = RET_ERROR;
		return RET_OK;
	}
	rsp->result = swg_get_chan_mod_power_status(channel, states);
    	if ( rsp->result != RET_OK ){
		BSP_PRT( ERR,"[%s %d] swg_get_chan_mod_power_status failed,res = %d\n", __FILE__, __LINE__, chn, rsp->result);	
		return RET_OK; /* rsp->result´«½á¹û£¬ÕâÀïÓÀÔ¶·µ»ØOK */
	}
    if ( rsp->value == NULL )
    {
		//¶à·ÖÅäÒ»¸ö×Ö½Ú£¬±ãÓÚlog´òÓ¡
        rsp->value = soap_malloc(soap, cnt+1);
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }
	memset(rsp->value, 0, cnt + 1);

    /* ×ª³É×Ö·û´®ÔÙ´«Êä */
    for ( i = 0; i < cnt; i++ )
        rsp->value[i] = (char)(states[i] + CODEC_VALUE);
    
    
    rsp->cnt = cnt;
	if(channel == 0xFFFF)
		BSP_PRT( DBG,"[%s:%s:%d] get all channels turn state ok, states is %s(1 means on)\n", __FILE__,__func__ ,__LINE__, rsp->value);
	else
		BSP_PRT( DBG,"[%s:%s:%d] get chn[%d] turn state ok, states is %s\n", __FILE__,__func__ ,__LINE__, chn, states[0]?"ON":"OFF");
    return RET_OK;
}

/*
desc      : power on specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_power_on(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
	int channel = 0xFFFF & chn;
	rsp->result = swg_chan_mod_vbat_supply(channel,ON);
	if(rsp->result != RET_OK){
		BSP_PRT( ERR,"[%s %d] swg_chan_mod_vbat_supply failed,res = %d\n", __FILE__, __LINE__,chn, rsp->result);
		return SOAP_OK;
	}
	BSP_PRT( INFO,"[%s:%s:%d] power on chn[%d] ok!\n", __FILE__,__func__ ,__LINE__, chn);
	return RET_OK;
}

/*
desc      : power off specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_power_off(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
	int channel = 0xFFFF & chn;
	rsp->result = swg_chan_mod_vbat_supply(channel,OFF);
	if(rsp->result != RET_OK){
		BSP_PRT( ERR ,"[%s %d] swg_chan_mod_vbat_supply failed, chn = %d,res = %d\n", __FILE__, __LINE__,channel, rsp->result);
		return SOAP_OK;
	}
	BSP_PRT( INFO,"[%s:%s:%d] power off chn[%d] ok!\n", __FILE__,__func__ ,__LINE__, chn);
	
    return RET_OK;
}

/*
desc      : get module state(power on/power off)
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_power_state_get(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp)
{
    int i;
    int cnt = 0;
	int channel = 0xFFFF & chn;
    int states[MAX_CHAN_NUM] = {0};

	cnt = swg_get_total_chan_num();
	if(cnt < 0){
		BSP_PRT( ERR,"[%s %d] swg_get_total_chan_num failed,res = %d\n", __FILE__, __LINE__, rsp->result);
		rsp->result = RET_ERROR;
		return RET_OK;
	}
	rsp->result = swg_get_chan_mod_vbat_status(channel,states);
	if ( rsp->result != RET_OK ){
		BSP_PRT( ERR,"[%s %d] swg_get_chan_mod_vbat_status failed,chn = %d, res = %d\n", __FILE__, __LINE__, chn, rsp->result);
        	return RET_OK;
    	}

	if ( rsp->value == NULL )
	{
		//¶à·ÖÅäÒ»¸ö×Ö½Ú£¬±ãÓÚlog´òÓ¡
        rsp->value = soap_malloc(soap, cnt+1);
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }
	memset(rsp->value, 0, cnt + 1);

    /* ×ª³É×Ö·û´®ÔÙ´«Êä */
    for ( i = 0; i < cnt; i++ )
        rsp->value[i] = (char)(states[i] + CODEC_VALUE);
    
    
    rsp->cnt = cnt;
	if(channel == 0xFFFF)
		BSP_PRT( DBG,"[%s:%s:%d] get all channel power ok, state is %s(1 means on)!\n", __FILE__,__func__ ,__LINE__, rsp->value);
	else
		BSP_PRT( DBG,"[%s:%s:%d] get chn[%d] power state ok, state is %s!\n", __FILE__,__func__ ,__LINE__, chn,states[0]?"on":"off");
    return RET_OK;
}

/*
desc      : get module num
param in  : 
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2017.3.25
*/

int ns__module_num_get(struct soap *soap, struct ns__gsoap_api_rsp_t *rsp){
	int num, i;
	int len = 32;
	//char buf[32] = {0};
	num = swg_get_total_mod_brd_num();
	if(num < 0)
	{
		BSP_PRT( ERR,"[%s %d] swg_get_total_mod_brd_num failed,res = %d\n", __FILE__, __LINE__, rsp->result);
		rsp->result = num;
		return RET_OK;
	}
	 if ( rsp->value == NULL )
	{
	    rsp->value = soap_malloc(soap, len);
	    if ( rsp->value == NULL )
	        return SOAP_ERR;
	}
	 memset( rsp->value,0,sizeof(int));
	for( i = 0; i < 8; i++)
	{
	    /* ±àÂë³É´òÓ¡×Ö·û£¬ÏÈ´«¸ßÎ»ÔÙ´«µÍÎ» */
	    rsp->value[i] = ((num >> (4 * (8 -i - 1))) & 0xF) + CODEC_VALUE;
	}
	rsp->result = RET_OK;
	rsp->cnt = 8;
	BSP_PRT( INFO,"[%s:%s:%d] get module num ok, module total %d!\n", __FILE__,__func__ ,__LINE__, num);
	return SOAP_OK;
}

/*
desc      : get module uid
param in  : idx  --module index
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2017.3.25
*/
int ns__module_uid_get(struct soap *soap, int idx,  struct ns__gsoap_api_rsp_t *rsp){
	int num;
	char buf[32] = {0};
	num = swg_get_total_mod_brd_num();
	if(idx > num){
		BSP_PRT( ERR,"Invalied mod brd %d(totlal %d)\n ", idx, num);
		return RET_OK;
	}
	num = swg_get_mod_brd_uid(idx, buf, 32 );
	if(num < 0)
	{
		BSP_PRT( ERR,"[%s %d] swg_get_mod_brd_uid failed, index = %d, res = %d\n", __FILE__, __LINE__, idx, rsp->result);
		rsp->result = num;
		return SOAP_OK;
	}
#if 0
	 if ( rsp->value == NULL )
	{
	    rsp->value = soap_malloc(soap, 32);
	    if ( rsp->value == NULL )
	        return SOAP_ERR;
	}
	int i = 0;
	for(i = 0; i < 64; i ++){
		rsp->value[i] = ((num) >> (4 *( 64 -i - 1)&0xF) + CODEC_VALUE;
	}
	rsp->result = RET_OK;
	rsp->cnt = 64;
#endif
	rsp->value = soap_strdup(soap, buf);
	rsp->result = RET_OK;
	rsp->cnt = num;
	BSP_PRT( INFO,"[%s:%s:%d] get module[%d] uid ok, module uid is %s!\n", __FILE__,__func__ ,__LINE__, idx, buf);
	return SOAP_OK;
}

/*
desc      : get module reset key status
param in  : index --module index
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2017.3.25
*/

int ns__module_reset_status_get(struct soap *soap, int index, struct ns__gsoap_api_rsp_t *rsp ){
	int status;
	char status_buf[32] = {0};
	rsp->result = swg_get_mod_brd_reset_key_status(index,  &status);
	if(rsp->result < 0){
		BSP_PRT( INFO,"[%s %d] swg_get_mod_brd_reset_key_status failed, index = %d, res = %d\n", __FILE__, __LINE__, index, rsp->result);
		return SOAP_OK;
    	}
	
	sprintf(status_buf, "%d", status);

	rsp->value = soap_strdup(soap, status_buf);

	rsp->cnt = strlen(status_buf);
	BSP_PRT( INFO,"[%s:%s:%d] get module[%d] reset key state ok, reset key state is %spress!\n", __FILE__,__func__ ,__LINE__, index, status?"":" not ");
	return SOAP_OK;
}

int ns__module_powerkey_hign_low(struct soap *soap, int chn, int level, char* id, struct ns__gsoap_api_rsp_t *rsp ){
    rsp->result = swg_chan_mod_powerkey_hign_low(chn, level, id);
    if(rsp->result < 0){
		BSP_PRT( ERR,"[%s %d] swg_chan_mod_powerkey_on_off failed,chn = %d, res = %d\n", __FILE__, __LINE__, chn, rsp->result);
        return SOAP_OK;
    }
    BSP_PRT( INFO,"[%s:%s:%d] chn[%d] set powerkey %s success!\n", __FILE__,__func__ ,__LINE__, chn, level?"high":"low "); 
    return SOAP_OK;
}
#endif

#if T_DESC("board mcu")
/*
desc      : get board mcu version info
param in  : 
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__brdmcu_version(struct soap *soap, struct ns__gsoap_api_rsp_t *rsp)
{
    if ( rsp->value == NULL )
    {
        rsp->value = soap_malloc(soap, 32);
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }

    memset(rsp->value, 0, 32);
    strcpy(rsp->value, "version: 1.5");
    rsp->cnt = strlen(rsp->value);
    rsp->result = 0;
	BSP_PRT( INFO,"[%s:%s:%d] get version ok, version is %s!\n", __FILE__,__func__ ,__LINE__, rsp->value);
	return RET_OK;
}

/*
desc      : get board mcu version info
param in  : idx -- mcu id
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__brdmcu_int_version(struct soap *soap, unsigned int idx, struct ns__gsoap_api_rsp_t *rsp)
{
	struct mod_brd_ver_info info;
	
	if(RET_OK != (rsp->result = swg_get_mod_brd_ver_info(idx,&info))) //º¯Êý»ñÈ¡infoÊ§°Ü´¦Àí£¿
	{
		BSP_PRT( ERR,"[%s %d] swg_get_mod_brd_ver_info failed,res = %d\n", __FILE__, __LINE__, rsp->result);
		return SOAP_OK;
	}
	
    if ( rsp->value == NULL )
    {
        rsp->value = soap_malloc(soap, 64);
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }

    memset(rsp->value, 0, 64);
	snprintf(rsp->value, 64, "BOARD_TYPE:%s\nHW_VER:V%d.%d\nSW_VER:V%d.%d\n",module_brd_name_to_str(info.name),
		    (int)GET_VERSION_MAJOR(info.hw_ver),(int)GET_VERSION_MINOR(info.hw_ver),
		    (int)GET_VERSION_MAJOR(info.sw_ver),(int)GET_VERSION_MINOR(info.sw_ver));

    rsp->cnt = strlen(rsp->value);
    rsp->result = RET_OK;
	BSP_PRT( INFO,"[%s:%s:%d] get int_version ok, version is %s!\n", __FILE__,__func__ ,__LINE__, rsp->value);
    return RET_OK;
}

/*
desc      : read board mcu reg
param in  : brd -- board num(0,1)
            reg -- reg addr
            num -- reg num to read
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__brdmcu_reg_read(struct soap *soap, int brd, int reg, int num, struct ns__gsoap_api_rsp_t *rsp)
{
    int i, j;
    int len;
    int values[128] = {0};

    rsp->result = swg_read_mod_brd_reg(brd,reg,num,values); 
    if ( rsp->result != RET_OK ){
	 BSP_PRT( INFO,"[%s %d] swg_read_mod_brd_reg failed,res = %d\n", __FILE__, __LINE__, rsp->result);
	 return SOAP_OK;
    }

    len = num * 2;

    if ( rsp->value == NULL )
    {
        rsp->value = soap_malloc(soap, len);
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }

    memset(rsp->value, 0, len);

    /* ²ð¿ªºóÔÙ·¢ËÍ */
    for ( i = 0, j = 0; i < num; i++ )
    {
        rsp->value[j++] = ((values[i] & 0xFF)>> 4) + CODEC_VALUE;	
        rsp->value[j++] = (values[i] & 0x0F) + CODEC_VALUE;
    }

    rsp->cnt = len;
	BSP_PRT( INFO,"[%s:%s:%d] read reg ok!\n", __FILE__,__func__ ,__LINE__);
    return RET_OK;
}

/*
desc      : read board mcu reg
param in  : brd -- board num(0,1)
            reg -- reg addr
            value -- value to write
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__brdmcu_reg_write(struct soap *soap, int brd, int reg, unsigned char value, struct ns__gsoap_api_rsp_t *rsp)
{
    rsp->result = swg_write_mod_brd_reg(brd,reg,value);
    if ( rsp->result != RET_OK ){
	 BSP_PRT( ERR,"[%s %d] swg_write_mod_brd_reg failed,res = %d\n", __FILE__, __LINE__, rsp->result);
	 return SOAP_OK;
    }
	BSP_PRT( INFO,"[%s:%s:%d] write reg ok!\n", __FILE__,__func__ ,__LINE__);
    return RET_OK;
}
#endif

#if T_DESC("board info")
int ns__brdinfo_chn_num_get(struct soap *soap, struct ns__gsoap_api_rsp_t *rsp)
{
    int num, i;
    if ( rsp->value == NULL )
    {
        rsp->value = soap_malloc(soap, sizeof(int));
        if ( rsp->value == NULL )
            return SOAP_ERR;
    }
	num = swg_get_total_chan_num();
    if(num < 0)
    {
        rsp->result = num; 
	 	BSP_PRT( ERR,"[%s %d] swg_get_total_chan_num failed,res = %d\n", __FILE__, __LINE__, rsp->result);
        return RET_OK;
    }
    memset( rsp->value,0,sizeof(int));
    for( i = 0; i < 8; i++)
    {
        /* ±àÂë³É´òÓ¡×Ö·û£¬ÏÈ´«¸ßÎ»ÔÙ´«µÍÎ» */
        rsp->value[i] = ((num >> (4 * (8 -i - 1))) & 0xF) + CODEC_VALUE;
    }

    rsp->result = RET_OK;
    rsp->cnt = 8;
	BSP_PRT( INFO,"[%s:%s:%d] get channel num ok, chn num is %d!\n", __FILE__,__func__ ,__LINE__, num);
    return RET_OK;
}

/*
desc      : get board info
param in  : 
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2017.3.25
*/

int ns__brdinfo_version(struct soap *soap, struct ns__gsoap_api_rsp_t *rsp){
	struct swg_ver_info info;
	
	char version[64] = {0};
	
	if(rsp->value == NULL){
		rsp->value = (char *)soap_malloc(soap, rsp->cnt);
		if(rsp->value == NULL){
			return SOAP_ERR;
		}
	}
	rsp->result = swg_get_dev_ver_info(&info);
	if ( rsp->result != RET_OK ){
	 	BSP_PRT( ERR,"[%s %d]swg_get_dev_ver_info failed,res = %d\n", __FILE__, __LINE__, rsp->result);
		return SOAP_OK;
	}
	sprintf(version, "name:%s\nversion:%ld.%ld.%ld\n", name_to_str(info.name),
											GET_VERSION_MAJOR(info.version),
								    			GET_VERSION_MINOR(info.version),
								    			GET_VERSION_BUGFIX(info.version));
	rsp->value = soap_strdup(soap, version);
	rsp->cnt = strlen(version);
	BSP_PRT( INFO,"[%s:%s:%d]get brd info ok, brd info is:\n%s\n", __FILE__,__func__ ,__LINE__, version);
	return SOAP_OK;
}

#endif

#if T_DESC("upgrade")

/*
desc      : select upgrade channel
param in  : chn --channel, if(chn == 0xFFFF), it means all channels unselect!
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2017.3.25
*/

int ns__upgrade_chan_select(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp){
	
	rsp->result = swg_select_upgrade_chan(chn&0xFFFF);
	if ( rsp->result != RET_OK ){
		BSP_PRT( ERR, "[%s %d] swg_select_upgrade_chan failed,res = %d\n", __FILE__, __LINE__, rsp->result);
		return SOAP_OK;
	}
	BSP_PRT( INFO,"[%s:%s:%d] select upgrade chn[%d] ok!\n", __FILE__,__func__ ,__LINE__, chn);
	return SOAP_OK;
}

/*
desc      : get channels status
param in  : chn --channel
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2017.3.25
*/
int ns__upgrade_chan_status(struct soap *soap, int chn, struct ns__gsoap_api_rsp_t *rsp){
	
	rsp->result = swg_upgrade_chan_is_selected(chn&0xFFFF);
	if ( rsp->result < 0 ){
		BSP_PRT( ERR,"[%s %d] swg_upgrade_chan_is_selected failed,res = %d\n", __FILE__, __LINE__, rsp->result);
		return SOAP_OK;
	}
	
	BSP_PRT( INFO,"[%s:%s:%d] get chn[%d] upgrade state ok, chn[%d] is %supgrade channel!\n", __FILE__,__func__ ,__LINE__, chn, chn, rsp->result?"":" not ");
	return SOAP_OK;
}

/*
desc      : set channels upgrade status
param in  : chn --channel
                 flag -- upgrade state
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.05.04
*/
int ns__chan_upgrade_status_set(struct soap *soap, int chn, int flag, char *id, struct ns__gsoap_api_rsp_t *rsp){
	
	rsp->result = swg_chan_mod_set_upgrade_status(chn&0xFFFF, flag, id);
	if ( rsp->result < 0 ){
		BSP_PRT( ERR,"[%s %d] swg_chan_mod_set_upgrade_status failed, chn = %d, res = %d\n", __FILE__, __LINE__, chn, rsp->result);
		return SOAP_OK;
	}
	
	BSP_PRT( INFO,"[%s:%s:%d] set chn[%d] upgrade state ok, chn[%d] is %supgrade status!\n", __FILE__,__func__ ,__LINE__, chn, chn, flag?"":" not ");
	return SOAP_OK;
}

#endif

#if T_DESC("debug")
int ns__bsp_server_debug_level(struct soap *soap, int value, struct ns__gsoap_api_rsp_t *rsp ){
	bsp_prt_unmask = value;
	rsp->result = 0;
	return SOAP_OK;
}

#endif

#if T_DESC("debug_uart")

int ns__brd_set_debug_uart(struct soap *soap, int idx, int enable, struct ns__gsoap_api_rsp_t *rsp ){
	rsp->result = swg_set_debug_uart(idx, enable);
	if(rsp->result < 0){
		BSP_PRT( ERR,"[%s %d] swg_set_debug_uart failed,  idx= %d, res = %d\n", __FILE__, __LINE__, idx, rsp->result);
		return SOAP_OK;
	}
        rsp->result = 0;
	BSP_PRT( INFO,"[%s:%s:%d] set mod_brd[%d] debug uart ok, mod_brd[%d] is %sdebug status!\n", __FILE__,__func__ ,__LINE__, idx, idx, enable?"":" not ");
	return SOAP_OK;
}
/*
desc      : get systype
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.08.02
*/
int ns__bsp_get_sys_type(struct soap *soap, struct ns__gsoap_api_rsp_t *rsp){
	int systype;
	systype = swg_get_sys_type();
	
	if(rsp->value == NULL){
		rsp->value = (char *)soap_malloc(soap, 32);
		if(rsp->value == NULL){
			rsp->result = -1;
			return SOAP_OK;
		}
	}
	sprintf(rsp->value, "%d", systype);
	rsp->result = 0;
	BSP_PRT( INFO,"[%s:%s:%d] get systype ok, systype=%d!\n", __FILE__,__func__ ,__LINE__, systype);
	return SOAP_OK;
}

#endif

#if T_DESC("led")


/*
desc      : set channel sig led status
param in  : chan -- channel, -1 means all channels
			status -- led status
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.08.03
*/

int ns__chan_led_set_sig(struct soap *soap, int chan, int status, struct ns__gsoap_api_rsp_t *rsp ){
	rsp->result = vs_usb_set_sig_led_map(chan&0xFFFF, status);
	if(rsp->result < 0){
		BSP_PRT( ERR,"[%s %d] set chan[%d] signal led failed, status = %d, res = %d\n", __FILE__, __LINE__, chan, status, rsp->result);
		return SOAP_OK;
	}
	rsp->result = 0;
	BSP_PRT( INFO,"[%s:%s:%d] set chan[%d] signal led ok, status = %d!\n", __FILE__,__func__ ,__LINE__, chan, status);
	return SOAP_OK;
}
/*
desc      : set channel work led status
param in  : chan -- channel, -1 means all channels
			status -- led status
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.08.03
*/

int ns__chan_led_set_work(struct soap *soap, int chan, int status, struct ns__gsoap_api_rsp_t *rsp ){
	rsp->result = vs_usb_set_work_led_map(chan&0xFFFF, status);
	if(rsp->result < 0){
		BSP_PRT( ERR,"[%s %d] set chan[%d] work led failed, status = %d, res = %d\n", __FILE__, __LINE__, chan, status, rsp->result);
		return SOAP_OK;
	}
	rsp->result = 0;
	BSP_PRT( INFO,"[%s:%s:%d] set chan[%d] work led ok, stauts = %d!\n", __FILE__,__func__ ,__LINE__, chan, status);
	return SOAP_OK;
}

/*
desc      : set all channels led on or off
param in  : status -- 0 means off
                      1 means on
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.08.03
*/

int ns__chan_led_set_all(struct soap *soap, int status, struct ns__gsoap_api_rsp_t *rsp ){
	rsp->result = vs_usb_set_all_led(status);
	if(rsp->result < 0){
		BSP_PRT( ERR,"[%s %d] vs_usb_set_all_led turn %s all led failed, res = %d\n", __FILE__, __LINE__, status ? "on":"off", rsp->result);
		return SOAP_OK;
	}
	rsp->result = 0;
	BSP_PRT( INFO,"[%s:%s:%d] turn %s all led ok!\n", __FILE__,__func__ ,__LINE__, status ? "on":"off");
	return SOAP_OK;
}

/*
desc      : set mod_brd system led on or off
param in  : status -- 0 means off
                      1 means on
            index  -- mod brd index
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.09.22
*/
int ns__mod_brd_led_set_sys(struct soap *soap, int index, int status, struct ns__gsoap_api_rsp_t *rsp ){

	rsp->result = vs_usb_set_mod_sys_led_map(index&0xFFFF, status);
	if(rsp->result < 0){
		BSP_PRT( ERR,"[%s %d] set mod brd[%d] work led failed, status = %d, res = %d\n", __FILE__, __LINE__, index, status, rsp->result);
		return SOAP_OK;
	}
	rsp->result = 0;

	BSP_PRT( INFO,"[%s:%s:%d] set mod brd[%d] work led ok, stauts = %d!\n", __FILE__,__func__ ,__LINE__, index, status);

	return SOAP_OK;
	
}

int ns__set_sim_card_slot(struct soap *soap, int chan, int slot, struct ns__gsoap_api_rsp_t *rsp ){

	rsp->result = swg_set_sim_card_slot(chan&0xFFFF, slot);
	if(rsp->result < 0){
		BSP_PRT( ERR,"[%s %d] set chn[%d] sim slot failed, slot = %d, res = %d\n", __FILE__, __LINE__, chan, slot, rsp->result);
		return SOAP_OK;
	}
	rsp->result = 0;

	BSP_PRT( INFO,"[%s:%s:%d] set chn[%d] sim slot ok, slot = %d!\n", __FILE__,__func__ ,__LINE__, chan, slot);

	return SOAP_OK;
	
}

int ns__get_sim_state_all(struct soap *soap, int chan, struct ns__gsoap_api_rsp_t *rsp ){
	int state[32][4] = {0};

	chan = chan & 0xFFFF;
	int channel = 0;
	
	if(chan == 0xFFFF)
		channel = swg_get_total_chan_num();
	else
		channel = 1;
	
	rsp->result = swg_get_sim_state_all(chan, state);
	if(rsp->result < 0){
		rsp->cnt = 0;
		BSP_PRT( ERR,"[%s %d] get chn[%d] sim state failed, res = %d\n", __FILE__, __LINE__, chan, rsp->result);
		return SOAP_OK;
	}
	if ( rsp->value == NULL )
	{
	    rsp->value = soap_malloc(soap, channel * 4+1);
	    if ( rsp->value == NULL )
	        return SOAP_ERR;
	}
	int i = 0, j = 0;
	 memset( rsp->value,0,channel*4+1);

	int cnt = channel*4;
	int count = 0;
    /* ×ª³É×Ö·û´®ÔÙ´«Êä */
    	for ( i = 0; i < channel; i++ ){
		for(j = 0; j < 4; j++){
			rsp->value[count] = (char)((!state[i][j]) + CODEC_VALUE);
			count++;
		}
    	}
	rsp->result = RET_OK;
	rsp->cnt = cnt;
	rsp->result = 0;

	BSP_PRT( INFO,"[%s:%s:%d] get chn[%d] sim state ok, slot = %d!\n", __FILE__,__func__ ,__LINE__, chan);

	return SOAP_OK;
	
}


int ns__get_sim_state_one(struct soap *soap, int chan, int card, struct ns__gsoap_api_rsp_t *rsp ){
	int state[32] = {0};

	chan = chan & 0xFFFF;
	int channel = 0;
	if(chan == 0xFFFF)
		channel = swg_get_total_chan_num();
	else
		channel = 1;
	
	rsp->result = swg_get_sim_state_one(chan, card,state);
	if(rsp->result < 0){
		rsp->cnt = 0;
		BSP_PRT( ERR,"[%s %d] get chn[%d] sim state failed, res = %d\n", __FILE__, __LINE__, chan, rsp->result);
		return SOAP_OK;
	}
	if ( rsp->value == NULL )
	{
	    rsp->value = soap_malloc(soap, channel * 4+1);
	    if ( rsp->value == NULL )
	        return SOAP_ERR;
	}
	int i = 0, j = 0;
	 memset( rsp->value,0,channel*4+1);

	int cnt = channel*4;
	int count = 0;
    /* ×ª³É×Ö·û´®ÔÙ´«Êä */
    	for ( i = 0; i < channel; i++ ){
		rsp->value[count] = (char)((!state[i]) + CODEC_VALUE);
		count++;
    	}
	rsp->result = RET_OK;
	rsp->cnt = cnt;
	rsp->result = 0;

	BSP_PRT( INFO,"[%s:%s:%d] get chn[%d] sim state ok, slot = %d!\n", __FILE__,__func__ ,__LINE__, chan);

	return SOAP_OK;
	
}
#endif

int ns__get_sim_version(struct soap *soap, int index, struct ns__gsoap_api_rsp_t *rsp)
{

	struct simswitch_ver_info_s info;
	
	if(RET_OK != (rsp->result = swg_get_simswitch_info(index,&info))) //º¯Êý»ñÈ¡infoÊ§°Ü´¦Àí£¿
	{
		BSP_PRT( ERR,"[%s %d] swg_get_mod_brd_ver_info failed,res = %d\n", __FILE__, __LINE__, rsp->result);
		return SOAP_OK;
	}

	if ( rsp->value == NULL )
	{
		rsp->value = soap_malloc(soap, 64);
		if ( rsp->value == NULL )
			return SOAP_ERR;
	}

	memset(rsp->value, 0, 64);
	snprintf(rsp->value, 64, "BOARD_TYPE:SWG_SIM_SWITCH_1_4\nHW_VER:V%d.%d\nSW_VER:V%d.%d\n",
			(int)GET_VERSION_MAJOR(info.hw_ver),(int)GET_VERSION_MINOR(info.hw_ver),
			(int)GET_VERSION_MAJOR(info.sw_ver),(int)GET_VERSION_MINOR(info.sw_ver));
	rsp->cnt = strlen(rsp->value);
	rsp->result = RET_OK;
	BSP_PRT( INFO,"[%s:%s:%d] get sim version ok, version is %s!\n", __FILE__,__func__ ,__LINE__, rsp->value);
	return SOAP_OK;
}
