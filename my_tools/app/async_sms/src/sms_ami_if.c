#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
//#include "sms_ami_queue.h"
#include "sms_ami_if.h"
#include "sms_list.h"
#include "sms_redis_if.h"
#include "log_debug.h"

#define SERVER_PORT            5038
#define SERVER_IP              "127.0.0.1"
#define WRITE_BUFF_LEN         256
#define BUFF_LEN               2048
#define EVENT_TIMEOUTMS        1000
#define SMS_SENDSTAUS_STR      "Event: SMSSendStatus"
#define EVENT_LABEL            "Event: "
#define CALL_STR_LEN           32

#define ID_STR            "ID: "
#define STATUS_STR        "Status: "
#define CHANNEL_ID_STR     "Channel: "

//Username end Secret config by /etc/asterisk/manager.conf
#define LOGIN_STR              "Action:Login\nUsername:event\nSecret:event\n\n"

struct ami_msg{
	int chan_id;
	char uuid[32];
	char imsi[32];
	char to_phone[32];
	unsigned char status;
};

struct sms_timeout_info{
	struct sms_list sms_timeout_list;
	pthread_mutex_t sms_lock;
};

struct sms_timeout_info sms_msg;

static int event_socket_create(){
	int sockfd = -1;
	int size = 0;
	int flags;
	struct sockaddr_in saddr;

	size = sizeof(struct sockaddr_in);
	bzero(&saddr,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(SERVER_PORT);
	saddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(sockfd < 0) {
		fprintf(stderr, "socket error : %s\n",strerror(errno));
		return -1;
	}

	if(connect(sockfd,(struct sockaddr*)&saddr,size) == -1) {
		fprintf(stderr, "connect error : %s\n",strerror(errno));
		return -1;
	}

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	return sockfd;
}

static int event_manager_login(int sockfd)
{
	char wbuf[WRITE_BUFF_LEN] = LOGIN_STR;
	int res = -1;

	res = write(sockfd, wbuf, sizeof(wbuf));
	return res;
}

static int event_sock_recreate(void)
{
	int sockfd = -1;
	while(1) {
		sockfd = event_socket_create();
		if(sockfd > 0) {
			event_manager_login(sockfd);
			break;
		}
		sleep(1);
	}
	return sockfd;
}
static int get_lable_value(const char *lable, char *src_buf, char *value){
	char *pos = NULL;
	char *ptr = value;
	int i = 0;
	if(!lable)
		return -1;
	pos = strstr(src_buf, lable);
	if(!pos)
		return -1;
	pos += strlen(lable);
	while(*pos != '\r' && (i++ < CALL_STR_LEN))
		*ptr++ = *pos++;
	*ptr = '\0';
	return 0;
}

int insert_to_fail_list(int chan_id, char *uuid, char *message, char *phonenumber){

	int ret = -1;
	
	struct sms_message msg;

	msg.chan_id = chan_id;

	strncpy(msg.uuid, uuid, sizeof(msg.uuid));

	strncpy(msg.message, message, sizeof(msg.message));

	strncpy(msg.phonenumber, phonenumber, sizeof(msg.phonenumber));
	
	msg.last_time= time(NULL);

	log_print(INFO," [%d]insert to fail list: uuid:%s, mesage:%s\n", chan_id, uuid, message);
	
	pthread_mutex_lock(&sms_msg.sms_lock);
	
	ret = sms_list_insert(&sms_msg.sms_timeout_list, &msg);

	pthread_mutex_unlock(&sms_msg.sms_lock);
	
	return ret;
}

void check_fail_overtime(struct sms_list *p_list){
	struct sms_list *head = p_list;
	struct sms_list *temp = p_list->_next;
	time_t time_now = time(NULL);
	while(temp){
		if(time_now - temp->sms.last_time > 150)//2min30s => 150s
		{
			//insert to outbox.
			insert_sms_to_outbox( temp->sms.chan_id, temp->sms.message, 0, temp->sms.uuid, "", temp->sms.phonenumber );
			head->_next = temp->_next;
			free(temp);
			temp = NULL;
			temp = head->_next;
		}else{
			break;
		}
	}
}

void check_sms_fail_list(struct ami_msg *p_ami_msg){
	int ret = 0;
	struct sms_message sms;
	pthread_mutex_lock(&sms_msg.sms_lock);
	ret  = sms_list_find(&sms_msg.sms_timeout_list, p_ami_msg->chan_id, p_ami_msg->uuid, &sms);
	pthread_mutex_unlock(&sms_msg.sms_lock);
	if(ret == 0){
		insert_sms_to_outbox( sms.chan_id, sms.message, p_ami_msg->status, sms.uuid, p_ami_msg->imsi, sms.phonenumber );
	}
}

static int parsing_event(char *readbuf){
	char *p  = readbuf, *pos = NULL;
	int i = 0;
	char callsta[32] = {0};
	char event_buf[1024] = {0};
	char chan_id_str[32] = {0};
	char uuid_str[64] = {0};
	char status_str[32] = {0};
	struct ami_msg msg;
	if(!p){
		return -1;
	}
	p = strstr(p, EVENT_LABEL);
	if(!p)
		return -1;
	while(p){
		i = 0;
		p = p + strlen(EVENT_LABEL);
		while(*p != '\r' && (i < CALL_STR_LEN))
			callsta[i++] = *p++;
		callsta[i] = '\0';
		memset(event_buf, 0, sizeof(event_buf));
		pos = strstr(p, EVENT_LABEL);
		if(pos){
			strncpy(event_buf, p, pos - p );
		}else{
			strncpy(event_buf, p, sizeof(event_buf));
		}
		if(strncmp(callsta, "SMSSendStatus", strlen("SMSSendStatus")) == 0){

			memset(chan_id_str, 0, sizeof(chan_id_str));

			memset(uuid_str, 0, sizeof(uuid_str));

			memset(status_str, 0, sizeof(status_str));

			if(get_lable_value(CHANNEL_ID_STR, event_buf, chan_id_str) < 0){
				log_print(ERROR,"parsing SMSSendStatus fail.\n");
				p = strstr(p, EVENT_LABEL);
				continue;
			}

			msg.chan_id = atoi(chan_id_str);

			if(get_lable_value( ID_STR, event_buf,msg.uuid) < 0){
				log_print(ERROR,"parsing SMSSendStatus id fail.\n");
				p = strstr(p, EVENT_LABEL);
				continue;
			}

			if(get_lable_value(STATUS_STR, event_buf, status_str) < 0){
				log_print(ERROR,"parsing SMSSendStatus status fail.\n");
				p = strstr(p, EVENT_LABEL);
				continue;
			}

			if(strcasecmp(status_str, "SUCCESS") == 0){
				msg.status = 1;
			}else{
				msg.status = 0;
			}
			
			if(get_lable_value("IMSI: ", event_buf, msg.imsi) < 0){
				log_print(ERROR,"parsing SMSSendStatus imsi fail.\n");
				p = strstr(p, EVENT_LABEL);
				continue;
			}
			log_print(INFO, "recevie sms result:[%d] uuid:%s, status:%s, imsi:%s\n", msg.chan_id, msg.uuid, status_str, msg.imsi);
			check_sms_fail_list(&msg);
			// printf("chan[%s]:%s ,ID:%s, Status:%s\n", chan_id_str, callsta, uuid_str, status_str);
		}
		p = strstr(p, EVENT_LABEL);
	}
	return 0;
}

void *ami_event_hander(void *data){
	int sockfd;
	int res;
	int i = 0;
	struct pollfd eventfds;
	char buf[2048];
	sockfd = event_sock_recreate();
	event_manager_login(sockfd);

	while(1) {
		eventfds.fd = sockfd;
		eventfds.events |= POLLIN;
		eventfds.revents = 0;
		res = poll(&eventfds, 1, EVENT_TIMEOUTMS);
		if(res < 0) {
			fprintf(stderr, "poll(%d) error : %s\n",eventfds.fd, strerror(errno));
		}
		if(eventfds.revents & POLLIN) {
			res = read(sockfd, buf, sizeof(buf));
			if(res == 0) {
				sockfd = event_sock_recreate();
			} else if (res > 0) {
				usleep(300000);//sleep 300ms, wait for send_sms return.
				parsing_event(buf);
				memset(buf, 0, sizeof(buf));
			}
		}else{
			usleep(20000);
		}
		if(i > 500){// 1 minute
			pthread_mutex_lock(&sms_msg.sms_lock);
			check_fail_overtime(&sms_msg.sms_timeout_list);
			pthread_mutex_unlock(&sms_msg.sms_lock);
			i = 0;
		}else{
			i++;
		}
	}
	return (void *)NULL;
}

void create_ami_thread(void *data){
	pthread_attr_t attr;
	pthread_t ami_event_thread;
	pthread_attr_init(&attr);
	log_print(INFO, "prepare create ami thread success\n");
	int ret = pthread_attr_setstacksize(&attr, 128 * 1024);
	if(0 != ret){
		log_print(INFO, "ret=%d\n", ret);
		return ;
	}
	pthread_create(&ami_event_thread, &attr, ami_event_hander, data);
	log_print(INFO, "create ami thread success\n");
}


int sms_ami_init(void){
	log_print(INFO, "create ami thread init\n");
	sms_list_init(&sms_msg.sms_timeout_list);
	pthread_mutex_init(&sms_msg.sms_lock, NULL);
	init_ami_redis_cli();
	create_ami_thread(&sms_msg);
	return 0;
}

