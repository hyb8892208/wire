#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h> 
#include <errno.h>
#include "astman.h"


#define MAX_LEN_IP		32
#define MAX_LEN_NAME		128
#define MAX_LEN_USERNAME	128
#define MAX_LEN_SECRET		128
#define MAX_LEN_COMMAND		1024
#define MAX_LEN_BUFFER		4096
#define MAX_LEN_LINE		1024
#define MAX_LEN_PHONENUMBER	32


#define RETINT_SUCCESS		0
#define RETINT_FAILURE		-1

#define AMI_COMMAND_MAX		256
#define AMI_SUPER_USERNAME	"internalspecifyuser"
#define AMI_SUPER_SECTET	"2rujzdndyznbg7u6xju"
#define AMI_SUPER_PORT		5038

#define __GSM_SUM__		4
#define __BRD_SUM__		5
#define __BRD_HEAD__		"Board-"

#define SOCKET_PORT		8000

#define DEFAULT_AMI_RECONNECT_DELAY 30
#define DEFAULT_GSM_UP_KEYWORD	"Power on, Provisioned, Up"

//512K
#define DEFAULT_LOG_MAX_SIZE	(1024*512)


/************************************************
 *
 *         debug settings
 *
 ************************************************/
#define DEBUG_LEVEL0 (1<<0)
#define DEBUG_LEVEL1 (1<<1)
#define DEBUG_LEVEL2 (1<<2)
#define DEBUG_LEVEL3 (1<<3)
#define DEBUG_LEVEL4 (1<<4)
#define DEBUG_LEVEL5 (1<<5)

static int debug = 0;
#define dlog(level,format,...) \
	do { \
		if(level & debug){ \
			time_t t = time(NULL); \
			char buf[256]; \
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t)); \
			fprintf(stdout, "[%s][ID:%6d][%6d %s %s] "format"", buf, (int)pthread_self(),\
				__LINE__, __FILE__, __FUNCTION__, \
				##__VA_ARGS__); \
		} \
	}while(0)


/************************************************/
enum cluster_mode{
	CLUSTER_MASTER, // master
	CLUSTER_STANDALONE, // standalone
	CLUSTER_OTHER,	 //slave
};


enum ca_type{
	CALL_AST_SOCKET, //communicate with ast by ami
	CALL_AST_PIPE, //communicate with ast by "asterisk -rx ..."
};

enum channel_use_state{
	CHANNEL_USE_STATE_IDLE,
	CHANNEL_USE_STATE_INUSE,
	CHANNEL_USE_STATE_RELEASED,
};

struct cluster_config{
	enum cluster_mode mode;
	char password[MAX_LEN_NAME];
	char slave_ips[__BRD_SUM__][MAX_LEN_IP];
};

struct incalls_config{
	/* internal_call.conf [general]*/
	int enable;
	int maxchannel;
	int incalls_min;
	int incalls_max;
	int call_duration_min;
	int call_duration_max;

	/* internal_call.conf [dialplan]*/
	char context[MAX_LEN_NAME];
	char exten[MAX_LEN_NAME];

	/* internal_call.conf  [advance]*/
	enum ca_type call_ast_type;
	int get_channel_delay;		// second
	int call_fail_max;		// call fail max times
	int call_fail_delay;		// call fail interval
	int call_wait_answer;		// wait timeout for callee answer
	int call_check_delay;		// The interval of checks whether call is connected
	int call_next_delay;		// delay between two calls
	int ami_timeout;
	int ami_reconnect_delay;	// Second
	int enforce_incall_sw;		//on=hangup caller and callee before internal call, off=~on
	int log_max_size;
};

struct channel_node{
	char tech[MAX_LEN_NAME];
	char phonenumber[MAX_LEN_NAME];		
	enum channel_use_state channel_use_state;			// use state
	int channel_use_count;
	int board;
	int span;
	char call_channel[MAX_LEN_NAME];
	char call_state[MAX_LEN_NAME];			// call state
	int call_success_count;
	int call_fail_count;
	int call_check_count;
};

struct worker_node{
	pthread_t tid;
	pthread_mutex_t pth_lock;
	pthread_cond_t pth_cond;
	struct mansession ami_session; // for AMI

	struct channel_node *caller;//channel call out
	struct channel_node *callee;//channel call in
	int call_connected;
};

struct workers_info{
	/* config settings (internal_call.conf) */
	struct incalls_config cfg;
	struct cluster_config cluster_cfg;

	/* runtime info */
	struct channel_node channel[__GSM_SUM__*__BRD_SUM__];
	struct worker_node worker[__GSM_SUM__*__BRD_SUM__/2]; // every worker serve two channels
	int worker_sum;
	pthread_mutex_t lock;
	int stop;
};

struct ami_info{
	char host[MAX_LEN_IP];
	char username[MAX_LEN_USERNAME];
	char secret[MAX_LEN_SECRET];
	int port;
};

static struct workers_info global_workers_info;
static struct ami_info global_ami_info = {
	.host = "127.0.0.1",
	.username = AMI_SUPER_USERNAME,
	.secret = AMI_SUPER_SECTET,
	.port = AMI_SUPER_PORT,
};



static int ami_hangup_channel(struct mansession *s, struct channel_node *channel);
static int ami_release_channel(struct mansession *s,  struct channel_node *channel);
static int astrx_hangup_channel(struct channel_node *channel);
static int astrx_release_channel(struct channel_node *channel);


/************************************************
 *
 *         general functions
 *
 ************************************************/

static char *trim( char *str )
{
	char *copied, *ret = str, *tail = NULL;
	if ( str == NULL )
		return NULL;

	for( copied = str; *str; str++ ) {
		if ( *str != ' ' && *str != '\t' ) {
			*copied++ = *str;
			tail = copied;
		} else {
			if ( tail )
				*copied++ = *str;
		}
	}

	if ( tail )
		*tail = 0;
	else
		*copied = 0;

	return ret;
}

static int lock_file(const char* path)
{
	char temp_path[256];
	char lock_path[256];
	int fd = -1, len = 0, i = 0;

	/* make lock file name */
	snprintf(temp_path,256,"%s",path);
	len = strlen(temp_path);
	for(i=0; i<len; i++) {
		if(temp_path[i] == '/') {
			temp_path[i] = '_';
		}
	}	
	snprintf(lock_path, sizeof(lock_path), "/tmp/lock/%s.lock", temp_path);

	if((fd = open(lock_path, O_WRONLY|O_CREAT)) < 0) {
		printf("open error\n");
		return -1;
	}

	//Lock
	flock(fd, LOCK_EX);

	return fd;
}

static int unlock_file(int fd)
{
	if(fd <= 0) {
		return -1;
	} else {
		//UnLock
		flock(fd,LOCK_UN);
		close(fd);
		return 0;
	}
}

static int is_running(char *running_path)
{
	int fd = -1, count = 0;
	
	if((fd = open(running_path, O_CREAT|O_WRONLY|O_TRUNC, 0664)) < 0) {
		fprintf(stdout, "Unable to open %s: %s\n", running_path, strerror(errno));
		return -1;
	}

	count = 10;
	while(count > 0 && count--){
		if(flock(fd, LOCK_EX|LOCK_NB) < 0) {
			sleep(1);
		}else{
			break;
		}
	}

	if(count <= 0){
		close(fd);
		return -1;
	}

	return 0;
}

static int is_true(const char *s)
{
	if ( NULL == s || strlen(s) <= 0 )
		return 0;

	/* Determine if this is a true value */
	if (!strcasecmp(s, "yes") ||
	    !strcasecmp(s, "true") ||
	    !strcasecmp(s, "y") ||
	    !strcasecmp(s, "t") ||
	    !strcasecmp(s, "1") ||
	    !strcasecmp(s, "on"))
		return -1;

	return 0;
}

static int random_int(int min, int max)
{
	if(min < 0){
		return 0;
	}else if(min >= max){
		return min;
	}else{
		return min + (rand() % (max - min));
	}
}

static int file_size(char *file_path)
{
	struct stat file_stat;
	
	stat(file_path, &file_stat);
	
	return file_stat.st_size;
}

static int file_get_contents(char *file_path, char *buffer, int len)
{
	char *p = NULL;
	int fd = -1, rlen = 0, len_rest = 0;
	
	if ((fd = open(file_path,O_RDONLY)) < 0) {
		fprintf(stdout, "Unable to open %s: %s\n", file_path, strerror(errno));
		return -1;
	}
	
	if(flock(fd, LOCK_EX) < 0) {
		fprintf(stdout,"Lock File %s failed.\n", file_path);
		close(fd);
		return -1;
	}
	
	p = buffer;
	len_rest = len;
	while(len_rest > 0){
		rlen = read(fd,p,len_rest);
		if(rlen < 0){
			flock(fd, LOCK_UN);
			close(fd);
			return -1;
		}else if(rlen == 0){
			break;
		}else{
			p += rlen;
			len_rest -= rlen;
		}
	}
	
	if((flock(fd, LOCK_UN)) < 0) {
		fprintf(stdout,"Unlock File %s failed.\n", file_path);
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static int file_put_contents(char *file_path, char *buffer, int len)
{
	char *p = NULL;
	int fd = -1, wlen = 0, len_rest = 0;
	
	if((fd = open(file_path, O_CREAT|O_WRONLY|O_TRUNC, 0664)) < 0) {
		fprintf(stdout, "Unable to open %s: %s\n", file_path, strerror(errno));
		return -1;
	}
	
	if(flock(fd, LOCK_EX) < 0) {
		fprintf(stdout,"Lock File %s failed.\n", file_path);
		close(fd);
		return -1;
	}
	
	p = buffer;
	len_rest = len;
	while(len_rest > 0){
		wlen = write(fd,p,len_rest);
		if(wlen < 0){
			flock(fd, LOCK_UN);
			close(fd);
			return -1;
		}else if(wlen == 0){
			break;
		}else{
			p += wlen;
			len_rest -= wlen;
		}
	}

	if((flock(fd, LOCK_UN)) < 0) {
		fprintf(stdout,"Unlock File %s failed.\n", file_path);
		close(fd);
		return -1;
	}
	
	close(fd);

	return 0;
}

static int file_append_contents(const char *file_path, char *buffer, int len)
{
	char *p = NULL;
	int fd = -1, wlen = 0, len_rest = 0;

	if((fd = open(file_path, O_CREAT|O_WRONLY|O_APPEND, 0664)) < 0) {
		fprintf(stdout, "Unable to open %s: %s\n", file_path, strerror(errno));
		return -1;
	}
	
	if(flock(fd, LOCK_EX) < 0) {
		fprintf(stdout,"Lock File %s failed.\n", file_path);
		close(fd);
		return -1;
	}
	
	p = buffer;
	len_rest = strlen(buffer);
	while(len_rest > 0){
		wlen = write(fd,p,len_rest);
		if(wlen < 0){
			flock(fd, LOCK_UN);
			close(fd);
			return -1;
		}else if(wlen == 0){
			break;
		}else{
			p += wlen;
			len_rest -= wlen;
		}
	}

	if((flock(fd, LOCK_UN)) < 0) {
		fprintf(stdout,"Unlock File %s failed.\n", file_path);
		close(fd);
		return -1;
	}
	
	close(fd);

	return 0;
}

static int file_slim_contents(const char *file_path, int valid_start, int len)
{
	char *buffer = NULL, *p = NULL;
	int fd = -1, size = 0, len_rest = 0, ret = -1;

	if(valid_start < 0 || len <= 0 || file_path == NULL){
		return -1;
	}

	if((buffer = malloc(len)) == NULL){
		return -1;
	}

	if ((fd = open(file_path, O_RDWR, 0664)) < 0) {
		fprintf(stdout, "Unable to open %s: %s\n", file_path, strerror(errno));
		goto fail1;;
	}

	if(flock(fd, LOCK_EX) < 0) {
		fprintf(stdout,"Lock File %s failed.\n", file_path);
		goto fail2;
	}

	/* set file offset */
	if((ret = lseek(fd, valid_start, SEEK_SET)) != valid_start){
		goto fail3;		
	}

	/* read valid contents to buffer */
	p = buffer;
	len_rest = len;
	while(len_rest > 0){
		ret = read(fd, p, len_rest);
		if(ret < 0){
			goto fail3;
		}else if(ret == 0){
			break;
		}else{
			p += ret;
			len_rest -= ret;
		}
	}

	/* clean file */
	ftruncate(fd, 0);
	if((ret = lseek(fd, 0, SEEK_SET)) != 0){
		goto fail3;		
	}

	/* write buffer to file */
	p = buffer;
	len_rest = len;
	while(len_rest > 0){
		ret = write(fd, p, len_rest);
		if(ret < 0){
			goto fail3;
		}else if(ret == 0){
			break;
		}else{
			p += ret;
			len_rest -= ret;
		}
	}	

	if((flock(fd, LOCK_UN)) < 0) {
		goto fail2;
	}

	close(fd);
	free(buffer);

	return 0;
	
fail3:
	flock(fd, LOCK_UN);
fail2:
	close(fd);
fail1:
	free(buffer);
	return -1;
}

static int pipe_exec(char *command, char *retbuf, int retlen)
{
	FILE *stream = NULL;
	char *p = NULL;
	int len_rest = 0, rlen = 0;
	
	if(command == NULL){
		return -1;
	}
	
	if((stream = popen(command, "r")) == NULL){
		return -1;
	}

	if(retbuf != NULL && retlen > 0){
		memset(retbuf, 0, retlen);
		p = retbuf;
		len_rest = retlen;
		while(len_rest > 0){
			rlen = fread(p, 1, len_rest, stream);
			if(rlen < 0){
				pclose(stream);
				return -1;
			}else if(rlen == 0){
				break;
			}else{
				p += rlen;
				len_rest -= rlen;
			}
		}
	}
	pclose(stream);

	return 0;
}

static int write_pid(char *pid_path)
{
	char buffer[128];
	
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "%d", getpid());
	if(file_put_contents(pid_path, buffer, strlen(buffer)) < 0){
		return -1;
	}

	return 0;
}

static int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int sec)
{
	struct timeval tval;
	socklen_t len;
	fd_set rset, wset;

	int flags = -1, ret = -1, error = 0;

	flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0) {
		fprintf(stderr, "fcntl:%s\a\n", strerror(errno));
		return -1;
	}
	fcntl(sockfd,F_SETFL,flags|O_NONBLOCK);

	error = 0;

	ret = connect(sockfd, saptr, salen);
	if (ret < 0) {
		if (errno != EINPROGRESS) {
			return -1;
		}
	}else if(ret == 0){
		goto done;	/*connect completed immediately*/
	}

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = sec;
	tval.tv_usec = 0;

	if ((ret = select(sockfd+1,&rset,&wset,NULL,sec ? &tval : NULL)) == 0) {
		close(sockfd);		/* timeout */
		errno = ETIMEDOUT;
		return -1;
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
			return -1;	/* Solaris pending error */
		}
	} else {
		fprintf(stderr, "select error: sockfd not set\n");
		return -1;
	}

done:
	fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
	if (error) {
		errno = error;
		return -1;
	}

	return 0;
}

static int request_slave(const char* ip, char *data, int data_len, char *ret_val, int ret_len, int timeo)
{
	struct timeval timeout = {timeo?timeo:1,0};
	struct sockaddr_in my_addr;
	char *p = NULL;
	int client_sockfd = -1, ret_rest = 0, ret = -1;

	if(ip == NULL || data == NULL){
		return -1;
	}

	if((client_sockfd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
		printf("Cannot create communication socket.\n");
		return -1;
	}
	
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = inet_addr(ip);
	my_addr.sin_port = htons(SOCKET_PORT);

	if (connect_nonb(client_sockfd, (struct sockaddr *)(&my_addr), sizeof(struct sockaddr), timeo) < 0) {
		printf("Cannot connect to the server(%s),%s\n",ip,strerror(errno));
		close(client_sockfd);
		return -1;
	}
	
	setsockopt(client_sockfd, SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
	setsockopt(client_sockfd, SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
	
	if(send(client_sockfd, data, data_len, 0) < 0) {
		printf("Socket send failed: \n");
		close(client_sockfd);
		return -1;
	} 

	if(ret_val != NULL && ret_len > 0){
		ret_rest = ret_len;
		p = ret_val;
		while(ret_rest > 0){
			ret = recv(client_sockfd,p,ret_rest,0);
			//dlog(DEBUG_LEVEL3,"socket recv data = [%s]\n",p);
			if(ret < 0){
				close(client_sockfd);
				return -1;
			}else if(ret == 0){
				break;
			}else{
				p += ret;
				ret_rest -= ret;
			}
		}
		
	}

	close(client_sockfd);
	return 0;
}

static int init_cluster_config(struct cluster_config *cfg)
{
/* cluster.conf
---------------
[general]
mode=master

[slave]
password=
ip=
masterip=
remain_ori_ip=1

[master]
password=5560
ip=192.168.5.56

[slavelist]
Board-2_ip=192.168.5.57
Board-2_ori_ip=172.16.5.57

----------------
*/
	char *file_path = "/etc/asterisk/gw/cluster.conf";
	char line[MAX_LEN_LINE];
	char key[MAX_LEN_LINE];
	char value[MAX_LEN_LINE];
	char section[MAX_LEN_LINE];
	int i = 0;

	FILE* fp;
	int lock;
	
	memset(cfg, 0, sizeof(*cfg));

	cfg->mode = CLUSTER_STANDALONE;

	lock = lock_file(file_path);
	if( NULL == (fp=fopen(file_path,"r")) ) {
		fprintf(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	memset(line, 0, sizeof(line));
	memset(section, 0, sizeof(line));
	while(!feof(fp) && fgets(line, sizeof(line), fp)) {
		memset(key,0,sizeof(key));
		memset(value,0,sizeof(value));
		if(sscanf(line, "[%1024[^]]]", section) == 1){
			trim(section);
		}else if(section[0] != '\0' && sscanf(line, "%1024[^=]=%1024s", key, value) == 2){
			trim(key);
			trim(value);
			if(0 == strcmp(section,"general")){
				if(0 == strcmp(key,"mode")) {
					if(0 == strcmp(value,"master")){
						cfg->mode = CLUSTER_MASTER;
					}else{
						cfg->mode = CLUSTER_OTHER;
					}
				}
			}else if(0 == strcmp(section,"master")){
				if(0 == strcmp(key,"password")) {
					strncpy(cfg->password,value,sizeof(cfg->password));
				}
			}else if(0 == strcmp(section,"slavelist")){
				int b;
				for (b=2; b<=__BRD_SUM__; b++) {
					char board_str[32];
					memset(board_str,0,sizeof(board_str));
					snprintf(board_str, sizeof(board_str), "%s%d_ip", __BRD_HEAD__, b);
					if(0 == strcmp(key,board_str)) {
						strncpy(cfg->slave_ips[b-2],value,sizeof(cfg->slave_ips[b-2]));
						break;
					}
				}

			}
		}
		memset(line,0,sizeof(line));
	}

	fclose(fp);
	unlock_file(lock);

	if(debug){
		printf("cluster_config:\n"\
			"[general]\n"\
			"cluster mode = [%d]\n"\
			"[master]\n"\
			"password = [%s]\n"\
			"[slavelist]\n",
			cfg->mode,
			cfg->password
			);
		for(i = 0; i < __BRD_SUM__-1; i++){
			printf("Board-%d_ip = [%s]\n", i+2, cfg->slave_ips[i]);
		}
	}

	return 0;
}

/************************************************
 *
 *         normal functions
 *
 ************************************************/
static int print_astmsg(int level, struct message *msg)
{
	int i = 0;
	
	if(debug & level){
		for(i = 0; i < msg->hdrcount; i++){
			if(msg->headers[i][0] != '\0'){
				printf("%s",msg->headers[i]);
			}
		}
	}
}

static int print_channels(int level)
{
	struct channel_node *channel;
	int i = 0;

	if(debug & level){
		printf("%-10s%-10s%-10s%-25s%-25s%-12s%-12s\n","[num]","[board]","[span]","[tech]","[phonenumber]","[use_count]","[use_state]");
		for(i = 0; i < global_workers_info.cfg.maxchannel; i++){
			channel = &global_workers_info.channel[i];
			printf("%-10d%-10d%-10d%-25s%-25s%-12d%-12d\n", 
				i,
				channel->board,
				channel->span,
				channel->tech,
				channel->phonenumber,
				channel->channel_use_count,
				channel->channel_use_state);
		}
	}
}

static int update_log(struct worker_node *worker, int result)
{
/*
-----------------
[log datetime][from span][to span:phonenumber][duration]

-----------------
*/
	char *file_path = "/tmp/log/auto_intercall.log";
	char buffer[MAX_LEN_BUFFER], tmp[256], *p = NULL;
	int fd = -1, wlen = 0, len_rest = 0, size = 0;
	time_t t;

	/* make log contents */
	t = time(NULL);
	memset(tmp, 0, sizeof(tmp));
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&t));
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "[%s] Call: [gsm-%d.%d:%s] ---> [gsm-%d.%d:%s]. Result: [%s]\n", 
		tmp, 
		worker->caller->board,worker->caller->span,worker->caller->phonenumber,
		worker->callee->board,worker->callee->span,worker->callee->phonenumber,
		result==0?"SUCCESS":"FAIL");
	dlog(DEBUG_LEVEL2, "buffer:%s", buffer);

	/* update log */
	if(file_append_contents(file_path, buffer, sizeof(buffer)) != 0){
		return -1;
	}
	
	/* slim log file */
	size = file_size(file_path);
	dlog(DEBUG_LEVEL1, "size=%d default=%d max=%d\n", size, DEFAULT_LOG_MAX_SIZE, global_workers_info.cfg.log_max_size);
	if(size > global_workers_info.cfg.log_max_size){
		dlog(DEBUG_LEVEL1, "file_slim_contents(%s, %d, %d)\n", file_path, size-(global_workers_info.cfg.log_max_size/2), global_workers_info.cfg.log_max_size/2);
		file_slim_contents(file_path, size-(global_workers_info.cfg.log_max_size/2), global_workers_info.cfg.log_max_size/2);
	}

	return 0;
}

static int get_value(char *src, char *key, char *value, int len)
{
/* eg:
D-channel: 2
Status: Power on, Provisioned, Up, Active, Standard
Type: CPE
Manufacturer: SIMCOM_Ltd
Model Name: SIMCOM_SIM840W
Model IMEI: 860041020985605
Revision: Revision:1224B02SIM840W16
Network Name: CHINA MOBILE
Network Status: Registered (Home network)
Signal Quality (0,31): 20
BER value (0,7): 0
SIM IMSI: 460021180590487
SIM SMS Center Number: +8613800755500
Own Number: 
Remain Time: 
PDD: 7
ASR: 20
ACD: 12
State: READY
*/
	char *start = NULL;
	char tmp[MAX_LEN_NAME];

	if(src == NULL || key == NULL || value == NULL){
		return -1;
	}
	
	if(NULL != (start = strstr(src, key))){
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp), "%s:%%%d[^\n]", key, len);
		if(sscanf(start, tmp, value) == 1){
			trim(value);
			return 0;
		}
	}
	
	return -1;
}

static int get_gsm_info(struct channel_node *channel)
{
	char file_path[256];
	char command[MAX_LEN_COMMAND];
	char buffer[MAX_LEN_BUFFER];
	char tmp[MAX_LEN_LINE];
	char *ip = NULL, *start = NULL, *end = NULL, *p = NULL;

	dlog(DEBUG_LEVEL3,"\n");

	memset(buffer, 0, sizeof(buffer));
	if(channel->board == 1){
		memset(file_path, 0, sizeof(file_path));
		snprintf(file_path, sizeof(file_path), "/tmp/gsm/%d", channel->span);
		
		if(file_get_contents(file_path, buffer, sizeof(buffer)) == 0){
			dlog(DEBUG_LEVEL5, "/tmp/gsm/%d buffer:%s\n", channel->span, buffer);
		}else{
			dlog(DEBUG_LEVEL3,"\n");
			return -1;
		}
	}else if(channel->board > 1 && channel->board <= __BRD_SUM__ && global_workers_info.cluster_cfg.mode == CLUSTER_MASTER){
		ip = global_workers_info.cluster_cfg.slave_ips[channel->board-2];
		if(ip[0] != '\0'){
			memset(command, 0, sizeof(command));
			snprintf(command, sizeof(command), "get_spans");
			if(request_slave(ip,command,strlen(command),buffer,sizeof(buffer),5) == 0){
				dlog(DEBUG_LEVEL5, "request [%s] buffer:%s\n",ip, buffer);
				memset(tmp, 0, sizeof(tmp));
				snprintf(tmp, sizeof(tmp), "---Start Span %d---", channel->span);
				if(NULL != (start = strstr(buffer, tmp))){
					start += strlen(tmp);
					memset(tmp, 0, sizeof(tmp));
					snprintf(tmp, sizeof(tmp), "---End Span %d---", channel->span);
					if(NULL != (end = strstr(buffer, tmp))){
						for(p = buffer;start < end; start++, p++){
							*p = *start;
						}
						*p = '\0';
					}
				}
			}else{
				dlog(DEBUG_LEVEL3,"\n");
				return -1;
			}
		}
	}else{
		dlog(DEBUG_LEVEL3,"\n");
		return -1;
	}
	
	dlog(DEBUG_LEVEL5, "board [%d] span [%d] buffer:%s\n", channel->board, channel->span, buffer);

	/* check gsm status */
	/*memset(tmp, 0, sizeof(tmp));
	if(get_value(buffer, "Status", tmp, sizeof(tmp)) != 0){
		return -1;
	}
	if(strncasecmp(tmp, DEFAULT_GSM_UP_KEYWORD, sizeof(DEFAULT_GSM_UP_KEYWORD)-1)){
		return -1;
	}*/

	/* get channel phone number */
	memset(tmp, 0, sizeof(tmp));
	if(get_value(buffer, "Own Number", tmp, sizeof(tmp)) != 0){
		return -1;
	}
	if(tmp[0] != '\0'){
		strncpy(channel->phonenumber, tmp, sizeof(channel->phonenumber));

		dlog(DEBUG_LEVEL3, "success [%s] phnum [%s]\n", channel->tech, channel->phonenumber);
		print_channels(DEBUG_LEVEL3);

		/* success */
		return 0;
	}

	dlog(DEBUG_LEVEL3, "fail [%s] phnum []\n", channel->tech);
	print_channels(DEBUG_LEVEL3);

	return -1;
}

static int get_new_channel(struct worker_node *worker)
{
	struct channel_node *channel = NULL, *channel1 = NULL, *channel2 = NULL;
	int i = 0;

	/* 1. get two free channel */
	pthread_mutex_lock(&global_workers_info.lock);
	for(i = 0; i < global_workers_info.cfg.maxchannel; i++){
		channel = &global_workers_info.channel[i];
		if(channel->tech[0] != '\0' && channel->channel_use_state == CHANNEL_USE_STATE_IDLE){
			if(channel1 == NULL){
				channel1 = channel;
			}else if(channel2 == NULL){
				channel2 = channel;
			}else{
				break;
			}
		}
	}

	/* 2. set caller and callee */
	if(channel1 != NULL && channel2 != NULL){
		/* The one who was used more times will be call callee */
		if(channel1->channel_use_count <= channel2->channel_use_count){
			worker->caller = channel1;
			worker->callee = channel2;
		}else{
			worker->caller = channel2;
			worker->callee = channel1;
		}
		
		worker->caller->channel_use_count++;
		//worker->callee->channel_use_count++;
		worker->caller->channel_use_state = CHANNEL_USE_STATE_INUSE;
		worker->callee->channel_use_state = CHANNEL_USE_STATE_INUSE;
		
		pthread_mutex_unlock(&global_workers_info.lock);
		print_channels(DEBUG_LEVEL3);
		
		/* Success get channels */
		return 0;
	}

	pthread_mutex_unlock(&global_workers_info.lock);
	dlog(DEBUG_LEVEL3, "get new channel over\n");
	print_channels(DEBUG_LEVEL3);
	/* fail */
	return -1;
}

static int set_channel_idle(struct channel_node *channel)
{
	if(channel == NULL){
		return -1;
	}
	
	pthread_mutex_lock(&global_workers_info.lock);
	if(channel->channel_use_state == CHANNEL_USE_STATE_RELEASED){
		memset(channel, 0, sizeof(*channel));
	}
	
	channel->channel_use_state = CHANNEL_USE_STATE_IDLE;
	pthread_mutex_unlock(&global_workers_info.lock);

	return 0;
}

static int set_channel_tech(char *tech, int board, int span)
{
	int i = 0;
	
	for(i = 0; i < global_workers_info.cfg.maxchannel; i++){
		if((board == global_workers_info.channel[i].board && span == global_workers_info.channel[i].span)
			|| !strcasecmp(tech, global_workers_info.channel[i].tech)
		){
			/* already exist */
			return 0;
		}
	}

	/* new channel */
	for(i = 0; i < global_workers_info.cfg.maxchannel; i++){
		if(global_workers_info.channel[i].tech[0] == '\0'){
			global_workers_info.channel[i].board = board;
			global_workers_info.channel[i].span = span;
			strncpy(global_workers_info.channel[i].tech, tech, sizeof(global_workers_info.channel[i].tech));	
			break;
		}
	}

	return 0;
}

static int worker_timewait(struct worker_node *worker, int timewait)
{
	struct timeval time_val;
	struct timespec time_spec;
	struct message msg;
	char command[MAX_LEN_COMMAND];
	char buffer[MAX_LEN_BUFFER];
	
	gettimeofday(&time_val, NULL);
	
	pthread_mutex_lock(&worker->pth_lock);
	time_spec.tv_sec = time_val.tv_sec + timewait;
	time_spec.tv_nsec = time_val.tv_usec * 1000;
	pthread_cond_timedwait(&worker->pth_cond, &worker->pth_lock, &time_spec);
	pthread_mutex_unlock(&worker->pth_lock);

	if(global_workers_info.stop){
		if(global_workers_info.cfg.call_ast_type == CALL_AST_PIPE){
			astrx_hangup_channel(worker->caller);
			astrx_release_channel(worker->callee);
		}else if(global_workers_info.cfg.call_ast_type == CALL_AST_SOCKET){
			ami_hangup_channel(&worker->ami_session, worker->caller);
			ami_release_channel(&worker->ami_session, worker->callee);
		}
		pthread_exit(NULL);
	}

	return 0;
}

static int ami_update_channel(struct mansession *s)
{
	struct channel_node *channel = NULL;
	struct message msg;
	char command[MAX_LEN_COMMAND];
	char tmp[MAX_LEN_NAME];
	int ret = -1, timeout = 0, i = 0, board = 0, span = 0;

	/* update internal call channel tech */
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "internalcall show invalid tech");
	memset(&msg, 0, sizeof(msg));
	ret = astman_command(s, &msg, command, NULL, timeout);
	print_astmsg(DEBUG_LEVEL3, &msg);
	if(ret != ASTMAN_SUCCESS){
		return ret;
	}

	pthread_mutex_lock(&global_workers_info.lock);
	for(i = 0; i < msg.hdrcount; i++){
		board = 0;
		span = 0;
		memset(tmp, 0, sizeof(tmp));
		if(sscanf(msg.headers[i], "%d.%d:%128s", &board, &span, tmp) == 3){
			if(board >= 1 && board <= __BRD_SUM__ 
				&& span >= 1 && span <= __GSM_SUM__
				&& tmp[0] != '\0'
			){
				set_channel_tech(tmp, board, span);
			}
		}
	}
	pthread_mutex_unlock(&global_workers_info.lock);

	dlog(DEBUG_LEVEL1, "update channels over\n");
	print_channels(DEBUG_LEVEL2);

	return ASTMAN_SUCCESS;
}

static int ami_channel_info(struct mansession *s, struct channel_node *channel)
{
	struct message msg;
	char command[MAX_LEN_COMMAND];
	int ret = 0, i = 0;

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "core show channels concise");
	dlog(DEBUG_LEVEL1, "check internal call status [%s]\n", command);
	memset(&msg, 0, sizeof(msg));	
	if((ret = astman_command(s, &msg, command, NULL, global_workers_info.cfg.ami_timeout)) != ASTMAN_SUCCESS){
		return ret;
	}
	print_astmsg(DEBUG_LEVEL3, &msg);
	
	memset(channel->call_channel, 0, sizeof(channel->call_channel));
	memset(channel->call_state, 0, sizeof(channel->call_state));
	for(i = 0; i < msg.hdrcount; i++){
		if(0 == strncasecmp(msg.headers[i], channel->tech, strlen(channel->tech))){
			/* Format: channel!...!...!...!Up!..  */
			if(2 == sscanf(msg.headers[i], "%128[^!]!%*[^!]!%*[^!]!%*[^!]!%128[^!]!", 
				channel->call_channel, 
				channel->call_state)
			){
				trim(channel->call_channel);
				trim(channel->call_state);
				return ASTMAN_SUCCESS;
			}
		}
	}

	return ASTMAN_FAILURE;
}

static int ami_hangup_channel(struct mansession *s, struct channel_node *channel)
{
	struct message msg;
	char command[MAX_LEN_COMMAND];

	if(s == NULL || channel == NULL || channel->call_channel[0] == '\0'){
		return ASTMAN_FAILURE;
	}
	
	memset(&msg, 0, sizeof(msg));
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "channel request hangup %s", channel->call_channel);
	dlog(DEBUG_LEVEL1, "internal call hangup [%s]\n", command);
	return astman_command(s, &msg, command, NULL, global_workers_info.cfg.ami_timeout);
}

static int ami_release_channel(struct mansession *s,  struct channel_node *channel)
{
	struct message msg;
	char command[MAX_LEN_COMMAND];
	int ret = -1;

	dlog(DEBUG_LEVEL1,"\n");

	if(s == NULL || channel == NULL 
		|| channel->board <= 0 || channel->board > __BRD_SUM__
		|| channel->span <= 0 || channel->span > __GSM_SUM__
	){
		return -1;
	}

	memset(&msg, 0, sizeof(msg));
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "internalcall clear out counter %d.%d", channel->board, channel->span);
	dlog(DEBUG_LEVEL1, "command release channel [%s]\n", command);

	ret = astman_command(s, &msg, command, NULL, global_workers_info.cfg.ami_timeout);

	if(ret == ASTMAN_SUCCESS){
		channel->channel_use_state = CHANNEL_USE_STATE_RELEASED;
	}
	
	return ret;
}

static int ami_internal_call(struct worker_node *worker)
{
	struct message msg;
	char command[MAX_LEN_COMMAND];
	char channel[128];
	int ret = -1, i = 0, j = 0;
	int call_wait = 30;
	int call_duration = 60;

	dlog(DEBUG_LEVEL1,"\n");
	
	/* if enforce internal call is on, hangup caller and callee before internal call */
	if(global_workers_info.cfg.enforce_incall_sw){
		if(ami_channel_info(&worker->ami_session, worker->caller) == ASTMAN_SUCCESS && worker->caller->call_channel[0] != '\0'){
			ami_hangup_channel(&worker->ami_session, worker->caller);
		}
		if(ami_channel_info(&worker->ami_session, worker->callee) == ASTMAN_SUCCESS && worker->callee->call_channel[0] != '\0'){
			ami_hangup_channel(&worker->ami_session, worker->callee);
		}
	}

	/* start internal call */
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "channel originate %s/%s extension %s@%s", 
		worker->caller->tech, 
		worker->callee->phonenumber,
		global_workers_info.cfg.exten,
		global_workers_info.cfg.context);
	dlog(DEBUG_LEVEL1, "start internal call command [%s]\n", command);
	memset(&msg, 0, sizeof(msg));
	if((ret = astman_command(&worker->ami_session, &msg, command, NULL, global_workers_info.cfg.call_wait_answer)) != ASTMAN_SUCCESS){
		return ret;
	}
	print_astmsg(DEBUG_LEVEL3, &msg);

	/* check whether call is connected */
	worker->call_connected = 0;
	for(i = 0; i < global_workers_info.cfg.call_wait_answer/global_workers_info.cfg.call_check_delay; i++){
		if((ret = ami_channel_info(&worker->ami_session, worker->caller)) != ASTMAN_SUCCESS){
			return ret;
		}	
		if(worker->caller->call_channel[0] != '\0' && !strcasecmp(worker->caller->call_state, "Up")){
			dlog(DEBUG_LEVEL1, "internal call channel [%s] state [%s]\n", worker->caller->call_channel, worker->caller->call_state);
			worker->call_connected = 1;
			break;
		}

		worker_timewait(worker, global_workers_info.cfg.call_check_delay);
	}
	if(!worker->call_connected){
		dlog(DEBUG_LEVEL1, "internal call connect fail\n");
		return ASTMAN_FAILURE;
	}
	
	/* wait internal call */
	call_duration = random_int(global_workers_info.cfg.call_duration_min, global_workers_info.cfg.call_duration_max);
	dlog(DEBUG_LEVEL1,"Internal call duration %ds\n", call_duration);
	worker_timewait(worker, call_duration);

	/* hangup internal call */
	if((ret = ami_hangup_channel(&worker->ami_session, worker->caller)) != ASTMAN_SUCCESS){
		return ret;
	}
	
	return ASTMAN_SUCCESS;
}

static void *ami_worker_pthread(void *arg)
{
	struct worker_node *worker = (struct worker_node *)arg;
	struct message msg;
	struct timeval now;
	struct timespec timeout;
	char buffer[4096];
	int ret = -1, i = 0, ami_connected = 0, j = 0;
	int call_max_count = 0;

	for(;;){
		if(ASTMAN_SUCCESS != astman_login(&worker->ami_session, NULL, global_ami_info.port, global_ami_info.username, global_ami_info.secret)){
			goto ami_close;
		}
		worker->ami_session.debug = debug;
		dlog(DEBUG_LEVEL1,"astman_login success . debug [%d/%d] \n", worker->ami_session.debug, debug);

		/* events off */
		memset(&msg, 0, sizeof(msg));
		ret = astman_events(&worker->ami_session, &msg, "off", NULL, global_workers_info.cfg.ami_timeout);
		if(ret == ASTMAN_SOCKCLOSED){
			goto ami_close;
		}

		for(;;){
			dlog(DEBUG_LEVEL1,"\n");

			/* 1. get caller and callee */
			for(;;){
				/* update channel */
				ret = ami_update_channel(&worker->ami_session);
				if(ret == ASTMAN_SUCCESS){
					if(get_new_channel(worker) == 0){
						dlog(DEBUG_LEVEL1, "get new channel [%s]->[%s]\n", worker->caller->tech, worker->callee->tech);
						break;
					}
				}else if(ret == ASTMAN_SOCKCLOSED){
					goto ami_close;
				}
				dlog(DEBUG_LEVEL1,"get channel delay sleep [%d]\n", global_workers_info.cfg.get_channel_delay);
				worker_timewait(worker, global_workers_info.cfg.get_channel_delay);
			}
			
			dlog(DEBUG_LEVEL1,"\n");

			/* 2. internal call */
			call_max_count = random_int(global_workers_info.cfg.incalls_min, global_workers_info.cfg.incalls_max);
			for(i = 0; i < call_max_count; i++){
				worker->caller->call_check_count = 0;
				worker->callee->call_check_count = 0;
				for(j = 0; j < global_workers_info.cfg.call_fail_max; j++){
					/* get callee phone number */
					if(get_gsm_info(worker->caller) != 0){
						dlog(DEBUG_LEVEL1,"\n");
						worker->caller->call_check_count++;
						worker_timewait(worker, global_workers_info.cfg.call_fail_delay);
						continue;
					}
					if(get_gsm_info(worker->callee) != 0){
						dlog(DEBUG_LEVEL1,"\n");
						worker->callee->call_check_count++;
						worker_timewait(worker, global_workers_info.cfg.call_fail_delay);
						continue;
					}
					
					ret = ami_internal_call(worker);
					update_log(worker, ret == ASTMAN_SUCCESS?0:-1);
					if(ret == ASTMAN_SUCCESS){
						dlog(DEBUG_LEVEL1, "internalcall ok\n");
						worker->callee->call_success_count++;
						break;
					}else if(ret == ASTMAN_SOCKCLOSED){
						dlog(DEBUG_LEVEL1, "Socket closed . Goto logoff\n");
						goto ami_close;
					}else{
						dlog(DEBUG_LEVEL1, "internalcall fail\n");
						worker->caller->call_fail_count++;
						worker->callee->call_fail_count++;
						worker_timewait(worker, global_workers_info.cfg.call_fail_delay);
					}
				}
				if(j == global_workers_info.cfg.call_fail_max){
					if(worker->caller->call_check_count >= global_workers_info.cfg.call_fail_max
						|| worker->caller->call_fail_count >= 2*global_workers_info.cfg.call_fail_max
					){
						/* release caller */
						dlog(DEBUG_LEVEL1,"release callee [%s]\n", worker->caller->tech);
						ret = ami_release_channel(&worker->ami_session, worker->caller);
						if(ret == ASTMAN_SOCKCLOSED){
							goto ami_close;
						}
					}
					if(worker->callee->call_check_count >= global_workers_info.cfg.call_fail_max
						|| worker->callee->call_fail_count >= 2*global_workers_info.cfg.call_fail_max
					){
						/* release callee */
						dlog(DEBUG_LEVEL1,"release callee [%s]\n", worker->callee->tech);
						ret = ami_release_channel(&worker->ami_session, worker->callee);
						if(ret == ASTMAN_SOCKCLOSED){
							goto ami_close;
						}
					}
					break;
				}
				
				dlog(DEBUG_LEVEL1,"[%d/%d/%d][%s->%s] call delay sleep %ds\n", i+1, call_max_count, worker->callee->call_success_count,
					worker->caller->tech, worker->callee->tech, 
					global_workers_info.cfg.call_next_delay);
				worker_timewait(worker, global_workers_info.cfg.call_next_delay);
			}

			/* 3. release call */
			if(worker->callee->call_success_count >= call_max_count){
				dlog(DEBUG_LEVEL1,"release callee [%s]\n", worker->callee->tech);
				ret = ami_release_channel(&worker->ami_session, worker->callee);
				if(ret == ASTMAN_SOCKCLOSED){
					goto ami_close;
				}
			}
			set_channel_idle(worker->caller);
			set_channel_idle(worker->callee);
			worker->caller = NULL;
			worker->callee = NULL;
			
			print_channels(DEBUG_LEVEL3);

			dlog(DEBUG_LEVEL1,"next call delay sleep %ds\n", global_workers_info.cfg.call_next_delay);
			worker_timewait(worker, global_workers_info.cfg.call_next_delay);
		}
ami_close:
		if(worker->caller != NULL){
			worker->caller->channel_use_state = CHANNEL_USE_STATE_RELEASED;
			set_channel_idle(worker->caller);
		}
		if(worker->callee != NULL){
			worker->callee->channel_use_state = CHANNEL_USE_STATE_RELEASED;
			set_channel_idle(worker->callee);
		}
		dlog(DEBUG_LEVEL1,"ami close\n");
		astman_close(&worker->ami_session);
		worker_timewait(worker, global_workers_info.cfg.ami_reconnect_delay);
	}

	return NULL;
}

static int astrx_update_channel(void)
{
	struct channel_node *channel = NULL;
	char buffer[MAX_LEN_BUFFER];
	char command[MAX_LEN_COMMAND];
	char tmp[MAX_LEN_NAME];
	FILE *stream = NULL;
	char *start = NULL, *end = NULL;
	int ret = -1, board = 0, span = 0;

	/* update internal call channel tech */
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "asterisk -rx \"internalcall show invalid tech\"");
	dlog(DEBUG_LEVEL1,"command [%s]\n", command);
	if((ret = pipe_exec(command, buffer, sizeof(buffer))) != 0){
		return -1;
	}

	dlog(DEBUG_LEVEL3,"buffer [%s]\n", buffer);
	pthread_mutex_lock(&global_workers_info.lock);
	start = buffer;
	end = strchr(start, '\n');
	while(*start != '\0' && end != NULL){
		board = 0;
		span = 0;
		memset(tmp, 0, sizeof(tmp));
		if(sscanf(start, "%d.%d:%128s", &board, &span, tmp) == 3){
			if(board >= 1 && board <= __BRD_SUM__ 
				&& span >= 1 && span <= __GSM_SUM__
				&& tmp[0] != '\0'
			){
				set_channel_tech(tmp, board, span);
			}
		}
		start = end + 1;
		end = strchr(start, '\n');
	}
	pthread_mutex_unlock(&global_workers_info.lock);

	dlog(DEBUG_LEVEL1, "update channels over\n");
	print_channels(DEBUG_LEVEL2);

	return 0;
}


static int astrx_channel_info(struct channel_node *channel)
{
	char command[MAX_LEN_COMMAND];
	char buffer[MAX_LEN_BUFFER];
	char *find = NULL;

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "asterisk -rx \"core show channels concise\"");
	dlog(DEBUG_LEVEL1,"command [%s]\n", command);
	if(pipe_exec(command, buffer, sizeof(buffer)) != 0){
		dlog(DEBUG_LEVEL1,"command fail [%s]\n", command);
		return -1;
	}
	
	memset(channel->call_channel, 0, sizeof(channel->call_channel));
	memset(channel->call_state, 0, sizeof(channel->call_state));
	if(NULL == (find = strcasestr(buffer, channel->tech))){
		return -1;
	}
	
	/* Format: channel!...!...!...!Up!..  */
	if(2 == sscanf(find, "%128[^!]!%*[^!]!%*[^!]!%*[^!]!%128[^!]!", 
		channel->call_channel, 
		channel->call_state)
	){
		trim(channel->call_channel);
		trim(channel->call_state);
		return 0;
	}
	
	return -1;
}

static int astrx_hangup_channel(struct channel_node *channel)
{
	char command[MAX_LEN_COMMAND];
	char buffer[MAX_LEN_BUFFER];
	char *find = NULL;
	int ret = 0;
	
	if(channel == NULL || channel->call_channel[0] == '\0'){
		return -1;
	}
	
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "asterisk -rx \"channel request hangup %s\"", channel->call_channel);
	dlog(DEBUG_LEVEL1,"command [%s]\n", command);
	if(pipe_exec(command, buffer, sizeof(buffer)) != 0){
		dlog(DEBUG_LEVEL1,"command fail [%s]\n", command);
		return -1;
	}		

	return 0;
}

static int astrx_release_channel(struct channel_node *channel)
{
	char command[MAX_LEN_COMMAND];
	char buffer[MAX_LEN_BUFFER];
	FILE *stream = NULL;

	if(channel == NULL 
		|| channel->board <= 0 || channel->board > __BRD_SUM__
		|| channel->span <= 0 || channel->span > __GSM_SUM__
	){
		return -1;
	}
	
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "asterisk -rx \"internalcall clear out counter %d.%d\"", channel->board, channel->span);
	dlog(DEBUG_LEVEL1,"command [%s]\n", command);
	if(pipe_exec(command, buffer, sizeof(buffer)) != 0){
		return -1;
	}

	channel->channel_use_state = CHANNEL_USE_STATE_RELEASED;
	
	return 0;
}

static int astrx_internal_call(struct worker_node *worker)
{
	char command[MAX_LEN_COMMAND];
	char buffer[MAX_LEN_BUFFER];
	FILE *stream = NULL;
	char *find = NULL;
	int ret = -1, call_connected = 0, i = 0;
	int call_duration = 60;

	dlog(DEBUG_LEVEL1,"\n");

	/* if enforce internal call is on, hangup caller and callee before internal call */
	if(global_workers_info.cfg.enforce_incall_sw){
		if(astrx_channel_info(worker->caller) == 0 && worker->caller->call_channel[0] != '\0'){
			astrx_hangup_channel(worker->caller);
		}
		if(astrx_channel_info(worker->callee) == 0 && worker->callee->call_channel[0] != '\0'){
			astrx_hangup_channel(worker->callee);
		}
	}

	/* start internal call */
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "asterisk -rx \"channel originate %s/%s extension %s@%s\"", 
		worker->caller->tech, 
		worker->callee->phonenumber,
		global_workers_info.cfg.exten,
		global_workers_info.cfg.context);
	dlog(DEBUG_LEVEL1,"command [%s]\n", command);
	if((ret = pipe_exec(command, buffer, sizeof(buffer))) != 0){
		dlog(DEBUG_LEVEL1,"command fail [%s]\n", command);
		return -1;
	}

	/* check whether call is connected */
	worker->call_connected = 0;
	if(astrx_channel_info(worker->caller) == 0){
		if(worker->caller->call_channel[0] != '\0' && !strcasecmp(worker->caller->call_state, "Up")){
			worker->call_connected = 1;
		}
	}else{
		return -1;
	}

	dlog(DEBUG_LEVEL1, "internal call channel [%s] state [%s]\n", worker->caller->call_channel, worker->caller->call_state);
	if(!worker->call_connected){
		return -1;
	}
	
	/* wait internal call */
	call_duration = random_int(global_workers_info.cfg.call_duration_min, global_workers_info.cfg.call_duration_max);
	dlog(DEBUG_LEVEL1,"internal call duration %ds\n", call_duration);
	worker_timewait(worker, call_duration);

	/* hangup internal call */
	if(astrx_hangup_channel(worker->caller) != 0){
		return -1;
	}
	worker->call_connected = 0;

	return 0;
}

static void *astrx_worker_pthread(void *arg)
{
	struct worker_node *worker = (struct worker_node *)arg;
	time_t timep;
	int ret = 0, i = 0, j = 0;
	int call_max_count = 0;

	if(worker == NULL){
		return NULL;
	}
	
	dlog(DEBUG_LEVEL1, "astrx_worker_pthread start\n");
	for(;;){
		/* 1. get caller and callee */
		dlog(DEBUG_LEVEL1,"\n");
		while(1){
			astrx_update_channel();
			if(get_new_channel(worker) == 0){
				dlog(DEBUG_LEVEL1, "get new channel [%s]->[%s]\n", worker->caller->tech, worker->callee->tech);
				break;
			}
			
			worker_timewait(worker, global_workers_info.cfg.get_channel_delay);
		}

		dlog(DEBUG_LEVEL1,"\n");
		
		/* 2. internal call */
		call_max_count = random_int(global_workers_info.cfg.incalls_min, global_workers_info.cfg.incalls_max);
		for(i = 0; i < call_max_count; i++){
			worker->caller->call_check_count = 0;
			worker->callee->call_check_count = 0;
			for(j = 0; j < global_workers_info.cfg.call_fail_max; j++){
				/* get callee phone number */
				if(get_gsm_info(worker->caller) != 0){
					dlog(DEBUG_LEVEL1,"\n");
					worker->caller->call_check_count++;
					worker_timewait(worker, global_workers_info.cfg.call_fail_delay);
					continue;
				}
				if(get_gsm_info(worker->callee) != 0){
					dlog(DEBUG_LEVEL1,"\n");
					worker->callee->call_check_count++;
					worker_timewait(worker, global_workers_info.cfg.call_fail_delay);
					continue;
				}
				
				ret = astrx_internal_call(worker);
				update_log(worker, ret);
				if(ret == 0){
					dlog(DEBUG_LEVEL2,"\n");
					worker->callee->call_success_count++;
					break;
				}else{
					dlog(DEBUG_LEVEL2,"\n");
					worker->caller->call_fail_count++;
					worker->callee->call_fail_count++;
					worker_timewait(worker, global_workers_info.cfg.call_fail_delay);
				}
			}
			if(j == global_workers_info.cfg.call_fail_max){
				if(worker->caller->call_check_count >= global_workers_info.cfg.call_fail_max
					|| worker->caller->call_fail_count >= 2*global_workers_info.cfg.call_fail_max
				){
					/* release caller */
					if((astrx_release_channel(worker->caller)) == 0){
						dlog(DEBUG_LEVEL1,"release callee [%s]\n", worker->caller->tech);
					}
				}
				if(worker->callee->call_check_count >= global_workers_info.cfg.call_fail_max 
					|| worker->callee->call_fail_count >= 2*global_workers_info.cfg.call_fail_max
				){
					/* release callee */
					if((astrx_release_channel(worker->callee)) == 0){
						dlog(DEBUG_LEVEL1,"release callee [%s]\n", worker->callee->tech);
					}
				}
				break;
			}
			
			dlog(DEBUG_LEVEL1,"[%d/%d/%d][%s->%s] call delay sleep %ds\n", i+1, call_max_count, worker->callee->call_success_count,
				worker->caller->tech, worker->callee->tech, 
				global_workers_info.cfg.call_next_delay);
			worker_timewait(worker, global_workers_info.cfg.call_next_delay);
		}
		
		/* 3. release callee */
		if(worker->callee->call_success_count >= call_max_count){
			/* internal call success */
			if(astrx_release_channel(worker->callee) == 0){
				dlog(DEBUG_LEVEL1,"release callee [%s]\n", worker->callee->tech);
			}
		}
		set_channel_idle(worker->caller);
		set_channel_idle(worker->callee);
		worker->caller = NULL;
		worker->callee = NULL;

		dlog(DEBUG_LEVEL1,"next call delay sleep [%d]\n", global_workers_info.cfg.call_next_delay);
		worker_timewait(worker, global_workers_info.cfg.call_next_delay);
	}
	
	return NULL;
}

static int init_config(struct incalls_config *cfg)
{
/*  file format:
----------------
[general]
enable=yes|no
maxchannel=2
outcalls=6|1-2|5-9 
incalls=1|1-2|5-9 
incall_duration=60-120 //second 

[advance] 
call_ast_type=pipe|socket
get_channel_delay=5 //second
call_fail_max=5
call_fail_delay=5 // second
call_wait_answer=30 //second
call_check_delay=5 // second
call_next_delay=10 //second
ami_timeout=10 //second
ami_reconnect_delay=30 //second
enforce_incall_sw=yes|no

----------------
*/
	char *file_path = "/etc/asterisk/gw/auto_intercall.conf";
	char line[MAX_LEN_LINE];
	char key[MAX_LEN_LINE];
	char value[MAX_LEN_LINE];
	char section[MAX_LEN_LINE];
	int val = 0, min = 0, max = 0;

	FILE* fp;
	int lock;

	/* set default value */
	/* [general] */
	cfg->enable = 0;
	cfg->maxchannel = 2;
	cfg->incalls_min = 1;
	cfg->incalls_max = 1;
	cfg->call_duration_min = 30;
	cfg->call_duration_max = 60;

	/* [dialplan] */
	strncpy(cfg->context, "internalcall", sizeof(cfg->context));
	strncpy(cfg->exten, "s", sizeof(cfg->exten));
	
	/* [advance] */
	cfg->call_ast_type = CALL_AST_SOCKET;	
	cfg->get_channel_delay = 3;
	cfg->call_fail_max = 3;
	cfg->call_fail_delay = 3;
	cfg->call_wait_answer = 60;
	cfg->call_check_delay = 3;
	cfg->call_next_delay = 3;
	cfg->ami_timeout = 10;
	cfg->ami_reconnect_delay = DEFAULT_AMI_RECONNECT_DELAY;
	cfg->enforce_incall_sw = 1;
	cfg->log_max_size = DEFAULT_LOG_MAX_SIZE;
	
	lock = lock_file(file_path);
	if( NULL == (fp = fopen(file_path,"r")) ) {
		dlog(DEBUG_LEVEL0,"Can't open %s\n",file_path);
		unlock_file(lock);
		return 1;
	}

	memset(line, 0, sizeof(line));
	memset(section, 0, sizeof(line));
	while(!feof(fp) && fgets(line,sizeof(line),fp)) {
		memset(key,0,sizeof(key));
		memset(value,0,sizeof(value));
		if(sscanf(line, "[%1024[^]]]", section) == 1){
			trim(section);
		}else if(section[0] != '\0' && sscanf(line, "%1024[^=]=%1024[^\n]", key, value) == 2){
			trim(key);
			trim(value);
			if(!strcasecmp(section,"general")){
				if(!strcasecmp(key,"enable")) {
					if(is_true(value)) {
						cfg->enable = 1;
					} else {
						cfg->enable = 0;
					}
				} else if(!strcasecmp(key,"maxchannel")) {
					if((val = atoi(value)) > 0 
						&& val < sizeof(global_workers_info.channel)/sizeof(global_workers_info.channel[0])
					){
						cfg->maxchannel = val;
						global_workers_info.worker_sum = val/2;
					}
				} else if(!strcasecmp(key,"incalls")) {
					max = min = 0;
					if(sscanf(value, "%d-%d", &min, &max) == 2){
						if(max >= min && min >= 0){
							cfg->incalls_max = max;
							cfg->incalls_min = min;
						}
					}else{
						if((val = atoi(value)) > 0){
							cfg->incalls_max = val;
							cfg->incalls_min = val;
						}
					}
				} else if(!strcasecmp(key,"incall_duration")) {
					max = min = 0;
					if(sscanf(value, "%d-%d", &min, &max) == 2){
						if(max >= min && min >= 0){
							cfg->call_duration_max = max;
							cfg->call_duration_min = min;
						}
					}else{
						if((val = atoi(value)) > 0){
							cfg->call_duration_max = val;
							cfg->call_duration_min = val;
						}
					}
				}
			}else if(!strcasecmp(section,"advance")){
				if(!strcasecmp(key,"call_ast_type")) {
					if(!strcasecmp(value,"socket")){
						cfg->call_ast_type = CALL_AST_SOCKET;
					}else if(!strcasecmp(value,"pipe")){
						cfg->call_ast_type = CALL_AST_PIPE;
					}
				}else if(!strcasecmp(key,"get_channel_delay")){
					if((val = atoi(value)) > 0){
						cfg->get_channel_delay = val;
					}
				}else if(!strcasecmp(key,"call_fail_max")){
					if((val = atoi(value)) > 0){
						cfg->call_fail_max = val;
					}
				}else if(!strcasecmp(key,"call_fail_delay")){
					if((val = atoi(value)) > 0){
						cfg->call_fail_delay = val;
					}
				}else if(!strcasecmp(key,"call_wait_answer")){
					if((val = atoi(value)) > 0){
						cfg->call_wait_answer = val;
					}
				}else if(!strcasecmp(key,"call_check_delay")){
					if((val = atoi(value)) > 0){
						cfg->call_check_delay = val;
					}
				}else if(!strcasecmp(key,"call_next_delay")){
					if((val = atoi(value)) > 0){
						cfg->call_next_delay = val;
					}
				}else if(!strcasecmp(key,"ami_timeout")){
					if((val = atoi(value)) > 0){
						cfg->ami_timeout = val;
					}
				}else if(!strcasecmp(key,"ami_reconnect_delay")){
					if((val = atoi(value)) > 0){
						cfg->ami_reconnect_delay = val;
					}
				}else if(!strcasecmp(key,"enforce_incall_sw")){
					if(is_true(value)) {
						cfg->enforce_incall_sw = 1;
					} else {
						cfg->enforce_incall_sw = 0;
					}
				}else if(!strcasecmp(key,"log_max_size")){
					if((val = atoi(value)) > 0 && val < DEFAULT_LOG_MAX_SIZE){
						cfg->log_max_size = val;
					}
				}
			}
		}
		memset(line,0,sizeof(line));
	}

	fclose(fp);
	unlock_file(lock);

	if(debug){
		printf("internal_call_config:\n"\
			"[general]\n"\
			"enable = [%d]\n"\
			"maxchannel = [%d]\n"\
			"incalls = [%d-%d]\n"\
			"call_duration = [%d-%d]\n"\
			"[debug]\n"\
			"call_ast_type = [%d]\n"\
			"get_channel_delay = [%d]\n"\
			"call_fail_max = [%d]\n"\
			"call_fail_delay = [%d]\n"\
			"call_wait_answer = [%d]\n"\
			"call_check_delay = [%d]\n"\
			"call_next_delay = [%d]\n"\
			"ami_timeout = [%d]\n"\
			"ami_reconnect_delay = [%d]\n\n",
			/* [general */
			global_workers_info.cfg.enable,
			global_workers_info.cfg.maxchannel,
			global_workers_info.cfg.incalls_min,
			global_workers_info.cfg.incalls_max,
			global_workers_info.cfg.call_duration_min,
			global_workers_info.cfg.call_duration_max,
			
			/* [advance] */
			global_workers_info.cfg.call_ast_type,
			global_workers_info.cfg.get_channel_delay,
			global_workers_info.cfg.call_fail_max,
			global_workers_info.cfg.call_fail_delay,
			global_workers_info.cfg.call_wait_answer,
			global_workers_info.cfg.call_check_delay,
			global_workers_info.cfg.call_next_delay,
			global_workers_info.cfg.ami_timeout,
			global_workers_info.cfg.ami_reconnect_delay,
			global_workers_info.cfg.enforce_incall_sw);
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	char *running_path = "/var/run/auto_intercall.ctl";
	char *pid_path = "/tmp/run/auto_intercall.pid";
	int ret = 0, i = 0, err = 0, sig = 0;
	pthread_t tid = 0;
	sigset_t mask, oldmask;

	if (argc >= 2) {
		if( 0 == memcmp(argv[1],"-d",2) ) {
			char *p;
			int j;
			debug |= 0x01;
			for ( j=1, p=argv[1]+2; *p && j<sizeof(int)*8; j++, p++) {
				if (*p == 'd') {
					debug |= (1 << j);
				}
			}
		}
	}

	if(is_running(running_path)){
		dlog(DEBUG_LEVEL0,"Please check if this program is already running.\n");
		return -1;
	}

	memset(&global_workers_info, 0, sizeof(global_workers_info));
	
	if(init_config(&global_workers_info.cfg) != 0){
		dlog(DEBUG_LEVEL0,"init config error\n");
	}

	if(global_workers_info.cfg.enable == 0){
		dlog(DEBUG_LEVEL0,"cfg disabled\n");
		return -1;
	}
	
	if(init_cluster_config(&global_workers_info.cluster_cfg) != 0){
		dlog(DEBUG_LEVEL0,"init cluster config error\n");
	}
	
	if(global_workers_info.cluster_cfg.mode != CLUSTER_MASTER && global_workers_info.cluster_cfg.mode != CLUSTER_STANDALONE){
		dlog(DEBUG_LEVEL0,"This program must run on master or standalone (cluster mode).\n");
		return -1;
	}

	if(debug == 0){
		if(daemon(1, 0) < 0){
			dlog(DEBUG_LEVEL1, "deamon error\n");
			return 1;
		}
	}

	if(write_pid(pid_path) < 0){
		dlog(DEBUG_LEVEL0,"write pid error\n");
		return -1;
	}

	srand((unsigned int)time((time_t *)NULL));

	/* set signal mask */
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGINT);
	if(pthread_sigmask(SIG_BLOCK, &mask , &oldmask) != 0){
		dlog(DEBUG_LEVEL1,"pthread_sigmask error\n");
		return -1;
	}

	dlog(DEBUG_LEVEL1,"start pthread\n");
	pthread_mutex_init(&global_workers_info.lock, NULL);
	//global_workers_info.worker_sum = 1;
	for(i = 0; i < global_workers_info.worker_sum; i++){
		/* init work node */
		pthread_mutex_init(&global_workers_info.worker[i].pth_lock, NULL);
		pthread_cond_init(&global_workers_info.worker[i].pth_cond, NULL);

		/* start work pthread */
		if((ret = pthread_create(&tid, 
				NULL, 
				(global_workers_info.cfg.call_ast_type == CALL_AST_SOCKET)?ami_worker_pthread:astrx_worker_pthread,
				&global_workers_info.worker[i])) != 0
			){
			dlog(DEBUG_LEVEL0,"pthread_create error\n");
			return -1;
		}else{
			global_workers_info.worker[i].tid = tid;
			dlog(DEBUG_LEVEL1,"create pthread [%d]\n", (int)tid);
		}
	}

	// reset
	if(sigprocmask(SIG_SETMASK , &oldmask , NULL) != 0){
		dlog(DEBUG_LEVEL1,"sigprocmask error\n");
		return -1;
	}

	while(1){
		err = sigwait(&mask , &sig);
		if(err != 0){
			error("error in sigwait");
		}
		switch(sig){
			case SIGUSR1:
			case SIGINT:
			{
				struct worker_node *worker;
				for(i = 0; i < global_workers_info.worker_sum; i++){
					worker = &global_workers_info.worker[i];
					pthread_mutex_lock(&worker->pth_lock);
					pthread_cond_signal(&worker->pth_cond);
					pthread_mutex_unlock(&worker->pth_lock);
				}
				global_workers_info.stop = 1;
				break;
			}
			case SIGUSR2:
				init_config(&global_workers_info.cfg);
				init_cluster_config(&global_workers_info.cluster_cfg);
				break;
			default:
				break;
		}

		if(global_workers_info.stop){
			break;
		}
	}
	
	for(i = 0; i < global_workers_info.worker_sum; i++){
		if(global_workers_info.worker[0].tid > 0){
			pthread_join(global_workers_info.worker[i].tid, NULL);
		}
		pthread_cond_destroy(&global_workers_info.worker[i].pth_cond);
		pthread_mutex_destroy(&global_workers_info.worker[i].pth_lock);
	}

	pthread_mutex_destroy(&global_workers_info.lock);
	return 0;
}

