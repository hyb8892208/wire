#ifndef _GWAPP_COMMON_H_
#define _GWAPP_COMMON_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h> 
#include <errno.h>
#include <sys/types.h>

#include <exception>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <signal.h> 

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>
#include <sys/mman.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>

#include <sys/ipc.h>  
#include <sys/sem.h>   
#include <sys/shm.h>

#include <locale.h>

#include "FCgiIO.h"
#include "hiredis.h"

#define __GSM_SUM__			44
#define __BRD_SUM__			1
#define __GSM_HEAD__			"gsm-"
#define __BRD_HEAD__			"board-"
#define __PORT_SUM__			44 //__BRD_SUM__ * __GSM_SUM__
#define GW_CLUSTER_SOCKET_PORT	8000

#define MAX_LEN_USERNAME		256
#define MAX_LEN_PASSWORD		256
#define MAX_LEN_BUFFER			4096
#define MAX_LEN_LINE			1024
#define MAX_LEN_COMMAND		1024
#define MAX_LEN_NAME			256
#define MAX_LEN_IP			32
#define MAX_LEN_PHONENUMBER		32
#define MAX_LEN_SMS_MESSAGE		256
#define MAX_LEN_ACTIONID		256

#define EXEC_TIMEOUT 			60 //execut cmmoand timeout.

#define ASTMANPROXY_KEYWORD_AUTH		"Authentication accepted"
#define ASTMANPROXY_HEADER_ACTIONID		"ActionID"
#define ASTMANPROXY_HEADER_SERVER		"Server"
#define ASTMANPROXY_HEADER_RESPONSE		"Response"
#define ASTMANPROXY_HEADER_PING			"Ping"
#define ASTMANPROXY_HEADER_PONG			"Pong"
#define ASTMANPROXY_DEFAULT_USERNAME		"internalspecifyuser"
#define ASTMANPROXY_DEFAULT_PASSWORD		"xn60qvh9dqx1j6ekcj1"

#define SMS_TO_DELIM				','
#define SMS_PORT_DELIM				','
#define SMS_RESULT_CHECK_NORMAL			"SEND SMS TO PHONE"
#define SMS_RESULT_CHECK_CONCATENATED		"SEND CONCATENATED SMS TO PHONE"
#define SMS_RESULT_SUCCESS_KEYWORD		"SUCCESSFULLY"
#define SMS_RESULT_FAIL_KEYWORD			"FAILED"
#define SMS_REPORT_SUCCESS			"success"
#define SMS_REPORT_FAIL				"fail"
#define SMS_REPORT_SENDING			"sending"

#define USSD_REPORT_SENDING                   "sending"
#define USSD_REPORT_FAIL                          "fail"

#define SIM_STATUS_NOT_READY		"not ready"
#define SIM_STATUS_NOT_SELECT		"not select"

#define PORT_STATE_CHECKING_TIMEOUT 1
#define PORT_STATE_SENDING_TIMEOUT 10
#define PORT_STATE_READY_TIMEOUT 1

enum
{
	STATUS_SIM_OK,
	STATUS_SIM_UNWORK,
	STATUS_SIM_UNSELECTED,
	STATUS_OTHER
};

/************************************************
 *
 *        debug settings
 *
 ************************************************/
#define DEBUG_LEVEL0 (1<<0)
#define DEBUG_LEVEL1 (1<<1)
#define DEBUG_LEVEL2 (1<<2)
#define DEBUG_LEVEL3 (1<<3)
#define DEBUG_LEVEL4 (1<<4)
#define DEBUG_LEVEL5 (1<<5)

#define dlog(level,format,...) \
	do { \
		if(level & debug){ \
			time_t t = time(NULL); \
			struct timeval tv; \
			char debug_buffer[4096]; \
			char debug_tmp[256]; \
			gettimeofday(&tv, NULL); \
			strftime(debug_tmp, sizeof(debug_tmp), "%Y-%m-%d %H:%M:%S", localtime(&t)); \
			snprintf(debug_buffer, sizeof(debug_buffer), "[%s %6ld][PID:%6d][%6d %s %s] "format"", \
				debug_tmp, \
				tv.tv_usec, \
				(int)getpid()/*pthread_self()*/,\
				__LINE__, __FILE__, __FUNCTION__, \
				##__VA_ARGS__); \
			if(global_debug_logfile){ \
				file_append_contents(global_debug_logfile, debug_buffer, strlen(debug_buffer)); \
			} \
			if(global_debug_io){ \
				(*global_debug_io) << debug_buffer; \
				global_debug_io->flush(); \
			}\
		}\
	}while(0)



/************************************************
 *
 *	  struct and global variable
 *
 ************************************************/
union sem_union  
{  
	int val;  
	struct semid_ds *buf;  
	unsigned short *array;  
};

enum cluster_mode{
	CLUSTER_MASTER, // master
	CLUSTER_STANDALONE, // standalone
	CLUSTER_OTHER,	 //slave
};

enum sms_report_format{
	SMS_REPORT_JSON,
	SMS_REPORT_STRING,
	SMS_REPORT_NO,
};

enum sms_port_order{
	PORT_ORDER_ROUND,
	PORT_ORDER_RANDOM,
	PORT_ORDER_ASCEND,
};

enum sms_port_state{
	PORT_STATE_NOTREADY,
	PORT_STATE_CHECKING,
	PORT_STATE_READY,
	PORT_STATE_SENDING,
};

enum sms_flag{
	SMS_FLAG_SEND_SMS,
	SMS_FLAG_PORT_STATE,
};

enum ussd_result_format{
	USSD_REPORT_JSON,
	USSD_REPORT_STRING,
	USSD_REPORT_NO,
};

struct clt_config{
	enum cluster_mode mode;
	char password[MAX_LEN_NAME];
	char master_ip[MAX_LEN_IP];
	char slave_ips[__BRD_SUM__][MAX_LEN_IP];
};

struct amp_config{
	int port;
	char username[MAX_LEN_USERNAME];
	char password[MAX_LEN_PASSWORD];
};

struct amp_command{
	char command[MAX_LEN_LINE];
	char server[MAX_LEN_IP];
	char actionid[MAX_LEN_ACTIONID];
};

/***********************
  *
  *    http2sms struct 
  *
  **********************/
struct sms_args{
	const char *from;
	const char *to;
	const char *message;
	const char *order;
	const char *report;
	const char *timeout;
	const char *debug;
	const char *id;
       const char *url;
	const char *type;
};

struct sms_to_node{
	char port[64];		// from=xxx
	char phonenumber[64];	// to=xxx
	char time[64];		// send time
	char result[64];		// success or fail or sending
	struct sms_to_node *next;
};

struct sms_port_node{
	int index;
	int board;
	int span;
	int is_selected;			// for user selected
	int is_valid;		// if gateway is in cluster mode, some slave is invalid
	char server[MAX_LEN_IP];
	char actionid[64];
	int longsms_count; // for long sms send count
	int longsms_succ; // for long sms send count
	int longsms_flag; // for long sms send flag
	struct timeval timestamp; // for determine who using the port
	struct sms_to_node *sms_to_cur;	// the sms phonenumber current sending to
};

struct sms_message{
	char original[16*MAX_LEN_SMS_MESSAGE];
	char contents[16][MAX_LEN_SMS_MESSAGE]; // long SMS: max 16 messages
	int count; // for long sms message max index
};

struct http2sms_config{
	int enable; // 1=enable 0= disable
	//int use_web_server_user; // 1=yes 0=no
	int use_default_user; // 1=yes 0=no
	char username[MAX_LEN_USERNAME];
	char password[MAX_LEN_PASSWORD];
	int port_valid[__BRD_SUM__][__GSM_SUM__];
	int debug;
	int timeout_wait;
	int timeout_total;
	int timeout_gsm_send;
	int enable_cors; // add CORS switch
	char allow_access_origin_url[128]; // add parameter of allow_access_origin_url
	char usage_file[MAX_LEN_NAME];
	enum sms_port_order port_order;
	enum sms_report_format report_format;
};

struct sms2http_config{
	int get_smsstatus_enabe;
	char url_time[32];
	char url_message[32];
	char url_imsi[32];
	char url_status[32];
	char url_port[32];
	char url_num[32];
	char url_id[32];
};
struct http2sms_task{
	struct http2sms_config sms_cfg;
	struct sms_message sms_msg;
	struct sms_port_node sms_port[__PORT_SUM__];
	struct sms_to_node *sms_to_head;
	struct sms_to_node *sms_to_cur;
	int sms_to_rest; // phonenumbers rest
	int amp_write_count; // count astmanproxy socket writeable times
};
struct http2ussd_config{
	int enable;
	int total_channel;
	int use_default_user;
	char username[MAX_LEN_USERNAME];
	char  password[MAX_LEN_PASSWORD];
	int port_valid[__BRD_SUM__][__GSM_SUM__];
	int enable_cors; // add CORS switch
	char allow_access_origin_url[128]; // add parameter of allow_access_origin_url
	char usage_file[MAX_LEN_NAME];
	enum ussd_result_format result_format;
};

struct ussd_args{
	const char *port;
	const char *message;
	const char *timeout;
	const char *url;
	const char *type;
	const char *result;
	const char *id;
};

struct http2ussd_task{
	http2ussd_config ussd_cfg;
};

struct service_shm{
	int cur_sms_port;
	enum sms_port_state sms_port_state[__PORT_SUM__];
	struct timeval sms_port_state_timestamp[__PORT_SUM__];
};
struct http_service_info{
	char username[32];
	char password[32];
	char usage_file[MAX_LEN_NAME];
};

struct service_info{
	/* config settings */
	int port_sum;
	struct clt_config cluster_cfg;
	struct amp_config ami_cfg;
	struct http2sms_config sms_cfg;
	struct sms2http_config get_smsstatus_cfg;
	struct http2ussd_config ussd_cfg;
	/* ami */
	int amp_fd;
	int amp_timeout;
	char recv_buf[MAX_LEN_BUFFER];
	int recv_len;

	/* shm */
	/*int *sms_port_cur_shmp;
	int  sms_port_cur_shmid;
	int  sms_port_cur_semid;*/
	struct http_service_info http_info;
	struct service_shm *shm;
	int shmid;
	int semid;
};

extern int debug;
extern char *global_debug_logfile;
extern cgicc::FCgiIO *global_debug_io;
extern struct service_info global_service;

/************************************************
 *
 *        functions
 *
 ************************************************/

extern int lock_file(const char* path);
extern int unlock_file(int fd);

extern char *trim(char *str);
extern int random_int(int min, int max);

extern int file_size(char *file_path);
extern int file_get_contents(char *file_path, char *buffer, int len);
extern int file_put_contents(char *file_path, char *buffer, int len);
extern int file_append_contents(const char *file_path, char *buffer, int len);
extern int file_slim_contents(const char *file_path, int valid_start, int len);

extern int utf8_strlen(char *s);
extern int utf8_charsize(char *s, int utf8_pos);

extern int sem_init(key_t key);
extern int sem_setval(int sem_id, int init_value);
extern int sem_delete(int sem_id);
extern int sem_p(int sem_id, int nowait);
extern int sem_v(int sem_id, int nowait);
extern void *shm_init(key_t key, int *shm_id, int shm_size);
extern int shm_delete(int shm_id, void *shmp);

extern int exec_pipe(const char *cmd, char *outbuf, int outlen);
extern int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int sec);
extern int request_slave(const char* ip, char *data, int data_len, char *ret_val, int ret_len, int timeo);
extern int get_gsm_header(char *src, char *key, char *value, int len);

extern int init_clt_config(struct clt_config *cfg);
extern int init_ami_config(struct amp_config *cfg);
extern int init_sms_config(struct http2sms_config *cfg);
extern int init_ussd_config(struct http2ussd_config *cfg);
extern int init_get_smsstatus_config(struct sms2http_config *cfg);
extern int amp_get_header(char *src, char *key, char *value, int vallen);
extern int amp_action(struct service_info *service, struct amp_command *cmd, char *outbuf, int outlen);
extern int amp_ping(struct service_info *service);
extern int amp_login(struct service_info *service);
extern int amp_logoff(struct service_info *service);

extern int service_shm_lock(int nowait);
extern int service_shm_unlock(int nowait);
extern int get_usage_info(char *usage_file);
extern int handle_action_sendsms(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request,redisContext *c);
extern int get_chanstate_usage_info(const char *usage_file);
extern int handle_action_chan_state(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request, redisContext *c);
extern int get_ussd_usage_info(char *usage_file);
extern int handle_action_send_ussd(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request, redisContext *c);
extern int handle_action_get_smsreports(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request);
extern int handle_action_get_simstatus(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request);
extern int get_simstatus_usage_info(char *usage_file);
extern int handle_action_get_smsremain(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request, redisContext *c);
#endif
