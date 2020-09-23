

/****************************************************************************
* 版权信息：
* 系统名称：SimRdrSvr
* 文件名称：EmuRegisterList.h 
* 文件说明：Emu注册管理列表头文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

/**************************** 条件编译选项和头文件 ****************************/
#ifndef __SIMRDRSVR_H__
#define __SIMRDRSVR_H__


#include <stdio.h>
#include <stdlib.h>


//#include "uicomm.h"

#include <pthread.h>
//#include <sched.h>
//#include <semaphore.h>

#include "CSocketEx.h"
#include "msg.h"
#include "hidapi.h"

//For burntest by WX 



#define MAXHIDPORT  40
#define SIMLEDCTROLLERID  500

#define RED_CODE  0x91
#define GREEN_CODE  0x92
#define BLACK_CODE  0x93

#define RESET "\033[0m"
#define BLACK "\033[30m" /* Black */
#define RED "\033[31m" /* Red */
#define GREEN "\033[32m" /* Green */
#define YELLOW "\033[33m" /* Yellow */
#define BLUE "\033[34m" /* Blue */
#define MAGENTA "\033[35m" /* Magenta */
#define CYAN "\033[36m" /* Cyan */
#define WHITE "\033[37m" /* White */
#define BOLDBLACK "\033[1m\033[30m" /* Bold Black */
#define BOLDRED "\033[1m\033[31m" /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m" /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m" /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m" /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m" /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m" /* Bold White */

#define SIMRDRHIDBUFLEN 320
#define SIMlEDCTLBUFLEN 64
#define SCANATRNUMBER 5
#define EXITDURATION 2
#define INVALID_HANDLE_VALUE  NULL

struct HidComPara{
hid_device *HidPortHandle;
int  ThreadOrder;		
};

void Initiate_SimBoxAll(void);



#define BANK_VID 0xC216
#define BANK_PID 0xFE02

#define USB_BUFF_SIZE 320

#define LEN_OFFS 4

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#define MAX_EMU_LINK_CONN 9999



// config file
#define SIMRDRSVR_CONFIG				"SimRdrSvr.conf"
#define KEY_SERI						"seri"
#define KEY_PASSWD						"passwd"
#define KEY_NET_NODE					"net_mode"
#define KEY_LOCAL_IP					"local_ip"
#define KEY_SERVER_IP					"server_ip"
#define KEY_SERVER_PORT					"server_port"
#define KEY_HB_INTERVAL					"heartbeat_interval"
#define KEY_USB_FILE					"usb_file"
//#define KEY_MAX_RDR_NBR					"max_rdr_nbr"
#define KEY_SIM_HDL_PORT_USB			"sim_hdl_port_usb"
#define KEY_SIM_HDL_PORT_NET			"sim_hdl_port_net"
#define KEY_SIM_HDL_PORT_PROXY			"sim_hdl_port_proxy"
#define KEY_USB_HDL_PORT				"usb_hdl_port"
#define KEY_NET_HDL_PORT				"net_hdl_port"
#define KEY_NET_HDL_PORT_PHP			"net_hdl_port_php"

#define KEY_PHP_HDL_IP					"php_hdl_ip"
#define KEY_PHP_HDL_PORT				"php_hdl_port"


#ifdef WIN32
#define SLOT_START 1
#define USB_FILE_PREFIX "ui_hid"
#else
#define SLOT_START 0
#define USB_FILE_PREFIX "/dev/usb/hiddev"
#endif



#define TIMEOUT_USEC 1000000

#define SIMCARD_PLUSPUSH_CHECK_INTERVAL 10


#define SLOT_IS_EMU_CTRL 0x80 //STM32 self;
#define SLOT_IS_RDR_CTRL 0x80 //STM32 self;

//  <->pc
//Bank Side 指令
#define CMD_IS_DATA    0    //不需要resp
//  <- pc
#define CMD_IS_SETATR  0x81 //需要resp
#define CMD_IS_GETVER  0x83 //需要resp
#define CMD_IS_REQRST  0x85 //需要resp reset itself
#define CMD_IS_GETATR  0X71
#define CMD_IS_GETATR_FULLRESET  0X72
#define CMD_IS_BACK2DFU  0X87
#define CMD_IS_GETSIMCARD_DETECT  0X89
#define CMD_IS_GETBOARDID  0X88
#define CMD_IS_IAP  0X61

//Bank Side LEDCtrlr指令
#define CMD_IS_LED_RED  0X91
#define CMD_IS_LED_GREEN  0X92
#define CMD_IS_LED_OFF   0X93
#define CMD_IS_LED_RED_FLASH  0X94
#define CMD_IS_LED_GREEN_FLASH  0X95

//EMU Side 指令
//  -> PC
#define IS_REQ_RST_ICC    0x10 //不需要resp
#define IS_VER            0x82 //resp to pc
#define IS_RESP_RST       0x84 //resp to pc at reset


#define TYPE_SBX_BD "SBX_BD"
#define TYPE_IAP_SBX_BD "IAPSBX_BD"
#define TYPE_SIMRDRII "SimRdrII"
#define TYPE_IAP_SIMRDRII "IAPRDR"



enum
{
	SBX_BD = 1,
	SimRdrII = 2
};



enum
{
	NoErr = 0,
	Error = 1
};

enum
{
	Sim_Plug = 0,
	Sim_Pull = 1
};

enum
{
	USB_Invalid = 0,
	USB_Valid   = 1
};

typedef int (*NET_HANDLE_FUNC)(char *buff_req, char *buff_rsp, CSocketEx *csock);
typedef struct NET_HANDLE_S
{
	unsigned short cmd;
	NET_HANDLE_FUNC cmd_handle;
}net_handle_t;




#pragma pack(1)



// enum
// {
// 	Sim_Pull = 0,	// 拔出
// 	Sim_Plug = 1	// 插入
// };

// sim卡插拔状态数据结构
typedef struct SIM_PLUS_STAT_S
{
	unsigned short stat; // 0:pull; 1:plug
}sim_plus_stat_t;

// sim卡属性ATR数据结构
typedef struct SIM_ATR_S
{
	unsigned char sim_atr_ori[320];		// 原始ATR
	unsigned char sim_atr_mod[320];		// 修正ATR
	unsigned int len_ori;				// 原始ATR长度
	unsigned int len_mod;				// 修正ATR长度
	unsigned char td;					// 0:反向；1:正向
}sim_atr_t;


// 卡槽信息数据结构
typedef struct SLOT_INFO_S
{
	unsigned short gw_bank_nbr;
	unsigned short slot_nbr;		// 卡槽号
	unsigned int svr_ip;			// 服务器IP地址
	unsigned short svr_port;		// 服务器PORT端口
	unsigned short sb_bank_nbr;
	unsigned short sim_nbr;			// sim卡号
	unsigned short stat;			// 状态
	//char svr_ip[16];
}slot_info_t;

// sim卡处理信息数据结构
typedef struct SIMCARD_HANDLE_INFO_S
{
	unsigned short cmd;			// 命令
	char emu_seri[10];			// Emu序列号
	slot_info_t slot_info;		// 卡槽信息
}simcard_handle_info_t;

// 消息头数据结构
typedef struct MSG_HEADER_S
{
	unsigned short cmd;			// 1: sim data; 2:sim release
	unsigned short bank;		// bank nunmber, emu is reserve
	unsigned short slot;		// slot number
	unsigned short data_len;	// data len
	unsigned int cnt;			// count
}msg_header_t;

// 消息数据结构
typedef struct MSG_S
{
	msg_header_t header;		// 消息头
	char data[BUFF_SIZE];		// 消息体
}msg_t;

#pragma pack()

// Rdr注册状态数据结构
typedef struct REGISTER_STAT_S
{
	int stat;				// 状态，0:unregister; 1:registed
	pthread_mutex_t lock;	// 互斥锁
}register_stat_t;

// SimRdrSvr服务器配置参数数据结构，用于保存程序从配置文件读取的参数信息
typedef struct SIMRDRSVRPARAM_S
{
	pthread_t tid;
	char seri[12];						// 
	char passwd[64];					// 注册密码
	char net_mode[12];					// 网络模式，server：服务端；client：客户端
	char local_ip[16];					// 本机IP地址
	char server_ip[16];					// 服务器IP地址
	unsigned short server_port;			// 服务器PORT端口
	unsigned short hb_interval;			// 心跳间隔
	char usb_file[32];					// usb口文件名
	//unsigned short max_rdr_nbr;				// rdr sim板卡最大数量
	unsigned short sim_hdl_port_usb;
	unsigned short sim_hdl_port_net;
	unsigned short sim_hdl_port_proxy;
	unsigned short usb_hdl_port;
	unsigned short net_hdl_port;
	unsigned short net_hdl_port_php;	// NetHdlTask与PHP通讯端口
	char php_hdl_ip[16];				// PHP与NetHdlTask通讯IP
	unsigned short php_hdl_port;		// PHP与NetHdlTask通讯端口
}simrdrsvrparam_t;

// sim卡状态信息数据结构
typedef struct SIM_STAT_S
{
	unsigned char stat;				// 当前状态
	unsigned char sync;				// 同步状态
}sim_stat_t;

// SimHdlTask线程参数数据结构
typedef struct SIMHDLTASK_PARAM_S
{
	pthread_t tid;						// 线程号
	unsigned short usb_nbr;				// usb文件号
	unsigned short sim_nbr;				// sim卡号
	unsigned short svr_port;			// Proxy服务器PORT端口
	unsigned short usb_port;			// Port端口，用于连接USBHdlTask
	unsigned short net_port;			// Port端口，用于连接NetHdlTask
	unsigned short proxy_port;			// Port端口，用于连接Proxy
	unsigned short usbhdl_port;
	unsigned short nethdl_port;
	unsigned short stop;				// 停止标志。0：not stop；1：stop
	char svr_ip[16];					// 服务器IP地址
	char localhost[16];					// 本地IP地址	
	sim_atr_t *sim_atr;
}simhdltask_param_t;

// USBHdlTask线程参数数据结构
typedef struct USBHDLTASK_PARAM_S
{
	pthread_t tid;
#ifdef WIN32
	void *handle;						// usb句柄
#else
	//FILE *handle;						// usb句柄
	hid_device *handle;
#endif
	char usb_file[32];
	unsigned short usb_nbr;				// usb文件号
	unsigned short bank_nbr;			// bank号
	unsigned short usb_hdl_port;
	unsigned short stop;
	unsigned char old_stat;				// sim卡原状态，0～7bit对应0～7通道
	sim_stat_t new_stat;				// sim新状态
	unsigned char type;					// usb类型。1：SBX_BD；2：SimRdrII
	unsigned short stat;				// 生效标志。0:invalid; 1:valid
	sim_atr_t sim_atr[SIM_NBR];			// sim atr数据
}usbhdltask_param_t;

typedef struct SIMSERVER_INFO_S
{
// 	char file[32];						// usb文件路径
// 	int bank_nbr;						// bank号

// 	unsigned short usb_hdl_port;
// 	unsigned short stop;

	usbhdltask_param_t usbhdltask_param;
	
	//simrdrsvrparam_t simrdrsvrparam[SIM_NBR];
	simhdltask_param_t simhdltask_param[SIM_NBR];
}simserver_info_t;

typedef struct SBX_BD_INFO_s
{
#ifdef WIN32
	void *handle;						// usb句柄
#else
	hid_device *handle;
#endif
	char usb_file[32];
}sbx_bd_info_t;

/**************************** 函数声明和定义 ****************************/

/**************************************************************************** 
* 函数名称 : readConfigValue
* 功能描述 : 读取配置文件
* 参    数 : void
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int readConfigValue(simrdrsvrparam_t *param);

/**************************************************************************** 
* 函数名称 : RdrSimDataSyncHdl
* 功能描述 : sim卡信息同步处理函数
* 参    数 : char *buff_req				: 请求缓冲，保存心跳请求内容
* 参    数 : char *buff_rsp				: 响应缓冲
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int RdrSimDataSyncHdl(char *buff_req, char *buff_rsp);

/**************************************************************************** 
* 函数名称 : parseSimAtr
* 功能描述 : 解析sim卡ATR属性
* 参    数 : unsigned char *data		: sim卡ATR缓冲，保存ATR内容
* 参    数 : unsigned long len			: sim卡ATR长度
* 参    数 : sim_atr_t *simAtr			: sim卡ATR信息数据结构缓冲
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int parseSimAtr(unsigned char *data, unsigned int len, sim_atr_t *simAtr);

/**************************************************************************** 
* 函数名称 : makeRdrCmd
* 功能描述 : rdr命令处理，转换Emu的负载数据的格式，用于发送到Rdr
* 参    数 : char *data					: 负载数据
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeRdrCmd(char *data);

/**************************************************************************** 
* 函数名称 : makeRspToEmu
* 功能描述 : rdr命令处理，转换Rdr响应的负载数据的格式，用于发送到Emu
* 参    数 : char *data					: 负载数据
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeRspToEmu(unsigned char *rsp, unsigned long *len, usbhdltask_param_t *param);

/**************************************************************************** 
* 函数名称 : RdrSimReqHdl
* 功能描述 : sim卡申请处理函数
* 参    数 : char *buff_req				: 请求数据缓冲，存储着sim卡申请报文数据
* 参    数 : char *buff_rsp				: 响应数据缓冲
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
//int RdrSimReqHdl(char *buff_req, char *buff_rsp);

/**************************************************************************** 
* 函数名称 : RdrSimReleaseHdl
* 功能描述 : sim卡释放处理函数
* 参    数 : char *buff_req				: 请求数据缓冲，存储着sim卡释放报文数据
* 参    数 : char *buff_rsp				: 响应数据缓冲
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
//int RdrSimReleaseHdl(char *buff_req, char *buff_rsp);

/**************************************************************************** 
* 函数名称 : initHid
* 功能描述 : 初始化USB连接
* 参    数 : char *usb_file		: usb文件路径, ui_hid1, ui_hid2, ...
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
//int initHid(char *usb_file);

/**************************************************************************** 
* 函数名称 : USBHdlTask
* 功能描述 : USB处理线程函数
* 参    数 : void *pParam				: 线程函数参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void * USBHdlTask(void *pParam);

/**************************************************************************** 
* 函数名称 : EmuConnHdlTask
* 功能描述 : Emu连接管理线程函数
* 参    数 : void *pParam				: 线程函数参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void * EmuConnHdlTask(void *pParam);

/**************************************************************************** 
* 函数名称 : NetHdlTask
* 功能描述 : 负责接收Emu连接
* 参    数 : void *pParam				: 线程函数参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void * NetHdlTask(void *pParam);

/**************************************************************************** 
* 函数名称 : SimHdlTask
* 功能描述 : 负责sim卡业务数据处理
* 参    数 : void *pParam				: 线程函数参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void * SimHdlTask(void *pParam);

/**************************************************************************** 
* 函数名称 : getSimStat
* 功能描述 : 获取sim卡插拔状态
* 参    数 : hid_device *handle		: usb句柄
* 参    数 : sim_stat_t *stats			: sim卡状态信息
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getSimStat(hid_device *handle, sim_stat_t *stats);

//int USBDataHdl(CSocketEx *csock, char *buff, unsigned buff_len, void * usbhdl);
int deviceLinkCreateReqHdl(char *buff_req, char *buff_rsp, CSocketEx *csock);
int deviceLinkReleaseReqHdl(char *buff_req, char *buff_rsp, CSocketEx *csock);

int HeartbeatRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int LinkInfoReportRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SimLinkCreateReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SimLinkReleaseReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

int rdrSimReport(char *local_ip, unsigned short local_port, unsigned short bank_nbr, unsigned short sim_nbr, unsigned char *sim_atr, unsigned int atr_len);

/**************************************************************************** 
* 函数名称 : getSimAtr
* 功能描述 : 获取sim卡atr属性
* 参    数 : hid_device *handle			: usb句柄
* 参    数 : sim_atr_t *atr				: sim卡属性数据结构
* 参    数 : unsigned short sim_nbr		: sim卡号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getSimAtr(hid_device *handle, sim_atr_t *atr, unsigned short sim_nbr);

/**************************************************************************** 
* 函数名称 : getBandNbr
* 功能描述 : 获取bank号
* 参    数 : hid_device *handle			: usb句柄
* 参    数 : unsigned char bank_nb		: bank号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getBankNbr(hid_device *handle, unsigned char *bank_nbr);

/**************************************************************************** 
* 函数名称 : getBankVersion
* 功能描述 : 获取bank版本
* 参    数 : getBankVersion *handle			: usb句柄
* 参    数 : unsigned char bank_nb		: bank号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getBankVersion(hid_device *handle, char *version, unsigned short version_len);

/**************************************************************************** 
* 函数名称 : hidWR
* 功能描述 : 读写HID设备
* 参    数 : getBankVersion *handle			: usb句柄
* 参    数 : unsigned char *req				: 写入缓冲
* 参    数 : unsigned short req_len				: 写入缓冲长度
* 参    数 : unsigned char *rsp				: 读出缓冲
* 参    数 : unsigned short rsp_len				: 读出缓冲长度
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int hidWR(hid_device *handle, unsigned char *req, unsigned short req_len, unsigned char *rsp, unsigned short *rsp_len);

#endif