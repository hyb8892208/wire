#ifndef SIM_QUERY_H
#define SIM_QUERY_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/file.h>

#define REDIS_HOST	"127.0.0.1"
#define REDIS_PORT	6379

#define REDIS_KEY_SIMQUERY_BALANCE_INFO		"simquery.collect.balance.info"
#define REDIS_KEY_SIMQUERY_PHONENUM_INFO	"simquery.collect.phonenum.info"

#define REDIS_KEY_SIMSTATUS					"app.asterisk.simstatus.channel"
#define REDIS_KEY_SIMBALANCE				"app.simquery.balance.channel"
#define REDIS_KEY_SIMPHONENUM				"app.simquery.phonenum.channel"

#define HW_INFO_FILE						"/tmp/hw_info.cfg"
#define CONF_FILE							"/etc/sim_query.conf"
#define STAT_CONF_FILE						"/etc/cfg/gw/sim_query.conf"

#define LOCK_FILE							"/tmp/lock/sim_query.lock"
#define LOG_FILE							"/tmp/log/sim_query.log"

#define USSD_TEXT_STRING                          "Text:"
#define	BALANCE_QUERY		(1<<2)
#define	PHONENUM_QUERY		(1<<6)

#define MAX_CHN				64

#define MAX_SLEEP_TIME		666
#define MAX_QUERY_TIME		60			// �ȴ����ų�ʱʱ��
#define MAX_QUERY_NUM         5                   // ����ѯ����

#define DEBUG				0
#define WARN				1
#define ERROR				2
#define NOTICE				3

#define RESET "\033[0m"
#define RED "\033[31m" /* Red */
#define GREEN "\033[32m" /* Green */
#define YELLOW "\033[33m" /* Yellow */
#define BLUE "\033[34m" /* Blue */
#define CYAN "\033[36m" /* Cyan */

#define debug(bits,...)		z_printf(bits,(__LINE__),__VA_ARGS__)


enum query_way
{
	UNKNOWN = 0,
	SMS ,
	CALL,
	USSD,
};

enum
{
	OFF,
	ON
};

enum
{
	RET_OK,
	RET_ERROR
};

enum sms_state
{
	SMS_IDLE		=0,
	SMS_NEED_SEND	=1,
	SMS_SENT		=2,
	SMS_RECEIVED	=3
};

typedef struct LOG_DEBUG_S
{
	int log_class;
	char prefix_bits[12];
	char color[24];
}LOG_DEBUG_T;


/********************************
query_typeֵ��Ӧ����

5 ���Ų�ѯ���
6 �绰��ѯ���
7 USSD��ѯ���
9 ���Ų�ѯ����
10 �绰��ѯ����
11 USSD��ѯ����
********************************/
typedef struct SMS_PARAM_S
{
	char dst_num[32];
	char recv_num[32];
	char send_msg[128];
	char match_key[32];
}SMS_PARAM_T;

typedef struct QUERY_INFO_S
{
	char query_balance_flag;		// 1 need query
	char query_phonenum_flag;
	char query_type;		//bit 0-1:balance query way bit 2:balance query  switch bit 4-5:phonenum query way bit 6:phonenum query switch
	//char query_type_bak;
	char bal_chan_stat;			// ����ѯע��״̬   0 ע���� 1 δע��
	char num_chan_stat;			// �����ѯע��״̬
	char registered_query;	//ע���Ϻ�ʼ��ѯ
	char point_char;		//С����
	char thousandth_char;	//ǧ��λ
	unsigned short interval;		//��ѯ���
	unsigned short call_counts_query;
	unsigned short call_query_counts;
	unsigned short query_phonenum_counts;//�����ѯ����ͳ�� 
	unsigned short channel;			//strart from 1
	sem_t sem;
	pthread_mutex_t flag_mtx;
	char ussd_res[256];
	struct SMS_PARAM_S balance_param;
	struct SMS_PARAM_S phone_num_param;
	struct QUERY_INFO_S* next;
}QUERY_INFO_T;

int SMSPhoneNumStringMatch(char *dst,char *key,char *context);

int SMSBanlanceStringMatch(char *dst,char *key,char *context,char point_char,char thousandth_char);
#endif
