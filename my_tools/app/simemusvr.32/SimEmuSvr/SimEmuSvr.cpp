/****************************************************************************
* 版权信息：
* 系统名称：SimEmuSvr
* 文件名称：SimEmuSvr.c
* 文件说明：SimEmuSvr功能处理实现文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
* 2014-10-21        v1.1				hlzheng			增加开始通话，结束通话换卡通知消息操作
****************************************************************************/
#include <signal.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "SimEmuSvr.h"

#include "config.h"

#include "serial.h"

//#include "debug.h"

#include "zprint.h"

#include "EmuVcard.h"
#include "hiredis.h"

extern "C" {
#include "base32.h"
}

int gCommHandle = -1;

register_stat_t gRegisterStat;
 
char gEmuVersion[320];
mini52_version_t gMini52Version[SLOT_NBR];


commhdltask_param_t gCommHdlTaskParam[MAX_BOARD];
slothdltask_param_t gSlotHdlTaskParam[MAX_BOARD][SLOT_NBR];
pthread_t gSlotHdlTaskTid[MAX_BOARD][SLOT_NBR];
pthread_t gNetHdlTaskTid;
pthread_t gCommHdlTaskTid[MAX_BOARD];


pthread_attr_t attr_nethdltask;
struct sched_param param_nethdltask;


ttyUSBx_t g_ttyUSBx;

int sock_hdl_stop = 0;

net_handle_t gNetHandles[] =
{
	{UNREGISTER_RSP,			UnRegisterRspHdl},
	{HEARTBEAT_RSP,				HeartbeatRspHdl},
	{LINKINFO_REPORT_RSP,		SimLinkReportRspHdl},
	{SIMLINK_CREATE_REQ,		SimLinkCreateReqHdl},
	{SIMLINK_RELEASE_REQ,		SimLinkReleaseReqHdl},
	{SIMDATA_RSP,				SimDataRspHdl},
	{SIM_PULLPLUG_NOTICE_REQ,	SimPullPlugNoticeHdl},
	{SIMSMS_REQ,				SimSMSReqHdl},
	{CHAN_HANG_UP_REQ,			SIMHangUpReqHdl},
	{SET_MODULE_LIMIT,			ModuleLimitReqHdl},
	{SET_MODULE_INTERNET,       ModuleInterReqHdl},
	{SEND_USSD,					SendUssdReqHdl},
	{SET_MODULE_SMS_MODE,		ModuleSetSmsModeHdl},
};

ast_handle_t gAstHandles[] =
{
	{CALL_MORING_NOTICE,	CallMoringHdl},
	{CALL_BEGIN_NOTICE,		CallBeginHdl},
	{CALL_END_NOTICE,		CallEndHdl},
	{SMS_RESULT_NOTICE,		SmsResultHdl},
	{SMS_REPORT_NOTICE,		SmsReportHdl},
	{USSD_RESULT_NOTICE,	UssdResultHdl}
};

struct module_name_s gModules[] = 
{
	{"Quectel_M35",GSM_MODULE},
	{"SIMCOM_SIM6320C",CDMA_MODULE},
	{"EC20",LTE_MODULE},
	{"EC25",GSM_MODULE},
	{"Quectel_M26",GSM_MODULE},
	{"m35",GSM_MODULE},
	{"sim840",GSM_MODULE},
	{"uc15",GSM_MODULE},
};


nethdl_params_t gNetHdlParams;
static time_t last_seen;

simemusvrparam_t gSimEmuSvrParam;
CSocketEx gSockReg;

port_map_info_t port_map_info;
extern int apdu_switch;
extern int log_class;
extern void *emu_soapservice(void * param);
static int getChannelInterStatFromRedis(int chan_no, unsigned char *inter_stat);
static void setRedisofChannelInterStat(int chan_no,unsigned char inter_stat);

int DataToHex(unsigned char *buff, unsigned short buff_len,const char *dire,unsigned short board_nbr,unsigned short slot_nbr)
{	
	if(0 == apdu_switch){
		return 0;
	}
	unsigned short i = 0;
	char buff_msg[1024] = {0};
	int len = 0;

	len += sprintf(buff_msg + len, "[%02d-%02d]",board_nbr,slot_nbr);
	strcpy(buff_msg+len, dire);
	len += strlen(dire);
	while (i < buff_len)
	{
		sprintf(buff_msg+len, "%02x ", buff[i]);
		len += 3;
		i++;
	}
	zprintf(INFO,"%s",buff_msg);
	return len;
}

int get_config(char* file_path, char* context_name, char* option_name,char *content)
{
	if (file_path == NULL || context_name == NULL || option_name == NULL || content == NULL)
	{
		return -1;
	}

	char buf[1024] = {0};
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	char name[256];
	FILE* fp;

	if( NULL == (fp=fopen(file_path,"r")) ) {
		zprintf(ERROR,"[ERROR]Can't open %s",file_path);
		return -1;
	}

	while(fgets(buf,1024,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0== strcmp(name,context_name) ) {
						finded=1;
					} else {
						if(finded) 
							finish=1;
					}
				}
				break;
			case '=':
				if(finded && !finish) {
					memcpy(name,buf,i);
					name[i] = '\0';
					if(0==strcmp(name,option_name)) {
						memcpy(name,buf+i+1,len-i-1-1);
						name[len-i-1-1] = '\0';
						sprintf(content,"%s",name);
						fclose(fp);
						return 1;
					}
				}
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	return 0;
}

#if 0
int cat_file(char *file_name,char *buff)
{
	FILE *fp = fopen(file_name,"r");
	int len = 0;
	if(fp)
	{
		while(0 == feof(fp)){
			len += fread(buff + len,1,128,fp);
		}
		fclose(fp);
	}else{
		zprintf(INFO,"open %s error:%s",file_name,strerror(errno));
	}
	buff[len] = '\0';
	return len;
}
#endif

int get_port_from_map(int dev)
{
	int i = 0;
	if (dev < 0)
	{
		return -1;
	}
	for (i = 0; i < MAX_BOARD*SLOT_NBR; i++)
	{
		if (port_map_info.port_map[i].dev == dev)
		{
			return port_map_info.port_map[i].port;
		}
	}
	return -1;
}
int get_dev_from_map(int port)
{
	int i = 0;
	if (port < 0)
	{
		return -1;
	}
	for (i = 0; i < MAX_BOARD*SLOT_NBR; i++)
	{
		if (port_map_info.port_map[i].port == port)
		{
			return port_map_info.port_map[i].dev;
		}
	}
	return -1;
}

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
int UnRegisterRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	unsigned short result = getResult(buff_recv);
	if (result != 0)
	{
		//zprintf("[ERRO]UnRegisterRspHdl: heartbeat rsp'result is fail");
		zprintf(ERROR,"[ERRO]UnRegisterRspHdl: heartbeat rsp'result is fail");
	}
	else
	{
		//zprintf("[INFO]UnRegisterRspHdl: heartbeat rsp success");
		zprintf(ERROR,"[INFO]UnRegisterRspHdl: heartbeat rsp success");
	}
	return 0;
}

void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

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
int HeartbeatRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	unsigned short result = getResult(buff_recv);
	if (result != 0)
	{
		zprintf(ERROR,"[ERRO]HearbeatRspHdl: heartbeat rsp'result is fail");
	}
	else
	{
		last_seen = time(NULL);
		zprintf(INFO,"[INFO]HearbeatRspHdl: heartbeat rsp success");
	}
	return 0;
}

int SimLinkCreateReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	link_info_t link_info;
	char seri_emu[12]  = {0};
	char seri_bank[12] = {0};
	msg_t msg;
	int ret = 0;
	unsigned short len = 0;
	char gwaddr[16] = {0};
	char sbaddr[16] = {0};

	len = getLen(buff_recv);
	// get req info
	memcpy((char *)&link_info, buff_recv+PACKAGE_HEADER_LEN, len);//sizeof(link_info));

	memcpy(seri_bank, link_info.sb_seri, SERIALNO_LEN);
	link_info.sb_port = ntohs(link_info.sb_port);
	link_info.sb_usb_nbr = ntohs(link_info.sb_usb_nbr);
	link_info.sb_ip = ntohl(link_info.sb_ip);
	link_info.sb_bank_nbr = ntohs(link_info.sb_bank_nbr);
	link_info.sim_nbr = ntohs(link_info.sim_nbr);
	link_info.atr_len = ntohs(link_info.atr_len);
	memcpy(seri_emu, link_info.gw_seri, SERIALNO_LEN);
	link_info.gw_port = ntohs(link_info.gw_port);
	link_info.gw_bank_nbr = ntohs(link_info.gw_bank_nbr);
	link_info.slot_nbr = ntohs(link_info.slot_nbr);
	//link_info.vgsm_len = ntohs(link_info.vgsm_len);
	link_info.call_rest_time = ntohs(link_info.call_rest_time);
	gSlotHdlTaskParam[link_info.gw_bank_nbr][link_info.slot_nbr].call_rest_time = link_info.call_rest_time;
	
	zprintf(INFO,"[INFO]SimLinkCreateReqHdl[00-%02d-%02d]: Emu[%s-00-%02d-%02d-%s:%d] == Rdr[%s-%02d-%02d-%s:%d] rest time=%d minutes", \
		link_info.gw_bank_nbr, link_info.slot_nbr, \
		seri_emu, link_info.gw_bank_nbr, link_info.slot_nbr, ip_num_to_char(ntohl(link_info.gw_ip), gwaddr), link_info.gw_port, \
		seri_bank, link_info.sb_bank_nbr, link_info.sim_nbr, ip_num_to_char(link_info.sb_ip, sbaddr), link_info.sb_port,link_info.call_rest_time);
	// send link info to SlotHdlTask
	memset(&msg, 0, sizeof(msg));
	msg.header.cmd = SIMLINK_CREATE_REQ;
	msg.header.data_len = len; //sizeof(link_info);
	msg.header.slot = link_info.slot_nbr;
	memcpy(msg.data, (char *)&link_info, len); //sizeof(link_info));
	ret = csock->WriteData(csock->getSocket(), (char *)&msg, sizeof(msg.header)+msg.header.data_len, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].local_ip, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].port_nettask);
	if ((unsigned short)ret != (sizeof(msg.header)+msg.header.data_len))
	{
		zprintf(ERROR,"[ERRO]SimLinkCreateReqHdl[00-%02d-%02d]: write link info to SlotHdlTask[%s:%d] error(%d:%s)\n", \
			link_info.gw_bank_nbr, link_info.slot_nbr, \
			gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].local_ip, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].port_nettask, errno, strerror(errno));
	}
	//
	return 0;
}
int SimLinkReleaseReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	int ret = 0;
	link_info_t link_info;
	char seri_emu[12]  = {0};
	char seri_bank[12] = {0};
	msg_t msg;
	unsigned short len = 0;
	char gwaddr[16] = {0};
	char sbaddr[16] = {0};
	
	// get req info
	len = getLen(buff_recv);
	memcpy((char *)&link_info, buff_recv+PACKAGE_HEADER_LEN, len); //sizeof(link_info));
	
	memcpy(seri_bank, link_info.sb_seri, SERIALNO_LEN);
	link_info.sb_port = ntohs(link_info.sb_port);
	link_info.sb_bank_nbr = ntohs(link_info.sb_bank_nbr);
	link_info.sim_nbr = ntohs(link_info.sim_nbr);
	link_info.atr_len = ntohs(link_info.atr_len);
	memcpy(seri_emu, link_info.gw_seri, SERIALNO_LEN);
	link_info.gw_port = ntohs(link_info.gw_port);
	link_info.gw_bank_nbr = ntohs(link_info.gw_bank_nbr);
	link_info.slot_nbr = ntohs(link_info.slot_nbr);
	
	zprintf(INFO,"[INFO]SimLinkReleaseReqHdl[00-%02d-%02d]: Emu[%s-00-%02d-%02d-%s:%d] == Rdr[%s-%02d-%02d-%s:%d]", \
		link_info.gw_bank_nbr, link_info.slot_nbr, \
		seri_emu, link_info.gw_bank_nbr, link_info.slot_nbr, ip_num_to_char(ntohl(link_info.gw_ip), gwaddr), link_info.gw_port, \
		seri_bank, link_info.sb_bank_nbr, link_info.sim_nbr, ip_num_to_char(ntohl(link_info.sb_ip), sbaddr), link_info.sb_port);
	
	// send link info to SlotHdlTask
	memset(&msg, 0, sizeof(msg));
	msg.header.cmd = SIMLINK_RELEASE_REQ;
	msg.header.data_len = len; //sizeof(link_info);
	msg.header.slot = link_info.slot_nbr;
	memcpy(msg.data, (char *)&link_info, len); //sizeof(link_info));
	ret = csock->WriteData(csock->getSocket(), (char *)&msg, sizeof(msg.header)+msg.header.data_len, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].local_ip, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].port_nettask);
	if ((unsigned short)ret != (sizeof(msg.header)+msg.header.data_len))
	{
		zprintf(ERROR,"[ERRO]SimLinkReleaseReqHdl[00-%02d-%02d]: write link info to SlotHdlTask[%s:%d] error(%d:%s)\n", \
			link_info.gw_bank_nbr, link_info.slot_nbr, \
			gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].local_ip, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].port_nettask, errno, strerror(errno));
	}
	//
	return 0;
}

int SimLinkReportRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	//char seri_emu[12]  = {0};
	//char seri_bank[12] = {0};
	//char gwaddr[16] = {0};
	//char sbaddr[16] = {0};
	report_rsp_info_t *report_rsp;
	report_rsp = (report_rsp_info_t *)(buff_recv+PACKAGE_HEADER_LEN);
	
	
	zprintf(INFO,"[INFO]SimLinkReportRspHdl[00-%02d-%02d]: Link Report Rsp Succ", ntohs(report_rsp->bank_nbr), ntohs(report_rsp->slot_nbr));
	
	return 0;
}

int SimDataRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	int ret = 0;
	link_relation_t *plink_relation = NULL;
	char seri_emu[12]  = {0};
	char seri_bank[12] = {0};
	//msg_t msg;
	msg_header_t *phdr = NULL;
	unsigned short len = 0;
	//char gwaddr[16] = {0};
	//char sbaddr[16] = {0};
	//report_rsp_info_t *report_rsp;
	//unsigned short bank_nbr = 0;
	//unsigned short slot_nbr = 0;

	
	len = getLen(buff_recv);
	phdr = (msg_header_t *)(buff_recv+PACKAGE_HEADER_LEN);
	phdr->cmd = ntohs(phdr->cmd);
	phdr->data_len = ntohs(phdr->data_len);

	plink_relation = (link_relation_t *)(buff_recv+PACKAGE_HEADER_LEN+sizeof(msg_header_t));
	plink_relation->sb_usb_nbr = ntohs(plink_relation->sb_usb_nbr);
	plink_relation->sb_bank_nbr = ntohs(plink_relation->sb_bank_nbr);
	plink_relation->sb_slot_nbr = ntohs(plink_relation->sb_slot_nbr);
	plink_relation->gw_bank_nbr = ntohs(plink_relation->gw_bank_nbr);
	plink_relation->gw_slot_nbr = ntohs(plink_relation->gw_slot_nbr);
	
	memcpy(seri_bank, plink_relation->sb_seri, SERIALNO_LEN);
	memcpy(seri_emu, plink_relation->gw_seri, SERIALNO_LEN);
	
	
	/*zprintf("[INFO]SimDataRspHdl[00-%02d-%02d]: Receive SimDataRsp[len:%d][gw:%s-00-%02d-%02d -- sb:gw:%s-%02d-%02d-%02d] Succ", \
		report_rsp->bank_nbr, report_rsp->slot_nbr, len, \
		seri_emu, plink_relation->gw_bank_nbr, plink_relation->gw_slot_nbr, \
		seri_bank, plink_relation->sb_usb_nbr, plink_relation->sb_bank_nbr, plink_relation->sb_sim_nbr);*/
	//logDataToHex((unsigned char *)buff_recv, PACKAGE_HEADER_LEN+len, 0, plink_relation->gw_bank_nbr, plink_relation->gw_slot_nbr, DIRE_SIMHDLTASK_TO_SLOTHDLTASK);
	
	// send link info to SlotHdlTask
	ret = csock->WriteData(csock->m_sockfd, buff_recv+PACKAGE_HEADER_LEN, len, gSlotHdlTaskParam[plink_relation->gw_bank_nbr][plink_relation->gw_slot_nbr].local_ip, gSlotHdlTaskParam[plink_relation->gw_bank_nbr][plink_relation->gw_slot_nbr].port_nettask);
	if ((unsigned short)ret != len)
	{
		zprintf(ERROR,"[ERRO]SimDataRspHdl[00-%02d-%02d]: Write SimDataRsp To SlotHdlTask[%s:%d] Error(%d:%s)\n", \
			plink_relation->gw_bank_nbr, plink_relation->gw_slot_nbr, \
			gSlotHdlTaskParam[plink_relation->gw_bank_nbr][plink_relation->gw_slot_nbr].local_ip, gSlotHdlTaskParam[plink_relation->gw_bank_nbr][plink_relation->gw_slot_nbr].port_nettask, errno, strerror(errno));
	}
	else
	{
		//logDataToHex((unsigned char *)buff_recv+PACKAGE_HEADER_LEN, ret, 0, plink_relation->gw_bank_nbr, plink_relation->gw_slot_nbr, DIRE_SIMHDLTASK_TO_SLOTHDLTASK);
	}
	//
	return 0;
}

int SimPullPlugNoticeHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	int ret = 0;
	sim_pullplug_info_t *psppi = NULL;
	char sb_seri[12]  = {0};
	char gw_seri[12] = {0};
	unsigned short event = 0;
	msg_t msg;

	
	//len = getLen(buff_recv);
	psppi = (sim_pullplug_info_t *)(buff_recv+PACKAGE_HEADER_LEN);
	memcpy(sb_seri, psppi->sb_seri, SERIALNO_LEN);
	psppi->sb_usb_nbr = ntohs(psppi->sb_usb_nbr);
	psppi->sb_bank_nbr = ntohs(psppi->sb_bank_nbr);
	psppi->sb_slot_nbr = ntohs(psppi->sb_slot_nbr);
	memcpy(gw_seri, psppi->gw_seri, SERIALNO_LEN);
	psppi->gw_bank_nbr = ntohs(psppi->gw_bank_nbr);
	psppi->gw_slot_nbr = ntohs(psppi->gw_slot_nbr);
	if (getLen(buff_recv) > sizeof(sim_pullplug_info_t)){
		event = ntohs(*((unsigned short*)(buff_recv + PACKAGE_HEADER_LEN + sizeof(sim_pullplug_info_t))));
		if (event == 1){
			zprintf(WARN, "gateway only receive pull event, plug event is error!");
			return 0;
		}
	}
	
	memset((char *)&msg, 0, sizeof(msg));
	msg.header.cmd = SIM_PULLPLUG_NOTICE_REQ;
	msg.header.data_len = sizeof(sim_pullplug_info_t);
	memcpy(msg.data, (char *)psppi, msg.header.data_len);
	
	
	
	zprintf(INFO,"[INFO]SimDataRspHdl[00-%02d-%02d]: Receive SimPullPlugNotice[sb:%s--%02d-%02d-%02d] Succ", \
		psppi->gw_bank_nbr, psppi->gw_slot_nbr, sb_seri, psppi->sb_usb_nbr, psppi->sb_bank_nbr, psppi->sb_slot_nbr);
	
	// send link info to SlotHdlTask
	ret = csock->WriteData(csock->m_sockfd, (char *)&msg, sizeof(msg_header_t)+msg.header.data_len, gSlotHdlTaskParam[psppi->gw_bank_nbr][psppi->gw_slot_nbr].local_ip, gSlotHdlTaskParam[psppi->gw_bank_nbr][psppi->gw_slot_nbr].port_nettask);
	if ((unsigned short)ret != sizeof(msg_header_t)+msg.header.data_len)
	{
		zprintf(ERROR,"[ERRO]SimDataRspHdl[00-%02d-%02d]: Write Link Report To SlotHdlTask[%s:%d] Error(%d:%s)\n", \
			psppi->gw_bank_nbr, psppi->gw_slot_nbr, \
			gSlotHdlTaskParam[psppi->gw_bank_nbr][psppi->gw_slot_nbr].local_ip, gSlotHdlTaskParam[psppi->gw_bank_nbr][psppi->gw_slot_nbr].port_nettask, errno, strerror(errno));
	}
	//
	return 0;
}

int SimSMSReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	sms_send_req_info_t *sms_req_info;
	char uuid[24] = {0};
	int ret;
	sms_req_info = (sms_send_req_info_t *)(buff_recv+PACKAGE_HEADER_LEN);
	unsigned short bank_nbr = ntohs(sms_req_info->gw_bank_nbr);
	unsigned short slot_nbr = ntohs(sms_req_info->gw_slot_nbr);

	if(bank_nbr >= MAX_BOARD || slot_nbr >= MAX_CHN){
		zprintf(ERROR,"[ERROR]SimSMSReqHdl:Invalid bank %d or slot %d ",bank_nbr,slot_nbr);
		return 0;
	}
	msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.header.cmd = SIMSMS_REQ;
	msg.header.data_len = sizeof(sms_send_req_info_t); 
	msg.header.slot = slot_nbr;
	memcpy(msg.data,(char *)sms_req_info, msg.header.data_len);
	if (getLen(buff_recv) > sizeof(sms_send_req_info_t)){
		memcpy(uuid, buff_recv + PACKAGE_HEADER_LEN + sizeof(sms_send_req_info_t), sizeof(uuid));
		memcpy((char*)msg.data + sizeof(sms_send_req_info_t), uuid, sizeof(uuid));
		msg.header.data_len += sizeof(uuid);
	}
	ret = csock->WriteData(csock->getSocket(), (char *)&msg, sizeof(msg.header)+msg.header.data_len, gSlotHdlTaskParam[bank_nbr][slot_nbr].local_ip, gSlotHdlTaskParam[bank_nbr][slot_nbr].port_nettask);
	if ((unsigned short)ret != (sizeof(msg.header)+msg.header.data_len))
	{
		zprintf(ERROR,"[ERRO]SimLinkCreateReqHdl[00-%02d-%02d]: write link info to SlotHdlTask[%s:%d] error(%d:%s)\n", \
			bank_nbr, slot_nbr, gSlotHdlTaskParam[bank_nbr][slot_nbr].local_ip, \
			gSlotHdlTaskParam[bank_nbr][slot_nbr].port_nettask, errno, strerror(errno));
		return 0;
	}
	zprintf(INFO,"\033[32m[INFO]SimSMSReqHdl:%s send sms, callee:%s uuid:%s send message:%s \033[0m",\
		sms_req_info->seri, sms_req_info->dst_num, uuid, sms_req_info->send_msg);
	
	return 0;
}

int SIMHangUpReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{	
	if(NULL == buff_recv || NULL == csock)
	{
		zprintf(ERROR,"[ERRO]SIMHangUpReqHdl: get NUll point");
		return 0;
	}

	int ret = 0;
	int len = getLen(buff_recv);
	sim_hang_up_req_t *sim_hang_up_req = (sim_hang_up_req_t *)(buff_recv+PACKAGE_HEADER_LEN);
	msg_header_t msg;
	memset(&msg,0,sizeof(msg_header_t));
	msg.cmd = CHAN_HANG_UP_REQ;
	msg.bank = ntohs(sim_hang_up_req->gw_bank_nbr);
	msg.slot = ntohs(sim_hang_up_req->gw_slot_nbr);
	
	if(len != sizeof(sim_hang_up_req_t))
	{
		zprintf(WARN,"[WARN]SIMHangUpReqHdl:lenth %d error",len);
		return 0;
	}
	
	ret = csock->WriteData(csock->m_sockfd, (char *)&msg, sizeof(msg_header_t), gSlotHdlTaskParam[msg.bank][msg.slot].local_ip, gSlotHdlTaskParam[msg.bank][msg.slot].port_nettask);
	if(ret != sizeof(msg_header_t))
	{
		zprintf(ERROR,"[ERRO]SIMHangUpReqHdl[00-%02d-%02d]: write link info to SlotHdlTask error(%d:%s)\n", \
			msg.slot, msg.slot,errno, strerror(errno));
		return 0;
	}
	return 0;
}

static int get_buff_from_popen(char *outbuff, char *cmd)
{
    FILE *fp;
    char buffer[40] = {0};

    if(!cmd) {
        return -1;
    }
	if (!outbuff){
		popen(cmd, "r");
		return 0;
	}

    fp = popen(cmd, "r");
    if (!fgets(buffer, sizeof(buffer), fp))
    {
		pclose(fp);
		return -1;
    }
    memcpy(outbuff, buffer, strlen(buffer));
    pclose(fp);                                                                                                                                                   
    return 0;
}

int get_file_info(char* filename, char* result_buff, unsigned long result_len)
{
	if (!filename || !result_buff){
		return -1;
	}
	
	FILE *fp = fopen(filename, "r");
	if (fp == NULL){
		return -1;
	}
	fread(result_buff, 1, result_len, fp);
	fclose(fp);
	return 0;
}

static int moduleLimitAll(unsigned short chan, unsigned char limit_stat){
	int res = 0;
	char limit_cmd[128] = {0};
	char outbuff[128] = {0};
	//set internet limit 
	setRedisofChannelInterStat(chan, limit_stat);

	//set call limit 
	sprintf(limit_cmd, "%s \"%s %d %d\"", AST_CLI, GSM_SET_CALLLIMIT, chan, limit_stat);
	if (get_buff_from_popen(outbuff, limit_cmd) == -1){
		zprintf(ERROR,"moduleLimitAll error : popen cmd error chan %d limit_stat %d!", chan, limit_stat);
		res = -2;
	}
	if (!strstr(outbuff, "success")){
		zprintf(WARN,"moduleLimitAll WARN : run calllimit cmd error chan %d limit_stat %d!", chan, limit_stat);
		res = -2;
	}
	
	//set sms limit
	sprintf(limit_cmd, "%s \"%s %d %d\"", AST_CLI, GSM_SET_SMSLIMIT, chan, limit_stat);
	get_buff_from_popen(outbuff, limit_cmd);
	
	if (get_buff_from_popen(outbuff, limit_cmd) == -1){
		zprintf(ERROR,"moduleLimitAll error : popen cmd error chan %d limit_stat %d!", chan, limit_stat);
		res = -2;
	}
	if (!strstr(outbuff, "success")){
		zprintf(WARN,"moduleLimitAll WARN : run smslimit cmd error chan %d limit_stat %d!", chan, limit_stat);
		res = -3;
	}
	return res;
		
}

static int getModuleLimitStat(unsigned short chan, char* limit){
	//cat /tmp/gsm/1
	char shell_cmd[32] = {0};
	char result_buff[128];
	int len = 0;
	unsigned char inter_stat;
	// /tmp/gsm/$chan_num file with some delay
	Sleep(1500);
	
	if (limit == NULL){
		zprintf(ERROR, "getLimitStat ptr limit is null!");
		return -1;
	}
	snprintf(shell_cmd, sizeof(shell_cmd), "cat /tmp/gsm/%d | grep Limit", chan);
	if (get_buff_from_popen(result_buff, shell_cmd) != 0){
		zprintf(ERROR, "getLimitStat error, popen error!");
		return -1;
	}
	if (strstr(result_buff, "Call")){
		len += sprintf(limit, "\"call\",");
	}
	if (strstr(result_buff, "Sms")){
		len += sprintf(limit + len, "\"sms\",");
	}
	if (getChannelInterStatFromRedis(chan, &inter_stat) == 0){
		if (inter_stat == MODULE_LIMIT){
			len += sprintf(limit + len, "\"internet\",");
		}
	}
	limit[len - 1] = '\0';
	return 0;
}

int ModuleLimitReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{	
	int ret = 0;
	if(NULL == buff_recv || NULL == csock)
	{
		zprintf(ERROR,"[ERRO]ModuleLimitReqHdl: get NUll point");
		return 0;
	}
	
	module_limit_t *module_limit = (module_limit_t *)(buff_recv+PACKAGE_HEADER_LEN);
	module_limit->bank = ntohs(module_limit->bank);
	module_limit->slot = ntohs(module_limit->slot);
	msg_t msg;
	memset(&msg,0,sizeof(msg_header_t));
	msg.header.cmd = SET_MODULE_LIMIT;
	msg.header.data_len = sizeof(module_limit_t);
	msg.header.bank = module_limit->bank;
	msg.header.slot = module_limit->slot;
	memcpy(msg.data, (char *)module_limit, sizeof(module_limit_t));
	zprintf(INFO,"[WARN]ModuleLimitReqHdl:bank %d slot %d seri %s",module_limit->bank, module_limit->slot, module_limit->seri);
	ret = csock->WriteData(csock->m_sockfd, (char *)&msg, sizeof(msg.header) + msg.header.data_len, gSlotHdlTaskParam[msg.header.bank][msg.header.slot].local_ip, gSlotHdlTaskParam[msg.header.bank][msg.header.slot].port_nettask);
	if(ret != (int)(sizeof(msg.header) + msg.header.data_len))
	{
		zprintf(ERROR,"[ERRO]ModuleLimitReqHdl[00-%02d-%02d]: write link info to SlotHdlTask error(%d:%s)\n", \
			msg.header.slot, msg.header.slot,errno, strerror(errno));
		return 0;
	}
	return 0;
}

int ModuleInterReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{	
	int ret = 0;
	if(NULL == buff_recv || NULL == csock){
		zprintf(ERROR,"[ERRO]ModuleInterReqHdl: get NUll point");
		return 0;
	}
	if (!fopen((char*)INTER_DIALUP_CLI, "r") || 
		!fopen((char*)INTER_SURF_CLI, "r") || 
		!fopen((char*)INTER_END_CLI, "r")){
		zprintf(ERROR, "[ERRO]ModuleInterReqHdl: module limit shell file is not exist");
		return 0;
	}
	
	module_internet_info_t *inter_info = (module_internet_info_t *)(buff_recv+PACKAGE_HEADER_LEN);
	inter_info->bank = ntohs(inter_info->bank);
	inter_info->slot = ntohs(inter_info->slot);
	inter_info->presize = ntohl(inter_info->presize);
	msg_t msg;
	memset(&msg,0,sizeof(msg_header_t));
	msg.header.cmd = SET_MODULE_INTERNET;
	msg.header.data_len = sizeof(module_internet_info_t);
	msg.header.bank = inter_info->bank;
	msg.header.slot = inter_info->slot;
	memcpy(msg.data, (char *)inter_info, sizeof(module_internet_info_t));
	
	zprintf(INFO,"ModuleInterReqHdl bank %d slot %d\n", msg.header.bank, msg.header.slot);
	ret = csock->WriteData(csock->m_sockfd, (char *)&msg, sizeof(msg.header) + msg.header.data_len, gSlotHdlTaskParam[msg.header.bank][msg.header.slot].local_ip, gSlotHdlTaskParam[msg.header.bank][msg.header.slot].port_nettask);
	if(ret != (int)(sizeof(msg.header) + (int)msg.header.data_len))
	{
		zprintf(ERROR,"[ERRO]ModuleInterReqHdl[00-%02d-%02d]: write link info to SlotHdlTask error(%d:%s)\n", \
			msg.header.slot, msg.header.slot,errno, strerror(errno));
		return 0;
	}
	return 0;
}

int SendUssdReqHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{	
	int ret = 0;
	if(NULL == buff_recv || NULL == csock)
	{
		zprintf(ERROR,"[ERRO]SendUssdReqHdl: get NUll point");
		return 0;
	}
	
	ussd_info_t *ussd_info = (ussd_info_t *)(buff_recv+PACKAGE_HEADER_LEN);
	ussd_info->bank_nbr = ntohs(ussd_info->bank_nbr);
	ussd_info->slot_nbr = ntohs(ussd_info->slot_nbr);
	ussd_info->timeout  = ntohl(ussd_info->timeout);
	msg_t msg;
	memset(&msg,0,sizeof(msg_header_t));
	msg.header.cmd = SEND_USSD;
	msg.header.data_len = sizeof(ussd_info_t);
	msg.header.bank = ussd_info->bank_nbr;
	msg.header.slot = ussd_info->slot_nbr;
	memcpy(msg.data, (char *)ussd_info, sizeof(ussd_info_t));
	zprintf(INFO,"[WARN]SendUssdReqHdl:bank %d slot %d seri %s",ussd_info->bank_nbr, ussd_info->slot_nbr, ussd_info->seri);	

	ret = csock->WriteData(csock->m_sockfd, (char *)&msg, sizeof(msg.header) + msg.header.data_len, gSlotHdlTaskParam[msg.header.bank][msg.header.slot].local_ip, gSlotHdlTaskParam[msg.header.bank][msg.header.slot].port_nettask);
	if(ret != (int)(sizeof(msg.header) + msg.header.data_len))
	{
		zprintf(ERROR,"[ERRO]SendUssdReqHdl[%02d-%02d]: write link info to SlotHdlTask error(%d:%s)\n", \
			msg.header.slot, msg.header.slot,errno, strerror(errno));
		return 0;
	}
	return 0;
}

static int set_sms_mode(unsigned short chan, unsigned char mode)
{
	char shell_cmd[128] = {0};
	char result_cmd[128] = {0};
	
	snprintf(shell_cmd, sizeof(shell_cmd), "asterisk -rx \"gsm set send sms mode %s %d\"", mode ? (char*)"text" : (char*)"pdu", chan);
	if (get_buff_from_popen(result_cmd, shell_cmd) != 0){
		zprintf(ERROR, "%s error, popen error!", __FUNCTION__);
		return -1;
	}

	zprintf(INFO, "%s info : chan %d set sms mode %d, result info %s", __FUNCTION__, chan, mode, result_cmd);
	return 0;
}


int ModuleSetSmsModeHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{	
	unsigned short chnl = 0;
	if(NULL == buff_recv || NULL == csock)
	{
		zprintf(ERROR,"[ERRO]: get NUll point", __FUNCTION__);
		return 0;
	}
	sms_mode_info_t *ptrSMSModeInfo = (sms_mode_info_t *)(buff_recv+PACKAGE_HEADER_LEN);
	chnl = ntohs(ptrSMSModeInfo->chnl_nbr);

	//cli
	set_sms_mode(chnl, ptrSMSModeInfo->mode);
	return 0;
}




/**************************************************************************** 
* 函数名称 : EmuSimReleaseRspHdl
* 功能描述 : sim卡释放响应处理， 目前关闭SimEmuSvr.exe即可，因改成启动后先设置sim Atr,需要重新注册时，重启程序即可，
*            需要连接其他SimRdrSvr服务器时，先关闭程序，修改配置，然后重启程序即可。
* 参    数 : char *buff_recv			: 接收缓冲，存储接收报文内容
* 参    数 : char *buff_send			: 发送缓冲
* 参    数 : CSocketEx *csock			: 通讯socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int EmuUnRegisterRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	return 0;
}

int CallMoringHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{	
	call_moring_info_t call_moring_info;
	unsigned short len = 0;
	unsigned short board_nbr = 0;
	unsigned short slot_nbr = 0;
	char calldst[16] = {0};
	int ret = 0;
	

	// get slot_nbr, calldst
	board_nbr = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[0]);
	slot_nbr  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[1]);
	zprintf(INFO,"[INFO]CallMoringHdl[00-%02d-%02d]:Call Moring...", board_nbr, slot_nbr);

	if(0 == gSlotHdlTaskParam[board_nbr][slot_nbr].slotinfo.link_stat){
		return 0;
	}
	
	// make package that send to proxysvr
	call_moring_info.type = htons(SimEmuSvr);
	memcpy(call_moring_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	call_moring_info.bank_nbr = htons(board_nbr);
	call_moring_info.slot_nbr = htons(slot_nbr);
	memcpy(call_moring_info.calldst, gSimEmuSvrParam.local_ip, sizeof(calldst));
	len = makeReqPackage(CALL_MORING_NOTICE, (char *)&call_moring_info, sizeof(call_moring_info), buff_send);
	
	ret = csock->WriteData(csock->getSocket(), buff_send, len);
	if (ret != len)
	{
		zprintf(ERROR,"[ERRO]CallMoringHdl[00-%02d-%02d]: send call moring[calldst:%s] info to ProxySvr error(%d:%s)", board_nbr, slot_nbr, calldst, errno, strerror(errno));
	}
	
	gSlotHdlTaskParam[board_nbr][slot_nbr].call_flag = 1;
	return 0;
}

int CallBeginHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	call_begin_info_t call_begin_info;
	unsigned short len = 0;
	unsigned short board_nbr = 0;
	unsigned short slot_nbr = 0;
	char calldst[16] = {0};
	int ret = 0;
	

	// get slot_nbr, calldst
	board_nbr = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[0]);
	slot_nbr  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[1]);
	zprintf(INFO,"[INFO]CallBeginHdl[00-%02d-%02d]: Call Begin...", board_nbr, slot_nbr);
	
	if(0 == gSlotHdlTaskParam[board_nbr][slot_nbr].slotinfo.link_stat){
		return 0;
	}
	
	// make package that send to proxysvr
	call_begin_info.type = htons(SimEmuSvr);
	memcpy(call_begin_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	call_begin_info.bank_nbr = htons(board_nbr);
	call_begin_info.slot_nbr = htons(slot_nbr);
	memcpy(call_begin_info.calldst, gSimEmuSvrParam.local_ip, sizeof(calldst));
	len = makeReqPackage(CALL_BEGIN_NOTICE, (char *)&call_begin_info, sizeof(call_begin_info), buff_send);
	ret = csock->WriteData(csock->getSocket(), buff_send, len);
	if (ret != len)
	{
		zprintf(ERROR,"[ERRO]CallBeginHdl[00-%02d-%02d]: send call begin[calldst:%s] info to ProxySvr error(%d:%s)", board_nbr, slot_nbr, calldst, errno, strerror(errno));
	}
	
	gSlotHdlTaskParam[board_nbr][slot_nbr].call_begin_time = time(NULL);
	gSlotHdlTaskParam[board_nbr][slot_nbr].call_flag = 1;
	return 0;
}
int CallEndHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	call_end_info_t call_end_info;
	unsigned short len = 0;
	unsigned short board_nbr = 0;
	unsigned short slot_nbr = 0;
	int ret = 0;

	// get slot_nbr, calldst
	board_nbr = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[0]);
	slot_nbr  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[1]);
	zprintf(INFO,"[INFO]CallEndHdl[00-%02d-%02d]: Call End...", board_nbr, slot_nbr);

	
	// make package that send to proxysvr
	call_end_info.type = htons(SimEmuSvr);
	memcpy(call_end_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	call_end_info.bank_nbr = htons(board_nbr);
	call_end_info.slot_nbr = htons(slot_nbr);
	len = makeReqPackage(CALL_END_NOTICE, (char *)&call_end_info, sizeof(call_end_info), buff_send);
	
	ret = csock->WriteData(csock->getSocket(), buff_send, len);
	if (ret != len)
	{
		zprintf(ERROR,"[ERRO]CallEndHdl[00-%02d-%02d]: send call end info to ProxySvr error(%d:%s)", board_nbr, slot_nbr, errno, strerror(errno));
	}

	gSlotHdlTaskParam[board_nbr][slot_nbr].call_flag = 0;
	if(gSlotHdlTaskParam[board_nbr][slot_nbr].call_begin_time > 0){
		gSlotHdlTaskParam[board_nbr][slot_nbr].call_rest_time -= (time(NULL) - gSlotHdlTaskParam[board_nbr][slot_nbr].call_begin_time)/60 + 1;
		gSlotHdlTaskParam[board_nbr][slot_nbr].call_begin_time = 0;
	}
	zprintf(INFO,"[INFO]CallEndHdl[00-%02d-%02d]: has %d miniutes calling time",board_nbr, slot_nbr,gSlotHdlTaskParam[board_nbr][slot_nbr].call_rest_time);
	return 0;
}

static int SendSMSEvent(char *buff_recv, char *buff_send, CSocketEx *csock, unsigned short event)
{
	char seri[SERIALNO_LEN] = {0};
	sms_event_t sms_event;
	sms_result_info_t sms_result_info;
	unsigned short len = 0;
	int ret = 0;
	int offset = 0;

	memset((char*)&sms_event, 0, sizeof(sms_event_t));
	// get event info
	memcpy(seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	sms_event.board_nbr = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[offset++]);
	sms_event.slot_nbr  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[offset++]);
	if (getLen(buff_recv) > 4){
		sms_event.state  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[offset++]);
		offset *= 2;
		sms_event.phone_len = (buff_recv+PACKAGE_HEADER_LEN)[offset++];
		if (sms_event.phone_len > 0) {
			strncpy(sms_event.phone, buff_recv + PACKAGE_HEADER_LEN + offset, sms_event.phone_len);
			memcpy(sms_result_info.phone, sms_event.phone, sizeof(sms_result_info.phone));
			offset += sms_event.phone_len;
		}
		if (event == SMS_RESULT_NOTICE){
			sms_event.uuid_len = ((unsigned char *)(buff_recv+PACKAGE_HEADER_LEN))[offset++];
			if (sms_event.uuid_len > 0) {
				strncpy(sms_event.uuid, buff_recv + PACKAGE_HEADER_LEN + offset, sms_event.uuid_len);
				memcpy(sms_result_info.uuid, sms_event.uuid, sizeof(sms_result_info.uuid));
				offset += sms_event.uuid_len;
			}
		}
	}
	
	sms_result_info.type = htons(SimEmuSvr);
	memcpy(sms_result_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	sms_result_info.bank_nbr = htons(sms_event.board_nbr);
	sms_result_info.slot_nbr = htons(sms_event.slot_nbr);
	sms_result_info.stat = sms_event.state;
	// make package that send to proxysvr
	len = makeReqPackage(event, (char *)&sms_result_info, sizeof(sms_result_info_t), buff_send);
	ret = csock->WriteData(csock->getSocket(), buff_send, len);
	if (ret != len)
	{
		zprintf(ERROR,"[ERRO]SmsResultHdl[00-%02d-%02d]: send sms evnet info to ProxySvr error(%d:%s)", sms_event.board_nbr, sms_event.slot_nbr, errno, strerror(errno));
	}
	
	zprintf(INFO,"[INFO]SmsResultHdl[00-%02d-%02d]: sms event %d state %s", sms_event.board_nbr, sms_event.slot_nbr, event, sms_event.state ? EVENT_STR_SUCCESS : EVENT_STR_FAIL);
	return 0;
}


int SmsResultHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	if (buff_recv == NULL || buff_send == NULL || csock == NULL){
		zprintf(ERROR,"[ERRO]SmsResultHdl:sms result error");
		return 0;
	}
	SendSMSEvent(buff_recv, buff_send, csock, SMS_RESULT_NOTICE);
	return 0;
}

int SmsReportHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	if (buff_recv == NULL || buff_send == NULL || csock == NULL){
		zprintf(ERROR,"[ERRO]SmsReportHdl:sms report error");
		return 0;
	}
	SendSMSEvent(buff_recv, buff_send, csock, SMS_REPORT_NOTICE);
	return 0;
}

int UssdResultHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	unsigned short len = 0;
	ussd_event_t ussd_event;
	char seri[SERIALNO_LEN] = {0};
	
	int ret = 0;
	char send_buf[2048] = {0};
	
	memset((char*)&ussd_event, 0, sizeof(ussd_event_t));
	// get slot_nbr
	memcpy(seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	ussd_event.board_nbr = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[0]);
	ussd_event.slot_nbr  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[1]);
	ussd_event.state = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[2]);
	ussd_event.ussd_stat = ((unsigned char *)(buff_recv+PACKAGE_HEADER_LEN))[6];
	ussd_event.ussd_coding = ((unsigned char *)(buff_recv+PACKAGE_HEADER_LEN))[7];
	
	//if state success, get ussd result
	if (ussd_event.state == EVENT_SUCCESS){
		ussd_event.ussd_len  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[4]);
		strncpy(ussd_event.ussd_info, buff_recv + PACKAGE_HEADER_LEN + 10, ussd_event.ussd_len);
		ussd_event.uuid_len = ((unsigned char *)(buff_recv+PACKAGE_HEADER_LEN))[10 + ussd_event.ussd_len];
		strncpy(ussd_event.uuid, buff_recv + PACKAGE_HEADER_LEN + 10 + ussd_event.ussd_len + 1, ussd_event.uuid_len);
		snprintf(send_buf, sizeof(send_buf), "{\"type\":\"%s\",\"seri\":\"%s\",\"chan\":\"%d\",\"stat\":\"%s\",\"ussd_stat\":\"%d\",\"ussd_coding\":\"%d\",\"result\":\"%s\",\"uuid\":\"%s\"}", 
				JOSN_TYPE_USSDRES, seri, TRANSFORM(ussd_event.board_nbr, ussd_event.slot_nbr), ussd_event.state ? "success" : "fail", 
				ussd_event.ussd_stat, ussd_event.ussd_coding, ussd_event.ussd_info, ussd_event.uuid);
	}
	else{
		ussd_event.uuid_len = ((unsigned char *)(buff_recv+PACKAGE_HEADER_LEN))[8];
		strncpy(ussd_event.uuid, buff_recv + PACKAGE_HEADER_LEN + 8 + 1, ussd_event.uuid_len);
		snprintf(send_buf, sizeof(send_buf), "{\"type\":\"%s\",\"seri\":\"%s\",\"chan\":\"%d\",\"stat\":\"%s\",\"uuid\":\"%s\"}", 
				JOSN_TYPE_USSDRES, seri, TRANSFORM(ussd_event.board_nbr, ussd_event.slot_nbr), 
				ussd_event.state ? "success" : "fail", ussd_event.uuid);
	}
	zprintf(INFO,"[INFO]UssdResultHdl[%02d-%02d]: state %d coding %d ussd result %s uuid %s", 
					ussd_event.board_nbr, ussd_event.slot_nbr, ussd_event.state, ussd_event.ussd_coding, 
					ussd_event.ussd_info, ussd_event.uuid);

	// make package that send to proxysvr
	len = makeReqPackage(USSD_RESULT_NOTICE, (char *)&send_buf, strlen(send_buf) + 1, buff_send);	
	ret = csock->WriteData(csock->getSocket(), buff_send, len);
	if (ret != len)
	{
		zprintf(ERROR,"[ERRO]UssdResultHdl[%02d-%02d]: send ussd result info to ProxySvr error(%d:%s)", ussd_event.board_nbr, ussd_event.slot_nbr, errno, strerror(errno));
	}
	return 0;
}


/**************************************************************************** 
* 函数名称 : findNetHandleFunc
* 功能描述 : 获取指定报文类型的处理函数
* 参    数 : unsigned short cmd		: 报文命令
* 返 回 值 : 成功返回函数指针，失败返回NULL
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
NET_HANDLE_FUNC findNetHandleFunc(unsigned short cmd)
{
	unsigned int i = 0;
	
	while (i < (sizeof(gNetHandles) / sizeof(net_handle_t)))
	{
		if (cmd == gNetHandles[i].cmd)
		{
			return gNetHandles[i].cmd_handle;
		}
		i++;
	}
	return NULL;
}

/**************************************************************************** 
* 函数名称 : findAstHandleFunc
* 功能描述 : 获取指定报文类型的处理函数
* 参    数 : unsigned short cmd		: 报文命令
* 返 回 值 : 成功返回函数指针，失败返回NULL
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
AST_HANDLE_FUNC findAstHandleFunc(unsigned short cmd)
{
	unsigned int i = 0;
	
	while (i < (sizeof(gAstHandles) / sizeof(ast_handle_t)))
	{
		if (cmd == gAstHandles[i].cmd)
		{
			return gAstHandles[i].cmd_handle;
		}
		i++;
	}
	return NULL;
}




/**************************************************************************** 
* 函数名称 : readConfigValue
* 功能描述 : 读取配置文件参数内容
* 参    数 : void
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int readConfigValue(void)
{
	int ret = 0;
	char content[128] = {0};
	unsigned char mac_addr[6] = {0};
	void *handle = NULL;
	//int i = 0;
	//char key_name[64] = {0};
	CSocketEx csock;	

	if(0 != csock.getEthernetMac((char*)"eth0", mac_addr))
	{
		zprintf(ERROR,"getEthernetMac error");
		return -1;
	}
	base32_encode(mac_addr + 1,5,(unsigned char *)gSimEmuSvrParam.seri);
	
	md5_encode((unsigned char *)gSimEmuSvrParam.seri,(unsigned char *)gSimEmuSvrParam.passwd);
	
	ret = conf_init(SIMEMUSVR_CONFIG, &handle);
	if (ret != 0)
	{
		return -1;
	}

	//
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SIMEMUSVR_SWITCH, content);
	if (ret == 0)
	{
		a_trim(gSimEmuSvrParam.simemusvr_switch, content);
	}
	
	// seri
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SERI, content);
	if (ret == 0)
	{
		if(0 != strcmp(content,gSimEmuSvrParam.seri))
		{
			sprintf(content,"/my_tools/set_config %s set option_value SimEmuSvr seri %s",\
				SIMEMUSVR_CONFIG,gSimEmuSvrParam.seri);
			system(content);
		}
	}
	// local_ip
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_LOCAL_IP, content);
	if (ret == 0)
	{
		a_trim(gSimEmuSvrParam.local_ip, content);
	}
	// server_ip
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SERVER_IP, content);
	if (ret == 0)
	{
		a_trim(gSimEmuSvrParam.server_ip, content);
	}

	
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_BAUDRATE, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.baudrate = atoi(content);
	}
	conf_release(&handle);

	gSimEmuSvrParam.server_port = SERVER_PORT;
	gSimEmuSvrParam.hb_interval = HB_INTERVAL;
	gSimEmuSvrParam.slot_port_comm = SLOT_PORT_COMM;
	gSimEmuSvrParam.slot_port_net = SLOT_PORT_NET;
	gSimEmuSvrParam.comm_hdl_port = COMM_HDL_PORT;
	gSimEmuSvrParam.ast_hdl_port = AST_HDL_PORT;
	gSimEmuSvrParam.net_hdl_port = NET_HDL_PORT;
	zprintf(INFO,"[INFO]SimEmuSvr param: switch:%s, seri:%s, local_ip:%s, server_ip:%s, server_port:%d, heartbeat_interval:%d, baudrate:%d, comm:%s, slot_port_comm:%d, slot_port_net:%d, comm_hdl_port:%d, net_hdl_port:%d, sim_data_out_port:%d, sim_data_in_port:%d, ast_hdl_port:%d", \
		gSimEmuSvrParam.simemusvr_switch, gSimEmuSvrParam.seri, gSimEmuSvrParam.local_ip, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port, gSimEmuSvrParam.hb_interval, gSimEmuSvrParam.baudrate, gSimEmuSvrParam.comm, \
		gSimEmuSvrParam.slot_port_comm, gSimEmuSvrParam.slot_port_net, gSimEmuSvrParam.comm_hdl_port, gSimEmuSvrParam.net_hdl_port, gSimEmuSvrParam.sim_data_out_port, gSimEmuSvrParam.sim_data_in_port, gSimEmuSvrParam.ast_hdl_port);
	
	return 0;
}

int readLogConfValue()
{
	int ret = 0;
	char content[128];
	void *handle = NULL;
	//int i = 0;
	//char key_name[64] = {0};
	
	ret = conf_init(LOGFILE_CONFIG, &handle);
	if (ret != 0)
	{
		return -1;
	}
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_APDU_LOG_SWITCH, content);
	if (ret == 0){
		apdu_switch = atoi(content);
	}else{
		apdu_switch = 0;
	}

	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_LOG_CLASS, content);
	if (ret == 0){
		log_class = atoi(content);
	}else{
		log_class = 0;
	}
	conf_release(&handle);
	zprintf(INFO,"[INFO]Log class = %d,apdu log switch = %d",log_class,apdu_switch);
	return 0;
}

/**************************************************************************** 
* 函数名称 : initRegisterStat
* 功能描述 : 初始化注册状态
* 参    数 : register_stat_t *stat		: 注册状态数据结构缓冲
* 返 回 值 : void
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void initRegisterStat(register_stat_t *stat)
{
	stat->stat = 0;
	pthread_mutex_init(&stat->lock, NULL);
}

/**************************************************************************** 
* 函数名称 : upRegisterStat
* 功能描述 : 拉起注册状态
* 参    数 : register_stat_t *stat		: 注册状态数据结构缓冲
* 返 回 值 : void
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void upRegisterStat(register_stat_t *regstat)
{
	pthread_mutex_lock(&regstat->lock);
	regstat->stat = REGISTER_UP;
	pthread_mutex_unlock(&regstat->lock);
}

/**************************************************************************** 
* 函数名称 : downRegisterStat
* 功能描述 : 拉低注册状态
* 参    数 : register_stat_t *stat		: 注册状态数据结构缓冲
* 返 回 值 : void
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void downRegisterStat(register_stat_t *stat)
{
	pthread_mutex_lock(&stat->lock);
	stat->stat = REGISTER_DOWN;
	pthread_mutex_unlock(&stat->lock);
}

/**************************************************************************** 
* 函数名称 : getRegisterStat
* 功能描述 : 获取注册状态
* 参    数 : register_stat_t *stat		: 注册状态数据结构缓冲
* 返 回 值 : 注册状态
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getRegisterStat(register_stat_t *stat)
{
	int stat_;
	pthread_mutex_lock(&stat->lock);
	stat_ = stat->stat;
	pthread_mutex_unlock(&stat->lock);
	return stat_;
}


/**************************************************************************** 
* 函数名称 : emuRegister
* 功能描述 : Emu注册函数
* 参    数 : CSocketEx *sock			: 通许socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int emuRegister(CSocketEx *csock)
{
	char buff[128] = {0};
	char context[8] = {0};
	int sys_type = 0,chan_count = 0;
	int len = 0;
	int ret = 0;
	register_req_info_t register_req_info;
	
	memset(buff, 0, sizeof(buff));
	memset((char *)&register_req_info, 0, sizeof(register_req_info));

	get_config((char*)GSM_INFO_FILE, (char*)"sys", (char*)"sys_type", context);
	sys_type = atoi(context);
	if(1 == sys_type)
	{
		memset(context, 0, sizeof(context));
		get_config((char*)GSM_INFO_FILE, (char*)"sys", (char*)"total_chan_count", context);
		chan_count = atoi(context);
	}else{
		zprintf(M_ERROR,"[ERROR]Unsupported device type");
		return -1;
	}

	if(0 <= chan_count && chan_count  <= 16){
		strcpy(register_req_info.model_name,SWG_2016);
	}else if(16 < chan_count && chan_count <= 32){
		strcpy(register_req_info.model_name,SWG_2032);
	}else{
		zprintf(M_ERROR,"[ERROR]Unsupported device type");
		exit(-1);
	}
	
	register_req_info.slot_num = htons(chan_count);
	register_req_info.type = htons(SimEmuSvr);
	memcpy(register_req_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	memcpy(register_req_info.passwd, gSimEmuSvrParam.passwd, sizeof(gSimEmuSvrParam.passwd));
	makeRegisterPack(buff, (char *)&register_req_info, sizeof(register_req_info), &len);

	// connect to server
	csock->Init(PROTO_TCP, gSimEmuSvrParam.local_ip, gSimEmuSvrParam.server_port, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port, 1);
	csock->CreateSocket();
	if (csock->Bind() == false)
	{
		return Error;
	}
	ret = csock->ConnectNonB(csock->getSocket(), 10);
	if (!ret)
	{
		csock->CloseSocket();
		zprintf(WARN,"[WARN]emuRegister: connect[%s:%d] to Server[%s:%d] error(%d:%s)", gSimEmuSvrParam.local_ip, gSimEmuSvrParam.server_port, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port, errno, strerror(errno));
		return Error;
	}
	zprintf(INFO,"[INFO]emuRegister: connect[%s:%d] to Server[%s:%d] succ", gSimEmuSvrParam.local_ip, gSimEmuSvrParam.server_port, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port);


	// send req pack
	ret = csock->WriteData(csock->getSocket(), buff, len);
	if (ret != len)
	{
		csock->CloseSocket();
		zprintf(ERROR,"[ERRO]emuRegister: send register request error(%d:%s)", errno, strerror(errno));
		return Error;
	}
	// recv rsp pack headr
	memset(buff, 0, sizeof(buff));
	if (!csock->checkSocketReady(csock->getSocket(), READ_WAIT, 10))
	{
		csock->CloseSocket();
		zprintf(ERROR,"[ERRO]emuRegister: register no rsp yet");
		return Error;
	}
	ret = csock->TCPReadData(csock->getSocket(), buff, PACKAGE_HEADER_LEN);
	if (ret != PACKAGE_HEADER_LEN)
	{
		csock->CloseSocket();
		zprintf(ERROR,"[ERRO]emuRegister: recv register response header error(%d:%s)", errno, strerror(errno));
		return Error;
	}
	len = getLen(buff);
// 	if (len <= 0)
// 	{
// 		printf("emuRegister: package header'len item error\n");
// 		return Error;
// 	}
	// recv rsp  pack body
	ret = csock->TCPReadData(csock->getSocket(), buff+PACKAGE_HEADER_LEN, len+2);
	if (ret != (len+2))
	{
		csock->CloseSocket();
		zprintf(ERROR,"[ERRO]emuRegister: recv register response body error(%d:%s)", errno, strerror(errno));
		return Error;
	}

	// check rsp pack
	ret = checksum((unsigned short *)buff, PACKAGE_HEADER_LEN+len+2);
	if (!ret)
	{
		csock->CloseSocket();
		zprintf(ERROR,"[ERRO]emuRegister: register response checksum fail\n,seri:%s,passwd:%s",register_req_info.seri,register_req_info.passwd);
		return Error;
	}
	unsigned short result = getResult(buff);
	if (result != 0x0000)
	{
		csock->CloseSocket();
		zprintf(ERROR,"[ERRO]emuRegister: the register result is fail");
		return Error;
	}
	last_seen = time(NULL);
	zprintf(INFO,"[INFO]emuRegister: SimEmuSvr[%s] register to %s:%d success", gSimEmuSvrParam.seri, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port);

	// return
	return NoErr;
}

/**************************************************************************** 
* 函数名称 : emuUnRegister
* 功能描述 : Emu注销函数
* 参    数 : CSocketEx *sock			: 通许socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int emuUnRegister(CSocketEx *sock)
{
	//
	char buff[1024];
	int len = 0;
	int ret = 0;
	unregister_req_info_t unregister_req_info;
	
	memset(buff, 0, sizeof(buff));
	memset((char *)&unregister_req_info, 0, sizeof(unregister_req_info));

	unregister_req_info.type = htons(SimEmuSvr);
	memcpy(unregister_req_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	unregister_req_info.slot_nbr = htons(SLOT_NBR);
	makeUnRegisterPack(buff, (char *)&unregister_req_info, sizeof(unregister_req_info), &len);
	
	// send req pack
	ret = sock->WriteData(sock->getSocket(), buff, len);
	if (ret != len)
	{
		zprintf(ERROR,"[ERRO]send unregister request error(%d:%s)", errno, strerror(errno));
		return Error;
	}
	// recv rsp pack headr
	memset(buff, 0, sizeof(buff));
	if (!sock->checkSocketReady(sock->getSocket(), READ_WAIT, 10))
	{
		zprintf(ERROR,"[ERRO]unregister no rsp yet");
		return Error;
	}
	ret = sock->TCPReadData(sock->getSocket(), buff, PACKAGE_HEADER_LEN);
	if (ret != PACKAGE_HEADER_LEN)
	{
		zprintf(ERROR,"[ERRO]recv unregister response header error(%d:%s)", errno, strerror(errno));
		return Error;
	}
	len = getLen(buff);
	// 	if (len <= 0)
	// 	{
	// 		printf("emuRegister: package header'len item error\n");
	// 		return Error;
	// 	}
	// recv rsp  pack body
	ret = sock->TCPReadData(sock->getSocket(), buff+PACKAGE_HEADER_LEN, len+2);
	if (ret != (len+2))
	{
		zprintf(ERROR,"[ERRO]emuUnRegister: recv unregister response body error(%d:%s)", errno, strerror(errno));
		return Error;
	}
	
	// check rsp pack
	ret = checksum((unsigned short *)buff, PACKAGE_HEADER_LEN+len+2);
	if (!ret)
	{
		zprintf(ERROR,"[ERRO]emuUnRegister: unregister response checksum fail");
		return Error;
	}
	unsigned short result = getResult(buff);
	if (result != 0x0000)
	{
		zprintf(ERROR,"[ERRO]emuUnRegister: the unregister result is fail");
		return Error;
	}
	zprintf(INFO,"[INFO]emuUnRegister: unregister success");
	
	// return
	return NoErr;


	//
	return 0;
}

/**************************************************************************** 
* 函数名称 : EmuHeartbeat
* 功能描述 : Emu心跳函数
* 参    数 : CSocketEx *sock			: 通许socket类
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int EmuHeartbeat(CSocketEx *sock)
{
	char buff[1024];
	int len = 0;
	int ret = 0;
	heartbeat_req_info_t hb_info;

	memset((char *)&hb_info, 0, sizeof(hb_info));
	hb_info.type = htons(SimEmuSvr);
	memcpy(hb_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	hb_info.bank_nbr = htons(EMU_BANK_NBR);
	hb_info.slot_nbr = htons(EMU_BANK_NBR * SLOT_NBR);

	// make heartbeat package
	makeHeartbeatPack(buff, (char *)&hb_info, sizeof(hb_info), &len);
	// send heartbeat package
	ret = sock->WriteData(sock->getSocket(), buff, len);
	if (ret != len)
	{
		zprintf(ERROR,"[ERRO]send heartbeat request error.(%d:%s)", errno, strerror(errno));
		return Error;
	}
	// return
	return NoErr;
}

int SMSPhoneNumStringMatch(char *dst,char *src,char *context)
{
	if(!dst || !src){
		return 0;
	}
	char *p = strstr(dst,src);
	int i = 0,j = 0;
	int find_step = 0;
	if(p)
	{
		i += strlen(src);
		while(i < (int)strlen(dst) && find_step < 2)
		{
			switch(find_step)
			{
				case 0:
					if( p[i] >= '0' && p[i] <= '9')
					{
						find_step = 1;
						break;
					}
					i++;
					break;
				case 1:
					if( (p[i] >= '0' && p[i] <= '9'))
					{
						context[j] = p[i];
						i++;
						j++;
					}
					else{
						find_step = 2;
					}
					break;
				case 2:
				default:
					break;
			}
		}
	}
	zprintf(INFO,"[INFO]\033[32m[INFO]SMSPhoneNumStringMatch:Match Message %s\033[0m",context);
	return j;
}


int SMSBanlanceStringMatch(char *dst,char *src,char *context)
{
	if(!dst || !src){
		return 0;
	}
	char *p = strstr(dst,src);
	int i = 0,j = 0;
	int find_step = 0;
	int point_index = MESSAGE_LEN;
	
	if(p)
	{
		i += strlen(src);
		while(i < (int)strlen(dst) && find_step < 3)
		{
			switch(find_step)
			{
				case 0:
					if( p[i] >= '0' && p[i] <= '9')
					{
						find_step = 1;
						break;
					}
					i++;
					break;
				case 1:
					if( (p[i] >= '0' && p[i] <= '9'))
					{
						context[j] = p[i];
						i++;
						j++;
					}
					else{
						if(p[i] == '.')
						{
							context[j] = p[i];
							i++;
							j++;
							point_index = j;
							find_step = 2;
						}
						else{
							find_step = 3;
						}
					}
					break;
				case 2:
					if( (p[i] >= '0' && p[i] <= '9'))
					{
						context[j] = p[i];
						i++;
						j++;
					}
					else
					{
						find_step = 3;
					}
					break;
				case 3:
				default:
					break;
			}
		}
	}
	zprintf(INFO,"\033[32m[INFO]SMSBanlanceStringMatch:Match Message %s\033[0m",context);
	return j > point_index ? j:0;
}

void delRedisofChannelSMSInfo(char *filed)
{
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	char rds_cmd[256] = {0};

	sprintf(rds_cmd,"hdel %s %s",REDIS_KEY_SIMBANK_SMS_INFO,filed);

	if (context->err)
	{
		zprintf(ERROR,"[ERRO]delRedisofChannelSMSInfo: redisConnect error(%s)",context->errstr);
	}
	else
	{
		redisReply *reply = (redisReply *)redisCommand(context, rds_cmd);
		if (reply->type == REDIS_REPLY_ERROR)
		{
			zprintf(ERROR,"[ERRO]delRedisofChannelSMSInfo: redisCommand(%s) error(%s)", \
				rds_cmd, reply->str);
		}
		else
		{
			zprintf(INFO,"[INFO]delRedisofChannelSMSInfo: redisCommand(%s) succ", \
				rds_cmd);
		}
		freeReplyObject((void *)reply);
	}
	redisFree(context);
}
void delRedisAllofChannelSMSInfo(int chan_no)
{
	char fileds1[32] = {0};
	char fileds2[32] = {0};
	char fileds3[32] = {0};

	sprintf(fileds1, "%d-recv-msg", chan_no);
	delRedisofChannelSMSInfo(fileds1);
	sprintf(fileds2, "%d-recv-num", chan_no);
	delRedisofChannelSMSInfo(fileds2);
	sprintf(fileds3, "%d-recv-time", chan_no);
	delRedisofChannelSMSInfo(fileds3);
}

static void setRedisofChannelUssdInfo(int chan_no, char* ussd_info, char* uuid, unsigned int timeout)
{
	char buf[256] = {0};
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);

	snprintf(buf, sizeof(buf), "{\"type\":\"asyncussd\",\"msg\":\"%s\",\"timeout\":\"%d\",\"id\":\"%s\"}", 
				ussd_info, timeout, uuid);
	
	if (context->err)
	{
		zprintf(ERROR,"[ERRO]setRedisofChannelUssdInfo: channel %d redisConnect error(%s)",chan_no, context->errstr);
	}
	else
	{
		redisReply *reply = (redisReply *)redisCommand(context, "rpush app.asterisk.async.ussdlist.%d %s", chan_no, buf);
		if (reply->type == REDIS_REPLY_ERROR)
		{
			zprintf(ERROR,"[ERRO]setRedisofChannelUssdInfo: redisCommand(rpush app.asterisk.async.ussdlist.%d %s) error(%s)", \
				chan_no, buf, reply->str);
		}
		else
		{
			zprintf(INFO,"[INFO]setRedisofChannelUssdInfo: redisCommand(rpush app.asterisk.async.ussdlist.%d %s) succ", \
				chan_no, buf);
		}
		freeReplyObject((void *)reply);
	}
	redisFree(context);
}

static void setRedisofChannelSMSInfo(unsigned short chan, char* callee, char* send_msg, char* uuid)
{
	char buf[256] = {0};
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);

	snprintf(buf, sizeof(buf), "{\"type\":\"asyncsms\",\"num\":\"%s\",\"msg\":\"%s\",\"flash\":\"0\",\"id\":\"%s\"}", 
				callee, send_msg, uuid);
	
	if (context->err)
	{
		zprintf(ERROR,"[ERRO]setRedisofChannelSMSInfo: channel %d redisConnect error(%s)",chan, context->errstr);
	}
	else
	{
		redisReply *reply = (redisReply *)redisCommand(context, "rpush app.asterisk.async.smslist.%d %s", chan, buf);
		if (reply->type == REDIS_REPLY_ERROR)
		{
			zprintf(ERROR,"[ERRO]setRedisofChannelSMSInfo: redisCommand(rpush app.asterisk.async.smslist.%d %s) error(%s)", \
				callee, buf, reply->str);
		}
		else
		{
			zprintf(INFO,"[INFO]setRedisofChannelSMSInfo: redisCommand(rpush app.asterisk.async.smslist.%d %s) succ", \
				callee, buf);
		}
		freeReplyObject((void *)reply);
	}
	redisFree(context);
}


static void setRedisofChannelInterStat(int chan_no,unsigned char inter_stat)
{
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err)
	{
		zprintf(ERROR,"[ERRO]setRedisofChannelInterStat: channel %d redisConnect error(%s)",chan_no, context->errstr);
	}
	else
	{
		redisReply *reply = (redisReply *)redisCommand(context, "hset app.module.dialup.limitstat.channel %d %d", chan_no, inter_stat);
		if (reply->type == REDIS_REPLY_ERROR)
		{
			zprintf(ERROR,"[ERRO]setRedisofChannelInterStat: redisCommand(hset app.module.dialup.limitstat.channel %d %d) error(%s)", \
				chan_no,inter_stat, reply->str);
		}
		else
		{
			zprintf(INFO,"[INFO]setRedisofChannelInterStat: redisCommand(hset app.module.dialup.limitstat.channel %d %d) succ", \
				chan_no,inter_stat);
		}
		freeReplyObject((void *)reply);
	}
	redisFree(context);
}

static int getChannelInterStatFromRedis(int chan_no, unsigned char *inter_stat)
{
	int ret = 0;
	char redis_result[12];
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err)
	{
		zprintf(ERROR,"[ERRO]getChannelInterStatFromRedis:channel %d redisConnect error(%s)",chan_no, context->errstr);
	}
	else
	{
		redisReply *reply = (redisReply *)redisCommand(context, "hget app.module.dialup.limitstat.channel %d",chan_no);
		if (reply->type == REDIS_REPLY_ERROR)
		{
			ret = -1;
			zprintf(ERROR,"[ERRO]getChannelInterStatFromRedis:%d error(%s)", chan_no, reply->str);
		}
		else
		{
			if (reply->str != NULL)
			{
				strcpy(redis_result,reply->str);
				zprintf(INFO,"[INFO]getChannelInterStatFromRedis:channel %d return %s",chan_no, redis_result);
			}
			else
			{
				ret = -1;
				zprintf(ERROR,"[ERROR]getChannelInterStatFromRedis: channel %d return NULL", chan_no);
			}
		}
		freeReplyObject((void *)reply);
	}
	redisFree(context);
	if (inter_stat != NULL){
		*inter_stat = atoi(redis_result);
	}
	return ret;
}

int getRedisofChannelSMSInfo(char *filed, char* strInfo)
{
	char rds_cmd[256] = {0};
	int ret = 0;
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err){
		zprintf(ERROR,"[ERRO]getChannelSMSReceiveFromRedis: redis cli %s, redisConnect error(%s)",filed, context->errstr);
	}
	else{
		sprintf(rds_cmd,"hget %s %s",REDIS_KEY_SIMBANK_SMS_INFO, filed);
		redisReply *reply = (redisReply *)redisCommand(context, rds_cmd);
		if (reply->type == REDIS_REPLY_ERROR){
			zprintf(ERROR,"[ERRO]getChannelSMSReceiveFromRedis:%s error", rds_cmd);
			ret = -1;
		}
		else{
			if (reply->str != NULL){
				chr_trim(strInfo, reply->str,'\'');
				zprintf(INFO,"[INFO]getChannelSMSReceiveFromRedis:redisCommand(%s).", rds_cmd);
			}
			else{
				zprintf(ERROR,"[ERROR]getChannelSMSReceiveFromRedis:redisCommand(%s) return NULL", rds_cmd);
				ret = -1;
			}
		}
		freeReplyObject((void *)reply);
	}
	redisFree(context);
	return ret;
}


static int getChannelSMSReceiveFromRedis(int chan_no, sms_send_rsp_info_t* sms_send_info)
{
	if(!sms_send_info){
		zprintf(ERROR, "[ERRO]getChannelSMSReceiveFromRedis error:sms info is NULL!");
		return -1;
	}
	char filed1[32] = {0};
	char filed2[32] = {0};
	char filed3[32] = {0};

	sprintf(filed1, "%d-recv-msg", chan_no);
	sprintf(filed2, "%d-recv-time", chan_no);
	sprintf(filed3, "%d-recv-num", chan_no);
	if (!getRedisofChannelSMSInfo(filed1, sms_send_info->recv_msg) &&
		!getRedisofChannelSMSInfo(filed2, sms_send_info->recv_time) &&
		!getRedisofChannelSMSInfo(filed3, sms_send_info->caller)){
		zprintf(INFO, "[INFO]getChannelSMSReceiveFromRedis:SMS caller %s, ", sms_send_info->caller);
		return 0;
	}
	zprintf(WARN, "[WARN]getChannelSMSReceiveFromRedis, No email received in this time");
	return -1;

}

int getChannelStatusFromRedis(int chan_no)
{
	int result = 0;
	
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err)
	{
		zprintf(ERROR,"[ERRO]getChannelStatusFromRedis:channel %d redisConnect error(%s)",chan_no,context->errstr);
	}
	else
	{
		redisReply *reply = (redisReply *)redisCommand(context, "hget %s %d",REDIS_SIMSTATUS, chan_no);
		if (reply->type == REDIS_REPLY_ERROR)
		{
				zprintf(ERROR,"[ERRO]getChannelStatusFromRedis: hget %s %d error(%s)", REDIS_SIMSTATUS,chan_no, reply->str);
		}
		else
		{
			if (reply->str != NULL)
			{
				result = atoi(reply->str);
				zprintf(INFO,"[INFO]getChannelStatusFromRedis: redisCommand(hget %s %d) return %d", REDIS_SIMSTATUS, chan_no, result);
			}
			else
			{
				result = 0;
				zprintf(ERROR,"[ERROR]getChannelStatusFromRedis: redisCommand(hget %s %d) return NULL",REDIS_SIMSTATUS, chan_no);
			}
		}
		freeReplyObject((void *)reply);
	}
	redisFree(context);
	return result;
}

/**************************************************************************** 
* 函数名称 : resetMini52
* 功能描述 : 重置指定的Mini52芯片
* 参    数 : unsigned short slot			: slot卡槽号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int resetMini52(int handle, unsigned short board, unsigned short slot)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int ret = -1;
	int trys = 2;
	int len = 0;

	buff_req[0] = (unsigned char)slot;
	buff_req[1] = CMD_IS_REQRST;
	buff_req[2] = 0;
	buff_req[3] = 0;
	for (int i = 0; i < trys; i++)
	{
		//ret = socket_pty_wr(csock, buff_req, 4, buff_rsp, &len, SERIAL_WR_TIMEOUT);
		ret = serial_wr_atr(handle, buff_req, 4, buff_rsp, len, SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == (unsigned char)slot && buff_rsp[1] == IS_RESP_RST && buff_rsp[2] == 0 && buff_rsp[3] == 0)
		{
			zprintf(ERROR,"[ERRO]resetMini52[%02d-%02d-%02d]: reset Mini52 succ", 0, board, slot);
			ret = 0;
			Sleep(1000);
			break;
		}
		else
		{
			zprintf(ERROR,"[ERRO]resetMini52[%02d-%02d-%02d]: reset Mini52 error(%d:%s)", 0, board, slot, errno, strerror(errno));
			Sleep(1000);
		}
	}

	return ret;
}
int resetMini52WithoutSleep(int handle, unsigned short board, unsigned short slot)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int ret = -1;
	int trys = 2;
	int len = 0;

	buff_req[0] = (unsigned char)slot;
	buff_req[1] = CMD_IS_REQRST;
	buff_req[2] = 0;
	buff_req[3] = 0;
	for (int i = 0; i < trys; i++)
	{
		//ret = socket_pty_wr(csock, buff_req, 4, buff_rsp, &len, SERIAL_WR_TIMEOUT);
		ret = serial_wr_atr((unsigned int)handle, buff_req, 4, buff_rsp, len, SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == (unsigned char)slot && buff_rsp[1] == IS_RESP_RST && buff_rsp[2] == 0 && buff_rsp[3] == 0)
		{
			zprintf(INFO,"[INFO]resetMini52[%02d-%02d-%02d]: reset Mini52 succ", 0, board, slot);
			ret = 0;
			break;
		}
		else
		{
			zprintf(ERROR,"[ERRO]resetMini52[%02d-%02d-%02d]: reset Mini52 error(%d:%s)", 0, board, slot, errno, strerror(errno));
			return -1;
		}
	}

	return ret;
}

int resetSTM32(int handle, unsigned short board)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int ret = -1;
	int trys = 2;
	//int len = 0;

	buff_req[0] = 0x80;
	buff_req[1] = CMD_IS_REQRST;
	buff_req[2] = 0;
	buff_req[3] = 0;
	for (int i = 0; i < trys; i++)
	{
		ret = serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == 0x80 && buff_rsp[1] == IS_RESP_RST && buff_rsp[2] == 0 && buff_rsp[3] == 0)
		{
			zprintf(ERROR,"[ERRO]resetSTM32[00-%02d]: reset STM32 succ", board);
			ret = 0;
			Sleep(1000);
			break;
		}
		else
		{
			zprintf(ERROR,"[ERRO]resetSTM32[00-%02d]: reset STM32 error(%d:%s)", board, errno, strerror(errno));
			Sleep(1000);
		}
	}

	return ret;
}

 int getSTM32Version(int handle, unsigned short board,char *version,int version_len)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int len = 0;
	int trys = 2;
	int ret = -1;
	
	buff_req[0] = 0x80;
	buff_req[1] = CMD_IS_GETVER;
	buff_req[2] = 0;
	buff_req[3] = 0;

	for (int i = 0; i < trys; i++)
	{
		len = serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == 0x80 && buff_rsp[1] == IS_VER && len == (buff_rsp[3]+4))
		{
			zprintf(INFO,"[INFO]getSTM32Version: STM32[%02d] version[%s]", board, buff_rsp+4);
			strncpy(version,(char *)(buff_rsp+4),version_len);
			ret = 0;
			//Sleep(1000);
			break;
		}
		zprintf(ERROR,"[ERRO]getSTM32Version: get STM32[%02d] version error(%d:%s)", board, errno, strerror(errno));
		//Sleep(1000);
	}
	
	return ret;
}

/**************************************************************************** 
* 函数名称 : resetAllMini52
* 功能描述 : 重置所有Mini52芯片
* 参    数 : void
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int resetAllMini52(int handle, unsigned short board)
{
	unsigned short i = 0;
	int ret = 0;
	int res = 0;
	for (i = 0; i < SLOT_NBR; i++)
	{
		//ret = resetMini52(handle, board, i);
		ret = resetMini52WithoutSleep(handle, board, i);
		if (ret < 0)
		{
			res = -1;
		}
	}
	Sleep(1000);
	return res;
}

int openSimbus(int handle,int slot_nbr)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	//int len = 0;
	int ret = -1;
	
	buff_req[0] = (unsigned char)slot_nbr;
	buff_req[1] = 0x53;
	buff_req[2] = 0;
	buff_req[3] = 0;

	serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
	//len = serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
	if (buff_rsp[0] == buff_req[0] && buff_rsp[1] == 0x52)
	{
		zprintf(INFO,"[INFO]openSimbus: %02d slot open success", slot_nbr);
		ret = 0;
	}else{
		zprintf(ERROR,"[ERROR]openSimbus: %02d slot open error", slot_nbr);
	}
	return ret;
}

int closeSimbus(int handle,int slot_nbr)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	//int len = 0;
	int ret = -1;
	
	buff_req[0] = (unsigned char)slot_nbr;
	buff_req[1] = 0x55;
	buff_req[2] = 0;
	buff_req[3] = 0;

	serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
	//len = serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
	if (buff_rsp[0] == buff_req[0] && buff_rsp[1] == 0x54)
	{
		zprintf(INFO,"[INFO]closeSimbus: %02d slot close success", slot_nbr);
		ret = 0;
	}else{
		zprintf(ERROR,"[ERROR]closeSimbus: %02d slot close error", slot_nbr);
	}
	return ret;
}

int closeallSimbus(int handle)
{
	int i;
	for(i = 0;i < SIM_NBR;i++){
		closeSimbus(handle,i);
	}
	return 0;
}
/**************************************************************************** 
* 函数名称 : getMini52Version
* 功能描述 : 获取指定的Mini52芯片的版本号
* 参    数 : unsigned short slot			: slot卡槽号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getMini52Version(int handle, unsigned short usb_nbr, unsigned short slot,char *version)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int len = 0;
	int trys = 2;
	int ret = -1;
	
	buff_req[0] = (unsigned char)slot;
	buff_req[1] = CMD_IS_GETVER;
	buff_req[2] = 0;
	buff_req[3] = 0;

	for (int i = 0; i < trys; i++)
	{
		len = serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == (unsigned char)slot && buff_rsp[1] == IS_VER && len == (buff_rsp[3]+4))
		{
			zprintf(INFO,"[INFO]getMini52Version: Mini52[usb:%d, slot:%d] version[%s]", usb_nbr, slot, buff_rsp+4);
			ret = 0;
			sprintf(version,"%d:%s\n",slot,buff_rsp+4);
			//Sleep(1000);
			break;
		}
		zprintf(ERROR,"[ERRO]getMini52Version: get Mini52[usb:%d, slot:%d] version error(%d:%s)", usb_nbr, slot, errno, strerror(errno));
		//Sleep(1000);
	}
	
	return ret;
}

/**************************************************************************** 
* 函数名称 : getAllMini52Version
* 功能描述 : 获取所有的Mini52芯片的版本号
* 参    数 : void
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getAllMini52Version(int handle, unsigned short usb_nbr,char *version)
{
	unsigned short i = 0;
	int ret = 0;
	int res = 0;
	for (i = 0; i < SLOT_NBR; i++)
	{
		ret = getMini52Version(handle, usb_nbr, i,version  + strlen(version));
		if (ret < 0)
		{
			res = -1;
		}
	}
	return res;
}

/**************************************************************************** 
* 函数名称 : getModuleCarrier
* 功能描述 : 获取端口号的运营商信息
* 参    数 : chan 端口号
			carrier_buf 运营商信息缓存
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : baozhenhua 
 *****************************************************************************/
static int getModuleCarrier(unsigned short chan, char* carrier_buf)
{
	//cat /tmp/gsm/1
	char shell_cmd[64] = {0};
	char result_buff[128] = {0};
	// /tmp/gsm/$chan_num file with some delay
	Sleep(1500);
	
	if (carrier_buf == NULL){
		zprintf(ERROR, "getModuleCarrier carrier_buf is null!");
		return -1;
	}
	snprintf(shell_cmd, sizeof(shell_cmd), "cat /tmp/gsm/%d | grep \"Network Name\"", chan);
	if (get_buff_from_popen(result_buff, shell_cmd) != 0){
		zprintf(ERROR, "getModuleCarrier error, popen error!");
		return -1;
	}
	//eg:Network Name: CHINA MOBILE
	strcpy(carrier_buf, result_buff + 14);
	carrier_buf[strlen(result_buff) - 15] = '\0';
	return 0;
}

/**************************************************************************** 
* function : smsRecvMsgMatch
* description : sms receive message type match
* parameter : sms_type sms type 1-,2-
			carrier_buf carrier info buffer
* return : success return 0，fail return -1
* writer : baozhenhua 
 *****************************************************************************/

static int smsRecvMsgMatch(sms_send_rsp_info_t* sms_send_info, char* match)
{
	int ret = 0;
	switch(sms_send_info->sms_type)
	{
		case SMS_QUERY_PHONENUM:
			ret = SMSPhoneNumStringMatch(sms_send_info->recv_msg, match, sms_send_info->match_msg);
			break;
		case SMS_QUERY_BALANCE:
			ret = SMSBanlanceStringMatch(sms_send_info->recv_msg, match, sms_send_info->match_msg);
			break;
		case SMS_QUERY_OTHER:
		default:
			ret = 2;
	}

	//sms match fail
	if(!ret){
		zprintf(ERROR, "smsRecvMsgMatch match %s fail!", match);
		return -1;
	}
	return 0;
}

int emuSlotReport(CSocketEx *csock, char *local_ip, unsigned short local_port, unsigned short bank_nbr, unsigned short slot_nbr, unsigned short chn_nbr)
{
	int ret = 0;
	int len;
	report_req_info_t report_info;
	char buff[BUFF_SIZE] = {0};

	memset((char *)&report_info, 0, sizeof(report_info));
	report_info.type = htons(SimEmuSvr);
	memcpy(report_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	report_info.bank_nbr = htons(bank_nbr);
	report_info.slot_nbr = htons(slot_nbr);
	report_info.chn_nbr = htons(chn_nbr);
	report_info.port = htons(local_port);
	report_info.module_type = htons(gSlotHdlTaskParam[bank_nbr][slot_nbr].module_type);
	//report_rsp_info_t *report_rsp = NULL;
	len = makeReportPackage(buff, (char *)&report_info, sizeof(report_info)-sizeof(report_info.vgsm_len)-sizeof(report_info.vgsm), &len);

	// 线路上报

	ret = csock->WriteData(csock->m_sockfd, buff, len);
	if (ret != len)
	{
		zprintf(ERROR,"[ERRO]emuReport[00-%02d-%02d]: send link report package to NetHdlTask error(%d:%s).", bank_nbr, slot_nbr, errno, strerror(errno));
		return -1;
	}
	zprintf(INFO,"[INFO]emuReport[00-%02d-%02d]: link report succ[ip:%s, port:%d, bank_nbr:%d, slot_nbr:%d]", \
		bank_nbr, slot_nbr, local_ip, local_port, bank_nbr, slot_nbr);
	return 0;
}

/**************************************************************************** 
* 函数名称 : SlotHdlTask
* 功能描述 : slot卡槽处理线程函数
* 参    数 : void *pParam					: 线程参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void * SlotHdlTask(void *pParam)
{
	int ret = 0;
	slothdltask_param_t *param = (slothdltask_param_t *)pParam;
	CSocketEx sock_comm;
	CSocketEx sock_nettask;
	char buff_recv[0xffff] = {0};
	char buff_send[0xffff] = {0};
	char bsp_cmd[128];
	int len_recv = 0;
	int len_send = 0;
	msg_header_t *pmsgh = NULL;
	msg_t msg;
	link_info_t link_info;
	char sim_vgsm_buf[VGSM_LEN] = {0};
	char seri_emu[12] = {0};
	char seri_bank[12] = {0};

	char cmd_buf[128] = {0};
	char result_buf[32]  = {0};
	char rdr_ip_buf[16] = {0};
	char vgsm_file[64] = {0};
	FILE *fp = NULL;
	//
	Emu_Engine *emu_eng = new Emu_Engine;
	memset(emu_eng, 0, sizeof(Emu_Engine));
	//
	char sim_atr[320] = {0};
	time_t link_create_last = 0;
	time_t link_create_curr = 0;
	int time_dur = 0;
	unsigned int link_create_cnt = 0;
	unsigned int link_release_cnt = 0;
	time_t last = 0;
	time_t curr = 0;
	time_t slothdl_last = 0;
	time_t slothdl_curr = 0;
	time_t link_check_time = 0;
	time_t icc_rst_time = 0;
	time_t icc_data_time = 0;
	time_t sms_recv_time = 0;
		
	time(&last);
	link_relation_t link_relation;
	
	char chan_stat = 2; // 1 means is registering 
	//sms recv
	//unsigned short sms_send_stat = SMS_SENT;
	unsigned short sms_type = 3;
	unsigned char sms_cnt = 0;
	char sms_match_str[32];
	
	char inter_stat = 0;
	char dialup_stat = INTER_DIALUP_DISCONN;    //0-inter_disconn 1-connecting 2-inter_conn
	int chan_no = get_port_from_map(param->slotinfo.board_nbr * SLOT_NBR + param->slotinfo.slot_nbr + 1);
	char dialup_file[32];
	char surf_file[32];
	int epollfd = -1;
	struct epoll_event events[2];
	int epollnum = 0;
	epollfd = epoll_create(2);

	zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: Start[port_comm:%d, port_nettask:%d,channel :%d]......", \
		0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->port_comm, param->port_nettask,chan_no);

	sprintf(dialup_file, "%s/%d%s", INTER_LOG_PATH, chan_no, DIAL_NAME);
	sprintf(surf_file, "%s/%d%s", INTER_LOG_PATH, chan_no, INTER_NAME);
	// create socket
	if (sock_comm.m_sockfd < 0)
	{
		sock_comm.Init(PROTO_UDP, param->local_ip, param->port_comm, param->local_ip, gCommHdlTaskParam[param->slotinfo.gw_bank_nbr].port, 10);
		sock_comm.CreateSocket();
		sock_comm.Bind();
		param->sockfd_comm = sock_comm.getSocket();
		zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: sock_comm    create succ(%s:%d<==>%s:%d)", 
			0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->local_ip, param->port_comm, param->local_ip, gCommHdlTaskParam[param->slotinfo.gw_bank_nbr].port);
	}
	if (sock_nettask.m_sockfd < 0)
	{
		sock_nettask.Init(PROTO_UDP, param->local_ip, param->port_nettask, param->local_ip, gSimEmuSvrParam.net_hdl_port, 10);
		sock_nettask.CreateSocket();
		sock_nettask.Bind();
		param->sockfd_nettask = sock_nettask.getSocket();
		zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: sock_nettask create succ(%s:%d<==>%s:%d)", 
			0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->local_ip, param->port_nettask, param->local_ip, gSimEmuSvrParam.net_hdl_port);
	}

	add_event(epollfd, sock_comm.m_sockfd, EPOLLIN);
	add_event(epollfd, sock_nettask.m_sockfd, EPOLLIN);

	sprintf(bsp_cmd,"asterisk -rx 'gsm set remotesim %d 0'",chan_no);
	system(bsp_cmd);
	sleep(10);

	while(gCommHdlTaskParam[param->slotinfo.gw_bank_nbr].initialed == 0 && param->stop != 1){
		Sleep(2000);
	}

REGISTER_WAIT:
	while(REGISTER_DOWN == gRegisterStat.stat){
		Sleep(10000);
	}
	
REPORT_LINE:
	// 线路上报
	// 改线路端口为统一对外端口
	while(emuSlotReport(&sock_nettask, param->local_ip, gSimEmuSvrParam.sim_data_out_port, \
		param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, \
		get_port_from_map(param->slotinfo.gw_bank_nbr*SLOT_NBR+param->slotinfo.slot_nbr+1)) != 0 \
		&& param->stop != 1){
		Sleep(3000);
	}

	time(&link_check_time);
	time(&slothdl_curr);
	while (param->stop != 1)
	{
		
		time(&slothdl_curr);
		if ((slothdl_curr - slothdl_last) > 180)
		{
			time(&slothdl_last);
			zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: Running[port_comm:%d, state:%d]......", \
				0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->port_comm, param->slotinfo.link_stat);
		}

		// 判断通道注册是否超时
		// 将注册状态变化发送给simbank
			
		time(&curr);
		if ((curr - last) > 10) // 间隔10以上秒检查一次
		{
			time(&last);
			if(REGISTER_DOWN == gRegisterStat.stat){
				if(param->slotinfo.link_stat == 1)
				{
					memset((char *)&msg, 0, sizeof(msg));
					msg.header.cmd = SIMLINK_RELEASE_REQ;
					msg.header.slot = param->slotinfo.slot_nbr;
					msg.header.data_len = 0;
					param->slotinfo.link_stat = 0;
					ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, sizeof(msg_header_t));
					if ((unsigned short)ret != sizeof(msg_header_t))
					{
						zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send power reset span command to CommHdlTask error(%d:%s)", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
					}
					else{
						zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: send power off span command to CommHdlTask ......", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
					}
					
					chan_stat = 2;
					param->call_flag = 0;
					
					emu_eng->SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PULL_OUT);
					emu_eng->SetEmuEngineStatus(EMU_ENGINE_STOP);
				}
				zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d]: Net off ......", param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
				goto REGISTER_WAIT;
			}
			
			if (param->slotinfo.link_stat == 1)
			{
				int channel_status = getChannelStatusFromRedis(chan_no);

				if(chan_stat != channel_status)
				{
					char carrier_buf[64] = {0};
					memset(&msg,0,sizeof(msg));
					memcpy(msg.data,&link_relation, sizeof(link_relation));
					msg.data[sizeof(link_relation)] = (char)channel_status;
					//register success, get module current carrier
					if (channel_status == 0){
						getModuleCarrier(chan_no, carrier_buf);
						strcpy((char*)msg.data + sizeof(link_relation) + 1, carrier_buf);
						
					}
					makeSimRegisterStatNoticePack(buff_send, msg.data, sizeof(link_relation) + 1 + strlen(carrier_buf) + 1, &len_send);
					chan_stat = channel_status;
					ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
					if(ret != len_send){
						zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sim data req package error(%d:%s)", \
							0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
					}
/*					if(1 == channel_status){
						// extra restart 后会将初始化通道注册状态，需要重新通知插卡状态
						sprintf(bsp_cmd,"asterisk -rx 'gsm set remotesim %d 1 %d %d'",chan_no, param->slotinfo.sb_bank_nbr, param->slotinfo.sim_nbr);
						system(bsp_cmd);
						zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d], set remotesim chan=%d.", 0, param->slotinfo.sb_bank_nbr, param->slotinfo.sim_nbr, chan_no);
					}*/
				}	
/*				if (channel_status == 1)
				{
					time(&link_create_curr);					
					time_dur = link_create_curr - link_create_last;
					if (time_dur > 300)		// 180 ==> 300
					{
						time(&link_create_last);
						memset((char *)&msg, 0, sizeof(msg));
						msg.header.cmd = SIMLINK_RELEASE_REQ;
						msg.header.slot = param->slotinfo.slot_nbr;
						msg.header.data_len = 0;
						param->slotinfo.link_stat = 0;
						ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, sizeof(msg_header_t));
						if ((unsigned short)ret != sizeof(msg_header_t))
						{
							zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send power reset span command to CommHdlTask error(%d:%s)", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
						}
						else{
							zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: send power reset span command to CommHdlTask ......", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
						}

						makeLinkBreakNoticePack(buff_send,(char *)&link_relation,sizeof(link_relation_t),&len_send);
						ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
						if (ret != len_send)
						{
							zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sim break req package error(%d:%s)", \
								0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
						}
						
						emu_eng->SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PULL_OUT);
						emu_eng->SetEmuEngineStatus(EMU_ENGINE_STOP);

						sprintf(bsp_cmd,"asterisk -rx 'gsm set remotesim %d 0'",chan_no);
						system(bsp_cmd);
						zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: sim card register fail ......", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
					}

				}
				else*/
				if (channel_status != 1)
				{
					time(&link_create_last);
					time(&link_check_time);
					if(curr - icc_data_time > 120){
						zprintf(ERROR,"\033[31m[ERRO]SlotHdlTask[%02d-%02d-%02d]:Over 2 mins not receive apdu data\033[0m",0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
						sprintf(bsp_cmd,"asterisk -rx 'gsm at reset %d'",chan_no);
						system(bsp_cmd);
						time(&icc_data_time);
					}
					//recv sms
					if(curr - sms_recv_time > 30)
					{
						time(&sms_recv_time);
						sms_send_rsp_info_t sms_send_rsp_info;
						memset(&sms_send_rsp_info, 0, sizeof(sms_send_rsp_info_t));
						if(!getChannelSMSReceiveFromRedis(chan_no, &sms_send_rsp_info))
						{
							sms_send_rsp_info.sms_type = sms_type;
							if (smsRecvMsgMatch(&sms_send_rsp_info, sms_match_str)){
								zprintf(ERROR, "[ERRO]SlotHdlTask[%02d-%02d]: sms match fail", param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
								sms_send_rsp_info.sms_type = SMS_QUERY_OTHER;
								++sms_cnt;
								if (sms_cnt > 10){
									memset(sms_match_str, 0 ,sizeof(sms_match_str));
									sms_type = SMS_QUERY_OTHER;
									zprintf(ERROR, "[ERRO]SlotHdlTask[%02d-%02d]: sms match fail over 10 times, resetting", param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
								}
							}
							else{
								sms_type = SMS_QUERY_OTHER;
							}
							//delete redis sms receive info
							delRedisAllofChannelSMSInfo(chan_no);

							memset(buff_send,0,sizeof(buff_send));
							sms_send_rsp_info.sb_bank_nbr = htons(param->slotinfo.sb_bank_nbr);
							sms_send_rsp_info.sb_slot_nbr = htons(param->slotinfo.sim_nbr);
							sms_send_rsp_info.sms_type = htons(sms_send_rsp_info.sms_type);
							strcpy(sms_send_rsp_info.sb_seri, seri_bank);		
							makeSMSRSPPackage(buff_send,(char *)&sms_send_rsp_info, sizeof(sms_send_rsp_info_t), &len_send);
							ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
							if(ret != len_send){
								zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sms data req package error(%d:%s)", \
						  		0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
							}
						}
					}
					if (dialup_stat == INTER_DIALUP_CONNECTING){
						char cmd_state_buf[32] = {0};
						char json_buf[256] = {0};
						//find result file
						if (get_file_info(dialup_file, cmd_state_buf, sizeof(cmd_state_buf)) != -1){
							if (strstr(cmd_state_buf, INTER_ERROR)){
								zprintf(ERROR, "SlotHdlTask error : dialup error!\n");
								snprintf(json_buf, sizeof(json_buf), "{\"type\":\"%s\",\"seri\":\"%s\",\"chan\":\"%d\",\"stat\":\"%s\"}", 
											JOSN_TYPE_INTER, seri_emu, chan_no, INTER_STAT_DISCONN);
								dialup_stat = INTER_DIALUP_DISCONN;
							}
							else{
								snprintf(json_buf, sizeof(json_buf), "{\"type\":\"%s\",\"seri\":\"%s\",\"chan\":\"%d\",\"stat\":\"%s\"}", 
											JOSN_TYPE_INTER, seri_emu, chan_no, INTER_STAT_CONN);
								dialup_stat = INTER_DIALUP_CONN;
							}
							

							makeEventModuleInterPackage(buff_send, json_buf, strlen(json_buf) + 1, &len_send);
							ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
							if(ret != len_send){
								zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send internet package error(%d:%s)", \
						  		0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
							}
						}
					}
					//internet surf file send
					if (inter_stat){
						char filename[32] = {0};
						char cmd_state_buf[32] = {0};
						//find result file
						if (get_file_info(surf_file, cmd_state_buf, sizeof(cmd_state_buf)) != -1){
							
							char* json_buf = (char*)malloc(BUFF_10M + 256);
							char* result_buf = (char*)malloc(BUFF_10M);
							//char shell_buf[32] = {0};
							int len = 0;
							memset(buff_send, 0, sizeof(buff_send));
							len = snprintf(json_buf, 256, "{\"type\":\"%s\",\"seri\":\"%s\",\"chan\":\"%d\",\"stat\":\"%s\",", 
												JOSN_TYPE_INTER, seri_emu, chan_no, INTER_STAT_CONN);

							if (strstr(cmd_state_buf, INTER_ERROR)){
								zprintf(ERROR, "SlotHdlTack error : internet surf error!");
							}
							switch (inter_stat){
								case INTER_SURF_MUST:						
									snprintf(filename, sizeof(filename), "%s/%d_internet_log", INTER_LOG_PATH, chan_no);
									get_file_info(filename, result_buf, BUFF_10M);
									snprintf(json_buf + len, sizeof(json_buf) - len, "\"result\":\"%s\"}", result_buf);
									//len += snprintf(json_buf + len, sizeof(json_buf) - len, "\"result\":\"");
									//if ((tmp_len = get_file_info(filename, json_buf + len, BUFF_10M)) == -1){
									//	zprintf(ERROR, "SlotHdlTack error : internet get result error!");
									//}
									//else{
									//	len += tmp_len;
									//}
									//snprintf(json_buf + len, sizeof(json_buf) - len, "\"}");
									break;
								case INTER_SURF_SIZE:
									snprintf(json_buf + len, sizeof(json_buf) - len, "\"result\":\"%s\"}", cmd_state_buf);
									break;
								default:
									zprintf(ERROR, "SlotHdlTack error : internet surf unknown command!");
									break;
							}
							makeEventModuleInterPackage(buff_send, json_buf, strlen(json_buf) + 1, &len_send);
							ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
							if(ret != len_send){
								zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send internet package error(%d:%s)", \
						  		0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
							}
							
							free(json_buf);
							json_buf = NULL;
							free(result_buf);
							result_buf = NULL;
							inter_stat = 0;
						}
					}
				}
			}	
			else
			{
				if(curr - link_check_time > 180)
				{
					zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: Link not been Create!!!", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
					chan_stat = 2;
					goto REPORT_LINE;
				}
			}
			
		}

		epollnum = epoll_wait(epollfd, events, 2, 5000);
		if (epollnum < 0)
		{
			zprintf(ERROR,"[ERRO]NetHdlTask: epoll_wait error(%d:%s)", errno, strerror(errno));
			continue;
		}
		else if (epollnum == 0)
		{
			continue;
		}
		else
		{
			int n = 0;				
			for(n = 0;n < epollnum;n++)
			{	// from NetHdlTask, sim link create or release notice
				if (events[n].events & EPOLLIN && events[n].data.fd == sock_nettask.m_sockfd)
				{
					memset(buff_recv, 0, sizeof(buff_recv));
					len_recv = sock_nettask.ReadData(sock_nettask.m_sockfd, buff_recv, sizeof(buff_recv));
					if (len_recv <= 0)
					{
						zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: read data from NetHdlTask error(%d:%s)", \
							0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
						//continue;
					}
					else
					{ // 线路建立，线路释放，Sim数据响应
						memset((char *)&msg, 0, sizeof(msg));
						memcpy((char *)&msg, buff_recv, sizeof(msg_header_t));
						if (msg.header.cmd == SIMLINK_CREATE_REQ)
						{
							memset(sim_vgsm_buf, 0, sizeof(sim_vgsm_buf));
							do{
								memset((char*)&link_relation, 0, sizeof(link_relation_t));
								memcpy((char *)&link_info, buff_recv+sizeof(msg_header_t), msg.header.data_len);
								snprintf(vgsm_file, sizeof(vgsm_file), "%s%s%s", SIMDATA_DIR, link_info.eficcid, SIMDATA_EXTENSION);
								fp = fopen(vgsm_file, "r");
								if (fp == NULL){					
									ip_num_to_char(link_info.sb_ip, rdr_ip_buf);
									if (strcmp(rdr_ip_buf, "127.0.0.1") == 0){
										strncpy(rdr_ip_buf, gSimEmuSvrParam.server_ip, sizeof(rdr_ip_buf));
									}
									snprintf(cmd_buf, sizeof(cmd_buf), "%s %s %s", GET_SIMDATA_CLI, link_info.eficcid, rdr_ip_buf);
									get_buff_from_popen(result_buf, cmd_buf);
									fp = fopen(vgsm_file, "r");
								}
								if (fp == NULL || (strlen(result_buf) != 0 && strstr(result_buf, "OK") == NULL)){
									zprintf(ERROR,"[ERRO]SlotHdlTask: get rdr siminfo error");
									strncpy(link_relation.gw_seri, link_info.gw_seri, sizeof(link_relation.gw_seri));
									link_relation.gw_bank_nbr = htons(link_info.gw_bank_nbr);
									link_relation.gw_slot_nbr = htons(link_info.slot_nbr);
									strncpy(link_relation.sb_seri, link_info.sb_seri, sizeof(link_relation.sb_seri));
									link_relation.sb_bank_nbr = htons(link_info.sb_bank_nbr);
									link_relation.sb_slot_nbr = htons(link_info.sim_nbr);
									makeLinkBreakNoticePack(buff_send,(char *)&link_relation,sizeof(link_relation_t),&len_send);
									ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
									if (ret != len_send)
									{
										zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sim data req package error(%d:%s)", \
											0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
										param->slotinfo.stat = SlotInvalid;
									}
									break;
								}
								fread(sim_vgsm_buf, 1, VGSM_LEN, fp);
								fclose(fp);
													
								memcpy(seri_emu, link_info.gw_seri, SERIALNO_LEN);
								memcpy(seri_bank, link_info.sb_seri, SERIALNO_LEN);								
								// set param
								memcpy(param->slotinfo.sb_seri, link_info.sb_seri, SERIALNO_LEN);
								param->slotinfo.sb_usb_nbr = link_info.sb_usb_nbr;
								param->slotinfo.sb_bank_nbr = link_info.sb_bank_nbr;
								param->slotinfo.sim_nbr = link_info.sim_nbr;
								param->slotinfo.svr_ip = link_info.sb_ip;
								param->slotinfo.svr_port = link_info.sb_port;
								//param->slotinfo.gw_bank_nbr = link_info.gw_bank_nbr;
								
								// emu engine
								emu_eng->EmuApduEngineInitWithOptimizationMode(EMU_TRANSLAOTR_OPTIMIZATION_NULL);
								emu_eng->SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PLUG_IN);
								emu_eng->SetEmuEngineStatus(EMU_ENGINE_WAITING_VCARD);
								emu_eng->SetEmuLocoalVCardReadyStatus(LOCOAL_VCARD_IS_READY);
								
								emu_eng->PreProcessEngineWithOptimizationMode((unsigned char *)sim_vgsm_buf,EMU_TRANSLAOTR_OPTIMIZATION_NULL);
								if (link_info.sim_atr[link_info.atr_len-1] == TRANSMISSION_PROTOCOL_T1)
								{
									emu_eng->SetEmuTransmissionProtocol(TRANSMISSION_PROTOCOL_T1);
								}
								else
								{
									emu_eng->SetEmuTransmissionProtocol(TRANSMISSION_PROTOCOL_T0);
								}
								//emu_eng->SetDeliverAtrToEmuStatus(DELIVER_ATR_TO_EMU_IS_READY);
								//eae.ucEmuApduNetApiStatus= EMU_APDU_NETAPI_STATUS_IDLE; 
								emu_eng->SetEmuEngineStatus(EMU_ENGINE_WORKING);
								emu_eng->SetDefaultCommandIdentifier();
								// sim atr
								memcpy(sim_atr, link_info.sim_atr, link_info.atr_len);
								//len_atr = link_info.atr_len;
								memcpy(param->sim_atr, link_info.sim_atr, link_info.atr_len);
								param->len_atr = link_info.atr_len;
								
								memcpy(msg.data, link_info.sim_atr, link_info.atr_len);
								msg.header.data_len = link_info.atr_len;
								ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, sizeof(msg_header_t) + msg.header.data_len);
								if ((unsigned int)ret != (sizeof(msg_header_t) + msg.header.data_len))
								{
									zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send SIMLINK_CREATE_REQ message(ret:%d) to CommHdlTask[port:%d] error(%d:%s)", \
										0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, ret, sock_comm.m_peerPort, errno, strerror(errno));
								}
								else
								{
									/*zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: send SIMLINK_CREATE_REQ message(ret:%d) to CommHdlTask[port:%d] succ", \
										0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, ret, sock_comm.m_peerPort);*/
								}
									
								// 设置通道注册开始时间
								time(&link_create_last);
								time(&icc_data_time);
								link_create_cnt++;
								param->slotinfo.link_stat = 1;
								
								memset((char *)&link_relation, 0, sizeof(link_relation));
								memcpy(link_relation.sb_seri, param->slotinfo.sb_seri, SERIALNO_LEN);
								memcpy(link_relation.gw_seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
								link_relation.sb_usb_nbr = htons(param->slotinfo.sb_usb_nbr);
								link_relation.sb_bank_nbr = htons(param->slotinfo.sb_bank_nbr);
								link_relation.sb_slot_nbr = htons(param->slotinfo.sim_nbr);
								link_relation.gw_bank_nbr = htons(param->slotinfo.gw_bank_nbr);
								link_relation.gw_slot_nbr = htons(param->slotinfo.slot_nbr);
								chan_stat = 2;
								param->call_flag = 0;

								zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: relation[gw:%s-00-%02d-%02d -- sb:%s-%02d-%02d-%02d]", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, \
									seri_emu, link_info.gw_bank_nbr, link_info.slot_nbr, \
									seri_bank, link_info.sb_usb_nbr, link_info.sb_bank_nbr, link_info.sim_nbr);
								sprintf(bsp_cmd,"asterisk -rx 'gsm set simloc %d %d %d'", chan_no, link_info.sb_bank_nbr, link_info.sim_nbr);
								system(bsp_cmd);
								zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d], set simloc chan=%d.", 0, link_info.sb_bank_nbr, link_info.sim_nbr, chan_no);
								DataToHex((unsigned char *)param->sim_atr,param->len_atr,DIRE_NET_TO_COMMHDLTASK,param->slotinfo.gw_bank_nbr,param->slotinfo.slot_nbr);
								logDataToHex(param->apdu_log_fd,(unsigned char *)param->sim_atr,param->len_atr,DIRE_NET_TO_COMMHDLTASK);
							}while(0);
						}
						else if (msg.header.cmd == SIMLINK_RELEASE_REQ)
						{
							zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d]: recv SIMLINK_RELEASE_REQ msg,module power off %d channel", param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr,chan_no);
					
							// reset Mini52
							msg.header.data_len = sizeof(msg_header_t);
							sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, sizeof(msg_header_t)); //msg.header.data_len);

							link_release_cnt++;
							param->slotinfo.link_stat = 0;
							
							memset(param->slotinfo.sb_seri, 0, sizeof(param->slotinfo.sb_seri));
							param->slotinfo.sb_usb_nbr = 0;
							param->slotinfo.sb_bank_nbr = 0;
							param->slotinfo.sim_nbr = 0;

							memset(link_relation.sb_seri, 0, sizeof(link_relation.sb_seri));
							link_relation.sb_usb_nbr = 0;
							link_relation.sb_bank_nbr = 0;
							link_relation.sb_slot_nbr = 0;
							chan_stat = 2;
							param->call_flag = 0;

							emu_eng->SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PULL_OUT);
							emu_eng->SetEmuEngineStatus(EMU_ENGINE_STOP);
							while (getChannelStatusFromRedis(chan_no) == 0){
//								zprintf(ERROR, "test status = 0");
								Sleep(20);
							}
						}
						else if (msg.header.cmd == SIM_PULLPLUG_NOTICE_REQ)
						{
							memset(link_relation.sb_seri, 0, sizeof(link_relation.sb_seri));
							link_relation.sb_usb_nbr = 0;
							link_relation.sb_bank_nbr = 0;
							link_relation.sb_slot_nbr = 0;
							
							chan_stat = 2;
							param->call_flag = 0;
							param->slotinfo.link_stat = 0;
							// 设置EmuEngine工作状态为停止
							emu_eng->SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PULL_OUT);
							emu_eng->SetEmuEngineStatus(EMU_ENGINE_STOP);
							
							zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: Receive SimPullPlugNotice And Stop EmuEngine", \
								0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
							
							// notice CommHldTask
							msg.header.slot = param->slotinfo.slot_nbr;
							ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, sizeof(msg.header));
						}
						else if(msg.header.cmd == SIMSMS_REQ)
						{
							char uuid[24] = {0};
							
							sms_send_req_info_t *sms_req_info = (sms_send_req_info_t *)(buff_recv+sizeof(msg_header_t));

							if (msg.header.data_len > sizeof(sms_send_req_info_t)){
								memcpy(uuid, buff_recv + sizeof(msg_header_t) + sizeof(sms_send_req_info_t), sizeof(uuid));
								zprintf(INFO, "[INFO]SlogtHdlTask[%02d-%02d]:SMS UUID is %s",param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, uuid);
							}
							strcpy(sms_match_str,sms_req_info->match_str);
							sms_type = ntohs(sms_req_info->sms_type);
							setRedisofChannelSMSInfo(chan_no, sms_req_info->dst_num, sms_req_info->send_msg, uuid);
							zprintf(INFO,"[INFO]SlogtHdlTask[%02d-%02d]:Get SMS request,callee is %s",\
								param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, sms_req_info->dst_num);
						}
						else if(msg.header.cmd == CHAN_HANG_UP_REQ)
						{
							/*停止引擎，避免在切卡完成之前又进行一次呼叫*/
							emu_eng->SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PULL_OUT);
							emu_eng->SetEmuEngineStatus(EMU_ENGINE_STOP);
							zprintf(INFO,"\033[31mSlotHdlTask[%02d-%02d-%02d]: calling over time\033[0m",0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
							Sleep(2000);
						}
						//send ussd
						else if(msg.header.cmd == SEND_USSD){
							ussd_info_t* ussd_info = (ussd_info_t *)(buff_recv+sizeof(msg_header_t));
							setRedisofChannelUssdInfo(chan_no, ussd_info->msg, ussd_info->uuid, ussd_info->timeout);
							zprintf(INFO,"\033[31mSlotHdlTask[%02d-%02d-%02d]: send ussd %s\033[0m",0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, ussd_info->msg);
						}
						else if(msg.header.cmd == SET_MODULE_LIMIT){
							char shell_cmd[128] = {0};
							char shell_result[40] = {0};
							char limit_cmd[40] = {0};
							char json_buf[256] = {0};
							module_limit_t* module_limit = (module_limit_t *)(buff_recv+sizeof(msg_header_t));
							switch(module_limit->limit){
								case MODULE_LIMITCMD_LIMIT:
									moduleLimitAll(chan_no, MODULE_LIMIT);
									break;
								case MODULE_LIMITCMD_UNLIMIT:
									moduleLimitAll(chan_no, MODULE_UNLIMIT);
									break;
								case MODULE_LIMITCMD_CALLLIMIT:
									sprintf(limit_cmd, "%s %d %d", GSM_SET_CALLLIMIT, chan_no, MODULE_LIMIT);
									break;
								case MODULE_LIMITCMD_CALLUNLIMIT:
									sprintf(limit_cmd, "%s %d %d", GSM_SET_CALLLIMIT, chan_no, MODULE_UNLIMIT);
									break;
								case MODULE_LIMITCMD_SMSLIMIT:
									sprintf(limit_cmd, "%s %d %d", GSM_SET_SMSLIMIT, chan_no, MODULE_LIMIT);
									break;
								case MODULE_LIMITCMD_SMSUNLIMIT:
									sprintf(limit_cmd, "%s %d %d", GSM_SET_SMSLIMIT, chan_no, MODULE_UNLIMIT);
									break;
								case MODULE_LIMITCMD_INTERLIMIT:
									setRedisofChannelInterStat(chan_no, MODULE_LIMIT);
									break;
								case MODULE_LIMITCMD_INTERUNLIMIT:
									setRedisofChannelInterStat(chan_no, MODULE_UNLIMIT);
									break;
								default:
									zprintf(ERROR, "module channel %d limit cmd value error!", chan_no);
									break;
									
							}
							if (strlen(limit_cmd)){
								sprintf(shell_cmd, "%s \"%s\"", AST_CLI, limit_cmd);
								if (get_buff_from_popen(shell_result, shell_cmd) == -1){
									zprintf(ERROR,"SlotHdlTask[%02d-%02d-%02d]: popen error",0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
								}
								else{
									zprintf(INFO, "test shell_cmd %s", shell_cmd);
									if (!strstr(shell_result, "success")){
										zprintf(WARN, "SlotHdlTask[%02d-%02d-%02d]: run cmd error!");
									}
								}							
							}
							shell_result[0] = '\0';
							getModuleLimitStat(chan_no, shell_result);
							snprintf(json_buf, sizeof(json_buf), "{\"type\":\"%s\",\"seri\":\"%s\",\"chan\":\"%d\",\"limit\":[%s]}", 
											JOSN_TYPE_LIMITSTAT, seri_emu, chan_no, shell_result);
							makeEventModuleLimitPackage(buff_send, json_buf, strlen(json_buf) + 1, &len_send);
							ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
							if (ret != len_send)
							{
								zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send event module limit package error(%d:%s)", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
								param->slotinfo.stat = SlotInvalid;
							}
							
						}
						else if(msg.header.cmd == SET_MODULE_INTERNET){
							char cmd_buff[128] = {0};
							char str_stat[12] = {0};
							char json_buf[256] = {0};
							module_internet_info_t* ptr_inter_info = (module_internet_info_t *)(buff_recv+sizeof(msg_header_t));

							//delete last file
							sprintf(cmd_buff, "rm %s", dialup_file);
							system(cmd_buff);
							//get_buff_from_popen(NULL, cmd_buff);
							sprintf(cmd_buff, "rm %s", surf_file);
							system(cmd_buff);
							
							if (ptr_inter_info->cmd == INTER_DIALUP){
								snprintf(cmd_buff, sizeof(cmd_buff), "%s %d %s %s %s", INTER_DIALUP_CLI, 
									chan_no, ptr_inter_info->apn, ptr_inter_info->user, ptr_inter_info->pwd);
								if (dialup_stat == INTER_DIALUP_DISCONN){
									dialup_stat = INTER_DIALUP_CONNECTING;
								}
								else{
									strcpy(str_stat, INTER_STAT_CONN);
								}
							}
							else if (ptr_inter_info->cmd == INTER_SURF){
								char result_type[12];
								if (dialup_stat == INTER_DIALUP_CONN){
									switch (ptr_inter_info->result){
										case INTER_SURF_MUST:
											strcpy(result_type, INTER_SURF_STR_MUST);
											inter_stat = ptr_inter_info->result;
											break;
										case INTER_SURF_NONE:
											strcpy(result_type, INTER_SURF_STR_NONE);
											strcpy(str_stat, INTER_STAT_SURFING);
											break;
										case INTER_SURF_SIZE:
											strcpy(result_type, INTER_SURF_STR_SIZE);
											inter_stat = ptr_inter_info->result;
											break;
										default:
											zprintf(ERROR, "SlotHdlTask error : internet surf unkown command!");
											break;
									}
									snprintf(cmd_buff, sizeof(cmd_buff), "%s %d %s %s %d &", INTER_SURF_CLI, 
										chan_no, result_type, ptr_inter_info->url, ptr_inter_info->presize);

								}
								else{
									zprintf(WARN, "SlotHdlTask warnning : internet is disconn!");
									strcpy(str_stat, INTER_STAT_DISCONN);
								}
								
							}
							else if (ptr_inter_info->cmd == INTER_END){
								snprintf(cmd_buff, sizeof(cmd_buff), "%s %d", INTER_END_CLI, chan_no);
								dialup_stat = INTER_DIALUP_DISCONN;
								strcpy(str_stat, INTER_STAT_DISCONN);
							}
							else{
								zprintf(ERROR, "SlotHdlTask error : internet unkown command!");
							}
							//run cmd
							zprintf(INFO, "SlotHdlTask info : cmd %s", cmd_buff);
							get_buff_from_popen(NULL, cmd_buff);
							
							//send pack
							if (strlen(str_stat) != 0){
								snprintf(json_buf, sizeof(json_buf), "{\"type\":\"%s\",\"seri\":\"%s\",\"chan\":\"%d\",\"stat\":\"%s\"}", 
											JOSN_TYPE_INTER, seri_emu, chan_no, str_stat);
								makeEventModuleInterPackage(buff_send, json_buf, strlen(json_buf) + 1, &len_send);
								ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
								if(ret != len_send){
									zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send internet package error(%d:%s)", \
							  		0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
								}
							}		
						}
						else// if (msg.header.cmd == SIMDATA_RSP)
						{
							//logDataToHex((unsigned char *)buff_recv+sizeof(msg_header_t)+sizeof(link_relation), pmsgh->data_len-sizeof(link_relation), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SIMHDLTASK_TO_SLOTHDLTASK);
						
							// 把网络远程发送回来的数据填进EmuEngine，执行EmuEngine，获取EmuEngine状态，取出数据发送到Emu小板
							pmsgh = (msg_header_t *)(buff_recv);
							len_send = pmsgh->data_len;
							if(0 == len_send-sizeof(link_relation)){
								emu_eng->SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PULL_OUT);
								emu_eng->SetEmuEngineStatus(EMU_ENGINE_STOP);
								zprintf(ERROR,"\033[31mSlotHdlTask[%02d-%02d-%02d]: receive NULL apdu\033[0m",0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
							}
							emu_eng->CopyNetDataToEaeBuf((unsigned char *)buff_recv+sizeof(msg_header_t)+sizeof(link_relation), len_send-sizeof(link_relation), pmsgh->cmd); //DATA_PROPERTY_IS_DATA);
							emu_eng->SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_DATA_FROM_NET_IS_READY);
							// 执行EmuEngine
							if(-1 == emu_eng->ProcessEmuDataCmd()){
								zprintf(INFO,"\033[31m[ERROR]ProcessEmuDataCmd ERROR!\033[0m");
							}
							// EmuEngine执行完成
							if (emu_eng->GetEmuEngineStatus() == EMU_ENGINE_WORKING)
							{ 			    	
								if (emu_eng->GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY)
								{
									// 组包，发送到网络远程
									unsigned char tmpDataProperty;
									memset(&msg, 0, sizeof(msg));
									emu_eng->CopyEaeDataToDeliverNetBuf((unsigned char *)msg.data+sizeof(link_relation_t), (unsigned short *)&msg.header.data_len, &tmpDataProperty);
									logDataToHex(param->apdu_log_fd,(unsigned char *)msg.data+sizeof(link_relation_t), msg.header.data_len, DIRE_EAE_TO_NET);
									// add for link relation data
									if  (tmpDataProperty==DATA_PROPERTY_IS_NETAPI_COMMAND){   //DATA_PROPERTY_IS_NETAPI_NODELIVER2NET 则不向网络传送
										memcpy(msg.data, (char *)&link_relation, sizeof(link_relation));
										msg.header.data_len += sizeof(link_relation);
										//memcpy((char *)&msg.header, buff_recv, sizeof(msg_header_t));
										msg.header.data_len = htons(msg.header.data_len);
										// 发送到simbank的网络报文的数据属性改成:复位、数据，组合三种，具体由函数:CopyEaeDataToDeliverNetBuf出参指定
										msg.header.cmd = htons(tmpDataProperty); //htons(CMD_IS_DATA);
										msg.header.slot = htons(param->slotinfo.slot_nbr);
										memset(buff_send, 0, sizeof(buff_send));
										len_send = makeReqPackage(SIMDATA_REQ, (char *)&msg, sizeof(msg_header_t)+ ntohs(msg.header.data_len), buff_send);

										ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send);
										if (ret != len_send)
										{
											zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sim data req package error(%d:%s)", \
												0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
											param->slotinfo.stat = SlotInvalid;
											//param->sockfd_netrdr = -1;
										}
									} else if (tmpDataProperty==DATA_PROPERTY_IS_NETAPI_NODELIVER2NET) {
										zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: error DATA_PROPERTY_IS_NETAPI_NODELIVER2NET.", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
									}
										
									//TransferToRemote_nts(&ActEae,&Actn2st);
									emu_eng->SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_WAITING_FROM_NET_READY);
									//logDataToHex((unsigned char *)&msg.data, ntohs(msg.header.data_len), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
								}	
								else if (emu_eng->GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_EMU_IS_READY)
								{
									memset((char *)&msg, 0, sizeof(msg));
									emu_eng->CopyEaeDataToEmuBuf((unsigned char *)msg.data+4, (unsigned short *)&len_send);
									//logDataToHex((unsigned char *)msg.data+4, len_send, 00, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SIMHDLTASK_TO_SLOTHDLTASK);
									msg.data[0] = param->slotinfo.slot_nbr; //0;
									msg.data[1] = CMD_IS_DATA; //0;
									msg.data[2] = len_send>>8;
									msg.data[3] = (len_send<<8)>>8;	
									msg.header.cmd = SIMDATA_RSP;
									msg.header.slot = param->slotinfo.slot_nbr;
									msg.header.data_len = len_send + 4;
									ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, msg.header.data_len+sizeof(msg_header_t));
									if ((unsigned int)ret != msg.header.data_len+sizeof(msg_header_t))
									{
										zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send data rsp package to CommHdlTask error(%d:%s)", \
											0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
									}
									DataToHex((unsigned char *)msg.data+3,len_send+1,DIRE_NET_TO_COMMHDLTASK,param->slotinfo.gw_bank_nbr,param->slotinfo.slot_nbr);
									logDataToHex(param->apdu_log_fd,(unsigned char *)msg.data+3,len_send+1,DIRE_NET_TO_COMMHDLTASK);
									//emu_eng->SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_IDLE);
								}
								
								else
								{
									zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: EmuEngine's ApduNetApi Status[%d] is not expected", \
										0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, emu_eng->GetEmuApduNetApiStatus());
									//sock_trans.CloseSocket();
									//param->slotinfo.stat = SlotInvalid;
								}
							}
							else
							{
								zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: EmuEngine's status[%d] is not EMU_ENGINE_WORKING", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, emu_eng->GetEmuEngineStatus());
								param->slotinfo.stat = SlotInvalid;
							}
						}
					}
				}
				if(events[n].events & EPOLLIN && events[n].data.fd == sock_comm.m_sockfd)
				{
					// read
					memset(buff_recv, 0, sizeof(buff_recv));
					len_recv = sock_comm.UDPReadData(sock_comm.m_sockfd, buff_recv, sizeof(buff_recv));
					if (len_recv < 0)
					{
						zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: read data from CommHdlTask error(%d:%s)", \
							0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
						continue;
					}
					pmsgh = (msg_header_t *)buff_recv;

					// 此处解析数据
					// slot 10 00 00
					if(((unsigned char *)(buff_recv+sizeof(msg_header_t)))[1] == IS_REQ_RST_ICC)
					{ // 回写sim atr
						emu_eng->SetEmuEngineStatus(EMU_ENGINE_WORKING);

						emu_eng->SetFromEmuDataProperty(DATA_PROPERTY_IS_REQ_RST_ICC);
						emu_eng->CopyFromEmuDataToEaeBuf((unsigned char *)buff_recv+sizeof(msg_header_t)+4, len_recv-sizeof(msg_header_t)-4, emu_eng->GetFromEmuDataProperty());
						emu_eng->SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_DATA_FROM_EMU_IS_READY);
						emu_eng->ProcessEmuDataCmd();
						if(emu_eng->GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY && time(NULL) -  icc_rst_time > 100)
						{
							unsigned char tmpDataProperty;
							memset(&msg, 0, sizeof(msg));
							emu_eng->CopyEaeDataToDeliverNetBuf((unsigned char *)msg.data+sizeof(link_relation), (unsigned short*)&msg.header.data_len, &tmpDataProperty);
							logDataToHex(param->apdu_log_fd,(unsigned char *)msg.data+sizeof(link_relation_t), msg.header.data_len, DIRE_EAE_TO_NET);
							memcpy(msg.data, (char *)&link_relation, sizeof(link_relation));
							msg.header.data_len += sizeof(link_relation);
							//memcpy((char *)&msg.header, buff_recv, sizeof(msg_header_t));
							msg.header.cmd = htons(tmpDataProperty); 
							msg.header.slot = htons(param->slotinfo.slot_nbr);
							msg.header.data_len = htons(msg.header.data_len);
							memset(buff_send, 0, sizeof(buff_send));
							len_send = makeReqPackage(SIMDATA_REQ, (char *)&msg, sizeof(msg_header_t)+ ntohs(msg.header.data_len), buff_send);
							//ret = sock_trans.WriteData(sock_trans.m_sockfd, buff_send, len_send);--------------------------
							ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send); // 改发送到nettask端口
							if (ret != len_send)
							{
								zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sim data req package[ret:%d] error(%d:%s)", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, ret, errno, strerror(errno));
								param->slotinfo.stat = SlotInvalid;
								//param->sockfd_netrdr = -1;
							}
							emu_eng->SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_ICC_RST_DELIVER_TO_REMOTE);
							time(&icc_rst_time);
						}
						emu_eng->ProcessEmuDataCmd();
					}
					else if (((unsigned char *)(buff_recv+sizeof(msg_header_t)))[1] == CMD_IS_DATA)
					{ 
						time(&icc_data_time);
						// 操作EmuEngine解析数据
						//emu_eng->SetEmuEngineStatus(EMU_ENGINE_WORKING);
						emu_eng->SetFromEmuDataProperty(DATA_PROPERTY_IS_DATA);
						//logDataToHex((unsigned char *)buff_recv+sizeof(msg_header_t)+4, len_recv-sizeof(msg_header_t)-4, 11, 11, 11, DIRE_UNKOWN);
						emu_eng->CopyFromEmuDataToEaeBuf((unsigned char *)buff_recv+sizeof(msg_header_t)+4, len_recv-sizeof(msg_header_t)-4, emu_eng->GetFromEmuDataProperty());
						emu_eng->SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_DATA_FROM_EMU_IS_READY);
						emu_eng->ProcessEmuDataCmd();
						DataToHex((unsigned char *)buff_recv+sizeof(msg_header_t)+3,len_recv-sizeof(msg_header_t)-3,DIRE_COMMHDLTASK_TO_SLOTHDLTASK,param->slotinfo.gw_bank_nbr,param->slotinfo.slot_nbr);
						logDataToHex(param->apdu_log_fd,(unsigned char *)buff_recv+sizeof(msg_header_t)+3,len_recv-sizeof(msg_header_t)-3,DIRE_COMMHDLTASK_TO_SLOTHDLTASK);
						if (emu_eng->GetEmuEngineStatus() == EMU_ENGINE_WORKING)
						{
							if (emu_eng->GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY)
							{
								// 组包，发送到网络远程
								unsigned char tmpDataProperty;
								memset(&msg, 0, sizeof(msg));
								emu_eng->CopyEaeDataToDeliverNetBuf((unsigned char *)msg.data+sizeof(link_relation), (unsigned short*)&msg.header.data_len, &tmpDataProperty);
								logDataToHex(param->apdu_log_fd,(unsigned char *)msg.data+sizeof(link_relation_t), msg.header.data_len, DIRE_EAE_TO_NET);
//								logDataToHex((unsigned char *)msg.data+sizeof(link_relation), msg.header.data_len, 00, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
								// add for link relation data
								if  (tmpDataProperty==DATA_PROPERTY_IS_NETAPI_COMMAND){   //DATA_PROPERTY_IS_NETAPI_NODELIVER2NET 则不向网络传送
									memcpy(msg.data, (char *)&link_relation, sizeof(link_relation));
									msg.header.data_len += sizeof(link_relation);
									//memcpy((char *)&msg.header, buff_recv, sizeof(msg_header_t));
									msg.header.cmd = htons(tmpDataProperty); //htons(CMD_IS_DATA);
									msg.header.slot = htons(param->slotinfo.slot_nbr);
									msg.header.data_len = htons(msg.header.data_len);
									memset(buff_send, 0, sizeof(buff_send));
									len_send = makeReqPackage(SIMDATA_REQ, (char *)&msg, sizeof(msg_header_t)+ ntohs(msg.header.data_len), buff_send);
									//ret = sock_trans.WriteData(sock_trans.m_sockfd, buff_send, len_send);--------------------------
									ret = sock_nettask.WriteData(sock_nettask.m_sockfd, buff_send, len_send); // 改发送到nettask端口
									if (ret != len_send)
									{
										zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sim data req package[ret:%d] error(%d:%s)", \
											0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, ret, errno, strerror(errno));
										param->slotinfo.stat = SlotInvalid;
										//param->sockfd_netrdr = -1;
									}
									else
									{
										//TransferToRemote_nts(&ActEae,&Actn2st);
										emu_eng->SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_WAITING_FROM_NET_READY);

										//logDataToHex((unsigned char *)&msg.data+sizeof(link_relation_t), ntohs(msg.header.data_len)-sizeof(link_relation_t), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
										//logDataToHex((unsigned char *)&msg.data, ntohs(msg.header.data_len), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
										//logDataToHex((unsigned char *)buff_send, (unsigned short)ret, 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
									}
								} else if (tmpDataProperty==DATA_PROPERTY_IS_NETAPI_NODELIVER2NET) {
									zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: error 2 DATA_PROPERTY_IS_NETAPI_NODELIVER2NET.", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
								}
							}	
							else if (emu_eng->GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_EMU_IS_READY)
							{
								buff_send[0] = param->slotinfo.slot_nbr; //0;
								buff_send[1] = CMD_IS_DATA; //0;
								buff_send[2] = (emu_eng->GetToEmuDataLength())>>8;
								buff_send[3] = ((emu_eng->GetToEmuDataLength())<<8)>>8;	
								emu_eng->CopyEaeDataToEmuBuf((unsigned char *)buff_send+4, (unsigned short *)&len_send);
								msg.header.cmd = SIMDATA_RSP;
								msg.header.slot = param->slotinfo.slot_nbr;
								msg.header.data_len = len_send + 4;
								memcpy(msg.data, buff_send, len_send+4);
								ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, msg.header.data_len+sizeof(msg_header_t));
								emu_eng->SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_IDLE);
								DataToHex((unsigned char *)msg.data+3,len_send+1,DIRE_SLOTHDLTASK_TO_COMMHDLTASK,param->slotinfo.gw_bank_nbr,param->slotinfo.slot_nbr);
								logDataToHex(param->apdu_log_fd,(unsigned char *)msg.data+3,len_send+1,DIRE_SLOTHDLTASK_TO_COMMHDLTASK);
							}
							else if(EMU_APDU_NETAPI_STATUS_DATA_FROM_EMU_IS_READY == emu_eng->GetEmuApduNetApiStatus())
							{
								emu_eng->ProcessEmuDataCmd();
							}
							else 
							{
								zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: EmuEngine's ApduNetApi Status[%d] is not expected", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, emu_eng->GetEmuApduNetApiStatus());
								//sock_trans.CloseSocket();
								//param->slotinfo.stat = SlotInvalid;
							}
						}
						else
						{
							chan_stat = 2;
							param->call_flag = 0;
							memset(param->slotinfo.sb_seri, 0, sizeof(param->slotinfo.sb_seri));
							param->slotinfo.sb_usb_nbr = 0;
							param->slotinfo.sb_bank_nbr = 0;
							param->slotinfo.sim_nbr = 0;

							memset(link_relation.sb_seri, 0, sizeof(link_relation.sb_seri));
							link_relation.sb_usb_nbr = 0;
							link_relation.sb_bank_nbr = 0;
							link_relation.sb_slot_nbr = 0;
							
							emu_eng->SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PULL_OUT);
							emu_eng->SetEmuEngineStatus(EMU_ENGINE_STOP);
							zprintf(ERROR,"[ERRO]SlotHdlTask[%02d-%02d-%02d]: EmuEngine's status[%d] is not EMU_ENGINE_WORKING", \
								0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, emu_eng->GetEmuEngineStatus());
							param->slotinfo.stat = SlotInvalid;
							param->slotinfo.link_stat = 0;
						}
					}
				}
			}
		}
	}

	//
	delete emu_eng;
	sock_comm.CloseSocket();
	zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: close sock_comm", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
	sock_nettask.CloseSocket();
	zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: close sock_nettask", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
	zprintf(INFO,"[INFO]SlotHdlTask[%02d-%02d-%02d]: End", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);

	//
	return NULL;
}

#if 0
void *SimLinkCreateRspHdl(void *pParam)
{
	nethdl_params_t *param = (nethdl_params_t *)pParam;
	int len = 0,len_recv = 0,recv_len = 0,link_reply_len = 0;
	int ret = 0,reg_pack_len;
	char link_reply_buff[PACKAGE_HEADER_LEN + 4];
	char buff_recv[0xffff];
	char reg_pack[256];
	CSocketEx vgsm_csock;
	CSocketEx sock_net;
	vgsm_csock.m_sockfd = -1;
	register_req_info_t register_req_info;
	char ast_pid_buff[32] = {0};
	cat_file(AST_RUN_STAT_FILE,ast_pid_buff);
	
	register_req_info.type = htons(SimEmuSvr);
	memcpy(register_req_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	reg_pack_len = makeRegisterPack(reg_pack, (char *)&register_req_info, sizeof(register_req_info), &reg_pack_len);

	link_reply_len = makeReqPackage(SIMLINK_CREATE_RSP,NULL,0,link_reply_buff);

	sock_net.Init(PROTO_UDP, gSimEmuSvrParam.local_ip, SERVER_PORT + 2, gSimEmuSvrParam.local_ip, gSimEmuSvrParam.net_hdl_port, 10);
	sock_net.CreateSocket();
	sock_net.Bind();
	zprintf(INFO,"[INFO]SimLinkCreateRspHdl: sock_nettask create succ,asterisk pid is %s",ast_pid_buff); 

	int epollfd = -1;
	struct epoll_event event;
	int epollnum = 0;
	epollfd = epoll_create(1);

	while(param->stop != 1)
	{
VGSM_START:		
		if(-1 == vgsm_csock.m_sockfd){
			zprintf(INFO,"vgsm sock rebuild !!!");
			Sleep(5000);
			vgsm_csock.Init(PROTO_TCP, gSimEmuSvrParam.local_ip,SERVER_PORT + 1, gSimEmuSvrParam.server_ip, 1025, 1);
			vgsm_csock.CreateSocket();
			if(false == vgsm_csock.Bind()){
				vgsm_csock.CloseSocket();
				zprintf(ERROR,"SimLinkCreateRspHdl:vgsm_csock bind error");
				continue;
			}
			
			ret = vgsm_csock.ConnectNonB(vgsm_csock.getSocket(), 10);
			if(false == ret)
			{
				vgsm_csock.CloseSocket();
				zprintf(ERROR,"[ERROR]SimLinkCreateRspHdl:connect vgsm port error");
				continue;
			}
			ret = vgsm_csock.WriteData(vgsm_csock.getSocket(),reg_pack,reg_pack_len);
			if(ret != reg_pack_len)
			{
				vgsm_csock.CloseSocket();
				zprintf(ERROR,"[ERROR]SimLinkCreateRspHdl:connect WriteData error");
				continue;
			}
			add_event(epollfd, vgsm_csock.getSocket(), EPOLLIN);
		}
		cat_file(AST_RUN_STAT_FILE,buff_recv);
		if(0 != strcmp(buff_recv,ast_pid_buff))
		{
			strcpy(ast_pid_buff,buff_recv);
			sprintf(buff_recv,"asterisk -rx 'gsm set sim disable'");
			system(buff_recv);
			zprintf(INFO,"[INFO]asterisk restart,set sim disable");
			param->stop = 1;
		}

		epollnum = epoll_wait(epollfd, &event, 1, 5000);
		if(epollnum != 1 || !(event.events & EPOLLIN) || event.data.fd != vgsm_csock.getSocket()){
			continue;
		}
		
		len_recv = 0;
		while(len_recv < PACKAGE_HEADER_LEN)
		{
			ret = vgsm_csock.ReadData(vgsm_csock.getSocket(), buff_recv + len_recv, PACKAGE_HEADER_LEN - len_recv);
			if(ret < 0)
			{
				zprintf(ERROR,"[ERRO]SimLinkCreateRspHdl[%d]: recv package header(ret:%d, header_len:%d) from SimProxySvr error(%d:%s)", \
					vgsm_csock.getSocket(),ret, PACKAGE_HEADER_LEN, errno, strerror(errno));
				delete_event(epollfd,vgsm_csock.getSocket(),EPOLLIN);
				vgsm_csock.CloseSocket();
				vgsm_csock.m_sockfd = -1;
				goto VGSM_START;
			} else if (ret == 0) {
				zprintf(ERROR,"[ERRO]SimLinkCreateRspHdl[%d]: recv package header(ret:%d) from SimProxySvr error(%d:%s)", \
					vgsm_csock.getSocket(),ret, errno, strerror(errno));
			}
			len_recv += ret;
		}		
		package_header_t *pheader = (package_header_t *)buff_recv;
		len = getLen(buff_recv) + 2;
		zprintf(INFO,"[INFO]SimLinkCreateRspHdl: package from net[version:0x%04x-cmd:0x%04x-len:0x%04x-result:0x%04x-reserve:0x%08x]", \
					ntohs(pheader->version), ntohs(pheader->cmd), ntohs(pheader->len), ntohs(pheader->result), ntohs(pheader->reserve));
		
		len_recv = 0;
		while(len)
		{
//			recv_len = len > 1300? 1300:len;
			recv_len = len > 1000? 1000:len;
			if (recv_len <= 0)
				break;
//			if(vgsm_csock.checkSocketReady(vgsm_csock.getSocket(),READ_WAIT,5))
//			{
/*				epollnum = epoll_wait(epollfd, &event, 1, 5000);
				if(epollnum != 1 || !(event.events & EPOLLIN) || event.data.fd != vgsm_csock.getSocket()){
					continue;
				}*/
				ret  = vgsm_csock.ReadData(vgsm_csock.getSocket(), buff_recv+PACKAGE_HEADER_LEN+len_recv,recv_len);
				if(ret < 0){
					zprintf(ERROR,"[ERRO]SimLinkCreateRspHdl[%d]: recv vgsm package from SimProxySvr error(%d:%s)", \
						ret,errno, strerror(errno));
//					delete_event(epollfd,vgsm_csock.getSocket(),EPOLLIN);
//					vgsm_csock.CloseSocket();
//					goto VGSM_START;
				} else if (ret == 0) {
					zprintf(ERROR,"[ERRO]SimLinkCreateRspHdl[%d]: recv vgsm package from SimProxySvr error(%d:%s)", \
						ret,errno, strerror(errno));
				} else {
					len_recv += ret;
					if (len > ret)
						len -= ret;
					else 
						len = 0;
					zprintf(INFO,"[INFO]SimLinkCreateRspHdl:recv %d bytes",ret);
					sock_net.WriteData(sock_net.getSocket(),link_reply_buff,link_reply_len);
				}
/*			}else{
				zprintf(WARN,"[INFO]SimLinkCreateRspHdl: continue");
				continue;
//				break;
			}*/
		}
		
		ret = checkPackage(buff_recv, PACKAGE_HEADER_LEN+ntohs(pheader->len)+2);
		if (ret != 0)
		{
			zprintf(ERROR,"[ERRO]NetDateRecv: package from SimRdrSvr checksum fail");
			delete_event(epollfd,vgsm_csock.getSocket(),EPOLLIN);
			vgsm_csock.CloseSocket();
			goto VGSM_START;
		}
		if((SIMLINK_CREATE_REQ == ntohs(pheader->cmd))){
			SimLinkCreateReqHdl(buff_recv,buff_recv,param->sock_sht);
		}

	}
	return NULL;
}
#endif

/**************************************************************************** 
* 函数名称 : NetHdlTask
* 功能描述 : 网络处理线程函数，连接SlotHdlTask和SimRdrSvr
* 参    数 : void *pParam					: 线程参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void * NetHdlTask(void *pParam)
{
	int ret = 0;
	//fd_set rset;
	//timeval tv;
	char buff_recv[0xffff] = {0};
	char buff_send[0xffff] = {0};
	int len = 0,len_recv = 0;
	//int max = 0;
	//unsigned int sockfd_net;
	time_t time_last = 0;
	AST_HANDLE_FUNC ast_hdl;
	NET_HANDLE_FUNC net_hdl;
	
	nethdl_params_t *param = (nethdl_params_t *)pParam;
	CSocketEx *psock_net = param->csock;//(CSocketEx *)pParam;
	//sockfd_net = psock_net->getSocket();
	CSocketEx sock_ast;
	CSocketEx *sock_sht = param->sock_sht;
	unsigned int sockfd_ast = 0;

	//package_header_t *pheader = NULL;

	zprintf(INFO,"[INFO]NetHdlTask: Start[net_hdl_port:%d, ast_hdl_port:%d]......", gSimEmuSvrParam.net_hdl_port, gSimEmuSvrParam.ast_hdl_port);
	// use to send msg to proxysvr for call begin and call end
	// maybe we have more than one ip,so use '0.0.0.0' to listen all of ethernet interface
	sock_ast.Init(PROTO_UDP, (char*)"0.0.0.0", gSimEmuSvrParam.ast_hdl_port, gSimEmuSvrParam.local_ip, 0, 1);
	sock_ast.CreateSocket();
	sock_ast.Bind();
	sockfd_ast = sock_ast.getSocket();
	
	int epollfd = -1;
	struct epoll_event events[3];
	int epollnum = 0;
	epollfd = epoll_create(3);
	add_event(epollfd, sock_sht->m_sockfd, EPOLLIN);
	add_event(epollfd, sock_ast.m_sockfd, EPOLLIN);
	add_event(epollfd,psock_net->m_sockfd,EPOLLIN);
	zprintf(ERROR,"[ERRO]NetHdlTask: epoll init ok.");
	
	while (param->stop != 1)
	{
		// heartbeat
REGISTER:
		if(REGISTER_DOWN == gRegisterStat.stat){
			if(NoErr == emuRegister(psock_net)){
				add_event(epollfd,psock_net->m_sockfd,EPOLLIN);
				sleep(15);	//ensure slot thread remove vitural card data
				upRegisterStat(&gRegisterStat);
			}
		}
		
		if ((time(NULL) - time_last) > gSimEmuSvrParam.hb_interval && REGISTER_UP == gRegisterStat.stat)
		{
			ret = EmuHeartbeat(psock_net);
			if (ret != NoErr)
			{
				delete_event(epollfd,psock_net->m_sockfd,EPOLLIN);
				psock_net->CloseSocket();
				downRegisterStat(&gRegisterStat);
				goto REGISTER;
			}
			time_last = time(NULL);
			if(last_seen - time_last > HB_INTERVAL * 10){
				zprintf(ERROR,"[ERROR]NetHdlTask:didn't receive heartbeat Over 12 minutes!!");
				param->stop = 1;
				continue;
			}
		}

		//
		epollnum = epoll_wait(epollfd, events, 3, 5000);
		if (epollnum < 0)
		{
			zprintf(ERROR,"[ERRO]NetHdlTask: epoll_wait error(%d:%s)", errno, strerror(errno));
			continue;
		}
		else if (epollnum == 0)
		{
			continue;
		}
		else
		{
			int n = 0;
			for (n = 0; n < epollnum; n++)
			{
				if (events[n].events & EPOLLIN && events[n].data.fd == sock_sht->m_sockfd)
				{ // data from sock_sht
					// read package
					memset(buff_recv, 0, sizeof(buff_recv));
					len= sock_sht->ReadData(sock_sht->m_sockfd, buff_recv, sizeof(buff_recv));
					if (len <= 0)
					{
						zprintf(ERROR,"[ERRO]NetHdlTask: recv package from SlotHdlTask error(%d:%s)", errno, strerror(errno));
					}
					else
					{
						// link report
						ret = psock_net->WriteData(psock_net->m_sockfd, buff_recv, len);
						if (ret != len)
						{
							zprintf(ERROR,"[ERRO]NetHdlTask: send package to SimProxy error(%d:%s)", errno, strerror(errno));
						}
					}
				}
				if (events[n].events & EPOLLIN && events[n].data.fd == sock_ast.m_sockfd)
				{ // data from sock_ast
					// asterisk port
					// read package
					memset(buff_recv, 0, sizeof(buff_recv));
					ret = sock_ast.ReadData(sockfd_ast, buff_recv, sizeof(buff_recv));
					if (ret < 0)
					{
						zprintf(ERROR,"[ERRO]NetHdlTask: recv package from Asterisk error(%d:%s)", errno, strerror(errno));
					} 
					else if (ret == 0) 
					{
						zprintf(ERROR,"[ERRO]NetHdlTask: recv package 0 from Asterisk error(%d:%s)", errno, strerror(errno));
					}
					else
					{
						// handle
						//printHex((unsigned char *)buff_recv, (unsigned short)ret);
						ast_hdl = findAstHandleFunc(getCmd(buff_recv));
						if (ast_hdl != NULL)
						{
							memset(buff_send, 0, sizeof(buff_send));
							ret = ast_hdl(buff_recv, buff_send, psock_net);
						}
					}
				}
				if (events[n].events & EPOLLIN && events[n].data.fd == psock_net->m_sockfd)
				{
					len_recv = 0;
					while(len_recv < PACKAGE_HEADER_LEN)
					{
						ret = psock_net->ReadData(psock_net->getSocket(), buff_recv + len_recv, PACKAGE_HEADER_LEN - len_recv);
						if(ret < 0)
						{
							zprintf(ERROR,"[ERRO]NetDateRecv: recv package header(ret:%d, header_len:%d) from SimProxySvr error(%d:%s), maybe SimRdrSvr close the connection", \
								ret, PACKAGE_HEADER_LEN, errno, strerror(errno));
							delete_event(epollfd,psock_net->m_sockfd,EPOLLIN);
							psock_net->CloseSocket();
							downRegisterStat(&gRegisterStat);
							goto REGISTER;
						} else if (ret == 0) {
							zprintf(ERROR,"[ERRO]NetDateRecv: recv package header(ret:%d ) from SimProxySvr error(%d:%s), maybe SimRdrSvr close the connection", \
								ret, errno, strerror(errno));
						}
						len_recv += ret;
					}
					
					//package_header_t *pheader = (package_header_t *)buff_recv;
					/*zprintf(ERROR,"[INFO]NetDateRecv: package from net[version:0x%04x-cmd:0x%04x-len:0x%04x-result:0x%04x-reserve:0x%08x]", \
						ntohs(pheader->version), ntohs(pheader->cmd), ntohs(pheader->len), ntohs(pheader->result), ntohs(pheader->reserve));*/
					// read package reload
					
					len = getLen(buff_recv);
					len_recv = 0;
					while(len_recv < len + 2)
					{
						ret = psock_net->ReadData(psock_net->getSocket(), buff_recv+PACKAGE_HEADER_LEN + len_recv, len+2 - len_recv); //<-- no checksum
						if(ret < 0)
						{
							zprintf(ERROR,"[ERRO]NetDateRece: recv package reload(ret:%d, reload_len:%d) error(%d:%s)", ret, len+2, errno, strerror(errno));
							delete_event(epollfd,psock_net->m_sockfd,EPOLLIN);
							psock_net->CloseSocket();
							downRegisterStat(&gRegisterStat);
							goto REGISTER;
						} else if (ret == 0){
							zprintf(ERROR,"[ERRO]NetDateRece: recv package reload(reload_len:%d) error(%d:%s)",  len+2, errno, strerror(errno));
						}
						len_recv += ret;
					}
					// check package
					ret = checkPackage(buff_recv, PACKAGE_HEADER_LEN+len+2);
					if (ret != 0)
					{
						zprintf(ERROR,"[ERRO]NetDateRecv: package from SimRdrSvr checksum fail");
						delete_event(epollfd,psock_net->m_sockfd,EPOLLIN);
						psock_net->CloseSocket();
						downRegisterStat(&gRegisterStat);
					}
					else
					{
						// handle
						net_hdl = findNetHandleFunc(getCmd(buff_recv));
						if (net_hdl != NULL)
						{
							memset(buff_send, 0, sizeof(buff_send));
							ret = net_hdl(buff_recv, buff_send, sock_sht);
						}
					}
				}
			}
		}
	}
	// unregister
	//emuUnRegister(psock_net);
	close(epollfd);
	//
	sock_sht->CloseSocket();
	sock_ast.CloseSocket();
	zprintf(INFO,"[INFO]NetHdlTask: End");
	return NULL;
}
/**************************************************************************** 
* 函数名称 : getCount
* 功能描述 : 获取计数
* 参    数 : void
* 返 回 值 : 累计计数
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned long getCount(void)
{
	static unsigned long sCount = 0;
	sCount++;
	return sCount;
}

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
int makeEmuDataPackage(msg_t *msg, char *buff, int len)
{
	unsigned short slot = 0;
	slot = ((unsigned char *)buff)[0];
	msg->header.cmd = SIMDATA_REQ; //htons(SIMDATA_REQ);
	msg->header.slot = slot; //htons(slot);
	msg->header.data_len = len; //htons(len);
	msg->header.cnt = getCount(); //htonl(getCount());
	memcpy(msg->data, buff, len);
	return 0;
}

int setSimAtr(int handle, char *sim_atr, unsigned short atr_len, unsigned short board_nbr, unsigned short slot_nbr)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int len = 0;
	int trys = 3;
	int ret = 0;
	int res = -1;

	if (handle < 0 || sim_atr == NULL)
	{
		return -1;
	}
	
	buff_req[0] = (unsigned char)slot_nbr;
	buff_req[1] = CMD_IS_SETATR;
	buff_req[2] = 0;
	buff_req[3] = (unsigned char)(atr_len);
	memcpy((char *)(buff_req+4), sim_atr, atr_len);
	len = 4 + atr_len;

	for (int i = 0; i < trys; i++)
	{
		memset(buff_rsp, 0, sizeof(buff_rsp));
		//ret = serial_wr_atr(handle, buff_req, len, buff_rsp, len, SERIAL_WR_TIMEOUT);
		//socket_pty_wr(csock, buff_req, len, buff_rsp, &ret, SERIAL_WR_TIMEOUT);
		ret = serial_wr_atr(handle, buff_req, len, buff_rsp, ret, SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == (unsigned char)slot_nbr && buff_rsp[1] == 0x80 && ret == (buff_rsp[3]+4))
		{
 			zprintf(INFO,"[INFO]setSimAtr[%02d-%02d-%02d]: set sim atr succ", 0, board_nbr, slot_nbr);
			res = 0;
			break;
		}
		else
		{
 			zprintf(ERROR,"[ERRO]setSimAtr[%02d-%02d-%02d]: set sim atr error(%d:%s)", 0, board_nbr, slot_nbr, errno, strerror(errno));
		}
	}
	return res;
}

// 暂时只检查包体长度
int checkEmuData(char *buff, int len)
{
	unsigned short len_body;
	if (buff == NULL || len <= 0)
	{
		return -1;
	}
	len_body = ((unsigned char)buff[2]<<8) + (unsigned char)buff[3];
	if ((len - 4) == len_body)
	{
		return 0;
	}
	return -1;
}

int socket_pty_w(CSocketEx *csock, unsigned char *buff_w, int len_w, int timeout)
{
	return csock->WriteData(csock->m_sockfd, (char *)buff_w, len_w);
}

int socket_pty_wr(CSocketEx *csock, unsigned char *buff_w, int len_w, unsigned char *buff_r, int *len_r, int timeout)
{
	int ret = 0;
	//timeval tv;
	//fd_set rset;
	int readn = 0;
	int len_body = 0;
	
	ret = csock->WriteData(csock->m_sockfd, (char *)buff_w, len_w);
	if (ret != len_w)
	{
		zprintf(ERROR,"[ERROR]socket_pty_wr: write data to socket[%d][len_w:%d, ret:%d] error(%s:%d)", csock->m_sockfd, len_w, ret, strerror(errno), errno);
		return -1;
	}
	while (1)
	{
		if (csock->checkSocketReady(csock->m_sockfd, READ_WAIT, 0, 600000))
		{
			ret = csock->UDPReadData(csock->m_sockfd, (char *)buff_r+readn, 320);
			if (ret < 0)
			{
				zprintf(ERROR,"[ERROR]socket_pty_wr: read data from socket[%d] error(%s:%d)", csock->m_sockfd, strerror(errno), errno);
				*len_r = readn;
				return readn;
			}
			else
			{
				readn += ret;
				len_body = (buff_r[2]<<8) + buff_r[3];
				if (len_body <= (readn - 4))
				{
					break;
				}
				//logDataToHex((unsigned char *)buff_r, (unsigned short)readn, 0, 80, 80, DIRE_COMM_TO_COMMHDLTASK);
				//zprintf("[INFO]socket_pty_wr: +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
			}
		}
		else
		{
			break;
		}
	}
	*len_r = readn;
	return readn;
}

/**************************************************************************** 
* 函数名称 : CommHdlTask
* 功能描述 : 串口处理线程函数
* 参    数 : void *pParam					: 串口线程函数参数
* 返 回 值 : void *
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void *CommHdlTask(void *pParam)
{
	int ret = 0;
	CSocketEx sock_sht;
	fd_set rset;
	timeval tv;
	char buff[320] = {0};
	char *pbuff = NULL;
	int plen = 0;
	int pos = 0;
	int len = 0;
	msg_t msg;
	//unsigned int sockfd = 0;
	unsigned char slot_nbr = 0;
	commhdltask_param_t *param = (commhdltask_param_t *)pParam;
	struct timeval tv_read;
	struct timeval tv_write;
	//unsigned long total_emu = 0;
	//unsigned long total_proc = 0;
	//int i = 0;
	//char cmd[320] = {0};
	int burst = 0,chan_no = 0;
	char bsp_cmd[128];
	
	zprintf(INFO,"[INFO]CommHdlTask[00-%02d]: Start[ip:%s, port:%d]......", param->board_nbr, param->ip, param->port);

	
	// sock open
	sock_sht.Init(PROTO_UDP, param->ip, param->port, param->ip, 0, 1);
	sock_sht.CreateSocket();
	sock_sht.Bind();
	//sockfd = sock_sht.getSocket();

	gettimeofday(&tv_read, NULL);
	gettimeofday(&tv_write, NULL);
	while (param->stop != 1)
	{
		if (param->handle < 0) // need initialize to -1 in main func
		{
			param->handle = open_serial(param->tty, param->baudrate, 0, 8, 1, 0);
			if (param->handle < 0)
			{
				zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d]: open comm(%s:%d) error(%d:%s)", param->board_nbr, param->tty, param->baudrate, errno, strerror(errno));
				Sleep(3000);
				continue;
			}
			zprintf(INFO,"[INFO]CommHdlTask[00-%02d]: open comm(%s:%d) succ", param->board_nbr, param->tty, param->baudrate);
			tcflush(param->handle, TCIOFLUSH);

			Sleep(500);
			// reset STM32F103
			resetSTM32(param->handle, param->board_nbr);
			// get STM32F103 version
			getSTM32Version(param->handle, param->board_nbr,buff,sizeof(buff));
			// reset Mini52
			resetAllMini52(param->handle, param->board_nbr);
			memset(buff, 0, sizeof(buff));
			getAllMini52Version(param->handle, param->board_nbr,buff);
			closeallSimbus(param->handle);
			param->initialed = 1;
		}

		// read from comm and send to SlotHdlTask
		memset(buff, 0, sizeof(buff));
		memset((char *)&msg, 0, sizeof(msg));
		//Sleep(5);

		//logDataToHex((unsigned char *)"\x80\x97\x00\x00", 4, 0, param->board_nbr, (unsigned short)80, DIRE_COMMHDLTASK_TO_COMM);
		len = serial_wr(param->handle, (unsigned char *)"\x80\x97\x00\x00", 4, (unsigned char *)buff, (unsigned int)sizeof(buff), SERIAL_WR_TIMEOUT);
		if (len > 0)
		{
			//if (((unsigned char *)buff)[0] == 0x80 && ((unsigned char *)buff)[1] == 0x94)
			//{
				//logDataToHex((unsigned char *)buff, (unsigned short)len, 0, param->board_nbr, 80, DIRE_COMM_TO_COMMHDLTASK);
				//zprintf("[INFO]CommHdlTask[00-%02d]: burst data from Emu", param->board_nbr);
			//}
			if (checkEmuData(buff, len) == 0)
			{
				if (((unsigned char *)buff)[0] == 0x80 && ((unsigned char *)buff)[1] == 0x94)
				{
					pos = 4;
					burst = 1;
				}
				else
				{
					pos = 0;
					burst = 0;
				}
				while (pos < len)
				{
					pbuff = buff + pos;
					if (burst == 1)
					{
						//pbuff = buff + pos;
						plen = ((unsigned char)pbuff[2]<<8) + (unsigned char)pbuff[3] + 4;
						if (plen > 320)
						{
							zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d]: Data from Emu invalid!!!", param->board_nbr);
 							break;
						}
						pos += plen;
					}
					else
					{
						//pbuff = buff;
						plen = len;
						pos += len;
					}
					if (((unsigned char *)pbuff)[1] == CMD_IS_DATA || ((unsigned char *)pbuff)[1] == IS_REQ_RST_ICC)
					{
						slot_nbr = ((unsigned char *)pbuff)[0];
			
						makeEmuDataPackage(&msg, pbuff, plen);

						if (slot_nbr > 7 && slot_nbr != SLOT_IS_EMU_CTRL){
							zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d]: slot_nbr(%d) invalid!!!", param->board_nbr, slot_nbr);
						}
						else
						{
							if (slot_nbr == SLOT_IS_EMU_CTRL) // must be delete
							{ // STM32F1xx data, send to SlotHdlTask[0]
								ret = sock_sht.WriteData(sock_sht.getSocket(), (char *)&msg, sizeof(msg.header)+msg.header.data_len, gSlotHdlTaskParam[param->board_nbr][0].local_ip, gSlotHdlTaskParam[param->board_nbr][0].port_comm);
								//logDataToHex((unsigned char *)msg.data, (unsigned short)msg.header.data_len, 0, param->board_nbr, (unsigned short)slot_nbr, DIRE_COMMHDLTASK_TO_SLOTHDLTASK);
							}
							else
							{ // Mini52 data
								ret = sock_sht.WriteData(sock_sht.getSocket(), (char *)&msg, sizeof(msg.header)+msg.header.data_len, gSlotHdlTaskParam[param->board_nbr][slot_nbr].local_ip, gSlotHdlTaskParam[param->board_nbr][slot_nbr].port_comm);
							}
						}
					}
					else
					{
						if (((unsigned char *)pbuff)[0] == 0x80 && ((unsigned char *)pbuff)[1] == 0x96 && ((unsigned char *)pbuff)[2] == 0 && ((unsigned char *)pbuff)[3] == 0 && plen == 4)
						{
							//logDataToHex((unsigned char *)buff, (unsigned short)len, 0, param->board_nbr, (unsigned short)80, DIRE_COMM_TO_COMMHDLTASK);
						}
						else
						{
							zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d]: slot_nbr(%d) invalid", param->board_nbr, slot_nbr);
							extern int apdu_switch;
							apdu_switch = 1;
							DataToHex((unsigned char *)pbuff,plen,"emu reset debug",param->board_nbr, slot_nbr);
							apdu_switch = 0;
						}
					}
				}
			}
			else
			{
				zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d]: Data From Emu Serial Error", param->board_nbr);
				//close_serial(param->handle);
				//param->handle = -1;
				continue;
			}
		}
		else
		{
			zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d]: socket_pty_wr[80 97 00 00] error(%d:%s)", param->board_nbr, errno, strerror(errno));
			//close_serial(param->handle);
			//param->handle = -1;
			Sleep(100);
			continue;
			// close socket to pty
			//sock_pty.CloseSocket();
			//zprintf("[INFO]CommHdlTask[00-%02d]: close socket to pty(%s)", param->board_nbr, g_tty_info.ip[param->board_nbr]);
			//Sleep(100);
		}


		// wait data from SlotHdlTask and write to comm
		FD_ZERO(&rset);
		FD_SET((unsigned int)(sock_sht.getSocket()), &rset);
		tv.tv_sec = 0;
		tv.tv_usec = 5000;
		
		ret = select(sock_sht.getSocket()+1, &rset, NULL, NULL, &tv);
		if (ret < 0)
		{
			zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d]: select error(%d:%s)", param->board_nbr, errno, strerror(errno));
			sock_sht.CloseSocket();
			sock_sht.Init(PROTO_UDP, param->ip, param->port, param->ip, 0, 1);
			sock_sht.CreateSocket();
			sock_sht.Bind();
		}
		else if (ret == 0)
		{
			continue;
		}
		else
		{
			memset((char *)&msg, 0, sizeof(msg));
			ret = sock_sht.UDPReadData(sock_sht.getSocket(), (char *)&msg, sizeof(msg));
			if (ret <= 0)
			{
				zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d]: recv data from SlotHdlTask error(%d:%s)", param->board_nbr, errno, strerror(errno));
			}
			else
			{
				switch(msg.header.cmd)
				{
						
					case SIMDATA_RSP:
						//logDataToHex((unsigned char *)&msg.data, msg.header.data_len, 0, param->board_nbr, (unsigned short)((unsigned char *)msg.data)[0], DIRE_SLOTHDLTASK_TO_COMMHDLTASK);
						if (((unsigned char *)msg.data)[1] == CMD_IS_SETATR)
						{
							//ret = socket_pty_wr(&sock_pty, (unsigned char *)msg.data, (unsigned int)msg.header.data_len, (unsigned char *)buff, &len, SERIAL_WR_TIMEOUT);
							len = serial_wr(param->handle, (unsigned char *)msg.data, (unsigned int)msg.header.data_len, (unsigned char *)buff, len, SERIAL_WR_TIMEOUT);
						}
						else
						{
							//len = socket_pty_w(&sock_pty, (unsigned char *)msg.data, (unsigned int)msg.header.data_len, SERIAL_WR_TIMEOUT);
							len = serial_write_n(param->handle, (unsigned char *)msg.data, (unsigned int)msg.header.data_len, SERIAL_WR_TIMEOUT);
							if (len != msg.header.data_len)
							{
								zprintf(ERROR,"[ERRO]CommHdlTask[00-%02d-%02d]: write data[len_w:%d, ret:%d] to serial error(%d:%s)", param->board_nbr, msg.header.slot, msg.header.data_len, len, errno, strerror(errno));
							}
						}
						break;
						
					case SIMLINK_CREATE_REQ:
						//zprintf("[INFO]CommHdlTask[%d]: receive SIMLINK_CREATE_REQ(slot:%d) .........", param->board_nbr, msg.header.slot);
						//resetMini52(&sock_pty, param->board_nbr, msg.header.slot);
						//zprintf("[INFO]CommHdlTask[%d]: set sim(slot:%d) atr succ, within link create", param->board_nbr, msg.header.slot);
						// 开始轮询emu
						//ena_wr_comm = 1;
						
						//gsm power reset

						if(0 > setSimAtr(param->handle, msg.data, msg.header.data_len, param->board_nbr, msg.header.slot))
						{
							close_serial(param->handle);
							param->handle = -1;
							break;
						}
						if(0 > openSimbus(param->handle,msg.header.slot))
						{
							close_serial(param->handle);
							param->handle = -1;
							break;
						}
						chan_no = get_port_from_map(param->board_nbr * SLOT_NBR + msg.header.slot + 1);
						sprintf(bsp_cmd,"asterisk -rx 'gsm set remotesim %d 1'",chan_no);
						system(bsp_cmd);
						zprintf(INFO,"[INFO]CommHdlTask[%02d-%02d]:module power reset %d channel success",param->board_nbr, msg.header.slot,chan_no);
						break;
						
					case SIMLINK_RELEASE_REQ:
					case SIM_PULLPLUG_NOTICE_REQ:
						//Mini52 must reset after module power off
						//ret = resetMini52WithoutSleep(param->handle, param->board_nbr, msg.header.slot);
						chan_no = get_port_from_map(param->board_nbr * SLOT_NBR + msg.header.slot + 1);
						if(0 > closeSimbus(param->handle,msg.header.slot))
						{						
							close_serial(param->handle);
							param->handle = -1;
						}
						sprintf(bsp_cmd,"asterisk -rx 'gsm set remotesim %d 0'",chan_no);
						system(bsp_cmd);
						
						break;
						
					case POWER_RESET_SPAN_REQ:
						zprintf(ERROR,"[ERROR]CommHdlTask[00-%02d-%02d]: gsm_power_reset_%d[%s:%d] succ for channel offline timeout");
						break;
					default:
						zprintf(INFO,"[INFO]CommHdlTask[00-%02d-%02d]: data from SlotHdlTask invalid", param->board_nbr, msg.header.slot);
						break;
				}
			}
		}
	}
	sock_sht.CloseSocket();
	close_serial(param->handle);
	
	zprintf(INFO,"[INFO]CommHdlTask[00-%02d]: End", param->board_nbr);
	//
	return NULL;
}

int start_master_mode(int board_nbr)
{
	int inc = 0;
	int i = 0;
	int j = 0;
	int ret = 0;

	zprintf(INFO,"[INFO]============================== start_master_mode Start ==============================");
	//create_local_pty();

START:
	// register
	if (emuRegister(&gSockReg) != NoErr)
	{
		zprintf(M_ERROR,"[ERROR]emuRegister error");
		Sleep(3000);
		goto START;
	}
	upRegisterStat(&gRegisterStat);
	zprintf(INFO,"[INFO]main: up register stat succ");

	CSocketEx sock_sht;
	// only send msg temporary
	sock_sht.Init(PROTO_UDP, gSimEmuSvrParam.local_ip, gSimEmuSvrParam.net_hdl_port, gSimEmuSvrParam.local_ip, 0, 1);
	sock_sht.CreateSocket();
	sock_sht.Bind();

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr,1024 * 1024);
	if(0 != ret){
		zprintf(ERROR,"[%ld][ERROR]pthread_attr_setstacksize set stack error(%d:%s)\n",errno, strerror(errno));
		exit(-1);
	}

	memset((char *)&gNetHdlParams, 0, sizeof(gNetHdlParams));
	gNetHdlParams.stop = 0;
	gNetHdlParams.csock = &gSockReg;
	gNetHdlParams.sock_sht = &sock_sht;
	//pthread_create(&gNetHdlTaskTid, NULL, NetHdlTask, (void *)&gSockReg);
	pthread_create(&gNetHdlTaskTid, &attr, NetHdlTask, (void *)&gNetHdlParams);

	//pthread_t vgsm_tid;
	//pthread_create(&vgsm_tid, &attr, SimLinkCreateRspHdl, (void *)&gNetHdlParams);


	pthread_t soap_tid;
	int port = 8868;
	pthread_create(&soap_tid,&attr,emu_soapservice,(void *)&port);
		
	//
	for (j = 0; j < board_nbr; j++)
	{
		strcpy(gCommHdlTaskParam[j].tty, g_ttyUSBx.grp[j].ttyUSB_Emu.dev);
		if (strlen(gCommHdlTaskParam[j].tty) > 0)
		{ 
			zprintf(INFO,"[INFO] gCommHdlTaskParam[j].tty = %s",gCommHdlTaskParam[j].tty);
			// 需要重新构建数据结构，最外层为板卡编号
			// 涉及到网络报文格式和simbank和SimServer的相关数据结构，需要修改

			
			// CommHdlTask start
			gCommHdlTaskParam[j].stop = 0;
			gCommHdlTaskParam[j].handle = -1;
			gCommHdlTaskParam[j].board_nbr = j;
			strcpy(gCommHdlTaskParam[j].ip, gSimEmuSvrParam.local_ip);
			gCommHdlTaskParam[j].port = gSimEmuSvrParam.comm_hdl_port + j;
			gCommHdlTaskParam[j].initialed = 0;
			//gCommHdlTaskParam[j].baudrate = gSimEmuSvrParam.baudrate;
			gCommHdlTaskParam[j].baudrate = g_ttyUSBx.grp[j].ttyUSB_Emu.baud;
			//memcpy(gCommHdlTaskParam[j].ami_ip, local, gSimEmuSvrParam.local_ip, strlen(gSimEmuSvrParam.local_ip));
#if 0			
			pthread_attr_init(&gCommHdlTaskParam[j].attr);
			pthread_attr_setinheritsched(&gCommHdlTaskParam[j].attr, PTHREAD_EXPLICIT_SCHED);
			pthread_attr_setschedpolicy(&gCommHdlTaskParam[j].attr, SCHED_RR);
			gCommHdlTaskParam[j].thr_param.sched_priority = sched_get_priority_max(SCHED_RR);
			pthread_attr_setschedparam(&gCommHdlTaskParam[j].attr, &gCommHdlTaskParam[j].thr_param);
			pthread_attr_setscope(&gCommHdlTaskParam[j].attr, PTHREAD_SCOPE_SYSTEM);
			ret = pthread_create(&gCommHdlTaskParam[j].tid, &gCommHdlTaskParam[j].attr, CommHdlTask, &gCommHdlTaskParam[j]);
#else
			ret = pthread_create(&gCommHdlTaskParam[j].tid, &attr, CommHdlTask, &gCommHdlTaskParam[j]);
#endif
			if (ret != 0)
			{
				zprintf(ERROR,"[ERROR]pthread_create CommHdlTask(%d) thread error(%d:%s)\n", j, errno, strerror(errno));
				continue;
			}
			else
			{
				zprintf(INFO,"[INFO]pthread_create CommHdlTask(%d) thread succ", j);
			}

			
			// SlotHdlTask start
			for (i = 0; i < SLOT_NBR; i++)
			{
				if (strlen(g_ttyUSBx.grp[j].ttyUSB_Gsm[i].dev) <= 0) // 如果没有插入GSM模块，则不启动该线路处理线程
				{
					zprintf(ERROR,"[ERROR]g_ttyUSBx.grp[%d].ttyUSB_Gsm[%d].dev is NULL",j,i);
					continue;
				}
				gSlotHdlTaskParam[j][i].slotinfo.link_nbr = inc;
				gSlotHdlTaskParam[j][i].slotinfo.gw_bank_nbr= j;
				gSlotHdlTaskParam[j][i].slotinfo.board_nbr = j;
				gSlotHdlTaskParam[j][i].slotinfo.slot_nbr = i;
				gSlotHdlTaskParam[j][i].slotinfo.stat = SlotInvalid;
				gSlotHdlTaskParam[j][i].slotinfo.link_stat = 0;
				strcpy(gSlotHdlTaskParam[j][i].local_ip, gSimEmuSvrParam.local_ip);
				gSlotHdlTaskParam[j][i].port_rdr = gSimEmuSvrParam.slot_port_rdr + j*SLOT_NBR+i;//inc;//i;
				gSlotHdlTaskParam[j][i].port_comm = gSimEmuSvrParam.slot_port_comm + j*SLOT_NBR+i;//inc;//i;
				gSlotHdlTaskParam[j][i].port_nettask = gSimEmuSvrParam.slot_port_net + j*SLOT_NBR+i;//inc;//i;
				gSlotHdlTaskParam[j][i].sockfd_comm = -1;
				gSlotHdlTaskParam[j][i].sockfd_netrdr = -1;
				gSlotHdlTaskParam[j][i].sockfd_nettask = -1;
				gSlotHdlTaskParam[j][i].stop = 0;
				memset(gSlotHdlTaskParam[j][i].sim_atr, 0, sizeof(gSlotHdlTaskParam[j][i].sim_atr));
				gSlotHdlTaskParam[j][i].len_atr = 0;				
				gSlotHdlTaskParam[j][i].call_flag = 0;
				gSlotHdlTaskParam[j][i].call_begin_time = 0;
				gSlotHdlTaskParam[j][i].call_rest_time = 0;
				gSlotHdlTaskParam[j][i].apdu_log_fd = open_apdu_log_file(j,i);
#if 0
				pthread_attr_init(&gSlotHdlTaskParam[j][i].attr);
				pthread_attr_setinheritsched(&gSlotHdlTaskParam[j][i].attr, PTHREAD_EXPLICIT_SCHED);
				pthread_attr_setschedpolicy(&gSlotHdlTaskParam[j][i].attr, SCHED_RR);
				gSlotHdlTaskParam[j][i].thr_param.sched_priority = sched_get_priority_max(SCHED_RR);
				pthread_attr_setschedparam(&gSlotHdlTaskParam[j][i].attr, &gSlotHdlTaskParam[j][i].thr_param);
				pthread_attr_setscope(&gSlotHdlTaskParam[j][i].attr, PTHREAD_SCOPE_SYSTEM);
				//pthread_create(&(gSlotHdlTaskTid[j][i]), NULL, SlotHdlTask, &(gSlotHdlTaskParam[j][i]));
				pthread_create(&(gSlotHdlTaskTid[j][i]), &gSlotHdlTaskParam[j][i].attr, SlotHdlTask, &(gSlotHdlTaskParam[j][i]));
#else
				ret = pthread_create(&(gSlotHdlTaskTid[j][i]), &attr, SlotHdlTask, &(gSlotHdlTaskParam[j][i]));
				if (ret != 0)
				{
					zprintf(ERROR,"[ERROR]pthread_create SlotHdlTask(%d) thread error(%d:%s)\n", i, errno, strerror(errno));
				}
#endif
				inc++;
			}
			Sleep(1000);
		}
	}
	//g_link_nbr = inc;
	
	Sleep(2000);
	// NetHdlTask start
#if 0	
	pthread_attr_init(&attr_nethdltask);
	pthread_attr_setinheritsched(&attr_nethdltask, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_nethdltask, SCHED_RR);
	param_nethdltask.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&attr_nethdltask, &param_nethdltask);
	pthread_attr_setscope(&attr_nethdltask, PTHREAD_SCOPE_SYSTEM);
	//pthread_create(&gNetHdlTaskTid, NULL, NetHdlTask, (void *)&gSockReg);
	pthread_create(&gNetHdlTaskTid, &attr_nethdltask, NetHdlTask, (void *)&gSockReg);
#else
	//pthread_create(&gNetHdlTaskTid, NULL, NetHdlTask, (void *)&gSockReg);
#endif
	Sleep(2000);

	//////////////////////////////////////////////////////////////////////////////////////////////////////

	
	// NetHdlTask stop
	pthread_join(gNetHdlTaskTid, NULL);
	pthread_attr_destroy(&attr_nethdltask);

	
	for (j = 0; j < board_nbr; j++)
	{
		if (strlen(gCommHdlTaskParam[j].tty) > 0)
		{
		 	for (i = 0; i < SLOT_NBR; i++)
		 	{
		 		gSlotHdlTaskParam[j][i].stop = 1;
		 	}
		}
		gCommHdlTaskParam[j].stop = 1;
	}
	
	// SlotHdlTask stop
	for (j = 0; j < board_nbr; j++)
	{
		if (strlen(gCommHdlTaskParam[j].tty) > 0)
		{
		 	for (i = 0; i < SLOT_NBR; i++)
		 	{
		 		pthread_join(gSlotHdlTaskTid[j][i], NULL);
				pthread_attr_destroy(&gSlotHdlTaskParam[j][i].attr);
		 	}
		}
		pthread_join(gCommHdlTaskTid[j], NULL);
		pthread_attr_destroy(&gCommHdlTaskParam[j].attr);
	}

	//
	gSockReg.CloseSocket();
	// kill all socat here

	zprintf(INFO,"[INFO]============================== start_master_mode End ==============================");
	return 0;
}


// 检查进程是否启动，没有启动返回0
int checkProcessExist(char *proc_name)
{
    int fd = open(proc_name,O_RDONLY|O_CREAT);
    if(-1 != fd){
        if(-1 ==  flock(fd,LOCK_EX|LOCK_NB)){
            fprintf(stderr,"%s is running...\n",proc_name);
            close(fd);
            return -1;
        }
    }
    else{
        fprintf(stderr,"%s::No such file or directory !!!\n",proc_name);
        return -1;
    }
    return 0;
}

//
int get_emu_info(ttyUSBx_t *ttyUSBx)
{
	int i = 0;
	char domain[16] = {0};
	char baud[16] = {0};

	get_config((char*)SIMEMUSVR_CONFIG_FILE, (char*)"SimEmuSvr", (char*)"baudrate", baud);
	if (strlen(baud) <= 0)
	{
		strcpy(baud, "460800");
	}

	for (i = 0; i < MAX_BOARD; i++)
	{
		sprintf(domain, "emu_%d", i+1);
		get_config((char*)EMU_INFO_FILE, (char*)"emu", domain, ttyUSBx->grp[i].ttyUSB_Emu.dev);
		if (strstr(ttyUSBx->grp[i].ttyUSB_Emu.dev, "/dev/opvx/emu") == NULL)
		{
			memset(ttyUSBx->grp[i].ttyUSB_Emu.dev, 0, sizeof(ttyUSBx->grp[i].ttyUSB_Emu.dev));
		}
		a_trim(ttyUSBx->grp[i].ttyUSB_Emu.dev, ttyUSBx->grp[i].ttyUSB_Emu.dev);
		ttyUSBx->grp[i].ttyUSB_Emu.baud = atoi(baud);
	}
	return 0; 
}
int get_gsm_info(ttyUSBx_t *ttyUSBx)
{
	int i = 0,j = 0;
	int m = 0;
	int n = 0;
	int ret = 0;
	char item[16] = {0};
	char context[32] = {0};

	
	memset((char *)&port_map_info, 0, sizeof(port_map_info));
	for (i = 0; i < MAX_CHN; i++)
	{
		sprintf(item, "chan_%d_type", i+1);
		ret = get_config((char*)GSM_INFO_FILE, (char*)"channel", item, context);
		if (1 != ret || strlen(context) <= 0)
		{
			continue;
		}
		else
		{
			m = i/SLOT_NBR;
			n = i%SLOT_NBR;
			sprintf(ttyUSBx->grp[m].ttyUSB_Gsm[n].dev,"%d",i + 1);
			ttyUSBx->grp[m].ttyUSB_Gsm[n].chn = i + 1;

			port_map_info.nbr++;
			port_map_info.port_map[i].port = i+1;
			port_map_info.port_map[i].dev = i+1;
			zprintf(INFO,"port_map_info: dev-%d = %d", port_map_info.port_map[i].port, port_map_info.port_map[i].dev);

			a_trim(context,context);
			for(j = 0;j < (int)(sizeof(gModules)/sizeof(struct module_name_s));j++)
			{
				if(0 == strncmp(context,gModules[j].module_name,strlen(gModules[j].module_name)))
				{
					gSlotHdlTaskParam[m][n].module_type = gModules[j].module_type;
					zprintf(INFO,"gSlotHdlTaskParam[%d][%d].module_type = %s",m,n,gModules[j].module_name);
					break;
				}
			}
		}
		memset(context,0,32);
	}
	return 0; 
}


int check_emu_tty(ttyUSBx_t *ttyUSBx)
{
	int i = 0;
	int fd = -1;
	int len = 0;
	char buff[320] = {0};

	for (i = 0; i < MAX_BOARD; i++)
	{
		if (strlen(ttyUSBx->grp[i].ttyUSB_Emu.dev) > 0)
		{
			fd = open_serial(ttyUSBx->grp[i].ttyUSB_Emu.dev, ttyUSBx->grp[i].ttyUSB_Emu.baud, 0, 8, 1, 0);
			if (fd < 0)
			{
				zprintf(ERROR,"[ERRO]check_emu_tty: open comm(%s:%d) error(%d:%s)", ttyUSBx->grp[i].ttyUSB_Emu.dev, ttyUSBx->grp[i].ttyUSB_Emu.baud, errno, strerror(errno));
				memset(ttyUSBx->grp[i].ttyUSB_Emu.dev, 0, sizeof(ttyUSBx->grp[i].ttyUSB_Emu.dev));
				ttyUSBx->grp[i].ttyUSB_Emu.baud = -1;
				continue;
			}
			memset(buff, 0, sizeof(buff));
			len = serial_wr(fd, (unsigned char *)"\x80\x83\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
			if (len > 0)
			{
				if (((unsigned char *)buff)[0] != SLOT_IS_EMU_CTRL || ((unsigned char *)buff)[1] != IS_VER)
				{
					zprintf(ERROR,"[ERROR]check_emu_tty: Emu Version [%s]", buff+4);
					//memset(ttyUSBx->grp[i].ttyUSB_Emu.dev, 0, sizeof(ttyUSBx->grp[i].ttyUSB_Emu.dev));
					//ttyUSBx->grp[i].ttyUSB_Emu.baud = -1;
				}
			}else{
				ttyUSBx->grp[i].ttyUSB_Emu.dev[0] = '\0';
			}


			close_serial(fd);
			fd = -1;
		}
	}
	return 0;
}

int	init_redis_SMS()
{
	char rds_cmd[256] = {0};

	sprintf(rds_cmd,"del %s",REDIS_KEY_SIMBANK_SMS_INFO);
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);

	if (context->err)
	{
		zprintf(ERROR,"[ERRO]setRedisofChannelSMSReceiveStat: redisConnect error(%s)",context->errstr);
		return -1;
	}
	else
	{
		redisReply *reply = (redisReply *)redisCommand(context, rds_cmd);
		if (reply->type == REDIS_REPLY_ERROR)
		{
			zprintf(ERROR,"[ERRO]setRedisofChannelSMSReceiveStat: redisCommand(%s) error(%s)", \
				rds_cmd, reply->str);
		}
		else
		{
			zprintf(INFO,"[INFO]setRedisofChannelSMSReceiveStat: redisCommand(%s) succ", \
				rds_cmd);
		}
		freeReplyObject((void *)reply);
	}
	redisFree(context);
	return 0;
}

int init_ttyUSBx(ttyUSBx_t *ttyUSBx)
{
	int i = 0;
	int j = 0;

	if (access(EMU_INFO_FILE, F_OK) != 0)
	{
		zprintf(ERROR,"[ERROR] Can't open /tmp/hwinfo");
		exit(-1);
	}

	get_emu_info(ttyUSBx);
	get_gsm_info(ttyUSBx);
	check_emu_tty(ttyUSBx);
	for (i = 0; i < MAX_BOARD; i++)
	{
		zprintf(INFO,"emu:%s, baud:%d", ttyUSBx->grp[i].ttyUSB_Emu.dev, ttyUSBx->grp[i].ttyUSB_Emu.baud);
		for (j = 0; j < SLOT_NBR; j++)
		{
			zprintf(INFO,"\t gsm[%d]chn[%d]:%s", j, ttyUSBx->grp[i].ttyUSB_Gsm[j].chn, ttyUSBx->grp[i].ttyUSB_Gsm[j].dev);
		}
	}
	return 0;
}

void sighdl(int signo)
{
	if (signo == SIGINT || signo == SIGTERM || signo == SIGABRT || signo == SIGKILL || signo == SIGSTOP)
	{
		gNetHdlParams.stop = 1;
		zprintf(INFO,"sighdl: signo(%d) receive, so stop NetHdlTask", signo);
	}
	//return NULL;
}

/**************************************************************************** 
* 函数名称 : main
* 功能描述 : 主函数
* 参    数 : void
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int main(int argc, char **argv)
{
	if(checkProcessExist((char*)LOCK_FILE)){
		return -1;
	}

	if(0 != openlogfile((char*)"/tmp/log/SimEmuSvr/SimEmuSvr.log")){
		return -1;
	}
	

	zprintf(INFO,"[INFO]============================== SimEmuSvr %X #%s %s ==============================",VERSION,__DATE__,__TIME__);
	
	signal(SIGINT, sighdl);
	signal(SIGTERM, sighdl);
	signal(SIGABRT, sighdl);
	signal(SIGKILL, sighdl);
	signal(SIGSTOP, sighdl);

	
	//
	memset(&gMini52Version, 0, sizeof(mini52_version_t)*SLOT_NBR);
	//
	memset((char *)&gSimEmuSvrParam, 0, sizeof(gSimEmuSvrParam));

	// init ttyUSBx
	init_ttyUSBx(&g_ttyUSBx);
	
	init_redis_SMS();
	readConfigValue();
	readLogConfValue();
	
	// 获取本地/远程模式。如果本地模式，则退出。
	// 暂时只支持网关整体统一本地模式或远程模式
	if (strstr(gSimEmuSvrParam.simemusvr_switch, "no") != NULL)
	{
		Sleep(5000);
		goto SIMEMUSVR_END;
	}
	
	// init
	initRegisterStat(&gRegisterStat);
	

	// get board serial file
	start_master_mode(MAX_BOARD);
	
SIMEMUSVR_END:
	zprintf(INFO,"[INFO]============================== SimEmuSvr End ==============================");
	
	return 0;
}
