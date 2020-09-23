/****************************************************************************
* ��Ȩ��Ϣ��
* ϵͳ���ƣ�SimServer
* �ļ����ƣ�msg.h 
* �ļ�˵����������Ϣͷ�ļ�
* ��    �ߣ�hlzheng 
* �汾��Ϣ��v1.0 
* ������ڣ�
* �޸ļ�¼��
* ��    ��		��    ��		�޸��� 		�޸�ժҪ  
****************************************************************************/

/**************************** ��������ѡ���ͷ�ļ� ****************************/
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



#define VERSION					0x0002					// ���İ汾��

#define REGISTER_REQ			0x0001					// ע�������
#define REGISTER_RSP			0x1001					// ע����Ӧ��
#define UNREGISTER_REQ			0x0002					// ע�������
#define UNREGISTER_RSP			0x1002					// ע����Ӧ��
#define HEARTBEAT_REQ			0x0003					// ���������
#define HEARTBEAT_RSP			0x1003					// ������Ӧ��
#define SIMDATASYNC_REQ			0x0004					// sim����ͬ�������
#define SIMDATASYNC_RSP			0x1004					// sim����ͬ����Ӧ��
// #define SIM_REQ					0x0005					// sim�����������
// #define SIM_RSP					0x1005					// sim��������Ӧ��
#define SIMLINK_CREATE_REQ		0x0005					// sim����·��������
#define SIMLINK_CREATE_RSP		0x1005					// sim����·������Ӧ
#define SIMDATA_REQ				0x0007					// sim����͸�������
#define SIMDATA_RSP				0x1007					// sim����͸����Ӧ��
// #define SIMRELEASE_REQ			0x0007					// sim���ͷ������
// #define SIMRELEASE_RSP			0x1007					// sim���ͷ���Ӧ��
#define SIMLINK_RELEASE_REQ		0x0006					// sim����·�ͷ�����
#define SIMLINK_RELEASE_RSP		0x1006					// sim����·�ͷ���Ӧ
#define SIMDATADELAY_REQ		0x0008					// sim��������ʱ֪ͨ�����		<---- ����Emu�忨ʵ�֣��������贫����౨��
#define SIMHPD_REQ				0x0009					// 
#define SIMHPD_RSP				0x1009					// 
#define SIMPULLPLUG_REQ			0x000A					// sim�����֪ͨ�����
#define SIMPULLPLUG_RSP			0x100A					// sim�����֪ͨ��Ӧ��

#define POWER_RESET_SPAN_REQ	0x000B
#define POWER_RESET_SPAN_RSP	0x100B

#define CHAN_HANG_UP_REQ		0x000C
#define CHAN_HANG_UP_RSP		0x100C

#define SIMSMS_REQ				0x000D					// sim�����ŷ��������
#define SIMSMS_RSP				0x100D					// sim�����ŷ�����Ӧ��

#define NAT_ACROSS_START_NOTICE		0x0011
#define NAT_ACROSS_INFO				0x0012
#define NAT_ACROSS_PEER_INFO		0x0013

#define LINKINFO_REPORT_REQ		0x0004					// ��·��Ϣ�ϱ�
#define LINKINFO_REPORT_RSP		0x1004					// ��·��Ϣ�ϱ���Ӧ

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

#define SERIALNO_LEN			10						// SimEmuSvr���кų���
#define PACKAGE_HEADER_LEN		10						// ����ͷ������
#define MESSAGE_LEN				140
#define LONG_MESSAGE_LEN		MESSAGE_LEN * 7			//�����ų���

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

#define SIM_PULL_NOTICE_REQ	0x0203						// sim���γ�֪ͨ
#define SIM_PULL_NOTICE_RSP	0x1203						// sim���γ�֪ͨ��Ӧ
#define SIM_PLUG_NOTICE_REQ	0x0204						// sim������֪ͨ
#define SIM_PLUG_NOTICE_RSP	0x1204						// sim������֪ͨ��Ӧ

#define AST_CHAN_STAT_NOTICE_REQ 0x0205					//sim��ע������Ӧ


#define LINK_BREAK_NOTICE_REQ	0x0211
#define LINK_BREAK_NOTICE_RSP	0x1212

#define AMI_EVENT_MSG			0x0301
#define AMI_CLI_MSG				0x0302

#define VGSM_LEN				0xF000	//60K		//0x10000		// ����sim�����泤�ȣ�64K


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


// ������������
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
// ���罻����ͷ���ݽṹ
typedef struct PACKAGE_HEADER_S
{
	unsigned short version;			// �汾��Ϣ
	unsigned short cmd;				// ��Ϣ����
	unsigned short len;				// ���س���
	unsigned short result;			// ����롣0���ɹ���1��ʧ��
	//unsigned short server_type;		// ����������
	//char serial[10];				// �������к�
	unsigned short reserve;			// ������
}package_header_t;
// ���罻�������ݽṹ
typedef struct PACKAGE_S
{
	package_header_t header;		// ��ͷ
	char body[BUFF_SIZE];			// ����
}package_t;


typedef struct SIM_HANG_UP_REQ_S
{
	char gw_seri[12];
	unsigned short gw_bank_nbr;
	unsigned short gw_slot_nbr;
}sim_hang_up_req_t;

typedef struct SMS_SEND_REQ_INFO_S
{
	unsigned short sms_type;		// ���Ͷ������� 1:��ѯ���� 2:�黰�� 3:����
	char seri[12];					// ���շ����к�
	unsigned short sb_bank_nbr;		// bank���
	unsigned short sb_slot_nbr;		// ����Sim���򿨲�����
	unsigned short chn_nbr;			// �˿ںţ�asterisk��ʾ�Ķ˿ں�
	unsigned short gw_bank_nbr;
	unsigned short gw_slot_nbr;
	char dst_num[16];
	char src_num[16];
	char match_str[32];
	char send_msg[MESSAGE_LEN];
}sms_send_req_info_t;

typedef struct SMS_SEND_RSP_INFO_S
{
	unsigned short sms_type;		// ���Ͷ������� 1:��ѯ���� 2:�黰�� 3:����
	char sb_seri[12];				// ���ͷ����к�
	unsigned short sb_bank_nbr;		// bank���
	unsigned short sb_slot_nbr;		// ����Sim���򿨲�����
	char recv_msg[LONG_MESSAGE_LEN];
	char match_msg[MESSAGE_LEN];
	char caller[16];
	char recv_time[32];
}sms_send_rsp_info_t;

// ע���������ݽṹ
typedef struct REGISTER_REQ_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	char model_name[12];
	unsigned short slot_num;
	char passwd[64];				// ����
}register_req_info_t;
typedef struct REGISTER_RSP_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	char model_name[12];
	unsigned short slot_num;
}register_rsp_info_t;

// ע�����ݽṹ
typedef struct UNREGISTER_REQ_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short bank_nbr;		// bank������
	unsigned short slot_nbr;		// ����Sim���򿨲�����
}unregister_req_info_t;
typedef struct UNREGISTER_RSP_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short bank_nbr;		// bank������
	unsigned short slot_nbr;		// ����Sim���򿨲�����
}unregister_rsp_info_t;

// �������ݽṹ
typedef struct HEARTBEAT_REQ_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short bank_nbr;		// bank������
	unsigned short slot_nbr;		// ����Sim���򿨲�����
}heartbeat_req_info_t;
typedef struct HEARTBEAT_RSP_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short bank_nbr;		// bank������
	unsigned short slot_nbr;		// ����Sim���򿨲�����
}heartbeat_rsp_info_t;


// ��·�ϱ����ݽṹ
typedef struct REPORT_REQ_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	unsigned short module_type;		// ͨ��ģ������ 1:GSM 2:CMDA 3:ȫ��ͨ 4:����
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short usb_nbr;
	unsigned short bank_nbr;		// bank���
	unsigned short slot_nbr;		// ����Sim���򿨲�����
	unsigned short chn_nbr;			// �˿ںţ�asterisk��ʾ�Ķ˿ں�
	unsigned int ip;
	unsigned short port;
	unsigned short atr_len;
	char atr[32];					// sim atr, emu�ϱ�ʱΪ��
	char eficcid[24];
	unsigned short vgsm_len;
	unsigned char vgsm[VGSM_LEN];	// ����sim������
}report_req_info_t;

typedef struct REPORT_RSP_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short usb_nbr;
	unsigned short bank_nbr;		// bank���
	unsigned short slot_nbr;		// ����Sim���򿨲�����
	unsigned short stat;
}report_rsp_info_t;

typedef struct LINK_INFO_S
{
	char sb_seri[10];
	unsigned short sb_usb_nbr;		// bank���
	unsigned short sb_bank_nbr;		// bank���
	unsigned short sim_nbr;
	char eficcid[24];
	unsigned int sb_ip;
	unsigned short sb_port;
	unsigned short atr_len;
	char sim_atr[32];
	char gw_seri[10];
	unsigned short gw_bank_nbr;		// ��ʱΪ0
	unsigned short slot_nbr;
	unsigned int gw_ip;
	unsigned short gw_port;
	unsigned short led_stat;
	//unsigned short vgsm_len;
	short call_rest_time;	//ͨ��ʣ��ʱ��
	//char vgsm[VGSM_LEN];			// ����sim������
}link_info_t;


// sim������ϱ����ݽṹ
typedef struct SIM_PULLPLUG_INFO_S
{
	char sb_seri[SERIALNO_LEN];		// �������к�
	unsigned short sb_usb_nbr;
	unsigned short sb_bank_nbr;		// bank���
	unsigned short sb_slot_nbr;		// ����Sim���򿨲�����
	char gw_seri[SERIALNO_LEN];		// �������к�
	unsigned short gw_bank_nbr;
	unsigned short gw_slot_nbr;		// bank���
}sim_pullplug_info_t;


typedef struct CALL_MORING_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short bank_nbr;
	unsigned short slot_nbr;
	char calldst[16];
}call_moring_info_t;
// ���غ���֪ͨ
typedef struct CALL_BEGIN_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short bank_nbr;
	unsigned short slot_nbr;
	char calldst[16];
}call_begin_info_t;
// ���غ��н�֪ͨͨ
typedef struct CALL_END_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short bank_nbr;
	unsigned short slot_nbr;
}call_end_info_t;

typedef struct SMS_RESULT_INFO_S
{
	unsigned short type;			// ���������ͣ�1��SimEmuSvr��2��SimRdrSvr��3��SimProxySvr
	char seri[SERIALNO_LEN];		// �������к�
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
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short bank_nbr;
	unsigned short slot_nbr;
	char msg[64];
	char uuid[24];
	unsigned int timeout;
}ussd_info_t;

typedef struct SMS_MODE_INFO_S
{	
	char seri[SERIALNO_LEN];		// �������к�
	unsigned short chnl_nbr;
	unsigned char mode;
}sms_mode_info_t;


#pragma pack()






/**************************** ���������Ͷ��� ****************************/

/**************************************************************************** 
* �������� : makeReqPackage
* �������� : ������������
* ��    �� : unsigned short cmd			: ���������
* ��    �� : char *seri					: SimEmuSvr���к�
* ��    �� : char *data					: ��������
* ��    �� : unsigned short data_len	: �������ݳ���
* ��    �� : char *buff					: �������ݻ��壬�������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeReqPackage(unsigned short cmd, char *data, unsigned short data_len, char *buff);

/**************************************************************************** 
* �������� : makeRspPackage
* �������� : ��Ӧ���������
* ��    �� : char *buff_req				: ��������壬����洢��������
* ��    �� : char *buff_rsp				: ��Ӧ�����壬�������
* ��    �� : char *data					: ��������
* ��    �� : unsigned short data_len	: �������ݳ���
* ��    �� : unsigned short result		: ���ĳɹ���־
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeRspPackage(char *buff_req, char *buff_rsp, char *data, unsigned short data_len, unsigned short result);

int makeSimDataRspPackage(char *buff_rsp, char *data, unsigned short data_len, unsigned short result);
int makeSimPullPlugReqPackage(char *buff_rsp, char *data, unsigned short data_len, unsigned short result);


/**************************************************************************** 
* �������� : parsePackage
* �������� : ���Ľ�������
* ��    �� : char *buff					: ���Ļ���
* ��    �� : package_t *msg				: ������Ϣ���ݽṹ����
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int parsePackage(char *buff, package_t *msg);

/**************************************************************************** 
* �������� : checkPackage
* �������� : ����У���麯��
* ��    �� : char *buff					: ���Ļ���
* ��    �� : int len					: ���ĳ���
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int checkPackage(char *buff, int len);

/**************************************************************************** 
* �������� : parsePackHeader
* �������� : ���İ�ͷ��������
* ��    �� : char *pack					: ���Ļ���
* ��    �� : package_header_t *header	: ��ͷ���ݽṹ����
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int parsePackHeader(char *pack, package_header_t *header);

/**************************************************************************** 
* �������� : getVersion
* �������� : ��ȡ���İ汾����
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : ���İ汾��
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getVersion(char *buff);

/**************************************************************************** 
* �������� : getCmd
* �������� : ��ȡ�������ͺ���
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : �������ͺ�
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getCmd(char *buff);

/**************************************************************************** 
* �������� : getLen
* �������� : ��ȡ���ĸ��س��Ⱥ���
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : ���ĸ��س���
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getLen(char *buff);

/**************************************************************************** 
* �������� : getResult
* �������� : ��ȡ���Ľ���뺯��
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : ���Ľ����
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getResult(char *buff);

/**************************************************************************** 
* �������� : getServerType
* �������� : ��ȡ�������������������
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : �������������������
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getServerType(char *buff);

/**************************************************************************** 
* �������� : getSerialNo
* �������� : ��ȡ���Ľ���뺯��
* ��    �� : char *buff					: ���Ļ���
* ��    �� : char *seri					: ����Emu���к�
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int getSerialNo(char *buff, char *seri);

/**************************************************************************** 
* �������� : getReserve
* �������� : ��ȡ����Ԥ�ú���
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : ����Ԥ��λ
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getReserve(char *buff);

/**************************************************************************** 
* �������� : checksum
* �������� : У�麯��
* ��    �� : unsigned short *buffer		: ���Ļ���
* ��    �� : int size					: ���ĳ���
* �� �� ֵ : У����1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short checksum(unsigned short *buffer, int size);

/**************************************************************************** 
* �������� : makeRegisterPack
* �������� : ע����������
* ��    �� : char *buff_req				: ע����������壬�������
* ��    �� : unsigned short server_type	: ע�����������
* ��    �� : char *seri					: ע����������к�
* ��    �� : char *data					: ��������
* ��    �� : int data_len				: �������ݳ���
* ��    �� : int len					: ע����������峤�ȣ��������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeRegisterPack(char *buff, char *data, int data_len, int *len);

/**************************************************************************** 
* �������� : makeUnRegisterPack
* �������� : ע�����������
* ��    �� : char *buff_req				: ע����������壬�������
* ��    �� : char *seri					: SimEmuSvr���к�
* ��    �� : char *data					: ��������
* ��    �� : int data_len				: �������ݳ���
* ��    �� : int len					: ע����������峤�ȣ��������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeUnRegisterPack(char *buff, char *data, int data_len, int *len);

/**************************************************************************** 
* �������� : makeHeartbeatPack
* �������� : �������������
* ��    �� : char *buff_req				: ������������壬�������
* ��    �� : char *seri					: SimEmuSvr���к�
* ��    �� : char *data					: ��������
* ��    �� : int data_len				: �������ݳ���
* ��    �� : int len					: ������������峤�ȣ��������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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
