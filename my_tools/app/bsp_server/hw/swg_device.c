
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "swg_device.h"
#include "swg_intf.h"
#include "bsp_tools.h"
#include "mod_brd_regs.h"
#include "bspsched.h"
#include "sim_device.h"
//#define VS_USB_M35_LED_DEBUG
/* 无线模块名称，由通道模块板返回，不可随意修改 */
#define CHAN_MOD_NAME_M35        "Quectel_M35"
#define CHAN_MOD_NAME_M26        "Quectel_M26"
#define CHAN_MOD_NAME_SIM6320C   "SIMCOM_SIM6320C"
#define CHAN_MOD_NAME_EC20F       "EC20CE"
#define CHAN_MOD_NAME_UC15        "Quectel_UC15"
#define CHAN_MOD_NAME_EC25E        "EC25EF"

/* SWG设备通道号等于SWG_ALL_CHANS表示SWG所有通道 */
#define SWG_ALL_CHANS          (0xFFFF)    
/* 模块板通道号等于MB_ALL_CHANS表示模块板所有通道 */
#define MB_ALL_CHANS            (0xFF)

/* 所有模块板 */
#define SWG_ALL_MODS SWG_ALL_CHANS 

/* SIM操作构造函数返回值 */
#define SIM_OPS_DFT_RET (-128)

/* 检测到复位按键按下，执行以下命令复位系统 */
#define SYS_RESET_CMD   "/my_tools/restore_cfg_file > /dev/null 2>&1 &"

static struct swg_device SwgDev;
static struct swg_device *g_swg_dev;

/**** 全局函数，变量声明 ***/
int chan_brd_rx_cmd_ret(int fd, int *mod_id, char *ret_buf, int len, int *actual_len);
void chan_brd_tx_cmd(int fd, char *cmd, int len, int mod_id);
int chan_brd_rx_at_ret(int fd, int *mod_id, char *ret_buf, int len, int *actual_len);
void chan_brd_tx_at(int fd, char *cmd, int len, int mod_id);
int mod_brd_select_upgrade_chan(struct swg_mod_brd *mb, int chan);
int mod_brd_upgrade_chan_is_selected(struct swg_mod_brd *mb, int chan);
int swg_chan_mod_all_power_on_off(int chan);
void swg_chan_vbat_sched(void *data);
void swg_chan_mod_power_on_off_detect(void *data);
int swg_get_mod_brd_solt_num(int idx, int *solt_num);
int swg_get_chan_powerkey_status(struct swg_device *dev);

#if SWG_TEST
int swg_device_test(void);
#endif

struct sim_card_ops sim_m35_ops_v010000;
struct sim_card_ops sim_m35_ops_v020000;
struct sim_card_ops sim_6320c_ops_v010000;
struct sim_card_ops sim_6320c_ops_v020000;
struct sim_card_ops sim_ec20f_ops_v010000;
struct sim_card_ops sim_ec20f_ops_v020000;
struct sim_card_ops sim_uc15_ops_v010000;
struct sim_card_ops sim_uc15_ops_v020000;


/*设备名称和槽位号对应关系*/
//2U机箱
mcu_dev_t mcu_dev_2u[MAX_MOD_BRD_NUM]  = 
{
	{"1-1.2.5.1", 1},
	{"1-1.2.6.6.1", 2},
	{"1-1.2.6.5.1", 3},
	{"1-1.2.1.1", 4},
	{"1-1.2.2.1", 5},
	{"1-1.2.6.1.1", 6},
	{"1-1.2.6.2.1", 7},
	{"1-1.2.3.1", 8},
	{"1-1.2.4.1", 9},
	{"1-1.2.6.4.1", 10},
	{"1-1.2.6.3.1", 11},
	{"", 0}
};
//1U机箱
mcu_dev_t mcu_dev_1u[MAX_MOD_BRD_NUM]  = 
{
	{"1-1.2.5.1", 1},
	{"1-1.2.6.6.1", 0},
	{"1-1.2.6.5.1", 0},
	{"1-1.2.1.1", 2},
	{"1-1.2.2.1", 3},
	{"1-1.2.6.1.1", 0},
	{"1-1.2.6.2.1", 0},
	{"1-1.2.3.1", 4},
	{"1-1.2.4.1", 5},
	{"1-1.2.6.4.1", 0},
	{"1-1.2.6.3.1", 0},
	{"", 0}
};
mcu_dev_t mcu_dev_vs1008[MAX_MOD_BRD_NUM]  = 
{
	{"1-1.2.5.1", 0},
	{"1-1.2.6.6.1", 0},
	{"1-1.2.6.5.1", 0},
	{"1-1.2.1.1", 1},
	{"1-1.2.2.1", 2},
	{"1-1.2.6.1.1", 0},
	{"1-1.2.6.2.1", 0},
	{"1-1.2.3.1", 0},
	{"1-1.2.4.1", 0},
	{"1-1.2.6.4.1", 0},
	{"1-1.2.6.3.1", 0},
	{"", 0}
};
/**** 全局函数，变量声明结束***/

/* 无线通信模块类型转字符串名称 */
static inline char *chan_mod_type_to_str(CHAN_MOD_TYPE_E type)
{
    switch (type)
    {
        case CHAN_MOD_TYPE_UNKOWN :
            return "CHAN_MOD_TYPE_UNKOWN";
            break;

        case CHAN_MOD_M35:
            return CHAN_MOD_NAME_M35;
            break;

        case CHAN_MOD_SIM6320C:
            return CHAN_MOD_NAME_SIM6320C;
            break;
        case CHAN_MOD_EC20F:
			return CHAN_MOD_NAME_EC20F;
			break;
		case CHAN_MOD_UC15:
			return CHAN_MOD_NAME_UC15; 
			break;
		case CHAN_MOD_EC25E:
			return CHAN_MOD_NAME_EC25E;
			break;
		case CHAN_MOD_M26:
			return CHAN_MOD_NAME_M26;
			break;
        default:
            BSP_PRT(ERR, "[%s] unknow channel module type %d\n", __FUNCTION__, type);
            return "CHAN_MOD_TYPE_UNKOWN";
            break;
    }
}

/* 通道单片机协议类型 */
static inline char *chan_brd_type_to_str(CHAN_MOD_TYPE_E type)
{
    switch (type)
    {
        case CHAN_BRD_PROTOCOL_UNKOWN :
            return "unkown";
            break;

        case CHAN_BRD_PROTOCOL_HDLC:
            return "hdlc";
            break;

        case CHAN_BRD_PROTOCOL_COBS:
            return "cobs";
            break;
        default:
            BSP_PRT(ERR, "[%s] unknow channel brd protocol type %d\n", __FUNCTION__, type);
            return "CHAN_MOD_TYPE_UNKOWN";
            break;
    }
}

/* 模块板枚举类型转字符串名称 */
static inline char *mb_name_to_str(MOD_BRD_NAME_E name)
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
         *[>目前16口与32口一样, 都是HW_SWG_1032_BASE <]
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

        default:
            BSP_PRT(ERR, "[%s] unknow hw name %d\n", __FUNCTION__, name);
            return "NAME_UNKOWN";
            break;
    }
}
/*************************************************
  函数描述 : 将全局通道号转为模块板下标和此模块板上的通道号
  输入参数 : dev -- SWG device
             gchan -- 全局通道号（从1开始编号）
  输出参数 : md_idx -- 模块板下标（从0开始编号）
             md_chan -- 此模块板上的通道号（从0开始编号）
  函数返回 :  0 -- 成功
             <0 -- 失败
  备注     ：调用此函数前须完成dev->mod_brd[], dev->total_chans初始化。
  作者/时间 : junyu.yang@openvox.cn/2018.02.01
*************************************************/
static int gchan_2_md_idx_chan(struct swg_device *dev, int gchan,  int *md_idx, int *md_chan)
{
    int i;
    
    if(gchan > dev->total_chans)
    {
        BSP_PRT(ERR, "Invalied channel number %d(total %d) \n", gchan, dev->total_chans);
        return -1;
    }
    for(i = 0; i < dev->total_mod_brds; i++)
    {
       if(MB_STATUS_INITED != dev->mod_brd[i].status)
           continue;

       if((gchan - 1) >= dev->mod_brd[i].base_chan_no 
          && (gchan - 1) < (dev->mod_brd[i].base_chan_no + dev->mod_brd[i].channels))
       {
           /* 找到 */
           if(NULL != md_idx)
               *md_idx = i;
           if(NULL != md_chan)
               *md_chan = gchan - dev->mod_brd[i].base_chan_no - 1; //从0开始编号
           return 0;
       }
    }

    return -2;
}

static int cstr_emergency_power_off(struct swg_mod_brd *brd, int chan)
{
    BSP_PRT( ERR,"Unkown module\n");
    return SIM_OPS_DFT_RET;
}

static int cstr_translate_vbat(struct swg_mod_brd *mb, int md_idx, int *reg_value_l, int *reg_value_h){

    BSP_PRT( ERR,"Unkown module\n");
    return SIM_OPS_DFT_RET;

}
static int cstr_translate_vbat_status(struct swg_mod_brd *mb, int md_idx, int *reg_value){

    BSP_PRT( ERR,"Unkown module\n");
    return SIM_OPS_DFT_RET;

}

static int cstr_translate_power_status(struct swg_mod_brd *mb, int md_idx, int *reg_value){

    BSP_PRT( ERR,"Unkown module\n");
    return SIM_OPS_DFT_RET;

}


struct sim_card_ops sim_ops_constructor =
{
    .emergency_power_off            = cstr_emergency_power_off,
    .translate_vbat                  = cstr_translate_vbat,
    .translate_vbat_status           = cstr_translate_vbat_status,
    .translate_power_status          = cstr_translate_power_status,
};

/************************************************
                  导出全局函数                       
************************************************/
/* Valid global channel number */
static inline int is_valid_g_chan_no( struct swg_device *dev, int chan)
{
    if((chan == SWG_ALL_CHANS)
       || ((0 < chan)  && (chan <= dev->total_chans)))
    {
        return 1;
    }

    return 0;
}

/* Valid global module board index */
static inline int is_valid_g_md_idx( struct swg_device *dev, int md_idx)
{
    if((0 < md_idx) || (md_idx <= dev->total_mod_brds))
    {
        return 1;
    }

    return 0;
}

/*************************************************
  函数描述 : 将全局通道号转为模块板下标和此模块板上的通道号
  输入参数 : dev -- SWG device
             gchan -- 全局通道号（从1开始编号）
  输出参数 : md_idx -- 模块板下标（从1开始编号）
             md_chan -- 此模块板上的通道号（从1开始编号）
  函数返回 :  0 -- 成功
             <0 -- 失败
  作者/时间 : junyu.yang@openvox.cn/2018.02.01
*************************************************/
int swg_gchan_2_mb_idx_chan(int chan, int *md_idx, int *md_chan)
{
    int res;

    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }

   res = gchan_2_md_idx_chan(g_swg_dev, chan, md_idx, md_chan);
   (*md_idx)++;    /*  从1开始编号 */
   (*md_chan)++;   /*  从1开始编号 */

   return res;
}

/*************************************************
  函数描述 : 初始化SWG设备信息。
  输入参数 : dev -- SWG设备结构体
  函数返回 :  =0 成功, <0 失败。
  作者/时间: junyu.yang@openvox.cn / 2018.01.31
*************************************************/
static int swg_init_dev_info(struct swg_device *dev)
{
  
    int i = 0;

    for(i = 0; i < dev->total_mod_brds; i++)
    {
        if(MB_SWG_1032_BASE == dev->mod_brd[i].name)
        {
            if(dev->total_chans <= 16)
            {
                dev->name = SWG_DEV_1016;
                break;
            }
            else if(dev->total_chans <= 32)
            {
                dev->name = SWG_DEV_1032;
                break;
            }
        }
        else if(MB_SWG_VS_USB_BASE== dev->mod_brd[i].name)
        {
            if(dev->total_chans <= 8)
            {
                dev->name = SWG_DEV_VS_USB_1008;
                break;
            }
            else if(dev->total_chans <= 20)
            {
                dev->name = SWG_DEV_VS_USB_1020;
                break;
            }
            else if(dev->total_chans <= 44)
            {
                dev->name = SWG_DEV_VS_USB_1044;
                break;
            }
        }else if(MB_SWG_VS2_USB_DOWN == dev->mod_brd[i].name){
			if(dev->total_chans <= 64){
				dev->name = SWG_DEV_VS2_8X_1064;
			}
		}
    }

    dev->version = VERSION_NUMBER(1,0,0); /* version is 1.0.0 */

    return 0;
}

/*************************************************
  函数描述 : 初始化模块板版本信息，包括名称，硬件版本，软件版本。
  输入参数 : mb -- 模块板结构体
  函数返回 : =0 成功, <0 失败。
  作者/时间: junyu.yang@openvox.cn / 2018.01.30
*************************************************/
static int mod_brd_init_ver_info(struct swg_mod_brd *mb)
{
    int i, res;
    int len = 0;

    char *p = NULL;
    char info_buf[256] = {0};
    char ver_buf[32] = {0};
    unsigned char major = 0, minor = 0, bugfix = 0;

    char *name_swg_1004_base = "NOT DEFINE"; /* 目前没定义 */
    char *name_swg_1008_base = "NOT DEFINE"; /* 目前没定义 */
    /*char *name_swg_1016_base = "SWG1016_BASED"; [> 16口与32口一样，都是1032 <]*/
    char *name_swg_1032_base = "SWG1032_BASED";
	char *name_vs2_8x_down = "VS2_8XEC20_DOWN";
    char *name_VS_USB_base = "VS_USB_BASED"; /* 4口可插拔模块(sim6320c)*/
    char *hw_head = "HwVer : V";
    char *sw_head = "SwVer : V";
    char *endptr = NULL;

    /* 初始化版本信息 */
    mb->name = MB_NAME_UNKOWN;
    mb->hw_ver = 0;
    mb->sw_ver = 0;

    /* 获取版本信息 */
    if (( res = hwif_get_verison_info(&mb->hwif, info_buf, sizeof(info_buf), &len)) != 0)
    {
        BSP_PRT(ERR, "Module board %d get version info failed, res = %d\n", mb->index, res);
        return -1;
    }


    BSP_PRT(VERB, "Module board %d info_buf:\n%s\n", mb->index, info_buf);
    /*
     *获取到版本内容(info_buf)格式如下
     *SWG1032_BASED
     *HwVer : V2.0
     *SwVer : V2.0
     *Jan  8 2018
     *17:32:28
     */


    /* 解析版本信息, 模块名称，硬件、软件版本号 */
    /* 1, 获取模块名称 */
    if(NULL != (strstr(info_buf, name_swg_1004_base)))
    {
        mb->name = MB_SWG_1004_BASE;
    }
    else if(NULL != (strstr(info_buf, name_swg_1008_base)))
    {
        mb->name = MB_SWG_1008_BASE;
    }
    else if(NULL != (strstr(info_buf, name_swg_1032_base)))
    {
        mb->name = MB_SWG_1032_BASE;
    }
    else if(NULL != (strstr(info_buf, name_VS_USB_base)))
    {
        mb->name = MB_SWG_VS_USB_BASE;
    }
	else if(NULL != (strstr(info_buf, name_vs2_8x_down)))
	{
		mb->name = MB_SWG_VS2_USB_DOWN;
	}
    else
    {
        mb->name = MB_NAME_UNKOWN;
        BSP_PRT(WARN, "Module board %d unkown board name, info_buf:\n%s\n", 
                mb->index, info_buf);
    }
    BSP_PRT(VERB, "Module board %d name = 0x%x\n", mb->index, mb->name);

    /* 2, 获取硬件版本 */
    if (NULL != (p = strstr(info_buf, hw_head)))
    {
        p += strlen(hw_head);

        i = 0;
        while ( *p && (i < sizeof(ver_buf)) && (*p != '\n') && (*p != '\r') )
            ver_buf[i++] = *p++;

        ver_buf[i] = '\0';

        major = strtol(ver_buf, &endptr, 10) & 0xFF;
        p = ++endptr; /* skip '.' */
        minor = strtol(p, &endptr, 10) & 0xFF;

        bugfix = 0; /* bugfix not define yet ,so allways = 0 */

        mb->hw_ver =  VERSION_NUMBER(major, minor, bugfix);

        BSP_PRT(VERB, "Module board %d hw_ver = 0x%x\n", mb->index, mb->hw_ver);
    }

    /* 3, 获取软件版本 */
    if (NULL != (p = strstr(info_buf, sw_head)))
    {
        p += strlen(sw_head);

        i = 0;
        while ( *p && (i < sizeof(ver_buf)) && (*p != '\n') && (*p != '\r') )
            ver_buf[i++] = *p++;

        ver_buf[i] = '\0';

        major = strtol(ver_buf, &endptr, 10) & 0xFF;
        p = ++endptr; /* skip '.' */
        minor = strtol(p, &endptr, 10) & 0xFF;

        bugfix = 0; /* bugfix not define yet ,so allways = 0 */
        mb->sw_ver =  VERSION_NUMBER(major, minor, bugfix);

        BSP_PRT(VERB, "Module board %d sw_ver = 0x%x\n", mb->index, mb->sw_ver);
    }

    BSP_PRT(INFO, "Module board %d name: %s, hw_ver: %d.%d.%d, sw_ver: %d.%d.%d\n",
            mb->index, mb_name_to_str(mb->name),
            GET_VERSION_MAJOR(mb->hw_ver),
            GET_VERSION_MINOR(mb->hw_ver),
            GET_VERSION_BUGFIX(mb->hw_ver),

            GET_VERSION_MAJOR(mb->sw_ver),
            GET_VERSION_MINOR(mb->sw_ver),  
            GET_VERSION_BUGFIX(mb->sw_ver)); 

    return 0;
}

/*************************************************
  函数描述 : 初始化一个模块板
  输入参数 : mb -- struct swg_mod_brd结构 
             swg_dev -- mb 所在swg_device
             idx -- mb 在 swg_dev中的下标
  函数返回 :  =0  成功
              <0  失败
*************************************************/
static int init_one_mod_brd(struct swg_mod_brd *mb, struct swg_device *swg_dev, int idx)
{
    int res = 0;
    int baud = 9600;
    char cmd_buf[PATH_MAX] = {0};

    mb->index = idx;
    mb->swg_dev = swg_dev;
    
    /* 初始化硬件接口 */
    memset(mb->hwif.name, 0, sizeof(mb->hwif.name));
    memset(cmd_buf, 0, sizeof(cmd_buf));
    snprintf(mb->hwif.name, sizeof(mb->hwif.name), "%s%d", MOD_BRD_PATH, mb->index);

    if(0 != access(mb->hwif.name, F_OK))
    {
        mb->status = MB_STATUS_NOT_INSERT;
        BSP_PRT(VERB, "Mod brd %d(%s) is not exists\n",mb->index, mb->hwif.name);
        return -1;
    }
    mb->hwif.fd = open_serial(mb->hwif.name, 9600, 0, 8, 1, 0);
    if (mb->hwif.fd < 0)
    {
		mb->status = MB_STATUS_NOT_INSERT;
        BSP_PRT(ERR, "Open %s failed: %s\n",mb->hwif.name, strerror(errno));
        return -1;
    }

    /* 设置接口波特率为9600 */
    sprintf(cmd_buf, "stty -F %s 9600",mb->hwif.name);
    system(cmd_buf);

    pthread_mutex_init(&mb->lock, NULL);
    mb->status = MB_STATUS_INSERT;

    /* 获取版本信息 */
    if((res = (mod_brd_init_ver_info(mb))) != 0) 
    {
		//尝试设置为115200
		close(mb->hwif.fd);
		mb->hwif.fd = open_serial(mb->hwif.name, 115200, 0, 8, 1, 0);
		if (mb->hwif.fd < 0)
		{
			mb->status = MB_STATUS_NOT_INSERT;
			BSP_PRT(ERR, "Open %s failed: %s\n",mb->hwif.name, strerror(errno));
			return -1;
		}
		/* 设置接口波特率为115200 */
		sprintf(cmd_buf, "stty -F %s 115200",mb->hwif.name);
		system(cmd_buf);
		if((res = (mod_brd_init_ver_info(mb))) != 0) {
			BSP_PRT(ERR, "Module board %d init ver info failed, res = %d\n", mb->index, res);
			mb->status = MB_STATUS_INIT_FAILED;
			close(mb->hwif.fd);
			return -2;
		}
		baud = 115200;
    }
	if(baud == 9600 && ((mb->name == MB_SWG_1032_BASE && mb->sw_ver >= VERSION_NUMBER(2, 2, 0))||
		(mb->name == MB_SWG_VS_USB_BASE && mb->sw_ver >= VERSION_NUMBER(1, 1, 0))))
	{
		//设置波特率为115200
		if(mod_brd_set_baud(&mb->hwif, 1) < 0){
			return -3;
		}
		close(mb->hwif.fd);
		mb->hwif.fd = open_serial(mb->hwif.name, 115200, 0, 8, 1, 0);
		if (mb->hwif.fd < 0)
		{
			mb->status = MB_STATUS_NOT_INSERT;
			BSP_PRT(ERR, "Open %s failed: %s\n",mb->hwif.name, strerror(errno));
			return -1;
		}
		/* 设置接口波特率为9600 */
		sprintf(cmd_buf, "stty -F %s 115200",mb->hwif.name);
		system(cmd_buf);
	}
    /*初始化因版本而异的信息*/
    /* 默认同设备，每个模块上通道数目相等 */
    if(MB_SWG_1032_BASE == mb->name)
    {
		mb->channels = 16;
		mb->base_chan_no = mb->channels * idx;
		if(swg_dev->sys_type  == SYS_TYPE_UNKOWN)
			swg_dev->sys_type = SYS_TYPE_SWG20XX;
    }
    else if(MB_SWG_VS_USB_BASE == mb->name)
    {
		mb->channels = 4;
		mb->base_chan_no = mb->channels * idx;
		if(swg_dev->sys_type == SYS_TYPE_UNKOWN)
			swg_dev->sys_type = SYS_TYPE_VS_USB;
    }
	else if(MB_SWG_VS2_USB_DOWN == mb->name){
		mb->channels = 8;
		mb->base_chan_no = mb->channels * idx;
		if(swg_dev->sys_type == SYS_TYPE_UNKOWN)
			swg_dev->sys_type = SYS_TYPE_VS2_USB;
	} 
    else
    {
        BSP_PRT(ERR, "Module board %d unknow board name: %s(%d)\n",
                mb->index, mb_name_to_str(mb->name), mb->name);
        mb->status = MB_STATUS_INIT_FAILED;
        close(mb->hwif.fd);
        return -4;
    }

    mb->status = MB_STATUS_INITED;

    return 0;
}

/*************************************************
  函数描述 : 初始化模块板
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  0>= 实际初始化成功的模块板数
              <0  失败
*************************************************/
static int swg_init_mod_brd(struct swg_device *dev)
{
    int i = 0, res, cnt = 0;

    dev->total_mod_brds = 0;
	dev->sys_type = SYS_TYPE_UNKOWN;
    for (i = 0; i < MAX_MOD_BRD_NUM; i++)
    {

        if((res = init_one_mod_brd(&dev->mod_brd[i], dev, i)) != 0)
        {
            BSP_PRT(VERB, "Module board %d init failed, res = %d\n", i, res);
            continue;
        }
        /* 更新最后初始化成功模块板 */
        dev->total_mod_brds = i + 1;
        cnt++;
    }

    return cnt;
}

static int swg_deinit_mod_brd(struct swg_device *dev)
{
    int i;

    for(i = 0; i < g_swg_dev->total_mod_brds; i++)
    {
        if(MB_STATUS_INITED == g_swg_dev->mod_brd[i].status)
            close(g_swg_dev->mod_brd[i].hwif.fd);
    }

    return 0;
}
/*************************************************
  函数描述 : 初始化通道板以及与初始化通道中与通道板相关的信息
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  =0  成功
              <0  失败
*************************************************/
static int swg_init_exec_cmd(char *cmd, int len, char *result){
	FILE *fd =popen(cmd, "r");
	if(fd == NULL){
		BSP_PRT(ERR, "exec cmd faild, cmd:\"%s\"", cmd);
		return -1;
	}
	if(fread(result, 1, len, fd) < 0){
		BSP_PRT(ERR, "read solt info, cmd: \"%s\"", cmd);
		return -1;
	}
	fclose(fd);
	return 0;
}

static int swg_init_get_chan_brd_get_total_chan(struct swg_device *dev){
	int total_chans = 0;
	int i, j;
	int slot_num = 0;
	char buf[1024] = {0};
	char cmd_buf[256] = {0};
//	if(dev->mod_brd->name == MB_SWG_1032_BASE){
	if(dev->sys_type == SYS_TYPE_SWG20XX){
		if(dev->total_mod_brds == 1){
			total_chans = 16;
		}else if(dev->total_mod_brds == 2){
			total_chans = 32;
		}
	} else if(dev->sys_type == SYS_TYPE_VS_USB) {
		if(dev->total_mod_brds > 1){
			for(i  = 2; i <= dev->total_mod_brds; i++){//??һ????λ????????
				if(dev->mod_brd[i-1].name != MB_NAME_UNKOWN){//?ж??Ƿ??в???ģ????
					if(0 == swg_get_mod_brd_solt_num( i, &slot_num)){//??ȡ??λ??
						if(slot_num < 1)
						{
							total_chans = 8;
//							dev->sys_type = SYS_TYPE_VS_1008;
							BSP_PRT(WARN, "mod brd total chans:%d, slot_num=%d\n",total_chans, slot_num);
							break;
						}
						snprintf(cmd_buf, sizeof(cmd_buf), "udevadm info -q path -n %s%d", MOD_BRD_PATH, i-1);
						swg_init_exec_cmd(cmd_buf, 1024 , buf);
						for(j = 1; j < MAX_MOD_BRD_NUM; j++){
							if(strstr(buf, mcu_dev_2u[j].node_name)){//ƥ??????
								if( mcu_dev_1u[j].slot_num == slot_num) {
									total_chans = 20;
								}else if( mcu_dev_2u[j].slot_num == slot_num){//ƥ????λ??
									total_chans = 44;
								}else if( mcu_dev_vs1008[j].slot_num == slot_num){//ƥ????λ??
									total_chans = 8;
								} else {
									total_chans = 8;
//									dev->sys_type = SYS_TYPE_VS_1008;
								}
								BSP_PRT(INFO, "mod brd total chans:%d, j=%d \n",total_chans, j);
								break;
							}
						}
					}
					if(total_chans != 0)
						break;
				}else{
					BSP_PRT(INFO, "mod brd %d is not availd!\n", i - 1);
				}
			}
		} else if(dev->total_mod_brds == 1) {
			i = 1;
			if(0 == swg_get_mod_brd_solt_num( i, &slot_num)){//??ȡ??λ??
				if(slot_num < 1)
				{
					total_chans = 8;
//					dev->sys_type = SYS_TYPE_VS_1008;
					BSP_PRT(WARN, "mod brd total chans:%d, slot_num=%d\n",total_chans, slot_num);
				}
				snprintf(cmd_buf, sizeof(cmd_buf), "udevadm info -q path -n %s%d", MOD_BRD_PATH, i-1);
				swg_init_exec_cmd(cmd_buf, 1024 , buf);
				for(j = 0; j < MAX_MOD_BRD_NUM; j++){
					if(strstr(buf, mcu_dev_2u[j].node_name)){//ƥ??????
						if( mcu_dev_1u[j].slot_num == slot_num) {
							total_chans = 20;
						}else if( mcu_dev_2u[j].slot_num == slot_num){//ƥ????λ??
							total_chans = 44;
						}else if( mcu_dev_vs1008[j].slot_num == slot_num){//ƥ????λ??
							total_chans = 8;
						} else {
							total_chans = 8;
//							dev->sys_type = SYS_TYPE_VS_1008;
						}
						BSP_PRT(INFO, "mod brd total chans:%d \n",total_chans);
						break;
					}
				}
			}
		}
	}else if(dev->sys_type == SYS_TYPE_VS2_USB){
		total_chans=32;
	}

	return total_chans;
}

static int swg_init_get_chan_brd_get_total_brd(struct swg_device *dev){
	int i = 0;
	for(i = 0 ; i < 11; i++){
		if(dev->mod_brd[i].name != MB_NAME_UNKOWN)
			dev->total_mod_brds = i+1;
	}
	return dev->total_mod_brds;
}
static int swg_init_dev_mod_chan_brd_info(struct swg_device *dev){
	dev->total_mod_brds = swg_init_get_chan_brd_get_total_brd(dev);
	if(dev->total_mod_brds <= 0){
		return -1;
	}
	dev->total_chans = swg_init_get_chan_brd_get_total_chan(dev);
	return dev->total_chans;
}

static int swg_init_chan_brd_channel(struct swg_device *dev)
{
    int i = 0, chan_num = 0;
	int flag = 0;
    int mod_id, actual_len;
    int fds[MAX_CHAN_NUM];
    char file_name[PATH_MAX] = {0};
    char buf[1024];
	int j = 0;
	char *p = NULL, *endptr = NULL;
	char *sw_head = "SW ver: ver_";
	char ver_buf[32] = {0};
	int  major, minor, bugfix;
	int try_count = 0;
	
	dev->total_chans = swg_init_get_chan_brd_get_total_chan(dev);
	
	if(dev->total_chans == 0){
		flag = 1;
		chan_num = MAX_CHAN_BRD_NUM;
	}else{
		flag = 0;
		chan_num = dev->total_chans / 2;
	}

	for(i = 0; i < chan_num; i++)
	{
		memset(file_name, 0, sizeof(file_name));
		snprintf(file_name, sizeof(file_name), "%s%d", CHAN_BRD_PATH, i);

		if(0 != access(file_name, F_OK))
		{
			fds[i] = -1;
			BSP_PRT(VERB, "Chan brd %d(%s) is not exists\n", i, file_name);
			continue;
		}

		fds[i] = open_serial(file_name, 115200, 0, 8, 1, 0);
		if (fds[i] < 0)
		{
			BSP_PRT(ERR, "Open %s failed: %s\n", file_name, strerror(errno));
			/* 没有通道模块板 */
			dev->chan_brd[i].status = CB_STATUS_NOT_INSERT;

			dev->chans[i*2].mod_type = CHAN_MOD_TYPE_UNKOWN;
			dev->chans[i*2 + 1].mod_type = CHAN_MOD_TYPE_UNKOWN;
			BSP_PRT(VERB, "Channel module %d (chan %d, %d) is %s\n", 
				i, i*2, i*2+1, chan_mod_type_to_str(dev->chans[i*2].mod_type));
			continue;
		}

		usleep(100 * 1000);

		/* 1个通道5块板上有2个通道,且通道类型相同 */
		/* 初始化通道板 */
		dev->chan_brd[i].index = i;
		dev->chan_brd[i].channels= 2;
		dev->chan_brd[i].base_chan_no = i * 2;
		dev->chan_brd[i].status = CB_STATUS_NOT_INSERT;
		dev->chan_brd[i].sw_ver = 0;
		dev->chan_brd[i].protocol = CHAN_BRD_PROTOCOL_UNKOWN;
		dev->total_chan_brds = i + 1;
		chan_brd_tx_cmd_cobs(fds[i], "ver", 3, 0); //获取版本
		/* 初始化通道中与通道板相关的信息 */
		memset(buf, 0, sizeof(buf));
		if(chan_brd_rx_cmd_ret_cobs(fds[i], &mod_id, buf, sizeof(buf), &actual_len, (try_count + 1) * 20) < 0 || actual_len <= 64){
			chan_brd_tx_cmd_hdlc(fds[i], "ver", 3, 0);
			chan_brd_rx_cmd_ret_hdlc(fds[i], &mod_id, buf, sizeof(buf), &actual_len, (try_count + 1) * 20);
		}
		BSP_PRT(VERB,"Channel module %d type info: \n%s\n", i, buf);

		if (strstr(buf, CHAN_MOD_NAME_SIM6320C))
		{
			dev->chans[i*2].mod_type = CHAN_MOD_SIM6320C;
			dev->chans[i*2 + 1].mod_type = CHAN_MOD_SIM6320C;
		}
		else if (strstr(buf,CHAN_MOD_NAME_M35))

		{
			dev->chans[i*2].mod_type = CHAN_MOD_M35;
			dev->chans[i*2 + 1].mod_type = CHAN_MOD_M35;
		}
		else if(strstr(buf, CHAN_MOD_NAME_EC20F))
			{
			dev->chans[i*2].mod_type = CHAN_MOD_EC20F;
			dev->chans[i*2 + 1].mod_type = CHAN_MOD_EC20F;
		}else if(strstr(buf, CHAN_MOD_NAME_EC25E))
		{
			dev->chans[i*2].mod_type = CHAN_MOD_EC25E;
			dev->chans[i*2 + 1].mod_type = CHAN_MOD_EC25E;
		}
		else if (strstr(buf, CHAN_MOD_NAME_UC15)){	
			dev->chans[i*2].mod_type = CHAN_MOD_UC15;
			dev->chans[i*2 + 1].mod_type = CHAN_MOD_UC15;
		}else if(strstr(buf, CHAN_MOD_NAME_M26)){
			dev->chans[i*2].mod_type = CHAN_MOD_M26;
			dev->chans[i*2 + 1].mod_type = CHAN_MOD_M26;
		}
		else 
		{
			dev->chans[i*2].mod_type = CHAN_MOD_TYPE_UNKOWN;
			dev->chans[i*2 + 1].mod_type = CHAN_MOD_TYPE_UNKOWN;
			BSP_PRT(WARN,"Channel module %d unkown type: \n%s\n", i, buf);
		}

		dev->chans[i*2].chan_brd = &dev->chan_brd[i];
		dev->chans[i*2 + 1].chan_brd = &dev->chan_brd[i];
		
		
		if(CHAN_MOD_TYPE_UNKOWN != dev->chans[i*2].mod_type && flag == 1) 
		dev->total_chans = (i + 1) * 2;
		/*get chan_brd sw version*/
		if (NULL != (p = strstr(buf, sw_head)))
		{
			p += strlen(sw_head);

			j = 0;
			while ( *p && (j < sizeof(ver_buf)) && (*p != '\n') && (*p != '\r') )
		    ver_buf[j++] = *p++;

			ver_buf[j] = '\0';

			major = strtol(ver_buf, &endptr, 10) & 0xFF;
			p = ++endptr; /* skip '.' */
			minor = strtol(p, &endptr, 10) & 0xFF;

			bugfix = 0; /* bugfix not define yet ,so allways = 0 */
			dev->chan_brd[i].sw_ver =  VERSION_NUMBER(major, minor, bugfix);

			BSP_PRT(DBG, "Chan board %d sw_ver = 0x%x\n", i+1, dev->chan_brd[i].sw_ver);
		}
		
		if(dev->chan_brd[i].sw_ver >= VERSION_NUMBER(3, 1, 0)){
			dev->chan_brd[i].protocol = CHAN_BRD_PROTOCOL_COBS;
		}else{
			dev->chan_brd[i].protocol = CHAN_BRD_PROTOCOL_HDLC;
		}
		BSP_PRT(DBG, "Channel module %d (chan %d, %d) is %s\n", 
		              i, i*2, i*2+1, chan_mod_type_to_str(dev->chans[i*2].mod_type));
		close(fds[i]);
	}

    return 0;
}

/*************************************************
  函数描述 : 初始化所有通道
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  0>= 实际初始化成功的通道数
              <0  失败
*************************************************/
static int swg_init_channel(struct swg_device *dev)
{
    int i, md_idx, md_chan;

    /* 获取通道无线模块类型 */
     swg_init_chan_brd_channel(dev);
    for(i = 0; i< dev->total_chans; i++)
    {
        dev->chans[i].chan_no = i;
        dev->chans[i].upgrade_flag = NOT_UPGRADE_STATUS; 
        if((gchan_2_md_idx_chan(dev, i+1, &md_idx, &md_chan)) != 0)
        {
            /* 标志此模块不可用 */
            dev->chans[i].mod_type = CHAN_MOD_TYPE_UNKOWN;
            continue;
        }

        dev->chans[i].mod_brd = &dev->mod_brd[md_idx];
		dev->mod_brd[md_idx].simcard_evet[md_chan] = CHAN_MOD_SIM_UNKOWN;
		dev->mod_brd[md_idx].vbat_event[md_chan] = CHAN_MOD_VBAT_UNKOWN;
		dev->mod_brd[md_idx].power_event[md_chan] = CHAN_MOD_POWER_UNKOWN;
		dev->mod_brd[md_idx].power_next_event[md_chan] = CHAN_MOD_POWER_UNKOWN;
		dev->mod_brd[md_idx].powerkey_event[md_chan] = CHAN_MOD_POWERKEY_UNKOWN;
        /* 初始化sim card操作，包括无效通道 */
        if(MB_SWG_1032_BASE == dev->chans[i].mod_brd->name)
        {
            if(CHAN_MOD_M35 == dev->chans[i].mod_type)
            {
                if(VERSION_NUMBER(1,0,0) == dev->mod_brd[md_idx].hw_ver)
                {
                    dev->chans[i].sim_ops = &sim_m35_ops_v010000;
                }
                else if (VERSION_NUMBER(2,0,0) <= dev->mod_brd[md_idx].hw_ver)
                {
                    dev->chans[i].sim_ops = &sim_m35_ops_v020000;
                }
            } 
            else if(CHAN_MOD_SIM6320C == dev->chans[i].mod_type)
            {
                if(VERSION_NUMBER(1,0,0) == dev->mod_brd[md_idx].hw_ver)
                {
                    dev->chans[i].sim_ops = &sim_6320c_ops_v010000;
                }
                else if (VERSION_NUMBER(2,0,0) <= dev->mod_brd[md_idx].hw_ver)
                {
                    dev->chans[i].sim_ops = &sim_6320c_ops_v020000;
                }

            }
			else if(CHAN_MOD_EC20F == dev->chans[i].mod_type)
			{
				dev->chans[i].sim_ops = &sim_ec20f_ops_v020000;
           	}
			else if(CHAN_MOD_EC25E == dev->chans[i].mod_type)
			{
				dev->chans[i].sim_ops = &sim_ec20f_ops_v020000;
			}
			else if(CHAN_MOD_UC15 == dev->chans[i].mod_type)
			{
				dev->chans[i].sim_ops = &sim_uc15_ops_v020000;
			}
			else if(CHAN_MOD_M26 == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_m35_ops_v020000;
			}
            else
            {
                dev->chans[i].sim_ops = &sim_ops_constructor;
            }
        }
        else if(MB_SWG_VS_USB_BASE == dev->chans[i].mod_brd->name)
        {
            if(CHAN_MOD_SIM6320C == dev->chans[i].mod_type)
            {
                dev->chans[i].sim_ops = &sim_6320c_ops_v010000;
            }
	        else if(CHAN_MOD_M35 == dev->chans[i].mod_type)
            {
  				dev->chans[i].sim_ops = &sim_m35_ops_v010000;
            }
            else if(CHAN_MOD_EC20F == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_ec20f_ops_v010000;
			}
			else if(CHAN_MOD_EC25E == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_ec20f_ops_v010000;
			}
			else if(CHAN_MOD_UC15 == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_uc15_ops_v010000;
			}
			else if(CHAN_MOD_M26 == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_uc15_ops_v010000;
			}
			else
			{
				dev->chans[i].sim_ops = &sim_ops_constructor;
			}

		}else if(MB_SWG_VS2_USB_DOWN == dev->chans[i].mod_brd->name){
			if(CHAN_MOD_SIM6320C == dev->chans[i].mod_type)
			{
				dev->chans[i].sim_ops = &sim_6320c_ops_v010000;
			}
			else if(CHAN_MOD_M35 == dev->chans[i].mod_type)
			{
				dev->chans[i].sim_ops = &sim_m35_ops_v010000;
			}
			else if(CHAN_MOD_EC20F == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_ec20f_ops_v010000;
			}
			else if(CHAN_MOD_EC25E == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_ec20f_ops_v010000;
			}
			else if(CHAN_MOD_UC15 == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_uc15_ops_v010000;
			}
			else if(CHAN_MOD_M26 == dev->chans[i].mod_type){
				dev->chans[i].sim_ops = &sim_uc15_ops_v010000;
			}
			else
			{
				dev->chans[i].sim_ops = &sim_ops_constructor;
			}
		}
		else
		{
			dev->chans[i].sim_ops = &sim_ops_constructor;
		}
		
        dev->chans[i].sim_evt = SIMCARD_EVENT_UNKOWN;
        dev->chans[i].sim_prv_evt = SIMCARD_EVENT_UNKOWN;
    }

    return dev->total_chans;
}


/*************************************************
  函数描述 : swg检测任务，定期检测swg设备变化
  输入参数 : arg -- 创建线程里传入的参数  
  函数返回 : 无 
*************************************************/
static void *swg_detect_task(void *arg)
{
    int res, i, total_chans;
    int status[MAX_CHAN_NUM] = {0};
    int reset_key_status = 0;
    struct swg_device *dev = (struct swg_device *)arg;

    /* 使能外部注销线程 */
    if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0)
    {
        BSP_PRT(ERR, "Detect task enable cancel failed: %s\n", strerror(errno));
        return arg;
    }

    while(1)
    {
        /* 检测SIM卡插拔事件 */
		
        res = swg_get_sim_card_insert_status(SWG_ALL_CHANS, status);
		
        if(0 != res)
        {
            BSP_PRT(ERR, "Detect task get insert status failed, res = %d\n", res);
        }
        else
        {
             total_chans = swg_get_total_chan_num();
             for(i = 0; i < total_chans; i++)
             {
                 if(dev->chans[i].mod_type == CHAN_MOD_TYPE_UNKOWN)
                     continue;

                 /*printf("[%s %d] chan %d status = %d\n", __FUNCTION__, __LINE__, i, status[i]);*/
                 if((dev->chans[i].sim_prv_evt != SIMCARD_EVENT_INSERT)
                    && (1 == status[i]))
                 {
                     /* 之前不是插入事件，现在检测到卡在槽位，说明有插入事件发生 */
                     BSP_PRT(DBG, "Chan %d sim card prev report event = %d\n",
                             i, dev->chans[i].sim_evt);
                     dev->chans[i].sim_prv_evt = SIMCARD_EVENT_INSERT;
                     dev->chans[i].sim_evt = SIMCARD_EVENT_INSERT;
                     BSP_PRT(DBG, "Chan %d sim card curr report event = %d\n",
                             i, dev->chans[i].sim_evt);

                 }
                 else if((dev->chans[i].sim_prv_evt != SIMCARD_EVENT_REMOVE)
                        && (0 == status[i]))
                 {
                     /* 之前不是拔出事件，现在检测到卡不在槽位，说明有拔出事件发生 */
                     BSP_PRT(DBG, "Chan %d sim card prev report event = %d\n",
                             i, dev->chans[i].sim_evt);
                     dev->chans[i].sim_prv_evt = SIMCARD_EVENT_REMOVE;
                     dev->chans[i].sim_evt = SIMCARD_EVENT_REMOVE;
                     BSP_PRT(DBG, "Chan %d sim card curr report event = %d\n",
                             i, dev->chans[i].sim_evt);
                 }
                 else
                 {
                     /* 如果卡状态没有改变，则没有事件发生 */
                     dev->chans[i].sim_evt = SIMCARD_EVENT_UNKOWN;
                 }

             }
        }

		swg_chan_mod_power_on_off_detect(dev);
		
        if((SWG_DEV_1016 == dev->name)
           || (SWG_DEV_1032 == dev->name))
        {
            /* 只有第1块模块板硬件上与复位按键相连，因此只检测第一块模块板 */
            if(0 == swg_get_mod_brd_reset_key_status(1,  &reset_key_status))
            {
                if( 1 == reset_key_status)
                {
                    BSP_PRT(INFO, "one key restore occur!\r\n");
                    BSP_PRT(INFO, "Execute \"%s\" reset system\n", SYS_RESET_CMD);
					system("/my_tools/add_syslog \"Factory reset from RST button [bsp].\"");
                    system(SYS_RESET_CMD);
                }
            }
        }


        if(stdout != bsp_log_fd)
            fflush(bsp_log_fd); /* 将log 刷到磁盘上*/


        usleep(dev->dtct_intv * 1000); /* 睡眠 */
    }

    return arg;
}


/*************************************************
  函数描述 : 初始化swg检测任务
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  =0 成功
              <0 失败
*************************************************/
static int swg_init_detect(struct swg_device *dev)
{
    int res;
    pthread_attr_t attr;
    const int stack_size = 256 * 1024; /* 256 K */

    /* Initialize thread creation attributes */
    res = pthread_attr_init(&attr);
    if (0 != res)
    {
        BSP_PRT(ERR, "Init detect thread attr failed, %s\n", strerror(errno));
        return -1;
    }

    res = pthread_attr_setstacksize(&attr, stack_size);
    if (0 != res)
    {
        BSP_PRT(ERR, "Init detect set thread size failed, %s\n", strerror(errno));
        return -2;
    }

    dev->dtct_intv = 2000;
    res = pthread_create(&dev->dt_id, &attr, &swg_detect_task, dev);
    if (0 != res)
    {
        BSP_PRT(ERR, "Init detect create thread failed, %s\n", strerror(errno));
        return -3;
    }

    return 0;
}

/*************************************************
  函数描述 : 销毁swg检测任务
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  =0 成功
              <0 失败
*************************************************/
static int swg_deinit_detect(struct swg_device *dev)
{
    int res;
    void *ret;

    /* 注销检测线程 */
    if(0 == dev->dt_id)
    {
        BSP_PRT(DBG, "Detect task not start yet\n");
        return 0;
    }

    res = pthread_cancel(dev->dt_id);
    if (0 != res)
    {
        BSP_PRT(ERR, "Cancel detect task failed: %s\n", strerror(errno));
        return -1;
    }

    /* Join with thread to see what its exit status was */
    res = pthread_join(dev->dt_id, &ret);
    if (0 != res)
    {
        BSP_PRT(ERR, "Join cancel detect task failed: %s\n", strerror(errno));
        return -2;
    }

    if (ret == PTHREAD_CANCELED)
    {
        BSP_PRT(DBG, "Detect task is canceled.\n");
    }
    else
    {
        BSP_PRT(ERR, "Cancel detect task failed\n");
        return -3;
    }

    return 0;
}

/*************************************************
  函数描述 :  生成swg 所有通道类型文件
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  =0 成功
              <0 失败
  备注     ：FIXME
             此文件由于历史原因应用层需要此文件，所以尽管
             swg_gen_hw_info()已经生成通道类型文件，但还是
             重复生成一遍，如果应用层修改，可以删除此函数.
*************************************************/
static int swg_gen_chan_type_info_file(struct swg_device *dev)
{
    int i;
    struct swg_channel *chan;
    FILE *fd;

    if((fd = fopen(SWG_CHAN_TYPE_FILE, "w+")) == NULL)
    {
        BSP_PRT(ERR, "Open %s failed: %s\n", SWG_CHAN_TYPE_FILE, strerror(errno));
        return -1;
    }

    for(i = 0; i < dev->total_chans; i++)
    {
        chan = &dev->chans[i];
        if(CHAN_MOD_TYPE_UNKOWN == chan->mod_type)
        {
            /* 无法识别的通道 */
            fprintf(fd, "null");
        }
        else
        {
            fprintf(fd, "%s", chan_mod_type_to_str(chan->mod_type));
        }

        /* 除最后一个通道外，末尾加',' */
        if(i < (dev->total_chans - 1));
            fprintf(fd, ",");
    }

    fprintf(fd, "\n"); /* 隔一行 */

    fclose(fd);

    return 0;
}

/*************************************************
  函数描述 :  生成swg 所有通道数据文件
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  =0 成功
              <0 失败
  备注     ：FIXME
             此文件由于历史原因应用层需要此文件，所以尽管
             swg_gen_hw_info()已经生成通道类型文件，但还是
             重复生成一遍，如果应用层修改，可以删除此函数.
*************************************************/
static int swg_gen_chan_total_num_file(struct swg_device *dev)
{
    FILE *fd;

    if((fd = fopen(SWG_TOTAL_CHAN_FILE, "w+")) == NULL)
    {
        BSP_PRT(ERR, "Open %s failed: %s\n", SWG_TOTAL_CHAN_FILE, strerror(errno));
        return -1;
    }

    fprintf(fd, "%d\n", dev->total_chans);

    fclose(fd);

    return 0;
}
/*************************************************
  函数描述 :  生成swg 所有硬件信息到文件中
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  =0 成功
              <0 失败
*************************************************/
static int swg_gen_hw_info(struct swg_device *dev)
{
    int i, cnt, mod_brd_idx = 0;
    char file_name[PATH_MAX];
    FILE *fd;
    struct swg_mod_brd *mb;
    struct swg_channel *chan;
    const int nb = 1; /* number base, 起始编号，从0或1开始 */

    if((fd = fopen(SWG_HW_INFO_FILE, "w+")) == NULL)
    {
        BSP_PRT(ERR, "Open %s failed: %s\n", SWG_HW_INFO_FILE, strerror(errno));
        return -1;
    }

    /* 标题 */
    fprintf(fd, "%s", 
                  "\n#Start with '#' is comment line\n\n"
                  "# SWG(Standalone Wireless Gateway) hardware information\n"
                  "#\n"
                  "# Autogenerated by 'bsp_server'\n"
                  "# If you edit this file and execute 'bsp_server' again\n"
                  "# your manual changes will be LOST.\n\n");

    /* 文件格式说明 */
    fprintf(fd, 
"###################\n"
"### FILE fomate ###\n"
"###################\n\n"
"# product_type: \n"
"# Value ProductionName      Note\n"
"#   0   SWG_UNKOWN          /* Unkown production */\n"
"#   1   SWG_1001            /* 1 Port Wireless Gateway */\n"
"#   2   SWG_1002            /* 2 Ports Wireless Gateway */\n"
"#   3   SWG_1004,           /* 4 Ports Wireless Gateway */\n"
"#   4   SWG_1008,           /* 8 Ports Wireless Gateway */\n"
"#   5   SWG_1016,           /* 16 Ports Wireless Gateway */\n"
"#   6   SWG_1032,           /* 32 Ports Wireless Gateway */\n"
"#   7   SWG_1064,           /* 64 Ports Wireless Gateway */\n"
"#   8   SWG_VS_USB_1020,    /* 20 Ports Wireless Gateway, USB, 1U Chassis */\n"
"#   9   SWG_VS_USB_1044,    /* 44 Ports Wireless Gateway, USB, 2U Chassis*/\n"
"#   10  SWG_VS_USB_1008,    /* 8 or 4 Ports Wireless Gateway, USB, vs1008 Chassis*/\n"
"#   11  SWG_VS2_8X_1064,    /* 32 or 64 Ports Wirless Gateway, VS2_8X*/\n"
"#\n"
"# sys_type: \n"
"# Value Type\n"
"#   0   Unkown production \n"
"#   1   1 channel corresponds to 1 SIM slot \n"
"#   2   1 channel corresponds to 4 SIM slots */\n"
"#   3   4 Ports, USB pluggable(SWG_VS_USB_xxx) */\n"
"#\n"
"#    [sys]                       # System paraters\n"
"#    product_type=7              # Production type，64 Ports Wireless Gateway \n"
"#    sys_type=1                  # 1 channel corresponds to 1 SIM slot \n"
"#    hw_ver=2.0                  # Production haraware version\n"
"#    chan_count=36               # Total initied channel numbers\n"
"#    total chan_count=40         # Total channel numbers, include inited and not inited channel\n"
"#\n"
"#    [mod_brd]                   #module board lable\n"
"#    mod_brd_1=/dev/xx           #module board 1 MCU absolute path\n"
"#    brd_1_totoal_chan=16        #module board 1 total channels，include inited and not inited channel\n"
"#    brd_1_base_chan_no=1        #module board 1 the first channel number on the system\n"
"#\n"
"#    mod_brd_2=/dev/xx           #module board 2 MCU absolute path\n"
"#    brd_2_totoal_chan=4         #module board 2 channels，include inited and not inited channel\n"
"#    brd_2_base_chan_no=17       #module board 2 the first channel number on the system\n"
"#\n"
"#    ....\n"
"#\n"
"#    mod_brd_11=/dev/xx          #module board 11 MCU absolute path\n"
"#    brd_11_totoal_chan=4        #module board 11 channels，include inited and not inited channel\n"
"#    brd_11_base_chan_no=70      #module board 11 the first channel number on the system\n"
"#\n"
"#\n"
"#    [channel]                   #channel Lable.The channle number is global, start from 1\n"
"#    chan_1=/dev/xx              #channel 1 MCU absolute path\n"
"#    chan_1_type=Quectel_M35     #channel 1 channel wireless moudule type\n"
"#    chan_1_brd_id=1             #belonged to module board 1\n"
"#    chan_1_protocol=hdlc        #Channel 1 transmission protocol is hdlc\n"
"#\n"
"#    chan_2=/dev/xx              #channel 2 MCU absolute path\n"
"#    chan_2_type=Quectel_M35     #channel 2 channel wireless moudule type\n"
"#    chan_2_brd_id=1             #belonged to module board 1\n"
"#    chan_2_protocol=hdlc        #Channel 1 transmission protocol is hdlc\n"
"#\n"
"#\n"
"#    ....\n"
"#\n"
"#    chan_64=/dev/xx                 #channel 64 MCU absolute path\n"
"#    chan_64_type=SIMCOM_SIM6320C    #channel channel wireless moudule type\n"
"#    chan_64_brd_id=2                #belonged to module board 2\n"
"#    chan_64_protocol=hdlc                #belonged to module board hdlc\n"
"#\n"
"#    ############ Define not inited(unkonw/not inserted) channel ##########\n"
"#    chan_2=/tmp/opvx_chan_unkown    #channle MCU absolute path, all operations of this file is undefined\n"
"#    chan_2_type=type_unkown         #unkonw type\n"
"#    chan_2_brd_id= -1               #belonged to module board -1(allways -1)\n"
"#    ######################################################\n"
"#\n"
"#    [emu]               # EMU Lable\n"
"#    emu_1=/dev/xx       # EMU 1 absolute path on system\n"
"#    emu_2=/dev/xx       # EMU 2 absolute path on system\n"
"#\n"
"#    [lcd]               # lcd Lable\n"
"#    lcd_1=/dev/xx       # lcd 1 absolute path on system\n"
"#\n"
"#    [upgrade]           # upgrade channel wireless moudle lable\n"
"#    upgrade_1=/dev/xx   # upgrade interface 1 absolute path on system\n"
    );
    /*****************************/

    fprintf(fd, "\n"); /* 隔一行 */
    /* 系统参数 */
    cnt = 0;
    for(i = 0; i < dev->total_chans; i++)
    {
        if(CHAN_MOD_TYPE_UNKOWN != dev->chans[i].mod_type)
            cnt++;
    }
/*    if(SWG_DEV_UNKOWN == dev->name)
    {
        sys_type = 0;
    }
    else if((SWG_DEV_VS_USB_1020 == dev->name)
            || (SWG_DEV_VS_USB_1044 == dev->name))
    {
        sys_type = 3;
    }
    else
    {
        sys_type = 1;
    }*/
	for(i = 0; i < dev->total_mod_brds; i++){
		if(dev->mod_brd[i].status == MB_STATUS_INITED){
			mod_brd_idx = i;
			break;
		}
	}
    fprintf(fd, "[sys]\n"
                "product_type=%d\n" 
                "sys_type=%d\n" 
				"hw_ver=%d.%d\n"
                "chan_count=%d\n"
                "total_chan_count=%d\n",
                dev->name,
                dev->sys_type,
				GET_VERSION_MAJOR(dev->mod_brd[mod_brd_idx].hw_ver),GET_VERSION_MINOR(dev->mod_brd[mod_brd_idx].hw_ver),
                cnt,
                dev->total_chans);

    fprintf(fd, "\n"); /* 隔一行 */

    /* 模块板 */
    fprintf(fd, "[mod_brd]\n"); /* 标签 */

    for(i = 0; i < dev->total_mod_brds; i++)
    {
        mb = &dev->mod_brd[i];
        if(MB_STATUS_INITED == mb->status)
        {
            fprintf(fd, "mod_brd_%d=%s%d\n"
                        "mod_%d_total_chan=%d\n" 
                        "mod_%d_base_chan_no=%d\n",
                        i + nb, MOD_BRD_PATH, i,
                        i + nb, mb->channels,
                        i + nb, mb->base_chan_no + nb);
        }
        /* 模块板初始化失败不输出 */
    }

    fprintf(fd, "\n"); /* 隔一行 */

    /* 通道 */
    fprintf(fd, "[channel]\n"); /* 标签 */

    for(i = 0; i < dev->total_chans; i++)
    {
        chan = &dev->chans[i];
        if(CHAN_MOD_TYPE_UNKOWN == chan->mod_type)
        {
            /* 无法识别的通道 */
            fprintf(fd, "chan_%d=%s\n"
                        "chan_%d_type=%s\n" 
                        "chan_%d_brd_id=%d\n"
                        "chan_%d_protocol=%s\n",
                        i + nb, "/tmp/opvx_chan_unkown",
                        i + nb, "unkown",
                        i + nb, -1,
                        i + nb, "unkown");
        }
        else
        {
            fprintf(fd, "chan_%d=%s%d\n"
                        "chan_%d_type=%s\n" 
                        "chan_%d_brd_id=%d\n"
                        "chan_%d_protocol=%s\n",
                        i + nb, CHAN_BRD_PATH, chan->chan_brd->index,
                        i + nb, chan_mod_type_to_str(chan->mod_type),
                        i + nb, chan->mod_brd->index + nb,
                        i + nb, chan_brd_type_to_str(dev->chan_brd[i/2].protocol));
        }
        fprintf(fd, "\n"); /* 隔一行 */
    }

    /* EMU */
    fprintf(fd, "[emu]\n"); /* 标签 */
    cnt = nb;
    for(i = 0; i < dev->total_mod_brds * 2; i++) /* 每个模块板有2个EMU */
    {
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%s%d", EMU_PATH, i);
        if(0 == access(file_name, F_OK))
        {
            /* 文件存在 */
            fprintf(fd, "emu_%d=%s\n", cnt, file_name);
        }
		cnt++;
    }

    fprintf(fd, "\n"); /* 隔一行 */

    /* LCD */
    fprintf(fd, "[lcd]\n"); /* 标签 */
    memset(file_name, 0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), "%s%d", LCD_PATH, 0);
    if(0 == access(file_name, F_OK))
    {
        /* 文件存在, 整个设备只有一个LCD */
        fprintf(fd, "lcd_1=%s\n", file_name);
    }

    fprintf(fd, "\n"); /* 隔一行 */

    /* upgrade */
    fprintf(fd, "[upgrade]\n"); /* 标签 */
    cnt = nb;
    for(i = 0; i < dev->total_mod_brds; i++)
    {
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%s%d", UPGRADE_PATH, i);
        if(0 == access(file_name, F_OK))
        {
            /* 文件存在 */
            fprintf(fd, "upgrade_%d=%s\n", cnt++, file_name);
        }
    }

    fclose(fd);

    /* FIXME 生成由于历史原因应用层需要的文件 
     * 当应用层直接使用SWG_HW_INFO_FILE时，可以不生成这2个文件*/   
    swg_gen_chan_type_info_file(dev);
    swg_gen_chan_total_num_file(dev);
    return 0;
}


int vs_usb_led_init(struct swg_device *dev);
static int vs_usb_led_thread(struct swg_device *dev);
static int vs_usb_deinit_led_thread(struct swg_device *dev);

void *swg_handler_thead(void *data);

/*************************************************
*描述：启动模块板事件处理线程
*输入参数：swg_device 结构体指针
*返回值： 0 --创建模块板事件处理线程
*返回值：-1 --一个线程也没有创建成功
*************************************************/
int swg_event_thread(struct swg_device *dev){
	int i = 0;
	int res;
	struct swg_mod_brd *mb;
	for(i = 0; i < dev->total_mod_brds; i++){
		
		if(dev->mod_brd[i].status !=MB_STATUS_INITED){
			continue;
		}
		pthread_attr_t attr;
	    const int stack_size = 256 * 1024; /* 256 K */

	    /* Initialize thread creation attributes */
	    res = pthread_attr_init(&attr);
	    if (0 != res)
	    {
	        BSP_PRT(ERR, "Init detect thread attr failed, %s\n", strerror(errno));
	        return -1;
	    }

	    res = pthread_attr_setstacksize(&attr, stack_size);
	    if (0 != res)
	    {
	        BSP_PRT(ERR, "Init detect set thread size failed, %s\n", strerror(errno));
	        return -2;
	    }
		mb = &dev->mod_brd[i];
	    res = pthread_create(&mb->handler_id, &attr, &swg_handler_thead, mb);
	    if (0 != res)
	    {
	        BSP_PRT(ERR, "Init detect create thread failed, %s\n", strerror(errno));
	        return -3;
	    }
	}
	return 0;
}

int swg_chan_mod_powerkey_init(struct swg_device *dev){
	int i = 0, res;
	int md_idx, md_chan;
	struct swg_mod_brd *mb;

	swg_get_chan_powerkey_status(g_swg_dev);

	for(i = 0; i < dev->total_chans; i++){
		if((res = gchan_2_md_idx_chan(dev, i+1, &md_idx,  &md_chan))!= 0){
			BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n",
				__FUNCTION__, i+1, res);
			continue;
		}

		mb = &dev->mod_brd[md_idx];

		if(dev->chans[i].mod_type == CHAN_MOD_M35 
			|| dev->chans[i].mod_type == CHAN_MOD_SIM6320C
			|| dev->chans[i].mod_type == CHAN_MOD_M26 )
			mb->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_HIGH;
	}
	return 0;
}

int swg_mod_brd_init_channel(struct swg_mod_brd *mb){
	int i = 0;
	int chan_index = 0;
	int md_chan;
//	struct swg_mod_brd *mb = (struct swg_mod_brd *)data;
	struct swg_device *dev = mb->swg_dev;

	 for(i = 0; i< mb->channels; i++)
    {
    	chan_index = i + mb->base_chan_no;
		md_chan = i;
        dev->chans[chan_index].chan_no = chan_index;
        dev->chans[chan_index].upgrade_flag = NOT_UPGRADE_STATUS; 

        dev->chans[chan_index].mod_brd = mb;
		mb->simcard_evet[md_chan] = CHAN_MOD_SIM_UNKOWN;
		mb->vbat_event[md_chan] = CHAN_MOD_VBAT_UNKOWN;
		mb->power_event[md_chan] = CHAN_MOD_POWER_UNKOWN;
		mb->power_next_event[md_chan] = CHAN_MOD_POWER_UNKOWN;
		mb->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_UNKOWN;
        /* 初始化sim card操作，包括无效通道 */
        if(MB_SWG_1032_BASE == dev->chans[chan_index].mod_brd->name)
        {
			if(CHAN_MOD_M35 == dev->chans[chan_index].mod_type)
			{
				if(VERSION_NUMBER(1,0,0) == mb->hw_ver)
				{
					dev->chans[chan_index].sim_ops = &sim_m35_ops_v010000;
				}
				else if (VERSION_NUMBER(2,0,0) <= mb->hw_ver)
				{
					dev->chans[chan_index].sim_ops = &sim_m35_ops_v020000;
				}
			} 
			else if(CHAN_MOD_SIM6320C == dev->chans[chan_index].mod_type)
			{
				if(VERSION_NUMBER(1,0,0) == mb->hw_ver)
				{
					dev->chans[chan_index].sim_ops = &sim_6320c_ops_v010000;
				}
				else if (VERSION_NUMBER(2,0,0) <= mb->hw_ver)
				{
					dev->chans[chan_index].sim_ops = &sim_6320c_ops_v020000;
				}

			}
			else if(CHAN_MOD_EC20F == dev->chans[chan_index].mod_type)
			{
				dev->chans[chan_index].sim_ops = &sim_ec20f_ops_v020000;
			}
			else if(CHAN_MOD_EC25E == dev->chans[chan_index].mod_type)
			{
				dev->chans[chan_index].sim_ops = &sim_ec20f_ops_v020000;
			}
			else if(CHAN_MOD_UC15 == dev->chans[chan_index].mod_type)
			{
				dev->chans[chan_index].sim_ops = &sim_uc15_ops_v020000;
			}
			else if(CHAN_MOD_M26 == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_m35_ops_v020000;
			}
			else
			{
				dev->chans[chan_index].sim_ops = &sim_ops_constructor;
			}
        }
        else if(MB_SWG_VS_USB_BASE == dev->chans[chan_index].mod_brd->name)
        {
			if(CHAN_MOD_SIM6320C == dev->chans[chan_index].mod_type)
			{
				dev->chans[chan_index].sim_ops = &sim_6320c_ops_v010000;
			}
			else if(CHAN_MOD_M35 == dev->chans[chan_index].mod_type)
			{
				dev->chans[chan_index].sim_ops = &sim_m35_ops_v010000;
			}
			else if(CHAN_MOD_EC20F == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_ec20f_ops_v010000;
			}
			else if(CHAN_MOD_EC25E == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_ec20f_ops_v010000;
			}
			else if(CHAN_MOD_UC15 == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_uc15_ops_v010000;
			}
			else if(CHAN_MOD_M26 == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_uc15_ops_v010000;
			}
			else
			{
				dev->chans[chan_index].sim_ops = &sim_ops_constructor;
			}
        
        }
		else if(MB_SWG_VS2_USB_DOWN == dev->chans[chan_index].mod_brd->name){
			if(CHAN_MOD_SIM6320C == dev->chans[chan_index].mod_type)
			{
				dev->chans[chan_index].sim_ops = &sim_6320c_ops_v010000;
			}
			else if(CHAN_MOD_M35 == dev->chans[chan_index].mod_type)
			{
				dev->chans[chan_index].sim_ops = &sim_m35_ops_v010000;
			}
			else if(CHAN_MOD_EC20F == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_ec20f_ops_v010000;
			}
			else if(CHAN_MOD_EC25E == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_ec20f_ops_v010000;
			}
			else if(CHAN_MOD_UC15 == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_uc15_ops_v010000;
			}
			else if(CHAN_MOD_M26 == dev->chans[chan_index].mod_type){
				dev->chans[chan_index].sim_ops = &sim_uc15_ops_v010000;
			}
			else
			{
				dev->chans[chan_index].sim_ops = &sim_ops_constructor;
			}
		}
        else
        {
            dev->chans[chan_index].sim_ops = &sim_ops_constructor;
        }

        dev->chans[chan_index].sim_evt = SIMCARD_EVENT_UNKOWN;
        dev->chans[chan_index].sim_prv_evt = SIMCARD_EVENT_UNKOWN;
    }
	return 0;
}

static int swg_mod_brd_init_chan_type(struct swg_mod_brd *mb){

	int i; 
	int mb_dev_index;
	int mb_chan_brd_num;
	int mod_id = 0;
	char buf[1024] = {0};
	char file_name[PATH_MAX] = {0};
	int actual_len = 0;
	int chan_index;//模块对应swg_dev->chans[] 的下标
	int fds[32] = {0};
	int j = 0;
	char *p = NULL, *endptr = NULL;
	char *sw_head = "SW ver: ver_";
	char ver_buf[32] = {0};
	int  major, minor, bugfix;
	int try_count = 0;
	enum chan_brd_protocol_type_e protol = CHAN_BRD_PROTOCOL_UNKOWN;	
	
	struct swg_device *dev = mb->swg_dev;
	mb_chan_brd_num = mb->channels/2;//一个mcu控制两个模块
	mb_dev_index = mb->base_chan_no/2;// 模块板上第一个模块在/dev/open/chan_brd/下的编号
	for(i = 0 ; i < mb_chan_brd_num; i++)
	{
		int chan_1 = mb->base_chan_no+ i*2;//mcu第一个模块
		int chan_2 = mb->base_chan_no+ i*2 + 1 ;//mcu第二个模块
		memset(file_name, 0, sizeof(file_name));
		snprintf(file_name, sizeof(file_name), "%s%d", CHAN_BRD_PATH, i+mb_dev_index);

		if(0 != access(file_name, F_OK))
		{
			fds[i] = -1;
			BSP_PRT(VERB, "Chan brd %d(%s) is not exists\n", i, file_name);
			continue;
		}

		fds[i] = open_serial(file_name, 115200, 0, 8, 1, 0);
		if (fds[i] < 0)
		{
			dev->chan_brd[i+mb_dev_index].status = CB_STATUS_NOT_INSERT;
			dev->chans[chan_1].mod_type = CHAN_MOD_TYPE_UNKOWN;
			dev->chans[chan_2].mod_type = CHAN_MOD_TYPE_UNKOWN;
			BSP_PRT(ERR, "Open %s failed: %s\n", file_name, strerror(errno));
			continue;
		}

		usleep(100 * 1000);

		dev->chans[chan_1].mod_brd = mb;
		dev->chans[chan_2].mod_brd = mb;

		/* 初始化通道板 */
		dev->chan_brd[i+mb_dev_index].index = i+mb_dev_index;
		dev->chan_brd[i+mb_dev_index].channels= 2;
		dev->chan_brd[i+mb_dev_index].base_chan_no = chan_1;
		dev->chan_brd[i+mb_dev_index].status = CB_STATUS_NOT_INSERT;
		dev->chan_brd[i+mb_dev_index].sw_ver = 0;
		dev->chan_brd[i+mb_dev_index].protocol = CHAN_BRD_PROTOCOL_UNKOWN;
		
	//	dev->total_chan_brds = i + 1;
		/* 初始化通道中与通道板相关的信息 */
		memset(buf, 0, sizeof(buf));
		for(try_count = 0; try_count < 2; try_count++){
			if(protol == CHAN_BRD_PROTOCOL_UNKOWN || protol == CHAN_BRD_PROTOCOL_COBS){
				chan_brd_tx_cmd_cobs(fds[i], "ver", 3, 0); //获取版本
				if(chan_brd_rx_cmd_ret_cobs(fds[i], &mod_id, buf, sizeof(buf), &actual_len, (try_count+1)*20) < 0 || actual_len <= 64){
					chan_brd_tx_cmd_hdlc(fds[i], "ver", 3, 0);
					if(chan_brd_rx_cmd_ret_hdlc(fds[i], &mod_id, buf, sizeof(buf), &actual_len, (try_count+1)*20) < 0 || actual_len <= 64){
						continue;
					}else{
						protol = CHAN_BRD_PROTOCOL_HDLC;
					}
				}else{
					protol = CHAN_BRD_PROTOCOL_COBS;
				}
			}else{
				chan_brd_tx_cmd_hdlc(fds[i], "ver", 3, 0);
				if(chan_brd_rx_cmd_ret_hdlc(fds[i], &mod_id, buf, sizeof(buf), &actual_len, (try_count+1)*20) < 0 || actual_len <= 64){
					chan_brd_tx_cmd_cobs(fds[i], "ver", 3, 0); //获取版本
					if(chan_brd_rx_cmd_ret_cobs(fds[i], &mod_id, buf, sizeof(buf), &actual_len, (try_count+1)*20) < 0 || actual_len <= 64){
						continue;
					}else{
						protol = CHAN_BRD_PROTOCOL_COBS;
					}
				}else{
					protol = CHAN_BRD_PROTOCOL_HDLC;
				}
			}
			break;
		}
		BSP_PRT(VERB,"Channel module %d type info: \n%s\n", i, buf);

		if (strstr(buf, CHAN_MOD_NAME_SIM6320C))
		{
			dev->chans[chan_1].mod_type = CHAN_MOD_SIM6320C;
			dev->chans[chan_2].mod_type = CHAN_MOD_SIM6320C;
		}
		else if (strstr(buf,CHAN_MOD_NAME_M35))

		{
			dev->chans[chan_1].mod_type = CHAN_MOD_M35;
			dev->chans[chan_2].mod_type = CHAN_MOD_M35;
		}
		else if(strstr(buf, CHAN_MOD_NAME_EC20F))
		{
			dev->chans[chan_1].mod_type = CHAN_MOD_EC20F;
			dev->chans[chan_2].mod_type = CHAN_MOD_EC20F;
		}
		else if(strstr(buf, CHAN_MOD_NAME_EC25E))
		{
			dev->chans[chan_1].mod_type = CHAN_MOD_EC25E;
			dev->chans[chan_2].mod_type = CHAN_MOD_EC25E;
		}
		else if (strstr(buf, CHAN_MOD_NAME_UC15)){	
			dev->chans[chan_1].mod_type = CHAN_MOD_UC15;
			dev->chans[chan_2].mod_type = CHAN_MOD_UC15;
		}else if(strstr(buf, CHAN_MOD_NAME_M26)){
			dev->chans[chan_1].mod_type = CHAN_MOD_M26;
			dev->chans[chan_2].mod_type = CHAN_MOD_M26;
		}

		else 
		{
			dev->chans[chan_1].mod_type = CHAN_MOD_TYPE_UNKOWN;
			dev->chans[chan_2].mod_type = CHAN_MOD_TYPE_UNKOWN;
			BSP_PRT(WARN,"Channel module %d unkown type: \n%s\n", i, buf);
		}

		dev->chans[chan_1].chan_brd = &dev->chan_brd[mb_dev_index+i];
		dev->chans[chan_2].chan_brd = &dev->chan_brd[mb_dev_index+i];

		if (NULL != (p = strstr(buf, sw_head)))
		{
			p += strlen(sw_head);

			j = 0;
			while ( *p && (j < sizeof(ver_buf)) && (*p != '\n') && (*p != '\r') )
			ver_buf[j++] = *p++;

			ver_buf[j] = '\0';

			major = strtol(ver_buf, &endptr, 10) & 0xFF;
			p = ++endptr; /* skip '.' */
			minor = strtol(p, &endptr, 10) & 0xFF;

			bugfix = 0; /* bugfix not define yet ,so allways = 0 */
			dev->chan_brd[mb_dev_index+i].sw_ver =  VERSION_NUMBER(major, minor, bugfix);

			BSP_PRT(DBG, "Chan board %d sw_ver = 0x%x\n", mb_dev_index+i, dev->chan_brd[mb_dev_index+i].sw_ver);
		}

		if(dev->chan_brd[mb_dev_index+i].sw_ver >= VERSION_NUMBER(3, 1, 0)){
			dev->chan_brd[mb_dev_index+i].protocol = CHAN_BRD_PROTOCOL_COBS;
		}else{
			dev->chan_brd[mb_dev_index+i].protocol = CHAN_BRD_PROTOCOL_HDLC;
		}

//	  if(CHAN_MOD_TYPE_UNKOWN != dev->chans[i*2].mod_type && flag == 1) 
//			dev->total_chans = (i + 1) * 2;

		BSP_PRT(DBG, "Channel module %d (chan %d, %d) is %s\n", 
				mb_dev_index+i, chan_1, chan_2, chan_mod_type_to_str(dev->chans[chan_1].mod_type));
		
		close(fds[i]);
	}
	return 0;
}
void *swg_mod_brd_hander(void *data){
	struct swg_mod_brd *mb = (struct swg_mod_brd *)data;
	struct swg_device *dev = mb->swg_dev;
	
	if(init_one_mod_brd(mb, dev, mb->index ) < 0){
		return (void *)NULL;
	}
	if(swg_mod_brd_init_chan_type(mb) < 0){//初始化通道类型
		return (void *)NULL;
	}
	if(swg_mod_brd_init_channel(mb)<0){//注册通道回调函数
		return (void *)NULL;
	}
	swg_handler_thead(mb);
}

static int create_brd_thread(struct swg_device *dev){
	
	struct swg_mod_brd *mb;
	int res, i;
	//1.根据最大模块板数创建线程
	for( i = 0; i < MAX_MOD_BRD_NUM; i++){
		//各个模块初始化自身，如果初始化失败，直接退出
		pthread_attr_t attr;
	    const int stack_size = 256 * 1024; /* 256 K */

	    /* Initialize thread creation attributes */
	    res = pthread_attr_init(&attr);
	    if (0 != res)
	    {
	        BSP_PRT(ERR, "Init detect thread attr failed, %s\n", strerror(errno));
	        return -1;
	    }

	    res = pthread_attr_setstacksize(&attr, stack_size);
	    if (0 != res)
	    {
	        BSP_PRT(ERR, "Init detect set thread size failed, %s\n", strerror(errno));
	        return -2;
	    }
		mb = &dev->mod_brd[i];
		mb->index = i;
		mb->swg_dev = dev;
	    res = pthread_create(&mb->handler_id, &attr, &swg_mod_brd_hander, mb);
	    if (0 != res)
	    {
	        BSP_PRT(ERR, "Init detect create thread failed, %s\n", strerror(errno));
	        return -3;
	    }
	}
	return 0;
}

int swg_device_init(void)
{
    int res;
    /* 创建 swg_device, 全局变量 */
    memset(&SwgDev, 0, sizeof(SwgDev));
    g_swg_dev = &SwgDev;
#if 0
    /* 初始化模块板 */
    if((res = swg_init_mod_brd(g_swg_dev)) <= 0)
    {
        BSP_PRT(WARN, "SWG device has no available module board, res = %d\n", res);
        res = 1;
        goto mod_brd_err;
    }

    /* 初始化通道 */
    if((res = swg_init_channel(g_swg_dev)) <= 0)
    {
        BSP_PRT(WARN, "SWG device has no available channels, res = %d\n", res);
        res = 2;
        goto chan_err;
    }
#endif
	if(create_brd_thread(g_swg_dev) < 0)
	{
		BSP_PRT(WARN, "Create mod brd failed, res = %d\n", res);
		res = -1;
		goto chan_err;
	}
	sleep(6);//等待模块版初始化完成
	if(swg_init_dev_mod_chan_brd_info(g_swg_dev) < 0)//统计通道总数，底板总数，mcu总数
    {
        BSP_PRT(WARN, "SWG device has no available channels, res = %d\n", res);
        res = -2;
        goto chan_err;
    }
    /* 初始化swg device 设备信息 */
    if((res = swg_init_dev_info(g_swg_dev)) < 0)
    {
        BSP_PRT(WARN, "SWG init dev info failed, res = %d\n", res);
        res = -2;
        goto dev_info_err;
    }

    /* 启动检测线程 */
    if((res = swg_init_detect(g_swg_dev)) < 0)
    {
        BSP_PRT(WARN, "SWG init detection failed, res = %d\n", res);
        res = -3;
        goto detect_err;
    }
    //there always return ok.
    if((res = sim_dev_init(g_swg_dev)) < 0)
   {
	BSP_PRT(WARN, "SWG init sim device failed.", res);	
    }
    /* 生成swg device 所有硬件信息 */
    if((res =swg_gen_hw_info(g_swg_dev)) < 0)
    {
        BSP_PRT(WARN, "SWG init detection failed, res = %d\n", res);
        res = -4;
        goto gen_hw_info_err;
    }

	/*如果是vs-usb设备，就开启led线程*/
	if(g_swg_dev->sys_type == SYS_TYPE_VS_USB ){
		vs_usb_led_init(g_swg_dev);
		if((res == vs_usb_led_thread(g_swg_dev))< 0){
			BSP_PRT(WARN, "VS-USB init led thread failed, res = %d\n", res);
			res = -5;
			goto detect_err;
		}
	}
#if 0	
	/*启动事件处理线程，每个模块板一个线程*/
	if(swg_event_thread(g_swg_dev) < 0){
		BSP_PRT(WARN, "SWG init detection failed, res = %d\n", res);
		res = -7;
		goto detect_err;
	}
#endif

#if SWG_TEST
    swg_device_test();
#endif
	sleep(1);
	swg_chan_mod_vbat_supply(SWG_ALL_CHANS,1);/*给所有模块上电*/
	sleep(3);/*等待模块开关机状态变化*/
//	swg_chan_mod_powerkey_hign_low(SWG_ALL_CHANS, CHAN_MOD_POWERKEY_LOW, (char *)"init");//统一拉高powerkey
	swg_chan_mod_powerkey_init(g_swg_dev);
//	swg_get_chan_powerkey_status(g_swg_dev);
	sleep(1);//等待电平状态发生改变
	swg_sim_card_enable(SWG_ALL_CHANS, 1);/*使能所有sim卡*/
	swg_chan_mod_all_power_on_off(CHAN_MOD_POWER_ON);/*开机所有模块*/

    return 0;

gen_hw_info_err:
    swg_deinit_detect(g_swg_dev);
detect_err:
dev_info_err:
chan_err:
    swg_deinit_mod_brd(g_swg_dev);
mod_brd_err:

    /* 失败仍生成配置文件 */
    swg_gen_hw_info(g_swg_dev);

    return res;
}
/*************************************************
*描述：关闭模块板事件处理线程
*输入参数：swg_device 结构体指针
*返回值： 0 --关闭模块板事件处理线程
*返回值：-1 --一个线程也没有关闭成功
*************************************************/
void swg_deinit_event_thread(struct swg_device *dev){
	int res, i = 0;
    void *ret;
	struct swg_mod_brd *mb;
	for(i = 0; i < dev->total_mod_brds; i++){
		mb = &dev->mod_brd[i];
		if(mb->status != MB_STATUS_INITED){
			continue;
		}
	    /* 注销检测线程 */
	    if(0 == mb->handler_id)
	    {
	        BSP_PRT(DBG, "Mod brd %d event task not start yet\n", i+1);
	        continue;
	    }

	    res = pthread_cancel(mb->handler_id);
	    if (0 != res)
	    {
	        BSP_PRT(ERR, "Mod brd %d event task failed: %s\n",i+1, strerror(errno));
	        continue;
	    }

	    /* Join with thread to see what its exit status was */
	    res = pthread_join(dev->dt_id, &ret);
	    if (0 != res)
	    {
	        BSP_PRT(ERR, "Join cancel mod brd %d event task failed: %s\n", i+1,strerror(errno));
	        continue;
	    }

	    if (ret == PTHREAD_CANCELED)
	    {
	        BSP_PRT(DBG, "Mod brd %d event task is canceled.\n", i+1);
	    }
	    else
	    {
	        BSP_PRT(ERR, "Mod brd %d event Cancel detect task failed\n", i+1);
	    }
	}
}

void swg_device_deinit(void)
{
    swg_deinit_detect(g_swg_dev);
	swg_deinit_event_thread(g_swg_dev);
	vs_usb_deinit_led_thread(g_swg_dev);
    swg_deinit_mod_brd(g_swg_dev);
    if(g_swg_dev->sys_type == SYS_TYPE_1CHAN4SIMS)
		sim_dev_deinit(g_swg_dev);
    g_swg_dev = NULL;

    return;
}


/*************************************************
  函数描述 : 读模块板1个或多个连续的寄存器
  输入参数 : idx --  全局模块板编号, 从1开始编号
             reg -- 起始寄存器号
             num -- 寄存器数量, 大于等于1.
  输出参数 : vals -- 指针，用于存放寄存器值，空间不能少于'num'。
  函数返回 :  0 -- 成功
             <0 -- 失败
*************************************************/
int swg_read_mod_brd_reg(int idx, int reg, int num, int *vals)
{
    int res;
    struct swg_mod_brd *mb;

    if(!is_valid_g_md_idx(g_swg_dev, idx))
    {
        BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n",
                __FUNCTION__, idx, g_swg_dev->total_mod_brds);
        return -1;
    }

    idx--;
    if(g_swg_dev->mod_brd[idx].status != MB_STATUS_INITED)
    {
        BSP_PRT(ERR, "Read mod brd %d(staus = %d) reg %d to reg %d failed\n",
                idx + 1, g_swg_dev->mod_brd[idx].status, reg, reg + num - 1);
        return -2;
    }

    mb = &g_swg_dev->mod_brd[idx];
    pthread_mutex_lock(&mb->lock);
    res = hwif_reg_read_mul(&mb->hwif, reg, num, vals);
    pthread_mutex_unlock(&mb->lock);

    if(0 != res)
    {
        BSP_PRT(ERR, "Read mod brd %d reg %d to reg %d failed, res = %d\n",
                idx + 1, reg, reg + num, res);
        return -3;
    }

    return 0;
}

/*************************************************
  函数描述 : 写模块板寄存器
  输入参数 : idx --  全局模块板编号, 从1开始编号
             reg -- 寄存器号
             val -- 寄存器值
  函数返回 :  0 -- 成功
             <0 -- 失败
*************************************************/
int swg_write_mod_brd_reg(int idx, int reg, int val)
{
    int res;
    struct swg_mod_brd *mb;


    if(!is_valid_g_md_idx(g_swg_dev, idx))
    {
        BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n",
                __FUNCTION__, idx, g_swg_dev->total_mod_brds);
        return -1;
    }

    idx--;
    if(g_swg_dev->mod_brd[idx].status != MB_STATUS_INITED)
    {
        BSP_PRT(ERR, "Write mod brd %d(staus = %d) reg %d failed\n",
                idx + 1, g_swg_dev->mod_brd[idx].status, reg);
        return -2;
    }

    mb = &g_swg_dev->mod_brd[idx];
    pthread_mutex_lock(&mb->lock);
    res = hwif_reg_write(&mb->hwif, reg, val);
    pthread_mutex_unlock(&mb->lock);

    if(0 != res)
    {
        BSP_PRT(ERR, "Write mod brd %d reg %d value %d failed, res = %d\n",
                idx + 1, reg, val, res);
        return -3;
    }

    return 0;
}


/*************************************************
  函数描述 : 获取模块板UID
  输入参数 : idx --  全局模块板编号, 从1开始编号
             uid_buf -- 存放uid缓冲区
             uid_buf_len -- uid_buf 空间
  函数返回 : >=0 -- 实际获取UID长度
              <0 -- 失败
*************************************************/
int swg_get_mod_brd_uid(int idx, char *uid_buf, int uid_buf_len)
{
    int res, len = 0;
    struct swg_mod_brd *mb;

    if(!is_valid_g_md_idx(g_swg_dev, idx))
    {
        BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n", 
                __FUNCTION__, idx, g_swg_dev->total_mod_brds);
        return -1;
    }

    idx--;
    if(g_swg_dev->mod_brd[idx].status != MB_STATUS_INITED)
    {
        BSP_PRT(ERR, "Get mod brd %d(status = %d) UID failed\n",
                idx + 1, g_swg_dev->mod_brd[idx].status);
        return -2;
    }

    mb = &g_swg_dev->mod_brd[idx];
    pthread_mutex_lock(&mb->lock);
    res =hwif_get_uid_info(&mb->hwif, uid_buf, uid_buf_len, &len);
    pthread_mutex_unlock(&mb->lock);

    if(0 != res)
    {
        BSP_PRT(ERR, "Get mod brd %d UID failed, res = %d\n", idx + 1, res);
        return -3;
    }

    return len;
}

/*************************************************
  函数描述 : 获取模块板复位按键状态
  输入参数 : idx --  全局模块板编号, 从1开始编号
  输出参数 : status --  返回的状态
                        =1 按键按下
                        =0 按键按没有下
  函数返回 :  0 -- 获取成功
             <0 -- 获取失败
*************************************************/
int swg_get_mod_brd_reset_key_status(int idx, int *status)
{
    int res ;
    struct swg_mod_brd *mb;

    if(!is_valid_g_md_idx(g_swg_dev, idx))
    {
        BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n", 
                __FUNCTION__, idx, g_swg_dev->total_mod_brds);
        return -1;
    }

    idx--;
    if(g_swg_dev->mod_brd[idx].status != MB_STATUS_INITED)
    {
        BSP_PRT(ERR, "Get mod brd %d(status = %d) key status failed\n",
                idx + 1, g_swg_dev->mod_brd[idx].status);
        return -2;
    }

    mb = &g_swg_dev->mod_brd[idx];
    pthread_mutex_lock(&mb->lock);
    res = hwif_get_key_status(&mb->hwif, status);
    pthread_mutex_unlock(&mb->lock);

    if(0 != res)
    {
        BSP_PRT(ERR, "Get mod brd %d key status, res = %d\n", idx + 1, res);
        return -3;
    }

    return 0;
}

/*************************************************
  函数描述 : 获取模块板槽位号
  输入参数 : idx --  全局模块板编号, 从1开始编号
  输出参数 : slot_nu -- 槽位号
  函数返回 :  0 -- 获取成功
             <0 -- 获取失败
*************************************************/
int swg_get_mod_brd_solt_num(int idx, int *solt_num)
{
    int res ;
    struct swg_mod_brd *mb;

    if(!is_valid_g_md_idx(g_swg_dev, idx))
    {
        BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n", 
                __FUNCTION__, idx, g_swg_dev->total_mod_brds);
        return -1;
    }

    idx--;
    if(g_swg_dev->mod_brd[idx].status != MB_STATUS_INITED)
    {
        BSP_PRT(ERR, "Get mod brd %d(status = %d) key status failed\n",
                idx + 1, g_swg_dev->mod_brd[idx].status);
        return -2;
    }

    mb = &g_swg_dev->mod_brd[idx];
    pthread_mutex_lock(&mb->lock);
    res = hwif_get_solt_num(&mb->hwif, solt_num);
    pthread_mutex_unlock(&mb->lock);

    if(0 != res)
    {
        BSP_PRT(ERR, "Get mod brd %d solt num, res = %d\n", idx + 1, res);
        return -3;
    }

    return 0;
}
/*************************************************
  函数描述 : 转换模块SIM卡使能/失能状态为寄存器值
  输入参数 : mb -- 模块板结构体指针
  输出参数     : reg_value_l -- 模块板上1-8通道对应sim使能/失能寄存器值
             reg_value_h -- 模块板上9-16通道对应sim使能/失能寄存器值
*************************************************/
void translate_simcard_enable(struct swg_mod_brd* mb, int *reg_value_l, int *reg_value_h){
	int md_chan;
	int *value;
	unsigned char *simcard_enable_state;
	int bit;
	for(md_chan = 0; md_chan < mb->channels; md_chan++){
		if(mb->swg_dev->chans[mb->base_chan_no+md_chan].mod_type == CHAN_MOD_TYPE_UNKOWN)
			continue;

		if(md_chan/MB_REG_WIDE){//判断存储在高位还是低位
			value = reg_value_h;
			simcard_enable_state = &mb->simcard_enable_status_h;
		}else{
			value = reg_value_l;
			simcard_enable_state = &mb->simcard_enable_status_l;
		}
		bit = md_chan % MB_REG_WIDE;//存储在第几个bit位

		//如果通道有SIM enable/disable事件，转换成对应的状态，并清除事件
		if(mb->simcard_evet[md_chan] == CHAN_MOD_SIM_UNKOWN){
			//nothing to do
		}else if(mb->simcard_evet[md_chan] == CHAN_MOD_SIM_DISABLE){
			*simcard_enable_state &= ~(1<<bit);
			mb->simcard_evet[md_chan] = CHAN_MOD_SIM_UNKOWN;
		}else if(mb->simcard_evet[md_chan] == CHAN_MOD_SIM_ENABLE){
			*simcard_enable_state |= 1 << bit;
			mb->simcard_evet[md_chan] = CHAN_MOD_SIM_UNKOWN;
		}

		if(((*simcard_enable_state>>bit)&0x1) == CHAN_MOD_SIM_ENABLE){
			*value |= 1 << bit;
		}else{
			*value &= ~(1 << bit);
		}
	}
	return;
}

/*************************************************
  函数描述 : 写入模块SIM卡使能/失能状态到寄存器
  输入参数 : data -- 模块板结构体指针
*************************************************/
void swg_sim_card_enabe_sched(void *data){
	int i ;
	int event_flag = 0;
	int reg_value_l, reg_value_h;
	struct swg_mod_brd* mb =(struct swg_mod_brd*)data;
	
	if(mb->name == MB_SWG_1016_BASE || mb->name == MB_SWG_1032_BASE){

		
		for(i = 0; i < mb->channels; i++){
			if(mb->simcard_evet[i] != CHAN_MOD_SIM_UNKOWN){
				event_flag = 1;
				break;
			}
		}

		if(event_flag == 1){
			translate_simcard_enable(mb, &reg_value_l, &reg_value_h);
			pthread_mutex_lock(&mb->lock);
			hwif_reg_write(&mb->hwif, MB_REG_SIM_EN_H, reg_value_h);
			hwif_reg_write(&mb->hwif, MB_REG_SIM_EN_L, reg_value_l);
			pthread_mutex_unlock(&mb->lock);
		}
	}
}
/*************************************************
  函数描述 : 转换模块powerkey状态为寄存器值
  输入参数 : mb -- 模块板结构体指针
  输出参数     : reg_value_l -- 模块板上1-8通道对应powerkey寄存器值
             reg_value_h -- 模块板上9-16通道对应powerkey寄存器值
*************************************************/
void translate_powerkey(struct swg_mod_brd *mod, unsigned int *reg_value_l, unsigned int *reg_value_h){
	int md_chan = 0;
	int bit;
	unsigned char *powerkey_state;
	unsigned int * value;
	for(md_chan = 0;  md_chan < mod->channels; md_chan++){
		//判断写入到高位寄存器还是低位寄存器
		if(md_chan/MB_REG_WIDE){
			value = reg_value_h;
			powerkey_state = &mod->powerkey_status_h;
		}else{
			value = reg_value_l;
			powerkey_state = &mod->powerkey_status_l;
		}

		bit = md_chan % MB_REG_WIDE;
		//事件转换为powerkey状态，并清除事件
		if(mod->powerkey_event[md_chan] == CHAN_MOD_POWERKEY_UNKOWN){
			//nothing to do
		}else if(mod->powerkey_event[md_chan] == CHAN_MOD_POWERKEY_HIGH){
			*powerkey_state |= 1 << bit;
			mod->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_UNKOWN;
			BSP_PRT(DBG, "set chan[%d] powerkey to highlevel ok, clear highlevel event.\n", md_chan+mod->base_chan_no + 1);
		}else{
			*powerkey_state &= ~(1 << bit);
			mod->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_UNKOWN;
			BSP_PRT(DBG, "set chan[%d] powerkey to lowlevel ok, clear lowlevel event.\n", md_chan+mod->base_chan_no + 1);
		}
		//powerkey状态转换为powerkey寄存器值
		if(((*powerkey_state >> bit)&0x1) == CHAN_MOD_POWERKEY_HIGH){
			*value |= 1 << bit;
		}else{
			*value &= ~(1 << bit);
		}
	}
}
/*************************************************
  函数描述 : 写入模块powerkey状态到寄存器
  输入参数 : data -- 模块板结构体指针
*************************************************/
void swg_powerkey_sched(void *data){
	int event_flag = 0;
	int i;
	unsigned int reg_value_l = 0, reg_value_h = 0;
	struct swg_mod_brd *mb = (struct swg_mod_brd*)data;
	if(mb->status != MB_STATUS_INITED ){
		return;
	}
	for(i = 0; i < mb->channels; i++){
		if(mb->powerkey_event[i] != CHAN_MOD_POWERKEY_UNKOWN){
			event_flag = 1;
			break;
		}
	}

	if(event_flag == 1){
		translate_powerkey(mb, &reg_value_l, &reg_value_h);
		pthread_mutex_lock(&mb->lock);
		hwif_reg_write(&mb->hwif, MB_REG_PWRKEY_L, reg_value_l);
		if(mb->channels/MB_REG_WIDE)
			hwif_reg_write(&mb->hwif, MB_REG_PWRKEY_H, reg_value_h);
		pthread_mutex_unlock(&mb->lock);
		BSP_PRT(DBG, " mod_brd[%d] powerkey high_reg bit map=%x, low_reg bit map=%x\n",mb->index+1, reg_value_h, reg_value_l);
	}
}

/*************************************************
  函数描述 : 转换sim卡在位/不在位状态到模块板power_lamp中
  输入参数 : mb -- 模块板结构体指针
  输出参数     : reg_value -- 模块板上sim卡使能/失能寄存器值
*************************************************/
void translate_power_status(struct swg_mod_brd *mb, int *reg_value){
	int i = 0;
	for(i = 0; i < mb->channels; i++){
		if(mb->swg_dev->chans[i+mb->base_chan_no].mod_brd != CHAN_MOD_TYPE_UNKOWN){
			mb->swg_dev->chans[i+mb->base_chan_no].sim_ops->translate_power_status(mb, i, reg_value);
		}
	}
}

/*************************************************
  函数描述 : 转换sim卡在位/不在位状态到模块板simcard_insert_lamp中
  输入参数 : mb -- 模块板结构体指针
  输出参数     : reg_value -- 模块板上sim卡使能/失能寄存器值
*************************************************/

int translate_simcard_insert_status(struct swg_mod_brd *mb, int *reg_value){
	int md_chan = 0;
	unsigned int insert_bit_map;
	unsigned int bit_value;
	int bit;
	insert_bit_map = ~(((reg_value[0] & 0xFF) << 8) | (reg_value[1] & 0xFF));  /* 寄存器值0表示插入，1未插入 */
	for(md_chan = 0; md_chan < mb->channels; md_chan++){
		if(mb->swg_dev->chans[md_chan + mb->base_chan_no].mod_brd != CHAN_MOD_TYPE_UNKOWN){

		    /* 获取所有通道状态值 */
		    /* MB_REG_SIM_INSERT_L地址在MB_REG_SIM_INSERT_H之后 */
			bit_value = (insert_bit_map >> md_chan)&0x1;
			bit = md_chan%MB_REG_WIDE;
			if(md_chan/MB_REG_WIDE){
				mb->simcard_insert_status_h &= ~(1 << bit);//清空通道对应的bit位
				mb->simcard_insert_status_h |= (bit_value << bit);
			}else{
				mb->simcard_insert_status_l &= ~(1 << bit);//清空通道对应的bit位
				mb->simcard_insert_status_l |= (bit_value << bit);
			}
		}
	}
	return 0;
}

/*************************************************
  函数描述 : 从模块底板读取sim卡的插入状态和模块开关机状态
  输入参数 : data -- 模块板结构体指针
*************************************************/
void swg_get_simcard_insert_state_and_power_state(struct swg_mod_brd *mb){
	int reg_value[5];
	int res;
	if(!mb)
		return;
	pthread_mutex_lock(&mb->lock);
	res = hwif_reg_read_mul(&mb->hwif, MB_REG_SIM_INSERT_H, 5, reg_value);
	if(res < 0){
		BSP_PRT(ERR, "thread[%d]read reg value failed!\n", pthread_self());
   		pthread_mutex_unlock(&mb->lock);
		return;
	}
	translate_simcard_insert_status(mb, reg_value);
	translate_power_status(mb, &reg_value[MB_REG_MODULE_ON_OFF_H - MB_REG_SIM_INSERT_H]);
	pthread_mutex_unlock(&mb->lock);
}
/*************************************************
  函数描述 : 模块板事件处理函数
  输入参数 : data -- 模块板结构体指针
*************************************************/
void swg_handler_sched(void *data){
	struct swg_mod_brd *mb = (struct swg_mod_brd*)data;
	mod_schedule_event(mb, 200, swg_handler_sched, data);

	swg_get_simcard_insert_state_and_power_state(data);//15ms
	swg_sim_card_enabe_sched(mb);//30ms
	//powerkey low/high
	swg_powerkey_sched(mb);//30ms
	// vbat
	swg_chan_vbat_sched(mb);//30ms

}

/*************************************************
  函数描述 : 模块板事件处理线程入口
  输入参数 : data -- 模块板结构体指针
*************************************************/
void *swg_handler_thead(void *data){

	mod_schedule_event((struct swg_mod_brd *)data, 0, swg_handler_sched, data);
	while(1){
		mod_schedule_run(data);
		usleep(50*1000);
	}
}

/*************************************************
  函数描述 : 设置使能/失能模块SIM卡事件
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道
             enable -- >0 使能，
                       =0 失能
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
*************************************************/
int swg_sim_card_enable(int chan, int enable)
{
    int res, i, start_chan, end_chan;
    int md_idx, md_chan;
    struct swg_mod_brd *mb = NULL;
    int sucess_cnt = 0;
	enum chan_mod_simcard_event_e event;
    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }

    if(SWG_ALL_CHANS == chan)
    {
        start_chan = 0;
        end_chan = g_swg_dev->total_chans;
    }
    else
    {
        start_chan = chan - 1; //从0开始编号
        end_chan =  chan;
    }
	if(enable){
		event = CHAN_MOD_SIM_ENABLE;
	}else{
		event = CHAN_MOD_SIM_DISABLE;
	}
	for(i = start_chan; i < end_chan; i++){
		if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
        {
            BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow\n",
                     i, g_swg_dev->chans[i].mod_type);
            continue;
        }
		if((res = gchan_2_md_idx_chan(g_swg_dev, i + 1, &md_idx, &md_chan)) != 0)
        {
            BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
                    __FUNCTION__, i + 1, res);
            continue;
        }
		mb = &g_swg_dev->mod_brd[md_idx];
		if(mb->simcard_evet[md_chan] != CHAN_MOD_SIM_UNKOWN){
			BSP_PRT(ERR, "Chan %d is setting simcard %s\n", i+1, mb->simcard_evet[md_chan] == CHAN_MOD_SIM_ENABLE?"enable":"disable");
			continue;
		}
		mb->simcard_evet[md_chan] = event;
		sucess_cnt++;
	}
	if(sucess_cnt > 0)
		return 0;
	else
		return -1;
}


/*************************************************
  函数描述 : 转换模块sim卡使能/使能寄存器值为状态，存储到simard_enable_status
  输入参数 : mb -- 模块板结构体指针
  输出参数     : reg_value -- 模块板上sim卡使能/失能寄存器值
*************************************************/
void translate_simcard_enable_status(struct swg_mod_brd *mb, int *reg_value){
	int md_chan = 0;
	unsigned int enable_bit_map;
	unsigned int bit_value;
	int bit;

	/* MB_REG_SIM_INSERT_L地址在MB_REG_SIM_INSERT_H之后 */
	enable_bit_map = ((reg_value[1] & 0xFF) << 8) | (reg_value[0] & 0xFF);	/* 寄存器值0表示插入，1未插入 */
	
	for(md_chan = 0; md_chan < mb->channels; md_chan++){
		if(mb->swg_dev->chans[md_chan+mb->base_chan_no].mod_brd != CHAN_MOD_TYPE_UNKOWN){

			/* 获取所有通道状态值 */
			
			bit_value = (enable_bit_map>>md_chan)&0x1;
		
			bit = md_chan%MB_REG_WIDE;

			//存储到高位
			if(md_chan/MB_REG_WIDE){
				mb->simcard_enable_status_h &= ~(1<<bit);
				mb->simcard_enable_status_h |= (bit_value << bit);
			}else{//存储到低位
				mb->simcard_enable_status_l &= ~(1<<bit);
				mb->simcard_enable_status_l |= (bit_value << bit);
			}
		}
	}
}
/*************************************************
  函数描述 : 从模块底板获取sim卡使能/失能状态寄存器值，
             并转换对应状态到模块板simard_enable_status中
  输入参数 : mb -- 模块板结构体指针
*************************************************/
int swg_get_chan_sim_card_enable_status(struct swg_mod_brd *mb){
	int reg_value[2] = {0};
	int res;
	if(mb->status != MB_STATUS_INITED)
		return 0;
    pthread_mutex_lock(&mb->lock);
	res = hwif_reg_read_mul(&mb->hwif, MB_REG_SIM_INSERT_H, 2, reg_value);
    pthread_mutex_unlock(&mb->lock);
	
    if(0 != res)
    {
        BSP_PRT(ERR, "Get Mod power status failed, res = %d\n", res);
        return -1;
    }
	
	translate_simcard_enable_status(mb, reg_value);
	return 0;
}

/*************************************************
  函数描述 : 获取模块powerkey，存储到powerkey_status中
  输入参数 : mb -- 模块板结构体指针
  输出参数     : reg_value -- 模块板上powerkey状态寄存器值
*************************************************/
void translate_powerkey_status(struct swg_mod_brd *mb, int *reg_value){
	int md_chan = 0;
	unsigned int powerkey_bit_map;
	unsigned int bit_value;
	int bit;

	/* MB_REG_POWERKEY_H地址在MB_REG_POWERKEY_L之后 */
	powerkey_bit_map = ((reg_value[1] & 0xFF) << 8) | (reg_value[0] & 0xFF);	/* 寄存器值0表示低电平，1表示高电平 */

	for(md_chan = 0; md_chan < mb->channels; md_chan++){
		if(mb->swg_dev->chans[md_chan+mb->base_chan_no].mod_brd != CHAN_MOD_TYPE_UNKOWN){

			/* 获取所有通道状态值 */

			bit_value = (powerkey_bit_map>>md_chan)&0x1;

			bit = md_chan%MB_REG_WIDE;

			//存储到高位
			if(md_chan/MB_REG_WIDE){
				mb->powerkey_status_h &= ~(1<<bit);
				mb->powerkey_status_h |= (bit_value << bit);
			}else{//存储到低位
				mb->powerkey_status_l &= ~(1<<bit);
				mb->powerkey_status_l |= (bit_value << bit);
			}
		}
	}
}
/*************************************************
  函数描述 : 从模块底板获取powerkey电平状态寄值，
             并转换对应状态到模块板powerkey_status_l,powerkey_status_h中
  输入参数 : mb -- 模块板结构体指针
*************************************************/
int swg_get_chan_powerkey_status(struct swg_device *dev){
	int reg_value[2] = {0};
	int res;
	int i;
	struct swg_mod_brd *mb;
	for(i = 0; i < dev->total_mod_brds; i++){
		mb =& dev->mod_brd[i];
		if(mb->status != MB_STATUS_INITED)
			continue;
		pthread_mutex_lock(&mb->lock);
		res = hwif_reg_read_mul(&mb->hwif, MB_REG_PWRKEY_L, 2, reg_value);
		pthread_mutex_unlock(&mb->lock);

		if(0 != res)
		{
			BSP_PRT(ERR, "Get Mod powerkey status failed, res = %d\n", res);
			return -1;
		}

		translate_powerkey_status(mb, reg_value);
	}
	return 0;
}


/*************************************************
  函数描述 : 从模块板simard_enable_lamp中获取SIM卡使能/失能状态
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道 
  输出参数 : status -- sim状态
                       =1 使能，
                       =0 失能
             当chan指定某一通道时，status[0]返回此通道状态.
             当chan == 0xFFFF指定所有通道时，status[0] ... status[total_chan - 1]
             依次表示通道1到通道total_chan 的状态。此时status必须包含足够空间。
             所有通道数目可由函数 swg_get_total_chan_num() 获得。
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
*************************************************/
int swg_get_sim_card_enable_status(int chan, int *status)
{

	int res, i, start_chan, end_chan;
	int md_idx, md_chan;
	int bit;
	struct swg_mod_brd *mb = NULL;
	int sucess_cnt = 0;
	if(status == NULL){
		BSP_PRT(ERR, "Invalied result buff!\n");
		return -1;
	}
	if(!is_valid_g_chan_no(g_swg_dev, chan))
	{
		BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n",
			__FUNCTION__, chan, g_swg_dev->total_chans);
		return -1;
	}

	if(SWG_ALL_CHANS == chan){
		start_chan = 0;
		end_chan = g_swg_dev->total_chans;
	}else{
		start_chan = chan -1;
		end_chan = chan;
	}
	for(i = start_chan; i < end_chan; i++){
		if((res = gchan_2_md_idx_chan(g_swg_dev, i+1, &md_idx, &md_chan)) != 0)
		{
			BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n",
				__FUNCTION__, i+1, res);
			 continue;
		}

		if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
		{
			BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
				i+1, g_swg_dev->chans[i].mod_type);
			continue;
		}

		bit = md_chan%MB_REG_WIDE;

		mb = &g_swg_dev->mod_brd[md_idx];

		if(SWG_ALL_CHANS != chan){
			if(md_chan/MB_REG_WIDE)
				status[0] = ((mb->simcard_enable_status_h) >> bit) & 0x1;
			else
				status[0] = ((mb->simcard_enable_status_l) >> bit) & 0x1;
		}else{
			if(md_chan/MB_REG_WIDE)
				status[i] = ((mb->simcard_enable_status_h) >> bit) & 0x1;
			else
				status[i] = ((mb->simcard_enable_status_l) >> bit) & 0x1;
		}
		sucess_cnt++;
	}
	if(sucess_cnt == 0)
		return -2;
	 return 0;
}


/*************************************************
  函数描述 : 从模块底板获取sim卡使能/失能状态寄存器值，
             并转换对应状态到模块板simcard_insert_lamp中
  输入参数 : mb -- 模块板结构体指针
*************************************************/
int swg_get_chan_sim_card_insert_status(struct swg_mod_brd *mb){
	int reg_value[2] = {0};
	int res;
	if(mb->status != MB_STATUS_INITED)
		return 0;
	
    pthread_mutex_lock(&mb->lock);
	res = hwif_reg_read_mul(&mb->hwif, MB_REG_SIM_INSERT_H, 2, reg_value);
    pthread_mutex_unlock(&mb->lock);
	
    if(0 != res)
    {
        BSP_PRT(ERR, "Get Mod simcard status failed, res = %d\n", res);
        return -1;
    }
	translate_simcard_insert_status(mb, reg_value);
	return 0;
}

/*************************************************
  函数描述 : 从模块板simcard_insert_lamp中获取SIM卡在位/不在位状态
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道
  输出参数 : status -- sim插入状态
                       =1 已插入，
                       =0 未插入
             当chan指定某一通道时，status[0]返回此通道状态.
             当chan == 0xFFFF指定所有通道时，status[0] ... status[total_chan - 1]
             依次表示通道1到通道total_chan的状态。此时status必须包含足够空间。
             所有通道数目可由函数 swg_get_total_chan_num() 获得。
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
*************************************************/
int swg_get_sim_card_insert_status(int chan, int *status)
{

	int res, i, start_chan, end_chan;
	int md_idx, md_chan;
	int bit;
	struct swg_mod_brd *mb = NULL;
	int sucess_cnt = 0;
	if(status == NULL){
		BSP_PRT(ERR, "Invalied result buff!\n");
		return -1;
	}
	if(!is_valid_g_chan_no(g_swg_dev, chan))
	{
		BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n",
			__FUNCTION__, chan, g_swg_dev->total_chans);
		return -1;
	}
	
	if(g_swg_dev->sys_type == SYS_TYPE_1CHAN4SIMS){
		
		return get_sim_state(g_swg_dev ,chan, status);
	}

	if(SWG_ALL_CHANS == chan){
		start_chan = 0;
		end_chan = g_swg_dev->total_chans;
	}else{
		start_chan = chan -1;
		end_chan = chan;
	}
	for(i = start_chan; i < end_chan; i++){
		if((res = gchan_2_md_idx_chan(g_swg_dev, i+1, &md_idx, &md_chan)) != 0)
		{
			BSP_PRT(VERB, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n",
				__FUNCTION__, i+1, res);
			 continue;
		}
		if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
		{
			BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
				i+1, g_swg_dev->chans[i].mod_type);
			continue;
		}

		mb = &g_swg_dev->mod_brd[md_idx];
		bit = md_chan%MB_REG_WIDE;
		pthread_mutex_lock(&mb->lock);
		if(chan != SWG_ALL_CHANS){
			if(md_chan/MB_REG_WIDE)
				status[0] = ((mb->simcard_insert_status_h) >> bit) & 0x1;
			else
				status[0] = ((mb->simcard_insert_status_l) >> bit) & 0x1;
		}else{
			if(md_chan/MB_REG_WIDE)
				status[i] = ((mb->simcard_insert_status_h) >> bit) & 0x1;
			else
				status[i] = ((mb->simcard_insert_status_l) >> bit) & 0x1;
		}
		pthread_mutex_unlock(&mb->lock);
		sucess_cnt++;
	}
	if(sucess_cnt == 0)
		return -2;
	 return 0;
}
/*************************************************
  函数描述 : 获取模块SIM卡插入事件
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道
  输出参数 : event -- 事件
                      =1  插入事件
                      =2  拔出事件 
                      其它 未定义
             当chan指定某一通道时，status[0]返回此通道状态.
             当chan == 0xFFFF指定所有通道时，status[0] ... status[total_chan - 1]
             依次表示通道1到通道total_chan的状态。此时status必须包含足够空间。
             所有通道数目可由函数 swg_get_total_chan_num() 获得。
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
*************************************************/
int swg_get_sim_card_insert_event(int chan, int *event)
{
    int i;

    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }

    if(SWG_ALL_CHANS == chan)
    {
        for(i = 0; i < g_swg_dev->total_chans; i++)
        {

            if(SIMCARD_EVENT_INSERT == g_swg_dev->chans[i].sim_evt)
                event[i] = 1;
            else if(SIMCARD_EVENT_REMOVE == g_swg_dev->chans[i].sim_evt)
                event[i] =  2;
            else
                event[i] =  0;
        }
    }
    else
    {
        chan--; //从0开始编号
        if(SIMCARD_EVENT_INSERT == g_swg_dev->chans[chan].sim_evt)
            event[0] = 1;
        else if(SIMCARD_EVENT_REMOVE == g_swg_dev->chans[chan].sim_evt)
            event[0] =  2;
        else
            event[0] =  0;
    }

    return 0;
}
void *swg_chan_mod_power_on_off_handler(void *data){
	int chan;
	int md_idx, md_chan;
	struct timespec tv;

	struct swg_channel *swg_chan = (struct swg_channel *)data;
	struct swg_mod_brd *swg_mod = swg_chan->mod_brd;
	struct swg_device *dev = swg_chan->mod_brd->swg_dev;
	chan = swg_chan->chan_no+1;

	pthread_detach(pthread_self());//分离线程，系统回收资源

	if(gchan_2_md_idx_chan(dev,chan, &md_idx, &md_chan) != 0)
		return NULL;

	if(swg_chan->chan_power_on_off_try_cnt > 3){
		BSP_PRT(ERR, "Power %s chan[%d] failed!\n", swg_mod->power_event[md_chan] == CHAN_MOD_POWER_ON? "on":"off", chan);
		swg_mod->power_event[md_chan] = CHAN_MOD_POWER_UNKOWN;
		swg_mod->power_next_event[md_chan] = CHAN_MOD_POWER_UNKOWN;
		swg_chan->chan_power_on_off_try_cnt = 0;
		swg_chan->timeout.tv_sec = 0;
		return NULL;
	}
	swg_chan->timeout.tv_sec = 0;//先置为0，防止检测线程检查
	swg_chan->chan_power_on_off_try_cnt++;
	BSP_PRT(INFO, "Power %s chan[%d]!\n", swg_mod->power_event[md_chan] == CHAN_MOD_POWER_ON? "on":"off", chan);
	if(swg_chan->chan_power_on_off_delay < 1800)
		swg_chan->chan_power_on_off_delay += 200;

	swg_mod->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_LOW;
	usleep(swg_chan->chan_power_on_off_delay*1000);
	swg_mod->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_HIGH;

	//更新下次检查模块开关机时间
	clock_gettime(CLOCK_MONOTONIC,&tv);
	swg_chan->timeout.tv_sec = tv.tv_sec + swg_chan->chan_power_on_off_duration;

	return NULL;
}


/*************************************************
  函数描述 : 设置模块开机/关机时间，拉低powerkey时间
*************************************************/
static void swg_chan_set_power_on_off_module_time(struct swg_channel *chan, int event){
	if(!chan)
		return;
	//根据不同模块设置拉低powerkey的时间，开机时间，关机时间	
	if(event == CHAN_MOD_POWER_OFF){
		//ec20 at least 600ms
		//m35 at least 600ms
		//sim6320 at least 1s
		//uc15 at least 600ms
		if(chan->mod_type == CHAN_MOD_EC20F
			||chan->mod_type == CHAN_MOD_EC25E)
		{
			chan->chan_power_on_off_delay = 650;
			chan->chan_power_on_off_duration = 35;//4G模块关机需要35s
		}
		else if(chan->mod_type == CHAN_MOD_UC15
				||chan->mod_type == CHAN_MOD_M35
				||chan->mod_type == CHAN_MOD_M26)
		{
			chan->chan_power_on_off_delay = 800;
			chan->chan_power_on_off_duration=4;//4s
		}
		else if(chan->mod_type == CHAN_MOD_SIM6320C)
		{
			chan->chan_power_on_off_delay = 1200;
			chan->chan_power_on_off_duration=4;//4s
		}
	}else{//CHAN_MOD_POWER_ON
		if(chan->mod_type == CHAN_MOD_EC20F
			||chan->mod_type == CHAN_MOD_EC25E)
		{
			chan->chan_power_on_off_delay = 150;//拉低powerkey时间为150ms
			chan->chan_power_on_off_duration=10;//4G模块开机需要10s
		}
		else if(chan->mod_type == CHAN_MOD_UC15){
			chan->chan_power_on_off_delay = 150;//拉低powerkey时间为150ms
			chan->chan_power_on_off_duration = 4;//开机时间设置为4s
		}
		else if(chan->mod_type == CHAN_MOD_SIM6320C)
		{
			chan->chan_power_on_off_delay = 30;
			chan->chan_power_on_off_duration = 4;//开机时间设置为4s
		}
		else if(chan->mod_type == CHAN_MOD_M35||chan->mod_type == CHAN_MOD_M26)
		{
			chan->chan_power_on_off_delay = 2000;
			chan->chan_power_on_off_duration = 4;//开机时间设置为4s
		}
	}
}

int attr_pthread_create(pthread_t *thread_id, void *(*handler)(void *), void *data){
	int stack_size=128*1024;
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	pthread_attr_setstacksize(&attr, stack_size);
	
	return pthread_create(thread_id, &attr, handler, data);
}

void swg_chan_mod_power_on_off_detect(void *data){
	int i = 0;
	int md_chan, md_idx;
	pthread_t thread_id;
	struct swg_device *dev = (struct swg_device *)data;
	struct timespec tv;
	enum chan_mod_power_event_e power_state;
	//模块开关机超时处理
	clock_gettime(CLOCK_MONOTONIC,&tv);
	for(i = 0; i < dev->total_chans;i++){
		if(gchan_2_md_idx_chan(dev,i+1, &md_idx, &md_chan) != 0)
			continue;
		//如果没有事件，则切换到下一通道
		if(dev->mod_brd[md_idx].power_event[md_chan] == CHAN_MOD_POWER_UNKOWN)
			continue;
		//如果有事件，并且已经到达超时时间,则检查模块开关机状态是否已经发生变化
		if(dev->chans[i].timeout.tv_sec != 0 && dev->chans[i].timeout.tv_sec <= tv.tv_sec){

			//获取模块开关机状态
			pthread_mutex_lock(&dev->mod_brd[md_idx].lock);
			if(md_chan/MB_REG_WIDE)
				power_state = (dev->mod_brd[md_idx].power_status_h >> (md_chan%MB_REG_WIDE)) & 0x1;
			else
				power_state = (dev->mod_brd[md_idx].power_status_l >> (md_chan%MB_REG_WIDE)) & 0x1;
			pthread_mutex_unlock(&dev->mod_brd[md_idx].lock);

			if(dev->mod_brd[md_idx].power_event[md_chan] == power_state){//开机/关机成功
				BSP_PRT(INFO, "Power %s chan[%d] success!\n", dev->mod_brd[md_idx].power_event[md_chan] == CHAN_MOD_POWER_ON? "on":"off", i+1);
				dev->mod_brd[md_idx].power_event[md_chan] = CHAN_MOD_POWER_UNKOWN;//清空事件
				dev->chans[i].timeout.tv_sec = 0;//清除时间
				if(dev->mod_brd[md_idx].power_next_event[md_chan] != CHAN_MOD_POWER_UNKOWN ){//power on module
					swg_chan_set_power_on_off_module_time(&dev->chans[i], CHAN_MOD_POWER_ON);
					dev->mod_brd[md_idx].power_event[md_chan] = CHAN_MOD_POWER_ON;
					dev->mod_brd[md_idx].power_next_event[md_chan] = CHAN_MOD_POWER_UNKOWN;
					dev->chans[i].chan_power_on_off_try_cnt = 0;
					//pthread_create(&thread_id, NULL, swg_chan_mod_power_on_off_handler, &dev->chans[i]);
					attr_pthread_create(&thread_id, swg_chan_mod_power_on_off_handler, &dev->chans[i]);
				}
			}else{//前面开机/关机失败，再次关机
				//pthread_create(&thread_id, NULL, swg_chan_mod_power_on_off_handler, &dev->chans[i]);
				attr_pthread_create(&thread_id, swg_chan_mod_power_on_off_handler, &dev->chans[i]);
			}
		}
	}
}

/*************************************************
  函数描述 : 模块开机/关机
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道
             on -- 开机还是关机
                   >0 开机
                   =0 关机
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
           关机时，拉低1s,开机时，拉低2s
*************************************************/
int swg_chan_mod_all_power_on_off( int on)
{
    int res;
    int i,md_idx, md_chan, sleep_time, success_cnt=0, timeout=0, start_chan, end_chan;
	int bit;
	unsigned char power_state;
	struct timespec tv;
	enum chan_mod_power_event_e event;
    struct swg_mod_brd *mb = NULL;

	if(on){
		event = CHAN_MOD_POWER_ON;
		sleep_time = 2000;
	}else{
		event = CHAN_MOD_POWER_OFF;
		sleep_time = 1000;
	}
	//执行时间
	sleep_time+=200;

	start_chan = 0;
	end_chan = g_swg_dev->total_chans;
		
	for(i = start_chan; i < end_chan; i++){

        if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
        {
            BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
                     i+1 , g_swg_dev->chans[i].mod_type);
            continue;
        }


        if(IS_UPGRADE_STATUS == g_swg_dev->chans[i].upgrade_flag){
            BSP_PRT(ERR, "Chan %d is updating, power %s failed!\n", i+1, on > 0 ? "on" : "off");
	     	continue;
        }

        if((res = gchan_2_md_idx_chan(g_swg_dev, i+1, &md_idx, &md_chan)) != 0)
        {
            BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
                    __FUNCTION__, i + 1, res);
            continue;
        }
        mb = &g_swg_dev->mod_brd[md_idx];
		bit = md_chan % MB_REG_WIDE;
		if(md_chan / MB_REG_WIDE){
			power_state = mb->power_status_h;
		}else{
			power_state = mb->power_status_l;
		}
		//在开机或关机，不能操作
		if(mb->power_event[md_chan] != CHAN_MOD_POWER_UNKOWN){
			BSP_PRT(DBG, "[%s](global chan %d) have already power on or power off, can't power on or power off again!\n", __FUNCTION__, i+1 );
			continue;
		}
		
		//已经是关机状态，不能再关机，已经是开机状态，不能再开机
		if(event == ((power_state >> bit)&0x1)){
			BSP_PRT(DBG, "[%s](global chan %d) is power on or power off status, don't need power on or power off again!\n", __FUNCTION__, i+1 );
			continue;
		}
		
		mb->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_LOW;
		//设置power事件，避免外面再次操作
		mb->power_event[md_chan] = event;
		
		success_cnt++;
    }

	if(success_cnt == 0)
		return -2;
	else
		success_cnt = 0;
	
	usleep(sleep_time*1000);
	clock_gettime(CLOCK_MONOTONIC,&tv);

	for(i = start_chan; i < end_chan; i++){

	    if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
	    {
	        BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
	                 i+1 , g_swg_dev->chans[i].mod_type);
	        continue;
	    }


	    if(IS_UPGRADE_STATUS == g_swg_dev->chans[i].upgrade_flag){
	        BSP_PRT(ERR, "Chan %d is updating, power %s failed!\n", i+1, on > 0 ? "on" : "off");
	     	continue;
	    }

	    if((res = gchan_2_md_idx_chan(g_swg_dev, i+1, &md_idx, &md_chan)) != 0)
	    {
	        BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
	                __FUNCTION__, i + 1, res);
	        continue;
	    }
	    mb = &g_swg_dev->mod_brd[md_idx];
		//没有开关机事件，不操作
		if(mb->power_event[md_chan] == CHAN_MOD_POWER_UNKOWN)
			continue;

		mb->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_HIGH;

		if( g_swg_dev->chans[i].mod_type == CHAN_MOD_EC20F
			|| g_swg_dev->chans[i].mod_type == CHAN_MOD_EC25E ){
			if(event == CHAN_MOD_POWER_ON)
				timeout = 13;
			else
				timeout = 35;
		}else{
			timeout = 4;
		}
		//第一次开机启动，只开机模块一次
		g_swg_dev->chans[i].chan_power_on_off_try_cnt = 4;
		//设置超时时间
		g_swg_dev->chans[i].timeout.tv_sec = tv.tv_sec + timeout;

		success_cnt++;
	}
	if(success_cnt == 0)
		return -3;
	else
		return 0;
}


/*************************************************
  函数描述 : 模块开机/关机
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道
             on -- 开机还是关机
                   >0 开机
                   =0 关机
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
           关机时，拉低1s,开机时，拉低2s
*************************************************/

int swg_chan_mod_power_on_off(int chan, int on)
{
    int res;
    int md_idx, md_chan, success_cnt=0;
	int bit;
	unsigned char power_state;
	struct timespec tv;
	enum chan_mod_power_event_e event;
    struct swg_mod_brd *mb = NULL;
	
    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }

	event = on;

    if(SWG_ALL_CHANS == chan)
    {

        if((res = swg_chan_mod_all_power_on_off(on)) != 0)
        {
            BSP_PRT(ERR, "Power %s all chan failed, res = %d\n", on > 0 ? "on" : "off", res);
            return -2;
        }
        BSP_PRT(DBG, "Power %s all channels\n", on > 0 ? "on" : "off");
        return 0;
	}else{
		chan--;
        if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[chan].mod_type)
        {
            BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
                     chan+1 , g_swg_dev->chans[chan].mod_type);
            return -1;
        }


        if(IS_UPGRADE_STATUS == g_swg_dev->chans[chan].upgrade_flag){
            BSP_PRT(ERR, "Chan %d is updating, power %s failed!\n", chan+1, on > 0 ? "on" : "off");
	     	return -1;
        }

        if((res = gchan_2_md_idx_chan(g_swg_dev, chan+1, &md_idx, &md_chan)) != 0)
        {
            BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
                    __FUNCTION__, chan + 1, res);
            return -1;
        }
        mb = &g_swg_dev->mod_brd[md_idx];

		bit = md_chan % MB_REG_WIDE;
		if(md_chan/MB_REG_WIDE){
			power_state = mb->power_status_h;
		}else{
			power_state = mb->power_status_l;
		}
		if(mb->power_event[md_chan] != CHAN_MOD_POWER_UNKOWN || mb->power_next_event[md_chan] != CHAN_MOD_POWER_UNKOWN)
			return -2;

		if(event == CHAN_MOD_POWER_ON || event == CHAN_MOD_POWER_OFF){
			if(event == ((power_state >> bit) & 0x1))
				return 0;
		}else if(event == CHAN_MOD_POWER_RESET){
			if(((power_state >> bit) & 0x1) == CHAN_MOD_POWER_ON){// power on => power off => power on
				event = CHAN_MOD_POWER_OFF;
				mb->power_next_event[md_chan] = CHAN_MOD_POWER_ON;
			}else{//power off => power on
				event = CHAN_MOD_POWER_ON;
			}
		}else{
			BSP_PRT(ERR,"event=%d, power reset =%d\n", event, CHAN_MOD_POWER_RESET);
			return -3;
		}
		
		swg_chan_set_power_on_off_module_time(&g_swg_dev->chans[chan], event);
		
		g_swg_dev->chans[chan].chan_power_on_off_delay+=400;//确保powerkey事件能正常处理
		mb->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_LOW;
		usleep(g_swg_dev->chans[chan].chan_power_on_off_delay*1000);
		mb->powerkey_event[md_chan] = CHAN_MOD_POWERKEY_HIGH;
		
		mb->power_event[md_chan] = event;
		
		clock_gettime(CLOCK_MONOTONIC, &tv);
		tv.tv_sec+= g_swg_dev->chans[chan].chan_power_on_off_duration;
		g_swg_dev->chans[chan].timeout.tv_sec = tv.tv_sec;//设置超时时间，保存模块正常起来
		g_swg_dev->chans[chan].chan_power_on_off_try_cnt = 1;
		success_cnt++;
    }
	if(success_cnt == 0)
		return -1;
	else
		return 0;
}


/*************************************************
  函数描述 : 获取通道模块开机/关机状态
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道
  输出参数 : status -- 开机/关机状态
                       >0 开机
                       =0 关机
             当chan指定某一通道时，status[0]返回此通道状态.
             当chan == 0xFFFF指定所有通道时，status[0] ... status[total_chan - 1]
             依次表示通道1到通道total_chan的状态。此时status必须包含足够空间。
             所有通道数目可由函数 swg_get_total_chan_num() 获得。
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
*************************************************/
int swg_get_chan_mod_power_status(int chan, int *status)
{

	int res, i, start_chan, end_chan;
	int md_idx, md_chan;
	struct swg_mod_brd *mb = NULL;
	int bit;
	int sucess_cnt = 0;
	enum chan_mod_vbat_event_e vbat_state;
	enum chan_mod_power_event_e power_state;
	if(status == NULL){
		BSP_PRT(ERR, "Invalied result buff!\n");
		return -1;
	}
	if(!is_valid_g_chan_no(g_swg_dev, chan))
	{
		BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n",
			__FUNCTION__, chan, g_swg_dev->total_chans);
		return -1;
	}

	if(SWG_ALL_CHANS == chan){
		start_chan = 0;
		end_chan = g_swg_dev->total_chans;
	}else{
		start_chan = chan -1;
		end_chan = chan;
	}
	
	for(i = start_chan; i < end_chan; i++){
		if((res = gchan_2_md_idx_chan(g_swg_dev, i+1, &md_idx, &md_chan)) != 0)
		{
			BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n",
				__FUNCTION__, i+1, res);
			 continue;
		}
		if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
		{
			BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
				i+1, g_swg_dev->chans[i].mod_type);
			continue;
		}

		mb = &g_swg_dev->mod_brd[md_idx];
		bit = md_chan % MB_REG_WIDE;
		if(md_chan/MB_REG_WIDE){
			vbat_state = (mb->vbat_status_h >> bit)&0x1;
		}else{
			vbat_state = (mb->vbat_status_l >> bit)&0x1;
		}
		if(vbat_state == CHAN_MOD_VBAT_OFF){//如果模块没有供电，查询到的状态可能不对，标记为关机状态
			power_state = CHAN_MOD_POWER_OFF;
		}else{
			if(mb->power_event[md_chan] != CHAN_MOD_POWER_UNKOWN){//模块正在开机或关机
				if(mb->power_event[md_chan] == CHAN_MOD_POWER_OFF){
					power_state = CHAN_MOD_IS_POWER_OFF;
				}else{
					power_state = CHAN_MOD_IS_POWER_ON;
				}
			}else{//模块处于开机状态或关机状态
				pthread_mutex_lock(&mb->lock);
				if(md_chan/MB_REG_WIDE)
					power_state = ((mb->power_status_h) >> bit) & 0x1;
				else
					power_state = ((mb->power_status_l) >> bit) & 0x1;
				pthread_mutex_unlock(&mb->lock);
			}
		}

		/* 操作指定通道 */
		if(SWG_ALL_CHANS != chan)
		{
			status[0] = power_state;
		}else{
			status[i] = power_state;
		}
		sucess_cnt++;
	}
	if(sucess_cnt == 0)
		return -2;
	 return 0;
}


/*************************************************
  函数描述 : 转换模块供电/断电状态为寄存器值
  输入参数 : mb -- 模块板结构体指针
  输出参数     : reg_value_l -- 模块板上1-8通道对应供电/断电寄存器值
             reg_value_h -- 模块板上9-16通道对应供电/断电寄存器值
*************************************************/
int translate_vbat_supply(struct swg_mod_brd* mb, int *reg_value_l, int *reg_value_h){
	int i = 0;
	for(i = 0; i < mb->channels; i++){
		if(mb->swg_dev->chans[mb->base_chan_no+i].mod_type == CHAN_MOD_TYPE_UNKOWN)
			continue;
		mb->swg_dev->chans[mb->base_chan_no+i].sim_ops->translate_vbat(mb, i,reg_value_l, reg_value_h );
	}
	return 0;
}
/*************************************************
  函数描述 : 写入模块供电/断电状态到寄存器
  输入参数 : data -- 模块板结构体指针
*************************************************/
void swg_chan_vbat_sched(void *data){
	int i = 0;
	int event_flag = 0;
	int reg_value_l = 0, reg_value_h=0;
	struct swg_mod_brd* mb =(struct swg_mod_brd*)data;
	for(i = 0; i < mb->channels; i++){
		if(CHAN_MOD_VBAT_UNKOWN != mb->vbat_event[i]){
			event_flag = 1;
			break;
		}
	}
	
	if(event_flag == 1){
		translate_vbat_supply(mb, &reg_value_l, &reg_value_h);
		pthread_mutex_lock(&mb->lock);
		hwif_reg_write(&mb->hwif, MB_REG_MODULE_VBAT_L, reg_value_l);
		if(mb->channels / MB_REG_WIDE)
			hwif_reg_write(&mb->hwif, MB_REG_MODULE_VBAT_H, reg_value_h);
		pthread_mutex_unlock(&mb->lock);
		
	}

}

/*************************************************
  函数描述 : 给通道模块供电/断电
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道
             supply -- 供电还是断电
                       >0 供电
                       =0 断电
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
             给模块提供电源与否，供电后才能power on
*************************************************/
int swg_chan_mod_vbat_supply(int chan, int supply)
{
    int res, i, start_chan, end_chan;
    int md_idx, md_chan;
    struct swg_mod_brd *mb = NULL;
    int sucess_cnt = 0;
	enum chan_mod_vbat_event_e event;
    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }

    if(SWG_ALL_CHANS == chan)
    {
        start_chan = 0;
        end_chan = g_swg_dev->total_chans;
    }
    else
    {
        start_chan = chan - 1; //从0开始编号
        end_chan =  chan;
    }
	
	if(supply)
		event = CHAN_MOD_VBAT_ON;
	else
		event = CHAN_MOD_VBAT_OFF;
	
    for(i = start_chan; i < end_chan; i++)
    {
        if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
        {
            BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
                     i, g_swg_dev->chans[i].mod_type);
            continue;
        }

        if(IS_UPGRADE_STATUS == g_swg_dev->chans[i].upgrade_flag){ 
            BSP_PRT(ERR, "Chan %d is updating status, vbat %ssupply failed!\n", i, supply > 0 ? " " : "not ");
            continue;
        }

        if((res = gchan_2_md_idx_chan(g_swg_dev, i + 1, &md_idx, &md_chan)) != 0)
        {
            BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
                    __FUNCTION__, i + 1, res);
            continue;
        }

        mb = &g_swg_dev->mod_brd[md_idx];

		if(mb->vbat_event[md_chan] != CHAN_MOD_VBAT_UNKOWN){
			BSP_PRT(ERR, "[%s](global chan %d) is vbat on or vbat off, cant't vbat off or vbat on again\n", __FUNCTION__, i+1);
			return -1;
		}
        mb->vbat_event[md_chan] = event;
        sucess_cnt++;
    }
    if(0 == sucess_cnt)
    {
        return -2;
    }

    return 0;
}


/*************************************************
  函数描述 : 转换模块供电/断电状态到模块板power_lamp中
  输入参数 : mb -- 模块板结构体指针
  输出参数     : reg_value -- 模块板上sim卡使能/失能寄存器值
*************************************************/
int translate_vbat_status(struct swg_mod_brd *mb, int *reg){
	int i;
	for(i = 0; i < mb->channels;i++){
		if(mb->swg_dev->chans[i+mb->base_chan_no].mod_brd != CHAN_MOD_TYPE_UNKOWN){
			mb->swg_dev->chans[i+mb->base_chan_no].sim_ops->translate_vbat_status(mb, i, reg);
		}
	}
	return 0;
}

/*************************************************
  函数描述 : 从模块底板获取模块开机/关机状态寄存器值，
             并转换对应状态到模块板power_lamp中
  输入参数 : mb -- 模块板结构体指针
*************************************************/

int swg_get_chan_vbat_status(struct swg_mod_brd *mb){
	int res;
	int reg_value[2] = {0};
	if(mb->status != MB_STATUS_INITED)
		return 0;
    pthread_mutex_lock(&mb->lock);
	res = hwif_reg_read_mul(&mb->hwif, MB_REG_MODULE_VBAT_L, 2, reg_value);
    pthread_mutex_unlock(&mb->lock);
    if(0 != res)
    {
        BSP_PRT(ERR, "Get Mod power status failed, res = %d\n", res);
        return -1;
    }
	translate_vbat_status(mb, reg_value);
	return 0;
}

/*************************************************
  函数描述 : 获取通道模块供电状态
  输入参数 : chan -- 通道号, =0xFFFF 表示所有通道
  输出参数 : status -- 供电状态
                       >0 供电
                       =0 断电
             当chan指定某一通道时，status[0]返回此通道状态.
             当chan == 0xFFFF指定所有通道时，status[0] ... status[total_chan - 1]
             依次表示通道1到通道total_chan的状态。此时status必须包含足够空间。
             所有通道数目可由函数 swg_get_total_chan_num() 获得。
  函数返回 :  0 -- 成功, 一个或多个通道操作成功
             <0 -- 失败, 所有通道操作都失败才返回
  备注     : 操作所有通道时，当其中一个失败，仍然操作其通道。
*************************************************/
int swg_get_chan_mod_vbat_status(int chan, int *status)
{
    int res, i;
    int md_idx, md_chan, start_chan, end_chan,sucess_cnt;
	int bit;
	struct swg_mod_brd *mb = NULL;
	if(status == NULL){
		BSP_PRT(ERR, "Invalied result buff!\n");
		return -1;
	}

    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }
	
	if(SWG_ALL_CHANS == chan){
		start_chan = 0;
		end_chan = g_swg_dev->total_chans;
	}else{
		start_chan = chan -1;
		end_chan = chan;
	}
	for(i = start_chan; i < end_chan; i++){
		if((res = gchan_2_md_idx_chan(g_swg_dev, i+1, &md_idx, &md_chan)) != 0)
		{
			BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n",
				__FUNCTION__, i+1, res);
			 continue;
		}
		if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
		{
			BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
				i+1, g_swg_dev->chans[i].mod_type);
			continue;
		}

		mb = &g_swg_dev->mod_brd[md_idx];
		bit = md_chan % MB_REG_WIDE;
		if(SWG_ALL_CHANS != chan){
			if(md_chan/MB_REG_WIDE)
				status[0] = ((mb->vbat_status_h) >> bit) & 0x1;
			else
				status[0] = ((mb->vbat_status_l) >> bit) & 0x1;
		}else{
			if(md_chan/MB_REG_WIDE)
				status[i] = ((mb->vbat_status_h) >> bit) & 0x1;
			else
				status[i] = ((mb->vbat_status_l) >> bit) & 0x1;
		}
		sucess_cnt++;
	}
	
    return 0;
}



/*************************************************
  函数描述 : 获取所有通道数目
  函数返回 : >=0 -- 通道数目
             <0  -- 获取失败
*************************************************/
int swg_get_total_chan_num(void)
{
    return g_swg_dev->total_chans;
}

/*************************************************
  函数描述 : 获取所有模块板数目(包括插入与未插入)
  函数返回 : >=0 -- 模块板总数
             <0  -- 获取失败
*************************************************/
int swg_get_total_mod_brd_num(void)
{
    return g_swg_dev->total_mod_brds;
}
/*************************************************
  函数描述 : 获取模块板版本信息
  输入参数 : idx -- 模块板编号(从1开始编号)
  输出参数 : info -- 返回版本信息
  函数返回 :  0 -- 成功
             <0 -- 失败
*************************************************/
int swg_get_mod_brd_ver_info(int idx, struct mod_brd_ver_info *info)
{
    if(!is_valid_g_md_idx(g_swg_dev, idx))
    {
        BSP_PRT(ERR, "[%s] Invalied global mod brd index %d(total %d)\n",
                __FUNCTION__, idx, g_swg_dev->total_mod_brds);
        return -1;
    }

    idx--;
    if(MB_STATUS_INITED != g_swg_dev->mod_brd[idx].status)
        return -2;
    info->name = g_swg_dev->mod_brd[idx].name;
    info->hw_ver = g_swg_dev->mod_brd[idx].hw_ver;
    info->sw_ver = g_swg_dev->mod_brd[idx].sw_ver;

    return 0;
}

/*************************************************
  函数描述 : 获取SWG设备名称
  输出参数 : info -- 返回版本信息
  函数返回 :  0 -- 成功
             <0 -- 失败
*************************************************/
int swg_get_dev_ver_info(struct swg_ver_info *info)
{
    if(NULL == g_swg_dev)
        return -1;

    info->name = g_swg_dev->name;
    info->version = g_swg_dev->version;

    return 0;
}


/*************************************************
  函数描述 : 选择升级通道无线模块固件通道
  输入参数 : chan -- 指定升级的通道, =0xFFFF 所有通道都 不 选
  函数返回 : 0 -- 成功
             1 -- 不支持选择升级通道功能
             <0 -- 失败
*************************************************/
int swg_select_upgrade_chan(int chan)
{
    int res, i, mb_idx, mb_chan;
    struct swg_mod_brd *mb;

    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }

    if(SWG_ALL_CHANS == chan) /* unselect all */
    {
        for(i = 0; i < g_swg_dev->total_mod_brds; i++)
        {
            mb = &g_swg_dev->mod_brd[i];
            if(MB_STATUS_INITED  != mb->status)
                continue;

            pthread_mutex_lock(&mb->lock);
            res = mod_brd_select_upgrade_chan(mb, MB_ALL_CHANS);
            pthread_mutex_unlock(&mb->lock);
            if(0 != res && 1 != res)
            {
                BSP_PRT(INFO, "Unselect all upgrade chan failed, res = %d, faild md =%d) \n", 
                        res, i);
                res = -2;
                break;
            }
        }
    } 
    else
    {
        if((res = gchan_2_md_idx_chan(g_swg_dev, chan, &mb_idx, &mb_chan)) != 0)
        {
            BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
                    __FUNCTION__, chan, res);
            return -3;
        }

        mb = &g_swg_dev->mod_brd[mb_idx];

        pthread_mutex_lock(&mb->lock);
        res = mod_brd_select_upgrade_chan(mb, mb_chan);
        pthread_mutex_unlock(&mb->lock);
        if(0 != res && 1 != res)
        {
            BSP_PRT(INFO, "Module board %d select chan %d upgrade failed, res = %d) \n", 
                    mb_idx, mb_chan, res);
            res = -2;
        }
    }

    return res;
}



/*************************************************
  函数描述 : 检测升级通道是否已经被选中。
  输入参数 : chan -- 指定检测的通道
  函数返回 :  0 -- 通道没有被选择为升级通道
              1 -- 通道被选择为升级通道
              2 -- 通道不支持选择升级功能
             <0 出错
*************************************************/
int swg_upgrade_chan_is_selected(int chan)
{
    int res, mb_idx, mb_chan;
    struct swg_mod_brd *mb;

    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }

    if((res = gchan_2_md_idx_chan(g_swg_dev, chan, &mb_idx, &mb_chan)) != 0)
    {
            BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
                    __FUNCTION__, chan, res);
        return -2;
    }

    mb = &g_swg_dev->mod_brd[mb_idx];

    pthread_mutex_lock(&mb->lock);
    res = mod_brd_upgrade_chan_is_selected(mb, mb_chan);
    pthread_mutex_unlock(&mb->lock);
    if(res < 0)
    {
        BSP_PRT(INFO, "Detect module board %d chan %d whether is upgrade selected failed, res = %d \n", 
                mb_idx, mb_chan, res);
        res = -3;
    }
    return res;
}


/*************************************************
  函数描述 : 改变power_key关键电压值状态,init和升级状态有效
  输入参数 : chan --  指定通道号，
             level -- 0 拉低管脚电平
                      1 拉高管脚电平
  函数返回 : 成功操作的通道数
             <0 出错
*************************************************/
int swg_chan_mod_powerkey_hign_low(int chan, int level, char *id){
     int res, i, start_chan, end_chan;
    int md_idx, md_chan;
    struct swg_mod_brd *mb = NULL;
    int sucess_cnt = 0;
	CHAN_MOD_POWERKEY_EVENT_E event;
    if((strncmp(id, "init", 4) != 0)&&(strcmp(id, "upgrade") != 0 || g_swg_dev->chans[chan-1].upgrade_flag != IS_UPGRADE_STATUS)){
	    BSP_PRT(ERR, "[%s]Chan [%d] press powerkey failed, only upgrade mode can set powerkey!", __FUNCTION__, chan );
        return -1;
    }
    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }

	if(level)
		event = CHAN_MOD_POWERKEY_HIGH;
	else
		event = CHAN_MOD_POWERKEY_LOW;
	
    if(SWG_ALL_CHANS == chan)
    {
        start_chan = 0;
        end_chan = g_swg_dev->total_chans;
    }
    else
    {
        start_chan = chan - 1; //从0开始编号
        end_chan =  chan;
    }

    for(i = start_chan; i < end_chan; i++)
    {
        if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
        {
            BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
                     i, g_swg_dev->chans[i].mod_type);
            continue;
        }

        if((res = gchan_2_md_idx_chan(g_swg_dev, i + 1, &md_idx, &md_chan)) != 0)
        {
            BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
                    __FUNCTION__, i + 1, res);
            continue;
        }

        mb = &g_swg_dev->mod_brd[md_idx];

		mb->powerkey_event[md_chan] = event;
        
        sucess_cnt++;
    }
    if(0 == sucess_cnt)
    {
        return -2;
    }
	return res;
}


/*************************************************
 *   函数描述 : 改变升级状态标记位
 *   输入参数 : chan --  指定通道号，
 *              flag -- 0 清除升级标记位
 *                      1 设置升级标记位
 *   函数返回 : 成功操作的通道数
 *              <0 出错
 **************************************************/
int swg_chan_mod_set_upgrade_status(int chan, int flag, char *id){
    int i, start_chan, end_chan;
    int success_cnt = 0; 
    CHAN_UPGRADE_STATUS_E status;
    if(strcmp(id , "upgrade") != 0){
	   BSP_PRT(ERR, "set chn[%d] upgrade status failed, only upgrade module is vaild!\n", chan);
	   return -1;
	}
	
	if(flag == 0)
      status = NOT_UPGRADE_STATUS;
    else
      status = IS_UPGRADE_STATUS;
    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
       BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
              __FUNCTION__, chan, g_swg_dev->total_chans);
       return -1;
    }
                      
    if(SWG_ALL_CHANS == chan)
    {
       start_chan = 0; 
       end_chan = g_swg_dev->total_chans;
    }
    else 
    {
       start_chan = chan - 1; //从0开始编号
       end_chan =  chan;
    }
               
    for(i = start_chan; i < end_chan; i++) 
    {
       if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
       {    
           BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
                  i, g_swg_dev->chans[i].mod_type);
           continue;
       }
       g_swg_dev->chans[i].upgrade_flag = status;
       success_cnt++;
    }

   if(success_cnt == 0)
   	return -1;
    return 0;
}

/*************************************************
  函数描述 : 打开/关闭模块底板debug开关
  输入参数 : idx -- 模块板全局编号
             enabe >0 打开
                   =0 关闭
*************************************************/
int swg_set_debug_uart(int idx, int enable)
{
    int res,i, success_cnt=0;
	int start_brd, end_brd;
    struct swg_mod_brd *mb;

	if((0xFF &idx) == 0xFF){
		start_brd = 0;
		end_brd = g_swg_dev->total_mod_brds;
	}else{
		start_brd = idx - 1;
		end_brd = idx;
	}
        
	for(i = start_brd; i < end_brd; ++i){

		if(!is_valid_g_md_idx(g_swg_dev, i))
		{
			BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n", 
					__FUNCTION__, idx, g_swg_dev->total_mod_brds);
			continue;
		}
            
		if(g_swg_dev->mod_brd[i].status != MB_STATUS_INITED)
		{
			BSP_PRT(ERR, "Get mod brd %d(status = %d) key status failed\n",
					i + 1, g_swg_dev->mod_brd[i].status);
			continue;
		}
            
		mb = &g_swg_dev->mod_brd[i];
		pthread_mutex_lock(&mb->lock);
		res = hwif_set_debug_uart(&mb->hwif, enable);
		pthread_mutex_unlock(&mb->lock);

		if(0 != res)
		{
			BSP_PRT(ERR, "set mod brd %d debug uart failed, res = %d\n", i + 1, res);
			continue;
		}
		success_cnt++;
	}
	if(success_cnt <= 0)
		return -1;
	else
	   return 0;
}


/*************************************************
 *   函数描述 : 初始化led灯状态
 *   输入参数 : dev --swg_device 结构体
 *   函数返回 : 成功操作的通道数
 *              <0 出错
 **************************************************/

int vs_usb_led_init(struct swg_device *dev){
	int i = 0;
	struct swg_mod_brd *mb;
	for(i = 0; i < dev->total_mod_brds; i++){
		if(!is_valid_g_md_idx(g_swg_dev, i))
		{
			BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n", 
					__FUNCTION__, i, g_swg_dev->total_mod_brds);
			continue;
		}
            
		if(g_swg_dev->mod_brd[i].status != MB_STATUS_INITED || g_swg_dev->mod_brd[i].name != MB_SWG_VS_USB_BASE)
		{
			BSP_PRT(ERR, "Mod brd %d is not a vs usb device\n",i + 1);
			continue;
		}
		
		mb = &g_swg_dev->mod_brd[i];
		//关闭所有的sig和call指示灯
		pthread_mutex_lock(&mb->lock);
		hwif_set_led_lamp(&mb->hwif, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, 0);
		hwif_set_led_lamp(&mb->hwif, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, CHAN_LAMP_STATUS_OFF, 1);
		pthread_mutex_unlock(&mb->lock);
		mb->lamp_flash = 0;

		//打开模块底板板system led灯
		mb->sys_lamp_status = MOD_BRD_LAMP_STATUS_GREEN_FLASH;
		mb->sys_lamp_flash = 0x1;
	}
	return 0;
}


/*************************************************
 *   函数描述 : led状态和对应寄存器bit位值转换
 *   输入参数 : mb --模块板指针
 *              mb_chan --模块板上的通道索引
 *   输出参数： cts--cts位的值
 *              rts--rts位的值
 *              dtr--dtr位的值
 *   函数返回 : 成功操作的通道数
 *              <0 出错
 **************************************************/

static int vs_usb_translate_lamp(struct swg_mod_brd *mb, int mb_chan, int *cts, int *rts, int *dtr)
{
	int signal_flash = 0;
	int work_flash = 0;
	// mb/mb_chan的合法性
	*rts = 0;
	*cts = 0;
	*dtr = 0;
	if (mb->reg_lamp_status[mb_chan] == CHAN_LAMP_STATUS_RED_ON ) {
		*cts = 1;
	} else if (mb->reg_lamp_status[mb_chan] == CHAN_LAMP_STATUS_GREEN_ON) {
		*rts = 1;
	}else if(mb->reg_lamp_status[mb_chan] == CHNA_LAMP_STATUS_YELLOW_ON){
		*cts = 1;
		*rts = 1;
	}

	if (mb->call_lamp_status[mb_chan] == CHAN_LAMP_STATUS_GREEN_ON) {
		*dtr = 1;
	}

	// handle signal flash
	if(mb->reg_lamp_status[mb_chan] == CHAN_LAMP_STATUS_RED_FLASH) {
		signal_flash = (mb->lamp_flash >> (mb_chan*2) & 0x1);
		*cts = signal_flash;
	} else if(mb->reg_lamp_status[mb_chan] == CHAN_LAMP_STATUS_GREEN_FLASH) {
		signal_flash = (mb->lamp_flash >> (mb_chan*2) & 0x1);
		*rts = signal_flash;
	}else if(mb->reg_lamp_status[mb_chan] == CHNA_LAMP_STATUS_YELLOW_FLASH){
		signal_flash = (mb->lamp_flash >> (mb_chan*2) & 0x1);
		*cts = signal_flash;
		*rts = signal_flash;
	}

	if(mb->call_lamp_status[mb_chan] == CHAN_LAMP_STATUS_GREEN_FLASH) {
		work_flash = (mb->lamp_flash >> (mb_chan*2+1) & 0x1);
		*dtr = work_flash;
	}
	return 0;
}

/*************************************************
 *   函数描述 : led闪烁线程
 *   输入参数 : data --struct swg_device 结构体
 *   函数返回 : 成功操作的通道数
 *              <0 出错
 **************************************************/

void *vs_usb_loop_set_led_lamp(void *data){
	int i = 0, j = 0;
	int flash_flag = 0;
	int cts1=0, rts1=0, dtr1=0;
	int cts2=0, rts2=0, dtr2=0;
	int high_flag = 0;
	struct swg_mod_brd *mb;

	while(1){
		for(i = 0; i < g_swg_dev->total_mod_brds; i++){
			if(!is_valid_g_md_idx(g_swg_dev, i))
			{
				BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n", 
						__FUNCTION__, i, g_swg_dev->total_mod_brds);
				continue;
			}
	            
			if(g_swg_dev->mod_brd[i].status != MB_STATUS_INITED || g_swg_dev->mod_brd[i].name != MB_SWG_VS_USB_BASE)
			{
				BSP_PRT(WARN, "Mod brd %d(status = %d) is not a vs-usb mod brd\n",
						i + 1, g_swg_dev->mod_brd[i].status);
				continue;
			}

			mb = &g_swg_dev->mod_brd[i];

			for(j = 0 ; j < MAX_CHAN_NUM_PER_VS_USB_MCU; j = j + 2 ){
				if(mb->reg_lamp_status[j] == CHAN_LAMP_STATUS_RED_FLASH || 
					mb->reg_lamp_status[j] == CHAN_LAMP_STATUS_GREEN_FLASH||
					mb->reg_lamp_status[j] == CHNA_LAMP_STATUS_YELLOW_FLASH) {
					flash_flag = 1;
					mb->lamp_flash ^= ((1<<(j*2)) & 0xFF);
				}
					
				if(mb->call_lamp_status[j] == CHAN_LAMP_STATUS_GREEN_FLASH) {
					flash_flag = 1;
					mb->lamp_flash ^= ((1<<(j*2+1)) & 0xFF);
				}
				
				if(mb->reg_lamp_status[j+1] == CHAN_LAMP_STATUS_RED_FLASH || 
					mb->reg_lamp_status[j+1] == CHAN_LAMP_STATUS_GREEN_FLASH ||
					mb->reg_lamp_status[j+1] == CHNA_LAMP_STATUS_YELLOW_FLASH) {
					flash_flag = 1;
					mb->lamp_flash ^= ((1<<((j+1)*2)) & 0xFF);
				}
					
				if(mb->call_lamp_status[j+1] == CHAN_LAMP_STATUS_GREEN_FLASH) {
					flash_flag = 1;
					mb->lamp_flash ^= ((1<<((j+1)*2+1)) & 0xFF);
				}

				if(flash_flag) {

					if(j < MAX_CHAN_NUM_PER_VS_USB_MCU/2){
						high_flag = 0;
					}else{
						high_flag = 1;
					}
#ifdef VS_USB_M35_LED_DEBUG
					if(mb->swg_dev->chans[mb->base_chan_no].mod_type == CHAN_MOD_M35){
						if(j < MAX_CHAN_NUM_PER_VS_USB_MCU/2){
							high_flag = 1;
						}else{
							high_flag = 0;
						}
					}
#endif
					vs_usb_translate_lamp(mb, j, &cts1, &rts1, &dtr1);
					vs_usb_translate_lamp(mb, j+1, &cts2, &rts2, &dtr2);
					pthread_mutex_lock(&mb->lock);
					hwif_set_led_lamp(&mb->hwif, cts1, rts1, dtr1, cts2, rts2, dtr2, high_flag);
					pthread_mutex_unlock(&mb->lock);
					BSP_PRT(DBG, "mod_brd[%d] channel[%d, %d] cts1 = %d, rts1 = %d, dtr1 = %d, cts2 = %d, rts2=%d, dtr2 = %d\n",i+1, j+1, j+2 , cts1, rts1, dtr1, cts2, rts2, dtr2);
				}
				flash_flag = 0;
			}
			if(mb->sys_lamp_status == MOD_BRD_LAMP_STATUS_GREEN_FLASH){
				mb->sys_lamp_flash = mb->sys_lamp_flash ^ 1;
				pthread_mutex_lock(&mb->lock);
				mod_brd_set_sys_led_lamp(&mb->hwif, mb->sys_lamp_flash & 0x1);
				pthread_mutex_unlock(&mb->lock);
			}
		}
		usleep(500000);
	}
}


/*************************************************
 *   函数描述 : 开启led线程
 *   输入参数 : dev --struct swg_device结构体  
 *   函数返回 : 成功操作的通道数
 *              <0 出错
 **************************************************/

int vs_usb_led_thread(    struct swg_device *dev){
	
    int res;
    pthread_attr_t attr;
    const int stack_size = 256 * 1024; /* 256 K */

    /* Initialize thread creation attributes */
    res = pthread_attr_init(&attr);
    if (0 != res)
    {
        BSP_PRT(ERR, "Init led thread attr failed, %s\n", strerror(errno));
        return -1;
    }

    res = pthread_attr_setstacksize(&attr, stack_size);
    if (0 != res)
    {
        BSP_PRT(ERR, "Init led set thread size failed, %s\n", strerror(errno));
        return -2;
    }
    res = pthread_create(&dev->led_id, &attr, &vs_usb_loop_set_led_lamp, dev);
    if (0 != res)
    {
        BSP_PRT(ERR, "Init led create thread failed, %s\n", strerror(errno));
        return -3;
    }
	return 0;
}
/*************************************************
  函数描述 : 销毁led线程
  输入参数 : dev -- struct swg_device结构 
  函数返回 :  =0 成功
              <0 失败
*************************************************/
static int vs_usb_deinit_led_thread(struct swg_device *dev)
{
    int res;
    void *ret;

    /* 注销检测线程 */
    if(0 == dev->led_id)
    {
        BSP_PRT(DBG, "Led task not start yet\n");
        return 0;
    }

    res = pthread_cancel(dev->led_id);
    if (0 != res)
    {
        BSP_PRT(ERR, "Cancel led task failed: %s\n", strerror(errno));
        return -1;
    }

    /* Join with thread to see what its exit status was */
    res = pthread_join(dev->led_id, &ret);
    if (0 != res)
    {
        BSP_PRT(ERR, "Join cancel led task failed: %s\n", strerror(errno));
        return -2;
    }

    if (ret == PTHREAD_CANCELED)
    {
        BSP_PRT(DBG, "Led task is canceled.\n");
    }
    else
    {
        BSP_PRT(ERR, "Cancel led task failed\n");
        return -3;
    }

    return 0;
}
/*************************************************
 *   函数描述 : 设置通道led灯状态
 *   输入参数 : chan --通道号
 *              SigOrWork --led灯名称,CHAN_MOD_LED_SIG或者CHAN_MOD_LED_WORK
 *              status --led灯状态
 *   函数返回 : =0 成功
 *              <0 出错
 **************************************************/
int vs_usb_set_led_map(int chan, int SigOrWork, int status){
    int res, mb_idx, mb_chan;
	int i,start_chan, end_chan, success_cnt = 0;
	int cts1 = 0, rts1 = 0,  dtr1 = 0,  cts2 = 0, rts2 = 0, dtr2 = 0, high_flag = 0, pos_flag = 0;
    struct swg_mod_brd *mb;

	if(SWG_ALL_CHANS == chan)
	{
		start_chan = 0;
		end_chan = g_swg_dev->total_chans;
	}
	else
	{
		start_chan = chan - 1; //从0开始编号
		end_chan =  chan;
	}
	for(i = start_chan; i < end_chan; i++ ){
		if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[i].mod_type)
		{
			BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
				i, g_swg_dev->chans[i].mod_type);
			continue;
		}

		if((res = gchan_2_md_idx_chan(g_swg_dev, i+1, &mb_idx, &mb_chan)) != 0)
		{
			BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
				__FUNCTION__,i+1, res);
			continue;
		}

		mb = &g_swg_dev->mod_brd[mb_idx];
		if(mb->name != MB_SWG_VS_USB_BASE){
			BSP_PRT(ERR, "%s is not support channel led\n", mb_name_to_str(mb->name));
			continue;
		}
		
		if(SigOrWork == CHAN_MOD_LED_SIG){
			mb->reg_lamp_status[mb_chan] = status;
		}else if(SigOrWork == CHAN_MOD_LED_WORK){
			mb->call_lamp_status[mb_chan] = status;
		}else{
			BSP_PRT(ERR, "%s unkown led!", __FUNCTION__);
			continue;
		}
		
		pos_flag = mb_chan % CHAN_NUM_PER_CHAN_BRD;
		if(pos_flag == 0){
			vs_usb_translate_lamp(mb, mb_chan, &cts1, &rts1, &dtr1);
			vs_usb_translate_lamp(mb, mb_chan + 1, &cts2, &rts2, &dtr2);
		}else{
			vs_usb_translate_lamp(mb, mb_chan - 1, &cts1, &rts1, &dtr1);
			vs_usb_translate_lamp(mb, mb_chan, &cts2, &rts2, &dtr2);
		}

		if(mb_chan < MAX_CHAN_NUM_PER_VS_USB_MCU/2){
			high_flag = 0;
		}else{
			high_flag = 1;
		}
#ifdef VS_USB_M35_LED_DEBUG
		if(mb->swg_dev->chans[mb->base_chan_no].mod_type == CHAN_MOD_M35){
			if(mb_chan < MAX_CHAN_NUM_PER_VS_USB_MCU/2){
				high_flag = 1;
			}else{
				high_flag = 0;
			}
		}
#endif
		pthread_mutex_lock(&mb->lock);
		hwif_set_led_lamp(&mb->hwif, cts1, rts1, dtr1, cts2, rts2, dtr2, high_flag );
		pthread_mutex_unlock(&mb->lock);
		success_cnt++;
	}
	if(success_cnt <= 0)
		return -1;
	else
		return 0;
}

/*************************************************
 *   函数描述 : 设置通道signal led灯状态
 *   输入参数 : chan --通道号
 *              status --led灯状态
 *   函数返回 : =0 成功
 *              <0 出错
 **************************************************/

int vs_usb_set_sig_led_map(int chan, int status){
	int res = 0;
	if(!is_valid_g_chan_no(g_swg_dev, chan))
	{
		BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
				__FUNCTION__, chan, g_swg_dev->total_chans);
		return -1;
	}
	if(status < CHAN_LAMP_STATUS_OFF || status >= CHAN_LAMP_STATUS_MAX){
		BSP_PRT(ERR, "Unkown sig led status\n");
		return -1;
	}
	res = vs_usb_set_led_map(chan, CHAN_MOD_LED_SIG, status);
	return res;
}

/*************************************************
 *   函数描述 : 设置通道led灯状态
 *   输入参数 : chan --通道号
 *              SigOrWork --led灯名称
 *              status --led灯状态
 *   函数返回 : =0 成功
 *              <0 出错
 **************************************************/

int vs_usb_set_work_led_map(int chan, int status){
	int res = 0;
	if(!is_valid_g_chan_no(g_swg_dev, chan))
	{
		BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
				__FUNCTION__, chan, g_swg_dev->total_chans);
		return -1;
	}
	if(status == CHAN_LAMP_STATUS_OFF || status == CHAN_LAMP_STATUS_GREEN_ON || status == CHAN_LAMP_STATUS_GREEN_FLASH){
		res = vs_usb_set_led_map(chan, CHAN_MOD_LED_WORK, status);
	}else{		
		BSP_PRT(ERR, "Unkown work led status\n");
		return -1;
	}
	return res;
}

int vs_usb_set_mod_brd_led_map(int md_idx, int led,int status){
	int start_md;
	int end_md;
	int i;
	int success_cnt = 0;
	struct swg_mod_brd *mb;
	if(md_idx == SWG_ALL_MODS){
		start_md = 0;
		end_md = g_swg_dev->total_mod_brds;
	}else{
		start_md = md_idx - 1;
		end_md = md_idx;
	}

	if(led != MOD_BRD_LED_SYS){
		BSP_PRT(ERR, "Unkonw mod brd sys led status !\n");
		return -1;
	}

	for(i = start_md; i < end_md; i++){

		mb = &g_swg_dev->mod_brd[i];
		if(mb->name != MB_SWG_VS_USB_BASE){
			BSP_PRT(ERR, "mod brd is not vs usb device !\n");
			continue;
		}
		mb->sys_lamp_status = status;
		if(status != MOD_BRD_LAMP_STATUS_GREEN_FLASH){
			pthread_mutex_lock(&mb->lock);
			mod_brd_set_sys_led_lamp(&mb->hwif, status);
			pthread_mutex_unlock(&mb->lock);
		}
		success_cnt++;
	}
	return success_cnt > 0 ? 0 : -1;
}

/*************************************************
 *   函数描述 : 设置模块底板sys led灯状态
 *   输入参数 : chan --模块板编号
 *              SigOrWorkOrSys --led灯名称
 *              status --led灯状态
 *   函数返回 : =0 成功
 *              <0 出错
 **************************************************/
int vs_usb_set_mod_sys_led_map(int md, int status){
	int res = 0;
	if(!is_valid_g_md_idx(g_swg_dev, md))
	{
		BSP_PRT(ERR, "[%s] Invalied global mod_brd number %d(total %d) \n", 
				__FUNCTION__, md, g_swg_dev->total_mod_brds);
		return -1;
	}
	if(status == MOD_BRD_LAMP_STATUS_OFF ||
		status == MOD_BRD_LAMP_STATUS_GREEN_FLASH||
		status == MOD_BRD_LAMP_STATUS_GREEN_ON){
		res = vs_usb_set_mod_brd_led_map(md, MOD_BRD_LED_SYS, status);
	}else{
		BSP_PRT(ERR, "Unkown system led status\n");
		return -1;
	}
	return res;
}

/*************************************************
 *   函数描述 : 打开或关闭所有sig红灯，sig绿灯，work绿灯
 *   输入参数 : status --led灯状态， 0关闭，1开启
 *   函数返回 : =0 成功
 *              <0 出错
 **************************************************/

int vs_usb_set_all_led(int status){
	int i = 0, j = 0, success_cnt;
	struct swg_mod_brd *mb;
	if(status < 0){
		BSP_PRT(ERR, "%s unkown status\n", __FUNCTION__);
		return -1;
	}
	
	for(i = 0; i < g_swg_dev->total_mod_brds; i++){
		if(!is_valid_g_md_idx(g_swg_dev, i))
		{
			BSP_PRT(ERR, "[%s] Invalied mod brd %d(total %d) \n", 
					__FUNCTION__, i, g_swg_dev->total_mod_brds);
			continue;
		}
            
		if(g_swg_dev->mod_brd[i].status != MB_STATUS_INITED || g_swg_dev->mod_brd[i].name != MB_SWG_VS_USB_BASE)
		{
			BSP_PRT(WARN, "Mod brd %d(status = %d) is not a vs-usb mod brd\n",
					i + 1, g_swg_dev->mod_brd[i].status);
			continue;
		}
		
		mb = &g_swg_dev->mod_brd[i];
		//控制所有led灯时，将控制线程里面的led状态都设置为关闭。
		for(j = 0; j < MAX_CHAN_NUM_PER_VS_USB_MCU; j++){
			mb->call_lamp_status[j] = CHAN_LAMP_STATUS_OFF;
			mb->reg_lamp_status[j] = CHAN_LAMP_STATUS_OFF;
		}
		pthread_mutex_lock(&mb->lock);
		hwif_set_all_led_lamp(&mb->hwif, status);
		pthread_mutex_unlock(&mb->lock);
		success_cnt++;
	}
	
	if(success_cnt <= 0)
		return -1;
	else
		return 0;
}
/*************************************************
 *   函数描述 : 获取系统类型
 *   输入参数 : 无
 *   函数返回 : 系统类型
 *              <0 出错
 **************************************************/
int swg_get_sys_type(void){
	return g_swg_dev->sys_type;
}

/*************************************************
 *   函数描述 : 定时开机模块
 *   输入参数 : 无
 *   函数返回 : 系统类型
 *              <0 出错
 **************************************************/
int swg_chan_mod_power_on_module_timer(int chan, int timer){
	int res;
    int md_idx, md_chan;
	struct timespec tv;
	enum chan_mod_power_event_e event;
    struct swg_mod_brd *mb = NULL;
	
    if(!is_valid_g_chan_no(g_swg_dev, chan))
    {
        BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
                __FUNCTION__, chan, g_swg_dev->total_chans);
        return -1;
    }
	
	event = CHAN_MOD_POWER_ON;
	
	chan--;
	
    if(CHAN_MOD_TYPE_UNKOWN == g_swg_dev->chans[chan].mod_type)
    {
        BSP_PRT(VERB, "Chan %d channel module type(%d) is unknow)\n",
                 chan+1 , g_swg_dev->chans[chan].mod_type);
        return -1;
    }

    if(IS_UPGRADE_STATUS == g_swg_dev->chans[chan].upgrade_flag){
        BSP_PRT(ERR, "Chan %d is updating, power off failed!\n");
     	return -1;
    }

    if((res = gchan_2_md_idx_chan(g_swg_dev, chan+1, &md_idx, &md_chan)) != 0)
    {
        BSP_PRT(INFO, "[%s](global chan %d) gchan_2_md_idx_chan failed , res = %d) \n", 
                __FUNCTION__, chan + 1, res);
        return -1;
    }
    mb = &g_swg_dev->mod_brd[md_idx];

	if(mb->power_event[md_chan] != CHAN_MOD_POWER_UNKOWN || mb->power_next_event[md_chan] != CHAN_MOD_POWER_UNKOWN)
		return -2;

	swg_chan_set_power_on_off_module_time(&g_swg_dev->chans[chan], event);
	
	g_swg_dev->chans[chan].chan_power_on_off_delay+=200;//确保powerkey事件能正常处理
	
	mb->power_event[md_chan] = event;
	
	clock_gettime(CLOCK_MONOTONIC, &tv);
	tv.tv_sec+= timer;
	g_swg_dev->chans[chan].timeout.tv_sec = tv.tv_sec;//设置超时时间，保存模块正常起来
	g_swg_dev->chans[chan].chan_power_on_off_try_cnt = 1;

	return 0;
}


int swg_set_sim_card_slot(int chan, int slot){
	if(g_swg_dev->sys_type != SYS_TYPE_1CHAN4SIMS){
		BSP_PRT(ERR, "Is Not 1CHAN4SIMS SYS TYPE. \n");
		return -1;
	}
	if(slot <= 0)
		return -2;
	if(slot > 4)
		return -3;
	
	if((chan & 0xFFFF) == SWG_ALL_CHANS)
		return set_all_sim_slot(g_swg_dev, slot);
	else
		return set_sim_slot(g_swg_dev,chan, slot);
}

int swg_get_sim_state_all(int chan,int state[][4]){
	if(g_swg_dev->sys_type != SYS_TYPE_1CHAN4SIMS){
		BSP_PRT(ERR, "Is Not 1CHAN4SIMS SYS TYPE. \n");
		return -1;
	}
	if(!is_valid_g_chan_no(g_swg_dev, chan))
	{
		BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
		        __FUNCTION__, chan, g_swg_dev->total_chans);
		return -1;
	}
	get_sim_state_all(g_swg_dev, chan, state);
	return 0;
}

int swg_get_sim_state_one(int chan, int card, int *state){
	if(g_swg_dev->sys_type != SYS_TYPE_1CHAN4SIMS){
		BSP_PRT(ERR, "Is Not 1CHAN4SIMS SYS TYPE. \n");
		return -1;
	}
	if(!is_valid_g_chan_no(g_swg_dev, chan))
	{
		BSP_PRT(ERR, "[%s] Invalied global channel number %d(total %d) \n", 
		        __FUNCTION__, chan, g_swg_dev->total_chans);
		return -1;
	}
	if(card <= 0)
		return -1;
	if(card > 4)//max card is 4
		return -1;
	get_sim_state_one(g_swg_dev, chan, card,state);
	return 0;
}

int swg_get_simswitch_info(int index, struct simswitch_ver_info_s *info)
{
	if(g_swg_dev->sys_type != SYS_TYPE_1CHAN4SIMS){
		BSP_PRT(ERR, "Is not 1CHAN4SIMS SYS TYPE. \n");
		return -1;
	}
	if(index > SIM_SWITCH_NUM){
		BSP_PRT(ERR, "index[%d] out of range.\n", index);
		return -1;
	}
	if(index <= 0){
		BSP_PRT(ERR, "index[%d] out of range.\n", index);
		return -1;
	}
	--index;

	if(SIM_SWTICH_STATUS_INITED != g_swg_dev->sim_switch[index].state){
		BSP_PRT(ERR, "simswitch[%d] not inited.\n", index+1);
		return -1;
	}
	info->hw_ver = g_swg_dev->sim_switch[index].hw_ver;
	info->sw_ver = g_swg_dev->sim_switch[index].sw_ver;
	return 0;
}
