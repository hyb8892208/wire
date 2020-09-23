/*
  *  Filename: common.cpp  Version: 0.1 
  *  
  *  Copyright (C) OPVX QMD
  *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
  *  Reference Stephen F. Booth
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU Lesser General Public
  *  License as published by the Free Software Foundation; either
  *  version 3 of the License, or (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  *  Lesser General Public License for more details.
  *
  *  You should have received a copy of the GNU Lesser General Public
  *  License along with this library; if not, write to the Free Software
  *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
  */

#include "common.h"

char *trim( char *str )
{
	char *copied, *ret = str, *tail = NULL;
	if ( str == NULL )
		return NULL;

	for( copied = str; *str; str++ ) {
		if ( *str > 32 ) {
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

int lock_file(const char* path)
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

int unlock_file(int fd)
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

int is_running(char *running_path)
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

int is_true(const char *s)
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

int random_int(int min, int max)
{
/*
 *    return [min, max)
 */
	if(min < 0){
		return 0;
	}else if(min >= max){
		return min;
	}else{
		return min + (rand() % (max - min));
	}
}

int file_size(char *file_path)
{
	struct stat file_stat;
	
	stat(file_path, &file_stat);
	
	return file_stat.st_size;
}

int file_get_contents(char *file_path, char *buffer, int len)
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

int file_put_contents(char *file_path, char *buffer, int len)
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

int file_append_contents(const char *file_path, char *buffer, int len)
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

int file_slim_contents(const char *file_path, int valid_start, int len)
{
	char *buffer = NULL, *p = NULL;
	int fd = -1, size = 0, len_rest = 0, ret = -1;

	if(valid_start < 0 || len <= 0 || file_path == NULL){
		return -1;
	}

	if((buffer = (char*)malloc(len)) == NULL){
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

/**************************************
 *
 *    utf8 operation functions
 *
 **************************************/

/*
0xxxxxxx
110xxxxx 10xxxxxx
1110xxxx 10xxxxxx 10xxxxxx
11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
int utf8_strlen(char *s) 
{
	int ascii_len = 0, utf8_len = 0;

	if(!s)
		return -1;
	
	while(s[ascii_len]){
		if((s[ascii_len] & 0xc0) != 0x80)
			utf8_len++;
		ascii_len++;
	}
	
	return utf8_len;
}

int utf8_charsize(char *s, int utf8_pos)
{
	int ascii_len = 0, utf8_len = 0, utf8_size = 0;

	if(!s)
		return -1;
	
	while(s[ascii_len]){
		if((s[ascii_len] & 0xC0) != 0x80)
			utf8_len++;
		if(utf8_len == utf8_pos)
			break;
		ascii_len++;
	}

	if(utf8_len != utf8_pos)
		return -1;

	if(s[ascii_len] & 0xFC == 0xFC)
		utf8_size = 6;
	else if(s[ascii_len] & 0xF8 == 0xF8)
		utf8_size = 5;
	else if(s[ascii_len] & 0xF0 == 0xF0)
		utf8_size = 4;
	else if(s[ascii_len] & 0xE0 == 0xE0)
		utf8_size = 3;	
	else if(s[ascii_len] & 0xC0 == 0xC0)
		utf8_size = 2;
	else if(s[ascii_len] & 0x80 == 0)
		utf8_size = 1;	
	else
		utf8_size = -1;
		
	return utf8_size;
}

/**************************************
 *
 *    semaphore and share memery functions
 *
 **************************************/
int sem_init(key_t key)  
{
	int sem_id = -1;
	
	if ((sem_id = semget(key, 1, 0666|IPC_CREAT)) < 0){
		perror("failed to semget"); 
		return -1;
	}  
	
	return sem_id;  
}  

int sem_setval(int sem_id, int init_value)
{
	union sem_union sem_union;  

	sem_union.val = init_value;  

	if (semctl(sem_id, 0, SETVAL, sem_union) < 0){  
		perror("failed to SETVAL");  
		return -1;  
	}  

	return 0;
}

int sem_delete(int sem_id)  
{  
	union sem_union sem_union;  

	if (semctl(sem_id, 0, IPC_RMID, sem_union) < 0){  
		perror("failed to delete_sem");  
		return -1;  
	}  

	return 0;  
}  

int sem_p(int sem_id, int nowait)	
{  
	struct sembuf sem_b;  

	sem_b.sem_num = 0;	
	sem_b.sem_op = -1;
	 
	if(nowait){
		sem_b.sem_flg = SEM_UNDO|IPC_NOWAIT;  
	}else{
		sem_b.sem_flg = SEM_UNDO; 
	}

	if(semop(sem_id, &sem_b, 1) < 0){  
		perror("sem_p error");  
		return -1;  
	}  

	return 0;
}  

int sem_v(int sem_id, int nowait)	
{  
	struct sembuf sem_b;  

	sem_b.sem_num = 0;	
	sem_b.sem_op = 1;  
	
	if(nowait){
		sem_b.sem_flg = SEM_UNDO|IPC_NOWAIT;  
	}else{
		sem_b.sem_flg = SEM_UNDO; 
	}

	if (semop(sem_id, &sem_b, 1) < 0){  
		perror("sem_v error");  
		return -1;  
	}

	return 0;  
} 

void *shm_init(key_t key, int *shm_id, int shm_size)
{
	int id = -1;
	void *p = NULL;

	if((id = shmget(key, shm_size, 0666|IPC_CREAT)) < 0){
		perror("failed to shmget");  
		return NULL; 	
	}

	if ((p = shmat(id, NULL, 0)) < 0){  
		perror("failed to shmat");	
		return NULL;  
	}

	*shm_id = id;
	
	return p;
}

int shm_delete(int shm_id, void *shmp)
{
	if(shm_id < 0 || shmp == NULL){
		return -1;
	}

	if(shmp != NULL){
	        if (shmdt(shmp) < 0){  
			perror("failed to shmdt memory");  
			return -1;  
	        }
	}
	
	if (shmctl(shm_id, IPC_RMID, NULL) == -1){  
		perror("failed to delete share memory");  
		return -1;  
	}  
	
	return 0;
}

int service_shm_lock(int nowait)
{
	if(global_service.semid < 0)
		return -1;
	
	if(sem_p(global_service.semid, nowait) < 0)
		return -1;
		
	return 0;
}

int service_shm_unlock(int nowait)
{
	if(global_service.semid < 0)
		return -1;
	
	if(sem_v(global_service.semid, nowait) < 0)
		return -1;

	return 0;
}

/**************************************
 *
 *     abc
 *
 **************************************/

int write_pid(char *pid_path)
{
	char buffer[128];
	
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "%d", getpid());
	if(file_put_contents(pid_path, buffer, strlen(buffer)) < 0){
		return -1;
	}

	return 0;
}

int exec_pipe(const char *cmd, char *outbuf, int outlen)
{
	FILE *stream;
	time_t stime;
	char *p = NULL;
	int rest = 0, len = 0;

	if(cmd == NULL || cmd[0] == '\0'){
		return -1;
	}

	stime = time(NULL);

	if( (stream = popen(cmd, "r")) == NULL ) {
		return -1;
	}

	if(outbuf != NULL && outlen > 0){
		p = outbuf;
		rest = outlen;
		while( rest > 0){
			len = fread(p,1,rest,stream);
			if(len < 0){
				pclose(stream);
				return -1;
			}else if(len == 0){
				break;
			}else{
				p += len;
				rest -= len;
			}
			
			if ((time(NULL) - stime) > EXEC_TIMEOUT) {
				break;
			}
		}
	}
	pclose(stream);

	return 0;
}

int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int sec)
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

int request_slave(const char* ip, char *data, int data_len, char *ret_val, int ret_len, int timeo)
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
	my_addr.sin_port = htons(GW_CLUSTER_SOCKET_PORT);

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

int get_gsm_header(char *src, char *key, char *value, int vallen)
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
		snprintf(tmp, sizeof(tmp), "%s:%%%d[^\n]", key, vallen);
		if(sscanf(start, tmp, value) == 1){
			trim(value);
			return 0;
		}
	}
	
	return -1;
}

int get_gsm_info(char *ip, int span, char *outbuf, int outlen)
{
	char file_path[256];
	char *start = NULL, *end = NULL;
	int len = 0;
	
	if(ip == NULL){
		memset(file_path, 0, sizeof(file_path));
		snprintf(file_path, sizeof(file_path), "/tmp/gsm/%d", span);
		return file_get_contents(file_path, outbuf, outlen);
	}else if(ip[0] != '\0'){
		char tmp[32];
		char buffer[MAX_LEN_BUFFER];
		if(request_slave(ip, "get_spans", strlen("get_spans"), buffer, sizeof(buffer), 5) == 0){
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "---Start Span %d---", span);
			if(NULL != (start = strstr(buffer, tmp))){
				start += strlen(tmp);
				memset(tmp, 0, sizeof(tmp));
				snprintf(tmp, sizeof(tmp), "---End Span %d---", span);
				if(NULL != (end = strstr(start, tmp))){
					for(len = 0; (start + len) < end && len < outlen; len++){
						*(outbuf+len) = *(start+len);
					}
					*(outbuf+len) = '\0';
					return 0;
				}
			}
		}
	}

	return -1;
}

/*
int get_web_auth(char *usr, int ulen, char *pwd, int plen)
{
	char *file_path = "/etc/asterisk/gw/lighttpdpassword";
	char line[MAX_LEN_LINE];
	char tmp[MAX_LEN_LINE];
	char username[MAX_LEN_NAME], password[MAX_LEN_NAME];
	int i = 0;

	FILE* fp;
	int lock;

	lock = lock_file(file_path);
	if( NULL == (fp = fopen(file_path, "r")) ) {
		fprintf(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp), "%%%d[^:]:%%%d[^\n]", sizeof(username), sizeof(password));	
	memset(line, 0, sizeof(line));
	while(!feof(fp) && fgets(line, sizeof(line), fp)) {
		if(sscanf(line, tmp, username, password) == 2){
			trim(username);
			trim(password);
			break;
		}
		memset(line,0,sizeof(line));
	}

	fclose(fp);
	unlock_file(lock);

	if(username[0] != '\0' && password[0] != '\0'){
		strncpy(usr, username, ulen);
		strncpy(pwd, password, plen);
		return 0;
	}

	return -1;

}
*/

int init_clt_config(struct clt_config *cfg)
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
			if(!strcasecmp(section,"general")){
				if(!strcasecmp(key,"mode")) {
					if(!strcasecmp(value,"master")){
						cfg->mode = CLUSTER_MASTER;
					}else{
						cfg->mode = CLUSTER_OTHER;
					}
				}
			}else if(!strcasecmp(section,"master")){
				if(!strcasecmp(key,"password")) {
					strncpy(cfg->password,value,sizeof(cfg->password));
				}else if(!strcasecmp(key,"ip")) {
					strncpy(cfg->master_ip,value,sizeof(cfg->master_ip));
				}
			}else if(!strcasecmp(section,"slavelist")){
				int b;
				for (b=2; b<=__BRD_SUM__; b++) {
					char board_str[32];
					memset(board_str,0,sizeof(board_str));
					snprintf(board_str, sizeof(board_str), "%s%d_ip", __BRD_HEAD__, b);
					if(!strcasecmp(key,board_str)) {
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

	return 0;
}

int init_ami_config(struct amp_config *cfg)
{
/* astmanproxy.conf
---------------


----------------
*/
	char *file_path = "/etc/asterisk/gw/astmanproxy.conf";
	char line[MAX_LEN_LINE];
	char key[MAX_LEN_LINE];
	char value[MAX_LEN_LINE];
	char kv_format[64];
	int val = 0;

	FILE* fp;
	int lock;

	cfg->port = 1234;
	strncpy(cfg->username, ASTMANPROXY_DEFAULT_USERNAME, sizeof(cfg->username));
	strncpy(cfg->password, ASTMANPROXY_DEFAULT_PASSWORD, sizeof(cfg->password));
	
	lock = lock_file(file_path);
	if( NULL == (fp = fopen(file_path,"r")) ) {
		fprintf(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}
	
	memset(kv_format, 0, sizeof(kv_format));
	snprintf(kv_format, sizeof(kv_format), "%%%d[^=]=%%%ds", sizeof(key), sizeof(value));

	memset(line, 0, sizeof(line));
	while(!feof(fp) && fgets(line, sizeof(line), fp)) {
		memset(key,0,sizeof(key));
		memset(value,0,sizeof(value));
		if(sscanf(line, kv_format, key, value) == 2){
			trim(key);
			trim(value);
			if(!strcasecmp(key,"listenport")) {
				if((val = atoi(value)) > 0){
					cfg->port = val;
				}
			}
		}
		memset(line,0,sizeof(line));
	}

	fclose(fp);
	unlock_file(lock);

	return 0;
}

int init_sms_config(struct http2sms_config *cfg)
{
/* sms.conf
-------------
[general]			// for astmanproxy
port=1234

[sms_http_api]
debug=0
timeout_gsm_send=10000	//msecond
amp_timeout=3		//second
timeout_total=0		//second
-------------
*/

	char *file_path = "/etc/asterisk/gw/sms.conf";
	char line[MAX_LEN_LINE];
	char key[MAX_LEN_LINE];
	char value[MAX_LEN_LINE];
	char section[MAX_LEN_LINE];
	char kv_format[64], s_format[64];
	int val = 0, i = 0, j = 0;

	FILE* fp;
	int lock;

	cfg->debug = 0;
	cfg->timeout_gsm_send = 20000; // ms
	cfg->timeout_total = 0; //s
	cfg->timeout_wait = 20; //s
	cfg->port_order = PORT_ORDER_ROUND;
	cfg->report_format = SMS_REPORT_JSON;
	for(i = 0; i < __BRD_SUM__; i++){
		for(j = 0; j < __GSM_SUM__; j++){
			cfg->port_valid[i][j] = 0;
		}
	}

	lock = lock_file(file_path);
	if( NULL == (fp = fopen(file_path,"r")) ) {
		fprintf(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	/* set sscanf format */
	memset(s_format, 0, sizeof(s_format));
	memset(kv_format, 0, sizeof(kv_format));
	snprintf(s_format, sizeof(s_format), "[%%%d[^]]]", sizeof(section));
	snprintf(kv_format, sizeof(kv_format), "%%%d[^=]=%%%ds", sizeof(key), sizeof(value));

	/* read config file */
	memset(line, 0, sizeof(line));
	memset(section, 0, sizeof(line));
	while(!feof(fp) && fgets(line, sizeof(line), fp)) {
		memset(key,0,sizeof(key));
		memset(value,0,sizeof(value));
		
		if(sscanf(line, s_format, section) == 1){
			trim(section);
		}else if(section[0] != '\0' && sscanf(line, kv_format, key, value) == 2){
			trim(key);
			trim(value);
			if(!strcasecmp(section,"http_to_sms")){
				if(!strcasecmp(key,"enable")) {
					if(is_true(value)){
						cfg->enable = 1;
					}
				}else if(!strcasecmp(key,"use_default_user")) {
					if(is_true(value)){
						cfg->use_default_user = 1;
					}
				}else if(!strcasecmp(key,"username")) {
					strncpy(cfg->username,value,sizeof(cfg->username));
				}else if(!strcasecmp(key,"password")) {
					strncpy(cfg->password,value,sizeof(cfg->password));
				}else if(!strcasecmp(key,"report")) {
					if(!strcasecmp(value,"json")){
						cfg->report_format = SMS_REPORT_JSON;
					}else if(!strcasecmp(value,"string")){
						cfg->report_format = SMS_REPORT_STRING;
					}else if(!strcasecmp(value,"no")){
						cfg->report_format = SMS_REPORT_NO;
					}else{
						cfg->report_format = SMS_REPORT_JSON;
					}
				}else if(!strcasecmp(key,"port")) {
					char *start = value;
					char *end = start;
					int board = 0, span = 0;
					if(*start == '\0' || !strcasecmp(start, "all")){
						for(i = 0; i < __BRD_SUM__; i++){
							for(j = 0; j < __GSM_SUM__; j++){
								cfg->port_valid[i][j] = 1;
							}
						}
					}else{
						while(*start != '\0'){
							if(sscanf(start, __GSM_HEAD__"%d.%d", &board, &span) == 2 
								&& board > 0 && board <= __BRD_SUM__
								&& span > 0 && span <= __GSM_SUM__
							){
								cfg->port_valid[board-1][span-1] = 1;
							}
							
							end = strchr(start, SMS_PORT_DELIM);
							if(end != NULL){
								start = end + 1;
							}else{
								break;
							}
						}
					}
				}else if(!strcasecmp(key,"debug")) {
					if((val = atoi(value)) > 0){
						cfg->debug = val;
					}
				}else if(!strcasecmp(key,"timeout_gsm_send")) {
					if((val = atoi(value)) > 0){
						cfg->timeout_gsm_send = val;
					}	
				}else if(!strcasecmp(key,"timeout_total")){
					if((val = atoi(value)) > 0){
						cfg->timeout_total = val;
					}
				}else if(!strcasecmp(key,"timeout_wait")){
					if((val = atoi(value)) > 0){
						cfg->timeout_wait = val;
					}
				}else if(!strcasecmp(key, "cors_enable")){ /* Add the switch of CORS */
					if(is_true(value)){
						cfg->enable_cors = 1;
					}
				}else if(!strcasecmp(key, "allow_access_origin_url")){ /* Add paramter of allow_access_origin_url */
					strncpy(cfg->allow_access_origin_url, value, sizeof(cfg->allow_access_origin_url));
				}else if(!strcasecmp(key, "usage_file")){ /* Add paramter of usage_file */
					strncpy(cfg->usage_file, value, sizeof(cfg->usage_file));
				}
			}

		}
		memset(line,0,sizeof(line));
	}

	fclose(fp);
	unlock_file(lock);

	if(cfg->use_default_user){
		strncpy(cfg->username, "smsuser", sizeof("smsuser"));
		strncpy(cfg->password, "smspwd", sizeof("smspwd"));
		/*
		char username[MAX_LEN_USERNAME];
		char password[MAX_LEN_PASSWORD];
		memset(username, 0, sizeof(username));
		memset(password, 0, sizeof(password));
		if(get_web_auth(username, sizeof(username), password, sizeof(password)) == 0){
			strncpy(cfg->username,username,sizeof(cfg->username));
			strncpy(cfg->password,password,sizeof(cfg->password));
		}
		*/
	}

	return 0;
}

int init_get_smsstatus_config(struct sms2http_config *cfg){
	char *file_path = "/etc/asterisk/gw/sms.conf";
	char line[MAX_LEN_LINE];
	char key[MAX_LEN_LINE];
	char value[MAX_LEN_LINE];
	char section[MAX_LEN_LINE];
	char kv_format[64], s_format[64];
	int val = 0, i = 0, j = 0;

	FILE* fp;
	int lock;
	lock = lock_file(file_path);
	if( NULL == (fp = fopen(file_path,"r")) ) {
		fprintf(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}
	memset(cfg, 0, sizeof(struct sms2http_config));
	/* set sscanf format */
	memset(s_format, 0, sizeof(s_format));
	memset(kv_format, 0, sizeof(kv_format));
	snprintf(s_format, sizeof(s_format), "[%%%d[^]]]", sizeof(section));
	snprintf(kv_format, sizeof(kv_format), "%%%d[^=]=%%%ds", sizeof(key), sizeof(value));

	/* read config file */
	memset(line, 0, sizeof(line));
	memset(section, 0, sizeof(line));
	while(!feof(fp) && fgets(line, sizeof(line), fp)) {
		memset(key,0,sizeof(key));
		memset(value,0,sizeof(value));
		
		if(sscanf(line, s_format, section) == 1){
			trim(section);
		}else if(section[0] != '\0' && sscanf(line, kv_format, key, value) == 2){
			trim(key);
			trim(value);
			if(!strcasecmp(section,"sms_to_http")){
				if(!strcasecmp(key, "get_smsstatus_enable")){
					if(is_true(value)){
						cfg->get_smsstatus_enabe = 1;
					}
				}else if(!strcasecmp(key, "url_from_num")){
					strncpy(cfg->url_num, value, sizeof(cfg->url_port));
				}else if(!strcasecmp(key, "url_to_num")){
					strncpy(cfg->url_port, value, sizeof(cfg->url_num));
				}else if(!strcasecmp(key, "url_message")){
					strncpy(cfg->url_message, value, sizeof(cfg->url_message));
				}else if(!strcasecmp(key, "url_time")){
					strncpy(cfg->url_time, value, sizeof(cfg->url_time));
				}else if(!strcasecmp(key, "url_status")){
					strncpy(cfg->url_status, value, sizeof(cfg->url_status));
				}else if(!strcasecmp(key, "url_id")){
					strncpy(cfg->url_id, value, sizeof(cfg->url_id));
				}else if(!strcasecmp(key, "url_imsi")){
					strncpy(cfg->url_imsi, value, sizeof(cfg->url_imsi));
				}
			}
		}
		memset(line,0,sizeof(line));
	}

	if(strlen(cfg->url_id) == 0)
		strcpy(cfg->url_id, "id");
	if(strlen(cfg->url_imsi) == 0)
		strcpy(cfg->url_imsi, "imsi");
	
	fclose(fp);
	unlock_file(lock);
			

}
int init_ussd_config(struct http2ussd_config *cfg){
	char *file_path = "/etc/asterisk/gw/ussd.conf";
	char line[MAX_LEN_LINE];
	char key[MAX_LEN_LINE];
	char value[MAX_LEN_LINE];
	char section[MAX_LEN_LINE];
	char kv_format[64], s_format[64];
	int val = 0, i = 0, j = 0;
	char total_chan_str[12] = {0};
	char *toto_chan_cmd="cat /tmp/hw_info.cfg |grep ^total_chan_count|awk -F '=' '{print $2}'";
	FILE* fp;
	int lock;

	cfg->result_format = USSD_REPORT_JSON;
	for(i = 0; i < __BRD_SUM__; i++){
		for(j = 0; j < __GSM_SUM__; j++){
			cfg->port_valid[i][j] = 0;
		}
	}

	lock = lock_file(file_path);
	if( NULL == (fp = fopen(file_path,"r")) ) {
		fprintf(stderr,"Can't open %s\n",file_path);
		unlock_file(lock);
		return -1;
	}

	/* set sscanf format */
	memset(s_format, 0, sizeof(s_format));
	memset(kv_format, 0, sizeof(kv_format));
	snprintf(s_format, sizeof(s_format), "[%%%d[^]]]", sizeof(section));
	snprintf(kv_format, sizeof(kv_format), "%%%d[^=]=%%%ds", sizeof(key), sizeof(value));

	/* read config file */
	memset(line, 0, sizeof(line));
	memset(section, 0, sizeof(line));
	while(!feof(fp) && fgets(line, sizeof(line), fp)) {
		memset(key,0,sizeof(key));
		memset(value,0,sizeof(value));
		
		if(sscanf(line, s_format, section) == 1){
			trim(section);
		}else if(section[0] != '\0' && sscanf(line, kv_format, key, value) == 2){
			trim(key);
			trim(value);
			if(!strcasecmp(section,"http_to_ussd")){
				if(!strcasecmp(key,"enable")) {
					if(is_true(value)){
						cfg->enable = 1;
					}
				}else if(!strcasecmp(key,"use_default_user")) {
					if(is_true(value)){
						cfg->use_default_user = 1;
					}
				}else if(!strcasecmp(key,"username")) {
					strncpy(cfg->username,value,sizeof(cfg->username));
				}else if(!strcasecmp(key,"password")) {
					strncpy(cfg->password,value,sizeof(cfg->password));
				}else if(!strcasecmp(key,"report")) {
					if(!strcasecmp(value,"json")){
						cfg->result_format = USSD_REPORT_JSON;
					}else if(!strcasecmp(value,"string")){
						cfg->result_format = USSD_REPORT_STRING;
					}else if(!strcasecmp(value,"no")){
						cfg->result_format= USSD_REPORT_NO;
					}else{
						cfg->result_format = USSD_REPORT_JSON;
					}
				}else if(!strcasecmp(key,"port")) {
					char *start = value;
					char *end = start;
					int board = 0, span = 0;
					if(*start == '\0' || !strcasecmp(start, "all")){
						for(i = 0; i < __BRD_SUM__; i++){
							for(j = 0; j < __GSM_SUM__; j++){
								cfg->port_valid[i][j] = 1;
							}
						}
					}else{
						while(*start != '\0'){
							if(sscanf(start, __GSM_HEAD__"%d.%d", &board, &span) == 2 
								&& board > 0 && board <= __BRD_SUM__
								&& span > 0 && span <= __GSM_SUM__
							){
								cfg->port_valid[board-1][span-1] = 1;
							}
							
							end = strchr(start, SMS_PORT_DELIM);
							if(end != NULL){
								start = end + 1;
							}else{
								break;
							}
						}
					}
				}else if(!strcasecmp(key, "cors_enable")){ /* Add the switch of CORS */
					if(is_true(value)){
						cfg->enable_cors = 1;
					}
				}else if(!strcasecmp(key, "allow_access_origin_url")){ /* Add paramter of allow_access_origin_url */
					strncpy(cfg->allow_access_origin_url, value, sizeof(cfg->allow_access_origin_url));
				}else if(!strcasecmp(key, "usage_file")){ /* Add paramter of usage_file */
					strncpy(cfg->usage_file, value, sizeof(cfg->usage_file));
				}
			}
		}
		memset(line,0,sizeof(line));
	}

	fclose(fp);
	unlock_file(lock);
	if(cfg->use_default_user){
		strncpy(cfg->username, "ussduser", sizeof("ussduser"));
		strncpy(cfg->password, "ussdpwd", sizeof("ussdpwd"));
	}
	if(exec_pipe(toto_chan_cmd, total_chan_str, 12) == 0){
		cfg->total_channel = atoi(total_chan_str);
	}
	return 0;
}

