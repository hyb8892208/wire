

/****************************************************************************
* ��Ȩ��Ϣ��
* ϵͳ���ƣ�SimRdrSvr
* �ļ����ƣ�EmuRegisterList.h 
* �ļ�˵����Emuע������б�ͷ�ļ�
* ��    �ߣ�hlzheng 
* �汾��Ϣ��v1.0 
* ������ڣ�
* �޸ļ�¼��
* ��    ��		��    ��		�޸��� 		�޸�ժҪ  
****************************************************************************/

/**************************** ��������ѡ���ͷ�ļ� ****************************/
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
//Bank Side ָ��
#define CMD_IS_DATA    0    //����Ҫresp
//  <- pc
#define CMD_IS_SETATR  0x81 //��Ҫresp
#define CMD_IS_GETVER  0x83 //��Ҫresp
#define CMD_IS_REQRST  0x85 //��Ҫresp reset itself
#define CMD_IS_GETATR  0X71
#define CMD_IS_GETATR_FULLRESET  0X72
#define CMD_IS_BACK2DFU  0X87
#define CMD_IS_GETSIMCARD_DETECT  0X89
#define CMD_IS_GETBOARDID  0X88
#define CMD_IS_IAP  0X61

//Bank Side LEDCtrlrָ��
#define CMD_IS_LED_RED  0X91
#define CMD_IS_LED_GREEN  0X92
#define CMD_IS_LED_OFF   0X93
#define CMD_IS_LED_RED_FLASH  0X94
#define CMD_IS_LED_GREEN_FLASH  0X95

//EMU Side ָ��
//  -> PC
#define IS_REQ_RST_ICC    0x10 //����Ҫresp
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
// 	Sim_Pull = 0,	// �γ�
// 	Sim_Plug = 1	// ����
// };

// sim�����״̬���ݽṹ
typedef struct SIM_PLUS_STAT_S
{
	unsigned short stat; // 0:pull; 1:plug
}sim_plus_stat_t;

// sim������ATR���ݽṹ
typedef struct SIM_ATR_S
{
	unsigned char sim_atr_ori[320];		// ԭʼATR
	unsigned char sim_atr_mod[320];		// ����ATR
	unsigned int len_ori;				// ԭʼATR����
	unsigned int len_mod;				// ����ATR����
	unsigned char td;					// 0:����1:����
}sim_atr_t;


// ������Ϣ���ݽṹ
typedef struct SLOT_INFO_S
{
	unsigned short gw_bank_nbr;
	unsigned short slot_nbr;		// ���ۺ�
	unsigned int svr_ip;			// ������IP��ַ
	unsigned short svr_port;		// ������PORT�˿�
	unsigned short sb_bank_nbr;
	unsigned short sim_nbr;			// sim����
	unsigned short stat;			// ״̬
	//char svr_ip[16];
}slot_info_t;

// sim��������Ϣ���ݽṹ
typedef struct SIMCARD_HANDLE_INFO_S
{
	unsigned short cmd;			// ����
	char emu_seri[10];			// Emu���к�
	slot_info_t slot_info;		// ������Ϣ
}simcard_handle_info_t;

// ��Ϣͷ���ݽṹ
typedef struct MSG_HEADER_S
{
	unsigned short cmd;			// 1: sim data; 2:sim release
	unsigned short bank;		// bank nunmber, emu is reserve
	unsigned short slot;		// slot number
	unsigned short data_len;	// data len
	unsigned int cnt;			// count
}msg_header_t;

// ��Ϣ���ݽṹ
typedef struct MSG_S
{
	msg_header_t header;		// ��Ϣͷ
	char data[BUFF_SIZE];		// ��Ϣ��
}msg_t;

#pragma pack()

// Rdrע��״̬���ݽṹ
typedef struct REGISTER_STAT_S
{
	int stat;				// ״̬��0:unregister; 1:registed
	pthread_mutex_t lock;	// ������
}register_stat_t;

// SimRdrSvr���������ò������ݽṹ�����ڱ������������ļ���ȡ�Ĳ�����Ϣ
typedef struct SIMRDRSVRPARAM_S
{
	pthread_t tid;
	char seri[12];						// 
	char passwd[64];					// ע������
	char net_mode[12];					// ����ģʽ��server������ˣ�client���ͻ���
	char local_ip[16];					// ����IP��ַ
	char server_ip[16];					// ������IP��ַ
	unsigned short server_port;			// ������PORT�˿�
	unsigned short hb_interval;			// �������
	char usb_file[32];					// usb���ļ���
	//unsigned short max_rdr_nbr;				// rdr sim�忨�������
	unsigned short sim_hdl_port_usb;
	unsigned short sim_hdl_port_net;
	unsigned short sim_hdl_port_proxy;
	unsigned short usb_hdl_port;
	unsigned short net_hdl_port;
	unsigned short net_hdl_port_php;	// NetHdlTask��PHPͨѶ�˿�
	char php_hdl_ip[16];				// PHP��NetHdlTaskͨѶIP
	unsigned short php_hdl_port;		// PHP��NetHdlTaskͨѶ�˿�
}simrdrsvrparam_t;

// sim��״̬��Ϣ���ݽṹ
typedef struct SIM_STAT_S
{
	unsigned char stat;				// ��ǰ״̬
	unsigned char sync;				// ͬ��״̬
}sim_stat_t;

// SimHdlTask�̲߳������ݽṹ
typedef struct SIMHDLTASK_PARAM_S
{
	pthread_t tid;						// �̺߳�
	unsigned short usb_nbr;				// usb�ļ���
	unsigned short sim_nbr;				// sim����
	unsigned short svr_port;			// Proxy������PORT�˿�
	unsigned short usb_port;			// Port�˿ڣ���������USBHdlTask
	unsigned short net_port;			// Port�˿ڣ���������NetHdlTask
	unsigned short proxy_port;			// Port�˿ڣ���������Proxy
	unsigned short usbhdl_port;
	unsigned short nethdl_port;
	unsigned short stop;				// ֹͣ��־��0��not stop��1��stop
	char svr_ip[16];					// ������IP��ַ
	char localhost[16];					// ����IP��ַ	
	sim_atr_t *sim_atr;
}simhdltask_param_t;

// USBHdlTask�̲߳������ݽṹ
typedef struct USBHDLTASK_PARAM_S
{
	pthread_t tid;
#ifdef WIN32
	void *handle;						// usb���
#else
	//FILE *handle;						// usb���
	hid_device *handle;
#endif
	char usb_file[32];
	unsigned short usb_nbr;				// usb�ļ���
	unsigned short bank_nbr;			// bank��
	unsigned short usb_hdl_port;
	unsigned short stop;
	unsigned char old_stat;				// sim��ԭ״̬��0��7bit��Ӧ0��7ͨ��
	sim_stat_t new_stat;				// sim��״̬
	unsigned char type;					// usb���͡�1��SBX_BD��2��SimRdrII
	unsigned short stat;				// ��Ч��־��0:invalid; 1:valid
	sim_atr_t sim_atr[SIM_NBR];			// sim atr����
}usbhdltask_param_t;

typedef struct SIMSERVER_INFO_S
{
// 	char file[32];						// usb�ļ�·��
// 	int bank_nbr;						// bank��

// 	unsigned short usb_hdl_port;
// 	unsigned short stop;

	usbhdltask_param_t usbhdltask_param;
	
	//simrdrsvrparam_t simrdrsvrparam[SIM_NBR];
	simhdltask_param_t simhdltask_param[SIM_NBR];
}simserver_info_t;

typedef struct SBX_BD_INFO_s
{
#ifdef WIN32
	void *handle;						// usb���
#else
	hid_device *handle;
#endif
	char usb_file[32];
}sbx_bd_info_t;

/**************************** ���������Ͷ��� ****************************/

/**************************************************************************** 
* �������� : readConfigValue
* �������� : ��ȡ�����ļ�
* ��    �� : void
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int readConfigValue(simrdrsvrparam_t *param);

/**************************************************************************** 
* �������� : RdrSimDataSyncHdl
* �������� : sim����Ϣͬ��������
* ��    �� : char *buff_req				: ���󻺳壬����������������
* ��    �� : char *buff_rsp				: ��Ӧ����
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int RdrSimDataSyncHdl(char *buff_req, char *buff_rsp);

/**************************************************************************** 
* �������� : parseSimAtr
* �������� : ����sim��ATR����
* ��    �� : unsigned char *data		: sim��ATR���壬����ATR����
* ��    �� : unsigned long len			: sim��ATR����
* ��    �� : sim_atr_t *simAtr			: sim��ATR��Ϣ���ݽṹ����
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int parseSimAtr(unsigned char *data, unsigned int len, sim_atr_t *simAtr);

/**************************************************************************** 
* �������� : makeRdrCmd
* �������� : rdr�����ת��Emu�ĸ������ݵĸ�ʽ�����ڷ��͵�Rdr
* ��    �� : char *data					: ��������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeRdrCmd(char *data);

/**************************************************************************** 
* �������� : makeRspToEmu
* �������� : rdr�����ת��Rdr��Ӧ�ĸ������ݵĸ�ʽ�����ڷ��͵�Emu
* ��    �� : char *data					: ��������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeRspToEmu(unsigned char *rsp, unsigned long *len, usbhdltask_param_t *param);

/**************************************************************************** 
* �������� : RdrSimReqHdl
* �������� : sim�����봦����
* ��    �� : char *buff_req				: �������ݻ��壬�洢��sim�����뱨������
* ��    �� : char *buff_rsp				: ��Ӧ���ݻ���
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
//int RdrSimReqHdl(char *buff_req, char *buff_rsp);

/**************************************************************************** 
* �������� : RdrSimReleaseHdl
* �������� : sim���ͷŴ�����
* ��    �� : char *buff_req				: �������ݻ��壬�洢��sim���ͷű�������
* ��    �� : char *buff_rsp				: ��Ӧ���ݻ���
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
//int RdrSimReleaseHdl(char *buff_req, char *buff_rsp);

/**************************************************************************** 
* �������� : initHid
* �������� : ��ʼ��USB����
* ��    �� : char *usb_file		: usb�ļ�·��, ui_hid1, ui_hid2, ...
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
//int initHid(char *usb_file);

/**************************************************************************** 
* �������� : USBHdlTask
* �������� : USB�����̺߳���
* ��    �� : void *pParam				: �̺߳�������
* �� �� ֵ : void *
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void * USBHdlTask(void *pParam);

/**************************************************************************** 
* �������� : EmuConnHdlTask
* �������� : Emu���ӹ����̺߳���
* ��    �� : void *pParam				: �̺߳�������
* �� �� ֵ : void *
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void * EmuConnHdlTask(void *pParam);

/**************************************************************************** 
* �������� : NetHdlTask
* �������� : �������Emu����
* ��    �� : void *pParam				: �̺߳�������
* �� �� ֵ : void *
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void * NetHdlTask(void *pParam);

/**************************************************************************** 
* �������� : SimHdlTask
* �������� : ����sim��ҵ�����ݴ���
* ��    �� : void *pParam				: �̺߳�������
* �� �� ֵ : void *
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
void * SimHdlTask(void *pParam);

/**************************************************************************** 
* �������� : getSimStat
* �������� : ��ȡsim�����״̬
* ��    �� : hid_device *handle		: usb���
* ��    �� : sim_stat_t *stats			: sim��״̬��Ϣ
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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
* �������� : getSimAtr
* �������� : ��ȡsim��atr����
* ��    �� : hid_device *handle			: usb���
* ��    �� : sim_atr_t *atr				: sim���������ݽṹ
* ��    �� : unsigned short sim_nbr		: sim����
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int getSimAtr(hid_device *handle, sim_atr_t *atr, unsigned short sim_nbr);

/**************************************************************************** 
* �������� : getBandNbr
* �������� : ��ȡbank��
* ��    �� : hid_device *handle			: usb���
* ��    �� : unsigned char bank_nb		: bank��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int getBankNbr(hid_device *handle, unsigned char *bank_nbr);

/**************************************************************************** 
* �������� : getBankVersion
* �������� : ��ȡbank�汾
* ��    �� : getBankVersion *handle			: usb���
* ��    �� : unsigned char bank_nb		: bank��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int getBankVersion(hid_device *handle, char *version, unsigned short version_len);

/**************************************************************************** 
* �������� : hidWR
* �������� : ��дHID�豸
* ��    �� : getBankVersion *handle			: usb���
* ��    �� : unsigned char *req				: д�뻺��
* ��    �� : unsigned short req_len				: д�뻺�峤��
* ��    �� : unsigned char *rsp				: ��������
* ��    �� : unsigned short rsp_len				: �������峤��
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int hidWR(hid_device *handle, unsigned char *req, unsigned short req_len, unsigned char *rsp, unsigned short *rsp_len);

#endif