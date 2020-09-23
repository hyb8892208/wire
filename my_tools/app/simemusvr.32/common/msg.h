/****************************************************************************
* 版权信息：
* 系统名称：SimServer
* 文件名称：msg.h 
* 文件说明：网络消息头文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

/**************************** 条件编译选项和头文件 ****************************/
#ifndef __MSG_H__
#define __MSG_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "CSocketEx.h"



#define VERSION					0x0002					// 报文版本号

#define REGISTER_REQ			0x0001					// 注册请求包
#define REGISTER_RSP			0x1001					// 注册响应包
#define UNREGISTER_REQ			0x0002					// 注销请求包
#define UNREGISTER_RSP			0x1002					// 注销响应包
#define HEARTBEAT_REQ			0x0003					// 心跳请求包
#define HEARTBEAT_RSP			0x1003					// 心跳响应包
#define SIMDATASYNC_REQ			0x0004					// sim数据同步请求包
#define SIMDATASYNC_RSP			0x1004					// sim数据同步响应包
// #define SIM_REQ					0x0005					// sim卡申请请求包
// #define SIM_RSP					0x1005					// sim卡申请响应包
#define SIMLINK_CREATE_REQ		0x0005					// sim卡线路建立请求
#define SIMLINK_CREATE_RSP		0x1005					// sim卡线路建立响应
#define SIMDATA_REQ				0x0007					// sim数据透传请求包
#define SIMDATA_RSP				0x1007					// sim数据透传响应包
// #define SIMRELEASE_REQ			0x0007					// sim卡释放请求包
// #define SIMRELEASE_RSP			0x1007					// sim卡释放响应包
#define SIMLINK_RELEASE_REQ		0x0006					// sim卡线路释放请求
#define SIMLINK_RELEASE_RSP		0x1006					// sim卡线路释放响应
#define SIMDATADELAY_REQ		0x0008					// sim卡数据延时通知请求包		<---- 已在Emu板卡实现，网络无需传输此类报文
#define SIMHPD_REQ				0x0009					// 
#define SIMHPD_RSP				0x1009					// 
#define SIMPULLPLUG_REQ			0x000A					// sim卡插拔通知请求包
#define SIMPULLPLUG_RSP			0x100A					// sim卡插拔通知响应包

#define POWER_RESET_SPAN_REQ	0x000B
#define POWER_RESET_SPAN_RSP	0x100B

#define CHAN_HANG_UP_REQ		0x000C
#define CHAN_HANG_UP_RSP		0x100C

#define SIMSMS_REQ				0x000D					// sim卡短信发送请求包
#define SIMSMS_RSP				0x100D					// sim卡短信发送响应包

#define NAT_ACROSS_START_NOTICE		0x0011
#define NAT_ACROSS_INFO				0x0012
#define NAT_ACROSS_PEER_INFO		0x0013

#define LINKINFO_REPORT_REQ		0x0004					// 线路信息上报
#define LINKINFO_REPORT_RSP		0x1004					// 线路信息上报响应

//new interface(amq)
#define SET_MODULE_LIMIT		0x0020
#define EVENT_MODULE_LIMIT      0x1020
#define SET_MODULE_INTERNET     0x0021 
#define EVENT_MODULE_INTERNET   0x1021
#define EVENT_SMS_RECEIVE       0x1022
#define SEND_USSD				0x0023
#define EVENT_USSD_RESULT		0x1023
#define SET_MODULE_SMS_MODE		0x0024




#define SIMATR_RESET_NOTICE		0x0051

#define SERIALNO_LEN			10						// SimEmuSvr序列号长度
#define PACKAGE_HEADER_LEN		10						// 报文头部长度
#define MESSAGE_LEN				140
#define LONG_MESSAGE_LEN		MESSAGE_LEN * 7			//长短信长度

#define SIM_NBR 8

// asterisk msg
#define CALL_BEGIN_NOTICE		0x0101
#define CALL_END_NOTICE			0x0102
#define CALL_MORING_NOTICE		0X0103
#define SMS_RESULT_NOTICE			0X0104
#define SMS_REPORT_NOTICE			0X0105
#define USSD_RESULT_NOTICE		0x0106


#define SIM_PULLPLUG_NOTICE_REQ	0x0201
#define SIM_PULLPLUG_NOTICE_RSP	0x1202

#define SIM_PULL_NOTICE_REQ	0x0203						// sim卡拔出通知
#define SIM_PULL_NOTICE_RSP	0x1203						// sim卡拔出通知响应
#define SIM_PLUG_NOTICE_REQ	0x0204						// sim卡插入通知
#define SIM_PLUG_NOTICE_RSP	0x1204						// sim卡插入通知响应

#define AST_CHAN_STAT_NOTICE_REQ 0x0205					//sim卡注册结果响应


#define LINK_BREAK_NOTICE_REQ	0x0211
#define LINK_BREAK_NOTICE_RSP	0x1212

#define AMI_EVENT_MSG			0x0301
#define AMI_CLI_MSG				0x0302

#define VGSM_LEN				0xF000	//60K		//0x10000		// 虚拟sim卡缓存长度，64K


// Emu Slot nbr define
#ifndef SLOT_NBR
#define SLOT_NBR 8
#endif

#ifndef MAX_CHN
#define MAX_CHN 32
#endif

#define MAX_BOARD (MAX_CHN / SLOT_NBR)
#define MAX_UUID_LEN 24
#define MAX_PHONE_LEN 16

// Rdr Sim nbr define
#ifdef RDR10
#define RDR_NBR 10
#endif
#ifdef RDR20
#define RDR_NBR 20
#endif
#ifdef RDR30
#define RDR_NBR 30
#endif
#ifdef RDR40
#define RDR_NBR 40
#endif
#ifdef RDR50
#define RDR_NBR 50
#endif
#ifdef RDR60
#define RDR_NBR 60
#endif


#define DIRE_UNKOWN							"[Unkown                     ]"
#define DIRE_COMM_TO_COMMHDLTASK			"[Comm        ==> CommHdlTask]"
#define DIRE_COMMHDLTASK_TO_COMM			"[CommHdlTask ==> Comm       ]"
#define DIRE_COMMHDLTASK_TO_SLOTHDLTASK		"[CommHdlTask ==> SlotHdlTask]"
#define DIRE_SLOTHDLTASK_TO_COMMHDLTASK		"[SlotHdlTask ==> CommHdlTask]"
#define DIRE_SLOTHDLTASK_TO_SIMHDLTASK		"[SlotHdlTask ==> SimHdlTask ]"
#define DIRE_SIMHDLTASK_TO_SLOTHDLTASK		"[SimHdlTask  ==> SlotHdlTask]"
#define DIRE_USB_TO_USBHDLTASK				"[Usb         ==> UsbHdlTask ]"
#define DIRE_USBHDLTASK_TO_USB				"[UsbHdlTask  ==> Usb        ]"
#define DIRE_USBHDLTASK_TO_SIMHDLTASK		"[UsbHdlTask  ==> SimHdlTask ]"
#define DIRE_SIMHDLTASK_TO_USBHDLTASK		"[SimHdlTask  ==> UsbHdlTask ]"
#define DIRE_NET_TO_COMMHDLTASK				"[NET         ==> CommHdlTask]"
#define DIRE_EAE_TO_NET						"[EAE         ==> NET        ]"


#define BUFF_SIZE 8192//0xFFFF	//2048//1024
#define TRANSFORM(bank,slot) ((bank) * 8 + (slot) + 1)
#define EVENT_STR_SUCCESS "success"
#define EVENT_STR_FAIL    "fail"


// 服务器类型码
enum
{
	SimEmuSvr		= 1,		// SimEmuSvr
	SimRdrSvr		= 2,		// SimRdrSvr
	SimProxySvr		= 3			// SimProxySvr
};


enum
{
	REGISTER_UP   = 0,
	REGISTER_DOWN = 1
};

enum
{
	EVENT_FAIL   = 0,
	EVENT_SUCCESS = 1
};


#pragma pack(1)
// 网络交互包头数据结构
typedef struct PACKAGE_HEADER_S
{
	unsigned short version;			// 版本信息
	unsigned short cmd;				// 消息命令
	unsigned short len;				// 负载长度
	unsigned short result;			// 结果码。0：成功；1：失败
	//unsigned short server_type;		// 服务器类型
	//char serial[10];				// 发起方序列号
	unsigned short reserve;			// 保留。
}package_header_t;
// 网络交互包数据结构
typedef struct PACKAGE_S
{
	package_header_t header;		// 包头
	char body[BUFF_SIZE];			// 负载
}package_t;


typedef struct SIM_HANG_UP_REQ_S
{
	char gw_seri[12];
	unsigned short gw_bank_nbr;
	unsigned short gw_slot_nbr;
}sim_hang_up_req_t;

typedef struct SMS_SEND_REQ_INFO_S
{
	unsigned short sms_type;		// 发送短信类型 1:查询号码 2:查话费 3:其他
	char seri[12];					// 接收方序列号
	unsigned short sb_bank_nbr;		// bank板号
	unsigned short sb_slot_nbr;		// 发起方Sim卡或卡槽数量
	unsigned short chn_nbr;			// 端口号，asterisk显示的端口号
	unsigned short gw_bank_nbr;
	unsigned short gw_slot_nbr;
	char dst_num[16];
	char src_num[16];
	char match_str[32];
	char send_msg[MESSAGE_LEN];
}sms_send_req_info_t;

typedef struct SMS_SEND_RSP_INFO_S
{
	unsigned short sms_type;		// 发送短信类型 1:查询号码 2:查话费 3:其他
	char sb_seri[12];				// 发送方序列号
	unsigned short sb_bank_nbr;		// bank板号
	unsigned short sb_slot_nbr;		// 发起方Sim卡或卡槽数量
	char recv_msg[LONG_MESSAGE_LEN];
	char match_msg[MESSAGE_LEN];
	char caller[16];
	char recv_time[32];
}sms_send_rsp_info_t;

// 注册请求数据结构
typedef struct REGISTER_REQ_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	char model_name[12];
	unsigned short slot_num;
	char passwd[64];				// 密码
}register_req_info_t;
typedef struct REGISTER_RSP_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	char model_name[12];
	unsigned short slot_num;
}register_rsp_info_t;

// 注销数据结构
typedef struct UNREGISTER_REQ_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;		// bank板数量
	unsigned short slot_nbr;		// 发起方Sim卡或卡槽数量
}unregister_req_info_t;
typedef struct UNREGISTER_RSP_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;		// bank板数量
	unsigned short slot_nbr;		// 发起方Sim卡或卡槽数量
}unregister_rsp_info_t;

// 心跳数据结构
typedef struct HEARTBEAT_REQ_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;		// bank板数量
	unsigned short slot_nbr;		// 发起方Sim卡或卡槽数量
}heartbeat_req_info_t;
typedef struct HEARTBEAT_RSP_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;		// bank板数量
	unsigned short slot_nbr;		// 发起方Sim卡或卡槽数量
}heartbeat_rsp_info_t;


// 线路上报数据结构
typedef struct REPORT_REQ_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	unsigned short module_type;		// 通信模块类型 1:GSM 2:CMDA 3:全网通 4:其他
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short usb_nbr;
	unsigned short bank_nbr;		// bank板号
	unsigned short slot_nbr;		// 发起方Sim卡或卡槽数量
	unsigned short chn_nbr;			// 端口号，asterisk显示的端口号
	unsigned int ip;
	unsigned short port;
	unsigned short atr_len;
	char atr[32];					// sim atr, emu上报时为空
	char eficcid[24];
	unsigned short vgsm_len;
	unsigned char vgsm[VGSM_LEN];	// 虚拟sim卡缓存
}report_req_info_t;

typedef struct REPORT_RSP_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short usb_nbr;
	unsigned short bank_nbr;		// bank板号
	unsigned short slot_nbr;		// 发起方Sim卡或卡槽数量
	unsigned short stat;
}report_rsp_info_t;

typedef struct LINK_INFO_S
{
	char sb_seri[10];
	unsigned short sb_usb_nbr;		// bank板号
	unsigned short sb_bank_nbr;		// bank板号
	unsigned short sim_nbr;
	char eficcid[24];
	unsigned int sb_ip;
	unsigned short sb_port;
	unsigned short atr_len;
	char sim_atr[32];
	char gw_seri[10];
	unsigned short gw_bank_nbr;		// 暂时为0
	unsigned short slot_nbr;
	unsigned int gw_ip;
	unsigned short gw_port;
	unsigned short led_stat;
	//unsigned short vgsm_len;
	short call_rest_time;	//通话剩余时间
	//char vgsm[VGSM_LEN];			// 虚拟sim卡缓存
}link_info_t;


// sim卡插拔上报数据结构
typedef struct SIM_PULLPLUG_INFO_S
{
	char sb_seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short sb_usb_nbr;
	unsigned short sb_bank_nbr;		// bank板号
	unsigned short sb_slot_nbr;		// 发起方Sim卡或卡槽数量
	char gw_seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short gw_bank_nbr;
	unsigned short gw_slot_nbr;		// bank板号
}sim_pullplug_info_t;


typedef struct CALL_MORING_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;
	unsigned short slot_nbr;
	char calldst[16];
}call_moring_info_t;
// 网关呼叫通知
typedef struct CALL_BEGIN_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;
	unsigned short slot_nbr;
	char calldst[16];
}call_begin_info_t;
// 网关呼叫接通通知
typedef struct CALL_END_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;
	unsigned short slot_nbr;
}call_end_info_t;

typedef struct SMS_RESULT_INFO_S
{
	unsigned short type;			// 服务器类型，1：SimEmuSvr；2：SimRdrSvr；3：SimProxySvr
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;
	unsigned short slot_nbr;
	unsigned char stat;
	char phone[MAX_PHONE_LEN];
 	char uuid[MAX_UUID_LEN];
}sms_result_info_t;

typedef struct MODULE_LIMIT_T
{
	char seri[SERIALNO_LEN];
	unsigned char limit;
	unsigned char cil_stat;           //cli run fail or success
	unsigned short bank;
	unsigned short slot;
}module_limit_t;

typedef struct module_internet_info_s{
	char seri[SERIALNO_LEN];
	unsigned short bank;
	unsigned short slot;
	unsigned char cmd;
	unsigned char result;
	char apn[10];
	char user[20];
	char pwd[20];
	char url[128];
	unsigned int presize;
}module_internet_info_t;

typedef struct _sms_event_s 
{
	unsigned short board_nbr;
	unsigned short slot_nbr;
	unsigned short state;
	unsigned char uuid_len;
	unsigned char phone_len;
	char uuid[MAX_UUID_LEN];
	char phone[MAX_PHONE_LEN];
}sms_event_t;

typedef struct _ussd_event_s
{
	unsigned short board_nbr;
	unsigned short slot_nbr;
	unsigned short state;
	unsigned char ussd_stat;
	unsigned char ussd_coding;
	unsigned short ussd_len;
	char ussd_info[1024];
	unsigned char uuid_len;
	char uuid[MAX_UUID_LEN];
}ussd_event_t;

typedef struct USSD_INFO_S
{	
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short bank_nbr;
	unsigned short slot_nbr;
	char msg[64];
	char uuid[24];
	unsigned int timeout;
}ussd_info_t;

typedef struct SMS_MODE_INFO_S
{	
	char seri[SERIALNO_LEN];		// 发起方序列号
	unsigned short chnl_nbr;
	unsigned char mode;
}sms_mode_info_t;


#pragma pack()






/**************************** 函数声明和定义 ****************************/

/**************************************************************************** 
* 函数名称 : makeReqPackage
* 功能描述 : 请求包打包函数
* 参    数 : unsigned short cmd			: 网络包类型
* 参    数 : char *seri					: SimEmuSvr序列号
* 参    数 : char *data					: 负载数据
* 参    数 : unsigned short data_len	: 负载数据长度
* 参    数 : char *buff					: 报文内容缓冲，用于输出
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeReqPackage(unsigned short cmd, char *data, unsigned short data_len, char *buff);

/**************************************************************************** 
* 函数名称 : makeRspPackage
* 功能描述 : 响应包打包函数
* 参    数 : char *buff_req				: 请求包缓冲，里面存储请求内容
* 参    数 : char *buff_rsp				: 响应包缓冲，用于输出
* 参    数 : char *data					: 负载数据
* 参    数 : unsigned short data_len	: 负载数据长度
* 参    数 : unsigned short result		: 报文成功标志
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeRspPackage(char *buff_req, char *buff_rsp, char *data, unsigned short data_len, unsigned short result);

int makeSimDataRspPackage(char *buff_rsp, char *data, unsigned short data_len, unsigned short result);
int makeSimPullPlugReqPackage(char *buff_rsp, char *data, unsigned short data_len, unsigned short result);


/**************************************************************************** 
* 函数名称 : parsePackage
* 功能描述 : 报文解析函数
* 参    数 : char *buff					: 报文缓冲
* 参    数 : package_t *msg				: 报文消息数据结构缓冲
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int parsePackage(char *buff, package_t *msg);

/**************************************************************************** 
* 函数名称 : checkPackage
* 功能描述 : 报文校验检查函数
* 参    数 : char *buff					: 报文缓冲
* 参    数 : int len					: 报文长度
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int checkPackage(char *buff, int len);

/**************************************************************************** 
* 函数名称 : parsePackHeader
* 功能描述 : 报文包头解析函数
* 参    数 : char *pack					: 报文缓冲
* 参    数 : package_header_t *header	: 包头数据结构缓冲
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int parsePackHeader(char *pack, package_header_t *header);

/**************************************************************************** 
* 函数名称 : getVersion
* 功能描述 : 获取报文版本函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文版本号
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getVersion(char *buff);

/**************************************************************************** 
* 函数名称 : getCmd
* 功能描述 : 获取报文类型函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文类型号
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getCmd(char *buff);

/**************************************************************************** 
* 函数名称 : getLen
* 功能描述 : 获取报文负载长度函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文负载长度
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getLen(char *buff);

/**************************************************************************** 
* 函数名称 : getResult
* 功能描述 : 获取报文结果码函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文结果码
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getResult(char *buff);

/**************************************************************************** 
* 函数名称 : getServerType
* 功能描述 : 获取报文请求发起服务器类型
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文请求发起服务器类型
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getServerType(char *buff);

/**************************************************************************** 
* 函数名称 : getSerialNo
* 功能描述 : 获取报文结果码函数
* 参    数 : char *buff					: 报文缓冲
* 参    数 : char *seri					: 报文Emu序列号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getSerialNo(char *buff, char *seri);

/**************************************************************************** 
* 函数名称 : getReserve
* 功能描述 : 获取报文预置函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文预置位
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getReserve(char *buff);

/**************************************************************************** 
* 函数名称 : checksum
* 功能描述 : 校验函数
* 参    数 : unsigned short *buffer		: 报文缓冲
* 参    数 : int size					: 报文长度
* 返 回 值 : 校验码1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short checksum(unsigned short *buffer, int size);

/**************************************************************************** 
* 函数名称 : makeRegisterPack
* 功能描述 : 注册包打包函数
* 参    数 : char *buff_req				: 注册请求包缓冲，用于输出
* 参    数 : unsigned short server_type	: 注册服务器类型
* 参    数 : char *seri					: 注册服务器序列号
* 参    数 : char *data					: 负载数据
* 参    数 : int data_len				: 负载数据长度
* 参    数 : int len					: 注册请求包缓冲长度，用于输出
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeRegisterPack(char *buff, char *data, int data_len, int *len);

/**************************************************************************** 
* 函数名称 : makeUnRegisterPack
* 功能描述 : 注销包打包函数
* 参    数 : char *buff_req				: 注销请求包缓冲，用于输出
* 参    数 : char *seri					: SimEmuSvr序列号
* 参    数 : char *data					: 负载数据
* 参    数 : int data_len				: 负载数据长度
* 参    数 : int len					: 注销请求包缓冲长度，用于输出
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeUnRegisterPack(char *buff, char *data, int data_len, int *len);

/**************************************************************************** 
* 函数名称 : makeHeartbeatPack
* 功能描述 : 心跳包打包函数
* 参    数 : char *buff_req				: 心跳请求包缓冲，用于输出
* 参    数 : char *seri					: SimEmuSvr序列号
* 参    数 : char *data					: 负载数据
* 参    数 : int data_len				: 负载数据长度
* 参    数 : int len					: 心跳请求包缓冲长度，用于输出
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeHeartbeatPack(char *buff, char *data, int data_len, int *len);

int makeReportPackage(char *buff, char *data, int data_len, int *len);

int makeLinkBreakNoticePack(char *buff, char *data, int data_len, int *len);

int makeSimPullPlugNoticePack(char *buff, char *data, int data_len, int *len);

int makeSimRegisterStatNoticePack(char *buff, char *data, int data_len, int *len);

int makeSMSRSPPackage(char *buff, char *data, int data_len, int *len);
int makeRcvSMSEvnetPackage(char *buff, char *data, int data_len, int *len);
int makeEventModuleLimitPackage(char * buff, char * data, int data_len, int * len);
int makeEventModuleInterPackage(char *buff, char *data, int data_len, int *len);

int recvNetPackage(CSocketEx *csock, char *buff, int *buff_len);

int printHex(unsigned char *buff, unsigned short buff_len);
int printSimDataToHex(unsigned char *buff, unsigned short buff_len, unsigned short usb_nbr, unsigned short bank_nbr, unsigned short sim_nbr);

int hex2char(unsigned char *buff_hex, unsigned short len_hex, char *buff_char, unsigned short *len_char);

char * ip_num_to_char(unsigned long num, char *ip);

#endif
