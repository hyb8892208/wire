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
#include "soap_gsmio.nsmap"

#include "czmq.h"
#include "serial.h"

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
//#include <mcheck.h>

//=====================================================
#define BACKLOG				(100)
#define MAX_THR				(10)
#define MAX_QUEUE			(1000)

#define MAX_REG				(13)

#define MAX_REQ_LEN			(32)
#define MAX_RSP_LEN			(32)
#define MAX_CHN_PER_REG		(8)


#define SERIAL_READ_TIMEOUT	(600)//(1)

#define ALL_BITS			(-1)

#define HDL_OK				(0)
#define HDL_ERR				(-1)
#define HDL_MCU_ERR			(-2)

#define MCU_INFO_FILE		"/tmp/mcu_info"

#define GSM_MCU_HDL_CONF	"/etc/asterisk/gsmio/gsmiosvr.conf"
#define MODULE_SEQUENCE_DIR	"/etc/asterisk/gsmio/mod_seq"

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

#define MAX_CHN_PER_MCU (MAX_CHN / MAX_MCU)

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


#define MODULE_NAME_LEN				(64)
#define MAX_ACTION_COUNT			(8)


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

typedef struct module_status_s
{
	int sig_on;		// 开启的信号值
	int sig_off;	// 关闭的信号值
}module_status_t;

typedef struct module_seq_s
{
	int cnt;
	char action[MAX_ACTION_COUNT][16];
}module_seq_t;

typedef struct mcu_options_s
{
	int pwr_off_force;	// 强制掉电，不等模块关机，立即掉电
	int check_chn_exist; // 0:不检查模块是否存在；1:检查模块是否存在。缺省:1
}mcu_options_t;
mcu_options_t g_mcu_options;
/*
typedef struct module_info_s
{
	int found;
	char name[64];
	char cgmm[32];
	int at_baud;
	module_seq_t pwr_on;
	module_seq_t pwr_off;
	module_seq_t pwrkey_on;
	module_seq_t pwrkey_off;
	module_seq_t emerg_off;
	module_status_t pwr_stat;
	module_status_t pwrkey_stat;
	
}module_info_t;
module_info_t module_info[MAX_CHN];*/


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

typedef struct module_port_map_s
{
	int port;
	int chn;
	char dev[64];
}module_port_map_t;

typedef struct chn_list_s
{
	int cnt;
	//int chn[MAX_CHN]; // default:-1, 0~31
	module_port_map_t port_map[MAX_CHN];
}chn_list_t;

typedef struct module_conf_s
{
	char name[64];
	char cgmm[32];
	int at_baud;
	module_seq_t pwr_on;
	module_seq_t pwr_off;
	module_seq_t pwrkey_on;
	module_seq_t pwrkey_off;
	module_seq_t emerg_off;
	module_status_t pwr_stat;
	module_status_t pwrkey_stat;
	chn_list_t chn_info;
}module_conf_t;


typedef struct module_info_s
{
	int cnt;
	module_conf_t module_conf[MAX_CHN];
}module_info_t;
module_info_t g_module_info;


typedef struct module_port_s
{
	int cnt;
	module_port_map_t module_port[MAX_CHN];
}module_port_t;
module_port_t g_module_port;


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

//=====================================================
void * queue_handle(void *arg);			// 线程处理函数
int enqueue(SOAP_SOCKET sock);		// 入队函数
SOAP_SOCKET dequeue(void);				// 出队函数
int get_mcu_version(int fd, char *ver, int len);
int get_mcu_help(int fd, char *buf, int len);
int set_module_power_on(int chn, module_info_t *module_info);
int set_module_power_off(int chn);
int get_module_power_status(int chn, ns__gsmio_rsp_t *result);
int set_module_on(int chn, module_info_t *module_info);
int set_module_off(int chn);
int set_module_emerg_off(int chn);
int get_module_status(int chn, ns__gsmio_rsp_t *result);
int set_simcard_status(int chn, int status);
int set_simcard_src(int chn, int status);
int get_simcard_src(int chn, ns__gsmio_rsp_t *result);
int get_simcard_insert_status(int chn, ns__gsmio_rsp_t *result);
int get_module_insert_status(int chn, ns__gsmio_rsp_t *result);
int read_from_mcu(int board, int reg, char *result);
int read_from_mcu_seri(int board, int reg, int nbr, char *result);
int write_to_mcu(int board, int reg, int chn, int val);

//=====================================================

int action_sig(int board, int reg, int bit, char *value);
int action_sleep(int board, int reg, int bit, char *value);
typedef int (*ACTION_FUNC)(int board, int reg, int bit, char *value);

// 网络处理信息数据结构
typedef struct ACTION_S
{
	char *type;				// 消息命令
	ACTION_FUNC func;		// 处理函数
}action_t;
action_t actions[] =
{
	{"sig",			action_sig},
	{"sleep",		action_sleep}
};
ACTION_FUNC find_action_func(char *type)
{
	int i = 0;
	for (i = 0; i < (sizeof(actions) / sizeof(action_t)); i++)
	{
		if (strcmp(actions[i].type, type) == 0)
		{
			return actions[i].func;
		}
	}
	return NULL;
}
int action_sig(int board, int reg, int bit, char *value)
{
	return write_to_mcu(board, reg, bit, atoi(value));
}
int action_sleep(int board, int reg, int bit, char *value)
{
	usleep(atoi(value));
	return HDL_OK;
}


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

int chn_exist(int chn)
{
	int i = 0;
	int j = 0;

	if (g_mcu_options.check_chn_exist == 1)
	{
		for (i = 0; i < g_module_info.cnt; i++)
		{
			for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++)
			{
				if (chn == g_module_info.module_conf[i].chn_info.port_map[j].chn)
				{
					return 1;
				}
			}
		}
		return 0;
	}
	else
		return 1;
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

module_conf_t * get_moudle_conf_by_chn(int chn)
{
	int i = 0;
	int j = 0;

	for (i = 0; i < g_module_info.cnt; i++)
	{
		for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++)
		{
			if (g_module_info.module_conf[i].chn_info.port_map[j].chn == chn)
			{
				return &g_module_info.module_conf[i];
			}
		}
	}
	return NULL;
}

int get_status_by_chn(char *status, int chn)
{
	int reg = chn / MAX_CHN_PER_REG;
	int bit = chn % MAX_CHN_PER_REG;
	char buf[4] = {0};
	int stat = -1;

	memcpy(buf, status+(reg*2), 2);
	stat = strtol(buf, NULL, 0x10);
	return ((stat & (1<<bit))>>bit);
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


int get_board_reg_bit_by_chn(int chn, int *board, int *reg, int *port, int max_chn)
{
	
	*board = chn / max_chn;
	*port  = chn % max_chn;
	*reg    = *port / MAX_CHN_PER_REG;
	*port  = *port % MAX_CHN_PER_REG;
	return 0;
}

#if 1
int get_module_seq_conf(module_conf_t *module_conf, char *file)
{
	int i = 0;
	char item[64] = {0};
	zconfig_t *root = NULL;
	char *buf = NULL;
	root = zconfig_load(file);
	if (root == NULL)
	{
		printf("init_log: load %s error(%d:%s)\n", file, errno, strerror(errno));
		return -1;
	}
	
	// module_info
	buf = zconfig_get(root, "/module/name", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_conf->name, buf);
	}
	buf = zconfig_get(root, "/module/cgmm", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_conf->cgmm, buf);
	}
	buf = zconfig_get(root, "/module/at_baud", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_conf->at_baud = atoi(buf);
	}
	
	// gsmio_pwr_on
	for (i = 0; i < MAX_ACTION_COUNT; i++)
	{
		sprintf(item, "/gsmio_pwr_on/%d", i+1);
		buf = zconfig_get(root, item, NULL);
		if (buf == NULL)
		{
			break;
		}
		strcpy(module_conf->pwr_on.action[i], buf);
		module_conf->pwr_on.cnt++;
	}
	// gsmio_pwr_off
	for (i = 0; i < MAX_ACTION_COUNT; i++)
	{
		sprintf(item, "/gsmio_pwr_off/%d", i+1);
		buf = zconfig_get(root, item, NULL);
		if (buf == NULL)
		{
			break;
		}
		strcpy(module_conf->pwr_off.action[i], buf);
		module_conf->pwr_off.cnt++;
	}
	// gsmio_pwr_status
	buf = zconfig_get(root, "/gsmio_pwr_status/sig_on", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_conf->pwr_stat.sig_on = atoi(buf);
	}
	buf = zconfig_get(root, "/gsmio_pwr_status/sig_off", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_conf->pwr_stat.sig_off= atoi(buf);
	}
	// gsmio_pwrkey_on
	for (i = 0; i < MAX_ACTION_COUNT; i++)
	{
		sprintf(item, "/gsmio_pwrkey_on/%d", i+1);
		buf = zconfig_get(root, item, NULL);
		if (buf == NULL)
		{
			break;
		}
		strcpy(module_conf->pwrkey_on.action[i], buf);
		module_conf->pwrkey_on.cnt++;
	}
	// gsmio_pwrkey_off
	for (i = 0; i < MAX_ACTION_COUNT; i++)
	{
		sprintf(item, "/gsmio_pwrkey_off/%d", i+1);
		buf = zconfig_get(root, item, NULL);
		if (buf == NULL)
		{
			break;
		}
		strcpy(module_conf->pwrkey_off.action[i], buf);
		module_conf->pwrkey_off.cnt++;
	}
	// gsmio_emerg_off
	for (i = 0; i < MAX_ACTION_COUNT; i++)
	{
		sprintf(item, "/gsmio_emerg_off/%d", i+1);
		buf = zconfig_get(root, item, NULL);
		if (buf == NULL)
		{
			break;
		}
		strcpy(module_conf->emerg_off.action[i], buf);
		module_conf->emerg_off.cnt++;
	}
	// gsmio_pwrkey_status
	buf = zconfig_get(root, "/gsmio_pwrkey_status/sig_on", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_conf->pwrkey_stat.sig_on = atoi(buf);
	}
	buf = zconfig_get(root, "/gsmio_pwrkey_status/sig_off", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_conf->pwrkey_stat.sig_off= atoi(buf);
	}
	
	zconfig_destroy(&root);
	return 0;
}
#else
int get_module_seq_conf(int chn, char *file)
{
	zconfig_t *root = NULL;
	char *buf = NULL;
	root = zconfig_load(file);
	if (root == NULL)
	{
		printf("init_log: load %s error(%d:%s)\n", file, errno, strerror(errno));
		return -1;
	}
	
	// module_info
	buf = zconfig_get(root, "/module/name", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].name, buf);
	}
	buf = zconfig_get(root, "/module/cgmm", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].cgmm, buf);
	}
	buf = zconfig_get(root, "/module/at_baud", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_info[chn].at_baud = atoi(buf);
	}
	// gsmio_pwr_on
	buf = zconfig_get(root, "/gsmio_pwr_on/sig_s", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwr_on.sig_s, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwr_on/sleep_i", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwr_on.sleep_i, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwr_on/sig_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwr_on.sig_e, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwr_on/sleep_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwr_on.sleep_e, buf);
	}
	// gsmio_pwr_off
	buf = zconfig_get(root, "/gsmio_pwr_off/sig_s", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwr_off.sig_s, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwr_off/sleep_i", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwr_off.sleep_i, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwr_off/sig_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwr_off.sig_e, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwr_off/sleep_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwr_off.sleep_e, buf);
	}
	// gsmio_pwr_status
	buf = zconfig_get(root, "/gsmio_pwr_status/sig_on", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_info[chn].pwr_stat.sig_on = atoi(buf);
	}
	buf = zconfig_get(root, "/gsmio_pwr_status/sig_off", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_info[chn].pwr_stat.sig_off= atoi(buf);
	}
	// gsmio_pwrkey_on
	buf = zconfig_get(root, "/gsmio_pwrkey_on/sig_s", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwrkey_on.sig_s, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwrkey_on/sleep_i", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwrkey_on.sleep_i, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwrkey_on/sig_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwrkey_on.sig_e, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwrkey_on/sleep_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwrkey_on.sleep_e, buf);
	}
	// gsmio_pwrkey_off
	buf = zconfig_get(root, "/gsmio_pwrkey_off/sig_s", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwrkey_off.sig_s, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwrkey_off/sleep_i", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwrkey_off.sleep_i, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwrkey_off/sig_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwrkey_off.sig_e, buf);
	}
	buf = zconfig_get(root, "/gsmio_pwrkey_off/sleep_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].pwrkey_off.sleep_e, buf);
	}
	// gsmio_emerg_off
	buf = zconfig_get(root, "/gsmio_emerg_off/sig_s", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].emerg_off.sig_s, buf);
	}
	buf = zconfig_get(root, "/gsmio_emerg_off/sleep_i", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].emerg_off.sleep_i, buf);
	}
	buf = zconfig_get(root, "/gsmio_emerg_off/sig_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].emerg_off.sig_e, buf);
	}
	buf = zconfig_get(root, "/gsmio_emerg_off/sleep_e", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		strcpy(module_info[chn].emerg_off.sleep_e, buf);
	}
	// gsmio_pwrkey_status
	buf = zconfig_get(root, "/gsmio_pwrkey_status/sig_on", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_info[chn].pwrkey_stat.sig_on = atoi(buf);
	}
	buf = zconfig_get(root, "/gsmio_pwrkey_status/sig_off", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		module_info[chn].pwrkey_stat.sig_off= atoi(buf);
	}
	
	zsys_info("module_info        [chn:%02d]: name   = [%s], cgmm = [%s], at_baud = [%d]", chn, module_info[chn].name, module_info[chn].cgmm, module_info[chn].at_baud);
	zsys_info("gsmio_pwr_on       [chn:%02d]: sig_s  = [%s], sleep_i = [%s], sig_e = [%s], sleep_e = [%s]", chn, module_info[chn].pwr_on.sig_s, module_info[chn].pwr_on.sleep_i, module_info[chn].pwr_on.sig_e, module_info[chn].pwr_on.sleep_e);
	zsys_info("gsmio_pwr_off      [chn:%02d]: sig_s  = [%s], sleep_i = [%s], sig_e = [%s], sleep_e = [%s]", chn, module_info[chn].pwr_off.sig_s, module_info[chn].pwr_off.sleep_i, module_info[chn].pwr_off.sig_e, module_info[chn].pwr_off.sleep_e);
	zsys_info("gsmio_pwr_status   [chn:%02d]: sig_on = [%d], sig_off = [%d]", chn, module_info[chn].pwr_stat.sig_on, module_info[chn].pwr_stat.sig_off);
	zsys_info("gsmio_pwrkey_on    [chn:%02d]: sig_s  = [%s], sleep_i = [%s], sig_e = [%s], sleep_e = [%s]", chn, module_info[chn].pwrkey_on.sig_s, module_info[chn].pwrkey_on.sleep_i, module_info[chn].pwrkey_on.sig_e, module_info[chn].pwrkey_on.sleep_e);
	zsys_info("gsmio_pwrkey_off   [chn:%02d]: sig_s  = [%s], sleep_i = [%s], sig_e = [%s], sleep_e = [%s]", chn, module_info[chn].pwrkey_off.sig_s, module_info[chn].pwrkey_off.sleep_i, module_info[chn].pwrkey_off.sig_e, module_info[chn].pwrkey_off.sleep_e);
	zsys_info("gsmio_emerg_off    [chn:%02d]: sig_s  = [%s], sleep_i = [%s], sig_e = [%s], sleep_e = [%s]", chn, module_info[chn].emerg_off.sig_s, module_info[chn].emerg_off.sleep_i, module_info[chn].emerg_off.sig_e, module_info[chn].emerg_off.sleep_e);
	zsys_info("gsmio_pwrkey_status[chn:%02d]: sig_on = [%d], sig_off = [%d]", chn, module_info[chn].pwrkey_stat.sig_on, module_info[chn].pwrkey_stat.sig_off);
	zsys_info("===============================================================================================================");
	zconfig_destroy(&root);
	return 0;
}
#endif

int module_start(int chn)
{
	set_module_power_on(-1, &g_module_info);
	return 0;
}
int init_module_port_from_hwinfo(void)
{
	int i = 0;
	char item[32] = {0};
	char content[64] = {0};
	int cnt = 0;

	for (i = 0; i < MAX_CHN; i++)
	{
		sprintf(item, "dev-%d", i+1);
		memset(content, 0, sizeof(content));
		get_config("/tmp/hwinfo", "port_map", item, content);
		if (strlen(content) <= 0)
		{
			break;
		}
		g_module_port.module_port[cnt].chn = atoi(content) - 1;
		g_module_port.module_port[cnt].port = i+1;
		memset(content, 0, sizeof(content));
		get_config("/tmp/hwinfo", "gsm", item, content);
		if (strlen(content) > 0)
		{
			memcpy(g_module_port.module_port[cnt].dev, content, strlen(content));
		}
		cnt++;
	}
	g_module_port.cnt = cnt;
	zsys_debug("init_module_port: cnt = [%d]", g_module_port.cnt);
	for (i = 0; i < g_module_port.cnt; i++)
	{
		zsys_debug("init_module_port: [No.:%d] chn = [%d], port = [%d] dev = [%s]", \
			i, g_module_port.module_port[i].chn, g_module_port.module_port[i].port, g_module_port.module_port[i].dev);
	}
	return 0;
}
int init_module_port(void)
{
	int i = 0;
	int j = 0;
	char buf[4] = {0};
	char item[32] = {0};
	char content[64] = {0};
	unsigned char val = 0;
	ns__gsmio_rsp_t rsp;

	memset((char *)&g_module_port, 0, sizeof(g_module_port));
	rsp.value = (char *)soap_malloc(NULL, SOAP_TMPLEN);
	memset(rsp.value, 0, SOAP_TMPLEN);
	if (get_module_insert_status(ALL_BITS, &rsp) == HDL_ERR)
	{
		SOAP_FREE(NULL, rsp.value);
		return -1;
	}
	zsys_debug("init_module_port: module insert status = [%s]", rsp.value);
	for (i = 0; i < (strlen(rsp.value) / 2); i++)
	{
		memcpy(buf, rsp.value+(i*2), 2);
		val = strtol(buf, NULL, 0x10);
		for (j = 0; j < 8; j++)
		{
			if ((val & (1 << j)) != 0)
			{
				g_module_port.module_port[g_module_port.cnt++].chn = i*MAX_CHN_PER_REG*2 + j*2;
				g_module_port.module_port[g_module_port.cnt++].chn = i*MAX_CHN_PER_REG*2 + j*2+1;
			}
		}
	}
	SOAP_FREE(NULL, rsp.value);
	//for (i = 0; i < g_module_port.cnt; i++)
	for (i = 0; i < MAX_CHN; i++)
	{
		sprintf(item, "dev-%d", i+1);
		memset(content, 0, sizeof(content));
		get_config("/tmp/hwinfo", "port_map", item, content);
		zsys_debug("content = [%s]", content);
		if (strlen(content) <= 0)
		{
			break;
		}
		for (j = 0; j < g_module_port.cnt; j++)
		{
			if (atoi(content) == g_module_port.module_port[j].chn+1)
			{
				g_module_port.module_port[j].port = i+1;
				memset(content, 0, sizeof(content));
				get_config("/tmp/hwinfo", "gsm", item, content);
				if (strlen(content) > 0)
				{
					memcpy(g_module_port.module_port[j].dev, content, strlen(content));
				}
			}
		}
	}
	zsys_debug("init_module_port: cnt = [%d]", g_module_port.cnt);
	for (i = 0; i < g_module_port.cnt; i++)
	{
		zsys_debug("init_module_port: [No.:%d] chn = [%d], port = [%d] dev = [%s]", \
			i, g_module_port.module_port[i].chn, g_module_port.module_port[i].port, g_module_port.module_port[i].dev);
	}
	return 0;
}
void init_module_conf(void)
{
	int i = 0;
	int j = 0;
	memset((char *)&g_module_info, 0, sizeof(g_module_info));
	for (i = 0; i < (sizeof(g_module_info.module_conf) / sizeof(module_conf_t)); i++)
	{
		for (j = 0; j < (sizeof(g_module_info.module_conf[i].chn_info.port_map) / sizeof(module_port_map_t)); j++)
		{
			g_module_info.module_conf[i].chn_info.port_map[j].chn = -1;
			g_module_info.module_conf[i].chn_info.port_map[j].port = -1;
		}
	}
}


void print_module_info(module_info_t *module_info)
{
	int i = 0;
	int j = 0;
	int m = 0;

	zsys_info("module_info        cnt: %d", module_info->cnt);
	for (i = 0; i < module_info->cnt; i++)
	{
		zsys_info("module_info            [nbr:%02d]: name   = [%s], cgmm = [%s], at_baud = [%d]", i, module_info->module_conf[i].name, module_info->module_conf[i].cgmm, module_info->module_conf[i].at_baud);

		zsys_info("gsmio_pwr_on           cnt = [%d]", module_info->module_conf[i].pwr_on.cnt);
		for (j = 0; j < module_info->module_conf[i].pwr_on.cnt; j++)
		{
			zsys_info("gsmio_pwr_on               [nbr:%02d]: action = [%s]", j, module_info->module_conf[i].pwr_on.action[j]);
		}
		zsys_info("gsmio_pwr_off          cnt = [%d]", module_info->module_conf[i].pwr_off.cnt);
		for (j = 0; j < module_info->module_conf[i].pwr_off.cnt; j++)
		{
			zsys_info("gsmio_pwr_off              [nbr:%02d]: action = [%s]", j, module_info->module_conf[i].pwr_off.action[j]);
		}
		zsys_info("gsmio_pwr_status       [nbr:%02d]: sig_on = [%d], sig_off = [%d]", i, module_info->module_conf[i].pwr_stat.sig_on, module_info->module_conf[i].pwr_stat.sig_off);
		zsys_info("gsmio_pwrkey_on        cnt = [%d]", module_info->module_conf[i].pwrkey_on.cnt);
		for (j = 0; j < module_info->module_conf[i].pwrkey_on.cnt; j++)
		{
			zsys_info("gsmio_pwrkey_on            [nbr:%02d]: action = [%s]", j, module_info->module_conf[i].pwrkey_on.action[j]);
		}
		zsys_info("gsmio_pwrkey_off       cnt = [%d]", module_info->module_conf[i].pwrkey_off.cnt);
		for (j = 0; j < module_info->module_conf[i].pwrkey_off.cnt; j++)
		{
			zsys_info("gsmio_pwrkey_off           [nbr:%02d]: action = [%s]", j, module_info->module_conf[i].pwrkey_off.action[j]);
		}
		zsys_info("gsmio_emerg_off        cnt = [%d]", module_info->module_conf[i].emerg_off.cnt);
		for (j = 0; j < module_info->module_conf[i].emerg_off.cnt; j++)
		{
			zsys_info("gsmio_emerg_off            [nbr:%02d]: action = [%s]", j, module_info->module_conf[i].emerg_off.action[j]);
		}
		zsys_info("gsmio_pwrkey_status    [nbr:%02d]: sig_on = [%d], sig_off = [%d]", i, module_info->module_conf[i].pwrkey_stat.sig_on, module_info->module_conf[i].pwrkey_stat.sig_off);
		zsys_info("===============================================================================================================");
	
		for (m = 0; m < module_info->module_conf[i].chn_info.cnt; m++)
		{
			zsys_info("chn_info.chn           [nbr:%02d]: chn = [%d], port = [%d], dev = [%s]", \
				m, module_info->module_conf[i].chn_info.port_map[m].chn, module_info->module_conf[i].chn_info.port_map[m].port, module_info->module_conf[i].chn_info.port_map[m].dev);
		}
		zsys_info("===============================================================================================================");
	}
}
void record_module_info(module_info_t *module_info, char *file)
{
	int i = 0;
	int j = 0;
	int m = 0;
	FILE *fp = NULL;

	fp = fopen(file, "w");
	if (fp == NULL)
	{
		return;
	}

	fprintf(fp, "module_info        cnt: %d", module_info->cnt);
	for (i = 0; i < module_info->cnt; i++)
	{
		fprintf(fp, "module_info            [nbr:%02d]: name   = [%s], cgmm = [%s], at_baud = [%d]\n", i, module_info->module_conf[i].name, module_info->module_conf[i].cgmm, module_info->module_conf[i].at_baud);

		fprintf(fp, "gsmio_pwr_on           cnt = [%d]\n", module_info->module_conf[i].pwr_on.cnt);
		for (j = 0; j < module_info->module_conf[i].pwr_on.cnt; j++)
		{
			fprintf(fp, "gsmio_pwr_on               [nbr:%02d]: action = [%s]\n", j, module_info->module_conf[i].pwr_on.action[j]);
		}
		fprintf(fp, "gsmio_pwr_off          cnt = [%d]\n", module_info->module_conf[i].pwr_off.cnt);
		for (j = 0; j < module_info->module_conf[i].pwr_off.cnt; j++)
		{
			fprintf(fp, "gsmio_pwr_off              [nbr:%02d]: action = [%s]\n", j, module_info->module_conf[i].pwr_off.action[j]);
		}
		fprintf(fp, "gsmio_pwr_status       [nbr:%02d]: sig_on = [%d], sig_off = [%d]\n", i, module_info->module_conf[i].pwr_stat.sig_on, module_info->module_conf[i].pwr_stat.sig_off);
		fprintf(fp, "gsmio_pwrkey_on        cnt = [%d]\n", module_info->module_conf[i].pwrkey_on.cnt);
		for (j = 0; j < module_info->module_conf[i].pwrkey_on.cnt; j++)
		{
			fprintf(fp, "gsmio_pwrkey_on            [nbr:%02d]: action = [%s]\n", j, module_info->module_conf[i].pwrkey_on.action[j]);
		}
		fprintf(fp, "gsmio_pwrkey_off       cnt = [%d]\n", module_info->module_conf[i].pwrkey_off.cnt);
		for (j = 0; j < module_info->module_conf[i].pwrkey_off.cnt; j++)
		{
			fprintf(fp, "gsmio_pwrkey_off           [nbr:%02d]: action = [%s]\n", j, module_info->module_conf[i].pwrkey_off.action[j]);
		}
		fprintf(fp, "gsmio_emerg_off        cnt = [%d]\n", module_info->module_conf[i].emerg_off.cnt);
		for (j = 0; j < module_info->module_conf[i].emerg_off.cnt; j++)
		{
			fprintf(fp, "gsmio_emerg_off            [nbr:%02d]: action = [%s]\n", j, module_info->module_conf[i].emerg_off.action[j]);
		}
		fprintf(fp, "gsmio_pwrkey_status    [nbr:%02d]: sig_on = [%d], sig_off = [%d]\n", i, module_info->module_conf[i].pwrkey_stat.sig_on, module_info->module_conf[i].pwrkey_stat.sig_off);
		fprintf(fp, "===============================================================================================================\n");
	
		for (m = 0; m < module_info->module_conf[i].chn_info.cnt; m++)
		{
			fprintf(fp, "chn_info.chn           [nbr:%02d]: chn = [%d], port = [%d], dev = [%s]\n", \
				m, module_info->module_conf[i].chn_info.port_map[m].chn, module_info->module_conf[i].chn_info.port_map[m].port, module_info->module_conf[i].chn_info.port_map[m].dev);
		}
		fprintf(fp, "===============================================================================================================\n");
	}
	fflush(fp);
	fclose(fp);
}



int gsm_trim(const char *in, int in_len, char *out, int out_len) 
{
    int i=0;
    int j=0;

    for (i = 0; i < in_len && j < (out_len-1); i++) {
        if ((in[i] != '\r') && (in[i] != '\n') ) {
            out[j++] = in[i];
        }
    }
    out[j] = '\0';
	
    return j;
}

int get_module_type_by_at(int fd, char *cgmm, char *type, int len)
{
	int ret = 0;
	char buf_w[1024] = {0};
	char buf_r[1024] = {0};

	sprintf(buf_w, "%s\r\n", cgmm);
	//zsys_debug("get_module_type_by_at: [PC ==> GSMModule]: [%s]", buf_w);
	ret = serial_wr_gsm(fd, (uint8 *)buf_w, (uint32)strlen(buf_w), (uint8 *)buf_r, (uint32)sizeof(buf_r), 3600);
	if (ret < 0)
	{
		zsys_error("get_module_type_by_at: get module type error(%d:%s)", errno, strerror(errno));
		return -1;
	}
	if (strlen(buf_r) <= 0)
	{
		return -1;
	}
	//zsys_debug("get_module_type_by_at: [PC <== GSMModule]: [%s]", buf_r);
	gsm_trim(buf_r, strlen(buf_r), type, len);
	//zsys_debug("get_module_type_by_at: type = [len:%d - %s]", strlen(type), type);
	return 0;
}

int check_module_type_valid(char *type)
{
	int i = 0;

	for (i = 0; i < g_module_info.cnt; i++)
	{
		if (strstr(type, g_module_info.module_conf[i].name) != NULL)
		{
			return 0;
		}
	}
	return -1;
}

int init_module_type(void)
{
	int i = 0;
	int j = 0;
	int n = 0;
	int k = 0;
	int trys = 3;
	char type[1024] = {0};
	ns__gsmio_rsp_t rsp;
	module_info_t module_info;
	module_port_t module_port;

	memset((char *)&module_info, 0, sizeof(module_info));
	memcpy((char *)&module_port, (char *)&g_module_port, sizeof(module_port));
	rsp.value = (char *)soap_malloc(NULL, SOAP_TMPLEN);
	memset(rsp.value, 0, SOAP_TMPLEN);

	for (i = 0; i < g_module_info.cnt; i++) // 模块类型
	{
		memset((char *)&module_info, 0, sizeof(module_info));
		module_info.cnt = 1;
		memcpy((char *)&module_info.module_conf[0], (char *)&g_module_info.module_conf[i], sizeof(g_module_info.module_conf[i]));
		// 拷贝在位模块号及数量到g_module_info
		// 模块上电
		// 模块开机
		// 检查模块开机状态，如果开机，则发at获取版本，并记录到g_module_info中
		module_info.module_conf[0].chn_info.cnt = module_port.cnt;
		if (module_info.module_conf[0].chn_info.cnt == 0)
		{
			break;
		}
		zsys_debug("init_module_type: module_info.module_conf[0].chn_info.cnt = %d", module_info.module_conf[0].chn_info.cnt);
		for (j = 0; j < module_info.module_conf[0].chn_info.cnt; j++)
		{
			module_info.module_conf[0].chn_info.port_map[j].chn = module_port.module_port[j].chn;
			module_info.module_conf[0].chn_info.port_map[j].port = module_port.module_port[j].port;
			strcpy(module_info.module_conf[0].chn_info.port_map[j].dev, module_port.module_port[j].dev);
		}
		//print_module_info(&module_info);
		if (set_module_power_on(ALL_BITS, &module_info) != 0)
		{
			zsys_error("init_module_type: set all module power on error(%d:%s)", errno, strerror(errno));
			continue;
		}
		zsys_debug("init_module_type: set module power on succ");
		if (set_module_on(ALL_BITS, &module_info) != 0)
		{
			zsys_error("init_module_type: set all module on error(%d:%s)", errno, strerror(errno));
			continue;
		}
		if (get_module_status(ALL_BITS, &rsp) == HDL_ERR)
		{
			zsys_error("init_module_type: set all module power on error(%d:%s)", errno, strerror(errno));
			continue;
		}
		for (j = 0; j < module_info.module_conf[0].chn_info.cnt; j++)
		{
			if (get_status_by_chn(rsp.value, module_info.module_conf[0].chn_info.port_map[j].chn) == 1)
			{
				// 发送at获取类型
				int fd = -1;
				fd = open_serial(module_info.module_conf[0].chn_info.port_map[j].dev, 115200, 0, 8, 1, 0);
				if (fd < 0)
				{
					zsys_error("init_module_type: open dev[%s] error(%d:%s)", module_info.module_conf[0].chn_info.port_map[j].dev, errno, strerror(errno));
					continue;
				}
				// 
				trys = 3;
				while (trys > 0)
				{
					memset(type, 0, sizeof(type));
					if (get_module_type_by_at(fd, module_info.module_conf[0].cgmm, type, sizeof(type)) == 0)
					{
						if (check_module_type_valid(type) == 0)
						{
							break;
						}
					}
					trys--;
				}
				if (trys > 0)
				{
					#if 1
					if (strstr(type, g_module_info.module_conf[i].name) != NULL)
					{
						zsys_debug("init_module_type: i = %d, j = %d, type = [%s], name = [%s]", i, j, type, g_module_info.module_conf[i].name);
						g_module_info.module_conf[i].chn_info.port_map[g_module_info.module_conf[i].chn_info.cnt].chn = module_info.module_conf[0].chn_info.port_map[j].chn;
						g_module_info.module_conf[i].chn_info.port_map[g_module_info.module_conf[i].chn_info.cnt].port = module_info.module_conf[0].chn_info.port_map[j].port;
						strcpy(g_module_info.module_conf[i].chn_info.port_map[g_module_info.module_conf[i].chn_info.cnt++].dev, module_info.module_conf[0].chn_info.port_map[j].dev);

						// update module_port
						for (n = 0; n < module_port.cnt; n++)
						{
							if (module_port.module_port[n].chn == module_info.module_conf[0].chn_info.port_map[j].chn)
							{
								for (k = n; k < (module_port.cnt - n - 1); k++)
								{
									module_port.module_port[k].chn = module_port.module_port[k+1].chn;
									module_port.module_port[k].port = module_port.module_port[k+1].port;
									strcpy(module_port.module_port[k].dev, module_port.module_port[k+1].dev);
								}
								module_port.module_port[k].chn = 0;
								module_port.module_port[k].port = 0;
								memset(module_port.module_port[k].dev, 0, sizeof(module_port.module_port[k].dev));
								module_port.cnt--;
								break;
							}
						}
					}
					#else
					for (m = 0; m < g_module_info.cnt; m++)
					{
						if (strlen(type) > 0 && strstr(type, g_module_info.module_conf[m].name) != NULL)
						{
							zsys_debug("init_module_type: i = %d, j = %d, m = %d, type = [%s], name = [%s]", i, j, m, type, g_module_info.module_conf[m].name);
							g_module_info.module_conf[m].chn_info.port_map[g_module_info.module_conf[m].chn_info.cnt].chn = module_info.module_conf[0].chn_info.port_map[j].chn;
							g_module_info.module_conf[m].chn_info.port_map[g_module_info.module_conf[m].chn_info.cnt].port = module_info.module_conf[0].chn_info.port_map[j].port;
							strcpy(g_module_info.module_conf[m].chn_info.port_map[g_module_info.module_conf[m].chn_info.cnt++].dev, module_info.module_conf[0].chn_info.port_map[j].dev);

							// update module_port
							for (n = 0; n < module_port.cnt; n++)
							{
								if (g_module_info.module_conf[m].chn_info.port_map[g_module_info.module_conf[m].chn_info.cnt].chn = module_port.module_port[n].chn)
								{
									for (k = n; k < (module_port.cnt - n - 1); k++)
									{
										module_port.module_port[k].chn = module_port.module_port[k+1].chn;
										module_port.module_port[k].port = module_port.module_port[k+1].port;
										strcpy(module_port.module_port[k].dev, module_port.module_port[k+1].dev);
									}
									module_port.module_port[k].chn = 0;
									module_port.module_port[k].port = 0;
									memset(module_port.module_port[k].dev, 0, sizeof(module_port.module_port[k].dev));
									module_port.cnt--;
									break;
								}
							}
						}
					}
					#endif
				}
				//
				close_serial(fd);
			}
		}
	}

	
	SOAP_FREE(NULL, rsp.value);
	return 0;
}

int init_module_info(void)
{
/*
	循环所有通道模块
		打开模块电平信号配置文件目录，循环遍历文件
			读取配置文件
			操作模块上电和开机
			判断模块开关机状态
			如果关机，则循环下一文件
			如果开机，则打开该模块AT串口，输入cgmm命令，查询模块名称，和配置的模块名对比，
			如果模块名相同，则进入下一模块操作
 */
/*
	循环所有模块类型
		统一按模块类型开机，逐一检查模块开机状态，开机则通过at获取并记录模块类型名，没有开机则跳过，
	最后根据记录的模块类型名补充操作参数。
	
 */

/*
	1. 循环操作配置文件目录，读取所有配置文件并记录
	2. 循环所有配置
	2.1 操作开机
	2.2 检测开机状态，如开机发送AT读取模块名，并记录该模块
 */

	struct dirent* ent = NULL;
	DIR *pdir = NULL;  
	//char buf[128] = {0};
	char filename[128] = {0};

	init_module_conf();
	
	//init_module_port();
	init_module_port_from_hwinfo();

	
 	// 尝试每个配置文件
	pdir = opendir(MODULE_SEQUENCE_DIR);
	if (pdir == NULL)
	{
		zsys_error("init_module_info: open dir [%s] error(%d:%s)", MODULE_SEQUENCE_DIR, errno, strerror(errno));
		return -1;
	}
	while ((ent = readdir(pdir)) != NULL)
	{
		if (ent->d_type == DT_REG)
		{
			sprintf(filename, "%s/%s", MODULE_SEQUENCE_DIR, ent->d_name);
			zsys_debug("init_module_info[cnt:%d]: filename = [%s]", g_module_info.cnt, filename);

			// 读取配置
			if (get_module_seq_conf(&g_module_info.module_conf[g_module_info.cnt], filename) == 0)
			{
				g_module_info.cnt++;
			}
		}
	}
	closedir(pdir);
	//print_module_info(&g_module_info);

	init_module_type();

	print_module_info(&g_module_info);
	record_module_info(&g_module_info, "/tmp/mod_info");

	//
	return 0;
}

int get_module_off_last_time(int chn)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int max = 0;

	if (chn == -1)
	{
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
	}
	else
	{
		get_board_reg_bit_by_chn(chn, &i, &j, &k, max_chn_mcu_module_pwrkey);
		max = regs[i][regs_module_pwrkey[j].reg].reg_bit[k].last;
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
	set_simcard_src(-1, 1);
	get_simcard_src(-1, &result);
	zsys_debug("result = [%x]", result);

	set_simcard_src(-1, 0);
	get_simcard_src(-1, &result);
	zsys_debug("result = [%x]", result);

	
	set_simcard_src(5, 1);
	get_simcard_src(5, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_src(13, 1);
	get_simcard_src(13, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_src(18, 1);
	get_simcard_src(18, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_src(30, 1);
	get_simcard_src(30, &result);
	zsys_debug("result = [%x]", result);
	
	get_simcard_src(-1, &result);
	zsys_debug("result = [%x]", result);

	
	set_simcard_src(-1, 0);
	get_simcard_src(-1, &result);
	zsys_debug("result = [%x]", result);

	
	set_simcard_src(5, 1);
	get_simcard_src(5, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_src(13, 1);
	get_simcard_src(13, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_src(18, 1);
	get_simcard_src(18, &result);
	zsys_debug("result = [%x]", result);
	
	set_simcard_src(30, 1);
	get_simcard_src(30, &result);
	zsys_debug("result = [%x]", result);
	
	get_simcard_src(-1, &result);
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
	serial_wr_mcu(mcu_serial[0].fd, (uint8 *)buf_w, strlen(buf_w), (uint8 *)buf_r, (uint32)len_r, SERIAL_READ_TIMEOUT);

	zsys_debug("muc[%d], cmd = [%s], rsp = [%s]", 0, buf_w, buf_r);

usleep(800*1000);
	
	sprintf(buf_w, "write %d=%02xh\n", 6, 0xff);
	serial_w_mcu(mcu_serial[0].fd, (uint8 *)buf_w, strlen(buf_w), 1);
	zsys_debug("muc[%d], cmd = [%s]", 0, buf_w);

	sprintf(buf_w, "read %d\n", 6);
	serial_wr_mcu(mcu_serial[0].fd, (uint8 *)buf_w, strlen(buf_w), (uint8 *)buf_r, (uint32)len_r, SERIAL_READ_TIMEOUT);

	zsys_debug("muc[%d], cmd = [%s], rsp = [%s]", 0, buf_w, buf_r);

	char buf[128] = {0};
	get_mcu_version(mcu_serial[0].fd, buf, sizeof(buf));
	zsys_debug("version = [%s]", buf);
	
	close_mcu_serial();
	
}
void test_simcard_enable(void)
{
	ns__gsmio_rsp_t rsp;


	
	init_mcu_serial();


	
	rsp.value = soap_malloc(NULL, SOAP_TMPLEN);
	memset(rsp.value, 0, SOAP_TMPLEN);

	
	set_simcard_src(-1, 1);
	get_simcard_src(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);
	set_simcard_src(-1, 0);
	get_simcard_src(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);


	
	set_simcard_src(-1, 1);
	get_simcard_src(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);

	set_simcard_src(0, 0);
	set_simcard_src(7, 0);
	set_simcard_src(8, 0);
	set_simcard_src(15, 0);
	set_simcard_src(16, 0);
	set_simcard_src(23, 0);
	set_simcard_src(24, 0);
	set_simcard_src(31, 0);
	get_simcard_src(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);
	
	
	
	set_simcard_src(-1, 0);
	get_simcard_src(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);

	
	set_simcard_src(0, 1);
	set_simcard_src(7, 1);
	set_simcard_src(8, 1);
	set_simcard_src(15, 1);
	set_simcard_src(16, 1);
	set_simcard_src(23, 1);
	set_simcard_src(24, 1);
	set_simcard_src(31, 1);
	get_simcard_src(-1, &rsp);
	zsys_debug("test: result: [%s]", rsp.value);


	//
	close_mcu_serial();
}
void test_simcard_insert(void)
{
	int i = 0;
	ns__gsmio_rsp_t rsp;

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
	ns__gsmio_rsp_t rsp;

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
	ns__gsmio_rsp_t rsp;

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
	ns__gsmio_rsp_t rsp;

	init_mcu_serial();

	rsp.value = soap_malloc(NULL, SOAP_TMPLEN);

	
#if 1 // power on
zsys_debug("module power on: -------------------------------------------------------------------------");
	set_module_power_on(ALL_BITS, &g_module_info);

	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_power_status(ALL_BITS, &rsp);
	//zsys_debug("test: get_module_power_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);

	memset(rsp.value, 0, SOAP_TMPLEN);
	get_module_status(ALL_BITS, &rsp);
	zsys_debug("test: get_module_status chn = %d, rsp.value = [%s]", ALL_BITS, rsp.value);
#endif


#if 1 // module on
zsys_debug("set_module_on: -------------------------------------------------------------------------");
	set_module_on(ALL_BITS, &g_module_info);


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

int get_config_value(char *file, char *domain, char *item, char *value)
{
	int ret = 0;
	zconfig_t *root = NULL;
	char *buf = NULL;
	char path[128] = {0};
	
	if (file == NULL || domain == NULL || item == NULL || value == NULL)
		return -1;
	
	root = zconfig_load(file);
	if (root == NULL)
	{
		zsys_error("init_log: load %s error(%d:%s)", file, errno, strerror(errno));
		return -1;
	}
	sprintf(path, "/%s/%s", domain, item);
	buf = zconfig_get(root, path, NULL);
	if (buf == NULL || strlen(buf) <= 0)
	{
		ret = -1;
	}
	strcpy(value, buf);
	zconfig_destroy(&root);
	return ret;
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

int init_mcu_options(void)
{
	zconfig_t *root = NULL;
	char *buf = NULL;

	root = zconfig_load(GSM_MCU_HDL_CONF);
	if (root == NULL)
	{
		zsys_error("init_mcu_options: load %s error", GSM_MCU_HDL_CONF);
		return -1;
	}
	
	buf = zconfig_get(root, "/mcu/pwr_off_force", NULL);
	if (buf == NULL || strlen(buf) <= 0)
	{
		g_mcu_options.pwr_off_force = 0;
		zsys_error("init_mcu_options: get /mcu/pwr_off_force from %s error", GSM_MCU_HDL_CONF);
	}
	else
		g_mcu_options.pwr_off_force = atoi(buf);
	
	buf = zconfig_get(root, "/mcu/check_chn_exist", NULL);
	if (buf == NULL || strlen(buf) <= 0)
	{
		g_mcu_options.check_chn_exist = 1;
		zsys_error("init_mcu_options: get /mcu/check_chn_exist from %s error", GSM_MCU_HDL_CONF);
	}
	else
		g_mcu_options.check_chn_exist = atoi(buf);
	
	zsys_info("init_mcu_options: pwr_off_force = [%d], check_chn_exist = [%d]", \
		g_mcu_options.pwr_off_force, g_mcu_options.check_chn_exist);
	zconfig_destroy(&root);
	
	return 0;
}

int main(int argc, char **argv)
{
	struct soap server_soap;
	queue_handle_param_t hdl_params[MAX_THR];
	zpoller_t *poller = NULL;
	int ret = 0;
	void *res = NULL;

	//setenv("MALLOC_TRACE", "output.server", 1);
	//mtrace();

	//zsys_init();
	init_log(argv[0]);
	soap_init(&server_soap);
	init_mcu_options();
	
	ret = init_mcu_serial();
	if (ret != 0)
	{
		return -1;
	}
	init_module_info();
	
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
		zsys_info("Accept connection from IP = %d.%d.%d.%d, socket = %d", \
			((server_soap.ip)>>24)&0xFF, ((server_soap.ip)>>16)&0xFF, ((server_soap.ip)>>8)&0xFF, \
			(server_soap.ip)&0xFF, server_soap.socket);

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

	
	soap_done(&server_soap);
	close_mcu_serial();
	zsys_shutdown();
	fclose(fd_log);
	return 0;
}

//gsoap ns service method: set gsm simcard source
int ns__gsmio_set_simcard_src(struct soap *soap, int chn, int src, int *result)
{(void)soap;
	
	if (chn < -1 || chn >= MAX_CHN || (src != 0 && src != 1))
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsmio_set_simcard_src[sock:%d][chn:%d]: args invalid", soap->socket, chn);
		return SOAP_OK;
	}

	//*result = set_simcard_src(chn, 1);
	*result = set_simcard_src(chn, src);
		
	zsys_debug("ns__gsmio_set_simcard_src[sock:%d][chn:%d]: result = %d", soap->socket, chn, *result);
	return *result;
}

//gsoap ns service method: get gsm simcard status
int ns__gsmio_get_simcard_src(struct soap *soap, int chn, ns__gsmio_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		result->result= SOAP_ERR;
		zsys_error("ns__gsmio_get_simcard_src: args invalid");
		return SOAP_OK;
	}

	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsmio_get_simcard_src: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}

	get_simcard_src(chn, result);
	zsys_debug("ns__gsmio_get_simcard_src[sock:%d][chn:%d]: result->result = %d, result->value[%s]", soap->socket, chn, result->result, result->value);
	return SOAP_OK;
}

//gsoap ns service method: gsm module power on
int ns__gsmio_pwr_on(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsmio_pwr_on: args invalid");
		return SOAP_OK;
	}
	if (!chn_exist(chn) && chn != -1)
	{
		*result = HDL_MCU_ERR;
		zsys_error("ns__gsmio_pwr_on[sock:%d][chn:%d]: chn invalid", soap->socket, chn);
		return SOAP_OK;
	}
	*result = set_module_power_on(chn, &g_module_info);
	zsys_debug("ns__gsmio_pwr_on: chn = %d, result = %d", chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: gsm module power off
int ns__gsmio_pwr_off(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsmio_pwr_off: args invalid");
		return SOAP_OK;
	}
	if (!chn_exist(chn) && chn != -1)
	{
		*result = HDL_MCU_ERR;
		zsys_error("ns__gsmio_pwr_off[sock:%d][chn:%d]: chn invalid", soap->socket, chn);
		return SOAP_OK;
	}
	*result = set_module_power_off(chn);
	zsys_debug("ns__gsmio_pwr_off: chn = %d, result = %d", chn, *result);
	return SOAP_OK;
}

// gsoap ns service method: get gsm module power status
int ns__gsmio_get_pwr_status(struct soap *soap, int chn, ns__gsmio_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn >= MAX_CHN)
	{
		result->result = SOAP_ERR;
		zsys_error("ns__gsmio_get_pwr_status: args invalid");
		return SOAP_OK;
	}
	if (!chn_exist(chn) && chn != -1)
	{
		result->result = HDL_MCU_ERR;
		zsys_error("ns__gsmio_get_pwr_status[sock:%d][chn:%d]: chn invalid", soap->socket, chn);
		return SOAP_OK;
	}
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsmio_get_pwr_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	get_module_power_status(chn, result);
	zsys_debug("ns__gsmio_get_pwr_status[sock:%d][chn:%d]: result->result = %d, result->value[%s]", soap->socket, chn, result->result, result->value);
	return SOAP_OK;
}

//gsoap ns service method: gsm module on
int ns__gsmio_pwrkey_on(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsmio_pwrkey_on: args invalid");
		return SOAP_OK;
	}
	if (!chn_exist(chn) && chn != -1)
	{
		*result = HDL_MCU_ERR;
		zsys_error("ns__gsmio_pwrkey_on[sock:%d][chn:%d]: chn invalid", soap->socket, chn);
		return SOAP_OK;
	}
	*result = set_module_on(chn, &g_module_info);
	zsys_debug("ns__gsmio_pwrkey_on: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: gsm module off
int ns__gsmio_pwrkey_off(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsmio_pwrkey_off: args invalid");
		return SOAP_OK;
	}
	if (!chn_exist(chn) && chn != -1)
	{
		*result = HDL_MCU_ERR;
		zsys_error("ns__gsmio_pwrkey_off[sock:%d][chn:%d]: chn invalid", soap->socket, chn);
		return SOAP_OK;
	}
	*result = set_module_off(chn);
	zsys_debug("ns__gsmio_pwrkey_off: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: gsm module emerg off
int ns__gsmio_emerg_off(struct soap *soap, int chn, int *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		*result = SOAP_ERR;
		zsys_error("ns__gsmio_emerg_off: args invalid");
		return SOAP_OK;
	}
	if (!chn_exist(chn) && chn != -1)
	{
		*result = HDL_MCU_ERR;
		zsys_error("ns__gsmio_emerg_off[sock:%d][chn:%d]: chn invalid", soap->socket, chn);
		return SOAP_OK;
	}
	*result = set_module_emerg_off(chn);
	zsys_debug("ns__gsmio_emerg_off: chn = %d, result = 0x%x", chn, *result);
	return SOAP_OK;
}

//gsoap ns service method: get gsm module status
int ns__gsmio_get_pwrkey_status(struct soap *soap, int chn, ns__gsmio_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		result->result = SOAP_ERR;
		zsys_error("ns__gsmio_get_pwrkey_status: args invalid");
		return SOAP_OK;
	}
	if (!chn_exist(chn) && chn != -1)
	{
		result->result = HDL_MCU_ERR;
		zsys_error("ns__gsmio_get_pwrkey_status[sock:%d][chn:%d]: chn invalid", soap->socket, chn);
		return SOAP_OK;
	}
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsmio_get_pwrkey_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	get_module_status(chn, result);
	zsys_debug("ns__gsmio_get_pwrkey_status: chn = %d, result->result = %d, result->value = [%s]", chn, result->result, result->value);
	return SOAP_OK;
}

//gsoap ns service method: get gsm simcard insert status
int ns__gsmio_get_simcard_insert_status(struct soap *soap, int chn, ns__gsmio_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN)
	{
		result->result = SOAP_ERR;
		zsys_error("ns__gsmio_get_simcard_insert_status: args invalid");
		return SOAP_OK;
	}
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsmio_get_simcard_insert_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	get_simcard_insert_status(chn, result);
	zsys_debug("ns__gsmio_get_simcard_insert_status: chn = %d, result->result = %d, result->value = [%s]", chn, result->result, result->value);
	return SOAP_OK;
}

// gsoap ns service method: get gsm gsmboard insert status
int ns__gsmio_get_gsmboard_insert_status(struct soap *soap, int chn, ns__gsmio_rsp_t *result)
{(void)soap;
	if (chn < -1 || chn > MAX_CHN/2)
	{
		result->result = SOAP_ERR;
		zsys_error("ns__gsmio_get_gsmboard_insert_status: args invalid");
		return SOAP_OK;
	}
	if (result->value == NULL)
	{
		result->value = (char *)soap_malloc(soap, SOAP_TMPLEN);
		if (result->value == NULL)
		{
			result->result = SOAP_ERR;
			zsys_error("ns__gsmio_get_gsmboard_insert_status: SOAP_MALLOC fail");
			return SOAP_OK;
		}
		memset((char *)result->value, 0, SOAP_TMPLEN);
	}
	get_module_insert_status(chn, result);
	zsys_debug("ns__gsmio_get_gsmboard_insert_status: chn = %d, result->result = %d, result->value = [%s]", chn, result->result, result->value);
	return SOAP_OK;
}

//gsoap ns service method: get mcu version
int ns__gsmio_get_mcu_version(struct soap *soap, char **result)
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
	
	zsys_debug("ns__gsmio_get_mcu_version:  result[len:%d] = [%s]", strlen(*result), *result);
	return SOAP_OK;
}

//gsoap ns service method: get mcu help
int ns__gsmio_get_mcu_help(struct soap *soap, char **result)
{(void)soap;
	char help[128] = {0};
	pthread_mutex_lock(&mcu_serial[0].lock);
	get_mcu_help(mcu_serial[0].fd, help, sizeof(help));
	pthread_mutex_unlock(&mcu_serial[0].lock);

	*result = (char*)soap_malloc(soap, strlen(help)+1);
	memset(*result, 0, strlen(help)+1);
	memcpy(*result, help, strlen(help));
	
	zsys_debug("ns__gsmio_get_mcu_help:  result[len:%d] = [%s]", strlen(*result), *result);
	return SOAP_OK;
}

int get_mcu_version(int fd, char *buf, int len)
{
	int ret = 0;

	ret = serial_wr_mcu(fd, (uint8 *)GET_MCU_VERSION, (uint32)strlen(GET_MCU_VERSION), (uint8 *)buf, (uint32)len, SERIAL_READ_TIMEOUT);
	if (ret < 0)
		return -1;
	return 0;
}

int get_mcu_help(int fd, char *buf, int len)
{
	int ret = 0;

	ret = serial_wr_mcu(fd, (uint8 *)GET_MCU_HELP, (uint32)strlen(GET_MCU_HELP), (uint8 *)buf, (uint32)len, SERIAL_READ_TIMEOUT);
	if (ret < 0)
		return -1;
	return 0;
}

// status: 1:enbale;0:disable
int set_simcard_src(int chn, int status)
{
	int board = 0;
	int reg = -1;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int value = 0;
	
	if (chn == -1)
	{
		value = (status == 0) ? VAL_SIMCARD_DISABLE_ALL : VAL_SIMCARD_ENABLE_ALL;

		// lock regs
		//zsys_debug("set_simcard_src: lock begin ...");
		for (i = 0; i < MAX_MCU; i++)
		{
			for (j = 0; j < max_reg_mcu_simcard_enable; j++)
			{
				// 加锁该寄存器所有通道
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					pthread_mutex_lock(&regs[i][regs_simcard_enable[j].reg].reg_bit[m].lock);
					//zsys_debug("set_simcard_src: pthread_mutex_lock(&regs[%d][regs_simcard_enable[%d].reg].reg_bit[%d].lock)", i, j, m);
				}
				// 设置该寄存器
				ret = write_to_mcu(i, regs_simcard_enable[j].reg, ALL_BITS, value);
				usleep(20*1000);
				// 更新该寄存器所有通道状态值, 并解锁
				for (m = 0; m < MAX_CHN_PER_REG; m++)
				{
					regs[i][regs_simcard_enable[j].reg].reg_bit[m].stat = status;
					pthread_mutex_unlock(&regs[i][regs_simcard_enable[j].reg].reg_bit[m].lock);
					//zsys_debug("set_simcard_src: pthread_mutex_unlock(&regs[%d][regs_simcard_enable[%d].reg].reg_bit[%d].lock)", i, j, m);
				}
			}
		}
	}
	else
	{
		//value = (status == 0) ? VAL_SIMCARD_DISABLE : VAL_SIMCARD_ENABLE;
		// 获取该通道所属mcu
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_simcard_enable);
		
		pthread_mutex_lock(&regs[board][regs_simcard_enable[reg].reg].reg_bit[bit].lock);
		//zsys_debug("set_simcard_src: pthread_mutex_lock(&regs[%d][regs_simcard_enable[%d].reg].reg_bit[%d].lock)", board, reg, _chn);
		ret = write_to_mcu(board, regs_simcard_enable[reg].reg, bit, status);
		regs[board][regs_simcard_enable[reg].reg].reg_bit[bit].stat = status;
		pthread_mutex_unlock(&regs[board][regs_simcard_enable[reg].reg].reg_bit[bit].lock);
		//zsys_debug("set_simcard_src: pthread_mutex_unlock(&regs[%d][regs_simcard_enable[%d].reg].reg_bit[%d].lock)", board, reg, _chn);
		if (ret != 0)
		{
			zsys_error("set_simcard_src: write_to_mcu error(%d:%s)", errno, strerror(errno));
			return -1;
		}
	}
	//zsys_debug("set_simcard_src: set chn(%d) with status(%x) succ", chn, status);
	return 0;
}


int get_simcard_src(int chn, ns__gsmio_rsp_t *result)
{
	int board = 0;
	int reg = -1;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	unsigned char value = 0;
	int ret = 0;
	result->result = 0;
	result->cnt = 0;
	
	if (chn == -1)
	{
		// lock regs
		for (i = 0; i < MAX_MCU; i++)
		{
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
				if (ret != HDL_OK)
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
				//zsys_debug("get_simcard_src: board[%d], reg[%d], result->cnt = [%d], result->value[%d] = [%x]", i, reg, result->cnt, (i*max_reg_mcu_simcard_enable+m), result->value[(i*max_reg_mcu_simcard_enable+m)]);
				
				result->cnt++;
			}
		}
	}
	else
	{
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_simcard_enable);

		pthread_mutex_lock(&regs[board][regs_simcard_enable[reg].reg].reg_bit[bit].lock);
		ret = read_from_mcu_seri(board, regs_simcard_enable[reg].reg, 1, result->value);
		if (ret == HDL_OK)
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << bit)) >> bit; 
			sprintf(result->value, "%x", value);
			regs[board][regs_simcard_enable[reg].reg].reg_bit[bit].stat = value;
		}
		else
		{
			strcpy(result->value, "0");
		}
		result->result = ret;
		result->cnt = 1;
		pthread_mutex_unlock(&regs[board][regs_simcard_enable[reg].reg].reg_bit[bit].lock);
	}
	//zsys_debug("get_simcard_src: chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
}

int action_handle(int board, int reg, int bit, char *action)
{
	char type[64] = {0};
	char value[64] = {0};
    char *p = NULL;
	ACTION_FUNC func = NULL;
	
	p = strstr(action, "_");
	if (p == NULL)
        return -1;
    memcpy(type, action, p-action);
    memcpy(value, p+1, &action[strlen(action)-1]-p);
	func = find_action_func(type);
	if (func != NULL)
	{
		return func(board, reg, bit, value);
	}
	
	return HDL_ERR;
}

/*
	设置所有通道上电，如果有个别通道无法上电(如没有顶板，没插模块)，返回值不做体现
 */
int set_module_power_on(int chn, module_info_t *module_info)
{
	int board = 0;
	int reg = -1;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	
	
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_power_on: lock begin ...");
		for (i = 0; i < module_info->cnt; i++)
		{
			// 获取通道号，根据通道号转换board,reg,chn，然后写入
			// lock
			for (j = 0; j < module_info->module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(module_info->module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_vbat);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_lock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
			for (m = 0; m < module_info->module_conf[i].pwr_on.cnt; m++) // 上电操作步骤数量
			{
				for (j = 0; j < module_info->module_conf[i].chn_info.cnt; j++) // 模块数量
				{
					get_board_reg_bit_by_chn(module_info->module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_vbat);
					if (strlen(module_info->module_conf[i].pwr_on.action[m]) > 0) // 操作配置是否存在
					{
						ret = action_handle(board, regs_module_vbat[reg].reg, bit, module_info->module_conf[i].pwr_on.action[m]);
						if (ret == HDL_MCU_ERR)
						{
							continue;
						}
						if (ret != HDL_OK)
						{
							zsys_error("set_module_power_on[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
								board, reg, bit, module_info->module_conf[i].pwr_on.action[m], errno, strerror(errno));
						}
						else
							regs[board][regs_module_vbat[reg].reg].reg_bit[bit].stat = 1;
						
						if (strstr(g_module_info.module_conf[i].pwr_on.action[m], "sleep") != NULL) // 休眠1次即可
							break;
						else
							usleep(20*1000);
					}
				}
			}
			// unlock
			for (j = 0; j < module_info->module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(module_info->module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_vbat);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
		}
	}
	else
	{
		module_conf_t * pconf = get_moudle_conf_by_chn(chn);
		if (pconf == NULL)
		{
			return -1;
		}
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_module_vbat);

		pthread_mutex_lock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
		//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
		for (i = 0; i < pconf->pwr_on.cnt; i++) // 
		{
			if (strlen(pconf->pwr_on.action[i]) > 0) // 操作配置是否存在
			{
				ret = action_handle(board, regs_module_vbat[reg].reg, bit, pconf->pwr_on.action[i]);
				if (ret != HDL_OK)
				{
					zsys_error("set_module_power_on[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
						board, reg, bit, pconf->pwr_on.action[i], errno, strerror(errno));
					pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
					return ret;
				}
				else
					regs[board][regs_module_vbat[reg].reg].reg_bit[bit].stat = 1;
			}
		}
		pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
		//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
	}
	//zsys_debug("set_module_power_on: set chn(%d) with status(%x) succ", chn, status);
	return HDL_OK;
}

int set_module_power_off(int chn)
{
	int board = 0;
	int reg = -1;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	int time_sleep = 0;

	// 模块关机，记录并获取并计算等待时间，休眠
	// 然后进行断电操作

	// 如果已处于关机状态则不会重复关机
	if (g_mcu_options.pwr_off_force != 1)
	{
		set_module_off(chn);

		// 等待模块关机完成
		time_sleep = time(NULL) - get_module_off_last_time(chn);
		time_sleep = (time_sleep < 0) ? 0 : time_sleep;
		time_sleep = (time_sleep > 12) ? 0 : (12 - time_sleep);
		sleep(time_sleep);
	}
	if (chn == -1)
	{
		// lock regs
		//zsys_debug("set_module_power_on: lock begin ...");
		for (i = 0; i < g_module_info.cnt; i++)
		{
			// 获取通道号，根据通道号转换board,reg,chn，然后写入
			// lock
			for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_vbat);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_lock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
			for (m = 0; m < g_module_info.module_conf[i].pwr_off.cnt; m++) // 上电操作步骤数量
			{
				for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++) // 模块数量
				{
					get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_vbat);
					if (strlen(g_module_info.module_conf[i].pwr_off.action[m]) > 0) // 操作配置是否存在
					{
						ret = action_handle(board, regs_module_vbat[reg].reg, bit, g_module_info.module_conf[i].pwr_off.action[m]);
						if (ret != 0)
						{
							zsys_error("set_module_power_on[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
								board, reg, bit, g_module_info.module_conf[i].pwr_off.action[m], errno, strerror(errno));
						}
						else
							regs[board][regs_module_vbat[reg].reg].reg_bit[bit].stat = 0;
						
						if (strstr(g_module_info.module_conf[i].pwr_off.action[m], "sleep") != NULL) // 休眠1次即可
							break;
						else
							usleep(20*1000);
					}
				}
			}
			// unlock
			for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_vbat);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
		}
	}
	else
	{
		module_conf_t * pconf = get_moudle_conf_by_chn(chn);
		if (pconf == NULL)
		{
			return -1;
		}
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_module_vbat);

		pthread_mutex_lock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
		//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
		for (i = 0; i < pconf->pwr_off.cnt; i++) // 
		{
			if (strlen(pconf->pwr_off.action[i]) > 0) // 操作配置是否存在
			{
				ret = action_handle(board, regs_module_vbat[reg].reg, bit, pconf->pwr_off.action[i]);
				if (ret != HDL_OK)
				{
					zsys_error("set_module_power_on[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
						board, reg, bit, pconf->pwr_off.action[i], errno, strerror(errno));
					pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
					return ret;
				}
				else
					regs[board][regs_module_vbat[reg].reg].reg_bit[bit].stat = 1;
			}
		}
		pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
		//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
	}
	//zsys_debug("set_module_power_off: set chn(%d) with status(%x) succ", chn, status);
	return HDL_OK;
}

int get_module_power_status(int chn, ns__gsmio_rsp_t *result)
{
	int bit = 0;
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
		}
		//zsys_debug("get_module_power_status: 1 chn = [%d], result->value = [%s]", chn, result->value);
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
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_module_vbat);

		pthread_mutex_lock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
		ret = read_from_mcu_seri(board, regs_module_vbat[reg].reg, 1, result->value);
		if (ret == HDL_OK)
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << bit)) >> bit;
			value = (value == VAL_MODULE_POWER_ON) ? 1 : 0;
			sprintf(result->value, "%x", value);
			regs[board][regs_module_vbat[reg].reg].reg_bit[bit].stat = value;
		}
		else
		{
			strcpy(result->value, "0");
		}
		result->result = ret;
		result->cnt = 1;
		pthread_mutex_unlock(&regs[board][regs_module_vbat[reg].reg].reg_bit[bit].lock);
	}
	//zsys_debug("get_module_power_status: chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
}



// 先判断通道开关机状态，如果状态为开机，则不做操作，如果为关机，则执行开机操作
// 开机操作：拉低2s以上再拉高
int set_module_on(int chn, module_info_t *module_info)
{
	int board = 0;
	int ret = 0;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	time_t last = 0;
	time_t curr = 0;
	int reg = -1;
	char res[SOAP_TMPLEN] = {0};
	char buf[4] = {0};
	unsigned char bits = 0;


	// 查询模块开关机状态
	ns__gsmio_rsp_t rsp;
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
		for (i = 0; i < module_info->cnt; i++)
		{
			// 获取通道号，根据通道号转换board,reg,chn，然后写入
			// lock
			for (j = 0; j < module_info->module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(module_info->module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_pwrkey);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
			for (m = 0; m < module_info->module_conf[i].pwrkey_on.cnt; m++) // 上电操作步骤数量
			{
				for (j = 0; j < module_info->module_conf[i].chn_info.cnt; j++) // 模块数量
				{
					// 检测模块状态，如果当前为开机状态，则不进行操作
					if (get_status_by_chn(res,  module_info->module_conf[i].chn_info.port_map[j].chn) == VAL_MODULE_ON)
					{
						continue;
					}
					get_board_reg_bit_by_chn(module_info->module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_pwrkey);
					if (strlen(module_info->module_conf[i].pwrkey_on.action[m]) > 0) // 操作配置是否存在
					{
						ret = action_handle(board, regs_module_pwrkey[reg].reg, bit, module_info->module_conf[i].pwrkey_on.action[m]);
						if (ret != HDL_OK)
						{
							zsys_error("set_module_power_on[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
								board, reg, bit, module_info->module_conf[i].pwrkey_on.action[m], errno, strerror(errno));
						}
						else
							regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].stat = 0;
						
						if (strstr(g_module_info.module_conf[i].pwrkey_on.action[m], "sleep") != NULL) // 休眠1次即可
							break;
						else
							usleep(20*1000);
					}
				}
			}
			// unlock
			for (j = 0; j < module_info->module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(module_info->module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_pwrkey);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
		}
	}
	else
	{
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_module_pwrkey);
		if (mcu_serial[board].fd < 0)
		{
			free(rsp.value);
			return HDL_MCU_ERR;;
		}

		//memcpy(buf, res+board*max_reg_mcu_module_pwrkey*2+reg*2, 2);
		memcpy(buf, res, 2);
		bits = strtol(buf, NULL, 0x10);
		if (bits == VAL_MODULE_OFF)
		{
			module_conf_t * pconf = get_moudle_conf_by_chn(chn);
			if (pconf == NULL)
			{
				free(rsp.value);
				return -1;
			}
			pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
			//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			for (i = 0; i < pconf->pwrkey_on.cnt; i++) // 
			{
				if (strlen(pconf->pwrkey_on.action[i]) > 0) // 操作配置是否存在
				{
					ret = action_handle(board, regs_module_pwrkey[reg].reg, bit, pconf->pwrkey_on.action[i]);
					if (ret != 0)
					{
						zsys_error("set_module_on[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
							board, reg, bit, pconf->pwrkey_on.action[i], errno, strerror(errno));
						pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
						free(rsp.value);
						return -1;
					}
					else
						regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].stat = 1;
				}
			}
			// 解锁
			pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
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
	return 0;
}

// 先判断通道开关机状态，如果状态为开机，则不做操作，如果为关机，则执行开机操作
// 开机操作：拉低0.6~1s再拉高
int set_module_off(int chn)
{
	int board = 0;
	int reg = -1;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	time_t last = 0;
	time_t curr = 0;
	char res[SOAP_TMPLEN] = {0};
	char buf[4] = {0};
	unsigned char bits = 0;
	int sleep_time = 0;


	// 查询模块开关机状态
	ns__gsmio_rsp_t rsp;
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
		for (i = 0; i < g_module_info.cnt; i++)
		{
			for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_pwrkey);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
			for (m = 0; m < g_module_info.module_conf[i].pwrkey_off.cnt; m++) // 上电操作步骤数量
			{
				for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++) // 模块数量
				{
					// 检测模块状态，如果当前为关机状态，则不进行操作
					if (get_status_by_chn(res,  g_module_info.module_conf[i].chn_info.port_map[j].chn) == VAL_MODULE_OFF)
					{
						continue;
					}
					get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_pwrkey);
					if (strlen(g_module_info.module_conf[i].pwrkey_off.action[m]) > 0) // 操作配置是否存在
					{
						ret = action_handle(board, regs_module_pwrkey[reg].reg, bit, g_module_info.module_conf[i].pwrkey_off.action[m]);
						if (ret != 0)
						{
							zsys_error("set_module_off[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
								board, reg, bit, g_module_info.module_conf[i].pwrkey_off.action[m], errno, strerror(errno));
						}
						else
						{
							regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].stat = VAL_MODULE_OFF;
							time(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].last);
						}
						
						if (strstr(g_module_info.module_conf[i].pwrkey_off.action[m], "sleep") != NULL) // 休眠1次即可
							break;
						else
							usleep(20*1000);
					}
				}
			}
			// unlock
			for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_pwrkey);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
		}
	}
	else
	{
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_module_pwrkey);
		if (mcu_serial[board].fd < 0)
		{
			free(rsp.value);
			return HDL_MCU_ERR;
		}

		//memcpy(buf, res+board*max_reg_mcu_module_pwrkey*2+reg*2, 2);
		memcpy(buf, res, 2);
		bits = strtol(buf, NULL, 0x10);
		if (bits == VAL_MODULE_ON)
		{
			module_conf_t * pconf = get_moudle_conf_by_chn(chn);
			if (pconf == NULL)
			{
				free(rsp.value);
				return -1;
			}
			pthread_mutex_lock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
			//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			for (i = 0; i < pconf->pwrkey_off.cnt; i++) // 
			{
				if (strlen(pconf->pwrkey_off.action[i]) > 0) // 操作配置是否存在
				{
					ret = action_handle(board, regs_module_pwrkey[reg].reg, bit, pconf->pwrkey_off.action[i]);
					if (ret != 0)
					{
						zsys_error("set_module_off[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
							board, reg, bit, pconf->pwrkey_off.action[i], errno, strerror(errno));
						pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
						return -1;
					}
					else
					{
						regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].stat = 1;
						time(&(regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].last));
					}
				}
			}
			// 解锁
			pthread_mutex_unlock(&regs[board][regs_module_pwrkey[reg].reg].reg_bit[bit].lock);
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
	usleep((sleep_time == 0) ? 0 : 500*1000);
	free(rsp.value);
	//zsys_debug("set_module_off: set chn(%d) succ", chn);
	return 0;
}

int set_module_emerg_off(int chn)
{
	int board = 0;
	int reg = -1;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int ret = 0;
	
	if (chn == -1)
	{
		for (i = 0; i < g_module_info.cnt; i++)
		{
			for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_emerg_off);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_lock(&regs[board][regs_module_emerg_off[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
			for (m = 0; m < g_module_info.module_conf[i].emerg_off.cnt; m++) // 上电操作步骤数量
			{
				for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++) // 模块数量
				{
					get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_emerg_off);
					if (strlen(g_module_info.module_conf[i].emerg_off.action[m]) > 0) // 操作配置是否存在
					{
						ret = action_handle(board, regs_module_emerg_off[reg].reg, bit, g_module_info.module_conf[i].emerg_off.action[m]);
						if (ret != 0)
						{
							zsys_error("set_emerg_off[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
								board, reg, bit, g_module_info.module_conf[i].emerg_off.action[m], errno, strerror(errno));
						}
						else
							regs[board][regs_module_emerg_off[reg].reg].reg_bit[bit].stat = 1;
						
						if (strstr(g_module_info.module_conf[i].emerg_off.action[m], "sleep") != NULL) // 休眠1次即可
							break;
						else
							usleep(20*1000);
					}
				}
			}
			// unlock
			for (j = 0; j < g_module_info.module_conf[i].chn_info.cnt; j++)
			{
				get_board_reg_bit_by_chn(g_module_info.module_conf[i].chn_info.port_map[j].chn, &board, &reg, &bit, max_chn_mcu_module_emerg_off);
				if (mcu_serial[board].fd < 0)
				{
					continue;
				}
				pthread_mutex_unlock(&regs[board][regs_module_emerg_off[reg].reg].reg_bit[bit].lock);
				//zsys_debug("set_module_power_on: pthread_mutex_unlock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
			}
		}
	}
	else
	{
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_module_emerg_off);
		if (mcu_serial[board].fd < 0)
		{
			return HDL_MCU_ERR;
		}

		module_conf_t * pconf = get_moudle_conf_by_chn(chn);
		if (pconf == NULL)
		{
			return -1;
		}
		pthread_mutex_lock(&regs[board][regs_module_emerg_off[reg].reg].reg_bit[bit].lock);
		//zsys_debug("set_module_power_on: pthread_mutex_lock(&regs[%d][regs_module_vbat[%d].reg].reg_bit[%d].lock)", board, reg, bit);
		for (i = 0; i < pconf->emerg_off.cnt; i++) // 
		{
			if (strlen(pconf->emerg_off.action[i]) > 0) // 操作配置是否存在
			{
				ret = action_handle(board, regs_module_emerg_off[reg].reg, bit, pconf->emerg_off.action[i]);
				if (ret != 0)
				{
					zsys_error("set_emerg_off[board:%d,reg:%d,bit:%d]: action[%s] handle error(%d:%s)", 
						board, reg, bit, pconf->emerg_off.action[i], errno, strerror(errno));
					pthread_mutex_unlock(&regs[board][regs_module_emerg_off[reg].reg].reg_bit[bit].lock);
					return -1;
				}
				else
					regs[board][regs_module_emerg_off[reg].reg].reg_bit[bit].stat = 1;
			}
		}
		// 解锁
		pthread_mutex_unlock(&regs[board][regs_module_emerg_off[reg].reg].reg_bit[bit].lock);
	}
	
	// 紧急关机操作完成后休眠2s
	sleep(2);
	
	//zsys_debug("set_module_emerg_off: set chn(%d) succ", chn);
	return 0;
}

int get_module_status(int chn, ns__gsmio_rsp_t *result)
{
	int board = 0;
	int reg = 0;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
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
		//zsys_debug("get_module_status: 1 chn = [%d], result->value = [%s]", chn, result->value);
	}
	else
	{
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_module_pwr_det);

		pthread_mutex_lock(&regs[board][regs_module_pwr_det[reg].reg].reg_bit[bit].lock);
		ret = read_from_mcu_seri(board, regs_module_pwr_det[reg].reg, 1, result->value);
		if (ret == HDL_OK)
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << bit)) >> bit;
			value = (value == VAL_MODULE_ON) ? 1 : 0;
			sprintf(result->value, "%x", value);
			regs[board][regs_module_pwr_det[reg].reg].reg_bit[bit].stat = value;
		}
		else
		{
			strcpy(result->value, "0");
		}
		result->result = ret;
		result->cnt = 1;
		pthread_mutex_unlock(&regs[board][regs_module_pwr_det[reg].reg].reg_bit[bit].lock);
	}
	//zsys_debug("get_module_status: 2 chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
}

int get_simcard_insert_status(int chn, ns__gsmio_rsp_t *result)
{
	int board = 0;
	int reg = 0;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
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
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_simcard_insert_det);

		pthread_mutex_lock(&regs[board][regs_simcard_insert_det[reg].reg].reg_bit[bit].lock);
		ret = read_from_mcu_seri(board, regs_simcard_insert_det[reg].reg, 1, result->value);
		if (ret == HDL_OK)
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << bit)) >> bit;
			value = (value == VAL_SIMCARD_INSERT) ? 1 : 0;
			sprintf(result->value, "%x", value);
			regs[board][regs_simcard_insert_det[reg].reg].reg_bit[bit].stat = value;
		}
		else
		{
			strcpy(result->value, "0");
		}
		result->result = ret;
		result->cnt = 1;
		//zsys_debug("get_simcard_insert_status: 1 chn = [%d], board = [%d], reg = [%d], _chn = [%d], result->value = [%s]", chn, board, regs_simcard_insert_det[reg].reg, _chn, result->value);
		pthread_mutex_unlock(&regs[board][regs_simcard_insert_det[reg].reg].reg_bit[bit].lock);
	}
	//zsys_debug("get_simcard_insert_status: 2 chn = [%d], result->value = [%s]", chn, result->value);
	
	return 0;
}

int get_module_insert_status(int chn, ns__gsmio_rsp_t *result)
{
	int board = 0;
	int reg = 0;
	int bit = 0;
	int i = 0;
	int j = 0;
	int m = 0;
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
		get_board_reg_bit_by_chn(chn, &board, &reg, &bit, max_chn_mcu_module_insert_det);

		pthread_mutex_lock(&regs[board][regs_module_insert_det[reg].reg].reg_bit[bit].lock);
		ret = read_from_mcu_seri(board, regs_module_insert_det[reg].reg, 1, result->value);
		if (ret == HDL_OK)
		{
			value = strtol(result->value, NULL, 0x10);
			value = (value & (1 << bit)) >> bit;
			value = (value == VAL_MODULE_INSERT) ? 1 : 0;
			sprintf(result->value, "%x", value);
			regs[board][regs_module_insert_det[reg].reg].reg_bit[bit].stat = value;
		}
		else
		{
			strcpy(result->value, "0");
		}
		result->result = HDL_OK;
		result->cnt = 1;
		pthread_mutex_unlock(&regs[board][regs_module_insert_det[reg].reg].reg_bit[bit].lock);
	}
	//zsys_debug("get_module_insert_status: 2 chn = [%d], result->value = [%s]", chn, result->value);
	
	return result->result;
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
		return HDL_MCU_ERR;
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
		return HDL_ERR;
	}
	memcpy(result, buf_rsp, sizeof(char)*2);
	zsys_error("read_from_mcu: result = [%s]", result);
	
	return HDL_OK;
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
		return HDL_MCU_ERR;
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
		return HDL_ERR;
	}
	if (strlen((char *)buf_rsp) < 2)
	{
		return HDL_ERR;
	}
	for (i = 0; i < nbr; i++)
	{
		memcpy(result+ i*2, buf_rsp+i*3, 2);
	}
	//zsys_error("read_from_mcu_seri:board[%d], reg[%d], result = [%s]", board, reg, result);
	
	return HDL_OK;
}
// 一次不可以写连续多个寄存器
// 可以写寄存器的指定位
int write_to_mcu(int board, int reg, int bit, int val)
{
	int ret = 0;
	unsigned char buf_req[MAX_REQ_LEN] = {0};
	
	if (mcu_serial[board].fd < 0)
	{
		return HDL_MCU_ERR;
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
		return HDL_ERR;
	}
	
	return HDL_OK;
}

