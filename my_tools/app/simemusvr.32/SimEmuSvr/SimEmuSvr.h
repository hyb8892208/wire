/****************************************************************************
* ��Ȩ��Ϣ��
* ϵͳ���ƣ�SimEmuSvr
* �ļ����ƣ�SimEmuSvr.h 
* �ļ�˵����SimEmuSvr���ܴ���ͷ�ļ�
* ��    �ߣ�hlzheng 
* �汾��Ϣ��v1.0 
* ������ڣ�
* �޸ļ�¼��
* ��    ��		��    ��		�޸��� 		�޸�ժҪ  
****************************************************************************/

/**************************** ��������ѡ���ͷ�ļ� ****************************/
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
#define CMD_IS_DATA    0    //����Ҫresp
//  <- pc
#define CMD_IS_SETATR  0x81 //��Ҫresp
#define CMD_IS_GETVER  0x83 //��Ҫresp
#define CMD_IS_REQRST  0x85 //��Ҫresp reset itself
//  -> PC
#define IS_REQ_RST_ICC    0x10 //����Ҫresp
#define IS_VER            0x82 //resp to pc
#define IS_RESP_RST       0x84 //resp to pc at reset

#define SOCAT_PORT		5501




// config file
#define SIMEMUSVR_CONFIG				"/etc/asterisk/simemusvr.conf"				// �����ļ�·��
#define KEY_SERI						"seri"							// SimEmuSvr���к�key
#define KEY_LOCAL_IP					"local_ip"
#define KEY_SERVER_IP					"server_ip"						// SimRdrSvr������IP��ַkey
#define KEY_SERVER_PORT					"server_port"					// SimRdrSvr������PORT�˿�key
#define KEY_PASSWD						"passwd"						// ע������
#define KEY_HB_INTERVAL					"heartbeat_interval"			// ����key
#define KEY_COMM						"comm"							// ���ں�key
#define KEY_BAUDRATE					"baudrate"						// ���ں�key
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
	NoErr = 0,		// �ɹ�
	Error =1		// ʧ��
};

enum
{
	SlotInvalid = 0,	// ������Ч
	SlotValid   = 1		// ������Ч
};


#define REDIS_KEY_SIMBANK_SMS_INFO		"simbank.collect.sim.info"
//��������״̬
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

// ���а忨�������ӣ�����ű�־��·
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
	unsigned short stop;					// ֹͣ��־��0�����У�1��ֹͣ
	unsigned short resetAtr;				// ����
	unsigned short board_nbr;				// board nbr
	unsigned int baudrate;
	char tty[16];							// tty path
	char ip_tty[16];
	char version[320];						// emu version
	int initialed;							// CommHdlTask�̳߳��º���ɱ�ʶ
	pthread_attr_t attr;
	struct sched_param thr_param;
}commhdltask_param_t;


typedef struct nethdl_params_s
{
	int stop;
	CSocketEx *csock;
	CSocketEx *sock_sht;
}nethdl_params_t;

// PORT���ݽṹ
typedef struct PORT_S
{
	unsigned short port;
}Port_t;

// Mini52�汾���ݽṹ
typedef struct MINI52_VERSION_S
{
	unsigned char version[320];
}mini52_version_t;

#pragma pack(1)

// ��Ϣͷ���ݽṹ�������̼߳�ͨѶ
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

// ��Ϣ���ݽṹ
typedef struct MSG_S
{
	msg_header_t header;		// ��Ϣͷ
	//char data[BUFF_SIZE];		// sim data
	char data[0xffff];		// sim data
}msg_t;

// �������ݽṹ
typedef struct SLOT_INFO_S
{
	unsigned short gw_bank_nbr;	// 
	unsigned short link_nbr;	// ��·���
	unsigned short board_nbr;	// �忨��
	unsigned short slot_nbr;	// ���ۺ�
	unsigned long svr_ip;		// ������IP��ַ
	//char svr_ip[16];
	unsigned short svr_port;	// ������PORT�˿�
	char sb_seri[12];
	unsigned short sb_usb_nbr;
	unsigned short sb_bank_nbr;	// simbank bank number
	unsigned short sim_nbr;		// sim����
	unsigned short stat;		// ״̬
	unsigned short link_stat;	// 0:δ����sim��; 1:�ѷ���sim��
}slot_info_t;


// SlotHdlTask�̲߳������ݽṹ
typedef struct SLOTHDLTASK_PARAM_S
{
	slot_info_t slotinfo;
	int sockfd_comm;						// ����CommHdlTask��sockfd
	int sockfd_nettask;						// ����NetHdlTask��sockfd
	int sockfd_netrdr;						// ����Rdr������sockfd
	unsigned short port_rdr;				// ���ӷ������Ķ˿�
	unsigned short port_comm;				// ����CommHdlTask�˿�
	unsigned short port_nettask;			// ����NetHdlTask�˿�
	unsigned short stop;					// ֹͣ��־��0�����У�1��ֹͣ
	unsigned short call_flag;				// ͨ����־��0:δͨ�� 1:ͨ����
	unsigned short module_type;
	short call_rest_time;					// ͨ��ʣ��ʱ��
	time_t call_begin_time;					// ͨ����ʼʱ��
	char local_ip[16];						// ���ص�ַ��127.0.0.1
	pthread_attr_t attr;
	struct sched_param thr_param;
	char sim_atr[320];
	unsigned short len_atr;
	int apdu_log_fd;
}slothdltask_param_t;

// sim��������Ϣ���ݽṹ
typedef struct SIMCARD_HANDLE_INFO_S
{
	unsigned short cmd;			// ��������
	char emu_seri[10];			// SimEmuSvr���к�
	slot_info_t slot_info;		// ����������Ϣ
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



// EmuԪ�����ݽṹ
typedef struct EMU_S
{
	register_req_info_t emu;	// ע��Emu
	struct EMU_S *prev;			// ǰָ��
	struct EMU_S *next;			// ��ָ��
}emu_t;

// ע���б����ݽṹ
typedef struct REGISTER_LIST_S
{
	unsigned short nbr;			// ע��Emu����
	pthread_mutex_t lock;		// ������
	emu_t *head;				// ע��Emu����ͷ
	emu_t *tail;				// ע��Emu����β
}register_list_t;


// Emuע��״̬���ݽṹ
typedef struct REGISTER_STAT_S
{
	int stat;				// ״̬��0:unregister; 1:registed
	pthread_mutex_t lock;	// ������
}register_stat_t;

// SimEmuSvr���������ò������ݽṹ�����ڱ���������ļ���ȡ�Ĳ���
typedef struct SIMEMUSVRPARAM_S
{
	char simemusvr_switch[12];
	char seri[12];					// SimEmuSvr���к�
	char passwd[64];				// ע������
	char local_ip[16];				// ����IP��ַ
	char server_ip[16];				// SimRdrSvr������IP��ַ
	char comm[32];					// ���ںţ�����Emu�忨
	unsigned int baudrate;			// ����ʱ��������λ����
	unsigned short server_port;		// SimRdrSvr������PORT�˿�
	unsigned short hb_interval;		// ����ʱ��������λ����
	unsigned short slot_port_rdr;	// 
	unsigned short slot_port_comm;	// 
	unsigned short slot_port_net;	// 
	unsigned short net_hdl_port;	// ָ��˿�
	unsigned short sim_data_out_port;	// sim���ݶ���˿�
	unsigned short sim_data_in_port;	// sim���ݶ��ڶ˿�
	unsigned short comm_hdl_port;	// 
	unsigned short ast_hdl_port;	// ����asterisk���͹����Ŀ�ʼ���кͽ���������Ϣ
}simemusvrparam_t;


typedef struct transhdltask_param_s
{
	int stop;
}transhdltask_param_t;


typedef int (*NET_HANDLE_FUNC)(char *buff_req, char *buff_rsp, CSocketEx *csock);

// ���紦����Ϣ���ݽṹ
typedef struct NET_HANDLE_S
{
	unsigned short cmd;				// ��Ϣ����
	NET_HANDLE_FUNC cmd_handle;		// ������
}net_handle_t;



typedef int (*AST_HANDLE_FUNC)(char *buff_req, char *buff_rsp, CSocketEx *csock);

// ���紦����Ϣ���ݽṹ
typedef struct AST_HANDLE_S
{
	unsigned short cmd;				// ��Ϣ����
	AST_HANDLE_FUNC cmd_handle;		// ������
}ast_handle_t;

typedef struct ttyUSB_Emu_s
{
	char dev[16]; // ����emu�����豸�����磺/dev/ttyUSB1
	int baud;
}ttyUSB_Emu_t;
typedef struct ttyUSB_Gsm_s
{
	char dev[16]; // ����GSMģ���豸�����磺/dev/ttyUSB2
	int chn;      // ͨ����
}ttyUSB_Gsm_t;
typedef struct ttyUSBx_grp_s
{
	//char ttyUSB_Emu[16]; // ����emu�����豸�����磺/dev/ttyUSB1
	ttyUSB_Emu_t ttyUSB_Emu;
	//char ttyUSB_Gsm[SLOT_NBR][16]; // ����GSMģ���豸�����磺/dev/ttyUSB2
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


/**************************** ���������Ͷ��� ****************************/


/**************************************************************************** 
* �������� : readConfigValue
* �������� : ��ȡ�����ļ���������
* ��    �� : void
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int readConfigValue(void);


/**************************************************************************** 
* �������� : HeartbeatRspHdl
* �������� : ������Ӧ����
* ��    �� : char *buff_recv			: ���ջ��壬�洢���ձ�������
* ��    �� : char *buff_send			: ���ͻ���
* ��    �� : CSocketEx *csock			: ͨѶsocket��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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
* �������� : SimSMSReqHdl
* �������� : ���ŷ���������
* ��    �� : char *buff_recv			: ���ջ��壬�洢���ձ�������
* ��    �� : char *buff_send			: ���ͻ���
* ��    �� : CSocketEx *csock			: ͨѶsocket��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : lyz 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int SimSMSReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

/**************************************************************************** 
* �������� : EmuSimRspHdl
* �������� : sim��������Ӧ����
* ��    �� : char *buff_recv			: ���ջ��壬�洢���ձ�������
* ��    �� : char *buff_send			: ���ͻ���
* ��    �� : CSocketEx *csock			: ͨѶsocket��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int EmuSimRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

/**************************************************************************** 
* �������� : EmuSimReleaseRspHdl
* �������� : sim���ͷ���Ӧ���� Ŀǰ�޸�ΪSlotHdlTask�Ͽ���SimHdlTask�����Ӽ��ɣ���SimHdlTask����Sim�ͷ�
* ��    �� : char *buff_recv			: ���ջ��壬�洢���ձ�������
* ��    �� : char *buff_send			: ���ͻ���
* ��    �� : CSocketEx *csock			: ͨѶsocket��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int EmuSimReleaseRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock);

/**************************************************************************** 
* �������� : UnRegisterRspHdl
* �������� : ע����Ӧ����
* ��    �� : char *buff_recv			: ���ջ��壬�洢���ձ�������
* ��    �� : char *buff_send			: ���ͻ���
* ��    �� : CSocketEx *csock			: ͨѶsocket��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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
* �������� : findNetHandleFunc
* �������� : ��ȡָ���������͵Ĵ�����
* ��    �� : unsigned short cmd			: ��������
* �� �� ֵ : �ɹ����غ���ָ�룬ʧ�ܷ���NULL
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
NET_HANDLE_FUNC findNetHandleFunc(unsigned short cmd);

/**************************************************************************** 
* �������� : initRegisterStat
* �������� : ��ʼ��ע��״̬
* ��    �� : register_stat_t *stat		: ע��״̬���ݽṹ����
* �� �� ֵ : void
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void initRegisterStat(register_stat_t *stat);

/**************************************************************************** 
* �������� : upRegisterStat
* �������� : ����ע��״̬
* ��    �� : register_stat_t *stat		: ע��״̬���ݽṹ����
* �� �� ֵ : void
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void upRegisterStat(register_stat_t *regstat);

/**************************************************************************** 
* �������� : downRegisterStat
* �������� : ����ע��״̬
* ��    �� : register_stat_t *stat		: ע��״̬���ݽṹ����
* �� �� ֵ : void
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void downRegisterStat(register_stat_t *stat);

/**************************************************************************** 
* �������� : getRegisterStat
* �������� : ��ȡע��״̬
* ��    �� : register_stat_t *stat		: ע��״̬���ݽṹ����
* �� �� ֵ : ע��״̬
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int getRegisterStat(register_stat_t *stat);

/**************************************************************************** 
* �������� : makeSimCardReqInterPack
* �������� : sim�������ڲ��������������
* ��    �� : simcard_handle_info_t *simhdl	: sim���������ݽṹ����
* ��    �� : unsigned short slot_nbr		: slot���ۺ�
* �� �� ֵ : 0
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeSimCardReqInterPack(simcard_handle_info_t *simhdl, unsigned short slot_nbr);

/**************************************************************************** 
* �������� : makeSimCardReleaseInterPack
* �������� : sim���ͷ��ڲ��������������
* ��    �� : simcard_handle_info_t *simhdl	: sim���������ݽṹ����
* ��    �� : unsigned short slot_nbr		: slot���ۺ�
* �� �� ֵ : 0
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeSimCardReleaseInterPack(simcard_handle_info_t *simhdl, unsigned short slot_nbr);

/**************************************************************************** 
* �������� : makeSimCardPackage
* �������� : sim�������ڲ��������������
* ��    �� : simcard_handle_info_t *simhdl	: sim���������ݽṹ����
* ��    �� : char *buff						: �������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeSimCardPackage(simcard_handle_info_t *simhdl, char *buff);

/**************************************************************************** 
* �������� : emuRegister
* �������� : Emuע�ắ��
* ��    �� : CSocketEx *sock			: ͨ��socket��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int emuRegister(CSocketEx *sock);

/**************************************************************************** 
* �������� : emuUnRegister
* �������� : Emuע������
* ��    �� : CSocketEx *sock			: ͨ��socket��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int emuUnRegister(CSocketEx *sock);

/**************************************************************************** 
* �������� : EmuHeartbeat
* �������� : Emu��������
* ��    �� : CSocketEx *sock			: ͨ��socket��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int EmuHeartbeat(CSocketEx *sock);

/**************************************************************************** 
* �������� : resetMini52
* �������� : ����ָ����Mini52оƬ
* ��    �� : int handle					: ���ھ��
* ��    �� : unsigned short board			: �忨��
* ��    �� : unsigned short slot			: slot���ۺ�
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int resetSTM32(int handle, unsigned short board);

int getSTM32Version(int handle, unsigned short board,char *version,int version_len);

//int resetMini52(int handle, unsigned short board, unsigned short slot);
int resetMini52(CSocketEx *csock, unsigned short board, unsigned short slot);

/**************************************************************************** 
* �������� : resetAllMini52
* �������� : ��������Mini52оƬ
* ��    �� : void
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int resetAllMini52(int handle, unsigned short board);
//int resetAllMini52(CSocketEx *csock, unsigned short board);

/**************************************************************************** 
* �������� : getMini52Version
* �������� : ��ȡָ����Mini52оƬ�İ汾��
* ��    �� : unsigned short slot			: slot���ۺ�
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
//int getMini52Version(unsigned short slot);
int getMini52Version(CSocketEx *csock, unsigned short usb_nbr, unsigned short slot);

/**************************************************************************** 
* �������� : getAllMini52Version
* �������� : ��ȡ���е�Mini52оƬ�İ汾��
* ��    �� : void
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
//int getAllMini52Version(void);
int getAllMini52Version(int handle, unsigned short usb_nbr,char *version);

/**************************************************************************** 
* �������� : SlotHdlTask
* �������� : slot���۴����̺߳���
* ��    �� : void *pParam					: �̲߳���
* �� �� ֵ : void *
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void * SlotHdlTask(void *pParam);

/**************************************************************************** 
* �������� : NetHdlTask
* �������� : ���紦���̺߳���������SlotHdlTask��SimRdrSvr
* ��    �� : void *pParam					: �̲߳���
* �� �� ֵ : void *
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void * NetHdlTask(void *pParam);
/**************************************************************************** 
* �������� : getCount
* �������� : ��ȡ����
* ��    �� : void
* �� �� ֵ : �ۼƼ���
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned long getCount(void);

/**************************************************************************** 
* �������� : makeEmuDataPackage
* �������� : Emu͸�����ݴ������
* ��    �� : msg_t *msg						: ͸����Ϣ���ݽṹ����
* ��    �� : char *buff						: �������ݻ���
* ��    �� : int len						: �������ݻ��峤��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeEmuDataPackage(msg_t *msg, char *buff, int len);

/**************************************************************************** 
* �������� : CommHdlTask
* �������� : ���ڴ����̺߳���
* ��    �� : void *pParam					: �����̺߳�������
* �� �� ֵ : void *
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void *CommHdlTask(void *pParam);

int socket_pty_wr(CSocketEx *csock, unsigned char *buff_w, int len_w, unsigned char *buff_r, int *len_r, int timeout);


#endif
