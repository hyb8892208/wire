#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>

#include <errno.h>

#define THREAD_COUNT 8
#define SOCKET_PORT 8000
#define GSM_SUM 8 //4
#define BRD_SUM 11
#define GSM_HEAD "gsm-"
#define BRD_HEAD "Board-"

char clusterip_array[BRD_SUM][32];

struct cs {
	int client_sockfd;	//connect socket handle.
	const char *pbuf;	//Point to receive buf.
};

void usage(const char * str)
{
	fprintf(stderr,"Usage1: %s master [1=slaveip_1] [2=slaveip_2] ...\n",str);
	fprintf(stderr,"Usage2: %s slave ip \n",str);
	fprintf(stderr,"Usage3: %s stand \n",str);
}

/*char* get_spans(int * len)
{
	FILE *pf;
	pf = fopen("/tmp/gsm/spans","r");
	if(pf) {
		char *text;

		int fd ;
		fd = fileno(pf);
		flock(fd, LOCK_EX);

		fseek(pf,0,SEEK_END);
		long lSize = ftell(pf);

		text = (char*)malloc(lSize+1+1);
		rewind(pf);
		fread(text,sizeof(char),lSize,pf);
		flock(fd, LOCK_UN);
		fclose(pf);

		text[lSize] = '\n';
		text[lSize+1] = '\0';
		*len = lSize+1+1;

		return text;
	}

	return NULL;
}*/

char* get_spans(int span)
{
	FILE *pf;
	char path[256];
	snprintf(path,sizeof(path),"/tmp/gsm/%d",span);
	pf = fopen(path,"r");
	if(pf) {
		char *text;
		int real_size;

		int fd ;
		fd = fileno(pf);
		flock(fd, LOCK_EX);

		fseek(pf,0,SEEK_END);
		long lSize = ftell(pf);

		text = (char*)malloc(lSize+1+1);
		rewind(pf);
		real_size = fread(text,sizeof(char),lSize,pf);
		flock(fd, LOCK_UN);
		fclose(pf);

		text[real_size] = '\n';
		text[real_size+1] = '\0';

		return text;
	}

	return NULL;
}

/*
int ast_running()
{
	if(access("/var/run/asterisk.ctl",F_OK) != -1) {
		return 1;
	}

	return 0;
}
*/

int ast_running()
{
	struct stat statbuf;

	if( -1 == access("/var/run/asterisk.ctl",F_OK) ) return 0;

	if( -1 == stat("/tmp/gsm/1", &statbuf) ) return 0;

	if(time(NULL) - statbuf.st_mtime > 10) return 0;

	return 1;
}

/*
void request_get_spans(struct cs* pCS)
{
	if(ast_running()) {
		char* spans_info;
		int len;
		spans_info = get_spans(&len);
		if(spans_info) {
			send(pCS->client_sockfd, spans_info, len, 0);
			free(spans_info);
		}
	}
	close(pCS->client_sockfd);
}*/

void request_get_spans(struct cs* pCS)
{
	if(ast_running()) {
		char* spans_info;
		int len;
		int span;
		char send_buf[4096] = {0};
		char buf[64] = {0};

		for(span=1; span<=GSM_SUM; span++) {
			snprintf(buf,sizeof(buf),"---Start Span %d---\n",span);
			strncat(send_buf, buf, sizeof(send_buf) - strlen(send_buf) - 1);

			spans_info = get_spans(span);
			strncat(send_buf,spans_info, sizeof(send_buf) - strlen(send_buf) - 1);
			free(spans_info);

			snprintf(buf,sizeof(buf),"---End Span %d---\n",span);
			strncat(send_buf, buf, sizeof(send_buf) - strlen(send_buf) - 1);
		}

		if((len = strlen(send_buf)) > 0) {
			send(pCS->client_sockfd, send_buf, len, 0);
		}
	}
	close(pCS->client_sockfd);
}

void request_setto_mode(struct cs* pCS)
{
	char masterip[64] = {0};
	char slaveip[64] = {0};
	int password;
	int remain_ori_ip = 1;
	sscanf(pCS->pbuf,"mode=slave;password=%d;masterip=%[0-9.];slaveip=%[0-9.];remain_ori_ip=%d\n",&password,masterip,slaveip,&remain_ori_ip);

	FILE *fp;
	fp = fopen("/etc/asterisk/gw/cluster.conf","w");
	if(fp) {
		char write[256];
		sprintf(write,"[general]\nmode=slave\n[slave]\npassword=%d\nip=%s\nmasterip=%s\nremain_ori_ip=%d\n",password,slaveip,masterip,remain_ori_ip);
		fwrite(write,1,strlen(write),fp);
		fclose(fp);
	}
	system("/my_tools/cluster_mode > /dev/null 2>&1 &");
	close(pCS->client_sockfd);
}


void request_ping(struct cs* pCS)
{
	send(pCS->client_sockfd, "ping\n", strlen("ping\n"), 0);
	close(pCS->client_sockfd);
}

void request_exec_astcmd(struct cs* pCS)
{
	char cmd[1024];
	char *pcmd;
	int len;
	char astcmd[1024];
	int err;
	FILE *stream;
	char sendbuf[1024];
	char *psendbuf;
	int size;
	char buf[1024];

	strncpy(cmd,pCS->pbuf,sizeof(cmd));
	len = strlen(cmd);
	if(len && cmd[len-1] == '\n') {
		cmd[len-1] = '\0';
	}
	pcmd = cmd + strlen("astcmd:");

	snprintf(astcmd,sizeof(astcmd),"asterisk -rx \"%s\"",pcmd);

	psendbuf = sendbuf;
	memset(sendbuf,0,sizeof(sendbuf));

	if( (stream = popen(astcmd, "r")) == NULL ) {
		close(pCS->client_sockfd);
		return;
	}

	while( fgets(buf, sizeof(buf), stream) ) {
		strncat(sendbuf, buf, sizeof(sendbuf) - strlen(sendbuf) - 1 - 2);
	}  
	pclose(stream);

	len = strlen(sendbuf);
	if(len && len < (sizeof(sendbuf)-1)) {
		sendbuf[len] = '\n';
		sendbuf[len+1] = '\0';
		send(pCS->client_sockfd, sendbuf, strlen(sendbuf), 0);
	}

	close(pCS->client_sockfd);
}

void request_exec_syscmd(struct cs* pCS)
{
	char cmd[1024];
	char *pcmd;
	int len;
	int err;
	FILE *stream;
	char sendbuf[1024];
	char *psendbuf;
	int size;
	char buf[1024];

	psendbuf = sendbuf;
	memset(sendbuf,0,sizeof(sendbuf));

	strncpy(cmd,pCS->pbuf,sizeof(cmd));
	len = strlen(cmd);
	if(len && cmd[len-1] == '\n') {
		cmd[len-1] = '\0';
	}
	pcmd = cmd + strlen("syscmd:");

	if( (stream = popen(pcmd, "r")) == NULL ) {
		close(pCS->client_sockfd);
		return;
	}

	while( fgets(buf, sizeof(buf), stream) ) {
		strncat(sendbuf, buf, sizeof(sendbuf) - strlen(sendbuf) - 1 - 2);
	}  
	pclose(stream);

	len = strlen(sendbuf);
	if(len && len < (sizeof(sendbuf)-1)) {
		sendbuf[len] = '\n';
		sendbuf[len+1] = '\0';
		send(pCS->client_sockfd, sendbuf, strlen(sendbuf), 0);
	}

	close(pCS->client_sockfd);
}


void request_get_slavesms(struct cs* pCS)
{
	int	i,j,board;
	char	slaveip[32];
	char	port[128];
	char	board_port[128];
	char	phonenumber[64];
	char	time[32];
	char	message[1024];
	char	buf[1024];

	//Not Need.
	close(pCS->client_sockfd);

	//get slaveip
	j = 0;
	for(i=strlen("slavesms:");;i++){
		if(pCS->pbuf[i] == '\t'){
			break;
		}
		slaveip[j++]=pCS->pbuf[i];
		if((sizeof(slaveip)-1)==j){
			break;
		}
	}
	slaveip[j] = '\0';

	//get port
	j = 0;
	for(i++;;i++){
		if(pCS->pbuf[i] == '\t'){
			break;
		}
		port[j++]=pCS->pbuf[i];
		if((sizeof(port)-1)==j){
			break;
		}
	}
	port[j] = '\0';
	
	//get phonenumber
	j = 0;
	for(i++;;i++){
		if(pCS->pbuf[i] == '\t'){
			break;
		}
		phonenumber[j++]=pCS->pbuf[i];
		if((sizeof(phonenumber)-1)==j){
			break;
		}
	}
	phonenumber[j] = '\0';
	
	//get time
	j = 0;
	for(i++;;i++){
		if(pCS->pbuf[i] == '\t'){
			break;
		}
		time[j++]=pCS->pbuf[i];
		if((sizeof(time)-1)==j){
			break;
		}
	}
	time[j] = '\0';
	
	//get message
	j = 0;
	for(i++;i<sizeof(message);i++){
		if(pCS->pbuf[i] == '\0'){
			break;
		}
		message[j++]=pCS->pbuf[i];
	}

	//clear last character '\n'
	if(j>0) {
		if(message[j-1] == '\n') {
			message[j-1] = '\0';
		}
	}

	for(j=0;j<BRD_SUM;j++) {
		if(strcmp(clusterip_array[j], slaveip)==0) {
			board = j+1;
			sprintf(board_port, BRD_HEAD"%d-"GSM_HEAD"%s", board, port);
		}
	}

	snprintf(buf,sizeof(buf),"cd /my_tools/lua/sms_receive/ && lua sms_receive.lua \"%s\" \"%s\" \"%s\" \"%s\" > /dev/null 2>&1 &",board_port,phonenumber,time,message);
		
	system(buf);
}

static int get_remain(int span)
{
	char path[256];
	char buf[4096];
	int len;
	int fd;

	snprintf(path, sizeof(path), "/tmp/gsm/%d",span);
	fd = open(path, O_CREAT|O_RDWR|O_APPEND, 0774);
	if(fd < 0) {
		fprintf(stderr,"Unable to open %s: %s\n", path, strerror(errno));
		return -1;
	}
	if(flock(fd, LOCK_EX) < 0) {
		fprintf(stderr,"File %s was not locked.\n",path);
		close(fd);
		return -1;
	}

	len = read(fd,buf,sizeof(buf));

	if((flock(fd, LOCK_UN)) < 0) {
		fprintf(stderr,"File %s unlocked error.\n",path);
		close(fd);
		return -1;
	}
	close(fd);

	if(len > 0) {
		char *start = NULL;
		start = strstr(buf,"Remain Time: ");
		if(start) {
			char *end = NULL;
			end = strchr(start,'\n');
			if(end) {
				int remain = -1;
				*end = '\0';
				if(strlen(start) > (sizeof("Remain Time: ")-1)) {
					char * p = start + sizeof("Remain Time: ")-1;
					return atoi(p);
				}
			}
		}
	}

	return -1;
}

void request_get_diallimit_remain_time(struct cs* pCS)
{
	char send_buf[128];

	strcpy(send_buf,"-1\n");

	if(ast_running()) {
		int span = -1;
		if(sscanf(pCS->pbuf,"get_diallimit_remain_time:%d\n",&span)) {
			if(span>=1 && span<=GSM_SUM) {
				int remain;
				remain = get_remain(span);
				snprintf(send_buf,sizeof(send_buf),"%d\n",remain);
			}
		}
	}
	
	send(pCS->client_sockfd, send_buf, strlen(send_buf), 0);
	close(pCS->client_sockfd);
}


void request_putfile(struct cs* pCS)
{
	//put file
	// 1. client --> server
	//    "putfile:filepath:filesize\n"
	// 2. server --> client
	//    "ready\n" or "error\n" .....
	// 3. client --> server
	//    read client file send to server
	// 4. server --> client
	//    "ok\n" or "error\n" .....

	char filepath[256+1];
	int filesize;
	char send_buf[1024];
	char recv_buf[1024+1];
	int recvsize = 0;
	int len;

	// 1. client --> server
	//    "putfile:filepath:filesize\n"
	sscanf(pCS->pbuf,"putfile:%256[^:]:%d\n",filepath,&filesize);

	FILE *tempfp;  
	tempfp = tmpfile();
	if(tempfp == NULL ) {
		goto error;
	}


	// 2. server --> client
	//    "ready\n" or "error\n" .....
	strcpy(send_buf,"ready\n");	
	send(pCS->client_sockfd, send_buf, strlen(send_buf), 0);


	// 3. client --> server
	//    read client file send to server
	while( (len=recv(pCS->client_sockfd,recv_buf,sizeof(recv_buf)-1,0)) > 0 ) {
		recvsize += len;
		if (recvsize > filesize) {
			fclose(tempfp);
			goto error;
		}

		fwrite(recv_buf,1,len,tempfp);

		if (recvsize == filesize) { //Read file over.
			break;
		}
	}
	fseek(tempfp,0,SEEK_SET);

	if (recvsize != filesize) {
		fclose(tempfp);
		goto error;
	}

	int fd;
	fd = open(filepath, O_CREAT|O_WRONLY|O_TRUNC, 0774);
	if(fd < 0) {
		fclose(tempfp);
		goto error;
	}
	flock(fd, LOCK_EX);
	while( (len = fread(recv_buf,1,sizeof(recv_buf)-1,tempfp)) > 0 ) {
		write(fd,recv_buf,len);
	}
	flock(fd, LOCK_UN);
	close(fd);
	fclose(tempfp);


	// 4. server --> client
	//    "ok\n" or "error\n" .....
	strcpy(send_buf,"ok\n");
	send(pCS->client_sockfd, send_buf, strlen(send_buf), 0);
	close(pCS->client_sockfd);
	return;

error:
	strcpy(send_buf,"error\n");
	send(pCS->client_sockfd, send_buf, strlen(send_buf), 0);
	close(pCS->client_sockfd);
	return;
}


void request_getfile(struct cs* pCS)
{
	//get file
	// 1. client --> server
	//    "getfile:filepath\n"
	// 2. server --> client
	//    "filesize:filesize\n" or "error\n" .....
	// 3. client --> server
	//    "ready\n" or "error\n" .....
	// 4. server --> client
	//    read server file send to client
	// 5. client --> server
	//    "over\n" or "error\n" .....
	// 6. server --> client
	//    "ok\n" or "error\n" .....

	char filepath[256+1];
	int filesize;
	char send_buf[1024+1];
	char recv_buf[1024+1];
	int recvsize = 0;
	int len;


	// 1. client --> server
	//    "getfile:filepath\n"
	sscanf(pCS->pbuf,"getfile:%256s\n",filepath);

	int fd;

	fd = open(filepath, O_RDONLY, 0774);
	if(fd < 0) {
		goto error;
	}

	filesize = lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);

	// 2. server --> client
	//    "filesize:filesize\n" or "error\n" .....
	sprintf(send_buf,"filesize:%d\n",filesize);
	send(pCS->client_sockfd, send_buf, strlen(send_buf), 0);


	// 3. client --> server
	//    "ready\n" or "error\n" .....
	if( (len = recv(pCS->client_sockfd,recv_buf,sizeof(recv_buf)-1,0)) > 0 ) {
		recv_buf[len] = '\0';

		if (strcmp("ready\n",recv_buf)) {
			close(fd);
			goto error;
		} 
	} else {
		close(fd);
		goto error;
	}

	// 4. server --> client
	//    read server file send to client
	flock(fd, LOCK_EX);
	while( (len=read(fd,send_buf,sizeof(send_buf)-1)) > 0 ) {
		if(send(pCS->client_sockfd, send_buf, len, 0) != len) {
			flock(fd, LOCK_UN);
			close(fd);
			goto error;
		}
	}
	flock(fd, LOCK_UN);
	close(fd);


	// 5. client --> server
	//    "over\n" or "error\n" .....
	if( (len = recv(pCS->client_sockfd,recv_buf,sizeof(recv_buf)-1,0)) > 0 ) {
		recv_buf[len] = '\0';
		if (strcmp("over\n",recv_buf)) {
			goto error;
		} 
	} else {
		goto error;
	}


	// 6. server --> client
	//    "ok\n" or "error\n" .....
	strcpy(send_buf,"ok\n");
	send(pCS->client_sockfd, send_buf, strlen(send_buf), 0);
	close(pCS->client_sockfd);
	return;

error:
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	strcpy(send_buf,"error\n");
	send(pCS->client_sockfd, send_buf, strlen(send_buf), 0);
	close(pCS->client_sockfd);
	return;
}

void *connect_func(void *arg)
{
	int client_sockfd;
	int len;
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	int sin_size;
	int server_sockfd;
	server_sockfd = (int)arg;
	char buf[1024];

	sin_size = sizeof(struct sockaddr_in);

	for(;;) {
		if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr, &sin_size))<0) {
			perror("accept");
			return (void*)0;
		}

		fcntl(client_sockfd, F_SETFD, FD_CLOEXEC); //forbid child process inherit socket

		while( (len=recv(client_sockfd,buf,1024,0))>0 ) {
			buf[len]='\0';

			struct cs CS;
			CS.client_sockfd = client_sockfd;
			CS.pbuf = buf;

			struct timeval timeout; 
			timeout.tv_sec = 5;  /* 5 Secs Timeout */ 
			timeout.tv_usec = 0;  // Not init'ing this can cause strange errors 
			setsockopt(client_sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval)); 
			setsockopt(client_sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval));

			if(!strncmp(buf,"get_spans",strlen("get_spans"))) {
				request_get_spans(&CS);
			} else if(!strncmp(buf,"mode=slave;",strlen("mode=slave;"))) {  //"mode=slave;password=9999;ip=192.168.9.2\n"
				request_setto_mode(&CS);
			} else if(!strncmp(buf,"ping",strlen("ping"))) {
				request_ping(&CS);
			} else if(!strncmp(buf,"astcmd:",strlen("astcmd:"))) {
				request_exec_astcmd(&CS);
			} else if(!strncmp(buf,"syscmd:",strlen("syscmd:"))) {
				request_exec_syscmd(&CS);
			} else if(!strncmp(buf,"slavesms:",strlen("slavesms:"))) {
				request_get_slavesms(&CS);
			} else if(!strncmp(buf,"get_diallimit_remain_time:",strlen("get_diallimit_remain_time:"))) {
				request_get_diallimit_remain_time(&CS);
			} else if(!strncmp(buf,"getfile:",strlen("getfile:")) && (buf[len-1] == '\n')) {
				request_getfile(&CS);
			} else if(!strncmp(buf,"putfile:",strlen("putfile:")) && (buf[len-1] == '\n')) {
				request_putfile(&CS);
			}
		}
		close(client_sockfd);
	}

	return (void*)0;
}


int main(int argc, char *argv[])
{
	int server_sockfd;
	int client_sockfd;
	int len;
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	int sin_size, flag = 1, i = 0, j = 0, board_id = 0;
	char slaveip[32];

	if(argc <= 1) {
		usage(argv[0]);
		return 1;
	}
	
	if( strcmp(argv[1],"master") == 0 ) {
		if(argc <= 2 || argc > (BRD_SUM+1)){ 
			usage(argv[0]);
			return 1;
		}
		memset(&clusterip_array, 0, sizeof(clusterip_array));
		for(i=2; i<argc; i++) {
			if(0 != sscanf(argv[i], "%d=%32s", &board_id, slaveip)){
				if(board_id <= BRD_SUM && board_id >= 2) {
					board_id = board_id - 1;
					strncpy(clusterip_array[board_id], slaveip, sizeof(clusterip_array[i]));
				} else {
					usage(argv[0]);
					return 1;
				}
			}
		}
	} else if( strcmp(argv[1],"slave") == 0 ) {

	} else if( strcmp(argv[1],"stand") == 0 ) {
	
	} else {
		usage(argv[0]);
		return 1;
	}
	

	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(SOCKET_PORT);
	
	if((server_sockfd = socket(PF_INET,SOCK_STREAM,0))<0) {
		perror("socket");
		return 1;
	}
 
	if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
		perror("setsockopt");
		return 1;
	}

	if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0) {
		perror("bind");
		return 1;
	}
	
	listen(server_sockfd,THREAD_COUNT);

	fcntl(server_sockfd, F_SETFD, FD_CLOEXEC); //forbid child process inherit socket
	
	for(i=0; i<THREAD_COUNT; i++) {
		pthread_t ntid;
		pthread_create(&ntid, NULL, connect_func, (void*)server_sockfd);
	}

	for(;;)
		sleep(10000);

	close(server_sockfd);

	return 0;
}

