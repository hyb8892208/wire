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
#define MAX_LEN_SMS_CONTENTS	256

#define SUM_SMS			10


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

#define DEFAULT_AMI_TIMEOUT		10
#define DEFAULT_AMI_RECONNECT_DELAY	30
#define DEFAULT_GSM_UP_KEYWORD	"Power on, Provisioned, Up"
#define DEFAULT_SMS_CONTENTS	"Hello World"

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
				
enum call_ast_type{
	CALL_AST_SOCKET, //communicate with ast by ami
	CALL_AST_PIPE, //communicate with ast by "asterisk -rx ..."
};

enum sms_sender_policy{
	SMS_SENDER_RANDOM,
	SMS_SENDER_ASCEND,
	SMS_SENDER_DESCEND,
};

enum used_state{
	USE_STATE_UNUSED,
	USE_STATE_USED,
};

struct cluster_config{
	enum cluster_mode mode;
	char password[MAX_LEN_NAME];
	char slave_ips[__BRD_SUM__][MAX_LEN_IP];
};

struct insms_config{
	/* internal_sms.conf [general]*/
	int enable;
	enum sms_sender_policy sender_policy;
	int reply_sw; 			// 1=receiver will reply, 0=receiver will not reply
	int receiver_counts_min;	// 
	int receiver_counts_max;	//
	int send_counts_min; 		// send sms counts min for each gsm sender with one receiver
	int send_counts_max; 		// send sms counts max for each gsm sender with one receiver
	int send_delay_min; 		// send delay min for each gsm span before next sms send
	int send_delay_max; 		// send delay max for each gsm span before next sms send
	
	/* [advance] */
	enum call_ast_type call_ast_type;
	char contents_char_list[MAX_LEN_LINE];
	int contents_len_min;
	int contents_len_max;
	int get_gsm_delay;		//second
	int at_timeout;
	int ami_timeout;		//second
	int ami_reconnect_delay;	//second
	int log_max_size;
};

struct gsm_node{
	int board;
	int span;
	char phonenumber[MAX_LEN_PHONENUMBER];	
};

struct insms_info{
	/* config settings (internal_call.conf) */
	struct insms_config cfg;
	struct cluster_config cluster_cfg;
	struct mansession ami_session; // for AMI

	/* runtime info */
	struct gsm_node gsm_map[__BRD_SUM__*__GSM_SUM__];
	int gsm_key;	
	int sender_map[__BRD_SUM__*__GSM_SUM__];
	int sender_key;
	int receiver_map[__BRD_SUM__*__GSM_SUM__];
	int receiver_key;
	int receiver_count;
};

struct ami_info{
	char host[MAX_LEN_IP];
	char username[MAX_LEN_USERNAME];
	char secret[MAX_LEN_SECRET];
	int port;
};

static struct insms_info global_insms_info;
static struct ami_info global_ami_info = {
	.host = "127.0.0.1",
	.username = AMI_SUPER_USERNAME,
	.secret = AMI_SUPER_SECTET,
	.port = AMI_SUPER_PORT,
};

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

static int is_running(const char *running_path)
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

static int file_size(const char *file_path)
{
	struct stat file_stat;
	
	stat(file_path, &file_stat);
	
	return file_stat.st_size;
}

static int file_get_contents(const char *file_path, char *buffer, int len)
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

static int file_put_contents(const char *file_path, char *buffer, int len)
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
		goto fail1;
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
		fprintf(stdout,"Unlock File %s failed.\n", file_path);
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

static int write_pid(const char *pid_path)
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

static int print_map(int level)
{
	int i = 0, step = __GSM_SUM__*__BRD_SUM__;

	if(debug & level){
		printf("sender map: ------------------------\n");
		printf("val: ");
		for(i = 0; i < step; i++){
			printf("%-3d", global_insms_info.sender_map[i]);
		}
		printf("\n");
		printf("key: ");
		for(i = 0; i < step; i++){
			printf("%-3d", i);
		}
		printf("\n");
		printf("cur: ");
		for(i = 0; i < step; i++){
			if(i == global_insms_info.sender_key){
				printf("%-3d", 1);
			}else{
				printf("%-3d", 0);
			}
		}
		printf("\n");

		printf("receiver map: -----------------------\n");
		printf("val: ");
		for(i = 0; i < step; i++){
			printf("%-3d", global_insms_info.receiver_map[i]);
		}
		printf("\nkey: ");
		for(i = 0; i < step; i++){
			printf("%-3d", i);
		}
		printf("\n");
		printf("cur: ");
		for(i = 0; i < step; i++){
			if(i == global_insms_info.receiver_key){
				printf("%-3d", 1);
			}else{
				printf("%-3d", 0);
			}
		}
		printf("\n");
	}
	return 0;

}

static int update_log(struct gsm_node *sender, struct gsm_node *receiver, char *sms_contents, char *resbuf)
{
/*
-----------------
[log datetime][from span:phonenumber][to span:phonenumber][contents][result]

-----------------
*/
	const char *file_path = "/tmp/log/auto_intersms.log";
	char buffer[MAX_LEN_BUFFER], tmp[256], result[MAX_LEN_NAME], *p = NULL;
	int fd = -1, wlen = 0, len_rest = 0, size = 0;
	time_t t;

	/* make log contents */
	if(strcasestr(resbuf, "SUCCESSFULLY")){
		strncpy(result, "SUCCESS", sizeof(result));
	}else if(strcasestr(resbuf, "FAIL")){
		strncpy(result, "FAIL", sizeof(result));
	}else if(strcasestr(resbuf, "USING")){
		strncpy(result, "SPAN USING", sizeof(result));
	}else if(resbuf[0] == '\0'){
		strncpy(result, "Wait to Send", sizeof(result));
	}else{
		strncpy(result, "Unknown", sizeof(result));
	}
	t = time(NULL);
	memset(tmp, 0, sizeof(tmp));
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&t));
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "[%s] SMS: [gsm-%d.%d:%s] ---> [gsm-%d.%d:%s]. Result: [%s]. Contents [%s]\n", 
		tmp, 
		sender->board,sender->span,sender->phonenumber,
		receiver->board,receiver->span,receiver->phonenumber,
		result,
		sms_contents);
	dlog(DEBUG_LEVEL2, "buffer:%s", buffer);

	/* update log */
	if(file_append_contents(file_path, buffer, sizeof(buffer)) != 0){
		return -1;
	}
	
	/* slim log file */
	size = file_size(file_path);
	dlog(DEBUG_LEVEL1, "size=%d default=%d max=%d\n", size, DEFAULT_LOG_MAX_SIZE, global_insms_info.cfg.log_max_size);
	if(size > global_insms_info.cfg.log_max_size){
		dlog(DEBUG_LEVEL1, "file_slim_contents(%s, %d, %d)\n", file_path, size-(global_insms_info.cfg.log_max_size/2), global_insms_info.cfg.log_max_size/2);
		file_slim_contents(file_path, size-(global_insms_info.cfg.log_max_size/2), global_insms_info.cfg.log_max_size/2);
	}

	return 0;
}

static int get_gsm_info(struct gsm_node *gsm)
{
	char file_path[256];
	char command[MAX_LEN_COMMAND];
	char buffer[MAX_LEN_BUFFER];
	char tmp[MAX_LEN_LINE];
	char *ip = NULL, *start = NULL, *end = NULL, *p = NULL;

	dlog(DEBUG_LEVEL3,"gsm-%d.%d\n", gsm->board, gsm->span);

	memset(buffer, 0, sizeof(buffer));
	if(gsm->board == 1){
		memset(file_path, 0, sizeof(file_path));
		snprintf(file_path, sizeof(file_path), "/tmp/gsm/%d", gsm->span);
		
		if(file_get_contents(file_path, buffer, sizeof(buffer)) == 0){
			//dlog(DEBUG_LEVEL5, "/tmp/gsm/%d buffer:%s\n", gsm->span, buffer);
		}else{
			dlog(DEBUG_LEVEL3,"\n");
			return -1;
		}
	}else if(gsm->board > 1 && gsm->board <= __BRD_SUM__ && global_insms_info.cluster_cfg.mode == CLUSTER_MASTER){
		ip = global_insms_info.cluster_cfg.slave_ips[gsm->board-2];
		if(ip[0] != '\0'){
			memset(command, 0, sizeof(command));
			snprintf(command, sizeof(command), "get_spans");
			if(request_slave(ip,command,strlen(command),buffer,sizeof(buffer),5) == 0){
				//dlog(DEBUG_LEVEL5, "request [%s] buffer:%s\n",ip, buffer);
				memset(tmp, 0, sizeof(tmp));
				snprintf(tmp, sizeof(tmp), "---Start Span %d---", gsm->span);
				if(NULL != (start = strstr(buffer, tmp))){
					start += strlen(tmp);
					memset(tmp, 0, sizeof(tmp));
					snprintf(tmp, sizeof(tmp), "---End Span %d---", gsm->span);
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
	
	dlog(DEBUG_LEVEL5, "board [%d] span [%d] buffer:%s\n", gsm->board, gsm->span, buffer);

	/* check gsm status */
	/*memset(tmp, 0, sizeof(tmp));
	if(get_value(buffer, "Status", tmp, sizeof(tmp)) != 0){
		return -1;
	}
	if(strncasecmp(tmp, DEFAULT_GSM_UP_KEYWORD, sizeof(DEFAULT_GSM_UP_KEYWORD)-1)){
		return -1;
	}*/

	/* get gsm phone number */
	memset(tmp, 0, sizeof(tmp));
	if(get_value(buffer, "Own Number", tmp, sizeof(tmp)) != 0){
		return -1;
	}
	if(tmp[0] != '\0'){
		strncpy(gsm->phonenumber, tmp, sizeof(gsm->phonenumber));
		dlog(DEBUG_LEVEL3, "success phnum [%s]\n", gsm->phonenumber);
		/* success */
		return 0;
	}

	//strncpy(gsm->phonenumber, "10086", sizeof(gsm->phonenumber));
	//return 0;
	
	dlog(DEBUG_LEVEL3, "fail phnum []\n");

	return -1;
}

static int get_sms_sender(struct gsm_node **sender)
{
/*
gsm pos:	3	6	19	0	11	8	...
key:		0	1	2	3	4	5	...

*/
	int gsm_key = 0, step = 0, key = 0;

	dlog(DEBUG_LEVEL1, "gsm_key [%d] sender_key [%d] receiver_key [%d]\n", 
		global_insms_info.gsm_key,
		global_insms_info.sender_key,
		global_insms_info.receiver_key);

	while(global_insms_info.receiver_count <= 0){
		global_insms_info.receiver_count = random_int(global_insms_info.cfg.receiver_counts_min, global_insms_info.cfg.receiver_counts_max);

		/* find gsm pos from map */
		step = __GSM_SUM__*__BRD_SUM__;
		switch(global_insms_info.cfg.sender_policy){
		case SMS_SENDER_ASCEND:
			global_insms_info.sender_key = (global_insms_info.sender_key+1)%step;
			gsm_key = global_insms_info.sender_map[global_insms_info.sender_key];
			break;
		case SMS_SENDER_DESCEND:
			global_insms_info.sender_key = (global_insms_info.sender_key+step-1)%step;
			gsm_key = global_insms_info.sender_map[global_insms_info.sender_key];
			break;
		case SMS_SENDER_RANDOM:
		default:
			/* Find random key */
			key = global_insms_info.sender_key+(rand()%(step-global_insms_info.sender_key));

			//exchange 
			gsm_key = global_insms_info.sender_map[key];
			global_insms_info.sender_map[key] = global_insms_info.sender_map[global_insms_info.sender_key];
			global_insms_info.sender_map[global_insms_info.sender_key] = gsm_key;

			//next.
			global_insms_info.sender_key = (global_insms_info.sender_key+1)%step;
			
			break;
		}
		
		global_insms_info.gsm_key = gsm_key;
	}
	global_insms_info.receiver_count--;
	
	dlog(DEBUG_LEVEL1, "get gsm_key [%d] send_key [%d]\n", global_insms_info.gsm_key, global_insms_info.sender_key);
	
	/* get phone number */
	if(get_gsm_info(&global_insms_info.gsm_map[global_insms_info.gsm_key]) != 0){
		print_map(DEBUG_LEVEL3);
		return -1;
	}

	
	*sender = &global_insms_info.gsm_map[global_insms_info.gsm_key];
	print_map(DEBUG_LEVEL3);

	return 0;
}

static int get_sms_receiver(struct gsm_node **receiver)
{
/*
gsm pos:	0	1	3	sender
key:		0	1	2	3	

*/

	int i = 0, step = 0, gsm_key = 0, tmp = 0, key = 0;

	dlog(DEBUG_LEVEL1, "gsm_key [%d] sender_key [%d] receiver_key [%d]\n", 
		global_insms_info.gsm_key,
		global_insms_info.sender_key,
		global_insms_info.receiver_key);	
	
	/* exchange gsm_key with the last receiver */
	step = __GSM_SUM__*__BRD_SUM__;
	for(i = 0; i < step; i++){
		if(global_insms_info.receiver_map[i] == global_insms_info.gsm_key){
			tmp = global_insms_info.receiver_map[i];
			global_insms_info.receiver_map[i] = global_insms_info.receiver_map[step-1];
			global_insms_info.receiver_map[step-1] = tmp;
			break;
		}
	}

	/* find random key */
	key = global_insms_info.receiver_key+(rand()%(step-1-global_insms_info.receiver_key));

	/* exchange */
	gsm_key = global_insms_info.receiver_map[key];	
	global_insms_info.receiver_map[key] = global_insms_info.receiver_map[global_insms_info.receiver_key];
	global_insms_info.receiver_map[global_insms_info.receiver_key] = gsm_key;

	/* next */
	global_insms_info.receiver_key = (global_insms_info.receiver_key+1)%(step-1);
	
	dlog(DEBUG_LEVEL1, "get gsm_key [%d] receiver_key [%d]\n", global_insms_info.gsm_key, global_insms_info.receiver_key);
	
	/* get phone number */
	if(get_gsm_info(&global_insms_info.gsm_map[gsm_key]) != 0){
		print_map(DEBUG_LEVEL3);
		return -1;
	}

	*receiver = &global_insms_info.gsm_map[gsm_key];
	print_map(DEBUG_LEVEL3);

	return 0;
}

static int get_sms_contents(char *sms_contents, int len)
{
	int i = 0, char_max = 0, sms_len = 0;
	char *p;

	char_max = strlen(global_insms_info.cfg.contents_char_list);
	sms_len = random_int(global_insms_info.cfg.contents_len_min, global_insms_info.cfg.contents_len_max);
	sms_len = sms_len < len ? sms_len : len;

	if(sms_len <= 0 || sms_len > 50 || char_max <= 0){
		strncpy(sms_contents, DEFAULT_SMS_CONTENTS, len);
		return 0;
	}

	for(i = 0; i < sms_len; i++){
		p = global_insms_info.cfg.contents_char_list + random_int(0, char_max);
		if(*p == '\0'){
			strncpy(sms_contents, DEFAULT_SMS_CONTENTS, len);
			return 0;
		}
		sms_contents[i] = *p;
	}
	sms_contents[i] = '\0';

	return 0;
}

static int ami_send_sms(struct mansession *s, struct gsm_node *sender, struct gsm_node *receiver)
{
	struct message msg;
	char sms_contents[MAX_LEN_NAME];
	char buffer[MAX_LEN_BUFFER];
	char command[MAX_LEN_COMMAND];
	char phonenumber[MAX_LEN_PHONENUMBER]; 
	char *ip = NULL;
	int ret = 0, board = 0, span = 0;
	
	board = sender->board;
	span = sender->span;
	strncpy(phonenumber, receiver->phonenumber, sizeof(phonenumber));

	memset(sms_contents, 0, sizeof(sms_contents));
	if(get_sms_contents(sms_contents, sizeof(sms_contents)) != 0){
		return ASTMAN_FAILURE;
	}

	if(board == 1){
		memset(command, 0, sizeof(command));
		snprintf(command, sizeof(command), "gsm send sync sms %d %s \"%s\" %d", 
			span, 
			phonenumber,
			sms_contents,
			global_insms_info.cfg.at_timeout*1000);
		dlog(DEBUG_LEVEL1, "command [%s]\n", command);
		memset(&msg, 0, sizeof(msg));
		if((ret = astman_command(s, &msg, command, NULL, global_insms_info.cfg.ami_timeout)) != ASTMAN_SUCCESS){
			return ret;
		}
		print_astmsg(DEBUG_LEVEL1, &msg);
	}else if(board > 1 && board <= __BRD_SUM__ && global_insms_info.cluster_cfg.mode == CLUSTER_MASTER){
		ip = global_insms_info.cluster_cfg.slave_ips[board-2];
		if(ip[0] != '\0'){
			memset(command, 0, sizeof(command));
			snprintf(command, sizeof(command), "astcmd:gsm send sync sms %d %s \"%s\" %d",
				span, 
				phonenumber,
				sms_contents,
				global_insms_info.cfg.at_timeout*1000);
			dlog(DEBUG_LEVEL1,"command [%s][%s]\n", ip, command);
			if(request_slave(ip,command,strlen(command),buffer,sizeof(buffer),global_insms_info.cfg.ami_timeout) != 0){
				dlog(DEBUG_LEVEL3,"\n");
				return ASTMAN_FAILURE;
			}
			dlog(DEBUG_LEVEL1, "request [%s] buffer:%s\n",ip, buffer);
		}
	}

	/* update log */
	update_log(sender, receiver, sms_contents, buffer);

	return ASTMAN_SUCCESS;
}

static int ami_internal_sms(void)
{
	struct mansession *s = &global_insms_info.ami_session;
	struct message msg;
	struct gsm_node *sender = NULL, *receiver = NULL;
	int send_counts = 0, send_delay = 0;
	int i = 0, j = 0, ret = 0;
	
	for(;;){
		if(ASTMAN_SUCCESS != astman_login(s, NULL, global_ami_info.port, global_ami_info.username, global_ami_info.secret)){
			goto ami_close;
		}
		s->debug = debug;
		dlog(DEBUG_LEVEL1,"astman_login success\n");
		
		/* events off */
		memset(&msg, 0, sizeof(msg));
		ret = astman_events(s, &msg, "off", NULL, global_insms_info.cfg.ami_timeout);
		if(ret == ASTMAN_SOCKCLOSED){
			goto ami_close;
		}

		for(;;){
			while(get_sms_sender(&sender) != 0){
				if(global_insms_info.sender_key == 0){
					dlog(DEBUG_LEVEL1, "sleep [%d]\n", global_insms_info.cfg.get_gsm_delay);
					sleep(global_insms_info.cfg.get_gsm_delay);
				}
				continue;
			}

			while(get_sms_receiver(&receiver) != 0){
				if(global_insms_info.receiver_key == 0){
					dlog(DEBUG_LEVEL1, "sleep [%d]\n", global_insms_info.cfg.get_gsm_delay);
					sleep(global_insms_info.cfg.get_gsm_delay);
				}
				continue;
			}

			dlog(DEBUG_LEVEL1, "[%d/%d] new receiver [gsm-%d.%d:%s]->[gsm-%d.%d:%s]\n", i+1, global_insms_info.receiver_count+1, 
				sender->board, sender->span, sender->phonenumber, 
				receiver->board, receiver->span, receiver->phonenumber);


			send_counts = random_int(global_insms_info.cfg.send_counts_min, global_insms_info.cfg.send_counts_max);
			for(j = 0; j < send_counts; j++){
				ret = ami_send_sms(s, sender, receiver);
				if(ret == ASTMAN_SOCKCLOSED){
					dlog(DEBUG_LEVEL1, "Socket closed . Goto logoff\n");
					goto ami_close;
				}
				send_delay = random_int(global_insms_info.cfg.send_delay_min, global_insms_info.cfg.send_delay_max);
				dlog(DEBUG_LEVEL1, "[%d/%d] send over sleep [%d]\n", j+1, send_counts, send_delay);
				sleep(send_delay);
				if(global_insms_info.cfg.reply_sw){
					ret = ami_send_sms(s, receiver, sender);
					if(ret == ASTMAN_SOCKCLOSED){
						dlog(DEBUG_LEVEL1, "Socket closed . Goto logoff\n");
						goto ami_close;
					}
					send_delay = random_int(global_insms_info.cfg.send_delay_min, global_insms_info.cfg.send_delay_max);
					dlog(DEBUG_LEVEL1, "[%d/%d] reply over sleep [%d]\n", j+1, send_counts, send_delay);
					sleep(send_delay);
				}
			}
		}
ami_close:				
		dlog(DEBUG_LEVEL1,"ami close\n");
		astman_close(s);
		sleep(global_insms_info.cfg.ami_reconnect_delay);
	}

	return 0;
}

static int astrx_send_sms(struct gsm_node *sender, struct gsm_node *receiver)
{
	char sms_contents[MAX_LEN_NAME];
	char buffer[MAX_LEN_BUFFER];
	char command[MAX_LEN_COMMAND];
	char *ip = NULL;
	char phonenumber[MAX_LEN_PHONENUMBER]; 
	int board = 0, span = 0;

	board = sender->board;
	span = sender->span;
	strncpy(phonenumber, receiver->phonenumber, sizeof(phonenumber));
	
	memset(sms_contents, 0, sizeof(sms_contents));
	if(get_sms_contents(sms_contents, sizeof(sms_contents)) != 0){
		return -1;
	}
	
	if(board == 1){
		memset(command, 0, sizeof(command));
		snprintf(command, sizeof(command), "asterisk -rx \"gsm send sync sms %d %s \\\"%s\\\" %d\"", 
			span, 
			phonenumber,
			sms_contents,
			global_insms_info.cfg.at_timeout*1000);
		dlog(DEBUG_LEVEL1,"command [%s]\n", command);
		if( pipe_exec(command, buffer, sizeof(buffer)) != 0){
			dlog(DEBUG_LEVEL1,"command fail [%s]\n", command);
			return -1;
		}
		dlog(DEBUG_LEVEL1, "send over . buffer [%s]\n", buffer);
	}else if(board > 1 && board <= __BRD_SUM__ && global_insms_info.cluster_cfg.mode == CLUSTER_MASTER){
		ip = global_insms_info.cluster_cfg.slave_ips[board-2];
		if(ip[0] != '\0'){
			memset(command, 0, sizeof(command));
			snprintf(command, sizeof(command), "astcmd:gsm send sync sms %d %s \"%s\" %d",
				span, 
				phonenumber,
				sms_contents,
				global_insms_info.cfg.at_timeout*1000);
			dlog(DEBUG_LEVEL1,"command [%s][%s]\n", ip, command);
			if(request_slave(ip,command,strlen(command),buffer,sizeof(buffer),global_insms_info.cfg.ami_timeout) != 0){
				dlog(DEBUG_LEVEL3,"\n");
				return -1;
			}
			dlog(DEBUG_LEVEL1, "request [%s] buffer:%s\n",ip, buffer);
		}
	}

	dlog(DEBUG_LEVEL1, "\n");

	update_log(sender, receiver, sms_contents, buffer);
	
	return 0;
}

static int astrx_internal_sms(void)
{
	struct gsm_node *sender = NULL, *receiver = NULL;
	int send_counts = 0, send_delay = 0;
	int i = 0, j = 0;
	
	for(;;){
		while(get_sms_sender(&sender) != 0){
			if(global_insms_info.sender_key == 0){
				dlog(DEBUG_LEVEL1, "sleep [%d]\n", global_insms_info.cfg.get_gsm_delay);
				sleep(global_insms_info.cfg.get_gsm_delay);
			}
			continue;
		}

		while(get_sms_receiver(&receiver) != 0){
			if(global_insms_info.receiver_key == 0){
				dlog(DEBUG_LEVEL1, "sleep [%d]\n", global_insms_info.cfg.get_gsm_delay);
				sleep(global_insms_info.cfg.get_gsm_delay);
			}
			continue;
		}

		dlog(DEBUG_LEVEL1, "[%d/%d] new receiver [gsm-%d.%d:%s]->[gsm-%d.%d:%s]\n", i+1, global_insms_info.receiver_count+1, 
			sender->board, sender->span, sender->phonenumber, 
			receiver->board, receiver->span, receiver->phonenumber);

		send_counts = random_int(global_insms_info.cfg.send_counts_min, global_insms_info.cfg.send_counts_max);
		for(j = 0; j < send_counts; j++){
			astrx_send_sms(sender, receiver);
			send_delay = random_int(global_insms_info.cfg.send_delay_min, global_insms_info.cfg.send_delay_max);
			dlog(DEBUG_LEVEL1, "[%d/%d] send over sleep [%d]\n", j+1, send_counts, send_delay);
			sleep(send_delay);
			if(global_insms_info.cfg.reply_sw){
				astrx_send_sms(receiver, sender);
				send_delay = random_int(global_insms_info.cfg.send_delay_min, global_insms_info.cfg.send_delay_max);
				dlog(DEBUG_LEVEL1, "[%d/%d] reply over sleep [%d]\n", j+1, send_counts, send_delay);
				sleep(send_delay);
			}
		}
	}

	return 0;
}

static int init_config(struct insms_config *cfg)
{
/*  file format:
----------------
[general]
enable=yes|no
sender_policy=random|ascend|descend
reply_sw=yes|no
send_counts_min=1
send_counts_max=5
send_delay_min=60
send_delay_max=120

[advance]
call_ast_type=pipe|socket
contents_char_list=abc
contents_len_max=50
contents_len_min=10
get_gsm_delay=1
ami_timeout=10		//second
ami_reconnect_delay=30	//second

----------------
*/
	char *file_path = "/etc/asterisk/gw/auto_intersms.conf";
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
	cfg->sender_policy = SMS_SENDER_RANDOM;
	cfg->reply_sw = 1;
	cfg->receiver_counts_min = 1;
	cfg->receiver_counts_max = 2;
	cfg->send_counts_min = 1;
	cfg->send_counts_max = 3;
	cfg->send_delay_min = 5;
	cfg->send_delay_max = 10;
	/* [advance] */
	cfg->call_ast_type = CALL_AST_PIPE;
	cfg->contents_len_min = 10;
	cfg->contents_len_max = 20;
	cfg->get_gsm_delay = 3;
	cfg->at_timeout = 5;
	cfg->ami_timeout = DEFAULT_AMI_TIMEOUT;
	cfg->ami_reconnect_delay = DEFAULT_AMI_RECONNECT_DELAY;
	cfg->log_max_size = DEFAULT_LOG_MAX_SIZE;
	strncpy(cfg->contents_char_list, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890", sizeof(cfg->contents_char_list));
	
	lock = lock_file(file_path);
	if(NULL == (fp = fopen(file_path,"r"))) {
		dlog(DEBUG_LEVEL0,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
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
				} else if(!strcasecmp(key,"reply_sw")) {
					if(is_true(value)) {
						cfg->reply_sw = 1;
					} else {
						cfg->reply_sw = 0;
					}
				} else if(!strcasecmp(key,"sender_policy")) {
					if(!strcasecmp(value,"random")){
						cfg->call_ast_type = SMS_SENDER_RANDOM;
					}else if(!strcasecmp(value,"ascend")){
						cfg->call_ast_type = SMS_SENDER_ASCEND;
					}else if(!strcasecmp(value,"descend")){
						cfg->call_ast_type = SMS_SENDER_DESCEND;
					}
				}else if(!strcasecmp(key,"receiver_counts_min")){
					if((val = atoi(value)) > 0){
						cfg->receiver_counts_min = val;
					}
				}else if(!strcasecmp(key,"receiver_counts_max")){
					if((val = atoi(value)) > 0){
						cfg->receiver_counts_max = val;
					}				
				}else if(!strcasecmp(key,"send_counts_min")){
					if((val = atoi(value)) > 0){
						cfg->send_counts_min = val;
					}
				}else if(!strcasecmp(key,"send_counts_max")){
					if((val = atoi(value)) > 0){
						cfg->send_counts_max = val;
					}
				}else if(!strcasecmp(key,"send_delay_min")){
					if((val = atoi(value)) > 0){
						cfg->send_delay_min = val;
					}
				}else if(!strcasecmp(key,"send_delay_max")){
					if((val = atoi(value)) > 0){
						cfg->send_delay_max = val;
					}
				}
			}else if(!strcasecmp(section,"advance")){
				if(!strcasecmp(key,"call_ast_type")) {
					if(!strcasecmp(value,"socket")){
						cfg->call_ast_type = CALL_AST_SOCKET;
					}else if(!strcasecmp(value,"pipe")){
						cfg->call_ast_type = CALL_AST_PIPE;
					}
				}else if(!strcasecmp(key,"contents_char_list")){
					if(value[0] != '\0') {
						strncpy(cfg->contents_char_list, value, sizeof(cfg->contents_char_list));
					}
				}else if(!strcasecmp(key,"contents_len_min")){
					if((val = atoi(value)) > 0){
						cfg->contents_len_min = val;
					}
				}else if(!strcasecmp(key,"contents_len_max")){
					if((val = atoi(value)) > 0){
						cfg->contents_len_max = val;
					}
				}else if(!strcasecmp(key,"at_timeout")){
					if((val = atoi(value)) > 0){
						cfg->at_timeout = val;
					}
				}else if(!strcasecmp(key,"ami_timeout")){
					if((val = atoi(value)) > 0){
						cfg->ami_timeout = val;
					}
				}else if(!strcasecmp(key,"get_gsm_delay")){
					if((val = atoi(value)) > 0){
						cfg->get_gsm_delay = val;
					}
				}else if(!strcasecmp(key,"ami_reconnect_delay")){
					if((val = atoi(value)) > 0){
						cfg->ami_reconnect_delay = val;
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
		printf("internal_sms_config:\n"\
			"[general]\n"\
			"enable = [%d]\n"\
			"sender_policy = [%d]\n"\
			"reply_sw = [%d]\n"\
			"receiver_counts_min = [%d]\n"\
			"receiver_counts_max = [%d]\n"\
			"send_counts_min = [%d]\n"\
			"send_counts_max = [%d]\n"\
			"send_delay_min = [%d]\n"\
			"send_delay_max = [%d]\n"\
			"[advance]\n"\
			"call_ast_type = [%d]\n"\
			"contents_char_list = [%s]\n"\
			"contents_len_min = [%d]\n"\
			"contents_len_max = [%d]\n"\
			"at_timeout = [%d]\n"\
			"ami_timeout = [%d]\n"\
			"ami_reconnect_delay = [%d]\n\n",
			/* [general */
			global_insms_info.cfg.enable,
			global_insms_info.cfg.sender_policy,
			global_insms_info.cfg.reply_sw,
			global_insms_info.cfg.receiver_counts_min,
			global_insms_info.cfg.receiver_counts_max,
			global_insms_info.cfg.send_counts_min,
			global_insms_info.cfg.send_counts_max,
			global_insms_info.cfg.send_delay_min,
			global_insms_info.cfg.send_delay_max,
			
			/* [advance] */
			global_insms_info.cfg.call_ast_type,
			global_insms_info.cfg.contents_char_list,
			global_insms_info.cfg.contents_len_min,
			global_insms_info.cfg.contents_len_max,
			global_insms_info.cfg.at_timeout,
			global_insms_info.cfg.ami_timeout,
			global_insms_info.cfg.ami_reconnect_delay);
	}
	
	return 0;
}

static void signal_handle(int sig)
{
	switch(sig){
	case SIGUSR1:
		exit(0);
		break;
	case SIGUSR2:
		init_config(&global_insms_info.cfg);
		init_cluster_config(&global_insms_info.cluster_cfg);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	const char *running_path = "/var/run/auto_intersms.ctl";
	const char *pid_path = "/tmp/run/auto_intersms.pid";
	int ret = 0, i = 0;

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

	memset(&global_insms_info, 0, sizeof(global_insms_info));
	if(init_config(&global_insms_info.cfg) != 0){
		dlog(DEBUG_LEVEL0,"init config error\n");
	}

	if(global_insms_info.cfg.enable == 0){
		dlog(DEBUG_LEVEL0,"cfg disabled\n");
		return -1;
	}
	
	if(init_cluster_config(&global_insms_info.cluster_cfg) != 0){
		dlog(DEBUG_LEVEL0,"init cluster config error\n");
	}
	
	if(global_insms_info.cluster_cfg.mode != CLUSTER_MASTER && global_insms_info.cluster_cfg.mode != CLUSTER_STANDALONE){
		dlog(DEBUG_LEVEL0,"This program must run on master or standalone (cluster mode).\n");
		return -1;
	}

	/* start daemon */
	if(debug == 0){
		if(daemon(1, 0) != 0){
			return -1;
		}
	}

	if(write_pid(pid_path) < 0){
		dlog(DEBUG_LEVEL0,"write pid error\n");
		return -1;
	}

	srand((unsigned int)time((time_t *)NULL));
	signal(SIGUSR1, signal_handle); // for stop 
	signal(SIGUSR2, signal_handle); //for reload

	for(i = 0; i < __GSM_SUM__*__BRD_SUM__; i++){
		global_insms_info.gsm_map[i].board = 1 + ( i / __GSM_SUM__ );
		global_insms_info.gsm_map[i].span = 1 + ( i % __GSM_SUM__ );
		global_insms_info.sender_map[i] = i;
		global_insms_info.receiver_map[i] = i;
	}

	if(global_insms_info.cfg.call_ast_type == CALL_AST_SOCKET){
		ami_internal_sms();
	}else{
		astrx_internal_sms();
	}
	
	return 0;
}

