/*
	server.c

	Example gsm power handle service server in C

	Compilation in C (see samples/gsm32_mcu_hdl/gsm32_mcu_hdl.h):
	$ soapcpp2 -c gsm_mcu_hdl.h
	$ cc -o server server.c stdsoap2.c soapC.c soapServer.c
	where stdsoap2.c is in the 'gsoap' directory, or use libgsoap:
	$ cc -o server server.c soapC.c soapServer.c -lgsoap

--------------------------------------------------------------------------------
gSOAP XML Web services tools
Copyright (C) 2001-2008, Robert van Engelen, Genivia, Inc. All Rights Reserved.
This software is released under one of the following two licenses:
GPL or Genivia's license for commercial use.
--------------------------------------------------------------------------------
GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------
A commercial use license is available from Genivia, Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#include "soapH.h"
#include "gsm_mcu_hdl.nsmap"

#include "czmq.h"
#include "serial.h"

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
//#include <mcheck.h>

#define SOAP_TMPLEN         (1024)

//=====================================================
#define BACKLOG				(100)
#define MAX_THR				(10)
#define MAX_QUEUE			(1000)

#define MAX_REG				(13)

#define MAX_REQ_LEN			(32)
#define MAX_RSP_LEN			(32)
#define MAX_CHN_PER_REG		(8)


#define SERIAL_READ_TIMEOUT	(2)

#define ALL_BITS			(-1)

#define HDL_OK				(0)
#define HDL_ERR				(-1)
#define HDL_MCU_ERR			(-2)

#define MODULE_NAME_M35        "Quectel_M35"
#define MODULE_NAME_SIM6320C   "SIMCOM_SIM6320C"

#define MCU_INFO_FILE	"/tmp/mcu_info"
#define MODULE_MAP_FILE "/tmp/mcu_module_map"
#define NAME_LEN            (32)

#define GSM_MCU_HDL_CONF	"/etc/asterisk/mcuhdlsvr.conf"

// 屏蔽_REGISTER_SERI_，取消寄存器连读，因如寄存器读取失败，需补齐寄存器值为0，如果连读出错，不知该补哪一位寄存器
//#ifndef _REGISTER_SERI_			// 寄存器连续定义
//#define _REGISTER_SERI_
//#endif

#ifndef MAX_MCU
#define MAX_MCU (2)
#endif
#ifndef MAX_CHN
#define MAX_CHN (32)
#endif


// channel enable/disable
#define SET_SIM_CHN_L_ENABLE				"write 0=%02xh\n"
#define SET_SIM_CHN_H_ENABLE				"write 1=%02xh\n"
#define GET_SIM_CHN_L_STATUS				"read 0\n"
#define GET_SIM_CHN_H_STATUS				"read 1\n"
#define SET_17_24_SIM_CHN_ENABLE			"write 0=1h\n"
#define SET_17_24_SIM_CHN_DISABLE			"write 0=0h\n"
#define SET_25_32_SIM_CHN_ENABLE			"write 1=ffh\n"
#define SET_25_32_SIM_CHN_DISABLE			"write 1=0h\n"
#define GET_17_24_SIM_CHN_STATUS			"read 0\n"
#define GET_25_32_SIM_CHN_STATUS			"read 1\n"

// gsm power on/off
#define SET_17_24_GSM_POWER_ON				"write 2=0h\n"
#define SET_17_24_GSM_POWER_OFF				"write 2=1h\n"
#define SET_25_32_GSM_POWER_ON				"write 3=0h\n"
#define SET_25_32_GSM_POWER_OFF				"write 3=1h\n"

// gsm emerg off
#define SET_GSM_17_24_EMERG_OFF				"write 4=1h\n"
#define SET_GSM_25_32_EMERG_OFF				"write 5=1h\n"

// gsm start/shutdown
#define SET_GSM_17_24_START					"write 6=0h\n"
#define SET_GSM_17_24_SHUTDOWN				"write 6=1h\n"
#define SET_GSM_25_32_START					"write 7=0h\n"
#define SET_GSM_25_32_SHUTDOWN				"write 7=1h\n"

// simcard insert status
#define GET_SIMCARD_17_24_INSERT_STATUS		"read 11\n"
#define GET_SIMCARD_25_32_INSERT_STATUS		"read 10\n"

// 
#define GET_MCU_VERSION						"ver\n"
#define GET_MCU_HELP						"help\n"

//---------------------------------------------
// mcu regitster
#define REG_SIMCARD_ENABLE_L				(0)
#define REG_SIMCARD_ENABLE_H				(1)

#define REG_MODULE_POWER_ONOFF_L			(2)
#define REG_MODULE_POWER_ONOFF_H			(3)

#define REG_MODULE_EMERG_OFF_L				(4)
#define REG_MODULE_EMERG_OFF_H				(5)

#define REG_MODULE_ONOFF_L					(6)
#define REG_MODULE_ONOFF_H					(7)

#define REG_SIM_INSERT_DET_L				(10)
#define REG_SIM_INSERT_DET_H				(11)

#define REG_MOUDLE_EXIT_DET					(12)

#define REG_MODULE_POWER_ONOFF_DET_L		(13)
#define REG_MODULE_POWER_ONOFF_DET_H		(14)


//---------------------------------------------
// register value
#define VAL_SIMCARD_ENABLE			(1)
#define VAL_SIMCARD_DISABLE			(0)
#define VAL_SIMCARD_ENABLE_ALL		(0xFF)
#define VAL_SIMCARD_DISABLE_ALL		(0x00)

#define VAL_MODULE_POWER_ON			(0)
#define VAL_MODULE_POWER_OFF		(1)
#define VAL_MODULE_POWER_ON_ALL		(0x00)
#define VAL_MODULE_POWER_OFF_ALL	(0xFF)

#define VAL_MODULE_EMERG_OFF		(1)
#define VAL_MODULE_EMERG_OFF_ALL	(0xFF)

#define VAL_MODULE_ON				(1)
#define VAL_MODULE_OFF				(0)
#define VAL_MODULE_ON_ALL			(0xFF)
#define VAL_MODULE_OFF_ALL			(0x00)

#define VAL_SIMCARD_INSERT			(0)
#define VAL_SIMCARD_NOINSERT		(1)
#define VAL_SIMCARD_INSERT_ALL		(0x00)
#define VAL_SIMCARD_NOINSERT_ALL	(0xFF)

#define VAL_MODULE_INSERT			(0)
#define VAL_MODULE_NOINSERT			(1)
#define VAL_MODULE_INSERT_ALL		(0x00)
#define VAL_MODULE_NOINSERT_ALL		(0xFF)


//---------------------------------------------
#define E_LOW						(0)
#define E_HEIGH						(1)
#define E_LOW_ALL					(0x00)
#define E_HEIGH_ALL					(0xFF)



//=====================================================
typedef struct queue_handle_param_s
{
	int nbr;
	struct soap *psoap;
}queue_handle_param_t;


typedef struct regs_info_s
{
	int reg;
	int stat[MAX_CHN_PER_REG];
	int action[MAX_CHN_PER_REG];
	time_t last[MAX_CHN_PER_REG];
	pthread_mutex_t lock[MAX_CHN_PER_REG];
}regs_info_t;

// sim卡使能寄存器
regs_info_t regs_simcard_enable[] = {
	{ .reg = 0, },
	{ .reg = 1,	}
};
// 模块上电/断电寄存器
regs_info_t regs_module_vbat[] = {
	{ .reg = 2, },
	{ .reg = 3,	}
};
// 模块紧急关机寄存器
regs_info_t regs_module_emerg_off[] = {
	{ .reg = 4, },
	{ .reg = 5,	}
};
// 模块开机/关机寄存器
regs_info_t regs_module_pwrkey[] = {
	{ .reg = 6, },
	{ .reg = 7,	}
};
// sim卡插入检测寄存器
regs_info_t regs_simcard_insert_det[] = {
	{ .reg = 9, },
	{ .reg = 8,	}
};
// 模块板卡插入检测寄存器
regs_info_t regs_module_insert_det[] = {
	{ .reg = 10, }
};
// 模块开机/关机状态检测寄存器

regs_info_t regs_module_pwr_det[] = {
	{ .reg = 12, },
	{ .reg = 11, }
};


typedef struct mcu_serial_s
{
	int nbr;
	int fd;
	char ttyUSBx[64];
	pthread_mutex_t lock;
}mcu_serial_t;
mcu_serial_t mcu_serial[MAX_MCU];

typedef struct reg_bit_s
{
	int stat;				// 寄存器, 
	time_t last;			// 最后操作时间
	int laction;			// 最后操作动作
	pthread_mutex_t lock;	// 每个模块一个锁，读写时加解锁
}reg_bit_t;

typedef struct reg_s
{
	reg_bit_t reg_bit[MAX_CHN_PER_REG];
}reg_t;
reg_t regs[MAX_MCU][MAX_REG];




//=====================================================
pthread_mutex_t queue_lock;		// 队列锁
pthread_cond_t  queue_cond;		// 队列条件变量
SOAP_SOCKET queue[MAX_QUEUE];	// 数组队列
int head = 0;					// 队列头
int tail = 0;					// 队列尾

FILE *fd_log = NULL;

//static int g_stat[MAX_CHN] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};

// last request cmd within set, 0:off; 1:on
//static int g_last_cmd[MAX_CHN] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};

#if 0
static int reg_simcard_enable[] 	= {0, 1};		// sim卡使能寄存器
static int reg_module_vbat[] 		= {2, 3};		
static int reg_module_emerg_off[] 	= {4, 5};		
static int reg_module_pwrkey[] 		= {6, 7};		
static int reg_sim_insert_det[] 	= {8, 9};		
static int reg_module_exit_det[] 	= {10};			
static int reg_module_pwr_det[] 	= {11, 12};		
#endif

//
static int max_reg_mcu_simcard_enable = sizeof(regs_simcard_enable) / sizeof(regs_info_t);
static int max_chn_mcu_simcard_enable = sizeof(regs_simcard_enable) / sizeof(regs_info_t) * MAX_CHN_PER_REG;

static int max_reg_mcu_module_vbat = sizeof(regs_module_vbat) / sizeof(regs_info_t);
static int max_chn_mcu_module_vbat = sizeof(regs_module_vbat) / sizeof(regs_info_t) * MAX_CHN_PER_REG;

static int max_reg_mcu_module_pwrkey = sizeof(regs_module_pwrkey) / sizeof(regs_info_t);
static int max_chn_mcu_module_pwrkey = sizeof(regs_module_pwrkey) / sizeof(regs_info_t) * MAX_CHN_PER_REG;

static int max_reg_mcu_module_emerg_off = sizeof(regs_module_emerg_off) / sizeof(regs_info_t);
static int max_chn_mcu_module_emerg_off = sizeof(regs_module_emerg_off) / sizeof(regs_info_t) * MAX_CHN_PER_REG;

static int max_reg_mcu_simcard_insert_det = sizeof(regs_simcard_insert_det) / sizeof(regs_info_t);
static int max_chn_mcu_simcard_insert_det = sizeof(regs_simcard_insert_det) / sizeof(regs_info_t) * MAX_CHN_PER_REG;

static int max_reg_mcu_module_insert_det = sizeof(regs_module_insert_det) / sizeof(regs_info_t);
static int max_chn_mcu_module_insert_det = sizeof(regs_module_insert_det) / sizeof(regs_info_t) * MAX_CHN_PER_REG;

static int max_reg_mcu_module_pwr_det = sizeof(regs_module_pwr_det) / sizeof(regs_info_t);
static int max_chn_mcu_module_pwr_det = sizeof(regs_module_pwr_det) / sizeof(regs_info_t) * MAX_CHN_PER_REG;

/* 记录每个通道使用的模块类型 */
char g_module_types[MAX_CHN+1][NAME_LEN];

//=====================================================
void * queue_handle(void *arg);			// 线程处理函数
int enqueue(SOAP_SOCKET sock);		// 入队函数
SOAP_SOCKET dequeue(void);				// 出队函数
int set_led_control(int fd, char *buf_w, char *buf_r, int len);
int get_mcu_version(int fd, char *ver, int len);
int get_mcu_help(int fd, char *buf, int len);
int set_module_power_on(int chn);
int set_module_power_off(int chn);
int get_module_power_status(int chn, struct ns__gsm_mcu_rsp_t *result);
int set_module_on(int chn);
int set_module_off(int chn);
int get_module_onoff_status(int chn, struct ns__gsm_mcu_rsp_t *result);
int set_module_emerg_off(int chn);
int get_module_status(int chn, struct ns__gsm_mcu_rsp_t *result);
int set_simcard_status(int chn, int status);
int set_simcard_status_ex(int chn, int status);
int get_simcard_status_ex(int chn, struct ns__gsm_mcu_rsp_t *result);
int get_simcard_insert_status(int chn, struct ns__gsm_mcu_rsp_t *result);
int get_module_insert_status(int chn, struct ns__gsm_mcu_rsp_t *result);
int read_from_mcu(int board, int reg, char *result);
int read_from_mcu_seri(int board, int reg, int nbr, char *result);
int write_to_mcu(int board, int reg, int chn, int val);

int get_restore_event(int fd, char *buf, int len);
int mcu_reg_read(int fd, int reg, int num, char *buf, int len);
int mcu_reg_write(int fd, int reg, int val, char *buf, int len);

//=====================================================

// chn: 0~MAX_CHN
int get_reg_nbr(int *regs, int chn)
{
	return regs[chn / MAX_CHN_PER_REG];
}


int get_reg_value(char *buf, int reg, unsigned char *value)
{
	char buf_tmp[4] = {0};
	memcpy(buf_tmp, buf+reg*2, 2);
	*value = strtol(buf_tmp, NULL, 0x10);
	return 0;
}
int get_chn_value(char *buf, int chn, unsigned char *value)
{
	int reg = 0;
	int offset = 0;
	char buf_tmp[4] = {0};

	reg = chn / MAX_CHN_PER_REG;
	offset = chn % MAX_CHN_PER_REG;
	memcpy(buf_tmp, buf+reg*2, 2);
	*value = strtol(buf_tmp, NULL, 0x10);
	*value = (*value & (1 << offset)) >> offset;
	return 0;
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
	//zprintf("[INFO]get_mode_info: content[%s]", content);
	return 0;
}

void * queue_handle(void *arg)
{
	queue_handle_param_t *param = (queue_handle_param_t *)arg;

	zsys_info("queue_handle[%d]: Starting ...", param->nbr);

	while (1)
	{
		param->psoap->socket = dequeue();
		if (!soap_valid_socket(param->psoap->socket))
		{
			zsys_error("queue_handle[%d]: socket(%d) from queue not valid!!!", param->nbr, param->psoap->socket);
			break;
		}
		
		soap_serve(param->psoap);
		soap_destroy(param->psoap);
		soap_end(param->psoap);
	}
	zsys_info("queue_handle[%d]: End", param->nbr);
	return NULL;
}
int enqueue(SOAP_SOCKET sock)
{
	int status = SOAP_OK;
	int next = 0;
	//zsys_debug("enqueue: Starting[head:%03d-tail:%03d] ...", head, tail);
	pthread_mutex_lock(&queue_lock);
	next = tail + 1;
	if (next >= MAX_QUEUE)
	{
		next = 0;
	}
	if (next == head)
	{
		status = SOAP_EOM;
	}
	else
	{
		queue[tail] = sock;
		tail = next;
	}
	pthread_cond_signal(&queue_cond);
	pthread_mutex_unlock(&queue_lock);
	//zsys_debug("enqueue: done [head:%03d-tail:%03d]status:%d", head, tail, status);
	return status;
}
SOAP_SOCKET dequeue(void)
{
	SOAP_SOCKET sock;
	//zsys_debug("dequeue: begin[head:%03d-tail:%03d]", head, tail);
	pthread_mutex_lock(&queue_lock);
	while (head == tail)
	{
		pthread_cond_wait(&queue_cond, &queue_lock);
	}
	sock = queue[head++];
	if (head >= MAX_QUEUE)
	{
		head = 0;
	}
	pthread_mutex_unlock(&queue_lock);
	
	//zsys_debug("dequeue: done [head:%03d-tail:%03d]sock:%d", head, tail, sock);
	return sock;
}


/*
void sighdl(int signo)
{
	if (signo == SIGINT || signo == SIGTERM || signo == SIGABRT)// || signo == SIGKILL || signo == SIGSTOP)
	{
		stop = 1;
		zsys_info("sighdl: signo(%d) receive, so exit");
	}
}
*/
#if 1
int init_mcu_serial(void)
{
	char cmd[128] = {0};
	int i = 0;
	int ret = 0;
	char domain[4] = {0};
	int avalid_cnt = MAX_MCU;

	ret = access(MCU_INFO_FILE, F_OK);
	if (ret != 0)
	{
		system("/my_tools/probe_mcu");
	}

	for (i = 0; i < MAX_MCU; i++)
	{
		sprintf(domain, "%d", i+1);
		get_config(MCU_INFO_FILE, domain, "dev", mcu_serial[i].ttyUSBx);
		if (strlen(mcu_serial[i].ttyUSBx) == 0)
		{
			zsys_info("mcu_serial[%d].ttyUSBx not found", i);
			mcu_serial[i].fd = -1;
			avalid_cnt--;
			continue;
		}
		mcu_serial[i].nbr = i;
		pthread_mutex_init(&mcu_serial[i].lock, NULL);
		mcu_serial[i].fd = open_serial(mcu_serial[i].ttyUSBx, 115200, 0, 8, 1, 0);
		if (mcu_serial[i].fd < 0)
		{
			zsys_error("init_mcu_serial: open serial(%s) error(%d:%s)", mcu_serial[i].ttyUSBx, errno, strerror(errno));
			avalid_cnt--;
		}
		else
		{
			sprintf(cmd, "stty -F %s 9600", mcu_serial[i].ttyUSBx);
			system(cmd);
			zsys_info("init_mcu_serial: system(%s) succ", cmd);
			zsys_info("init_mcu_serial: open serial(%s) succ", mcu_serial[i].ttyUSBx);
		}
	}
	if (avalid_cnt == 0)
	{
		return -1;
	}
	return 0;
}
#else
void init_mcu_serial(void)
{
	char cmd[128] = {0};
	
	system("/my_tools/probe_mcu");
	
	get_config(MCU_INFO_FILE, "1", "dev", mcu_serial[0].ttyUSBx);
	mcu_serial[0].nbr = 0;
	pthread_mutex_init(&mcu_serial[0].lock, NULL);
	sprintf(cmd, "stty -F %s 9600", mcu_serial[0].ttyUSBx);
	//system(cmd);
	mcu_serial[0].fd = open_serial(mcu_serial[0].ttyUSBx, 115200, 0, 8, 1, 0);
	assert (mcu_serial[0].fd > 0);
	zsys_info("init_mcu_serial: open %s succ", mcu_serial[0].ttyUSBx);
	
	get_config(MCU_INFO_FILE, "2", "dev", mcu_serial[1].ttyUSBx);
	mcu_serial[1].nbr = 1;
	pthread_mutex_init(&mcu_serial[1].lock, NULL);
	sprintf(cmd, "stty -F %s 9600", mcu_serial[1].ttyUSBx);
	//system(cmd);
	mcu_serial[1].fd = open_serial(mcu_serial[1].ttyUSBx, 115200, 0, 8, 1, 0);
	assert (mcu_serial[1].fd > 0);
	zsys_info("init_mcu_serial: open %s succ", mcu_serial[1].ttyUSBx);
}
#endif

void close_mcu_serial(void)
{
	int i = 0;
	
	for (i = 0; i < MAX_MCU; i++)
	{
		pthread_mutex_destroy(&mcu_serial[i].lock);
		close(mcu_serial[i].fd);
	}
	zsys_info("close_mcu_serial done");
}

int init_reg(void)
{
	int mcu_nbr = 0;
	int reg_nbr = 0;
	int bit_nbr = 0;

	for (mcu_nbr = 0; mcu_nbr < MAX_MCU; mcu_nbr++)
	{
		for (reg_nbr = 0; reg_nbr < MAX_REG; reg_nbr++)
		{
			for (bit_nbr = 0; bit_nbr < MAX_CHN; bit_nbr++)
			{
				regs[mcu_nbr][reg_nbr].reg_bit[bit_nbr].stat = 0;
				regs[mcu_nbr][reg_nbr].reg_bit[bit_nbr].laction = 0;
				regs[mcu_nbr][reg_nbr].reg_bit[bit_nbr].last= 0;
				pthread_mutex_init(&regs[mcu_nbr][reg_nbr].reg_bit[bit_nbr].lock, NULL);
			}
		}
	}
	return 0;
}

int uinit_reg(void)
{
	int mcu_nbr = 0;
	int reg_nbr = 0;
	int bit_nbr = 0;

	for (mcu_nbr = 0; mcu_nbr < MAX_MCU; mcu_nbr++)
	{
		for (reg_nbr = 0; reg_nbr < MAX_REG; reg_nbr++)
		{
			for (bit_nbr = 0; bit_nbr < MAX_CHN; bit_nbr++)
			{
				regs[mcu_nbr][reg_nbr].reg_bit[bit_nbr].stat = 0;
				regs[mcu_nbr][reg_nbr].reg_bit[bit_nbr].laction = 0;
				regs[mcu_nbr][reg_nbr].reg_bit[bit_nbr].last= 0;
				pthread_mutex_destroy(&regs[mcu_nbr][reg_nbr].reg_bit[bit_nbr].lock);
			}
		}
	}
	return 0;
}

int get_module_off_last_time()
{
	int i = 0;
	int j = 0;
	int k = 0;
	int max = 0;
	
	for (i = 0; i < MAX_MCU; i++)
	{
		for (j = 0; j < max_reg_mcu_module_pwrkey; j++)
		{
			for (k = 0; k < MAX_CHN_PER_REG; k++)
			{
				max = (max > regs[i][regs_module_pwrkey[j].reg].reg_bit[k].last) ? max : regs[i][regs_module_pwrkey[j].reg].reg_bit[k].last;
			}
		}	
	}
	//zsys_debug("get_module_off_last_time: max = [%d]", max);
	return max;
}

//=====================================================


void test_mcu(void)
{
	//unsigned int result = 0;

	//char aa[4] = "FF\n";
	//zsys_debug("strtol(aa) = [0x%02x], aa = [%s]", strtol(aa, NULL, 0x10), aa);
	//int aa = 0xff;
	//aa &= ~(1 << (3-1));
	//zsys_debug("aa = [%02x]", aa);

	
	init_mcu_serial();

	// power on

#if 0
	set_simcard_status(-1, 1);
	get_simcard_status(-1, &result);
	zsys_debug("result = [%x]", result);

	set_simcard_status(-1, 0);
	get_simcard_status(-1, &result);
	zsys_debug("result = [%x]", result);

	
	set_simcard_status(5, 1);
	get_simcard_status(5, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_status(13, 1);
	get_simcard_status(13, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_status(18, 1);
	get_simcard_status(18, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_status(30, 1);
	get_simcard_status(30, &result);
	zsys_debug("result = [%x]", result);
	
	get_simcard_status(-1, &result);
	zsys_debug("result = [%x]", result);

	
	set_simcard_status(-1, 0);
	get_simcard_status(-1, &result);
	zsys_debug("result = [%x]", result);

	
	set_simcard_status(5, 1);
	get_simcard_status(5, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_status(13, 1);
	get_simcard_status(13, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_status(18, 1);
	get_simcard_status(18, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_status(30, 1);
	get_simcard_status(30, &result);
	zsys_debug("result = [%x]", result);
	
	get_simcard_status(-1, &result);
	zsys_debug("result = [%x]", result);
	
#endif

#if 0
	write_to_mcu(0, REG_MODULE_POWER_ONOFF_L, 0xff);
	write_to_mcu(0, REG_MODULE_POWER_ONOFF_H, 0xff);
	sleep(5);
	// module on
	write_to_mcu(0, REG_MODULE_ONOFF_L, 0xff);
	write_to_mcu(0, REG_MODULE_ONOFF_L, 0xff);
	
	return ;
	ret = write_to_mcu(0, 0, -1, 0xff);
	ret = write_to_mcu(0, 1, -1, 0xff);
	read_from_mcu(0, 0, 2, -1, &result);
	zsys_debug("result = [%x]", result);
	
	ret = write_to_mcu(0, 0, 2, 0);
	ret = write_to_mcu(0, 1, 6, 0);
	read_from_mcu(0, 0, 2, -1, &result);
	zsys_debug("result = [%x]", result);
	


#endif

	char buf_w[SOAP_TMPLEN] = {0};
	char buf_r[SOAP_TMPLEN] = {0};
	int len_r = 0;

	int board = 1;

	write_to_mcu(board, 6, -1, 0x00);
	write_to_mcu(board, 7, -1, 0x00);
	usleep(800*1000);
	
	read_from_mcu_seri(board, 6, 1, buf_r);
	zsys_debug("board[%d], reg = [%d], nbr = [%d], rsp = [%s]", board, 6, 1, buf_r);
	memset(buf_r, 0, sizeof(buf_r));
	read_from_mcu_seri(board, 7, 1, buf_r);
	zsys_debug("board[%d], reg = [%d], nbr = [%d], rsp = [%s]", board, 7, 1, buf_r);
	
	write_to_mcu(board, 6, -1, 0xff);
	write_to_mcu(board, 7, -1, 0xFF);
	
	
	read_from_mcu_seri(board, 6, 1, buf_r);
	zsys_debug("board[%d], reg = [%d], nbr = [%d], rsp = [%s]", board, 6, 1, buf_r);
	memset(buf_r, 0, sizeof(buf_r));
	read_from_mcu_seri(board, 7, 1, buf_r);
	zsys_debug("board[%d], reg = [%d], nbr = [%d], rsp = [%s]", board, 7, 1, buf_r);

	return;

	
	sprintf(buf_w, "write %d=%02xh\n", 6, 0);
	serial_w_mcu(mcu_serial[0].fd, (uint8 *)buf_w, strlen(buf_w), 1);
	zsys_debug("muc[%d], cmd = [%s]", 0, buf_w);

	sprintf(buf_w, "read %d\n", 6);
	serial_wr_mcu(mcu_serial[0].fd, (uint8 *)buf_w, strlen(buf_w), (uint8 *)buf_r, (uint32)len_r, 1);

	zsys_debug("muc[%d], cmd = [%s], rsp = [%s]", 0, buf_w, buf_r);

usleep(800*1000);
	
	sprintf(buf_w, "write %d=%02xh\n", 6, 0xff);
	serial_w_mcu(mcu_serial[0].fd, (uint8 *)buf_w, strlen(buf_w), 1);
	zsys_debug("muc[%d], cmd = [%s]", 0, buf_w);

	sprintf(buf_w, "read %d\n", 6);
	serial_wr_mcu(mcu_serial[0].fd, (uint8 *)buf_w, strlen(buf_w), (uint8 *)buf_r, (uint32)len_r, 1);

	zsys_debug("muc[%d], cmd = [%s], rsp = [%s]", 0, buf_w, buf_r);

	char buf[128] = {0};
	get_mcu_version(mcu_serial[0].fd, buf, sizeof(buf));
	zsys_debug("version = [%s]", buf);
	
	close_mcu_serial();
	
}
void test_simcard_enable(void)
{
	struct ns__gsm_mcu_rsp_t rsp;


	
	init_mcu_serial();


	
	rsp.value = soap_malloc(NULL, SOAP_TMPLEN);
	memset(rsp.value, 0, SOAP_TMPLEN);

	
	set_simcard_status_ex(-1, 1);
	get_simcard_status_ex(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);
	set_simcard_status_ex(-1, 0);
	get_simcard_status_ex(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);


	
	set_simcard_status_ex(-1, 1);
	get_simcard_status_ex(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);

	set_simcard_status_ex(0, 0);
	set_simcard_status_ex(7, 0);
	set_simcard_status_ex(8, 0);
	set_simcard_status_ex(15, 0);
	set_simcard_status_ex(16, 0);
	set_simcard_status_ex(23, 0);
	set_simcard_status_ex(24, 0);
	set_simcard_status_ex(31, 0);
	get_simcard_status_ex(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);
	
	
	
	set_simcard_status_ex(-1, 0);
	get_simcard_status_ex(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);

	
	set_simcard_status_ex(0, 1);
	set_simcard_status_ex(7, 1);
	set_simcard_status_ex(8, 1);
	set_simcard_status_ex(15, 1);
	set_simcard_status_ex(16, 1);
	set_simcard_status_ex(23, 1);
	set_simcard_status_ex(24, 1);
	set_simcard_status_ex(31, 1);
	get_simcard_status_ex(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);


	//
	close_mcu_serial();
}
void test_simcard_insert(void)
{
	int i = 0;
	struct ns__gsm_mcu_rsp_t rsp;

	init_mcu_serial();

	rsp.value = soap_malloc(NULL, SOAP_TMPLEN);
	memset(rsp.value, 0, SOAP_TMPLEN);

	for (i = -1; i < (MAX_MCU * max_chn_mcu_simcard_insert_det); i++)
	{
		memset(rsp.value, 0, SOAP_TMPLEN);
		get_simcard_insert_status(i, &rsp);
		zsys_debug("test_simcard_insert: chn = [%d], result: [%s]", i, rsp.value);
	}
	SOAP_FREE(NULL, rsp.value);
	
	close_mcu_serial();
}

void test_module_insert(void)
{
	int i = 0;
	struct ns__gsm_mcu_rsp_t rsp;

	init_mcu_serial();

	rsp.value = soap_malloc(NULL, SOAP_TMPLEN);
	memset(rsp.value, 0, SOAP_TMPLEN);

	for (i = -1; i < (MAX_MCU * max_chn_mcu_module_insert_det); i++)
	{
		memset(rsp.value, 0, SOAP_TMPLEN);
		get_module_insert_status(i, &rsp);
		zsys_debug("test_module_insert: chn = [%d], result: [%s]", i, rsp.value);
	}
	SOAP_FREE(NULL, rsp.value);
	
	close_mcu_serial();
}

void test_module_status(void)
{
	int i = 0;
	struct ns__gsm_mcu_rsp_t rsp;

	init_mcu_serial();

	rsp.value = soap_malloc(NULL, SOAP_TMPLEN);
	memset(rsp.value, 0, SOAP_TMPLEN);

	for (i = -1; i < (MAX_MCU * max_chn_mcu_module_pwr_det); i++)
	{
		memset(rsp.value, 0, SOAP_TMPLEN);
		get_module_status(i, &rsp);
		zsys_debug("test_module_status: chn = [%d], result: [%s]", i, rsp.value);
	}
	SOAP_FREE(NULL, rsp.value);
	
	close_mcu_serial();
}

void test_module_off(void)
{
	init_mcu_serial();

#if 1
	set_module_off(-1);
	zsys_debug("test_module_off: chn = [%d], done", -1);
#else
	for (i = 0; i < (MAX_MCU * max_chn_mcu_module_pwrkey); i++)
	{
		set_module_off(i);
		zsys_debug("test_module_status: chn = [%d]", i);
	}
#endif
	
	close_mcu_serial();
}


void test(void)
{
	struct ns__gsm_mcu_rsp_t rsp;

	init_mcu_serial();

	rsp.value = soap_malloc(NULL, SOAP_TMPLEN);

	
#if 1 // power on
zsys_debug("module power on: -------------------------------------------------------------------------");
	set_module_power_on(ALL_BITS);

	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_power_status(ALL_BITS, &rsp);
	//zsys_debug("test: get_module_power_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);

	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_status(ALL_BITS, &rsp);
	zsys_debug("test: get_module_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);
#endif


#if 1 // module on
zsys_debug("set_module_on: -------------------------------------------------------------------------");
	set_module_on(ALL_BITS);


	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_status(ALL_BITS, &rsp);
	//zsys_debug("test: get_module_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);
#endif
sleep(10);
set_module_power_off(ALL_BITS);
sleep(20);


set_module_power_off(ALL_BITS);
return;



zsys_debug("sleep(20): -------------------------------------------------------------------------");
sleep(20);



 // module off
zsys_debug("set_module_off: -------------------------------------------------------------------------");
	set_module_off(ALL_BITS);


	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_status(ALL_BITS, &rsp);
	//zsys_debug("test: get_module_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);




	

#if 0 // module emerg off
zsys_debug("test: -------------------------------------------------------------------------");
	set_module_emerg_off(ALL_BITS);

	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_status(ALL_BITS, &rsp);
	//zsys_debug("test: get_module_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);
#endif



#if 1 // power off
zsys_debug("power off: -------------------------------------------------------------------------");
	set_module_power_off(ALL_BITS);

	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_power_status(ALL_BITS, &rsp);
	//zsys_debug("test: get_module_power_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);

	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_status(ALL_BITS, &rsp);
	zsys_debug("test: get_module_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);
#endif


	SOAP_FREE(NULL, rsp.value);
	close_mcu_serial();
}


int init_log(char *logident)
{
	zconfig_t *root = NULL;
	char *buf = NULL;
	root = zconfig_load(GSM_MCU_HDL_CONF);
	if (root == NULL)
	{
		printf("init_log: load %s error\n", GSM_MCU_HDL_CONF);
		return -1;
	}
	//zsys_debug("init_log: load %s succ", GSM_MCU_HDL_CONF);

	buf = zconfig_get(root, "/log/logsystem", NULL);
	if (buf == NULL || strlen(buf) <= 0)
	{
		zsys_set_logsystem(0);
	}
	else
	{
		zsys_error("init_log: logsystem = %s", buf);
		zsys_set_logsystem(atoi(buf));
	}
	

	//
	zsys_info("init_log: logident = %s", logident);
	zsys_set_logident(logident);
	//
	buf = zconfig_get(root, "/log/logfile", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		zsys_info("init_log: logfile = [%s]", buf);
		fd_log = fopen(buf, "a+");
		if (fd_log == NULL)
		{
			zsys_error("init_log: open %s error(%d:%s)", buf, errno, strerror(errno));
		}
		else
		{
			zsys_set_logstream (fd_log);
		}
	}

	zconfig_destroy(&root);
	return 0;
}
int get_soap_port(void)
{
	zconfig_t *root = NULL;
	char *buf = NULL;
	int val = 0;

	root = zconfig_load(GSM_MCU_HDL_CONF);
	if (root == NULL)
	{
		zsys_error("get_soap_port: load %s error", GSM_MCU_HDL_CONF);
		return -1;
	}
	
	buf = zconfig_get(root, "/soap/port", NULL);
	if (buf == NULL || strlen(buf) <= 0)
	{
		zsys_error("get_soap_port: get /soap/port error");
		return -1;
	}
	val = atoi(buf);
	zsys_info("get_soap_port: port = [%d]", val);
	zconfig_destroy(&root);
	
	return val;
}

/* 获取每个通道的模块类型 */
void init_module_type(void)
{
    FILE *fp;
    int i, j;
    int ret;
    unsigned char *p;
    unsigned char buf[8192];
    unsigned char tmp[32];

    /* 默认都是m35 */
    for ( i = 1; i <= MAX_CHN; i++ )
    {
        strcpy(g_module_types[i], MODULE_NAME_M35);
    }

    fp = fopen(MODULE_MAP_FILE, "r");
    if ( NULL == fp ) 
    {
        printf("open [%s] fail\n", MODULE_MAP_FILE);
        return;
    }

    memset(buf, 0, sizeof(buf));
    ret = (int)fread(buf, 1, sizeof(buf)-1, fp);
    fclose(fp);
    if ( ret <= 0 )
    {
        printf("read file fail, ret = %d\n", ret);
        return;
    }

    /* 获取模块类型 */
    p = strstr(buf, "[module_type]");
    if ( NULL == p )
    {
        printf("get [module_type] fail!\n");
        return;
    }

    while ( *p && (*p != '\n') ) p++;
    while ( *p && ((*p == '\n') || (*p == '\r')) ) p++;
    if ( NULL == p )
    {
        printf("[module_type] end error!\n");
        return;
    }

    for ( i = 1; i <= MAX_CHN; i++ )
    {
        sprintf(tmp, "%d=", i);
        if ( strncmp(p, tmp, strlen(tmp)) )
            continue;

        p = p + strlen(tmp);
        j = 0;
        memset(g_module_types[i], 0, NAME_LEN);
        
        while ( *p && (*p != '\n') && (*p != '\r') )
            g_module_types[i][j++] = *p++;

        printf("%d: %s\n", i, g_module_types[i]);

        while ( *p && ((*p == '\n') || (*p == '\r')) ) p++;

        if ( (*p < '0') || (*p > '9') )
            break;
    }

    return;
}

#if 1
#if 1 // + czmq_poller
int main(int argc, char **argv)
{
	struct soap server_soap;
	queue_handle_param_t hdl_params[MAX_THR];
	zpoller_t *poller = NULL;
	int ret = 0;
	void *res = NULL;

	//setenv("MALLOC_TRACE", "output.server", 1);
	//mtrace();

	init_log(argv[0]);
	
	init_module_type();

	//test_mcu();
	//test();
	/*test_module_off();
	sleep(3);
	test_module_onoff_status();
	sleep(3);
	test_module_status();
	*/
	//return 0;

	/*signal(SIGINT, sighdl);
	signal(SIGTERM, sighdl);
	signal(SIGABRT, sighdl);*/
	
	soap_init(&server_soap);

	//zsys_init();
	
	ret = init_mcu_serial();
	if (ret != 0)
	{
		return -1;
	}
	
	/*if (argc < 2)
	{
		soap_serve(&server_soap);
		soap_destroy(&server_soap);
		soap_end(&server_soap);
	}
	else*/
	{
		struct soap *soap_thr[MAX_THR];
		pthread_t tid[MAX_THR];
		int i = 0;
		SOAP_SOCKET m = 0;
		SOAP_SOCKET s = 0;

		pthread_mutex_init(&queue_lock, NULL);
		pthread_cond_init(&queue_cond, NULL);

		m = soap_bind(&server_soap, NULL, get_soap_port(), BACKLOG);
		if (!soap_valid_socket(m))
		{
			soap_print_fault(&server_soap, stderr);
			exit(-1);
		}
		zsys_info("Socket connection successful: master socket = %d", m);

		for (i = 0; i < MAX_THR; i++)
		{
			soap_thr[i] = soap_copy(&server_soap);
			hdl_params[i].nbr = i;
			hdl_params[i].psoap = soap_thr[i];
			pthread_create(&tid[i], NULL, queue_handle, (void *)&hdl_params[i]);
		}

		//
		poller = zpoller_new(NULL);
		assert (poller);
		ret = zpoller_add(poller, (void *)&server_soap.master);
		assert (ret == 0);

		
		while (1)
		{
			res = zpoller_wait(poller, 5000);
			if (res != &server_soap.master)
			{
				continue;
			}
			
			s = soap_accept(&server_soap);
			if (!soap_valid_socket(s))
			{
				if (server_soap.errnum)
				{
					soap_print_fault(&server_soap, stderr);
					continue;
				}
				else
				{
					zsys_error("Server time out!!!");
					break;
				}
			}
			/*zsys_info("Accept connection from IP = %d.%d.%d.%d, socket = %d", \
				((server_soap.ip)>>24)&0xFF, ((server_soap.ip)>>16)&0xFF, ((server_soap.ip)>>8)&0xFF, \
				(server_soap.ip)&0xFF, server_soap.socket);*/

			while (enqueue(s) == SOAP_EOM)
			{
				sleep(1);
			}
		}
		zpoller_remove(poller, (void *)server_soap.master);
		zpoller_destroy (&poller);

		for (i = 0; i < MAX_THR; i++)
		{
			zsys_info("Waiting for thread %d to terminate ...", i);
			pthread_join(tid[i], NULL);
			zsys_info("Thread %d terminate", i);
			soap_done(soap_thr[i]);
			soap_free(soap_thr[i]);
		}
		pthread_mutex_destroy(&queue_lock);
		pthread_cond_destroy(&queue_cond);
	}
	soap_done(&server_soap);
	close_mcu_serial();
	zsys_shutdown();
	fclose(fd_log);
	return 0;
}
#else
int main(int argc, char **argv)
{
	struct soap server_soap;
	queue_handle_param_t hdl_params[MAX_THR];

	zsys_init();
	
	soap_init(&server_soap);
	if (argc < 2)
	{
		soap_serve(&server_soap);
		soap_destroy(&server_soap);
		soap_end(&server_soap);
	}
	else
	{
		struct soap *soap_thr[MAX_THR];
		pthread_t tid[MAX_THR];
		int i = 0;
		SOAP_SOCKET m = 0;
		SOAP_SOCKET s = 0;

		pthread_mutex_init(&queue_lock, NULL);
		pthread_cond_init(&queue_cond, NULL);

		m = soap_bind(&server_soap, NULL, atoi(argv[1]), BACKLOG);
		if (!soap_valid_socket(m))
		{
			soap_print_fault(&server_soap, stderr);
			exit(-1);
		}
		//fprintf(stderr, "Socket connection successful: master socket = %d\n", m);
		zsys_info("Socket connection successful: master socket = %d", m);

		for (i = 0; i < MAX_THR; i++)
		{
			soap_thr[i] = soap_copy(&server_soap);
			hdl_params[i].nbr = i;
			hdl_params[i].psoap = soap_thr[i];
			pthread_create(&tid[i], NULL, queue_handle, (void *)&hdl_params[i]);
		}

		while (1)
		{
			s = soap_accept(&server_soap);
			if (!soap_valid_socket(s))
			{
				if (server_soap.errnum)
				{
					soap_print_fault(&server_soap, stderr);
					continue;
				}
				else
				{
					zsys_error("Server time out!!!");
					break;
				}
			}
			zsys_info("Accept connection from IP = %d.%d.%d.%d, socket = %d", \
				((server_soap.ip)>>24)&0xFF, ((server_soap.ip)>>16)&0xFF, ((server_soap.ip)>>8)&0xFF, \
				(server_soap.ip)&0xFF, server_soap.socket);

			while (enqueue(s) == SOAP_EOM)
			{
				sleep(1);
			}
		}

		for (i = 0; i < MAX_THR; i++)
		{
			zsys_info("Waiting for thread %d to terminate ...", i);
			pthread_join(tid[i], NULL);
			zsys_info("Thread %d terminate", i);
			soap_done(soap_thr[i]);
			soap_free(soap_thr[i]);
		}
		pthread_mutex_destroy(&queue_lock);
		pthread_cond_destroy(&queue_cond);
	}
	soap_done(&server_soap);

	zsys_shutdown();
	return 0;
}
#endif
#else

int main(int argc, char **argv)
{ SOAP_SOCKET m, s; /* master and slave sockets */
  struct soap soap;
  soap_init1(&soap, SOAP_IO_CHUNK | SOAP_IO_KEEPALIVE);
  if (argc < 2)
    soap_serve(&soap);	/* serve as CGI application */
  else
  { m = soap_bind(&soap, NULL, atoi(argv[1]), 100);
    if (!soap_valid_socket(m))
    { soap_print_fault(&soap, stderr);
      exit(-1);
    }
    fprintf(stderr, "Socket connection successful: master socket = %d\n", m);
    for ( ; ; )
    { s = soap_accept(&soap);
      fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);
      if (!soap_valid_socket(s))
      { soap_print_fault(&soap, stderr);
        exit(-1);
      } 
      soap_serve(&soap);
      soap_end(&soap);
    }
  }
  return 0;
} 
#endif



//gsoap ns service method: gsm simcard enable
int ns__gsm_simcard_enable(struct soap *soap, int chn, int *result)
{(void)soap;
	
	if (chn < -1 || chn >= MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsm_simcard_enable: args invalid");
		return SOAP_OK;
	}

	//*result = set_simcard_status(chn, 1);
	*result = set_simcard_status_ex(chn, 1);
		
	zsys_debug("ns__gsm_simcard_enable[sock:%d][chn:%d]: result = 0x%x", soap->socket, chn, *result);
	return *result;
}

//gsoap ns service method: gsm simcard disable
int ns__gsm_simcard_disable(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsm_simcard_disable: args invalid");
		return SOAP_OK;
	}

	//*result = set_simcard_status(chn, 0);
	*result = set_simcard_status_ex(chn, 0);
	
	zsys_debug("ns__gsm_simcard_disable[sock:%d][chn:%d]: result = 0x%x", soap->socket, chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: get gsm simcard status
int ns__gsm_get_simcard_status(struct soap *soap, int chn, struct ns__gsm_mcu_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		result->result= SOAP_ERR;
		zsys_error("ns__gsm_get_simcard_status: args invalid");
		return SOAP_OK;
	}

	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsm_get_simcard_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}

	get_simcard_status_ex(chn, result);
	zsys_debug("ns__gsm_get_simcard_status[sock:%d][chn:%d]: result->result = %d, result->value[%s]", soap->socket, chn, result->result, result->value);
	return SOAP_OK;
}

//gsoap ns service method: gsm module power on
int ns__gsm_module_power_on(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsm_module_power_on: args invalid");
		return SOAP_OK;
	}
	*result = set_module_power_on(chn);
	zsys_debug("ns__gsm_module_power_on: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: gsm module power off
int ns__gsm_module_power_off(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsm_module_power_off: args invalid");
		return SOAP_OK;
	}
	*result = set_module_power_off(chn);
	zsys_debug("ns__gsm_module_power_off: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}

// gsoap ns service method: get gsm module power status
int ns__gsm_get_module_power_status(struct soap *soap, int chn, struct ns__gsm_mcu_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		result->result = SOAP_ERR;
		zsys_error("ns__gsm_get_module_power_status: args invalid");
		return SOAP_OK;
	}
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsm_get_simcard_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	get_module_power_status(chn, result);
	zsys_debug("ns__gsm_get_module_power_status[sock:%d][chn:%d]: result->result = %d, result->value[%s]", soap->socket, chn, result->result, result->value);
	return SOAP_OK;
}

//gsoap ns service method: gsm module emerg off
int ns__gsm_module_emerg_off(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsm_module_emerg_off: args invalid");
		return SOAP_OK;
	}
	*result = set_module_emerg_off(chn);
	zsys_debug("ns__gsm_module_emerg_off: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: get gsm module emerg off status
/*int ns__gsm_get_module_emerg_off_status(struct soap *soap, int chn, unsigned int *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsm_get_module_emerg_off_status: args invalid");
		return SOAP_OK;
	}
	else
		*result = 0;
	zsys_debug("ns__gsm_get_module_emerg_off_status: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}*/

//gsoap ns service method: gsm module on
int ns__gsm_module_on(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsm_module_on: args invalid");
		return SOAP_OK;
	}
	*result = set_module_on(chn);
	zsys_debug("ns__gsm_module_on: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: gsm module off
int ns__gsm_module_off(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsm_module_off: args invalid");
		return SOAP_OK;
	}
	*result = set_module_off(chn);
	zsys_debug("ns__gsm_module_off: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: get gsm module status
int ns__gsm_get_module_status(struct soap *soap, int chn, struct ns__gsm_mcu_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		result->result = SOAP_ERR;
		zsys_error("ns__gsm_get_module_status: args invalid");
		return SOAP_OK;
	}
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsm_get_simcard_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	get_module_status(chn, result);
	zsys_debug("ns__gsm_get_module_status: chn = %d, result->result = %d, result->value = [%s]", chn, result->result, result->value);
	return SOAP_OK;
}

//gsoap ns service method: get gsm simcard insert status
int ns__gsm_get_simcard_insert_status(struct soap *soap, int chn, struct ns__gsm_mcu_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		result->result = SOAP_ERR;
		zsys_error("ns__gsm_get_simcard_insert_status: args invalid");
		return SOAP_OK;
	}
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsm_get_simcard_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	get_simcard_insert_status(chn, result);
	zsys_debug("ns__gsm_get_simcard_insert_status: chn = %d, result->result = %d, result->value = [%s]", chn, result->result, result->value);
	return SOAP_OK;
}

// gsoap ns service method: get gsm gsmboard insert status
int ns__gsm_get_gsmboard_insert_status(struct soap *soap, int chn, struct ns__gsm_mcu_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN/2)
	{
		result->result = SOAP_ERR;
		zsys_error("ns__gsm_get_gsmboard_insert_status: args invalid");
		return SOAP_OK;
	}
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsm_get_simcard_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	get_module_insert_status(chn, result);
	zsys_debug("ns__gsm_get_gsmboard_insert_status: chn = %d, result->result = %d, result->value = [%s]", chn, result->result, result->value);
	return SOAP_OK;
}
//gsoap ns service method: get mcu version
/*int ns__gsm_get_mcu_version(struct soap *soap, struct ns__gsm_mcu_rsp_t *result)
{(void)soap;
	int ret = 0;
	
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsm_get_mcu_version: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	pthread_mutex_lock(&mcu_serial[0].lock);
	ret = get_mcu_version(mcu_serial[0].fd, result->value, SOAP_TMPLEN);
	pthread_mutex_unlock(&mcu_serial[0].lock);
	if (ret != 0)
	{
		result->result = SOAP_ERR;
		zsys_debug("ns__gsm_get_mcu_version:  get_mcu_version error");
		return SOAP_OK;
	}
	
	pthread_mutex_lock(&mcu_serial[1].lock);
	ret = get_mcu_version(mcu_serial[1].fd, result->value+strlen(result->value), SOAP_TMPLEN-strlen(result->value));
	pthread_mutex_unlock(&mcu_serial[1].lock);
	if (ret != 0)
	{
		result->result = SOAP_ERR;
		zsys_debug("ns__gsm_get_mcu_version:  get_mcu_version error");
		return SOAP_OK;
	}

	result->result = SOAP_OK;
	
	zsys_debug("ns__gsm_get_mcu_version:  result[result:%d][len:%d][value:%s]", result->result, strlen(result->value), result->value);
	return SOAP_OK;
}*/
int ns__gsm_get_mcu_version(struct soap *soap, char **result)
{(void)soap;
	char ver[128] = {0};
	pthread_mutex_lock(&mcu_serial[0].lock);
	get_mcu_version(mcu_serial[0].fd, ver, sizeof(ver));
	pthread_mutex_unlock(&mcu_serial[0].lock);
	
	pthread_mutex_lock(&mcu_serial[1].lock);
	get_mcu_version(mcu_serial[1].fd, ver+strlen(ver), sizeof(ver)-strlen(ver));
	pthread_mutex_unlock(&mcu_serial[1].lock);

	*result = (char*)soap_malloc(soap, strlen(ver)+1);
	memset(*result, 0, strlen(ver)+1);
	memcpy(*result, ver, strlen(ver));
	
	zsys_debug("ns__gsm_get_mcu_version:  result[len:%d] = [%s]", strlen(*result), *result);
	return SOAP_OK;
}

//gsoap ns service method: get mcu help
/*int ns__gsm_get_mcu_help(struct soap *soap, struct ns__gsm_mcu_rsp_t *result)
{(void)soap;
	int ret = 0;
	
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsm_get_mcu_help: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	pthread_mutex_lock(&mcu_serial[0].lock);
	ret = get_mcu_help(mcu_serial[0].fd, result->value, SOAP_TMPLEN);
	pthread_mutex_unlock(&mcu_serial[0].lock);
	if (ret != 0)
	{
		result->result = SOAP_ERR;
		zsys_debug("ns__gsm_get_mcu_help:  get_mcu_help error");
		return SOAP_OK;
	}

	result->result = SOAP_OK;
	
	zsys_debug("ns__gsm_get_mcu_help:  result[result:%d][len:%d][value:%s]",  result->result, strlen(result->value), result->value);
	return SOAP_OK;
}*/
int ns__gsm_get_mcu_help(struct soap *soap, char **result)
{(void)soap;
	char help[128] = {0};
	pthread_mutex_lock(&mcu_serial[0].lock);
	get_mcu_help(mcu_serial[0].fd, help, sizeof(help));
	pthread_mutex_unlock(&mcu_serial[0].lock);

	*result = (char*)soap_malloc(soap, strlen(help)+1);
	memset(*result, 0, strlen(help)+1);
	memcpy(*result, help, strlen(help));
	
	zsys_debug("ns__gsm_get_mcu_help:  result[len:%d] = [%s]", strlen(*result), *result);
	return SOAP_OK;
}

int ns__gsm_set_led_control(struct soap *soap, char *buf, char **result)
{
	(void)soap;
	char out[128] = {0};
	pthread_mutex_lock(&mcu_serial[0].lock);
	set_led_control(mcu_serial[0].fd, buf, out, sizeof(out));
	pthread_mutex_unlock(&mcu_serial[0].lock);

	pthread_mutex_lock(&mcu_serial[1].lock);
	set_led_control(mcu_serial[1].fd, buf, out, sizeof(out));
	pthread_mutex_unlock(&mcu_serial[1].lock);

	*result = (char*)soap_malloc(soap, strlen(out)+1);
	memset(*result, 0, strlen(out)+1);
	memcpy(*result, out, strlen(out));

	zsys_debug("ns__gsm_set_led_control:  result[%d] = [%s]", strlen(*result), *result);
	return SOAP_OK;
}

int ns__gsm_get_restore_event(struct soap *soap, char **result)
{
    (void)soap;
	char event[128] = {0};
	pthread_mutex_lock(&mcu_serial[0].lock);
	get_restore_event(mcu_serial[0].fd, event, sizeof(event));
	pthread_mutex_unlock(&mcu_serial[0].lock);

	*result = (char*)soap_malloc(soap, strlen(event)+1);
	memset(*result, 0, strlen(event)+1);
	memcpy(*result, event, strlen(event));

	zsys_debug("ns__gsm_get_restore_event:  result[len:%d] = [%s]", strlen(*result), *result);
	return SOAP_OK;
}

int ns__gsm_mcu_reg_read(struct soap *soap, int brd, int reg, int num, char **result)
{
    (void)soap;
	char buf[128] = "error";

    if ( (brd >= MAX_MCU) || (brd < 0) )
    {
        *result = (char*)soap_malloc(soap, strlen(buf)+1);
    	memset(*result, 0, strlen(buf)+1);
        memcpy(*result, buf, strlen(buf));
        return SOAP_OK;
    }

	pthread_mutex_lock(&mcu_serial[brd].lock);
	mcu_reg_read(mcu_serial[brd].fd, reg, num, buf, sizeof(buf));
	pthread_mutex_unlock(&mcu_serial[brd].lock);

	*result = (char*)soap_malloc(soap, strlen(buf)+1);
	memset(*result, 0, strlen(buf)+1);
	memcpy(*result, buf, strlen(buf));

	zsys_debug("ns__gsm_mcu_reg_read:  result[len:%d] = [%s]", strlen(*result), *result);
	return SOAP_OK;
}

int ns__gsm_mcu_reg_write(struct soap *soap, int brd, int reg, unsigned char val, char **result)
{
    (void)soap;
	char buf[128] = "error";

    if ( (brd >= MAX_MCU) || (brd < 0) )
    {
        *result = (char*)soap_malloc(soap, strlen(buf)+1);
    	memset(*result, 0, strlen(buf)+1);
        memcpy(*result, buf, strlen(buf));
        return SOAP_OK;
    }

	pthread_mutex_lock(&mcu_serial[brd].lock);
	mcu_reg_write(mcu_serial[brd].fd, reg, val, buf, sizeof(buf));
	pthread_mutex_unlock(&mcu_serial[brd].lock);

	*result = (char*)soap_malloc(soap, strlen(buf)+1);
	memset(*result, 0, strlen(buf)+1);
	memcpy(*result, buf, strlen(buf));

	zsys_debug("ns__gsm_mcu_reg_write:  result[len:%d] = [%s]", strlen(*result), *result);
	return SOAP_OK;
}

#if 1
int get_mcu_version(int fd, char *buf, int len)
{
	int ret = 0;

	ret = serial_wr_mcu(fd, (uint8 *)GET_MCU_VERSION, (uint32)strlen(GET_MCU_VERSION), (uint8 *)buf, (uint32)len, SERIAL_READ_TIMEOUT);
	if (ret < 0)
		return -1;
	return 0;
}
#endif

#if 1
int get_mcu_help(int fd, char *buf, int len)
{
	int ret = 0;

	ret = serial_wr_mcu(fd, (uint8 *)GET_MCU_HELP, (uint32)strlen(GET_MCU_HELP), (uint8 *)buf, (uint32)len, SERIAL_READ_TIMEOUT);
	if (ret < 0)
		return -1;
	return 0;
}
#endif

int set_led_control(int fd, char *buf_w, char *buf_r, int len)
{
	int ret = 0;
	ret = serial_wr_mcu(fd, (uint8 *)buf_w, (uint32)strlen(buf_w), (uint8 *)buf_r, (uint32)len, SERIAL_READ_TIMEOUT);
	if (ret < 0)
		return -1;
	return 0;
}



#if 1
// status: 1:enbale;0:disable
int set_simcard_status_ex(int chn, int status)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int value = 0;
	int reg = -1;
	
	if (chn == -1)
	{
		value = (status == 0) ? VAL_SIMCARD_DISABLE_ALL : VAL_SIMCARD_ENABLE_ALL;

		// lock regs
		//zsys_debug("set_simcard_status: lock begin ...");
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_simcard_enable; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_simcard_enable[j].reg].reg_bit[m].lock);
					//zsys_debug("set_simcard_status_ex: pthread_mutex_lock(&regs[%d][regs_simcard_enable[%d].reg].reg_bit[%d].lock)", i, j, m);
				}
				// 设置该寄存器
				ret = write_to_mcu(i, regs_simcard_enable[j].reg, ALL_BITS, value);
				usleep(20*1000);
				// 更新该寄存器所有通道状态值, 并解锁
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_simcard_enable[j].reg].reg_bit[m].stat = status;
					pthread_mutex_unlock(&regs[i][regs_simcard_enable[j].reg].reg_bit[m].lock);
					//zsys_debug("set_simcard_status_ex: pthread_mutex_unlock(&regs[%d][regs_simcard_enable[%d].reg].reg_bit[%d].lock)", i, j, m);
				}
			}
		}
	}
	else
	{
		value = (status == 0) ? VAL_SIMCARD_DISABLE : VAL_SIMCARD_ENABLE;
		// 获取该通道所属mcu
		board = chn / max_chn_mcu_simcard_enable;
		_chn = chn % max_chn_mcu_simcard_enable;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;
		
		pthread_mutex_lock(&regs[board][regs_simcard_enable[reg].reg].reg_bit[_chn].lock);
		//zsys_debug("set_simcard_status_ex: pthread_mutex_lock(&regs[%d][regs_simcard_enable[%d].reg].reg_bit[%d].lock)", board, reg, _chn);
		ret = write_to_mcu(board, regs_simcard_enable[reg].reg, _chn, value);
		regs[board][regs_simcard_enable[reg].reg].reg_bit[_chn].stat = status;
		pthread_mutex_unlock(&regs[board][regs_simcard_enable[reg].reg].reg_bit[_chn].lock);
		//zsys_debug("set_simcard_status_ex: pthread_mutex_unlock(&regs[%d][regs_simcard_enable[%d].reg].reg_bit[%d].lock)", board, reg, _chn);
		if (ret != 0)
		{
			zsys_error("set_simcard_status: write_to_mcu error(%d:%s)", errno, strerror(errno));
			return -1;
		}
	}
	//zsys_debug("set_simcard_status: set chn(%d) with status(%x) succ", chn, status);
	return 0;
}


int get_simcard_status_ex(int chn, struct ns__gsm_mcu_rsp_t *result)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int reg = 0;
	unsigned char value = 0;
	int ret = 0;
	result->result = 0;
	result->cnt = 0;
	
	if (chn == -1)
	{
		// lock regs
		for (i = 0; i < MAX_MCU; i++)
		{
		#ifdef _REGISTER_SERI_ // each time read all 
			// 加锁所有通道
			for (m = 0; m < max_reg_mcu_simcard_enable; m++)
			{
				reg = regs_simcard_enable[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
			}
			// 读取所有寄存器
			ret = read_from_mcu_seri(i, regs_simcard_enable[0].reg, max_reg_mcu_simcard_enable, result->value+i*max_reg_mcu_simcard_enable*sizeof(char)*2);
			if (ret == 0)
			{
				usleep(20*1000);
			}
			// 解锁所有通道
			for (m = 0; m < max_reg_mcu_simcard_enable; m++)
			{
				reg = regs_simcard_enable[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
			}
		#else // each time read one register
			for (m = 0; m < max_reg_mcu_simcard_enable; m++)
			{
				// 获取寄存器地址
				reg = regs_simcard_enable[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
				// 读取寄存器值
				ret = read_from_mcu_seri(i, reg, 1, result->value+i*max_reg_mcu_simcard_enable*2 + m*sizeof(char)*2);
				if (ret != 0)
				{
					strcpy(result->value+i*max_reg_mcu_simcard_enable*2 + m*sizeof(char)*2, "00");
					if (result->result == HDL_MCU_ERR || result->result == HDL_OK)
					{ // update if last is OK or MCU_ERR, else(last is ERR), don't change
						result->result = ret;
					}
				}
				usleep(20*1000);
				// 解锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
				//zsys_debug("get_simcard_status_ex: board[%d], reg[%d], result->cnt = [%d], result->value = [%s]", i, reg, result->cnt, result->value);
				
				result->cnt++;
			}
		#endif
		}
		/*if (strlen(result->value) < 2) // 已补齐查询失败的寄存器位值
		{
			result->result = HDL_ERR;
			
		}*/
	}
	else
	{
		board = chn / max_chn_mcu_simcard_enable;
		_chn = chn % max_chn_mcu_simcard_enable;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		pthread_mutex_lock(&regs[board][regs_simcard_enable[reg].reg].reg_bit[_chn].lock);
		ret = read_from_mcu_seri(board, regs_simcard_enable[reg].reg, 1, result->value);
		if (ret != 0 || strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
		}
		else
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << _chn)) >> _chn;
			value = (value == VAL_SIMCARD_ENABLE) ? 1 : 0;
			sprintf(result->value, "%x", value);
			result->result = HDL_OK;
			result->cnt = 1;
			regs[board][regs_simcard_enable[reg].reg].reg_bit[_chn].stat = value;
		}
		pthread_mutex_unlock(&regs[board][regs_simcard_enable[reg].reg].reg_bit[_chn].lock);
	}
	//zsys_debug("get_simcard_status_ex: chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
}
#endif


#if 1
int set_module_power_on(int chn)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int reg = -1;
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_power_on: lock begin ...");
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_vbat; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_module_vbat[j].reg].reg_bit[m].lock);
					//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", i, j, m);
				}
				// 设置该寄存器
				ret = write_to_mcu(i, regs_module_vbat[j].reg, ALL_BITS, VAL_MODULE_POWER_ON_ALL);

				// wait 30ms
				usleep(30*1000);

				// 更新该寄存器所有通道状态值, 并解锁
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_module_vbat[j].reg].reg_bit[m].stat = 1;
					pthread_mutex_unlock(&regs[i][regs_module_vbat[j].reg].reg_bit[m].lock);
					//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", i, j, m);
				}
			}
		}
	}
	else
	{
		// 获取该通道所属mcu
		board = chn / max_chn_mcu_module_vbat;
		_chn = chn % max_chn_mcu_module_vbat;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;
		
		pthread_mutex_lock(&regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].lock);
		//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, _chn);
		ret = write_to_mcu(board, regs_module_vbat[reg].reg, _chn, VAL_MODULE_POWER_ON_ALL);
		regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].stat = 1;
		
		// wait 30ms
		usleep(30*1000);

		pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].lock);
		//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, _chn);
		if (ret != 0)
		{
			zsys_error("set_module_power_on: write_to_mcu error(%d:%s)", errno, strerror(errno));
			return -1;
		}
	}
    // 上电操作完成后休眠500ms
    usleep(500 * 1000);
	//zsys_debug("set_module_power_on: set chn(%d) with status(%x) succ", chn, status);
	return 0;
}

int set_module_power_off(int chn)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int reg = -1;
	int time_sleep = 0;

	// 模块关机，记录并获取并计算等待时间，休眠
	// 然后进行断电操作

	// 如果已处于关机状态则不会重复关机
	//set_module_off(chn);

	//time_sleep = time(NULL) - get_module_off_last_time();
	//time_sleep = (time_sleep < 0) ? 0 : time_sleep;
	//time_sleep = (time_sleep > 12) ? 0 : (12 - time_sleep);
	
	// 等待12s
	//sleep(12);
	//sleep(time_sleep);
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_power_on: lock begin ...");
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_vbat; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_module_vbat[j].reg].reg_bit[m].lock);
					//zsys_debug("set_module_power_off: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", i, j, m);
				}
				// 设置该寄存器
				ret = write_to_mcu(i, regs_module_vbat[j].reg, ALL_BITS, VAL_MODULE_POWER_OFF_ALL);

				// wait 30ms, 休眠30ms后该释放该寄存器，用户可以操作它
				usleep(30*1000);

				// 更新该寄存器所有通道状态值, 并解锁
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_module_vbat[j].reg].reg_bit[m].stat = 0;
					pthread_mutex_unlock(&regs[i][regs_module_vbat[j].reg].reg_bit[m].lock);
					//zsys_debug("set_module_power_off: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", i, j, m);
				}
			}
		}
	}
	else
	{
		// 获取该通道所属mcu
		board = chn / max_chn_mcu_module_vbat;
		_chn = chn % max_chn_mcu_module_vbat;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;
		
		pthread_mutex_lock(&regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].lock);
		//zsys_debug("set_module_power_off: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, _chn);
		ret = write_to_mcu(board, regs_module_vbat[reg].reg, _chn, VAL_MODULE_POWER_OFF);
		regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].stat = 0;
		
		// wait 30ms
		usleep(30*1000);

		pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].lock);
		//zsys_debug("set_module_power_off: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, _chn);
		if (ret != 0)
		{
			zsys_error("set_module_power_off: write_to_mcu error(%d:%s)", errno, strerror(errno));
			return -1;
		}
	}
    // 掉电操作完成后休眠500ms
    usleep(500 * 1000);
	//zsys_debug("set_module_power_off: set chn(%d) with status(%x) succ", chn, status);
	return 0;
}

int get_module_power_status(int chn, struct ns__gsm_mcu_rsp_t *result)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int reg = 0;
	unsigned char value = 0;
	int ret = 0;
	char buf[4] = {0};

	result->result = 0;
	result->cnt = 0;
	
	if (chn == -1)
	{
		// lock regs
		for (i = 0; i < MAX_MCU; i++)
		{
		#ifdef _REGISTER_SERI_ // each time read all 
			// 加锁所有通道
			for (m = 0; m < max_reg_mcu_module_vbat; m++)
			{
				reg = regs_module_vbat[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
			}
			// 读取所有寄存器
			ret = read_from_mcu_seri(i, regs_module_vbat[0].reg, max_reg_mcu_module_vbat, result->value+i*max_reg_mcu_module_vbat*sizeof(char)*2);
			if (ret ==0)
			{
				usleep(20*1000);
			}
			// 解锁所有通道
			for (m = 0; m < max_reg_mcu_module_vbat; m++)
			{
				reg = regs_module_vbat[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
			}
		#else // each time read one register
			for (m = 0; m < max_reg_mcu_module_vbat; m++)
			{
				// 获取寄存器地址
				reg = regs_module_vbat[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
				// 读取寄存器值
				ret = read_from_mcu_seri(i, reg, 1, result->value+i*max_reg_mcu_module_vbat*2 + m*sizeof(char)*2);
				if (ret != 0)
				{
					strcpy(result->value+i*max_reg_mcu_module_vbat*2 + m*sizeof(char)*2, "FF");
					if (result->result == HDL_MCU_ERR || result->result == HDL_OK)
					{ // update if last is OK or MCU_ERR, else(last is ERR), don't change
						result->result = ret;
					}
				}
				usleep(20*1000);
				// 解锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
				//zsys_debug("get_module_power_status: board[%d], reg[%d], result->cnt = [%d], result->value[%d] = [%x]", i, reg, result->cnt, (i*max_reg_mcu_module_vbat+m), result->value[(i*max_reg_mcu_module_vbat+m)]);
				
				result->cnt++;
			}
		#endif
		}
		//zsys_debug("get_module_power_status: 1 chn = [%d], result->value = [%s]", chn, result->value);
		/*if (strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
			return HDL_ERR;
		}*/
		// 取反寄存器值
		for (i = 0; i < MAX_MCU; i++)
		{
			for (m = 0; m < max_reg_mcu_module_vbat; m++)
			{
				memcpy(buf, result->value+i*max_reg_mcu_module_vbat*2+m*2, 2);
				value = strtol(buf, NULL, 0x10);
				value = ~value;
				sprintf(buf, "%02x", value);
				memcpy(result->value+i*max_reg_mcu_module_vbat*2 + m*2, buf, 2);
			}
		}
	}
	else
	{
		board = chn / max_chn_mcu_module_vbat;
		_chn = chn % max_chn_mcu_module_vbat;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		pthread_mutex_lock(&regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].lock);
		ret = read_from_mcu_seri(board, regs_module_vbat[reg].reg, 1, result->value);
		if (ret != 0 || strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
		}
		else
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << _chn)) >> _chn;
			value = (value == VAL_MODULE_POWER_ON) ? 1 : 0;
			sprintf(result->value, "%x", value);
			result->result = HDL_OK;
			result->cnt = 1;
			regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].stat = value;
		}
		pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[_chn].lock);
	}
	//zsys_debug("get_module_power_status: chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
}


// set module power status
// status: 0:off; 1:on
/*
int set_module_power_status(int chn, int status)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int ret = 0;
	int value = 0;
	int reg = -1;
	
	if (chn == -1)
	{
		// 查询模块开关机状态
		// 如果有开机，则执行关机操作
		// 等待12s
		// 执行上下电操作
		value = (status == 0) ? VAL_MODULE_POWER_OFF_ALL : VAL_MODULE_POWER_ON_ALL;

		
		// lock regs
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < MAX_CHN_PER_EMU; j++)
			{
				pthread_mutex_lock(&regs[i][REG_MODULE_POWER_ONOFF_L].reg_bit[j].lock);
				regs[i][REG_MODULE_POWER_ONOFF_L].reg_bit[j].stat = status;
				pthread_mutex_lock(&regs[i][REG_MODULE_POWER_ONOFF_H].reg_bit[j].lock);
				regs[i][REG_MODULE_POWER_ONOFF_H].reg_bit[j].stat = status;
			}
		}

		if (status == 0) // power on
		{
			sleep(12);
		}
		
		// write to mcu
		for (i = 0; i < MAX_MCU; i++)
		{
			ret = write_to_mcu(i, REG_MODULE_POWER_ONOFF_L, ALL_BITS, value);
			ret = write_to_mcu(i, REG_MODULE_POWER_ONOFF_H, ALL_BITS, value);
		}

		if (status == 1) // power on
		{
			usleep(30*1000);
		}
		
		// unlock regs
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < MAX_CHN_PER_EMU; j++)
			{
				pthread_mutex_unlock(&regs[i][REG_MODULE_POWER_ONOFF_L].reg_bit[j].lock);
				pthread_mutex_unlock(&regs[i][REG_MODULE_POWER_ONOFF_H].reg_bit[j].lock);
			}
		}
	}
	else
	{
		board = (chn >= MAX_CHN_PER_BOARD) ? 1 : 0;
		_chn  = (chn >= MAX_CHN_PER_BOARD) ? (chn - MAX_CHN_PER_BOARD) : chn;
		reg   = (_chn >= MAX_CHN_PER_EMU) ? REG_MODULE_POWER_ONOFF_H : REG_MODULE_POWER_ONOFF_L;
		_chn  = (_chn >= MAX_CHN_PER_EMU) ? (_chn - MAX_CHN_PER_EMU) : _chn;
		value = (status == 0) ? VAL_MODULE_POWER_OFF : VAL_MODULE_POWER_ON;
		
		pthread_mutex_lock(&regs[board][reg].reg_bit[_chn].lock);
		regs[board][reg].reg_bit[_chn].stat = status;
		ret = write_to_mcu(board, reg, _chn, value);
		if (status == 1) // power on
		{
			usleep(30*1000);
		}
		pthread_mutex_unlock(&regs[board][reg].reg_bit[_chn].lock);
	}
	//zsys_debug("set_module_power_status: set chn(%d) with status(%x) succ", chn, status);
	return 0;
}
*/
#endif

#if 1

int set_module_onoff(int chn, int status)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int reg = -1;
	char res[SOAP_TMPLEN] = {0};
	char buf[4] = {0};
	unsigned char bits = 0;


	// 查询模块开关机状态
	struct ns__gsm_mcu_rsp_t rsp;
	rsp.value = malloc(SOAP_TMPLEN);
	if (rsp.value == NULL)
	{
		return -1;
	}
	memset(rsp.value, 0, SOAP_TMPLEN);
	ret = get_module_status(chn, &rsp);
	if (ret != 0)
	{
		return -1;
	}
	zsys_info("set_module_onoff: curr status[%s]", rsp.value);

	memcpy(res, rsp.value, strlen(rsp.value));
	// 
	free(rsp.value);
	rsp.value = NULL;
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_onoff: lock begin ...");
		// 加锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_pwrkey; j++)
			{
				memcpy(buf, res+i*max_reg_mcu_module_pwrkey*2+j*2, 2);
				bits = strtol(buf, NULL, 0x10);
				
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					if ((((bits & (1 << m)) >> m) == 1 && status == 0) || (((bits & (1 << m)) >> m) == 0 && status == 1))
					{
						pthread_mutex_lock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
						ret = write_to_mcu(i, regs_module_pwrkey[j].reg, m, E_LOW_ALL);
					}
				}
			}
		}
		
		// 等待。开机：2s; 关机：0.6~1s
		usleep((status == 1) ? 2*1000*1000 : 800*1000);

		// 拉高所哟寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_pwrkey; j++)
			{
				memcpy(buf, res+i*max_reg_mcu_module_pwrkey*2+j*2, 2);
				bits = strtol(buf, NULL, 0x10);
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					if ((((bits & (1 << m)) >> m) == 1 && status == 0) || (((bits & (1 << m)) >> m) == 0 && status == 1))
					{
						ret = write_to_mcu(i, regs_module_pwrkey[j].reg, m, E_HEIGH_ALL);
						regs[i][regs_module_pwrkey[j].reg].reg_bit[m].stat = status;
						// 解锁所有寄存器通道
						pthread_mutex_unlock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
					}
				}
			}
		}
	}
	else
	{
		// 获取该通道所属mcu
		board = chn / max_chn_mcu_module_pwrkey;
		_chn  = chn % max_chn_mcu_module_pwrkey;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		//memcpy(buf, res+board*max_reg_mcu_module_pwrkey*2+reg*2, 2);
		memcpy(buf, res, 2);
		bits = strtol(buf, NULL, 0x10);
		if ((bits == 1 && status == 0) || (bits == 0 && status == 1))
		{
			// 加锁
			pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
			// 拉低
			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_LOW_ALL);
		
			// 等待。开机：2s; 关机：0.6~1s
			usleep((status == 1) ? 2*1000*1000 : 600*1000);
			
			// 拉高
			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_HEIGH_ALL);
			// 更新状态
			regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].stat = 1;
			// 解锁
			pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
			if (ret != 0)
			{
				zsys_error("set_module_onoff: write_to_mcu error(%d:%s)", errno, strerror(errno));
				return -1;
			}
		}
	}
	
	// 关机操作完成后休眠500ms
	usleep((status == 1) ? 0 : 500*1000);
	
	//zsys_debug("set_module_onoff: set chn(%d) with status(%x) succ", chn, status);
	return 0;
}

#if 1
// 先判断通道开关机状态，如果状态为开机，则不做操作，如果为关机，则执行开机操作
// 开机操作：拉低2s以上再拉高
int set_module_on(int chn)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
    int k = 0;
	int m = 0;
	int ret = 0;
	int reg = -1;
	char res[SOAP_TMPLEN] = {0};
	char buf[4] = {0};
	unsigned char bits = 0;
	int sleep_time = 0;
	time_t last = 0;
	time_t curr = 0;


	// 查询模块开关机状态
	struct ns__gsm_mcu_rsp_t rsp;
	rsp.value = malloc(SOAP_TMPLEN);
	if (rsp.value == NULL)
	{
		return -1;
	}
	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_status(chn, &rsp);
	zsys_info("set_module_on: curr status[%s]", rsp.value);

	memcpy(res, rsp.value, strlen(rsp.value));
	// 
	//free(rsp.value);
	//rsp.value = NULL;
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_on: lock begin ...");
		// 加锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_pwrkey; j++)
			{
				memcpy(buf, res+i*max_reg_mcu_module_pwrkey*2+j*2, 2);
				bits = strtol(buf, NULL, 0x10);
				if (bits == 0xff) // 寄存器全部通道处于开机状态
				{
					continue;
				}
				sleep_time = 2;
				
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
				}
				// 拉低寄存器,原来是开机的通道则不拉低，其值保持1，其余为0，拉低
				ret = write_to_mcu(i, regs_module_pwrkey[j].reg, ALL_BITS, bits);
				usleep(20*1000);
			}

            /* 如果是sim6320c模块提前拉高 */
            for ( k = i * 16 + 1; k <= i * 16 + 16; k++ )
            {
                if ( strcmp(g_module_types[k], MODULE_NAME_SIM6320C) == 0 )
                {
                    // 获取该通道所属mcu
            		board = (k-1) / max_chn_mcu_module_pwrkey;
            		_chn  = (k-1) % max_chn_mcu_module_pwrkey;
            		reg   = _chn / MAX_CHN_PER_REG;
            		_chn  = _chn % MAX_CHN_PER_REG;

                    // 拉高
        			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_HEIGH);
                }
            }
		}
		
		// 等待2s
		sleep(sleep_time);

		// 拉高所有寄存器，原来没有拉低的通道不受影响。这样，就只对关机状态的通道做拉低拉高操作。
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_pwrkey; j++)
			{
				memcpy(buf, res+i*max_reg_mcu_module_pwrkey*2+j*2, 2);
				bits = strtol(buf, NULL, 0x10);
				if (bits == 0xff)
				{
					continue;
				}
				
				// 拉高
				ret = write_to_mcu(i, regs_module_pwrkey[j].reg, ALL_BITS, E_HEIGH_ALL);
				usleep(20*1000);

				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_module_pwrkey[j].reg].reg_bit[m].stat = 1;
					// 解锁所有寄存器通道
					pthread_mutex_unlock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
				}
			}
		}
	}
	else
	{
		// 获取该通道所属mcu
		board = chn / max_chn_mcu_module_pwrkey;
		_chn  = chn % max_chn_mcu_module_pwrkey;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		//memcpy(buf, res+board*max_reg_mcu_module_pwrkey*2+reg*2, 2);
		memcpy(buf, res, 2);
		bits = strtol(buf, NULL, 0x10);
		if (bits == 0)
		{
			// 加锁
			pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
			// 拉低
			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_LOW);

            if ( (chn <= MAX_CHN) && (strcmp(g_module_types[chn+1], MODULE_NAME_SIM6320C) == 0) )
                usleep(30 * 1000); /* sim6320c等待30ms */
            else
    			sleep(2); // m35等待2s
			
			// 拉高
			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_HEIGH);
			// 更新状态
			regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].stat = 1;
			// 解锁
			pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
			if (ret != 0)
			{
				zsys_error("set_module_on: write_to_mcu error(%d:%s)", errno, strerror(errno));
				free(rsp.value);
				return -1;
			}
			// get status
			time(&last);
			time(&curr);
			while ((curr - last) < 10)
			{
				memset(rsp.value, 0, SOAP_TMPLEN);
				ret = get_module_status(chn, &rsp);
				if (!(ret == HDL_OK || ret == HDL_MCU_ERR))
				{
					usleep(500000);
					time(&curr);
					continue;
				}
				ret = -1;
				if (strstr(rsp.value, "1") != NULL)
				{
					ret = 0;
					break;
				}
				usleep(500000);
				time(&curr);
			}
		}
	}
	
	
	free(rsp.value);
	//zsys_debug("set_module_on: set chn(%d) succ", chn);
	return ret;
}

// 先判断通道开关机状态，如果状态为开机，则不做操作，如果为关机，则执行开机操作
// 开机操作：拉低0.6~1s再拉高
int set_module_off(int chn)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
    int k = 0;
	int m = 0;
	int ret = 0;
	int reg = -1;
	char res[SOAP_TMPLEN] = {0};
	char buf[4] = {0};
	unsigned char bits = 0;
	int sleep_time = 0;
	time_t last = 0;
	time_t curr = 0;


	// 查询模块开关机状态
	struct ns__gsm_mcu_rsp_t rsp;
	rsp.value = malloc(SOAP_TMPLEN);
	if (rsp.value == NULL)
	{
		return -1;
	}
	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_status(chn, &rsp);
	zsys_info("set_module_off: curr status[%s]", rsp.value);

	memcpy(res, rsp.value, strlen(rsp.value));
	// 
	//free(rsp.value);
	//rsp.value = NULL;
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_off: lock begin ...");
		// 加锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_pwrkey; j++)
			{
				memcpy(buf, res+i*max_reg_mcu_module_pwrkey*2+j*2, 2);
				bits = strtol(buf, NULL, 0x10);
				if (bits == 0x00)
				{
					continue;
				}
				sleep_time = 800*1000;
				
				bits = ~bits; // 取反，只对开机状态的通道进行拉低操作
				
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
				}
				// 拉低寄存器,原来开机的通道拉低
				ret = write_to_mcu(i, regs_module_pwrkey[j].reg, ALL_BITS, bits);
				usleep(20*1000);
			}
		}
		
		// 等待0.6~1s
		usleep(sleep_time);

        /* 如果是m35模块拉高 */
        for ( k = 1; k <= MAX_CHN; k++ )
        {
            if ( strcmp(g_module_types[k], MODULE_NAME_M35) == 0 )
            {
                // 获取该通道所属mcu
        		board = (k-1) / max_chn_mcu_module_pwrkey;
        		_chn  = (k-1) % max_chn_mcu_module_pwrkey;
        		reg   = _chn / MAX_CHN_PER_REG;
        		_chn  = _chn % MAX_CHN_PER_REG;

                // 拉高
    			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_HEIGH);
                usleep(20*1000);
            }
        }

        usleep(400*1000);

        /* 如果是sim6320c模块拉高 */
        for ( k = 1; k <= MAX_CHN; k++ )
        {
            if ( strcmp(g_module_types[k], MODULE_NAME_SIM6320C) == 0 )
            {
                // 获取该通道所属mcu
        		board = (k-1) / max_chn_mcu_module_pwrkey;
        		_chn  = (k-1) % max_chn_mcu_module_pwrkey;
        		reg   = _chn / MAX_CHN_PER_REG;
        		_chn  = _chn % MAX_CHN_PER_REG;

                // 拉高
    			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_HEIGH);
                usleep(20*1000);
            }
        }

		// 拉高所有寄存器，原来没有拉低的通道不受影响。这样，就只对关机状态的通道做拉低拉高操作。
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_pwrkey; j++)
			{
				memcpy(buf, res+i*max_reg_mcu_module_pwrkey*2+j*2, 2);
				bits = strtol(buf, NULL, 0x10);
				if (bits == 0x00)
				{
					continue;
				}

				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_module_pwrkey[j].reg].reg_bit[m].stat = 0;
					if (((bits & 1 << m) >> m) == 1)
					{
						time(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].last);
						//zsys_debug("------------------------------------> regs[%d][%d].reg_bit[%d]last = [%d]", i, regs_module_pwrkey[j].reg, m, regs[i][regs_module_pwrkey[j].reg].reg_bit[m].last);
					}
					// 解锁所有寄存器通道
					pthread_mutex_unlock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
				}
			}
		}
	}
	else
	{
		// 获取该通道所属mcu
		board = chn / max_chn_mcu_module_pwrkey;
		_chn  = chn % max_chn_mcu_module_pwrkey;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		//memcpy(buf, res+board*max_reg_mcu_module_pwrkey*2+reg*2, 2);
		memcpy(buf, res, 2);
		bits = strtol(buf, NULL, 0x10);
		if (bits == 1)
		{
			// 加锁
			pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
			// 拉低
			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_LOW);
		
			// 等待2s
			if ( (chn <= MAX_CHN) && (strcmp(g_module_types[chn+1], MODULE_NAME_SIM6320C) == 0) )
                sleep_time = 1200 * 1000;
            else
    			sleep_time = 800*1000;
                
			usleep(sleep_time);
			
			// 拉高
			ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_HEIGH);
			// 更新状态
			regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].stat = 1;
			time(&(regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].last));
			// 解锁
			pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
			if (ret != 0)
			{
				zsys_error("set_module_off: write_to_mcu error(%d:%s)", errno, strerror(errno));
				free(rsp.value);
				return -1;
			}
			// get status
			time(&last);
			time(&curr);
			while ((curr - last) < 10)
			{
				memset(rsp.value, 0, SOAP_TMPLEN);
				ret = get_module_status(chn, &rsp);
				if (!(ret == HDL_OK || ret == HDL_MCU_ERR))
				{
					usleep(500000);
					time(&curr);
					continue;
				}
				ret = -1;
				if (strstr(rsp.value, "0") != NULL)
				{
					ret = 0;
					break;
				}
				usleep(500000);
				time(&curr);
			}
		}
	}
	
	// 关机操作完成后休眠500ms
	//usleep((sleep_time == 0) ? 0 : 500*1000);
	free(rsp.value);
	//zsys_debug("set_module_off: set chn(%d) succ", chn);
	
	return ret;
}
#else
// 先判断通道开关机状态，如果状态为开机，则不做操作，如果为关机，则执行开机操作
// 开机操作：拉低2s以上再拉高
int set_module_on(int chn)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int reg = -1;
	int max_chn_mcu = 0;
	int max_reg_mcu = 0;

	max_reg_mcu = sizeof(regs_module_pwrkey) / sizeof(regs_info_t);
	max_chn_mcu = max_reg_mcu * MAX_CHN_PER_REG;

	
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_on: lock begin ...");
		// 加锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
				}
			}
		}
		// 拉低所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu; j++)
			{
				// 设置该寄存器
				ret = write_to_mcu(i, regs_module_pwrkey[j].reg, ALL_BITS, E_LOW_ALL);
			}
		}
		// 等待2s
		sleep(2);

		// 拉高所哟寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu; j++)
			{
				// 设置该寄存器
				ret = write_to_mcu(i, regs_module_pwrkey[j].reg, ALL_BITS, E_HEIGH_ALL);
			}
		}
		// 解锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_module_pwrkey[j].reg].reg_bit[m].stat = 1;
					pthread_mutex_unlock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
				}
			}
		}
	}
	else
	{
		// 获取该通道所属mcu
		board = chn / max_chn_mcu;
		_chn = chn % max_chn_mcu;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		// 加锁
		pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
		// 拉低
		ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_LOW_ALL);
		// 等待2s
		sleep(2);
		// 拉高
		ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_HEIGH_ALL);
		// 更新状态
		regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].stat = 1;
		// 解锁
		pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
		if (ret != 0)
		{
			zsys_error("set_module_on: write_to_mcu error(%d:%s)", errno, strerror(errno));
			return -1;
		}
	}
	//zsys_debug("set_module_on: set chn(%d) with status(%x) succ", chn, status);
	return 0;
}

// 先判断通道开关机状态，如果状态为开机，则不做操作，如果为关机，则执行开机操作
// 开机操作：拉低0.6~1s再拉高
int set_module_off(int chn)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int reg = -1;
	int max_chn_mcu = 0;
	int max_reg_mcu = 0;

	max_reg_mcu = sizeof(regs_module_pwrkey) / sizeof(regs_info_t);
	max_chn_mcu = max_reg_mcu * MAX_CHN_PER_REG;
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_off: lock begin ...");
		// 加锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
				}
			}
		}
		// 拉低所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu; j++)
			{
				// 设置该寄存器
				ret = write_to_mcu(i, regs_module_pwrkey[j].reg, ALL_BITS, E_LOW_ALL);
			}
		}
		// 等待0.6~1s
		usleep(800*1000);

		// 拉高所哟寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu; j++)
			{
				// 设置该寄存器
				ret = write_to_mcu(i, regs_module_pwrkey[j].reg, ALL_BITS, E_HEIGH_ALL);
			}
		}
		// 解锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_module_pwrkey[j].reg].reg_bit[m].stat = 1;
					pthread_mutex_unlock(&regs[i][regs_module_pwrkey[j].reg].reg_bit[m].lock);
				}
			}
		}
	}
	else
	{
		// 获取该通道所属mcu
		board = chn / max_chn_mcu;
		_chn = chn % max_chn_mcu;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		// 加锁
		pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
		// 拉低
		ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_LOW_ALL);
		// 等待0.6~1s
		usleep(800*1000);
		// 拉高
		ret = write_to_mcu(board, regs_module_pwrkey[reg].reg, _chn, E_HEIGH_ALL);
		// 更新状态
		regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].stat = 1;
		// 解锁
		pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
		if (ret != 0)
		{
			zsys_error("set_module_off: write_to_mcu error(%d:%s)", errno, strerror(errno));
			return -1;
		}
	}
	
	// 操作完成后休眠500ms
	usleep(500*1000);
	
	//zsys_debug("set_module_off: set chn(%d) with status(%x) succ", chn, status);
	return 0;
}
#endif
#if 1
int get_module_onoff_status(int chn, struct ns__gsm_mcu_rsp_t *result) 
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int reg = 0;
	unsigned char value = 0;
	int ret = 0;

	result->result = HDL_OK;
	result->cnt = 0;
	
	if (chn == -1)
	{
		// lock regs
		for (i = 0; i < MAX_MCU; i++)
		{
		#ifdef _REGISTER_SERI_ // each time read all 
			// 加锁所有通道
			for (m = 0; m < max_reg_mcu_module_pwrkey; m++)
			{
				reg = regs_module_pwrkey[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
			}
			// 读取所有寄存器
			ret = read_from_mcu_seri(i, regs_module_pwrkey[0].reg, max_reg_mcu_module_pwrkey, result->value+i*max_reg_mcu_module_pwrkey*sizeof(char)*2);
			if (ret == 0)
			{
				usleep(20*1000);
			}
			// 解锁所有通道
			for (m = 0; m < max_reg_mcu_module_pwrkey; m++)
			{
				reg = regs_module_pwrkey[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
			}
		#else // each time read one register
			for (m = 0; m < max_reg_mcu_module_pwrkey; m++)
			{
				// 获取寄存器地址
				reg = regs_module_pwrkey[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
				// 读取寄存器值
				ret = read_from_mcu_seri(i, reg, 1, result->value+i*max_reg_mcu_module_pwrkey*2 + m*sizeof(char)*2);
				usleep(20*1000);
				// 解锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
				//zsys_debug("get_module_power_status: board[%d], reg[%d], result->cnt = [%d], result->value[%d] = [%x]", i, reg, result->cnt, (i*max_reg_mcu_module_pwrkey+m), result->value[(i*max_reg_mcu_module_pwrkey+m)]);
				
				result->cnt++;
			}
		#endif
		}
		if (strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
		}
	}
	else
	{
		board = chn / max_chn_mcu_module_pwrkey;
		_chn = chn % max_chn_mcu_module_pwrkey;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
		ret = read_from_mcu_seri(board, regs_module_pwrkey[reg].reg, 1, result->value);
		if (ret != 0 || strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
		}
		else
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << _chn)) >> _chn;
			value = (value == VAL_MODULE_ON) ? 1 : 0;
			sprintf(result->value, "%x", value);
			result->cnt = 1;
			result->result = HDL_OK;
			regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].stat = value;
		}
		pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[_chn].lock);
	}
	//zsys_debug("get_module_power_status: chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
}
#endif
#endif

#if 1
int set_module_emerg_off(int chn)
{
	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int reg = -1;
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_emerg_off: lock begin ...");
		// 加锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_emerg_off; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_module_emerg_off[j].reg].reg_bit[m].lock);
				}
			}
		}
		// 拉低所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_emerg_off; j++)
			{
				// 设置该寄存器
				ret = write_to_mcu(i, regs_module_emerg_off[j].reg, ALL_BITS, E_HEIGH_ALL);
				usleep(20*1000);
			}
		}
		
		// 等待20ms以上
		usleep(20*1000);

		// 拉高所哟寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_emerg_off; j++)
			{
				// 设置该寄存器
				ret = write_to_mcu(i, regs_module_emerg_off[j].reg, ALL_BITS, E_LOW_ALL);
				usleep(20*1000);
			}
		}
		// 解锁所有寄存器通道
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_module_emerg_off; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_module_emerg_off[j].reg].reg_bit[m].stat = 1;
					pthread_mutex_unlock(&regs[i][regs_module_emerg_off[j].reg].reg_bit[m].lock);
				}
			}
		}
	}
	else
	{
		// 获取该通道所属mcu
		board = chn / max_chn_mcu_module_emerg_off;
		_chn = chn % max_chn_mcu_module_emerg_off;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		// 加锁
		pthread_mutex_lock(&regs[board][regs_module_emerg_off[reg].reg].reg_bit[_chn].lock);
		// 拉低
		ret = write_to_mcu(board, regs_module_emerg_off[reg].reg, _chn, E_HEIGH);
		// 等待20ms以上
		usleep(20*1000);
		// 拉高
		ret = write_to_mcu(board, regs_module_emerg_off[reg].reg, _chn, E_LOW);
		// 更新状态
		regs[board][regs_module_emerg_off[reg].reg].reg_bit[_chn].stat = 1;
		// 解锁
		pthread_mutex_unlock(&regs[board][regs_module_emerg_off[reg].reg].reg_bit[_chn].lock);
		if (ret != 0)
		{
			zsys_error("set_module_emerg_off: write_to_mcu error(%d:%s)", errno, strerror(errno));
			return -1;
		}
	}
	
	// 紧急关机操作完成后休眠2s
	sleep(2);
	
	//zsys_debug("set_module_emerg_off: set chn(%d) succ", chn);
	return 0;
}
#endif


#if 1
int get_simcard_insert_status(int chn, struct ns__gsm_mcu_rsp_t *result)
{

	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int reg = 0;
	unsigned char value = 0;
	int ret = 0;
	char buf[4] = {0};

	result->result = 0;
	result->cnt = 0;
	
	if (chn == -1)
	{
		// lock regs
		for (i = 0; i < MAX_MCU; i++)
		{
			for (m = 0; m < max_reg_mcu_simcard_insert_det; m++)
			{
				// 获取寄存器地址
				reg = regs_simcard_insert_det[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
				// 读取寄存器值
				ret = read_from_mcu_seri(i, reg, 1, result->value+i*max_reg_mcu_simcard_insert_det*2 + m*sizeof(char)*2);
				if (ret != 0)
				{
					strcpy(result->value+i*max_reg_mcu_simcard_insert_det*2 + m*sizeof(char)*2, "FF");
					if (result->result == HDL_MCU_ERR || result->result == HDL_OK)
					{ // update if last is OK or MCU_ERR, else(last is ERR), don't change
						result->result = ret;
					}
				}
				usleep(20*1000);
				// 解锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
				//zsys_debug("get_simcard_insert_status: board[%d], reg[%d], result->cnt = [%d], result->value[%d] = [%x]", i, reg, result->cnt, (i*max_reg_mcu_simcard_insert_det+m), result->value[(i*max_reg_mcu_simcard_insert_det+m)]);
				result->cnt++;
			}
		}
		//zsys_debug("get_simcard_insert_status: 1 chn = [%d], result->value = [%s]", chn, result->value);
		/*if (strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
			return HDL_ERR;
		}*/
		// 取反寄存器值
		for (i = 0; i < MAX_MCU; i++)
		{
			for (m = 0; m < max_reg_mcu_simcard_insert_det; m++)
			{
				memcpy(buf, result->value+i*max_reg_mcu_simcard_insert_det*2+m*2, 2);
				value = strtol(buf, NULL, 0x10);
				value = ~value;
				sprintf(buf, "%02x", value);
				memcpy(result->value+i*max_reg_mcu_simcard_insert_det*2 + m*2, buf, 2);
			}
		}
	}
	else
	{
		board = chn / max_chn_mcu_simcard_insert_det;
		_chn = chn % max_chn_mcu_simcard_insert_det;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		pthread_mutex_lock(&regs[board][regs_simcard_insert_det[reg].reg].reg_bit[_chn].lock);
		ret = read_from_mcu_seri(board, regs_simcard_insert_det[reg].reg, 1, result->value);
		if (ret != 0 || strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
		}
		else
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << _chn)) >> _chn;
			value = (value == VAL_SIMCARD_INSERT) ? 1 : 0;
			sprintf(result->value, "%x", value);
			result->result = HDL_OK;
			result->cnt = 1;
			regs[board][regs_simcard_insert_det[reg].reg].reg_bit[_chn].stat = value;
		}

		
		//zsys_debug("get_simcard_insert_status: 1 chn = [%d], board = [%d], reg = [%d], _chn = [%d], result->value = [%s]", chn, board, regs_simcard_insert_det[reg].reg, _chn, result->value);

		pthread_mutex_unlock(&regs[board][regs_simcard_insert_det[reg].reg].reg_bit[_chn].lock);
	}
	//zsys_debug("get_simcard_insert_status: 2 chn = [%d], result->value = [%s]", chn, result->value);
	
	return 0;
}
#endif

#if 1
int get_module_insert_status(int chn, struct ns__gsm_mcu_rsp_t *result)
{

	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int reg = 0;
	unsigned char value = 0;
	int ret = 0;
	char buf[4] = {0};

	result->result = 0;
	result->cnt = 0;
	
	if (chn == -1)
	{
		// lock regs
		for (i = 0; i < MAX_MCU; i++)
		{
			for (m = 0; m < max_reg_mcu_module_insert_det; m++)
			{
				// 获取寄存器地址
				reg = regs_module_insert_det[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
				// 读取寄存器值
				ret = read_from_mcu_seri(i, reg, 1, result->value+i*max_reg_mcu_module_insert_det*2 + m*sizeof(char)*2);
				if (ret != 0)
				{
					strcpy(result->value+i*max_reg_mcu_module_insert_det*2 + m*sizeof(char)*2, "FF");
					if (result->result == HDL_MCU_ERR || result->result == HDL_OK)
					{ // update if last is OK or MCU_ERR, else(last is ERR), don't change
						result->result = ret;
					}
				}
				usleep(20*1000);
				// 解锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
				//zsys_debug("get_module_insert_status: board[%d], reg[%d], result->cnt = [%d], result->value[%d] = [%x]", i, reg, result->cnt, (i*max_reg_mcu_module_insert_det+m), result->value[(i*max_reg_mcu_module_insert_det+m)]);
				
				result->cnt++;
			}
		}
		//zsys_debug("get_module_insert_status: 1 chn = [%d], result->value = [%s]", chn, result->value);
		if (strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
			return HDL_ERR;
		}
		// 取反寄存器值
		for (i = 0; i < MAX_MCU; i++)
		{
			for (m = 0; m < max_reg_mcu_module_insert_det; m++)
			{
				memcpy(buf, result->value+i*max_reg_mcu_module_insert_det*2+m*2, 2);
				value = strtol(buf, NULL, 0x10);
				value = ~value;
				sprintf(buf, "%02x", value);
				memcpy(result->value+i*max_reg_mcu_module_insert_det*2 + m*2, buf, 2);
			}
		}
	}
	else
	{
		board = chn / max_chn_mcu_module_insert_det;
		_chn = chn % max_chn_mcu_module_insert_det;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		pthread_mutex_lock(&regs[board][regs_module_insert_det[reg].reg].reg_bit[_chn].lock);
		ret = read_from_mcu_seri(board, regs_module_insert_det[reg].reg, 1, result->value);
		if (ret != 0 || strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
		}
		else
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << _chn)) >> _chn;
			value = (value == VAL_MODULE_INSERT) ? 1 : 0;
			sprintf(result->value, "%x", value);
			result->result = HDL_OK;
			result->cnt = 1;
			regs[board][regs_module_insert_det[reg].reg].reg_bit[_chn].stat = value;
		}
		pthread_mutex_unlock(&regs[board][regs_module_insert_det[reg].reg].reg_bit[_chn].lock);
	}
	//zsys_debug("get_module_insert_status: 2 chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
}
#endif


#if 1
int get_module_status(int chn, struct ns__gsm_mcu_rsp_t *result)
{

	int _chn = 0;
	int board = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int reg = 0;
	unsigned char value = 0;
	int ret = 0;

	result->result = 0;
	result->cnt = 0;
	
	if (chn == -1)
	{
		// lock regs
		for (i = 0; i < MAX_MCU; i++)
		{
			for (m = 0; m < max_reg_mcu_module_pwr_det; m++)
			{
				// 获取寄存器地址
				reg = regs_module_pwr_det[m].reg;
				// 加锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_lock(&regs[i][reg].reg_bit[j].lock);
				}
				// 读取寄存器值
				ret = read_from_mcu_seri(i, reg, 1, result->value+i*max_reg_mcu_module_pwr_det*2 + m*sizeof(char)*2);
				if (ret != 0)
				{
					strcpy(result->value+i*max_reg_mcu_module_pwr_det*2 + m*sizeof(char)*2, "00");
					if (result->result == HDL_MCU_ERR || result->result == HDL_OK)
					{ // update if last is OK or MCU_ERR, else(last is ERR), don't change
						result->result = ret;
					}
				}
				usleep(20*1000);
				// 解锁
				for (j = 0; j < MAX_CHN_PER_REG; j++)
				{
					pthread_mutex_unlock(&regs[i][reg].reg_bit[j].lock);
				}
				//zsys_debug("get_module_status: board[%d], reg[%d], result->cnt = [%d], result->value[%d] = [%x]", i, reg, result->cnt, (i*max_reg_mcu_module_pwr_det+m), result->value[(i*max_reg_mcu_module_pwr_det+m)]);
				
				result->cnt++;
			}
		}
		/*if (strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
		}*/
		//zsys_debug("get_module_status: 1 chn = [%d], result->value = [%s]", chn, result->value);
	}
	else
	{
		board = chn / max_chn_mcu_module_pwr_det;
		_chn = chn % max_chn_mcu_module_pwr_det;
		reg   = _chn / MAX_CHN_PER_REG;
		_chn  = _chn % MAX_CHN_PER_REG;

		pthread_mutex_lock(&regs[board][regs_module_pwr_det[reg].reg].reg_bit[_chn].lock);
		ret = read_from_mcu_seri(board, regs_module_pwr_det[reg].reg, 1, result->value);
		if (ret != 0 || strlen(result->value) < 2)
		{
			result->result = HDL_ERR;
		}
		else
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << _chn)) >> _chn;
			value = (value == VAL_MODULE_ON) ? 1 : 0;
			sprintf(result->value, "%x", value);
			result->result = HDL_OK;
			result->cnt = 1;
			regs[board][regs_module_pwr_det[reg].reg].reg_bit[_chn].stat = value;
		}
		pthread_mutex_unlock(&regs[board][regs_module_pwr_det[reg].reg].reg_bit[_chn].lock);
	}
	//zsys_debug("get_module_status: 2 chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
}
#endif


int get_restore_event(int fd, char *buf, int len)
{
    int ret = 0;
    char buf_w[128] = "read 61\n";

	ret = serial_wr_mcu(fd, (uint8 *)buf_w, (uint32)strlen(buf_w), (uint8 *)buf, (uint32)len, SERIAL_READ_TIMEOUT);
	if (ret < 0)
		return -1;

	return 0;
}

int mcu_reg_read(int fd, int reg, int num, char *buf, int len)
{
    int ret = 0;
    char buf_w[128] = {0};

    sprintf(buf_w, "read %d-0%01xh\n", reg, num);

	ret = serial_wr_mcu(fd, (uint8 *)buf_w, (uint32)strlen(buf_w), (uint8 *)buf, (uint32)len, SERIAL_READ_TIMEOUT);
	if (ret < 0)
		return -1;
	return 0;
}

int mcu_reg_write(int fd, int reg, int val, char *buf, int len)
{
    int ret = 0;
    char buf_w[128] = {0};

    (void)len;
    strcpy(buf, "ok\n");

    sprintf(buf_w, "write %d=%02xh\n", reg, val);

	ret = serial_w_mcu(fd, (uint8 *)buf_w, (uint32)strlen(buf_w), SERIAL_READ_TIMEOUT);
	if (ret < 0)
		return -1;
	return 0;
}


// bit: -1:hold register value
// nbr: 寄存器连续数量,1,2,...
// 一次可以读取连续多个寄存器
// 不可以读取寄存器的位值
int read_from_mcu(int board, int reg, char *result)
{
	int ret = 0;
	//unsigned int result = 0;
	unsigned char buf_req[MAX_REQ_LEN] = {0};
	unsigned char buf_rsp[MAX_RSP_LEN] = {0};
	
	if (mcu_serial[board].fd < 0)
	{
		return -2;
	}

	sprintf((char *)buf_req, "read %d\n", reg);
	zsys_info("read_from_mcu: [PC ==> MCU] [%s]", buf_req);
	
	pthread_mutex_lock(&mcu_serial[board].lock);
	ret = serial_wr_mcu(mcu_serial[board].fd, buf_req, strlen((char *)buf_req), buf_rsp, (uint32)sizeof(buf_rsp), SERIAL_READ_TIMEOUT);
	pthread_mutex_unlock(&mcu_serial[board].lock);
	zsys_info("read_from_mcu: [PC <== MCU] [%s]", buf_rsp);
	
	if (ret <= 0)
	{
		zsys_error("read_from_mcu: serial_wr_mcu error(%d:%s)", errno, strerror(errno));
		return -1;
	}
	memcpy(result, buf_rsp, sizeof(char)*2);
	zsys_error("read_from_mcu: result = [%s]", result);
	
	return 0;
}
// nbr:1,2,3,...
int read_from_mcu_seri(int board, int reg, int nbr, char *result)
{
	int ret = 0;
	int i = 0;
	//unsigned int result = 0;
	unsigned char buf_req[MAX_REQ_LEN] = {0};
	unsigned char buf_rsp[MAX_RSP_LEN] = {0};

	if (mcu_serial[board].fd < 0)
	{
		zsys_error("read_from_mcu_seri: serial(board:%d)'s fd invalid", board);
		return -2;
	}

	if (nbr > 1)
		sprintf((char *)buf_req, "read %d-%02dh\n", reg, nbr);
	else
		sprintf((char *)buf_req, "read %d\n", reg);
	zsys_info("read_from_mcu_seri: [PC ==> MCU][board:%d] [%s]", board, buf_req);
	
	pthread_mutex_lock(&mcu_serial[board].lock);
	ret = serial_wr_mcu(mcu_serial[board].fd, buf_req, strlen((char *)buf_req), buf_rsp, (uint32)sizeof(buf_rsp), SERIAL_READ_TIMEOUT);
	pthread_mutex_unlock(&mcu_serial[board].lock);
	zsys_info("read_from_mcu_seri: [PC <== MCU][board:%d] [%s]", board, buf_rsp);
	if (ret <= 0)
	{
		zsys_error("read_from_mcu_seri: serial_wr_mcu error(%d:%s)", errno, strerror(errno));
		return -1;
	}
	if (strlen(buf_rsp) < 2)
	{
		return -1;
	}

	for (i = 0; i < nbr; i++)
	{
		memcpy(result+ i*2, buf_rsp+i*3, 2);
	}
	//zsys_error("read_from_mcu_seri:board[%d], reg[%d], result = [%s]", board, reg, result);
	
	return 0;
}
// 一次不可以写连续多个寄存器
// 可以写寄存器的指定位
int write_to_mcu(int board, int reg, int bit, int val)
{
	int ret = 0;
	unsigned char buf_req[MAX_REQ_LEN] = {0};
	
	if (mcu_serial[board].fd < 0)
	{
		return -2;
	}

	if (bit == -1)
		sprintf((char *)buf_req, "write %d=%02xh\n", reg, val);
	else
		sprintf((char *)buf_req, "write %d.%d=%d\n", reg, bit, val);
	zsys_info("write_to_mcu: [PC ==> MCU][board:%d] [%s]", board, buf_req);
		
	pthread_mutex_lock(&mcu_serial[board].lock);
	ret = serial_w_mcu(mcu_serial[board].fd, buf_req, strlen((char *)buf_req), SERIAL_READ_TIMEOUT);
	pthread_mutex_unlock(&mcu_serial[board].lock);
	
	if (ret != strlen((char *)buf_req))
	{
		zsys_error("write_to_mcu: serial_w_mcu error(%d:%s)", errno, strerror(errno));
		return -1;
	}
	
	return 0;
}

