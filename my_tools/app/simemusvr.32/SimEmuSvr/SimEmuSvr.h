/****************************************************************************
* 版权信息：
* 系统名称：SimEmuSvr
* 文件名称：SimEmuSvr.h 
* 文件说明：SimEmuSvr功能处理头文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

/**************************** 条件编译选项和头文件 ****************************/
#ifndef __SIMEMUSVR_H__
#define __SIMEMUSVR_H__


#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <pthread.h>
#include <string.h>

#include "msg.h"
#include "CSocketEx.h"
//#include "SlotList.h"

//#include "include/ctb-0.16/ctb.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))


#define TIMEOUT_USEC 1000000

#define EMU_BANK_NBR 5


#define SLOT_IS_EMU_CTRL 0x80 //STM32 self;
#define SLOT_IS_RDR_CTRL 0x80 //STM32 self;

//  <->pc
#define CMD_IS_DATA    0    //不需要resp
//  <- pc
#define CMD_IS_SETATR  0x81 //需要resp
#define CMD_IS_GETVER  0x83 //需要resp
#define CMD_IS_REQRST  0x85 //需要resp reset itself
//  -> PC
#define IS_REQ_RST_ICC    0x10 //不需要resp
#define IS_VER            0x82 //resp to pc
#define IS_RESP_RST       0x84 //resp to pc at reset

#define SOCAT_PORT		5501




// config file
#define SIMEMUSVR_CONFIG				"/etc/asterisk/simemusvr.conf"				// 配置文件路径
#define KEY_SERI						"seri"							// SimEmuSvr序列号key
#define KEY_LOCAL_IP					"local_ip"
#define KEY_SERVER_IP					"server_ip"						// SimRdrSvr服务器IP地址key
#define KEY_SERVER_PORT					"server_port"					// SimRdrSvr服务器PORT端口key
#define KEY_PASSWD						"passwd"						// 注册密码
#define KEY_HB_INTERVAL					"heartbeat_interval"			// 心跳key
#define KEY_COMM						"comm"							// 串口号key
#define KEY_BAUDRATE					"baudrate"						// 串口号key
#define KEY_SLOT_PORT_RDR				"slot_port_rdr"					// 
#define KEY_SLOT_PORT_COMM				"slot_port_comm"				// 
#define KEY_SLOT_PORT_NET				"slot_port_net"					// 
#define KEY_NET_HDL_PORT				"net_hdl_port"					// 
#define KEY_SIM_DATA_OUT_PORT			"sim_data_out_port"				//
#define KEY_SIM_DATA_IN_PORT			"sim_data_in_port"					//  
#define KEY_COMM_HDL_PORT				"comm_hdl_port"					// 
#define KEY_AST_HDL_PORT				"ast_hdl_port"
#define KEY_SIMEMUSVR_SWITCH			"simemusvr_switch"

#define SERVER_PORT						6201
#define HB_INTERVAL						120
#define SLOT_PORT_COMM					4001
#define SLOT_PORT_NET					5001
#define COMM_HDL_PORT					2305
#define NET_HDL_PORT					2303
#define AST_HDL_PORT					2304

#define AST_RUN_STAT_FILE				"/var/run/asterisk.pid"
#define LOGFILE_CONFIG					"/etc/asterisk/gw/logfile_monitor.conf"
#define KEY_LOG_CLASS					"emu_log_class"
#define KEY_APDU_LOG_SWITCH				"emu_usb_data_logs"

#define LOCAL_HOST 						"127.0.0.1"

#define LOCK_FILE						"/tmp/lock/SimEmuSvr.lock"

#define REDIS_GSMSTATUS					"app.asterisk.gsmstatus.channel"
#define REDIS_SIMSTATUS					"app.asterisk.simstatus.channel"

#define SWG_2016						"SWG-2016"
#define SWG_2032						"SWG-2032"
#define AST_CLI                         "asterisk -rx"
#define GSM_SET_CALLLIMIT				"gsm set calllimit"
#define GSM_SET_SMSLIMIT				"gsm set smslimit"
#define GSM_SET_INTERLIMIT              "gsm set interlimit"
#define SIMDATA_FILE					"/my_tools/simdata_cli"
#define GET_SIMDATA_CLI					"/my_tools/simdata_cli get simdata"
#define SIMDATA_DIR						"/tmp/simcards/"
#define SIMDATA_EXTENSION               ".dat"
#define INTER_DIALUP_CLI				"/my_tools/emu_module_connect_net.sh"
#define INTER_SURF_CLI                  "/my_tools/emu_module_surf_internet.sh"
#define INTER_END_CLI					"/my_tools/emu_module_disconnect_net.sh"

enum
{
	NoErr = 0,		// 成功
	Error =1		// 失败
};

enum
{
	SlotInvalid = 0,	// 卡槽无效
	SlotValid   = 1		// 卡槽有效
};


#define REDIS_KEY_SIMBANK_SMS_INFO		"simbank.collect.sim.info"
//短信请求状态
enum
{
	SMS_IDLE		=0,
	SMS_NEED_SEND	=1,
	SMS_SENT		=2,
	SMS_RECEIVED	=3
};

enum
{
	GSM_MODULE = 1,
	CDMA_MODULE = 2,
	LTE_MODULE = 3,
	OTHER_MODULE = 4
};

enum 
{
	SMS_QUERY_PHONENUM = 1,
	SMS_QUERY_BALANCE = 2,
	SMS_QUERY_OTHER = 3
};

enum{
	MODULE_LIMITCMD_LIMIT = 1,
	MODULE_LIMITCMD_UNLIMIT,
	MODULE_LIMITCMD_CALLLIMIT,
	MODULE_LIMITCMD_CALLUNLIMIT,
	MODULE_LIMITCMD_SMSLIMIT,
	MODULE_LIMITCMD_SMSUNLIMIT,
	MODULE_LIMITCMD_INTERLIMIT,
	MODULE_LIMITCMD_INTERUNLIMIT
};

enum{
	MODULE_UNLIMIT = 0,
	MODULE_LIMIT   = 1
};

enum{
	INTER_DIALUP  = 1,
	INTER_SURF    = 2,
	INTER_END     = 3
};

enum{
	INTER_SURF_MUST  = 1,
	INTER_SURF_NONE  = 2,
	INTER_SURF_SIZE  = 3
};
#define BUFF_10M	(1024 * 1000 * 10)
#define JOSN_TYPE_RECVSMS 	"eventRecvSMS"
#define JOSN_TYPE_INTER  	"eventInternetResult"
#define JOSN_TYPE_LIMITSTAT "eventModuleLimit"
#define JOSN_TYPE_USSDRES 	"eventUssdResult"
#define JOSN_TYPE_SMS		"eventSendSMS"

#define INTER_LOG_PATH "/tmp"
#define DIAL_NAME   "_dialup.log"
#define INTER_NAME   "_surf_internet.log"
#define INTER_ERROR  "error"

enum{
	INTER_DIALUP_DISCONN = 0,
	INTER_DIALUP_CONNECTING,
	INTER_DIALUP_CONN,
};

//internet cmd

//internet surf cmd
#define INTER_SURF_STR_MUST		"must"
#define INTER_SURF_STR_NONE		"none"
#define INTER_SURF_STR_SIZE		"size"

//internet stat
#define INTER_STAT_DISCONN		"inter_disconn"
#define INTER_STAT_CONN			"inter_conn"
#define INTER_STAT_SURFING 		"inter_surfing"
typedef struct slave_info_s
{
	int nbr;
	char ip[MAX_BOARD-1][16];
	//char ip_ori[MAX_BOARD-1][16];
	char tty[MAX_BOARD-1][64];
}slave_info_t;

// 所有板卡不分主从，以序号标志线路
typedef struct tty_info_s
{
	int nbr;
	char ip[MAX_BOARD][16];
	char tty[MAX_BOARD][16];
}tty_info_t;


typedef struct COMMHDLTASK_PARAM_S
{
	int handle;								// tty handle
	pthread_t tid;							// pthread id
	char ip[16];							// udp ip
	unsigned short port;					// udp port
	unsigned short stop;					// 停止标志，0：运行；1：停止
	unsigned short resetAtr;				// 重置
	unsigned short board_nbr;				// board nbr
	unsigned int baudrate;
	char tty[16];							// tty path
	char ip_tty[16];
	char version[320];						// emu version
	int initialed;							// CommHdlTask线程出事后完成标识
	pthread_attr_t attr;
	struct sched_param thr_param;
}commhdltask_param_t;


typedef struct nethdl_params_s
{
	int stop;
	CSocketEx *csock;
	CSocketEx *sock_sht;
}nethdl_params_t;

// PORT数据结构
typedef struct PORT_S
{
	unsigned short port;
}Port_t;

// Mini52版本数据结构
typedef struct MINI52_VERSION_S
{
	unsigned char version[320];
}mini52_version_t;

#pragma pack(1)

// 消息头数据结构，用于线程间通讯
typedef struct MSG_HEADER_S
{
	unsigned short cmd;			// 1: sim data; 2:sim release
	unsigned short bank;		// bank nunmber, emu is reserve
	unsigned short slot;		// slot number
	unsigned short data_len;		// data len
	//unsigned int data_len;		// data len
	//unsigned long cnt;			// count
	unsigned int cnt;
}msg_header_t;

// 消息数据结构
typedef struct MSG_S
{
	msg_header_t header;		// 消息头
	//char data[BUFF_SIZE];		// sim data
	char data[0xffff];		// sim data
}msg_t;

// 卡槽数据结构
typedef struct SLOT_INFO_S
{
	unsigned short gw_bank_nbr;	// 
	unsigned short link_nbr;	// 线路编号
	unsigned short board_nbr;	// 板卡号
	unsigned short slot_nbr;	// 卡槽号
	unsigned long svr_ip;		// 服务器IP地址
	//char svr_ip[16];
	unsigned short svr_port;	// 服务器PORT端口
	char sb_seri[12];
	unsigned short sb_usb_nbr;
	unsigned short sb_bank_nbr;	// simbank bank number
	unsigned short sim_nbr;		// sim卡号
	unsigned short stat;		// 状态
	unsigned short link_stat;	// 0:未分配sim卡; 1:已分配sim卡
}slot_info_t;


// SlotHdlTask线程参数数据结构
typedef struct SLOTHDLTASK_PARAM_S
{
	slot_info_t slotinfo;
	int sockfd_comm;						// 连接CommHdlTask的sockfd
	int sockfd_nettask;						// 连接NetHdlTask的sockfd
	int sockfd_netrdr;						// 连接Rdr的网络sockfd
	unsigned short port_rdr;				// 连接服务器的端口
	unsigned short port_comm;				// 连接CommHdlTask端口
	unsigned short port_nettask;			// 连接NetHdlTask端口
	unsigned short stop;					// 停止标志，0：运行；1：停止
	unsigned short call_flag;				// 通话标志，0:未通话 1:通话中
	unsigned short module_type;
	short call_rest_time;					// 通话剩余时间
	time_t call_begin_time;					// 通话开始时间
	char local_ip[16];						// 本地地址，127.0.0.1
	pthread_attr_t attr;
	struct sched_param thr_param;
	char sim_atr[320];
	unsigned short len_atr;
	int apdu_log_fd;
}slothdltask_param_t;

// sim卡处理信息数据结构
typedef struct SIMCARD_HANDLE_INFO_S
{
	unsigned short cmd;			// 处理命令
	char emu_seri[10];			// SimEmuSvr序列号
	slot_info_t slot_info;		// 卡槽数据信息
}simcard_handle_info_t;

typedef struct link_relation_s
{
	char sb_seri[SERIALNO_LEN];
	unsigned short sb_bank_nbr;
	unsigned short sb_usb_nbr;
	unsigned short sb_slot_nbr;
	char gw_seri[SERIALNO_LEN];
	unsigned short gw_bank_nbr;
	unsigned short gw_slot_nbr;
	
}link_relation_t;
#pragma pack()



// Emu元素数据结构
typedef struct EMU_S
{
	register_req_info_t emu;	// 注册Emu
	struct EMU_S *prev;			// 前指针
	struct EMU_S *next;			// 后指针
}emu_t;

// 注册列表数据结构
typedef struct REGISTER_LIST_S
{
	unsigned short nbr;			// 注册Emu数量
	pthread_mutex_t lock;		// 互斥锁
	emu_t *head;				// 注册Emu链表头
	emu_t *tail;				// 注册Emu链表尾
}register_list_t;


// Emu注册状态数据结构
typedef struct REGISTER_STAT_S
{
	int stat;				// 状态，0:unregister; 1:registed
	pthread_mutex_t lock;	// 互斥锁
}register_stat_t;

// SimEmuSvr服务器配置参数数据结构，用于保存从配置文件读取的参数
typedef struct SIMEMUSVRPARAM_S
{
	char simemusvr_switch[12];
	char seri[12];					// SimEmuSvr序列号
	char passwd[64];				// 注册密码
	char local_ip[16];				// 本机IP地址
	char server_ip[16];				// SimRdrSvr服务器IP地址
	char comm[32];					// 串口号，连接Emu板卡
	unsigned int baudrate;			// 心跳时间间隔，单位：秒
	unsigned short server_port;		// SimRdrSvr服务器PORT端口
	unsigned short hb_interval;		// 心跳时间间隔，单位：秒
	unsigned short slot_port_rdr;	// 
	unsigned short slot_port_comm;	// 
	unsigned short slot_port_net;	// 
	unsigned short net_hdl_port;	// 指令端口
	unsigned short sim_data_out_port;	// sim数据对外端口
	unsigned short sim_data_in_port;	// sim数据对内端口
	unsigned short comm_hdl_port;	// 
	unsigned short ast_hdl_port;	// 接收asterisk发送过来的开始呼叫和结束呼叫信息
}simemusvrparam_t;


typedef struct transhdltask_param_s
{
	int stop;
}transhdltask_param_t;


typedef int (*NET_HANDLE_FUNC)(char *buff_req, char *buff_rsp, CSocketEx *csock);

// 网络处理信息数据结构
typedef struct NET_HANDLE_S
{
	unsigned short cmd;				// 消息命令
	NET_HANDLE_FUNC cmd_handle;		// 处理函数
}net_handle_t;



typedef int (*AST_HANDLE_FUNC)(char *buff_req, char *buff_rsp, CSocketEx *csock);

// 网络处理信息数据结构
typedef struct AST_HANDLE_S
{
	unsigned short cmd;				// 消息命令
	AST_HANDLE_FUNC cmd_handle;		// 处理函数
}ast_handle_t;

typedef struct ttyUSB_Emu_s
{
	char dev[16]; // 保存emu串口设备名，如：/dev/ttyUSB1
	int baud;
}ttyUSB_Emu_t;
typedef struct ttyUSB_Gsm_s
{
	char dev[16]; // 保存GSM模块设备名，如：/dev/ttyUSB2
	int chn;      // 通道号
}ttyUSB_Gsm_t;
typedef struct ttyUSBx_grp_s
{
	//char ttyUSB_Emu[16]; // 保存emu串口设备名，如：/dev/ttyUSB1
	ttyUSB_Emu_t ttyUSB_Emu;
	//char ttyUSB_Gsm[SLOT_NBR][16]; // 保存GSM模块设备名，如：/dev/ttyUSB2
	ttyUSB_Gsm_t ttyUSB_Gsm[SLOT_NBR];
}ttyUSBx_grp_t;

typedef struct ttyUSBx_s
{
	ttyUSBx_grp_t grp[MAX_BOARD];
}ttyUSBx_t;

struct module_name_s
{
	char module_name[20];
	unsigned short module_type;
};

#define REDIS_HOST	"127.0.0.1"
#define REDIS_PORT	6379

#define HWINFO_FILE "/tmp/hw_info.cfg"
#define EMU_INFO_FILE HWINFO_FILE
#define GSM_INFO_FILE HWINFO_FILE
#define SIMEMUSVR_CONFIG_FILE "/etc/asterisk/simemusvr.conf"

typedef struct port_map_s
{
	int port;
	int dev;
}port_map_t;
typedef struct port_map_info_s
{
	int nbr;
	port_map_t port_map[MAX_CHN];
}port_map_info_t;


/**************************** 函数声明和定义 ****************************/


/**************************************************************************** 
* 函数名称 : readConfigValue
* 功能描述 : 读取配置文件参数内容
* 参    数 : void
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int readConfigValue(void);


/**************************************************************************** 
* 函数名称 : HeartbeatRspHdl
* 功能描述 : 心跳响应处理
* 参    数 : char *buff_recv			: 接收缓冲，存储接收报文内容
* 参    数 : char *buff_send			: 发送缓冲
* 参    数 : CSocketEx *csock			: 通讯socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int HeartbeatRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);


int SimLinkReportRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SimDataRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SimPullPlugNoticeHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

int SIMHangUpReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int ModuleLimitReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int ModuleInterReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SendUssdReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int ModuleSetSmsModeHdl(char *buff_recv, char *buff_send, CSocketEx *csock);


/**************************************************************************** 
* 函数名称 : SimSMSReqHdl
* 功能描述 : 短信发送请求处理
* 参    数 : char *buff_recv			: 接收缓冲，存储接收报文内容
* 参    数 : char *buff_send			: 发送缓冲
* 参    数 : CSocketEx *csock			: 通讯socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : lyz 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int SimSMSReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

/**************************************************************************** 
* 函数名称 : EmuSimRspHdl
* 功能描述 : sim卡申请响应处理
* 参    数 : char *buff_recv			: 接收缓冲，存储接收报文内容
* 参    数 : char *buff_send			: 发送缓冲
* 参    数 : CSocketEx *csock			: 通讯socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int EmuSimRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

/**************************************************************************** 
* 函数名称 : EmuSimReleaseRspHdl
* 功能描述 : sim卡释放响应处理， 目前修改为SlotHdlTask断开与SimHdlTask的连接即可，由SimHdlTask处理Sim释放
* 参    数 : char *buff_recv			: 接收缓冲，存储接收报文内容
* 参    数 : char *buff_send			: 发送缓冲
* 参    数 : CSocketEx *csock			: 通讯socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int EmuSimReleaseRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

/**************************************************************************** 
* 函数名称 : UnRegisterRspHdl
* 功能描述 : 注销响应处理
* 参    数 : char *buff_recv			: 接收缓冲，存储接收报文内容
* 参    数 : char *buff_send			: 发送缓冲
* 参    数 : CSocketEx *csock			: 通讯socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int UnRegisterRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

int LinkInfoReportRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SimLinkCreateReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SimLinkReleaseReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

int CallMoringHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int CallBeginHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int CallEndHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SmsResultHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int SmsReportHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int UssdResultHdl(char *buff_recv, char *buff_send, CSocketEx *csock);
int ast_login(struct mansession *session, char * username, char *secret, char *host, int port);
int ast_logout(struct mansession *session);
int gsm_power_reset(struct mansession *session, unsigned short slot_nbr);
int gsm_power_on(struct mansession *session, unsigned short slot_nbr);
int gsm_power_off(struct mansession *session, unsigned short slot_nbr);

/**************************************************************************** 
* 函数名称 : findNetHandleFunc
* 功能描述 : 获取指定报文类型的处理函数
* 参    数 : unsigned short cmd			: 报文命令
* 返 回 值 : 成功返回函数指针，失败返回NULL
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
NET_HANDLE_FUNC findNetHandleFunc(unsigned short cmd);

/**************************************************************************** 
* 函数名称 : initRegisterStat
* 功能描述 : 初始化注册状态
* 参    数 : register_stat_t *stat		: 注册状态数据结构缓冲
* 返 回 值 : void
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void initRegisterStat(register_stat_t *stat);

/**************************************************************************** 
* 函数名称 : upRegisterStat
* 功能描述 : 拉起注册状态
* 参    数 : register_stat_t *stat		: 注册状态数据结构缓冲
* 返 回 值 : void
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void upRegisterStat(register_stat_t *regstat);

/**************************************************************************** 
* 函数名称 : downRegisterStat
* 功能描述 : 拉低注册状态
* 参    数 : register_stat_t *stat		: 注册状态数据结构缓冲
* 返 回 值 : void
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void downRegisterStat(register_stat_t *stat);

/**************************************************************************** 
* 函数名称 : getRegisterStat
* 功能描述 : 获取注册状态
* 参    数 : register_stat_t *stat		: 注册状态数据结构缓冲
* 返 回 值 : 注册状态
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getRegisterStat(register_stat_t *stat);

/**************************************************************************** 
* 函数名称 : makeSimCardReqInterPack
* 功能描述 : sim卡申请内部交互包打包函数
* 参    数 : simcard_handle_info_t *simhdl	: sim卡处理数据结构缓冲
* 参    数 : unsigned short slot_nbr		: slot卡槽号
* 返 回 值 : 0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeSimCardReqInterPack(simcard_handle_info_t *simhdl, unsigned short slot_nbr);

/**************************************************************************** 
* 函数名称 : makeSimCardReleaseInterPack
* 功能描述 : sim卡释放内部交互包打包函数
* 参    数 : simcard_handle_info_t *simhdl	: sim卡处理数据结构缓冲
* 参    数 : unsigned short slot_nbr		: slot卡槽号
* 返 回 值 : 0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeSimCardReleaseInterPack(simcard_handle_info_t *simhdl, unsigned short slot_nbr);

/**************************************************************************** 
* 函数名称 : makeSimCardPackage
* 功能描述 : sim卡处理内部交互包打包函数
* 参    数 : simcard_handle_info_t *simhdl	: sim卡处理数据结构缓冲
* 参    数 : char *buff						: 输出缓冲
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeSimCardPackage(simcard_handle_info_t *simhdl, char *buff);

/**************************************************************************** 
* 函数名称 : emuRegister
* 功能描述 : Emu注册函数
* 参    数 : CSocketEx *sock			: 通许socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int emuRegister(CSocketEx *sock);

/**************************************************************************** 
* 函数名称 : emuUnRegister
* 功能描述 : Emu注销函数
* 参    数 : CSocketEx *sock			: 通许socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int emuUnRegister(CSocketEx *sock);

/**************************************************************************** 
* 函数名称 : EmuHeartbeat
* 功能描述 : Emu心跳函数
* 参    数 : CSocketEx *sock			: 通许socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int EmuHeartbeat(CSocketEx *sock);

/**************************************************************************** 
* 函数名称 : resetMini52
* 功能描述 : 重置指定的Mini52芯片
* 参    数 : int handle					: 串口句柄
* 参    数 : unsigned short board			: 板卡号
* 参    数 : unsigned short slot			: slot卡槽号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int resetSTM32(int handle, unsigned short board);

int getSTM32Version(int handle, unsigned short board,char *version,int version_len);

//int resetMini52(int handle, unsigned short board, unsigned short slot);
int resetMini52(CSocketEx *csock, unsigned short board, unsigned short slot);

/**************************************************************************** 
* 函数名称 : resetAllMini52
* 功能描述 : 重置所有Mini52芯片
* 参    数 : void
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int resetAllMini52(int handle, unsigned short board);
//int resetAllMini52(CSocketEx *csock, unsigned short board);

/**************************************************************************** 
* 函数名称 : getMini52Version
* 功能描述 : 获取指定的Mini52芯片的版本号
* 参    数 : unsigned short slot			: slot卡槽号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
//int getMini52Version(unsigned short slot);
int getMini52Version(CSocketEx *csock, unsigned short usb_nbr, unsigned short slot);

/**************************************************************************** 
* 函数名称 : getAllMini52Version
* 功能描述 : 获取所有的Mini52芯片的版本号
* 参    数 : void
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
//int getAllMini52Version(void);
int getAllMini52Version(int handle, unsigned short usb_nbr,char *version);

/**************************************************************************** 
* 函数名称 : SlotHdlTask
* 功能描述 : slot卡槽处理线程函数
* 参    数 : void *pParam					: 线程参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void * SlotHdlTask(void *pParam);

/**************************************************************************** 
* 函数名称 : NetHdlTask
* 功能描述 : 网络处理线程函数，连接SlotHdlTask和SimRdrSvr
* 参    数 : void *pParam					: 线程参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void * NetHdlTask(void *pParam);
/**************************************************************************** 
* 函数名称 : getCount
* 功能描述 : 获取计数
* 参    数 : void
* 返 回 值 : 累计计数
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned long getCount(void);

/**************************************************************************** 
* 函数名称 : makeEmuDataPackage
* 功能描述 : Emu透传数据打包函数
* 参    数 : msg_t *msg						: 透传信息数据结构缓冲
* 参    数 : char *buff						: 负载数据缓冲
* 参    数 : int len						: 负载数据缓冲长度
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeEmuDataPackage(msg_t *msg, char *buff, int len);

/**************************************************************************** 
* 函数名称 : CommHdlTask
* 功能描述 : 串口处理线程函数
* 参    数 : void *pParam					: 串口线程函数参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void *CommHdlTask(void *pParam);

int socket_pty_wr(CSocketEx *csock, unsigned char *buff_w, int len_w, unsigned char *buff_r, int *len_r, int timeout);


#endif
