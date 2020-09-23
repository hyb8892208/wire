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

#include "SimEmuSvr.h"

#include "config.h"

#include "serial.h"

//#include "debug.h"

#include "zprint.h"

#include "EmuVcard_g2.h"
#include "hiredis.h"




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


// astman
struct message astmsg;
//struct mansession *session = NULL;

#define AST_USER	"admin"
#define AST_PASSWD	"admin"
#define AST_HOST	"127.0.0.1"
#define AST_PORT	5038

#define REDIS_HOST	"127.0.0.1"
#define REDIS_PORT	6379

int sock_hdl_stop = 0;

net_handle_t gNetHandles[] =
{
	{UNREGISTER_RSP,			UnRegisterRspHdl},
	{HEARTBEAT_RSP,				HeartbeatRspHdl},
	{LINKINFO_REPORT_RSP,		SimLinkReportRspHdl},
	{SIMLINK_CREATE_REQ,		SimLinkCreateReqHdl},
	{SIMLINK_RELEASE_REQ,		SimLinkReleaseReqHdl},
	{SIMDATA_RSP,				SimDataRspHdl},
	{SIM_PULLPLUG_NOTICE_REQ,	SimPullPlugNoticeHdl}
};

ast_handle_t gAstHandles[] =
{
	{CALL_MORING_NOTICE,	CallMoringHdl},
	{CALL_BEGIN_NOTICE,		CallBeginHdl},
	{CALL_END_NOTICE,		CallEndHdl}
};


simemusvrparam_t gSimEmuSvrParam;
CSocketEx gSockReg;
unsigned short g_link_nbr = 0;
char g_mode[32] = {0}; // master; stand_alone



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
		zprintf("[ERRO]UnRegisterRspHdl: heartbeat rsp'result is fail");
	}
	else
	{
		//zprintf("[INFO]UnRegisterRspHdl: heartbeat rsp success");
		zprintf("[INFO]UnRegisterRspHdl: heartbeat rsp success");
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
		zprintf("[ERRO]HearbeatRspHdl: heartbeat rsp'result is fail");
	}
	else
	{
		zprintf("[INFO]HearbeatRspHdl: heartbeat rsp success");
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
	link_info.sb_bank_nbr = ntohs(link_info.sb_bank_nbr);
	link_info.sim_nbr = ntohs(link_info.sim_nbr);
	link_info.atr_len = ntohs(link_info.atr_len);
	memcpy(seri_emu, link_info.gw_seri, SERIALNO_LEN);
	link_info.gw_port = ntohs(link_info.gw_port);
	link_info.gw_bank_nbr = ntohs(link_info.gw_bank_nbr);
	link_info.slot_nbr = ntohs(link_info.slot_nbr);
	link_info.vgsm_len = ntohs(link_info.vgsm_len);
	
	zprintf("[INFO]SimLinkCreateReqHdl[00-%02d-%02d]: Emu[%s-00-%02d-%02d-%s:%d] == Rdr[%s-%02d-%02d-%s:%d]", \
		link_info.gw_bank_nbr, link_info.slot_nbr, \
		seri_emu, link_info.gw_bank_nbr, link_info.slot_nbr, ip_num_to_char(ntohl(link_info.gw_ip), gwaddr), link_info.gw_port, \
		seri_bank, link_info.sb_bank_nbr, link_info.sim_nbr, ip_num_to_char(ntohl(link_info.sb_ip), sbaddr), link_info.sb_port);

	// send link info to SlotHdlTask
	memset(&msg, 0, sizeof(msg));
	msg.header.cmd = SIMLINK_CREATE_REQ;
	msg.header.data_len = len; //sizeof(link_info);
	msg.header.slot = link_info.slot_nbr;
	memcpy(msg.data, (char *)&link_info, len); //sizeof(link_info));
	ret = csock->WriteData(csock->getSocket(), (char *)&msg, sizeof(msg.header)+msg.header.data_len, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].local_ip, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].port_nettask);
	if ((unsigned short)ret != (sizeof(msg.header)+msg.header.data_len))
	{
		zprintf("[ERRO]SimLinkCreateReqHdl[00-%02d-%02d]: write link info to SlotHdlTask[%s:%d] error(%d:%s)\n", \
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
	
	zprintf("[INFO]SimLinkReleaseReqHdl[00-%02d-%02d]: Emu[%s-00-%02d-%02d-%s:%d] == Rdr[%s-%02d-%02d-%s:%d]", \
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
		zprintf("[ERRO]SimLinkReleaseReqHdl[00-%02d-%02d]: write link info to SlotHdlTask[%s:%d] error(%d:%s)\n", \
			link_info.gw_bank_nbr, link_info.slot_nbr, \
			gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].local_ip, gSlotHdlTaskParam[link_info.gw_bank_nbr][msg.header.slot].port_nettask, errno, strerror(errno));
	}
	//
	return 0;
}

int SimLinkReportRspHdl(char *buff_recv, char *buff_send, CSocketEx *csock)
{
	int ret = 0;
	link_info_t link_info;
	//char seri_emu[12]  = {0};
	//char seri_bank[12] = {0};
	msg_t msg;
	unsigned short len = 0;
	//char gwaddr[16] = {0};
	//char sbaddr[16] = {0};
	report_rsp_info_t *report_rsp;
	unsigned short bank_nbr = 0;
	unsigned short slot_nbr = 0;

	report_rsp = (report_rsp_info_t *)(buff_recv+PACKAGE_HEADER_LEN);
	bank_nbr = ntohs(report_rsp->bank_nbr);
	slot_nbr = ntohs(report_rsp->slot_nbr);
	
	
	//zprintf("[INFO]SimLinkReportRspHdl[00-%02d-%02d]: Link Report Rsp Succ", report_rsp->bank_nbr, report_rsp->slot_nbr);
	
	// send link info to SlotHdlTask
	memset(&msg, 0, sizeof(msg));
	msg.header.cmd = SIMLINK_RELEASE_REQ;
	msg.header.data_len = len; //sizeof(link_info);
	msg.header.slot = link_info.slot_nbr;
	memcpy(msg.data, (char *)&link_info, len); //sizeof(link_info));
	ret = csock->WriteData(csock->m_sockfd, buff_recv, PACKAGE_HEADER_LEN+sizeof(report_rsp_info_t), gSlotHdlTaskParam[bank_nbr][slot_nbr].local_ip, gSlotHdlTaskParam[bank_nbr][slot_nbr].port_nettask);
	if ((unsigned short)ret != (PACKAGE_HEADER_LEN+sizeof(report_rsp_info_t)))
	{
		zprintf("[ERRO]SimLinkReportRspHdl[00-%02d-%02d]: Write Link Report To SlotHdlTask[%s:%d] Error(%d:%s)\n", \
			bank_nbr, slot_nbr, \
			gSlotHdlTaskParam[bank_nbr][slot_nbr].local_ip, gSlotHdlTaskParam[bank_nbr][slot_nbr].port_nettask, errno, strerror(errno));
	}
	//
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
	plink_relation->sb_sim_nbr = ntohs(plink_relation->sb_sim_nbr);
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
		zprintf("[ERRO]SimDataRspHdl[00-%02d-%02d]: Write SimDataRsp To SlotHdlTask[%s:%d] Error(%d:%s)\n", \
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
	//msg_t msg;
	//msg_header_t *phdr = NULL;
	unsigned short len = 0;
	//char gwaddr[16] = {0};
	//char sbaddr[16] = {0};
	//report_rsp_info_t *report_rsp;
	//unsigned short bank_nbr = 0;
	//unsigned short slot_nbr = 0;
	msg_t msg;

	
	len = getLen(buff_recv);
	psppi = (sim_pullplug_info_t *)(buff_recv+PACKAGE_HEADER_LEN);
	memcpy(sb_seri, psppi->sb_seri, SERIALNO_LEN);
	psppi->sb_usb_nbr = ntohs(psppi->sb_usb_nbr);
	psppi->sb_bank_nbr = ntohs(psppi->sb_bank_nbr);
	psppi->sb_slot_nbr = ntohs(psppi->sb_slot_nbr);
	memcpy(gw_seri, psppi->gw_seri, SERIALNO_LEN);
	psppi->gw_bank_nbr = ntohs(psppi->gw_bank_nbr);
	psppi->gw_slot_nbr = ntohs(psppi->gw_slot_nbr);
	
	memset((char *)&msg, 0, sizeof(msg));
	msg.header.cmd = SIM_PULLPLUG_NOTICE_REQ;
	msg.header.data_len = sizeof(sim_pullplug_info_t);
	memcpy(msg.data, (char *)psppi, msg.header.data_len);
	
	
	
	zprintf("[INFO]SimDataRspHdl[00-%02d-%02d]: Receive SimPullPlugNotice[sb:%s--%02d-%02d-%02d] Succ", \
		psppi->gw_bank_nbr, psppi->gw_slot_nbr, sb_seri, psppi->sb_usb_nbr, psppi->sb_bank_nbr, psppi->sb_slot_nbr);
	
	// send link info to SlotHdlTask
	ret = csock->WriteData(csock->m_sockfd, (char *)&msg, sizeof(msg_header_t)+msg.header.data_len, gSlotHdlTaskParam[psppi->gw_bank_nbr][psppi->gw_slot_nbr].local_ip, gSlotHdlTaskParam[psppi->gw_bank_nbr][psppi->gw_slot_nbr].port_nettask);
	if ((unsigned short)ret != sizeof(msg_header_t)+msg.header.data_len)
	{
		zprintf("[ERRO]SimDataRspHdl[00-%02d-%02d]: Write Link Report To SlotHdlTask[%s:%d] Error(%d:%s)\n", \
			psppi->gw_bank_nbr, psppi->gw_slot_nbr, \
			gSlotHdlTaskParam[psppi->gw_bank_nbr][psppi->gw_slot_nbr].local_ip, gSlotHdlTaskParam[psppi->gw_bank_nbr][psppi->gw_slot_nbr].port_nettask, errno, strerror(errno));
	}
	//
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
	board_nbr = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[0]) - 1;
	slot_nbr  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[1]) - 1;
	memcpy(calldst, buff_recv+PACKAGE_HEADER_LEN+4, sizeof(calldst));
	zprintf("[INFO]CallMoringHdl[00-%02d-%02d]:Call Moring[calldst:%s]...", board_nbr, slot_nbr, calldst);

	// make package that send to proxysvr
	call_moring_info.type = htons(SimEmuSvr);
	memcpy(call_moring_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	call_moring_info.bank_nbr = htons(board_nbr);
	call_moring_info.slot_nbr = htons(slot_nbr);
	memcpy(call_moring_info.calldst, calldst, sizeof(calldst));
	len = makeReqPackage(CALL_MORING_NOTICE, (char *)&call_moring_info, sizeof(call_moring_info), buff_send);
	
	ret = csock->WriteData(csock->getSocket(), buff_send, len);
	if (ret != len)
	{
		zprintf("[ERRO]CallMoringHdl[%02d-%02d-%02d]: send call moring[calldst:%s] info to ProxySvr error(%d:%s)", board_nbr, slot_nbr, calldst, errno, strerror(errno));
	}
	
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
	board_nbr = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[0]) - 1;
	slot_nbr  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[1]) - 1;
	memcpy(calldst, buff_recv+PACKAGE_HEADER_LEN+4, sizeof(calldst));
	zprintf("[INFO]CallBeginHdl[00-%02d-%02d]: Call Begin[calldst:%s]...", board_nbr, slot_nbr, calldst);

	// make package that send to proxysvr
	call_begin_info.type = htons(SimEmuSvr);
	memcpy(call_begin_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	call_begin_info.bank_nbr = htons(board_nbr);
	call_begin_info.slot_nbr = htons(slot_nbr);
	memcpy(call_begin_info.calldst, calldst, sizeof(calldst));
	len = makeReqPackage(CALL_BEGIN_NOTICE, (char *)&call_begin_info, sizeof(call_begin_info), buff_send);
	
	ret = csock->WriteData(csock->getSocket(), buff_send, len);
	if (ret != len)
	{
		zprintf("[ERRO]CallBeginHdl[00-%02d-%02d]: send call begin[calldst:%s] info to ProxySvr error(%d:%s)", board_nbr, slot_nbr, calldst, errno, strerror(errno));
	}
	
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
	board_nbr = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[0]) - 1;
	slot_nbr  = ntohs(((unsigned short *)(buff_recv+PACKAGE_HEADER_LEN))[1]) - 1;
	zprintf("[INFO]CallEndHdl[00-%02d-%02d]: Call End...", board_nbr, slot_nbr);

	// make package that send to proxysvr
	call_end_info.type = htons(SimEmuSvr);
	memcpy(call_end_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	call_end_info.bank_nbr = htons(board_nbr);
	call_end_info.slot_nbr = htons(slot_nbr);
	len = makeReqPackage(CALL_END_NOTICE, (char *)&call_end_info, sizeof(call_end_info), buff_send);
	
	ret = csock->WriteData(csock->getSocket(), buff_send, len);
	if (ret != len)
	{
		zprintf("[ERRO]CallEndHdl[00-%02d-%02d]: send call end info to ProxySvr error(%d:%s)", board_nbr, slot_nbr, errno, strerror(errno));
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
	char content[128];
	void *handle = NULL;
	
	ret = conf_init(SIMEMUSVR_CONFIG, &handle);
	if (ret != 0)
	{
		return -1;
	}
	
	// seri
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SERI, content);
	if (ret == 0)
	{
		a_trim(gSimEmuSvrParam.seri, content);
	}
	// passwd
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_PASSWD, content);
	if (ret == 0)
	{
		a_trim(gSimEmuSvrParam.passwd, content);
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
	// server_port
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SERVER_PORT, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.server_port = atoi(content);
	}
	// heartbeat_interval
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_HB_INTERVAL, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.hb_interval = atoi(content);
	}
	// comm
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_COMM, content);
	if (ret == 0)
	{
		a_trim(gSimEmuSvrParam.comm, content);
	}
	// slot_port_rdr
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SLOT_PORT_RDR, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.slot_port_rdr = atoi(content);
	}
	// slot_port_comm
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SLOT_PORT_COMM, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.slot_port_comm = atoi(content);
	}
	// slot_port_net
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SLOT_PORT_NET, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.slot_port_net = atoi(content);
	}
	// comm_hdl_port
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_COMM_HDL_PORT, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.comm_hdl_port = atoi(content);
	}
	// net_hdl_port
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_NET_HDL_PORT, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.net_hdl_port = atoi(content);
	}
	// sim_data_out_port
	/*memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SIM_DATA_OUT_PORT, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.sim_data_out_port = atoi(content);
	}
	// sim_data_in_port
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_SIM_DATA_IN_PORT, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.sim_data_in_port = atoi(content);
	}*/
	// ast_hdl_port
	memset(content, 0, sizeof(content));
	ret = conf_getValue(handle, KEY_AST_HDL_PORT, content);
	if (ret == 0)
	{
		gSimEmuSvrParam.ast_hdl_port = atoi(content);
	}
	
	zprintf("[INFO]SimEmuSvr param: seri:%s, local_ip:%s, server_ip:%s, server_port:%d, heartbeat_interval:%d, comm:%s, slot_port_comm:%d, slot_port_net:%d, comm_hdl_port:%d, net_hdl_port:%d, sim_data_out_port:%d, sim_data_in_port:%d, ast_hdl_port:%d", \
		gSimEmuSvrParam.seri, gSimEmuSvrParam.local_ip, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port, gSimEmuSvrParam.hb_interval, gSimEmuSvrParam.comm, \
		gSimEmuSvrParam.slot_port_comm, gSimEmuSvrParam.slot_port_net, gSimEmuSvrParam.comm_hdl_port, gSimEmuSvrParam.net_hdl_port, gSimEmuSvrParam.sim_data_out_port, gSimEmuSvrParam.sim_data_in_port, gSimEmuSvrParam.ast_hdl_port);
	
	conf_release(&handle);
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
	char buff[BUFF_SIZE];
	int len = 0;
	int ret = 0;
	register_req_info_t register_req_info;

	memset(buff, 0, sizeof(buff));
	memset((char *)&register_req_info, 0, sizeof(register_req_info));

	// connect to server
	csock->Init(PROTO_TCP, gSimEmuSvrParam.local_ip, gSimEmuSvrParam.server_port+1, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port, 1);
	csock->CreateSocket();
	if (csock->Bind() == false)
	{
		return Error;
	}
	ret = csock->ConnectNonB(csock->getSocket(), 10);
	if (!ret)
	{
		csock->CloseSocket();
		zprintf("[ERRO]emuRegister: connect[%s:%d] to Server[%s:%d] error(%d:%s)", gSimEmuSvrParam.local_ip, gSimEmuSvrParam.server_port, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port, errno, strerror(errno));
		return Error;
	}
	zprintf("[INFO]emuRegister: connect[%s:%d] to Server[%s:%d] succ", gSimEmuSvrParam.local_ip, gSimEmuSvrParam.server_port, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port);

	register_req_info.type = htons(SimEmuSvr);
	memcpy(register_req_info.seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
	register_req_info.bank_nbr = htons(EMU_BANK_NBR);
	register_req_info.slot_nbr = htons(EMU_BANK_NBR * SLOT_NBR);
	memcpy(register_req_info.passwd, gSimEmuSvrParam.passwd, strlen(gSimEmuSvrParam.passwd));
	makeRegisterPack(buff, (char *)&register_req_info, sizeof(register_req_info), &len);

	// send req pack
	ret = csock->WriteData(csock->getSocket(), buff, len);
	if (ret != len)
	{
		csock->CloseSocket();
		zprintf("[ERRO]emuRegister: send register request error(%d:%s)", errno, strerror(errno));
		return Error;
	}
	// recv rsp pack headr
	memset(buff, 0, sizeof(buff));
	if (!csock->checkSocketReady(csock->getSocket(), READ_WAIT, 10))
	{
		csock->CloseSocket();
		zprintf("[ERRO]emuRegister: register no rsp yet");
		return Error;
	}
	ret = csock->TCPReadData(csock->getSocket(), buff, PACKAGE_HEADER_LEN);
	if (ret != PACKAGE_HEADER_LEN)
	{
		csock->CloseSocket();
		zprintf("[ERRO]emuRegister: recv register response header error(%d:%s)", errno, strerror(errno));
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
		zprintf("[ERRO]emuRegister: recv register response body error(%d:%s)", errno, strerror(errno));
		return Error;
	}

	// check rsp pack
	ret = checksum((unsigned short *)buff, PACKAGE_HEADER_LEN+len+2);
	if (!ret)
	{
		csock->CloseSocket();
		zprintf("[ERRO]emuRegister: register response checksum fail");
		return Error;
	}
	unsigned short result = getResult(buff);
	if (result != 0x0000)
	{
		csock->CloseSocket();
		zprintf("[ERRO]emuRegister: the register result is fail");
		return Error;
	}
	zprintf("[INFO]emuRegister: SimEmuSvr[%s] register to %s:%d success", gSimEmuSvrParam.seri, gSimEmuSvrParam.server_ip, gSimEmuSvrParam.server_port);

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
		zprintf("[ERRO]send unregister request error(%d:%s)", errno, strerror(errno));
		return Error;
	}
	// recv rsp pack headr
	memset(buff, 0, sizeof(buff));
	if (!sock->checkSocketReady(sock->getSocket(), READ_WAIT, 10))
	{
		zprintf("[ERRO]unregister no rsp yet");
		return Error;
	}
	ret = sock->TCPReadData(sock->getSocket(), buff, PACKAGE_HEADER_LEN);
	if (ret != PACKAGE_HEADER_LEN)
	{
		zprintf("[ERRO]recv unregister response header error(%d:%s)", errno, strerror(errno));
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
		zprintf("[ERRO]emuUnRegister: recv unregister response body error(%d:%s)", errno, strerror(errno));
		return Error;
	}
	
	// check rsp pack
	ret = checksum((unsigned short *)buff, PACKAGE_HEADER_LEN+len+2);
	if (!ret)
	{
		zprintf("[ERRO]emuUnRegister: unregister response checksum fail");
		return Error;
	}
	unsigned short result = getResult(buff);
	if (result != 0x0000)
	{
		zprintf("[ERRO]emuUnRegister: the unregister result is fail");
		return Error;
	}
	zprintf("[INFO]emuUnRegister: unregister success");
	
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
		zprintf("[ERRO]send heartbeat request error.(%d:%s)", errno, strerror(errno));
		return Error;
	}
	// return
	return NoErr;
}

int getChannelStatusFromRedis(unsigned short board_nbr, unsigned short slot_nbr)
{
	int result = 0;
	int chan_no = board_nbr * SLOT_NBR + slot_nbr + 1;
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err)
	{
		zprintf("[ERRO][00-%02d-%02d]: redisConnect error(%s)", board_nbr, slot_nbr, context->errstr);
	}
	else
	{
		redisReply *reply = (redisReply *)redisCommand(context, "GET app.asterisk.gsmstatus.channel%d", chan_no);
		if (reply->type == REDIS_REPLY_ERROR)
		{
				zprintf("[ERRO][00-%02d-%02d]: GET app.asterisk.gsmstatus.channel%d error(%s)", board_nbr, slot_nbr, chan_no, reply->str);
		}
		else
		{
			if (reply->str != NULL)
			{
				result = atoi(reply->str);
			}
			else
			{
				result = 0;
				zprintf("[INFO][00-%02d-%02d]: redisCommand(GET app.asterisk.gsmstatus.channel%d) return NULL", board_nbr, slot_nbr, chan_no);
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
			zprintf("[ERRO]resetMini52[%02d-%02d-%02d]: reset Mini52 succ", 0, board, slot);
			ret = 0;
			Sleep(1000);
			break;
		}
		else
		{
			logDataToHex(buff_req, 4, 0, board, slot, DIRE_COMMHDLTASK_TO_COMM);
			logDataToHex(buff_rsp, len, 0, board, slot, DIRE_COMM_TO_COMMHDLTASK);
			zprintf("[ERRO]resetMini52[%02d-%02d-%02d]: reset Mini52 error(%d:%s)", 0, board, slot, errno, strerror(errno));
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
			zprintf("[INFO]resetMini52[%02d-%02d-%02d]: reset Mini52 succ", 0, board, slot);
			ret = 0;
			break;
		}
		else
		{
			logDataToHex(buff_req, 4, 0, board, slot, DIRE_COMMHDLTASK_TO_COMM);
			logDataToHex(buff_rsp, len, 0, board, slot, DIRE_COMM_TO_COMMHDLTASK);
			zprintf("[ERRO]resetMini52[%02d-%02d-%02d]: reset Mini52 error(%d:%s)", 0, board, slot, errno, strerror(errno));
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
	int len = 0;

	buff_req[0] = 0x80;
	buff_req[1] = CMD_IS_REQRST;
	buff_req[2] = 0;
	buff_req[3] = 0;
	for (int i = 0; i < trys; i++)
	{
		ret = serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == 0x80 && buff_rsp[1] == IS_RESP_RST && buff_rsp[2] == 0 && buff_rsp[3] == 0)
		{
			zprintf("[ERRO]resetSTM32[00-%02d]: reset STM32 succ", board);
			ret = 0;
			Sleep(1000);
			break;
		}
		else
		{
			logDataToHex(buff_req, 4, 0, board, 80, DIRE_COMMHDLTASK_TO_COMM);
			logDataToHex(buff_rsp, len, 0, board, 80, DIRE_COMM_TO_COMMHDLTASK);
			zprintf("[ERRO]resetSTM32[00-%02d]: reset STM32 error(%d:%s)", board, errno, strerror(errno));
			Sleep(1000);
		}
	}

	return ret;
}

 int getSTM32Version(int handle, unsigned short board)
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
			zprintf("[INFO]getSTM32Version: STM32[%02d] version[%s]", board, buff_rsp+4);
			ret = 0;
			//Sleep(1000);
			break;
		}
		logDataToHex(buff_req, 4, 0, board, 80, DIRE_COMMHDLTASK_TO_COMM);
		logDataToHex(buff_rsp, len, 0, board, 80, DIRE_COMM_TO_COMMHDLTASK);
		zprintf("[ERRO]getSTM32Version: get STM32[%02d] version error(%d:%s)", board, errno, strerror(errno));
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
		ret = resetMini52(handle, board, i);
		if (ret < 0)
		{
			res = -1;
		}
	}
	return res;
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
int getMini52Version(int handle, unsigned short usb_nbr, unsigned short slot)
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
			zprintf("[INFO]getMini52Version: Mini52[usb:%d, slot:%d] version[%s]", usb_nbr, slot, buff_rsp+4);
			ret = 0;
			//Sleep(1000);
			break;
		}
		logDataToHex(buff_req, 4, usb_nbr, 80, slot, DIRE_COMMHDLTASK_TO_COMM);
		logDataToHex(buff_rsp, len, usb_nbr, 80, slot, DIRE_COMM_TO_COMMHDLTASK);
		zprintf("[ERRO]getMini52Version: get Mini52[usb:%d, slot:%d] version error(%d:%s)", usb_nbr, slot, errno, strerror(errno));
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
int getAllMini52Version(int handle, unsigned short usb_nbr)
{
	unsigned short i = 0;
	int ret = 0;
	int res = 0;
	for (i = 0; i < SLOT_NBR; i++)
	{
		ret = getMini52Version(handle, usb_nbr, i);
		if (ret < 0)
		{
			res = -1;
		}
	}
	return res;
}


int emuSlotReport(CSocketEx *csock, char *local_ip, unsigned short local_port, unsigned short bank_nbr, unsigned short slot_nbr)
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
	report_info.port = htons(local_port);
	report_rsp_info_t *report_rsp = NULL;
	len = makeReportPackage(buff, (char *)&report_info, sizeof(report_info)-sizeof(report_info.vgsm_len)-sizeof(report_info.vgsm), &len);

	// 线路上报

	ret = csock->WriteData(csock->m_sockfd, buff, len);
	if (ret != len)
	{
		zprintf("[ERRO]emuReport[00-%02d-%02d]: send link report package to NetHdlTask error(%d:%s).", bank_nbr, slot_nbr, errno, strerror(errno));
		return -1;
	}
	if (!csock->checkSocketReady(csock->m_sockfd, READ_WAIT, 30))
	{
		zprintf("[ERRO]emuReport[00-%02d-%02d]: wait for link report rsp error.", bank_nbr, slot_nbr);
		return -1;
	}
	memset(buff, 0, sizeof(buff));
	len = csock->ReadData(csock->m_sockfd, buff, sizeof(buff));
	if (len <= 0)
	{
		zprintf("[ERRO]emuReport[00-%02d-%02d]: read from NetHdlTask error(%d:%s).", bank_nbr, slot_nbr, errno, strerror(errno));
		return -1;
	}
	report_rsp = (report_rsp_info_t *)(buff+PACKAGE_HEADER_LEN);
	if (getCmd(buff) != LINKINFO_REPORT_RSP || getResult(buff) != 0x00 || ntohs(report_rsp->bank_nbr) != bank_nbr || ntohs(report_rsp->slot_nbr) != slot_nbr)
	{
		zprintf("[ERRO]emuReport[00-%02d-%02d]: not report rsp package or result is fail.", bank_nbr, slot_nbr);
		return -1;
	}
	zprintf("[INFO]emuReport[00-%02d-%02d]: link report succ[ip:%s, port:%d, bank_nbr:%d, slot_nbr:%d]", \
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
	fd_set rset;
	timeval tv;
	char buff_recv[0xffff] = {0};
	char buff_send[0xffff] = {0};
	int len_recv = 0;
	int len_send = 0;
	int max = 0;
	msg_header_t *pmsgh = NULL;
	msg_t msg;
	//int reported = 0;
	link_info_t link_info;
	char seri_emu[12] = {0};
	char seri_bank[12] = {0};
	//
	Emu_Engine emu_eng;
	memset(&emu_eng, 0, sizeof(emu_eng));
	//
	char sim_atr[320] = {0};
	unsigned short len_atr = 0;
	//char gwaddr[16] = {0};
	//char sbaddr[16] = {0};
	time_t link_create_last = 0;
	time_t link_create_curr = 0;
	int time_dur = 0;
	unsigned int link_create_cnt = 0;
	unsigned int link_release_cnt = 0;
	time_t last = 0;
	time_t curr = 0;
	time(&last);
	link_relation_t link_relation;
	//unsigned char print_cnt = 0;

	zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: Start[port_comm:%d, port_nettask:%d]......", \
		0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->port_comm, param->port_nettask);

	// create socket
	if (sock_comm.m_sockfd < 0)
	{
		sock_comm.Init(PROTO_UDP, param->local_ip, param->port_comm, param->local_ip, gCommHdlTaskParam[param->slotinfo.gw_bank_nbr].port, 10);
		sock_comm.CreateSocket();
		sock_comm.Bind();
		param->sockfd_comm = sock_comm.getSocket();
		zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: sock_comm    create succ(%s:%d<==>%s:%d)", 
			0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->local_ip, param->port_comm, param->local_ip, gCommHdlTaskParam[param->slotinfo.gw_bank_nbr].port);
	}
	if (sock_nettask.m_sockfd < 0)
	{
		sock_nettask.Init(PROTO_UDP, param->local_ip, param->port_nettask, param->local_ip, gSimEmuSvrParam.net_hdl_port, 10);
		sock_nettask.CreateSocket();
		sock_nettask.Bind();
		param->sockfd_nettask = sock_nettask.getSocket();
		zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: sock_nettask create succ(%s:%d<==>%s:%d)", 
			0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->local_ip, param->port_nettask, param->local_ip, gSimEmuSvrParam.net_hdl_port);
	}

WAIT_FOR_COMM_READY:
	if (gCommHdlTaskParam[param->slotinfo.gw_bank_nbr].initialed == 0)
	{
		Sleep(2000);
		goto WAIT_FOR_COMM_READY;
	}
REPORT_LINE:
	// 线路上报
	// 改线路端口为统一对外端口
	if (emuSlotReport(&sock_nettask, param->local_ip, gSimEmuSvrParam.sim_data_out_port, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr) != 0)
	{
		Sleep(3000);
		goto REPORT_LINE;
	}
	
	time_t slothdl_last = 0;
	time_t slothdl_curr = 0;
	//time(&slothdl_last);
	while (param->stop != 1)
	{
		
		time(&slothdl_curr);
		if ((slothdl_curr - slothdl_last) > 180)
		{
			time(&slothdl_last);
			zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: Running[port_comm:%d, port_nettask:%d]......", \
				0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->port_comm, param->port_nettask);
		}

		// 判断通道注册是否超时
			// 获取通道状态
			// 如果通道状态为offline，则获取当前时间，判断是否超时
			// 如果超时，则重启模块，更新通道统计的开始时间
		time(&curr);
		if ((curr - last) > 10) // 间隔10以上秒检查一次
		{
			time(&last);
			if (param->slotinfo.link_stat == 1)
			{
				int channel_status = getChannelStatusFromRedis(param->slotinfo.board_nbr, param->slotinfo.slot_nbr);
				if (channel_status == 1)
				{
					time(&link_create_curr);
					time_dur = link_create_curr - link_create_last;
					memset((char *)&msg, 0, sizeof(msg));
					msg.header.cmd = POWER_RESET_SPAN_REQ;
					msg.header.slot = param->slotinfo.slot_nbr;
					//msg.header.data_len = 0;
					if (param->len_atr > 0)
					{
						memcpy(msg.data, param->sim_atr, param->len_atr);
						msg.header.data_len = param->len_atr;
					}
					else
					{
						msg.header.data_len = 0;
					}
					if (time_dur > 180)
					{
						// power reset span
						ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, sizeof(msg_header_t)+msg.header.data_len);
						if ((unsigned short)ret != sizeof(msg_header_t)+msg.header.data_len)
						{
							zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: send power reset span command to CommHdlTask error(%d:%s)", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
						}
						else
						{
							time(&link_create_last);
							zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: send power reset span command to CommHdlTask ......", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
						}
					}
				}
				else
				{
					time(&link_create_last);
					//zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: channel online ......", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
				}
			}
			else
			{
				//zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: Link not been Create!!!", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
			}
		}
		


		max = 0;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		FD_ZERO(&rset);
		if (sock_comm.m_sockfd > 0)
		{
			FD_SET((unsigned)sock_comm.m_sockfd, &rset);
			max = MAX(max, sock_comm.m_sockfd);
		}
		if (sock_nettask.m_sockfd > 0)
		{
			FD_SET((unsigned)sock_nettask.m_sockfd, &rset);
			max = MAX(max, sock_nettask.m_sockfd);
		}
		ret = select(max+1, &rset, NULL, NULL, &tv);
		if (ret < 0)
		{
			zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: select error(%d:%s).", \
				0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
			continue;
		}
		else if (ret == 0)
		{
			//zprintf("[INFO]SlotHdlTask[%d-%d]: select timeout.", param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
			continue;
		}
		else
		{
			if (sock_nettask.getSocket() > 0)
			{	// from NetHdlTask, sim link create or release notice
				if (FD_ISSET(sock_nettask.m_sockfd, &rset))
				{
					memset(buff_recv, 0, sizeof(buff_recv));
					len_recv = sock_nettask.ReadData(sock_nettask.m_sockfd, buff_recv, sizeof(buff_recv));
					if (len_recv <= 0)
					{
						zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: read data from NetHdlTask error(%d:%s)", \
							0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
						//continue;
					}
					else
					{ // 线路建立，线路释放，Sim数据响应
						memset((char *)&msg, 0, sizeof(msg));
						memcpy((char *)&msg, buff_recv, sizeof(msg_header_t));
						if (msg.header.cmd == SIMLINK_CREATE_REQ)
						{
							memcpy((char *)&link_info, buff_recv+sizeof(msg_header_t), msg.header.data_len);
							//link_info = (link_info_t *)(buff_recv+sizeof(msg_header_t));
							memcpy(seri_emu, link_info.gw_seri, SERIALNO_LEN);
							memcpy(seri_bank, link_info.sb_seri, SERIALNO_LEN);
							//zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: recv SIMLINK_CREATE_REQ msg", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
							
							// set param
							memcpy(param->slotinfo.sb_seri, link_info.sb_seri, SERIALNO_LEN);
							param->slotinfo.sb_usb_nbr = link_info.sb_usb_nbr;
							param->slotinfo.sb_bank_nbr = link_info.sb_bank_nbr;
							param->slotinfo.sim_nbr = link_info.sim_nbr;
							param->slotinfo.svr_ip = link_info.sb_ip;
							param->slotinfo.svr_port = link_info.sb_port;
							//param->slotinfo.gw_bank_nbr = link_info.gw_bank_nbr;

							
							// emu engine
							emu_eng.EmuApduEngineInit();
							emu_eng.SetRemoteSimCardPlugStatus(REMOTE_SIMCARD_IS_PLUG_IN);
							emu_eng.SetEmuEngineStatus(EMU_ENGINE_WAITING_VCARD);
							emu_eng.SetEmuLocoalVCardReadyStatus(LOCOAL_VCARD_IS_READY);
							emu_eng.PreProcessEngine((unsigned char *)link_info.vgsm);
							if (link_info.sim_atr[link_info.atr_len-1] == TRANSMISSION_PROTOCOL_T1)
							{
								emu_eng.SetEmuTransmissionProtocol(TRANSMISSION_PROTOCOL_T1);
							}
							else
							{
								emu_eng.SetEmuTransmissionProtocol(TRANSMISSION_PROTOCOL_T0);
							}
							//emu_eng.SetDeliverAtrToEmuStatus(DELIVER_ATR_TO_EMU_IS_READY);
							//eae.ucEmuApduNetApiStatus= EMU_APDU_NETAPI_STATUS_IDLE; 
							emu_eng.SetEmuEngineStatus(EMU_ENGINE_WORKING);
							emu_eng.SetDefaultCommandIdentifier();
							// sim atr
							memcpy(sim_atr, link_info.sim_atr, link_info.atr_len);
							len_atr = link_info.atr_len;
							memcpy(param->sim_atr, link_info.sim_atr, link_info.atr_len);
							param->len_atr = link_info.atr_len;
							
							memcpy(msg.data, link_info.sim_atr, link_info.atr_len);
							msg.header.data_len = link_info.atr_len;
							ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, sizeof(msg_header_t) + msg.header.data_len);
							if ((unsigned int)ret != (sizeof(msg_header_t) + msg.header.data_len))
							{
								zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: send SIMLINK_CREATE_REQ message(ret:%d) to CommHdlTask[port:%d] error(%d:%s)", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, ret, sock_comm.m_peerPort, errno, strerror(errno));
							}
							else
							{
								/*zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: send SIMLINK_CREATE_REQ message(ret:%d) to CommHdlTask[port:%d] succ", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, ret, sock_comm.m_peerPort);*/
							}
								
							// 设置通道注册开始时间
							time(&link_create_last);
							link_create_cnt++;
							param->slotinfo.link_stat = 1;
							
							memset((char *)&link_relation, 0, sizeof(link_relation));
							memcpy(link_relation.sb_seri, param->slotinfo.sb_seri, SERIALNO_LEN);
							memcpy(link_relation.gw_seri, gSimEmuSvrParam.seri, SERIALNO_LEN);
							link_relation.sb_usb_nbr = htons(param->slotinfo.sb_usb_nbr);
							link_relation.sb_bank_nbr = htons(param->slotinfo.sb_bank_nbr);
							link_relation.sb_sim_nbr = htons(param->slotinfo.sim_nbr);
							link_relation.gw_bank_nbr = htons(param->slotinfo.gw_bank_nbr);
							link_relation.gw_slot_nbr = htons(param->slotinfo.slot_nbr);
							
							zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: relation[gw:%s-00-%02d-%02d -- sb:%s-%02d-%02d-%02d]", \
								0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, \
								seri_emu, link_info.gw_bank_nbr, link_info.slot_nbr, \
								seri_bank, link_info.sb_usb_nbr, link_info.sb_bank_nbr, link_info.sim_nbr);
						}
						else if (msg.header.cmd == SIMLINK_RELEASE_REQ)
						{
							//zprintf("SlotHdlTask[%d-%d]: recv SIMLINK_RELEASE_REQ msg", param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
							
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
							link_relation.sb_sim_nbr = 0;
						}
						else if (msg.header.cmd == SIM_PULLPLUG_NOTICE_REQ)
						{
							memset(link_relation.sb_seri, 0, sizeof(link_relation.sb_seri));
							link_relation.sb_usb_nbr = 0;
							link_relation.sb_bank_nbr = 0;
							link_relation.sb_sim_nbr = 0;
							// 设置EmuEngine工作状态为停止
							emu_eng.SetEmuEngineStatus(EMU_ENGINE_STOP);
							
							zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: Receive SimPullPlugNotice And Stop EmuEngine", \
								0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
							
							// notice CommHldTask
							msg.header.slot = param->slotinfo.slot_nbr;
							ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, sizeof(msg.header));
						}
						else// if (msg.header.cmd == SIMDATA_RSP)
						{
							//logDataToHex((unsigned char *)buff_recv+sizeof(msg_header_t)+sizeof(link_relation), pmsgh->data_len-sizeof(link_relation), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SIMHDLTASK_TO_SLOTHDLTASK);
						
							// 把网络远程发送回来的数据填进EmuEngine，执行EmuEngine，获取EmuEngine状态，取出数据发送到Emu小板
							
							pmsgh = (msg_header_t *)(buff_recv);
							len_send = pmsgh->data_len;
							//logDataToHex((unsigned char *)buff_recv+sizeof(msg_header_t)+sizeof(link_relation), len_send-sizeof(link_relation), 00, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SIMHDLTASK_TO_SLOTHDLTASK);
							//logDataToHex((unsigned char *)buff_recv+sizeof(msg_header_t)+sizeof(link_relation), len_send-sizeof(link_relation), 44, 44, 44, DIRE_UNKOWN);
							emu_eng.CopyNetDataToEaeBuf((unsigned char *)buff_recv+sizeof(msg_header_t)+sizeof(link_relation), len_send-sizeof(link_relation), pmsgh->cmd); //DATA_PROPERTY_IS_DATA);
							emu_eng.SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_DATA_FROM_NET_IS_READY);
							// 执行EmuEngine
							emu_eng.ProcessEmuDataCmd();
							// EmuEngine执行完成
							if (emu_eng.GetEmuEngineStatus() == EMU_ENGINE_WORKING)
							{ 			    	
								if (emu_eng.GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY)
								{
									// 组包，发送到网络远程
									unsigned char tmpDataProperty;
									memset(&msg, 0, sizeof(msg));
									emu_eng.CopyEaeDataToDeliverNetBuf((unsigned char *)msg.data+sizeof(link_relation_t), (unsigned short *)&msg.header.data_len, &tmpDataProperty);
									//logDataToHex((unsigned char *)msg.data+sizeof(link_relation_t), msg.header.data_len, 55, 55, 55, DIRE_UNKOWN);
									// add for link relation data
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
										zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sim data req package error(%d:%s)", \
											0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
										param->slotinfo.stat = SlotInvalid;
										//param->sockfd_netrdr = -1;
									}
										
									//TransferToRemote_nts(&ActEae,&Actn2st);
									emu_eng.SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_WAITING_FROM_NET_READY);
									//logDataToHex((unsigned char *)&msg.data, ntohs(msg.header.data_len), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
								}	
								else if (emu_eng.GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_EMU_IS_READY)
								{
									memset((char *)&msg, 0, sizeof(msg));
									emu_eng.CopyEaeDataToEmuBuf((unsigned char *)msg.data+4, (unsigned short *)&len_send);
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
										zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: send data rsp package to CommHdlTask error(%d:%s)", \
											0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
									}
									else
									{
										//logDataToHex((unsigned char *)&msg, msg.header.data_len+sizeof(msg_header_t), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_COMMHDLTASK);
									}
									//emu_eng.SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_IDLE);
								}
								
								else
								{
									zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: EmuEngine's ApduNetApi Status[%d] is not expected", \
										0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, emu_eng.GetEmuApduNetApiStatus());
									//sock_trans.CloseSocket();
									//param->slotinfo.stat = SlotInvalid;
								}
							}
							else
							{
								zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: EmuEngine's status[%d] is not EMU_ENGINE_WORKING", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, emu_eng.GetEmuEngineStatus());
								param->slotinfo.stat = SlotInvalid;
							}
						}
					}
				}
			}

			if (sock_comm.getSocket() > 0)
			{
				if (FD_ISSET(sock_comm.m_sockfd, &rset))
				{ // from CommHdlTask
					// read
					memset(buff_recv, 0, sizeof(buff_recv));
					len_recv = sock_comm.UDPReadData(sock_comm.m_sockfd, buff_recv, sizeof(buff_recv));
					if (len_recv < 0)
					{
						zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: read data from CommHdlTask error(%d:%s)", \
							0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, errno, strerror(errno));
						continue;
					}
					/*else
					{
						logDataToHex((unsigned char *)buff_recv, len_recv, 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_COMMHDLTASK_TO_SLOTHDLTASK);
					}*/
					pmsgh = (msg_header_t *)buff_recv;

					// 此处解析数据
					// slot 10 00 00
					if(((unsigned char *)(buff_recv+sizeof(msg_header_t)))[1] == IS_REQ_RST_ICC)
					{ // 回写sim atr
						/*memset(&msg, 0, sizeof(msg));
						msg.header.cmd = SIMDATA_RSP; //CMD_IS_SETATR;
						msg.header.slot = param->slotinfo.slot_nbr;
						msg.header.data_len = link_info.atr_len+4;
						msg.data[0] = param->slotinfo.slot_nbr;
						msg.data[1] = CMD_IS_SETATR;
						msg.data[2] = 0;
						msg.data[3] = link_info.atr_len;
						
						memcpy(msg.data+4, link_info.sim_atr, link_info.atr_len);
						ret = sock_comm.WriteData(sock_comm.getSocket(), (char *)&msg, msg.header.data_len+sizeof(msg_header_t));
						if (ret != msg.header.data_len+sizeof(msg_header_t))
						{
							zprintf("[ERRO]SlotHdlTask[%d-%d]: send data to commhdltask(%s:%d) error(%d:%s)", \
								param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, param->local_ip, gCommHdlTaskParam[param->slotinfo.gw_bank_nbr].port, errno, strerror(errno));
						}
						else
						{
							//logDataToHex((unsigned char *)msg.data, msg.header.data_len, 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_COMMHDLTASK);
						}*/
						//emu_eng.SetEmuEngineStatus(EMU_ENGINE_WORKING);

						
						emu_eng.SetFromEmuDataProperty(DATA_PROPERTY_IS_REQ_RST_ICC);
						//emu_eng.CopyFromEmuDataToEaeBuf((unsigned char *)buff_recv+sizeof(msg_header_t)+4, len_recv-sizeof(msg_header_t)-4, emu_eng.GetFromEmuDataProperty());
						emu_eng.SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_DATA_FROM_EMU_IS_READY);
						emu_eng.ProcessEmuDataCmd();
					}
					else if (((unsigned char *)(buff_recv+sizeof(msg_header_t)))[1] == CMD_IS_DATA)
					{ // 操作EmuEngine解析数据
						//emu_eng.SetEmuEngineStatus(EMU_ENGINE_WORKING);
						emu_eng.SetFromEmuDataProperty(DATA_PROPERTY_IS_DATA);
						//logDataToHex((unsigned char *)buff_recv+sizeof(msg_header_t)+4, len_recv-sizeof(msg_header_t)-4, 11, 11, 11, DIRE_UNKOWN);
						emu_eng.CopyFromEmuDataToEaeBuf((unsigned char *)buff_recv+sizeof(msg_header_t)+4, len_recv-sizeof(msg_header_t)-4, emu_eng.GetFromEmuDataProperty());
						emu_eng.SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_DATA_FROM_EMU_IS_READY);
						emu_eng.ProcessEmuDataCmd();
						if (emu_eng.GetEmuEngineStatus() == EMU_ENGINE_WORKING)
						{
							if (emu_eng.GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_NET_IS_READY)
							{
								// 组包，发送到网络远程
								unsigned char tmpDataProperty;
								memset(&msg, 0, sizeof(msg));
								emu_eng.CopyEaeDataToDeliverNetBuf((unsigned char *)msg.data+sizeof(link_relation), (unsigned short*)&msg.header.data_len, &tmpDataProperty);
								//logDataToHex((unsigned char *)msg.data+sizeof(link_relation), msg.header.data_len, 00, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
								// add for link relation data
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
									zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: send sim data req package[ret:%d] error(%d:%s)", \
										0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, ret, errno, strerror(errno));
									param->slotinfo.stat = SlotInvalid;
									//param->sockfd_netrdr = -1;
								}
								else
								{
									//TransferToRemote_nts(&ActEae,&Actn2st);
									emu_eng.SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_WAITING_FROM_NET_READY);

									//logDataToHex((unsigned char *)&msg.data+sizeof(link_relation_t), ntohs(msg.header.data_len)-sizeof(link_relation_t), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
									//logDataToHex((unsigned char *)&msg.data, ntohs(msg.header.data_len), 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
									//logDataToHex((unsigned char *)buff_send, (unsigned short)ret, 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_SIMHDLTASK);
								}
							}	
							else if (emu_eng.GetEmuApduNetApiStatus() == EMU_APDU_NETAPI_STATUS_DATA_TO_EMU_IS_READY)
							{
								buff_send[0] = param->slotinfo.slot_nbr; //0;
								buff_send[1] = CMD_IS_DATA; //0;
								buff_send[2] = (emu_eng.GetToEmuDataLength())>>8;
								buff_send[3] = ((emu_eng.GetToEmuDataLength())<<8)>>8;	
								emu_eng.CopyEaeDataToEmuBuf((unsigned char *)buff_send+4, (unsigned short *)&len_send);
								//logDataToHex((unsigned char *)buff_send+4, len_send, 33, 33, 33, DIRE_UNKOWN);
								msg.header.cmd = SIMDATA_RSP;
								msg.header.slot = param->slotinfo.slot_nbr;
								msg.header.data_len = len_send + 4;
								memcpy(msg.data, buff_send, len_send+4);
								ret = sock_comm.WriteData(sock_comm.m_sockfd, (char *)&msg, msg.header.data_len+sizeof(msg_header_t));
								emu_eng.SetEmuApduNetApiStatus(EMU_APDU_NETAPI_STATUS_IDLE);
								//logDataToHex((unsigned char *)&msg.data, msg.header.data_len, 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, DIRE_SLOTHDLTASK_TO_COMMHDLTASK);
							}
							else
							{
								zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: EmuEngine's ApduNetApi Status[%d] is not expected", \
									0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, emu_eng.GetEmuApduNetApiStatus());
								//sock_trans.CloseSocket();
								//param->slotinfo.stat = SlotInvalid;
							}
						}
						else
						{
							zprintf("[ERRO]SlotHdlTask[%02d-%02d-%02d]: EmuEngine's status[%d] is not EMU_ENGINE_WORKING", \
								0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr, emu_eng.GetEmuEngineStatus());
							param->slotinfo.stat = SlotInvalid;
						}
					}
				}
			}
		}
	}

	//
	sock_comm.CloseSocket();
	zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: close sock_comm", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
	sock_nettask.CloseSocket();
	zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: close sock_nettask", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);
	zprintf("[INFO]SlotHdlTask[%02d-%02d-%02d]: End", 0, param->slotinfo.gw_bank_nbr, param->slotinfo.slot_nbr);

	//
	return NULL;
}

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
	int len = 0;
	//int max = 0;
	unsigned int sockfd_net;
	time_t time_last = 0;
	NET_HANDLE_FUNC net_hdl;
	AST_HANDLE_FUNC ast_hdl;
	CSocketEx *psock_net = (CSocketEx *)pParam;
	sockfd_net = psock_net->getSocket();
	CSocketEx sock_sht;
	CSocketEx sock_ast;
	unsigned int sockfd_ast = 0;
	package_header_t *pheader = NULL;

	zprintf("[INFO]NetHdlTask: Start[net_hdl_port:%d, ast_hdl_port:%d]......", gSimEmuSvrParam.net_hdl_port, gSimEmuSvrParam.ast_hdl_port);

	// only send msg temporary
	sock_sht.Init(PROTO_UDP, gSimEmuSvrParam.local_ip, gSimEmuSvrParam.net_hdl_port, gSimEmuSvrParam.local_ip, 0, 1);
	sock_sht.CreateSocket();
	sock_sht.Bind();

	// use to send msg to proxysvr for call begin and call end
	sock_ast.Init(PROTO_UDP, gSimEmuSvrParam.local_ip, gSimEmuSvrParam.ast_hdl_port, gSimEmuSvrParam.local_ip, 0, 1);
	sock_ast.CreateSocket();
	sock_ast.Bind();
	sockfd_ast = sock_ast.getSocket();
	
	//
	int epollfd = -1;
	struct epoll_event events[3];
	int epollnum = 0;
	epollfd = epoll_create(3);
	add_event(epollfd, sock_sht.m_sockfd, EPOLLIN);
	add_event(epollfd, sock_ast.m_sockfd, EPOLLIN);
	add_event(epollfd, psock_net->m_sockfd, EPOLLIN);
	zprintf("[ERRO]NetHdlTask: epoll init ok.");
	
	while (1)
	{
		// heartbeat
		if ((time(NULL) - time_last) > gSimEmuSvrParam.hb_interval)
		{
			ret = EmuHeartbeat(psock_net);
			if (ret != NoErr)
			{
				psock_net->CloseSocket();
				downRegisterStat(&gRegisterStat);
				break;
			}
			time_last = time(NULL);
		}

		//
		epollnum = epoll_wait(epollfd, events, 3, 5000);
		if (epollnum < 0)
		{
			zprintf("[ERRO]NetHdlTask: epoll_wait error(%d:%s)", errno, strerror(errno));
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
				if (events[n].events & EPOLLIN && events[n].data.fd == sock_sht.m_sockfd)
				{ // data from sock_sht
					// read package
					memset(buff_recv, 0, sizeof(buff_recv));
					len= sock_sht.ReadData(sock_sht.m_sockfd, buff_recv, sizeof(buff_recv));
					if (len <= 0)
					{
						zprintf("[ERRO]NetHdlTask: recv package from Asterisk error(%d:%s)", errno, strerror(errno));
					}
					else
					{
						// link report
						ret = psock_net->WriteData(psock_net->m_sockfd, buff_recv, len);
						if (ret != len)
						{
							zprintf("[ERRO]NetHdlTask: send package to SimProxy error(%d:%s)", errno, strerror(errno));
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
						zprintf("[ERRO]NetHdlTask: recv package from Asterisk error(%d:%s)", errno, strerror(errno));
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
					// read package header
					memset(buff_recv, 0, sizeof(buff_recv));
					ret = psock_net->ReadData(psock_net->getSocket(), buff_recv, PACKAGE_HEADER_LEN);
					if (ret != PACKAGE_HEADER_LEN)
					{
						zprintf("[ERRO]NetHdlTask: recv package header(ret:%d, header_len:%d) from SimProxySvr error(%d:%s), maybe SimRdrSvr close the connection", \
							ret, PACKAGE_HEADER_LEN, errno, strerror(errno));
						psock_net->CloseSocket();
						//break;
						goto NETHDLTASK_QUIT;
					}
					else
					{
						pheader = (package_header_t *)buff_recv;
						/*zprintf("[INFO]NetHdlTask: package from net[version:0x%04x-cmd:0x%04x-len:0x%04x-result:0x%04x-reserve:0x%08x]", \
							ntohs(pheader->version), ntohs(pheader->cmd), ntohs(pheader->len), ntohs(pheader->result), ntohs(pheader->reserve));*/
						// read package reload
						len = getLen(buff_recv);
						
						ret = psock_net->ReadData(psock_net->getSocket(), buff_recv+PACKAGE_HEADER_LEN, len+2); //<-- no checksum
						if (ret != (len+2)) //<-- no checksum
						{
							zprintf("[ERRO]NetHdlTask: recv package reload(ret:%d, reload_len:%d) from SimRdrSvr error(%d:%s)", ret, len+2, errno, strerror(errno));
							psock_net->CloseSocket();
							//break;
							goto NETHDLTASK_QUIT;
						}
						else
						{
							// check package
							ret = checkPackage(buff_recv, PACKAGE_HEADER_LEN+len+2);
							if (ret != 0)
							{
								zprintf("[ERRO]NetHdlTask: package from SimRdrSvr checksum fail");
							}
							else
							{
								// handle
								net_hdl = findNetHandleFunc(getCmd(buff_recv));
								if (net_hdl != NULL)
								{
									memset(buff_send, 0, sizeof(buff_send));
									ret = net_hdl(buff_recv, buff_send, &sock_sht);
								}
							}
						}
					}
				}
			}
		}
	}
NETHDLTASK_QUIT:
	// unregister
	//emuUnRegister(psock_net);
	close(epollfd);
	//
	sock_sht.CloseSocket();
	sock_ast.CloseSocket();
	//
	zprintf("[INFO]NetHdlTask: End");
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
#if 1
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
			logDataToHex(buff_req, len, 0, board_nbr, slot_nbr, DIRE_COMMHDLTASK_TO_COMM);
			logDataToHex(buff_rsp, ret, 0, board_nbr, slot_nbr, DIRE_COMM_TO_COMMHDLTASK);
			zprintf("[INFO]setSimAtr[%02d-%02d-%02d]: set sim atr succ", 0, board_nbr, slot_nbr);
			res = 0;
			break;
		}
		else
		{
			logDataToHex(buff_req, len, 0, board_nbr, slot_nbr, DIRE_COMMHDLTASK_TO_COMM);
			logDataToHex(buff_rsp, ret, 0, board_nbr, slot_nbr, DIRE_COMM_TO_COMMHDLTASK);
			zprintf("[ERRO]setSimAtr[%02d-%02d-%02d]: set sim atr error(%d:%s)", 0, board_nbr, slot_nbr, errno, strerror(errno));
		}
	}
	return res;
}
#else
int setSimAtr(int handle, char *sim_atr, unsigned short atr_len, unsigned short board_nbr, unsigned short slot_nbr)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	unsigned int len = 0;
	int trys = 2;
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
		
		logDataToHex(buff_req, len, 0, board_nbr, slot_nbr, DIRE_COMMHDLTASK_TO_COMM);
		ret = serial_wr_atr(handle, buff_req, len, buff_rsp, len, SERIAL_WR_TIMEOUT);
		logDataToHex(buff_rsp, ret, 0, board_nbr, slot_nbr, DIRE_COMM_TO_COMMHDLTASK);
		if (buff_rsp[0] == (unsigned char)slot_nbr && buff_rsp[1] == 0x80 && len == (buff_rsp[3]+4))
		{
			//zprintf("[INFO]setSimAtr[%02d-%02d-%02d]: set sim atr succ", 0, board_nbr, slot_nbr);
			res = 0;
			Sleep(1000);
			break;
		}
		zprintf("[ERRO]setSimAtr[%02d-%02d-%02d]: set sim atr error(%d:%s)", 0, board_nbr, slot_nbr, errno, strerror(errno));
		//Sleep(1000);
	}
	return res;
}
#endif

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

#if 1
#if 1
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
		zprintf("[ERROR]socket_pty_wr: write data to socket[%d][len_w:%d, ret:%d] error(%s:%d)", csock->m_sockfd, len_w, ret, strerror(errno), errno);
		return -1;
	}
	while (1)
	{
		if (csock->checkSocketReady(csock->m_sockfd, READ_WAIT, 0, 600000))
		{
			ret = csock->UDPReadData(csock->m_sockfd, (char *)buff_r+readn, 320);
			if (ret < 0)
			{
				zprintf("[ERROR]socket_pty_wr: read data from socket[%d] error(%s:%d)", csock->m_sockfd, strerror(errno), errno);
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
#else
int socket_pty_wr(CSocketEx *csock, unsigned char *buff_w, int len_w, unsigned char *buff_r, int *len_r, int timeout)
{
	int ret = 0;
	
	ret = csock->WriteData(csock->m_sockfd, (char *)buff_w, len_w);
	if (ret != len_w)
	{
		zprintf("[ERROR]udpWR: write data to socket[%d][len_w:%d, ret:%d] error(%s:%d)", csock->m_sockfd, len_w, ret, strerror(errno), errno);
		return -1;
	}
	if (csock->checkSocketReady(csock->m_sockfd, READ_WAIT, 0, 600000))
	{
		ret = csock->UDPReadData(csock->m_sockfd, (char *)buff_r, len_r);
		if (ret < 0)
		{
			zprintf("[ERROR]udpWR: read data from socket[%d] error(%s:%d)", csock->m_sockfd, strerror(errno), errno);
			return -1;
		}
		else
		{
			return ret;
		}
	}
	return 0;
}
#endif
#else
int socket_pty_wr(CSocketEx *csock, unsigned char *buff_w, int len_w, unsigned char *buff_r, int *len_r, int timeout)
{
	int ret = 0;
	int len_body = 0;
	ret = csock->WriteData(csock->m_sockfd, (char *)buff_w, len_w);
	if (ret != len_w)
	{
		zprintf("[ERROR]socket_pty_wr: write data to socket[%d][len_w:%d, ret:%d] error(%s:%d)", csock->m_sockfd, len_w, ret, strerror(errno), errno);
		return -1;
	}
	if (csock->checkSocketReady(csock->m_sockfd, READ_WAIT, 0, 200000))
	{
		ret = csock->ReadData(csock->m_sockfd, (char *)buff_r, 4);
		if (ret != 4)
		{
			zprintf("[ERROR]socket_pty_wr: read header error(%d:%s).", errno, strerror(errno));
			return -1;
		}
		else
		{
			*len_r = 4;
			len_body = (buff_r[2]<<8) + buff_r[3];
			if (len_body > 0)
			{
				ret = csock->ReadData(csock->m_sockfd, (char *)buff_r+4, len_body);
				if (ret != len_body)
				{
					zprintf("[ERROR]socket_pty_wr: read body error(%d:%s).", errno, strerror(errno));
					if (ret > 0)
					{
						*len_r += ret;
					}
					return -1;
				}
				*len_r += ret;
			}
		}
	}
	return 0;
}
#endif

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
	unsigned int sockfd = 0;
	unsigned char slot_nbr = 0;
	commhdltask_param_t *param = (commhdltask_param_t *)pParam;
	struct timeval tv_read;
	struct timeval tv_write;
	//unsigned long total_emu = 0;
	//unsigned long total_proc = 0;
	//int i = 0;
	//char cmd[320] = {0};
	int burst = 0;

	zprintf("[INFO]CommHdlTask[00-%02d]: Start[ip:%s, port:%d]......", param->board_nbr, param->ip, param->port);

	
	// sock open
	sock_sht.Init(PROTO_UDP, param->ip, param->port, param->ip, 0, 1);
	sock_sht.CreateSocket();
	sock_sht.Bind();
	sockfd = sock_sht.getSocket();

	gettimeofday(&tv_read, NULL);
	gettimeofday(&tv_write, NULL);
	while (param->stop != 1)
	{
		if (param->handle < 0) // need initialize to -1 in main func
		{
			param->handle = open_serial(param->tty, 115200, 0, 8, 1, 0);
			if (param->handle < 0)
			{
				zprintf("[ERRO]CommHdlTask[00-%02d]: open comm(%s) error(%d:%s)", param->board_nbr, param->tty, errno, strerror(errno));
				Sleep(3000);
				continue;
			}
			zprintf("[INFO]CommHdlTask[00-%02d]: open comm(%s) succ", param->board_nbr, param->tty);
			tcflush(param->handle, TCIOFLUSH);

			// reset STM32F103
			resetSTM32(param->handle, param->board_nbr);
			// get STM32F103 version
			getSTM32Version(param->handle, param->board_nbr);
			// reset Mini52
			resetAllMini52(param->handle, param->board_nbr);
			// get channel version
			getAllMini52Version(param->handle, param->board_nbr);
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
							zprintf("[ERRO]CommHdlTask[00-%02d]: Data from Emu invalid!!!", param->board_nbr);
							logDataToHex((unsigned char *)pbuff, (unsigned short)(len - pos), 0, param->board_nbr, 80, DIRE_COMM_TO_COMMHDLTASK);
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
						logDataToHex((unsigned char *)pbuff, (unsigned short)plen, 0, param->board_nbr, (unsigned short)slot_nbr, DIRE_COMM_TO_COMMHDLTASK);
			
						makeEmuDataPackage(&msg, pbuff, plen);

						if (slot_nbr > 7 && slot_nbr != SLOT_IS_EMU_CTRL)
						{
							zprintf("[ERRO]CommHdlTask[00-%02d]: slot_nbr(%d) invalid!!!", param->board_nbr, slot_nbr);
							logDataToHex((unsigned char *)pbuff, (unsigned short)plen, 0, param->board_nbr, 80, DIRE_COMM_TO_COMMHDLTASK);
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
								//logDataToHex((unsigned char *)msg.data, (unsigned short)msg.header.data_len, 0, param->board_nbr, (unsigned short)slot_nbr, DIRE_COMMHDLTASK_TO_SLOTHDLTASK);
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
							logDataToHex((unsigned char *)pbuff, (unsigned short)plen, 0, param->board_nbr, 80, DIRE_COMM_TO_COMMHDLTASK);
							zprintf("[ERRO]CommHdlTask[00-%02d]: slot_nbr(%d) invalid", param->board_nbr, slot_nbr);
						}
					}
				}
			}
			else
			{
				zprintf("[ERRO]CommHdlTask[00-%02d]: Data From Emu Serial Error", param->board_nbr);
				logDataToHex((unsigned char *)buff, (unsigned short)len, 0, param->board_nbr, 80, DIRE_COMM_TO_COMMHDLTASK);
				exit(0);
			}
		}
		else
		{
			zprintf("[ERRO]CommHdlTask[00-%02d]: socket_pty_wr[80 97 00 00] error(%d:%s)", param->board_nbr, errno, strerror(errno));
			
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
			zprintf("[ERRO]CommHdlTask[00-%02d]: select error(%d:%s)", param->board_nbr, errno, strerror(errno));
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
				zprintf("[ERRO]CommHdlTask[00-%02d]: recv data from SlotHdlTask error(%d:%s)", param->board_nbr, errno, strerror(errno));
			}
			else
			{
				if (msg.header.cmd == SIMDATA_RSP)
				{
					//logDataToHex((unsigned char *)&msg.data, msg.header.data_len, 0, param->board_nbr, (unsigned short)((unsigned char *)msg.data)[0], DIRE_SLOTHDLTASK_TO_COMMHDLTASK);
					if (((unsigned char *)msg.data)[1] == CMD_IS_SETATR)
					{
						logDataToHex((unsigned char *)msg.data, msg.header.data_len, 0, param->board_nbr, (unsigned short)((unsigned char *)msg.data)[0], DIRE_COMMHDLTASK_TO_COMM);
						//ret = socket_pty_wr(&sock_pty, (unsigned char *)msg.data, (unsigned int)msg.header.data_len, (unsigned char *)buff, &len, SERIAL_WR_TIMEOUT);
						len = serial_wr(param->handle, (unsigned char *)msg.data, (unsigned int)msg.header.data_len, (unsigned char *)buff, len, SERIAL_WR_TIMEOUT);
						if (len > 0)
						{
							//logDataToHex((unsigned char *)buff, (unsigned short)len, 0, param->board_nbr, (unsigned short)((unsigned char *)msg.data)[0], DIRE_COMM_TO_COMMHDLTASK);
						}
					}
					else
					{
						//len = socket_pty_w(&sock_pty, (unsigned char *)msg.data, (unsigned int)msg.header.data_len, SERIAL_WR_TIMEOUT);
						len = serial_write_n(param->handle, (unsigned char *)msg.data, (unsigned int)msg.header.data_len, SERIAL_WR_TIMEOUT);
						if (len != msg.header.data_len)
						{
							zprintf("[ERRO]CommHdlTask[00-%02d-%02d]: write data[len_w:%d, ret:%d] to serial error(%d:%s)", param->board_nbr, msg.header.slot, msg.header.data_len, len, errno, strerror(errno));
						}
						else
						{
							logDataToHex((unsigned char *)msg.data, msg.header.data_len, 0, param->board_nbr, (unsigned short)((unsigned char *)msg.data)[0], DIRE_COMMHDLTASK_TO_COMM);
						}
					}
				}
				else if (msg.header.cmd == SIMLINK_CREATE_REQ)
				{
					//zprintf("[INFO]CommHdlTask[%d]: receive SIMLINK_CREATE_REQ(slot:%d) .........", param->board_nbr, msg.header.slot);
					//resetMini52(&sock_pty, param->board_nbr, msg.header.slot);
					setSimAtr(param->handle, msg.data, msg.header.data_len, param->board_nbr, msg.header.slot);
					//zprintf("[INFO]CommHdlTask[%d]: set sim(slot:%d) atr succ, within link create", param->board_nbr, msg.header.slot);
					// 开始轮询emu
					//ena_wr_comm = 1;
					
					//gsm power reset
					ast_login(&param->session, AST_USER, AST_PASSWD, param->ami_ip, AST_PORT);
					if (param->session.fd > 0)
					{
						len = gsm_power_reset(&param->session, msg.header.slot+1);
						//gsm_power_off(&param->session, msg.header.slot+1);
						//gsm_power_on(&param->session, msg.header.slot+1);
						zprintf("[INFO]CommHdlTask[00-%02d-%02d]: gsm_power_reset_%d[%s:%d] succ", param->board_nbr, msg.header.slot, msg.header.slot+1, param->ami_ip, AST_PORT);
						ast_logout(&param->session);
					}
				}
				else if (msg.header.cmd == SIMLINK_RELEASE_REQ)
				{
					resetMini52WithoutSleep(param->handle, param->board_nbr, msg.header.slot);
					zprintf("[INFO]CommHdlTask[00-%02d-%02d]: reset Mini52 succ, within link release", param->board_nbr, msg.header.slot);

					// update redis
					int chan_no = param->board_nbr * SLOT_NBR + msg.header.slot + 1;
					redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
					if (context->err)
					{
						zprintf("[ERRO]CommHdlTask[00-%02d-%02d]: redisConnect error(%s)", param->board_nbr, msg.header.slot, context->errstr);
					}
					else
					{
						redisReply *reply = (redisReply *)redisCommand(context, "SET app.asterisk.gsmstatus.channel%d 1", chan_no);
						if (reply->type == REDIS_REPLY_ERROR)
						{
 							zprintf("[ERRO]CommHdlTask[00-%02d-%02d]: redisCommand(SET app.asterisk.gsmstatus.channel%d 1) error(%s)", \
 								param->board_nbr, msg.header.slot, chan_no, reply->str);
						}
						else
						{
							zprintf("[INFO]CommHdlTask[00-%02d-%02d]: redisCommand(SET app.asterisk.gsmstatus.channel%d 1) succ", \
								param->board_nbr, msg.header.slot, chan_no);
						}
						freeReplyObject((void *)reply);
					}
					redisFree(context);
				}
				else if (msg.header.cmd == POWER_RESET_SPAN_REQ)
				{
					//zprintf("[INFO]CommHdlTask[00-%02d-%02d]: power reset span %d for channel offline timeout", param->board_nbr, msg.header.slot, msg.header.slot+1);
					
					/*resetMini52WithoutSleep(&sock_pty, param->board_nbr, msg.header.slot);
					if (msg.header.data_len > 0)
					{
						setSimAtr(&sock_pty, msg.data, msg.header.data_len, param->board_nbr, msg.header.slot);
					}*/
					ast_login(&param->session, AST_USER, AST_PASSWD, param->ami_ip, AST_PORT);
					if (param->session.fd > 0)
					{
						gsm_power_reset(&param->session, msg.header.slot+1);
						//gsm_power_off(&param->session, msg.header.slot+1);
						//gsm_power_on(&param->session, msg.header.slot+1);
						zprintf("[INFO]CommHdlTask[00-%02d-%02d]: gsm_power_reset_%d[%s:%d] succ for channel offline timeout", param->board_nbr, msg.header.slot, msg.header.slot+1, param->ami_ip, AST_PORT);
						ast_logout(&param->session);
					}
				}
				else if (msg.header.cmd == SIM_PULLPLUG_NOTICE_REQ)
				{
					resetMini52WithoutSleep(param->handle, param->board_nbr, msg.header.slot);
					zprintf("[INFO]CommHdlTask[00-%02d-%02d]: reset Mini52 succ, within link release", param->board_nbr, msg.header.slot);

					// update redis
					int chan_no = param->board_nbr * SLOT_NBR + msg.header.slot + 1;
					redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
					if (context->err)
					{
						zprintf("[ERRO]CommHdlTask[00-%02d-%02d]: redisConnect error(%s)", param->board_nbr, msg.header.slot, context->errstr);
					}
					else
					{
						redisReply *reply = (redisReply *)redisCommand(context, "SET app.asterisk.gsmstatus.channel%d 1", chan_no);
						if (reply->type == REDIS_REPLY_ERROR)
						{
 							zprintf("[ERRO]CommHdlTask[00-%02d-%02d]: redisCommand(SET app.asterisk.gsmstatus.channel%d 1) error(%s)", \
 								param->board_nbr, msg.header.slot, chan_no, reply->str);
						}
						else
						{
							zprintf("[INFO]CommHdlTask[00-%02d-%02d]: redisCommand(SET app.asterisk.gsmstatus.channel%d 1) succ", \
								param->board_nbr, msg.header.slot, chan_no);
						}
						freeReplyObject((void *)reply);
					}
					redisFree(context);
				}
				else
				{
					zprintf("[INFO]CommHdlTask[00-%02d-%02d]: data from SlotHdlTask invalid", param->board_nbr, msg.header.slot);
					logDataToHex((unsigned char *)&msg, sizeof(msg.header)+msg.header.data_len, 0, param->board_nbr, (unsigned short)((unsigned char *)msg.data)[0], DIRE_SLOTHDLTASK_TO_COMMHDLTASK);
				}
			}
		}
	}
	sock_sht.CloseSocket();
	close_serial(param->handle);
	
	zprintf("[INFO]CommHdlTask[00-%02d]: End", param->board_nbr);
	//
	return NULL;
}


#if 0
int testconnection(void)
{
	CSocketEx sock_trans;
	

	time_t time_last = time(NULL);
	while ((time(NULL) - time_last) < 30)
	{
		// connect sim link
		sock_trans.Init(PROTO_TCP, "172.16.6.101", 4201, 1678119084, 3001, 10);
		if (sock_trans.CreateSocket())
		{
		#if 1
			if (!sock_trans.Bind())
			{
				zprintf("bind socket error(%d:%s)", errno, strerror(errno));
				sock_trans.CloseSocket();
				continue;
			}
		#endif
			if (!sock_trans.ConnectNonB(sock_trans.getSocket(), 3))
			{
				sock_trans.CloseSocket();
				zprintf("connect to server error(%d:%s)", errno, strerror(errno));
			}
			else
			{
				zprintf("connect to server succ");
				break;
			}
		}
	}		

	Sleep(20000);
	sock_trans.CloseSocket();
	return 0;
}

int testSerial(char *serial_file)
{
	int len = 0;
	char buff[BUFF_SIZE] = {0};
	char version[BUFF_SIZE] = {0};
	int serial_fd = -1;
	
	if (serial_file == NULL)
	{
		return -1;
	}
	printf("serial_file = [%s]\n", serial_file);


	serial_fd = open_serial(serial_file, 115200, 0, 8, 1, 0);
	if (serial_fd < 0)
	{
		printf("open %s error(%s)\n", serial_file, strerror(errno));
		return -1;
	}
	printf("open %s succ\n", serial_file);
	
	// reset emu
	memset(buff, 0, sizeof(buff));
	len = serial_wr(serial_fd, (unsigned char *)"\x80\x85\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
	if (len < 0)
	{
		printf("CommHdlTask: reset Emu fail\n");
	}
	printf("CommHdlTask: reset Emu succ\n");
	Sleep(1000);
	//sleep(1);


	// get STM32F103 version
	memset(buff, 0, sizeof(buff));
	len = serial_wr(serial_fd, (unsigned char *)"\x80\x83\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
	if (len > 0)
	{
		if (((unsigned char *)buff)[0] == SLOT_IS_EMU_CTRL && ((unsigned char *)buff)[1] == IS_VER)
		{
			memcpy(version, buff+4, len);
			printf("CommHdlTask: get Emu Version succ[%s]\n", version);
		}
		else
		{
			printf("CommHdlTask: get Emu Version error\n");
		}
	}

	//
	close_serial(serial_fd);
	zprintf("testSerial: close serial");
	
	// return
	return 0;
}
#endif

int ast_login(struct mansession *session, char * username, char *secret, char *host, int port)
{
    if (session == NULL || username == NULL || secret == NULL || host == NULL || port < 0)
	{
        zprintf("[ERRO]ast_connect: args invalid");
		return -1;
    }

    session = astman_open_r(session);
    session->debug = 0;
    astman_connect(session, host, port);
    if (astman_login(session, username, secret) == ASTMAN_FAILURE)
    {
        zprintf("[ERRO]ast_connect: astman_login[%s:%d] error(%d:%s)", host, port, errno, strerror(errno));
    	astman_disconnect(session);
        return -1;
    }
	//zprintf("[INFO]ast_connect: astman_login[%s:%d] succ", host, port);
	return 0;
}
int ast_logout(struct mansession *session)
{
	char ast_ip[16] = {0};
	strcpy(ast_ip, inet_ntoa(session->sin.sin_addr));
    astman_logoff(session);
	//zprintf("[INFO]ast_logout: astman_logoff[%s:%d] ok", ast_ip, AST_PORT);
    astman_disconnect(session);
	//zprintf("[INFO]ast_logout: astman_disconnect[%s:%d] ok", ast_ip, AST_PORT);
	return 0;
}

int gsm_power_reset(struct mansession *session, unsigned short slot_nbr)
{
    return astman_manager_action(session, "Command", "Command: gsm power reset %d\r\n", slot_nbr);
}
int gsm_power_off(struct mansession *session, unsigned short slot_nbr)
{
    return astman_manager_action(session, "Command", "Command: gsm power off %d\r\n", slot_nbr);
}
int gsm_power_on(struct mansession *session, unsigned short slot_nbr)
{
    return astman_manager_action(session, "Command", "Command: gsm power on %d\r\n", slot_nbr);
}




int get_config(char *cfg_file, char *domain, char *item, char *content)
{
	FILE *fp = NULL;
	char cmd[1024] = {0};
	if (cfg_file == NULL || domain == NULL || item == NULL || content == NULL)
	{
		return -1;
	}

	sprintf(cmd, "/my_tools/set_config %s get option_value %s %s", cfg_file, domain, item);
	fp = popen(cmd, "r");
	if (fp == NULL)
	{
		return -1;
	}
	while (!feof(fp))
	{
		fgets(content, 1024, fp);
	}
	pclose(fp);
	zprintf("[INFO]get_mode_info: content[%s]", content);
	return 0;
}


int start_master_mode(int board_nbr)
{
	int inc = 0;
	int i = 0;
	int j = 0;
	
	zprintf("[INFO]============================== start_master_mode Start ==============================");
	//create_local_pty();
START:
	// register
	if (emuRegister(&gSockReg) != NoErr)
	{
		Sleep(3000);
		goto START;
	}
	upRegisterStat(&gRegisterStat);
	zprintf("[INFO]main: up register stat succ");

	
	//
	for (j = 0; j < board_nbr; j++)
	{
		// 需要重新构建数据结构，最外层为板卡编号
		// 涉及到网络报文格式和simbank和SimServer的相关数据结构，需要修改

		
		// CommHdlTask start
		gCommHdlTaskParam[j].stop = 0;
		gCommHdlTaskParam[j].handle = -1;
		gCommHdlTaskParam[j].board_nbr = j;
		//strcpy(gCommHdlTaskParam[j].tty, g_tty_info.tty[j]);
		sprintf(gCommHdlTaskParam[j].tty, "%s%d", gSimEmuSvrParam.comm, j+1);
		//strcpy(gCommHdlTaskParam[j].ip_tty, g_tty_info.ip[j]);
		strcpy(gCommHdlTaskParam[j].ip, gSimEmuSvrParam.local_ip);
		gCommHdlTaskParam[j].port = gSimEmuSvrParam.comm_hdl_port + j;
		gCommHdlTaskParam[j].initialed = 0;
		memcpy(gCommHdlTaskParam[j].ami_ip, gSimEmuSvrParam.local_ip, strlen(gSimEmuSvrParam.local_ip));
		pthread_attr_init(&gCommHdlTaskParam[j].attr);
		pthread_attr_setinheritsched(&gCommHdlTaskParam[j].attr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&gCommHdlTaskParam[j].attr, SCHED_RR);
		gCommHdlTaskParam[j].thr_param.sched_priority = sched_get_priority_max(SCHED_RR);
		pthread_attr_setschedparam(&gCommHdlTaskParam[j].attr, &gCommHdlTaskParam[j].thr_param);
		pthread_attr_setscope(&gCommHdlTaskParam[j].attr, PTHREAD_SCOPE_SYSTEM);
		pthread_create(&gCommHdlTaskParam[j].tid, &gCommHdlTaskParam[j].attr, CommHdlTask, &gCommHdlTaskParam[j]);

		
		// SlotHdlTask start
		for (i = 0; i < SLOT_NBR; i++)
		{
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
			pthread_attr_init(&gSlotHdlTaskParam[j][i].attr);
			pthread_attr_setinheritsched(&gSlotHdlTaskParam[j][i].attr, PTHREAD_EXPLICIT_SCHED);
			pthread_attr_setschedpolicy(&gSlotHdlTaskParam[j][i].attr, SCHED_RR);
			gSlotHdlTaskParam[j][i].thr_param.sched_priority = sched_get_priority_max(SCHED_RR);
			pthread_attr_setschedparam(&gSlotHdlTaskParam[j][i].attr, &gSlotHdlTaskParam[j][i].thr_param);
			pthread_attr_setscope(&gSlotHdlTaskParam[j][i].attr, PTHREAD_SCOPE_SYSTEM);
			//pthread_create(&(gSlotHdlTaskTid[j][i]), NULL, SlotHdlTask, &(gSlotHdlTaskParam[j][i]));
			pthread_create(&(gSlotHdlTaskTid[j][i]), &gSlotHdlTaskParam[j][i].attr, SlotHdlTask, &(gSlotHdlTaskParam[j][i]));
			inc++;
		}
		Sleep(1000);
	}
	g_link_nbr = inc;
	
	Sleep(2000);
	// NetHdlTask start
	
	pthread_attr_init(&attr_nethdltask);
	pthread_attr_setinheritsched(&attr_nethdltask, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_nethdltask, SCHED_RR);
	param_nethdltask.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&attr_nethdltask, &param_nethdltask);
	pthread_attr_setscope(&attr_nethdltask, PTHREAD_SCOPE_SYSTEM);
	//pthread_create(&gNetHdlTaskTid, NULL, NetHdlTask, (void *)&gSockReg);
	pthread_create(&gNetHdlTaskTid, &attr_nethdltask, NetHdlTask, (void *)&gSockReg);


	//////////////////////////////////////////////////////////////////////////////////////////////////////

	
	// NetHdlTask stop
	pthread_join(gNetHdlTaskTid, NULL);
	pthread_attr_destroy(&attr_nethdltask);

	
	for (j = 0; j < board_nbr; j++)
	{
		//if (strlen(g_tty_info.tty[j]) > 0)
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
		//if (strlen(g_tty_info.tty[j]) > 0)
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
	system("/usr/bin/killall socat");
	zprintf("[INFO]============================== start_master_mode End ==============================");
	return 0;
}


// 检查进程是否启动，启动返回1，停止返回0
int checkProcessExist(char *proc_name)
{
	FILE *fp = NULL;
	int ret = 0;
	char cmd[1024] = {0};
	char content[1024] = {0};

	sprintf(cmd, "ps -ef | grep %s | grep -v grep | cut -c 0-5", proc_name);
	fp = popen(cmd, "r");
	if (fp == NULL)
	{
		return -1;
	}
	while (!feof(fp))
	{
		fgets(content, sizeof(content), fp);
		if (strlen(content) > 0)
		{
			ret = 1;
			break;
		}
	}
	pclose(fp);
	//zprintf("[INFO]checkProcessExist: process[%s]'s status[%d]", proc_name, ret);
	return ret;
}


//
#define TTY "/dev/ttyS1"
int test_serial(char *tty, int baudrate)
{
	char buff[320] = {0};
	int len = 0;
	int gCommHandle = -1;
	//char gEmuVersion[320] = {0};
	unsigned short i = 0;
	char cmd[320] = {0};
	int ret = 0;
	int cnt = 1000;

	zprintf("[INFO]test_serial: Start......");

	//system("killall socat");
	//Sleep(1000);
	//system("/my_tools/socat -d -d -d -d pty,link=/dev/ttyS2,rawer,echo=0, tcp:192.168.141.159:5501 &");
	//Sleep(1000);	
	gCommHandle = open_serial(tty, baudrate, 0, 8, 1, 0);
	if (gCommHandle == -1)
	{
		zprintf("[ERRO]test_serial: open comm(%s:%d) error(%d:%s)", tty, baudrate, errno, strerror(errno));
		return -1;
	}
	zprintf("[INFO]test_serial: open comm(%s:%d) succ", tty, baudrate);


	while (cnt-- > 0)
	{
		// get STM32F103 version
		memset(buff, 0, sizeof(buff));
		//logDataToHex((unsigned char *)"\x80\x83\x00\x00", 4, 0, 1, 80, DIRE_COMMHDLTASK_TO_COMM);
		len = serial_wr(gCommHandle, (unsigned char *)"\x80\x83\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
		//logDataToHex((unsigned char *)buff, (unsigned short)len, 0, 1, 80, DIRE_COMM_TO_COMMHDLTASK);
		if (len > 0)
		{
			if (((unsigned char *)buff)[0] == SLOT_IS_EMU_CTRL && ((unsigned char *)buff)[1] == IS_VER)
			{
				zprintf("[INFO]CommHdlTask[%d]: Emu Version [%s]", 1, buff+4);
			}
		}
	}


	close_serial(gCommHandle);
	gCommHandle = -1;
	zprintf("[INFO]test_serial: close comm(%s:%d) succ", tty, baudrate);
	
	zprintf("[INFO]test_serial: End......");
	return 0;
}
//
#if 0
#define TTY "/dev/ttyS1"
int test_serial(void)
{
	char buff[320] = {0};
	int len = 0;
	int gCommHandle = -1;
	//char gEmuVersion[320] = {0};
	unsigned short i = 0;
	char cmd[320] = {0};
	int ret = 0;

	zprintf("[INFO]test_serial: Start......");

	//system("killall socat");
	//Sleep(1000);
	//system("/my_tools/socat -d -d -d -d pty,link=/dev/ttyS2,rawer,echo=0, tcp:192.168.141.159:5501 &");
	//Sleep(1000);	
	gCommHandle = open_serial(TTY, 115200, 0, 8, 1, 0);
	if (gCommHandle == -1)
	{
		zprintf("[ERRO]test_serial: open comm(%s) error(%d:%s)", TTY, errno, strerror(errno));
		return -1;
	}
	zprintf("[INFO]test_serial: open comm(%s) succ", TTY);

	memset(buff, 0, sizeof(buff));
	//len = serial_wr(param->handle, (unsigned char *)"\x80\x85\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
	logDataToHex((unsigned char *)"\x80\x85\x00\x00", 4, 0, 1, 80, DIRE_COMMHDLTASK_TO_COMM);
	len = serial_wr_atr(gCommHandle, (unsigned char *)"\x80\x85\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
	logDataToHex((unsigned char *)buff, 4, 0, 1, 80, DIRE_COMM_TO_COMMHDLTASK);
	if (len < 0)
	{
		zprintf("[ERRO]test_serial[%d]: reset Emu fail", 1);
	}
	zprintf("[INFO]test_serial[%d]: reset Emu succ", 1);
	Sleep(1000);


	// get STM32F103 version
	memset(buff, 0, sizeof(buff));
	logDataToHex((unsigned char *)"\x80\x83\x00\x00", 4, 0, 1, 80, DIRE_COMMHDLTASK_TO_COMM);
	len = serial_wr_atr(gCommHandle, (unsigned char *)"\x80\x83\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
	logDataToHex((unsigned char *)buff, (unsigned short)len, 0, 1, 80, DIRE_COMM_TO_COMMHDLTASK);
	if (len > 0)
	{
		if (((unsigned char *)buff)[0] == SLOT_IS_EMU_CTRL && ((unsigned char *)buff)[1] == IS_VER)
		{
			zprintf("[INFO]test_serial[%d]: Emu Version [%s]", 1, buff+4);
		}
	}

	// reset Mini52
	//resetAllMini52(gCommHandle, 1);
	for (i = 0; i < 8; i++)
	{
		ret = resetMini52(gCommHandle, 1, i);
		if (ret < 0)
		{
			zprintf("[ERRO]test_serial: reset Mini52[slot:%d] error", i);
		}
		zprintf("[INFO]test_serial: reset Mini52[slot:%d] succ", i);
	}

	// get channel version
	for (i = 0; i < SLOT_NBR; i++)
	{
		// get channel version
		memset(buff, 0, sizeof(buff));
		cmd[0] = i;
		cmd[1] = 0x83;
		cmd[2] = 0;
		cmd[3] = 0;
		logDataToHex((unsigned char *)cmd, 4, 0, 1, i, DIRE_COMMHDLTASK_TO_COMM);
		//len = serial_wr_atr(gCommHandle, (unsigned char *)cmd, 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
		len = serial_wr_atr(gCommHandle, (unsigned char *)cmd, 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
		logDataToHex((unsigned char *)buff, (unsigned short)len, 0, 1, i, DIRE_COMM_TO_COMMHDLTASK);
		if (len > 0)
		{
			if (((unsigned char *)buff)[0] == i && ((unsigned char *)buff)[1] == IS_VER)
			{
				zprintf("[INFO]test_serial[%d]: Emu Channel[%d] Version [%s]", 1, i, buff+4);
			}
		}
        else
        {
                zprintf("[ERRO]test_serial[%d]: get Emu Channel[%d] Version error(%s:%d)", 1, i, strerror(errno), errno);
        }
	}
	
	while (1)
	{
		memset(buff, 0, sizeof(buff));
		cmd[0] = 6;
		cmd[1] = 0x83;
		cmd[2] = 0;
		cmd[3] = 0;
		logDataToHex((unsigned char *)cmd, 4, 0, 1, 6, DIRE_COMMHDLTASK_TO_COMM);
		//len = serial_wr_atr(gCommHandle, (unsigned char *)cmd, 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
		len = serial_wr_atr(gCommHandle, (unsigned char *)cmd, 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
		logDataToHex((unsigned char *)buff, (unsigned short)len, 0, 1, 6, DIRE_COMM_TO_COMMHDLTASK);
		if (len > 0)
		{
			if (((unsigned char *)buff)[0] == 6 && ((unsigned char *)buff)[1] == IS_VER)
			{
				zprintf("[INFO]test_serial[%d]: Emu Channel[%d] Version [%s]", 1, 6, buff+4);
			}
		}
        else
        {
                zprintf("[ERRO]test_serial[%d]: get Emu Channel[%d] Version error(%s:%d)", 1, 6, strerror(errno), errno);
        }
        Sleep(1000);
	}

	close_serial(gCommHandle);
	gCommHandle = -1;
	zprintf("[INFO]test_serial: close comm(%s) succ", TTY);
	
	zprintf("[INFO]test_serial: End......");
	return 0;
}
#endif

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
	int i = 0;
	//test_serial();
	//return 0;
	if (argc < 3)
	{
		printf("Usage: SimEmuTest tty[/dev/ttyUSB2] baudrate[115200]\n");
		return -1;
	}
	pthread_t tid_log;
	test_serial(argv[1], atoi(argv[2]));
	//test_select();
	return 0;

	log_mon_dhl_param_t log_hdl_param;
	memset((char *)&log_hdl_param, 0, sizeof(log_hdl_param));
	log_hdl_param.stop = 0;
	strcpy(log_hdl_param.file_name, "/data/log/simemusvr.log");
	log_hdl_param.file_size = 10;
	log_hdl_param.intv = 60;
	pthread_create(&tid_log, NULL, logMonHdl, (void *)&log_hdl_param);
		
SIMEMUSVR_START:
	zprintf("[INFO]============================== SimEmuSvr Start ==============================");
	char switch_mode[64] = {0};
	get_config("/etc/cfg/simemusvr.conf", "SimEmuSvr", "simemusvr_switch", switch_mode);
	if (strstr(switch_mode, "no") != NULL)
	{
		Sleep(5000);
		goto SIMEMUSVR_END;
	}
	

	// 获取本地/远程模式。如果本地模式，则退出。
	// 暂时只支持网关整体统一本地模式或远程模式
	gCommHandle = -1;
	//gCommHdlTaskParam.stop = 0;
	for (i = 0; i < MAX_BOARD; i++)
	{
		gCommHdlTaskParam[i].stop = 0;
	}

	memset(&g_slave_info, 0, sizeof(g_slave_info));
	
	// init
	initRegisterStat(&gRegisterStat);
	
	//
	memset(&gMini52Version, 0, sizeof(mini52_version_t)*SLOT_NBR);
	//
	memset((char *)&gSimEmuSvrParam, 0, sizeof(gSimEmuSvrParam));
	readConfigValue();

	start_master_mode(MAX_BOARD);
	
	goto SIMEMUSVR_START;
SIMEMUSVR_END:
	zprintf("[INFO]============================== SimEmuSvr End ==============================");
	log_hdl_param.stop = 1;
	pthread_join(tid_log, NULL);
	
	return 0;
}
