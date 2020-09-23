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
#include <errno.h>
#include <sys/stat.h>
#include "astman.h"

#define MAX_LEN_IP 		16
#define MAX_LEN_NAME		128
#define MAX_LEN_COMMAND		1024
#define MAX_LEN_USERNAME 	128
#define MAX_LEN_SECRET 		128
#define MAX_LEN_BUFFER 		4096
#define MAX_LEN_LINE 		1024

#define RETINT_SUCCESS 		0
#define RETINT_FAILURE 		-1

#define AMI_COMMAND_MAX 	256
#define AMI_SUPER_USERNAME 	"internalspecifyuser"
#define AMI_SUPER_SECTET 	"2rujzdndyznbg7u6xju"
#define AMI_SUPER_PORT 		5038

#define __GSM_SUM__ 	4
#define __BRD_SUM__		5
#define __BRD_HEAD__	"Board-"
#define MAX_CELL 		7 
#define MAX_LOCK_CELL		3

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

#define DEBUG_SPAN1 (1<<11)
#define DEBUG_SPAN2 (1<<12)
#define DEBUG_SPAN3 (1<<13)
#define DEBUG_SPAN4 (1<<14)

static int debug = 0;
#define dlog(level,format,...) \
	do { \
		if(level & debug){ \
			time_t t = time(NULL); \
			char buf[256]; \
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t)); \
			fprintf(stdout, "[%s][ID:%6d][%s %d %s] "format"", buf, pthread_self(),\
				__FILE__, __LINE__, __FUNCTION__, \
				##__VA_ARGS__); \
		} \
	}while(0)

/************************************************/
struct cell_info { 
	int cell; 
	char arfcn[10];		//absolute radio frequency channel number 
	int rxl;		//receive level 
	int rxq;		//receive quality 
	char mcc[10];		//mobile country code 
	char mnc[10];		//mobile network code 
	int bsic;		//base station identity code 
	char cellid[10];	//cell id 
	int rla;		//receive level access minimum 
	int txp;		//transmit power maximum CCCH 
	char lac[10];		//location area code 
	int TA;			//Timing Advance 
	int count;
};

struct cell_counter{
	char arfcn[10]; 	//absolute radio frequency channel number 
	int count;
};

enum cluster_mode{
	CLUSTER_MASTER, // master
	CLUSTER_STANDALONE, // standalone
	CLUSTER_SLAVE,	 //slave
	CLUSTER_OTHER,
};

enum cs_type {
	CELL_SELECT_RANDOM,
	CELL_SELECT_ASCEND,
	CELL_SELECT_DESCEND,
};

enum wl_type {
	WHEN_LOCK_RANDOM_TIME,
	WHEN_LOCK_RANDOM_CALLS,
};

enum ca_type{
	CALL_AST_SOCKET, //communicate with ast by ami
	CALL_AST_PIPE, //communicate with ast by "asterisk -rx ..."
};

struct cluster_config{
	enum cluster_mode mode;
	char password[MAX_LEN_NAME];
	char slave_ips[__BRD_SUM__][MAX_LEN_IP];
};

struct gsm_node{
	int span;
	pthread_t tid;
	pthread_mutex_t cell_lock;
	pthread_mutex_t pth_lock;
	pthread_cond_t pth_cond;
	struct cell_info cell[MAX_CELL];
	struct cell_counter cell_count[MAX_CELL]; // increase when cell is locked
	int cell_sum;
	int cell_cur;
	int cell_next;
};


#define DEFAULT_RECONNECT_AMI_TIME 30
#define DEFAULT_REGET_CELL_READY_TIME 2
#define DEFAULT_CELL_READY_KEYWORD "Status: Power on, Provisioned, Up"

struct gsm_infomation{
	/* auto_lock_cell.conf [general]*/
	int enable;
	int max_lock;	// max value of locked cell at the same time
	enum cs_type cell_select_type;
	enum wl_type when_lock_type;

	int signal_threshold;
	/* auto_lock_cell.conf [general]*/
	int when_min;
	int when_max;

	/* auto_lock_cell.conf  [debug]*/
	int at_counts; //send at command fail max times
	int at_timeout; //send at command timeout
	int get_cells_from_at; //1=update cells info by at command, 0=update cells info by "gsm show cells"
	enum ca_type call_ast_type;
	int cell_mcc_check;
	int cell_mnc_check;
	int find_cell_fail_counts;
	int reconnect_ami_time;     //Second
	int reget_cell_ready_time;
	char cell_ready_keyword[1024];
	int immediate; // 0=wait no call, 1=lock cell immediate
	int log_max_size;

	/* cluster config */
	struct cluster_config cluster_cfg;
	int board;

	/* runtime info */
	struct gsm_node gsm[__GSM_SUM__];	
};

struct ami_infomation{
	char host[MAX_LEN_IP];
	char username[MAX_LEN_USERNAME];
	char secret[MAX_LEN_SECRET];
	int port;
};

static struct gsm_infomation global_gsm_info;
static struct ami_infomation global_ami_info = {
	.host = "127.0.0.1",
	.username = AMI_SUPER_USERNAME,
	.secret = AMI_SUPER_SECTET,
	.port = AMI_SUPER_PORT,
};

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

static const char *str_replace(char *path)
{
	int i;
	int len = strlen(path);
	for(i=0; i<len; i++) {
		if(path[i] == '/') {
			path[i] = '_';
		}
	}
	return path;
}

static int lock_file(const char* path)
{
	char temp_path[256];
	char lock_path[256];
	int fd;

	snprintf(temp_path,256,"%s",path);
	snprintf(lock_path,256,"/tmp/lock/%s.lock",str_replace(temp_path));

	fd = open(lock_path, O_WRONLY|O_CREAT);
	if(fd <= 0) {
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

static int file_size(const char *file_path)
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
			if(0 == strcasecmp(section,"general")){
				if(0 == strcasecmp(key,"mode")) {
					if(0 == strcasecmp(value,"standalone")){
						cfg->mode = CLUSTER_STANDALONE;
					}else if(0 == strcasecmp(value,"master")){
						cfg->mode = CLUSTER_MASTER;
					
					}else if(0 == strcasecmp(value,"slave")){
						cfg->mode = CLUSTER_SLAVE;
					}else{
						cfg->mode = CLUSTER_OTHER;
					}
				}
			}else if(0 == strcasecmp(section,"master")){
				if(0 == strcasecmp(key,"password")) {
					strncpy(cfg->password,value,sizeof(cfg->password));
				}
			}else if(0 == strcasecmp(section,"slavelist")){
				int b;
				for (b=2; b<=__BRD_SUM__; b++) {
					char board_str[32];
					memset(board_str,0,sizeof(board_str));
					snprintf(board_str, sizeof(board_str), "%s%d_ip", __BRD_HEAD__, b);
					if(0 == strcasecmp(key,board_str)) {
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

static int update_log(struct gsm_node *gsm, int cell_num, int result)
{
/*
-----------------
[log datetime][gsm-x.x][cellid:arfcn][result]

-----------------
*/
	char *file_path = "/tmp/log/auto_lock_cell.log";
	char buffer[MAX_LEN_BUFFER], tmp[256], *p = NULL;
	int fd = -1, wlen = 0, len_rest = 0, size = 0;
	time_t t;

	/* make log contents */
	t = time(NULL);
	memset(tmp, 0, sizeof(tmp));
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&t));
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "[%s] [gsm-%d.%d] locked [%d:%s]. Result: [%s]\n", 
		tmp, 
		global_gsm_info.board, gsm->span, cell_num, gsm->cell[cell_num].arfcn,
		result==0?"SUCCESS":"FAIL");
	dlog(DEBUG_LEVEL2, "buffer:%s", buffer);

	/* update log */
	if(file_append_contents(file_path, buffer, sizeof(buffer)) != 0){
		return -1;
	}
	
	/* slim log file */
	size = file_size(file_path);
	dlog(DEBUG_LEVEL1, "size=%d default=%d max=%d\n", size, DEFAULT_LOG_MAX_SIZE, global_gsm_info.log_max_size);
	if(size > global_gsm_info.log_max_size){
		dlog(DEBUG_LEVEL1, "file_slim_contents(%s, %d, %d)\n", file_path, size-(global_gsm_info.log_max_size/2), global_gsm_info.log_max_size/2);
		file_slim_contents(file_path, size-(global_gsm_info.log_max_size/2), global_gsm_info.log_max_size/2);
	}

	return 0;
}


static int is_cell_ready(struct gsm_node *gsm)
{
	char file_path[256];
	char buffer[MAX_LEN_BUFFER];
	char *find = NULL;
	char keyword[1024];
	char *p;
	
	snprintf(file_path, sizeof(file_path), "/tmp/gsm/%d", gsm->span);
	if(file_get_contents(file_path, buffer, sizeof(buffer)) < 0){
		return 0;
	}

	strncpy(keyword,global_gsm_info.cell_ready_keyword,sizeof(keyword));
	p = keyword;

	while ( (find = strchr(p,'|')) ) {
		*find = '\0';
		if (strlen(p) > 0) {
			if(strstr(buffer, p)){
				return 1;
			}else{
				return 0;
			}
		}
		p = find + 1;
	}

	if (strlen(p) > 0) {
		if(strstr(buffer, p)){
			return 1;
		}else{
			return 0;
		}
	}

	return 0;
}

static int increase_cell_count(struct gsm_node *gsm)
{
	int i = 0;

	dlog(DEBUG_LEVEL3,"cell_cur [%d] arfcn [%s]\n", gsm->cell_cur, gsm->cell[gsm->cell_cur].arfcn);

	if(strlen(gsm->cell[gsm->cell_cur].arfcn) > 0){
		for(i = 0; i < gsm->cell_sum; i++){
			dlog(DEBUG_LEVEL3,"i [%d] arfcn [%s]\n", i, gsm->cell_count[i].arfcn);
			if(strlen(gsm->cell_count[i].arfcn) > 0 
				&& !strcasecmp(gsm->cell[gsm->cell_cur].arfcn, gsm->cell_count[i].arfcn)){
				gsm->cell_count[i].count++;
			}
		}
	}
	return 0;
}

static int update_cell_count(struct gsm_node *gsm, int cell_num)
{
	int i = 0;

	if(cell_num >= 0 && cell_num < gsm->cell_sum && strlen(gsm->cell[cell_num].arfcn) > 0){
		for(i = 0; i < gsm->cell_sum; i++){
			if(strlen(gsm->cell_count[i].arfcn) > 0 
				&& !strcasecmp(gsm->cell[cell_num].arfcn, gsm->cell_count[i].arfcn)){
				gsm->cell[cell_num].count = gsm->cell_count[i].count;
			}
		}
	}
	return 0;
}

static int write_cells_info_file()
{
/*    file format:
-----------------
{
  "1":[{
            "0":[{"cell":"2","arfcn":"4","rxl":"21212","rxq":"212121","mcc":"21","mnc":"21","bsic":"221","cellid":"1","rla":"221","txp":"21","lac":"21","TA":"21","active":"0"}],
             ...
            "4":[{"cell":"2","arfcn":"4","rxl":"21212","rxq":"212121","mcc":"21","mnc":"21","bsic":"221","cellid":"1","rla":"221","txp":"21","lac":"21","TA":"21","active":"1"}]
   }],
   ...
   "4":[{
              ...
   }]
}
-----------------
*/

	char *file_path = "/tmp/auto_lock_cell/cells_info";
	struct gsm_node *gsm;
	struct cell_info *cell;
	char buffer[MAX_LEN_BUFFER*__GSM_SUM__];
	char tmp[MAX_LEN_BUFFER];
	int fd = -1, len = 0, i = 0, j = 0;
	
	dlog(DEBUG_LEVEL3,"\n");

	/* 1. set buffer string */
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "{");
	for(i = 0; i < __GSM_SUM__; i++){
		gsm = &global_gsm_info.gsm[i];
		dlog(DEBUG_LEVEL3,"i=[%d]\n", i);
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, sizeof(tmp), "\"%d\":[{", gsm->span);
		strncat(buffer, tmp, sizeof(buffer)-strlen(buffer)-1);

		pthread_mutex_lock(&gsm->cell_lock);
		for(j = 0; j < MAX_CELL; j++){
			cell = &gsm->cell[j];

			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "\"%d\":[{"\
					"\"cell\":\"%d\","\
					"\"arfcn\":\"%s\","\
					"\"rxl\":\"%d\","\
					"\"rxq\":\"%d\","\
					"\"mcc\":\"%s\","\
					"\"mnc\":\"%s\","\
					"\"bsic\":\"%d\","\
					"\"cellid\":\"%s\","\
					"\"rla\":\"%d\","\
					"\"txp\":\"%d\","\
					"\"lac\":\"%s\","\
					"\"TA\":\"%d\","\
					"\"cellcount\":\"%d\","\
					"\"active\":\"%d\""\
				"}]",
				j,
				cell->cell,
				cell->arfcn[0]?cell->arfcn:"0",
				cell->rxl,
				cell->rxq,
				cell->mcc[0]?cell->mcc:"0",
				cell->mnc[0]?cell->mnc:"0",
				cell->bsic,
				cell->cellid[0]?cell->cellid:"0",
				cell->rla,
				cell->txp,
				cell->lac[0]?cell->lac:"0",
				cell->TA,
				cell->count,
				(cell->arfcn[0] && j==gsm->cell_cur)?1:0);
			strncat(buffer, tmp, sizeof(buffer)-strlen(buffer)-1);
			
			if(j < MAX_CELL - 1){
				strncat(buffer, ",", sizeof(buffer)-strlen(buffer)-1);
			}
		}
		pthread_mutex_unlock(&gsm->cell_lock);
		
		if(i < __GSM_SUM__ - 1){
			strncat(buffer, "}],", sizeof(buffer)-strlen(buffer)-1);
		}else{
			strncat(buffer, "}]", sizeof(buffer)-strlen(buffer)-1);
		}
	}
	strncat(buffer, "}", sizeof(buffer)-strlen(buffer)-1);

	/* 2. write cells_info file */
	if((fd = open(file_path, O_CREAT|O_WRONLY|O_TRUNC, 0664)) < 0) {
		dlog(DEBUG_LEVEL1,"Unable to open %s: %s\n", file_path, strerror(errno));
		return -1;
	}
	if(flock(fd, LOCK_EX) < 0) {
		dlog(DEBUG_LEVEL1,"File %s was not locked.\n",file_path);
		close(fd);
		return -1;
	}

	len = write(fd,buffer,strlen(buffer));

	if((flock(fd, LOCK_UN)) < 0) {
		dlog(DEBUG_LEVEL1,"File %s unlocked error.\n",file_path);
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

static int get_wait_interval(void)
{
	int wait_interval = 24*60*60;

	if(global_gsm_info.when_lock_type == WHEN_LOCK_RANDOM_CALLS){
		wait_interval = 24*60*60;//wait one day
	}else if(global_gsm_info.when_lock_type == WHEN_LOCK_RANDOM_TIME){
		srand((unsigned int)time((time_t *)NULL));
		wait_interval = (rand() % (global_gsm_info.when_max- global_gsm_info.when_min)) + global_gsm_info.when_min;
	}

	return wait_interval;
}

static int get_new_cell(struct gsm_node *gsm)
{
	struct cell_info cell_cur = gsm->cell[gsm->cell_cur];
	struct cell_info cell_tmp;
	char cur_arfcn[10];
	int cell_next = gsm->cell_next;
	int step = gsm->cell_sum;
	int i = 0;

	dlog(DEBUG_LEVEL3,"[span=%d] cur cell [%d] next cell [%d]\n", gsm->span, gsm->cell_cur,	gsm->cell_next);

	if(step <= 0){
		return -1;
	}

	/* calc next cell */
	for(i = 0; i < global_gsm_info.find_cell_fail_counts; i++){
		while(cell_next == gsm->cell_cur || cell_next == gsm->cell_next){
			if(global_gsm_info.cell_select_type == CELL_SELECT_RANDOM){
				srand((unsigned int)time((time_t *)NULL));
				cell_next = rand() % step;
			}else if(global_gsm_info.cell_select_type == CELL_SELECT_ASCEND){
				cell_next = (cell_next + 1) % step;
			}else if(global_gsm_info.cell_select_type == CELL_SELECT_DESCEND){
				cell_next = (cell_next + step - 1) % step;
			}else{
				cell_next = (cell_next + 1) % step;
			}
		}
		cell_tmp = gsm->cell[cell_next];
		if(atoi(cell_tmp.arfcn) <= 0 
			|| 0 == strcasecmp(cell_cur.arfcn, cell_tmp.arfcn)
			|| cell_tmp.rxl < global_gsm_info.signal_threshold){
			continue;
		}
		if(global_gsm_info.cell_mcc_check && 0 != strcasecmp(cell_cur.mcc, cell_tmp.mcc)){
			continue;
		}
		if(global_gsm_info.cell_mnc_check && 0 != strcasecmp(cell_cur.mnc, cell_tmp.mnc)){
			continue;
		}
		break;
	}

	if(i == MAX_CELL){
		return -1;
	}
	
	gsm->cell_next = cell_next;
	
	dlog(DEBUG_LEVEL3,"[span=%d] cur cell [%d] next cell [%d]\n", gsm->span, gsm->cell_cur,	gsm->cell_next);

	return cell_next;
}

static int get_lock_cell_cli(struct gsm_node *gsm, char *command, int len, int *new_cell)
{
	int i = 0, j = 0, arfcn = 0;
	int cell_map[MAX_CELL], cell_start_key = 0, cell_new_value = 0, cell_new_key = 0;

	if(gsm->cell_sum < 1){
		return -1;
	}

	for(i = 0; i < sizeof(cell_map)/sizeof(cell_map[0]); i++){
		cell_map[i] = i;
	}

	snprintf(command, len, "gsm lock cell %d ",gsm->span);
	for(i = 0, j = 0, cell_start_key = 0, *new_cell = -1; i < gsm->cell_sum && j < global_gsm_info.max_lock; i++){
		/* get random new cell num from start key to cell sum */
		cell_new_key = random_int(cell_start_key, gsm->cell_sum);
		cell_new_value = cell_map[cell_new_key];
		dlog(DEBUG_LEVEL3, "random_int start [%d] new_key [%d] new_value [%d] arfcn [%s]\n", 
			cell_start_key, 
			cell_new_key, 
			cell_new_value,
			gsm->cell[cell_new_value].arfcn);
		
		/* exchange value of cell_new_key and value of cell_start_key, that is putting new cell to the front */
		cell_map[cell_new_key] = cell_map[cell_start_key];
		cell_map[cell_start_key] = cell_new_value;
		cell_start_key++;
		
		if(cell_new_value == gsm->cell_cur || (arfcn = atoi(gsm->cell[cell_new_value].arfcn)) <= 0){
			continue;
		}

		/* set new_cell and lock cell cli command */
		if(*new_cell < 0){
			*new_cell = cell_new_value;
			strncat(command, gsm->cell[cell_new_value].arfcn, sizeof(command));
			j++;
			continue;
		}
		strncat(command, ",", len);
		strncat(command, gsm->cell[cell_new_value].arfcn, len);
		j++;
	}

	if(global_gsm_info.immediate){
		strncat(command, " immediate", len);
	}

	return 0;
}

static int is_first_cell_info(const char *atbuf) 
{ 
	if (strlen(atbuf) > sizeof("+CENG: 0")) { 
		if (!memcmp(atbuf,"+CENG: 0",sizeof("+CENG: 0")-1)) { 
			return 1; 
		} 
	} 

	return 0; 
} 

static int ami_set_cell(struct gsm_node *gsm, struct message *msg) 
{
	struct cell_info cf; 
	char *find; 
	int ret = 0, i = 0;
	
	pthread_mutex_lock(&gsm->cell_lock);
	for(i = 0; i < msg->hdrcount; i++){
		if(global_gsm_info.get_cells_from_at){
			if(NULL != (find = strstr(msg->headers[i], "+CENG:"))){
				memset(&cf, 0, sizeof(cf));
				if (is_first_cell_info(msg->headers[i])) { 
					if( sscanf(msg->headers[i], "+CENG:%d,\"%10[^,],%d,%d,%10[^,],%10[^,],%d,%10[^,],%d,%d,%10[^,],%d\"", 
						&cf.cell, cf.arfcn, &cf.rxl, &cf.rxq, cf.mcc, cf.mnc, 
						&cf.bsic, cf.cellid, &cf.rla, &cf.txp, cf.lac, &cf.TA) == 12 ) { 
						if(cf.cell == 0 && atoi(cf.arfcn) > 0){
							ret++;
							gsm->cell[cf.cell] = cf; 
							update_cell_count(gsm, gsm->cell[cf.cell].cell);
						}
					} 
				} else { 
					if( sscanf(msg->headers[i], "+CENG:%d,\"%10[^,],%d,%d,%10[^,],%10[^,],%10[^,],%10s\"", 
						&cf.cell,cf.arfcn, &cf.rxl, &cf.bsic, cf.cellid, cf.mcc, cf.mnc, cf.lac) == 8 ) { 
						if(cf.cell > 0 && cf.cell < MAX_CELL && atoi(cf.arfcn) > 0){
							ret++;
							gsm->cell[cf.cell] = cf;
							update_cell_count(gsm, gsm->cell[cf.cell].cell);
						}
					} 
				} 	
			}
		}else{
			dlog(DEBUG_LEVEL4,"[span=%d] headers [%s]\n",gsm->span, msg->headers[i]);
			memset(&cf, 0, sizeof(cf));
			if(sscanf(msg->headers[i],"%d %s %d %d %s %s %d %s %d %d %s %d",
				&cf.cell, cf.arfcn, &cf.rxl, &cf.rxq, cf.mcc, cf.mnc,
				&cf.bsic, cf.cellid, &cf.rla, &cf.txp, cf.lac, &cf.TA) == 12){
				dlog(DEBUG_LEVEL4,"[span=%d] ret [%d]\n",gsm->span, ret);
				if(cf.cell >= 0 && cf.cell < MAX_CELL && atoi(cf.arfcn) > 0){
					ret++;
					dlog(DEBUG_LEVEL4,"[span=%d] ret [%d]\n",gsm->span, ret);
					gsm->cell[cf.cell] = cf;
					update_cell_count(gsm, gsm->cell[cf.cell].cell);
				}
			}
		}
	} 
	pthread_mutex_unlock(&gsm->cell_lock);

	gsm->cell_sum = ret;

	for(i = gsm->cell_sum; i < MAX_CELL; i++){
		memset(&gsm->cell[i], 0, sizeof(gsm->cell[i]));
	}

	/* update cell_count */
	for(i = 0; i < MAX_CELL; i++){
		memset(&gsm->cell_count[i], 0, sizeof(gsm->cell_count[i]));
		strncpy(gsm->cell_count[i].arfcn, gsm->cell[i].arfcn, sizeof(gsm->cell_count[i].arfcn));
		gsm->cell_count[i].count= gsm->cell[i].count;
	}

	dlog(DEBUG_LEVEL4,"[span=%d] ret [%d]\n",gsm->span, ret);
	
	return ret; 
}

static int ami_update_cell(struct mansession *s, struct gsm_node *gsm)
{
	struct message msg;
	char command[AMI_COMMAND_MAX];
	int ret = -1, i = 0;
	int timeout = global_gsm_info.at_timeout;

	dlog(DEBUG_LEVEL3,"[span=%d]\n",gsm->span);

	memset(command, 0, sizeof(command));
	memset(&msg, 0, sizeof(msg));
	if(global_gsm_info.get_cells_from_at){
		snprintf(command, sizeof(command), "gsm send sync at %d AT+CENG? %d \"ERROR|+CENG:\"", gsm->span, global_gsm_info.at_timeout*1000);
	}else{
		snprintf(command, sizeof(command), "gsm show cells %d", gsm->span);
	}
	ret = astman_command(s, &msg, command, NULL, timeout);
	if(debug & DEBUG_LEVEL3){
		dlog(DEBUG_LEVEL3,"[span=%d] command=[%s]\n", gsm->span, command);
		dlog(DEBUG_LEVEL3,"[span=%d] msg.hdrcount=[%d], gettingdata=[%d] headers=\n", gsm->span, msg.hdrcount, msg.gettingdata);
		for(i = 0; i < msg.hdrcount; i++){
			if(msg.headers[i][0] != '\0'){
				printf("%2d.%s",i,msg.headers[i]);
			}
		}
	}
	if(ret == ASTMAN_FAILURE){
		dlog(DEBUG_LEVEL3,"[span=%d] error\n", gsm->span);
		return ASTMAN_FAILURE;
	}else if(ret == ASTMAN_SOCKCLOSED){
		dlog(DEBUG_LEVEL3,"[span=%d] server closed\n", gsm->span);
		return ASTMAN_SOCKCLOSED;
	}else if(ret == ASTMAN_TIMEOUT){
		dlog(DEBUG_LEVEL3,"[span=%d] timeout\n", gsm->span);
		return ASTMAN_TIMEOUT;
	}

	if((ret = ami_set_cell(gsm, &msg)) > 0){
		if(debug & DEBUG_LEVEL3){
			dlog(DEBUG_LEVEL3,"[span=%d] cell sum [%d]:\n", gsm->span, gsm->cell_sum);
			for(i = 0; i < msg.hdrcount; i++){
				if(msg.headers[i][0] != '\0' && atoi(gsm->cell[i].arfcn) > 0){
					printf("%d\t%s\t%d\t%d\t%s\t%s\t%d\t%s\t%d\t%d\t%s\t%d\n", 
							 gsm->cell[i].cell, 
							 gsm->cell[i].arfcn, 
							 gsm->cell[i].rxl, 
							 gsm->cell[i].rxq, 
							 gsm->cell[i].mcc, 
							 gsm->cell[i].mnc, 
							 gsm->cell[i].bsic, 
							 gsm->cell[i].cellid, 
							 gsm->cell[i].rla, 
							 gsm->cell[i].txp, 
							 gsm->cell[i].lac, 
							 gsm->cell[i].TA);	
				}
			}
		}
		return ASTMAN_SUCCESS;
	}

	return ASTMAN_FAILURE;
}

static int ami_lock_cell(struct mansession *s, struct gsm_node *gsm, int *new_cell)
{
	struct message msg;
	char command[AMI_COMMAND_MAX], tmp[MAX_LEN_NAME];
	int ret = -1, i = 0, j = 0, res = 0, arfcn = 0;
	int timeout = global_gsm_info.at_timeout;

	dlog(DEBUG_LEVEL3,"[span=%d]\n",gsm->span);
	
	memset(command, 0, sizeof(command));
	memset(&msg, 0, sizeof(msg));
	if(get_lock_cell_cli(gsm, command, sizeof(command), new_cell) != 0){
		return -1;
	}
	dlog(DEBUG_LEVEL1, "span [%d] command [%s]\n", gsm->span, command);
	
	ret = astman_command(s, &msg, command, NULL, timeout);
	if(debug & DEBUG_LEVEL3){
		dlog(DEBUG_LEVEL3,"[span=%d] command=[%s]\n", gsm->span, command);
		dlog(DEBUG_LEVEL3,"[span=%d] msg.hdrcount=[%d], gettingdata=[%d] headers=\n", msg.hdrcount, msg.gettingdata);
		for(i = 0; i < msg.hdrcount; i++){
			if(msg.headers[i][0] != '\0'){
				printf("%2d.%s",i,msg.headers[i]);
			}
		}
	}
	if(ret == ASTMAN_SUCCESS){
		/*res = -1;
		for(i = 0; i < msg.hdrcount; i++){
			if(NULL != strstr(msg.headers[i], "OK")){
				res = 0;
			}
		}
		if(res == 0){
			dlog(DEBUG_LEVEL3,"[span=%d] lock cell [%d] ok\n", gsm->span, gsm->cell_cur);
			return ASTMAN_SUCCESS;
		}*/
		return ASTMAN_SUCCESS;
	}else if(ret == ASTMAN_SOCKCLOSED){
		dlog(DEBUG_LEVEL3,"[span=%d] server closed\n", gsm->span);
		return ASTMAN_SOCKCLOSED;
	}else if(ret == ASTMAN_TIMEOUT){
		dlog(DEBUG_LEVEL3,"[span=%d] timeout\n", gsm->span);
		return ASTMAN_TIMEOUT;
	}
	
	dlog(DEBUG_LEVEL3,"[span=%d] error\n", gsm->span);
	return ASTMAN_FAILURE;
}

static void *ami_gsm_pthread(void *arg)
{
	int span = (int)arg;
	struct gsm_node *gsm;
	struct mansession session;
	struct mansession *s = &session;
	struct timeval now;
	struct timespec timeout;
	struct message msg;
	char buffer[4096];
	int ret = -1, i = 0, count = 0;
	int wait_interval = 0, new_cell = 0;

	if(span >= 0 && span < __GSM_SUM__){
		gsm = &global_gsm_info.gsm[span];
		gsm->span = span + 1;
	}else{
		return NULL;
	}

	for(;;){
		if(ASTMAN_SUCCESS != astman_login(s, NULL, global_ami_info.port, global_ami_info.username, global_ami_info.secret)){
			goto ami_close;
		}
		s->debug = debug;
		dlog(DEBUG_LEVEL1,"[span=%d] ami login\n");

		/* events off */
		memset(&msg, 0, sizeof(msg));
		if(astman_events(s, &msg, "off", NULL, global_gsm_info.at_timeout) == ASTMAN_SOCKCLOSED){
			goto ami_close;
		}
		
		while(!is_cell_ready(gsm)){
			sleep(global_gsm_info.reget_cell_ready_time);
		}

		/* Init cells info */
		if(ami_update_cell(s, gsm) == ASTMAN_SOCKCLOSED){
			goto ami_close;
		}
		increase_cell_count(gsm);
		write_cells_info_file();
		
		while(1){
			wait_interval = get_wait_interval();
			
			dlog(DEBUG_LEVEL1,"[span=%d] wait interval [%d]\n", gsm->span, wait_interval);
			
			/* time wait */
			pthread_mutex_lock(&gsm->pth_lock);
			gettimeofday(&now, NULL);
			timeout.tv_sec = now.tv_sec + wait_interval;
			timeout.tv_nsec = now.tv_usec * 1000;
			pthread_cond_timedwait(&gsm->pth_cond, &gsm->pth_lock, &timeout);
			pthread_mutex_unlock(&gsm->pth_lock);

			dlog(DEBUG_LEVEL3,"[span=%d]\n", gsm->span);

			while(!is_cell_ready(gsm)){
				sleep(global_gsm_info.reget_cell_ready_time);
			}

			/* update gsm cell info */
			ret = ami_update_cell(s, gsm);
			if(ret  == ASTMAN_SOCKCLOSED){
				goto ami_close;
			} else if(ret  == ASTMAN_FAILURE || ret  == ASTMAN_TIMEOUT){
				continue;
			}

			dlog(DEBUG_LEVEL1,"[span=%d]\n", gsm->span);

			/* lock gsm cell */
			for(i = 0; i < MAX_CELL; i++){
				/*if((new_cell = get_new_cell(gsm)) < 0){
					//sleep(1);
					continue;
				}
				
				dlog(DEBUG_LEVEL1,"[span=%d] new_cell [%d]\n", gsm->span, new_cell);*/
				
				count = global_gsm_info.at_counts;
				while(count > 0 && count--){
					ret = ami_lock_cell(s, gsm, &new_cell);
					update_log(gsm, new_cell, ret==ASTMAN_SUCCESS?0:1);
					if(ret == ASTMAN_SUCCESS){
						gsm->cell_cur = new_cell;
						increase_cell_count(gsm);
						write_cells_info_file();
						break;//lock ok
					}else if(ret == ASTMAN_SOCKCLOSED){
						goto ami_close;// server socket closed
					}
				}

				dlog(DEBUG_LEVEL1,"[span=%d] lock_cell ret [%d]\n", gsm->span, ret);
					
				if(ret == ASTMAN_SUCCESS){ 
					break;//lock ok
				}
			}

			dlog(DEBUG_LEVEL1,"[span=%d] lock ok\n", gsm->span);
		}

ami_close:
		dlog(DEBUG_LEVEL1,"ami close\n");
		astman_close(s);
		sleep(global_gsm_info.reconnect_ami_time);
	}

	return NULL;
}

static int astrx_update_cell(struct gsm_node *gsm)
{
	struct cell_info cf; 
	FILE *stream = NULL;
	char command[AMI_COMMAND_MAX];
	char buffer[MAX_LEN_BUFFER];
	char *find = NULL;
	int ret = 0, i = 0;
	int timeout = global_gsm_info.at_timeout;

	memset(command, 0, sizeof(command));
	if(global_gsm_info.get_cells_from_at){
		snprintf(command, sizeof(command), "asterisk -rx \"gsm send sync at %d AT+CENG? %d\"", gsm->span, global_gsm_info.at_timeout*1000);
	}else{
		snprintf(command, sizeof(command), "asterisk -rx \"gsm show cells %d\"", gsm->span);
	}

	dlog(DEBUG_LEVEL1,"[span=%d] command[%s]\n", gsm->span, command);
	
	if((stream = popen(command, "r")) == NULL){
		return -1;
	}
	
	pthread_mutex_lock(&gsm->cell_lock);
	memset(buffer, 0, sizeof(buffer));
	if(global_gsm_info.get_cells_from_at){
		while(fgets(buffer, sizeof(buffer), stream) != NULL){ 
			dlog(DEBUG_LEVEL4,"[span=%d] buffer: %s", gsm->span, buffer);
			if(NULL != (find = strstr(buffer, "+CENG:"))){
				memset(&cf, 0, sizeof(cf));
				if (is_first_cell_info(buffer)) { 
					if( sscanf(buffer, "+CENG:%d,\"%10[^,],%d,%d,%10[^,],%10[^,],%d,%10[^,],%d,%d,%10[^,],%d\"", 
						&cf.cell, cf.arfcn, &cf.rxl, &cf.rxq, cf.mcc, cf.mnc, 
						&cf.bsic, cf.cellid, &cf.rla, &cf.txp, cf.lac, &cf.TA) == 12 ) { 
						if(cf.cell == 0 && atoi(cf.arfcn) > 0){
							ret++;
							gsm->cell[cf.cell] = cf; 
							update_cell_count(gsm, gsm->cell[cf.cell].cell);
						}
					} 
				} else { 
					if( sscanf(buffer, "+CENG:%d,\"%10[^,],%d,%d,%10[^,],%10[^,],%10[^,],%10s\"", 
						&cf.cell,cf.arfcn, &cf.rxl, &cf.bsic, cf.cellid, cf.mcc, cf.mnc, cf.lac) == 8 ) { 
						if(cf.cell > 0 && cf.cell < MAX_CELL && atoi(cf.arfcn) > 0){
							ret++;
							gsm->cell[cf.cell] = cf;
							update_cell_count(gsm, gsm->cell[cf.cell].cell);
						}
					} 
				} 	
			}
			memset(buffer, 0, sizeof(buffer));
		}
	}else{
		while(fgets(buffer, sizeof(buffer), stream) != NULL){ 
			dlog(DEBUG_LEVEL4,"[span=%d] buffer: %s", gsm->span, buffer);
			memset(&cf, 0, sizeof(cf));
			if(sscanf(buffer,"%d %s %d %d %s %s %d %s %d %d %s %d",
				&cf.cell, cf.arfcn, &cf.rxl, &cf.rxq, cf.mcc, cf.mnc,
				&cf.bsic, cf.cellid, &cf.rla, &cf.txp, cf.lac, &cf.TA) == 12){
				if(cf.cell >= 0 && cf.cell < MAX_CELL && atoi(cf.arfcn) > 0){
					ret++;
					gsm->cell[cf.cell] = cf;
					update_cell_count(gsm, gsm->cell[cf.cell].cell);
				}
			}	
			memset(buffer, 0, sizeof(buffer));
		}
	}
	pthread_mutex_unlock(&gsm->cell_lock);	
	
	pclose(stream);

	gsm->cell_sum = ret;

	for(i = gsm->cell_sum; i < MAX_CELL; i++){
		memset(&gsm->cell[i], 0, sizeof(gsm->cell[i]));
	}

	/* update cell_count */
	for(i = 0; i < MAX_CELL; i++){
		memset(&gsm->cell_count[i], 0, sizeof(gsm->cell_count[i]));
		strncpy(gsm->cell_count[i].arfcn, gsm->cell[i].arfcn, sizeof(gsm->cell_count[i].arfcn));
		gsm->cell_count[i].count= gsm->cell[i].count;
	}

	if(debug & DEBUG_LEVEL3){
		dlog(DEBUG_LEVEL3,"[span=%d] cell sum [%d]:\n", gsm->span, gsm->cell_sum);
		for(i = 0; i < gsm->cell_sum; i++){
			if(atoi(gsm->cell[i].arfcn) > 0){
				printf("%d\t%s\t%d\t%d\t%s\t%s\t%d\t%s\t%d\t%d\t%s\t%d\n", 
						 gsm->cell[i].cell, 
						 gsm->cell[i].arfcn, 
						 gsm->cell[i].rxl, 
						 gsm->cell[i].rxq, 
						 gsm->cell[i].mcc, 
						 gsm->cell[i].mnc, 
						 gsm->cell[i].bsic, 
						 gsm->cell[i].cellid, 
						 gsm->cell[i].rla, 
						 gsm->cell[i].txp, 
						 gsm->cell[i].lac, 
						 gsm->cell[i].TA);	
			}
		}
	}

	if(ret > 0){
		return 0;
	}else{
		return -1;
	}
}

static int astrx_lock_cell(struct gsm_node *gsm, int *new_cell)
{
	struct cell_info cf; 
	FILE *stream = NULL;
	char command[MAX_LEN_COMMAND];
	char tmp[MAX_LEN_COMMAND];
	char *find = NULL;
	int cell_used[MAX_CELL];
	int ret = -1, i = 0, j = 0, arfcn = 0;

	memset(command, 0, sizeof(command));
	memset(tmp, 0, sizeof(tmp));
	if(get_lock_cell_cli(gsm, tmp, sizeof(tmp), new_cell) != 0){
		return -1;
	}
	snprintf(command, sizeof(command), "asterisk -rx \"%s\"", tmp);
	dlog(DEBUG_LEVEL1,"[span=%d] command [%s]\n", gsm->span, command);

	/*ret = -1;
	memset(buffer, 0, sizeof(buffer));
	while(fgets(buffer, sizeof(buffer), stream) != NULL){
		dlog(DEBUG_LEVEL4,"[span=%d] command ret:%s", gsm->span, buffer);
		if(NULL != (find = strstr(buffer, "OK"))){
			ret = 0;
			break;
		}
		memset(buffer, 0, sizeof(buffer));
	}*/

	return pipe_exec(command, NULL, 0);
}

static void *astrx_gsm_pthread(void *arg)
{
	int span = (int)arg;
	struct gsm_node *gsm;
	struct timeval now;
	struct timespec timeout;
	time_t timep;
	int count = 0, ret = 0, i = 0;
	int wait_interval = 0, new_cell = 0;
	
	if(span >= 0 && span < __GSM_SUM__){
		gsm = &global_gsm_info.gsm[span];
		gsm->span = span + 1;
	}else{
		return NULL;
	}

	while(!is_cell_ready(gsm)){
		sleep(global_gsm_info.reget_cell_ready_time);
	}

	//Init cells.
	astrx_update_cell(gsm);
	increase_cell_count(gsm);
	write_cells_info_file();

	while(1){
		wait_interval = get_wait_interval();
		
		dlog(3,"[span=%d] while 1 wait interval [%d]\n", gsm->span, wait_interval);

		/* time wait */
		pthread_mutex_lock(&gsm->pth_lock);
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + wait_interval;
		timeout.tv_nsec = now.tv_usec * 1000;
		pthread_cond_timedwait(&gsm->pth_cond, &gsm->pth_lock, &timeout);
		pthread_mutex_unlock(&gsm->pth_lock);

		dlog(3,"[span=%d]\n", gsm->span);
		
		while(!is_cell_ready(gsm)){
			sleep(global_gsm_info.reget_cell_ready_time);
		}
		
		/* update gsm cell info */
		if((ret = astrx_update_cell(gsm)) != 0){
			continue;	
		}
		
		dlog(3,"[span=%d]\n", gsm->span);
		
		/* lock gsm cell */
		for(i = 0; i < MAX_CELL; i++){
			/*if((new_cell = get_new_cell(gsm)) < 0){
				continue;
			}
			
			dlog(3,"[span=%d] new_cell [%d]\n", gsm->span, new_cell);*/
			
			count = global_gsm_info.at_counts;
			while(count > 0 && count--){
				ret = astrx_lock_cell(gsm, &new_cell);
				update_log(gsm, new_cell, ret);
				dlog(3,"[span=%d] lock_cell ret [%d]\n", gsm->span, ret);
				if(ret == 0){
					gsm->cell_cur = new_cell;
					increase_cell_count(gsm);
					write_cells_info_file();
					break;//lock ok
				}
				sleep(1);
			}
						
			if(ret == 0){ 
				break;//lock ok
			}
		}
		
	}
	
	return NULL;
}

static int init_config(struct gsm_infomation *info)
{
/*  file format:
----------------
[general]
enable=yes|no
select_type=random|ascend|descend 
when_type=random_time|random_calls 
signal_threshold=10

[when] 
random_time=30-50 
random_calls=3-5 

[debug] 
at_counts=3 
at_timeout=5 
get_cells_from_at=no|yes
call_ast_type=pipe|socket
cell_mcc_check=yes|no
cell_mnc_check=yes|no
reconnect_ami_time=30 
reget_cell_ready_time=2 
cell_ready_keyword="Status: Power on, Provisioned, Up|xxxxx" 
immediate=yes|no

----------------
*/

	char *file_path = "/etc/asterisk/gw/auto_lock_cell.conf";
	char line[MAX_LEN_LINE];
	char key[MAX_LEN_LINE];
	char value[MAX_LEN_LINE];
	char section[MAX_LEN_LINE];
	int max = 0, min = 0, val = 0;

	FILE* fp;
	int lock;

	/* set default value */
	/* [general] */
	info->enable = 0;
	info->cell_select_type = CELL_SELECT_ASCEND;
	info->when_lock_type = WHEN_LOCK_RANDOM_TIME;
	info->max_lock = MAX_LOCK_CELL;
	/* [when] */
	info->when_max = 60;
	info->when_min = 30;
	/* [debug] */
	info->at_counts = 3;
	info->at_timeout = 5; //seconds
	info->get_cells_from_at = 0; //0=from "gsm show cells"
	info->call_ast_type = CALL_AST_PIPE;
	info->cell_mcc_check = 1;
	info->cell_mnc_check = 1;
	info->find_cell_fail_counts = 20;
	info->reconnect_ami_time = DEFAULT_RECONNECT_AMI_TIME;
	info->reget_cell_ready_time = DEFAULT_REGET_CELL_READY_TIME;
	strncpy(info->cell_ready_keyword,DEFAULT_CELL_READY_KEYWORD,sizeof(info->cell_ready_keyword));
	info->immediate=0;
	info->log_max_size = DEFAULT_LOG_MAX_SIZE;
	
	lock = lock_file(file_path);
	if( NULL == (fp=fopen(file_path,"r")) ) {
		fprintf(stderr,"Can't open %s\n",file_path);
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
			if(0 == strcasecmp(section,"general")){
				if(0 == strcasecmp(key,"enable")) {
					if(is_true(value)) {
						info->enable = 1;
					} else {
						info->enable = 0;
					}
				}else if(0 == strcasecmp(key,"select_type")) {
					if(0 == strcasecmp(value,"random")){
						info->cell_select_type = CELL_SELECT_RANDOM;
					}else if(0 == strcasecmp(value,"ascend")){
						info->cell_select_type = CELL_SELECT_ASCEND;
					}else if(0 == strcasecmp(value,"descend")){
						info->cell_select_type = CELL_SELECT_DESCEND;
					}
				}else if(0 == strcasecmp(key,"when_type")) {
					if(0 == strcasecmp(value,"random_time")){
						info->when_lock_type = WHEN_LOCK_RANDOM_TIME;
					}else if(0 == strcasecmp(value,"random_calls")){
						info->when_lock_type = WHEN_LOCK_RANDOM_CALLS;
					}
				}else if(0 == strcasecmp(key,"signal_threshold")) {
					if((val = atoi(value)) && val > 0){
						info->signal_threshold = val;
					}
				}else if(0 == strcasecmp(key,"max_lock")) {
					if((val = atoi(value)) && val > 0){
						info->max_lock = val;
					}
				}
			}else if(0 == strcasecmp(section,"when")){
				if(info->when_lock_type == WHEN_LOCK_RANDOM_TIME && 0 == strcasecmp(key,"random_time")) {
					max = min = 0;
					if(sscanf(value, "%d-%d", &min, &max) == 2){
						if(max >= min && min >= 0){
							info->when_max = max;
							info->when_min = min;
						}
					}
				} else if(info->when_lock_type == WHEN_LOCK_RANDOM_CALLS && 0 == strcasecmp(key,"random_calls")) {
					max = min = 0;
					if(sscanf(value, "%d-%d", &min, &max) == 2){
						if(max > min && min >= 0){
							info->when_max = max;
							info->when_min = min;
						}
					}
				}
			}else if(0 == strcasecmp(section,"debug")){
				if(0 == strcasecmp(key,"at_counts")) {
					if((val = atoi(value)) && val > 0){
						info->at_counts = val;
					}
				} else if(0 == strcasecmp(key,"at_timeout")) {
					if((val = atoi(value)) && val > 0){
						info->at_timeout = val;
					}
				} else if(0 == strcasecmp(key,"get_cells_from_at")) {
					if(is_true(value)){
						info->get_cells_from_at = 1;
					}else{
						info->get_cells_from_at = 0;
					}
				} else if(0 == strcasecmp(key,"call_ast_type")) {
					if(0 == strcasecmp(value,"socket")){
						info->call_ast_type = CALL_AST_SOCKET;
					}else if(0 == strcasecmp(value,"pipe")){
						info->call_ast_type = CALL_AST_PIPE;
					}
				} else if(0 == strcasecmp(key,"cell_mcc_check")) {
					if(is_true(value)){
						info->cell_mcc_check = 1;
					}else{
						info->cell_mcc_check = 0;
					}
				}else if(0 == strcasecmp(key,"cell_mnc_check")) {
					if(is_true(value)){
						info->cell_mnc_check = 1;
					}else{
						info->cell_mnc_check = 0;
					}
				} else if(0 == strcasecmp(key,"find_cell_fail_counts")) {
					if((val = atoi(value)) && val == 1){
						info->find_cell_fail_counts = val;
					}
				} else if(0 == strcasecmp(key,"reconnect_ami_time")) {
					info->reconnect_ami_time = atoi(value);
					if (info->reconnect_ami_time <= 0) {
						info->reconnect_ami_time = DEFAULT_RECONNECT_AMI_TIME;
					}
				} else if(0 == strcasecmp(key,"reget_cell_ready_time")) {
					info->reget_cell_ready_time = atoi(value);
					if (info->reget_cell_ready_time <= 0) {
						info->reget_cell_ready_time = DEFAULT_REGET_CELL_READY_TIME;
					}
				} else if(0 == strcasecmp(key,"cell_ready_keyword")) {
					if (value == NULL || strlen(value) <= 0) {
						strncpy(info->cell_ready_keyword,DEFAULT_CELL_READY_KEYWORD,sizeof(info->cell_ready_keyword));
					} else {
						strncpy(info->cell_ready_keyword,value,sizeof(info->cell_ready_keyword));
					}
				} else if(0 == strcasecmp(key,"immediate")) {
					if(is_true(value)){
						info->immediate = 1;
					}else{
						info->immediate = 0;
					}
				} else if(!strcasecmp(key,"log_max_size")){
					if((val = atoi(value)) > 0 && val < DEFAULT_LOG_MAX_SIZE){
						info->log_max_size = val;
					}
				}
			}
		}
		memset(line,0,sizeof(line));
	}

	fclose(fp);
	unlock_file(lock);

	if(debug){
		printf("global_gsm_info:\n"\
			"cs_type=[%d]\n"\
			"wl_type=[%d]\n"\
			"signal_threshold=[%d]\n"\
			"min=[%d]\nmax=[%d]\n"\
			"at_counts=[%d]\n"\
			"at_timeout=[%d]\n"\
			"get_cells_from_at=[%d]\n"\
			"call_ast_type=[%d]\n"\
			"mcc_check=[%d]\n"\
			"mnc_check=[%d]\n"\
			"immediate=[%d]\n",
			/* [general */
			global_gsm_info.cell_select_type,
			global_gsm_info.when_lock_type,
			global_gsm_info.signal_threshold,
			/* [when] */
			global_gsm_info.when_min,
			global_gsm_info.when_max,
			/* [debug] */
			global_gsm_info.at_counts,
			global_gsm_info.at_timeout,
			global_gsm_info.get_cells_from_at,
			global_gsm_info.call_ast_type,
			global_gsm_info.cell_mcc_check,
			global_gsm_info.cell_mnc_check,
			global_gsm_info.immediate);	
	}

	return 0;
}

static int write_pid(void)
{
	char *file_path = "/tmp/run/auto_lock_cell.pid";
	char buffer[128];
	int fd = -1, len = 0;
	
	if((fd = open(file_path, O_CREAT|O_WRONLY|O_TRUNC, 0664)) < 0) {
		dlog(DEBUG_LEVEL1,"Unable to open %s: %s\n", file_path, strerror(errno));
		return -1;
	}
	if(flock(fd, LOCK_EX) < 0) {
		dlog(DEBUG_LEVEL1,"File %s was not locked.\n",file_path);
		close(fd);
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "%d", getpid());
	len = write(fd,buffer,strlen(buffer));

	if((flock(fd, LOCK_UN)) < 0) {
		dlog(DEBUG_LEVEL1,"File %s unlocked error.\n",file_path);
		close(fd);
		return -1;
	}
	close(fd);

	if(len <= 0){
		return -1;
	}

	return 0;
}

static int is_running(void)
{
	char *file_path = "/var/run/auto_lock_cell.ctl";
	int fd = -1, count = 0;
	
	if((fd = open(file_path, O_CREAT|O_WRONLY|O_TRUNC, 0664)) < 0) {
		dlog(DEBUG_LEVEL2,"Unable to open %s: %s\n", file_path, strerror(errno));
		return -1;
	}

	count = 10;
	while(count--){
		if(flock(fd, LOCK_EX|LOCK_NB) < 0) {
			sleep(1);
		}else{
			break;
		}
	}

	if(count <= 0){
		dlog(DEBUG_LEVEL2,"Lock File %s failed.\n",file_path);
		close(fd);
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0, i = 0;
	pthread_t tid = 0;

	if(is_running()){
		dlog(DEBUG_LEVEL0,"Please check if the program [%s] is already running.\n", argv[0]);
		return -1;
	}

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

	memset(&global_gsm_info, 0, sizeof(global_gsm_info));
	if((ret = init_config(&global_gsm_info)) < 0){
		dlog(DEBUG_LEVEL1,"init_config error\n");
		return -1;
	}

	if(global_gsm_info.enable == 0){
		dlog(DEBUG_LEVEL1,"auto lock cell disabled\n");
		return -1;
	}

	if(init_cluster_config(&global_gsm_info.cluster_cfg) != 0){
		dlog(DEBUG_LEVEL0,"init cluster config error\n");
	}

	global_gsm_info.board = 1;
	if(global_gsm_info.cluster_cfg.mode == CLUSTER_SLAVE){
		char buffer[MAX_LEN_BUFFER];
		memset(buffer, 0, sizeof(buffer));
		if(file_get_contents("/tmp/.slotnum", buffer,sizeof(buffer))== 0){
			int val = atoi(buffer);
			if(val >= 1 && val <= __BRD_SUM__){
				global_gsm_info.board = val;
			}
		}
	}

	if(debug == 0){
		if(daemon(1, 0) < 0){
			dlog(DEBUG_LEVEL1, "deamon error\n");
			return -1;
		}
	}

	if((write_pid()) < 0){
		return -1;
	}

	for(i = 0; i < __GSM_SUM__; i++){
		/* init gsm node */
		pthread_mutex_init(&global_gsm_info.gsm[i].cell_lock, NULL);
		pthread_mutex_init(&global_gsm_info.gsm[i].pth_lock, NULL);
		pthread_cond_init(&global_gsm_info.gsm[i].pth_cond, NULL);

		/* start gsm pthread */
		if((ret = pthread_create(&tid,NULL,(global_gsm_info.call_ast_type==CALL_AST_SOCKET)?ami_gsm_pthread:astrx_gsm_pthread,(void*)i)) != 0){
			dlog(DEBUG_LEVEL1,"pthread_create error\n");
			return -1;
		}else{
			global_gsm_info.gsm[i].tid = tid;
			dlog(DEBUG_LEVEL1,"create pthread [%d], span [%d]\n", pthread_self(), i);
		}
	}

	for(i = 0; i < __GSM_SUM__; i++){
		if(global_gsm_info.gsm[0].tid > 0){
			pthread_join(global_gsm_info.gsm[i].tid, NULL);
		}
		pthread_cond_destroy(&global_gsm_info.gsm[i].pth_cond);
		pthread_mutex_destroy(&global_gsm_info.gsm[i].pth_lock);
		pthread_mutex_destroy(&global_gsm_info.gsm[i].cell_lock);
	}

	return 0;
}
