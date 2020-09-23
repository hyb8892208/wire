#include "mms_inc.h"
#include "mms_debug.h"
#define HW_INFO_CFG_CONF "/tmp/hw_info.cfg"
typedef enum _sys_type_e_                                                                                                                                                                                        
{
    SYS_TYPE_UNKOWN = 0,
    SYS_TYPE_SWG20XX,
    SYS_TYPE_1CHAN4SIMS,
    SYS_TYPE_VS_USB,
    SYS_TYPE_VS2_X8,
    SYS_TYPE_WG,
/*  SYS_TYPE_VS_1008,*/
}SYS_TYPE_E;

typedef struct hw_info_cfg_s{
	int total_chans;
	SYS_TYPE_E sys_type;
}hw_info_cfg_t;

hw_info_cfg_t *hw_info_cfg=NULL;


void mms_hw_info_init(void)
{
/*
[sys]
product_type=8
sys_type=3
hw_ver=1.0
chan_count=16
total_chan_count=20
*/
	char *linebuf;
	char *pos;
	size_t size = 0;
	if(hw_info_cfg)
		free(hw_info_cfg);
	hw_info_cfg = (hw_info_cfg_t *)malloc(sizeof(hw_info_cfg_t));
	if(hw_info_cfg == NULL){
		MMS_LOG_PRINT(LOG_ERROR, "malloc hw_info failed\n");
		return;
	}
	if(access(HW_INFO_CFG_CONF, F_OK) != 0)
		return;
	FILE *handle = fopen(HW_INFO_CFG_CONF, "r");
	if(handle == NULL){
		MMS_LOG_PRINT(LOG_ERROR, "open file failed\n");
		return ;
	}
	while(getline(&linebuf, &size, handle) != -1 ){
		if(linebuf[0] == '#')
			continue;
		if(strstr(linebuf, "sys_type")){
			pos = strstr(linebuf, "=");
			if(pos)
				hw_info_cfg->sys_type = atoi(pos+1);
		}else if(strstr(linebuf, "total_chan_count")){
			pos = strstr(linebuf, "=");
			if(pos)
				hw_info_cfg->total_chans = atoi(pos+1);
			break;
		}
	}
	//	printf("sys_type = %d, total_chan=%d\n", hw_info_cfg->sys_type,hw_info_cfg->total_chans);
	free(linebuf);
	fclose(handle);
}

void mms_hw_info_deinit(void)
{
	if(hw_info_cfg)
		free(hw_info_cfg);
	hw_info_cfg = NULL;
}


int mms_get_devport(int channel)
{
	int mod_brd_chans = 1;
	int dev_port = 0;
	if(channel <= 0)
		return -1;
	if(channel > hw_info_cfg->total_chans)
		return -1;
	switch(hw_info_cfg->sys_type){
		case SYS_TYPE_SWG20XX:
		case SYS_TYPE_1CHAN4SIMS:
			mod_brd_chans = 16;
			break;
		case SYS_TYPE_VS_USB:
			mod_brd_chans = 4;
			break;
		case SYS_TYPE_VS2_X8:
			mod_brd_chans = 8;
			break;
		case SYS_TYPE_WG:
			mod_brd_chans = 1;
			break;
		default:
			mod_brd_chans = 1;
			break;
	}
	
	dev_port = (channel - 1)/mod_brd_chans ;
	return dev_port;
}

int mms_get_total_chans(){
	return hw_info_cfg->total_chans;
}

