#include "../include/header.h"
#include "../include/calllimit.h"
#include "../include/callevent.h"
#include "../include/calllimit_log.h"


call_status_t str_to_call_status(char *call_sta)
{
	if(!call_sta)
		return CALL_UNKNOW;
	
	if(!strncmp(call_sta, STR_DIAL, strlen(STR_DIAL))) {
		return CALL_DAIL;
	} else if(!strncmp(call_sta, STR_RING, strlen(STR_RING))) {
		return CALL_RING;
	} else if (!strncmp(call_sta, STR_ANSWER, strlen(STR_ANSWER))) {
		return CALL_ANSWER;
	} else if (!strncmp(call_sta, STR_HANGUP, strlen(STR_HANGUP))) {
		return CALL_HANDUP;
	}else if(!strncmp(call_sta, STR_SIMOUT, strlen(STR_SIMOUT))) {
		return SIM_OUT;
	} else if (!strncmp(call_sta, STR_SMS, strlen(STR_SMS))) {
		return SMS_SUCCESS;
	} else if (!strncmp(call_sta, STR_SMS_REPORT, strlen(STR_SMS_REPORT))) {
		return SMS_REPORT_SUCCESS;
	}else if(!strncmp(call_sta, STR_READY, strlen(STR_READY))){
        return SIM_UP;
    }else if (!strncmp(call_sta, STR_RESTART, strlen(STR_RESTART))) {
		set_reboot_state();
	} else if(!strncmp(call_sta, STR_SIMIN, strlen(STR_SIMIN))) {
		return SIM_IN;
	} else if(!strncmp(call_sta, STR_INDIAL, strlen(STR_INDIAL))) {
		return CALL_IN;
	}
	return CALL_UNKNOW;
}

const char * call_status_to_str(call_status_t call_status)
{
	switch (call_status) {
	case CALL_DAIL:
		return STR_DIAL;
	case CALL_RING:
		return STR_RING;
	case CALL_ANSWER:
		return STR_ANSWER;
	case CALL_HANDUP:
		return STR_HANGUP;
	case CALL_NO_CARRIER:
		return STR_NO_CARRIER;
	case SIM_IN:
		return STR_SIMIN;
    case SIM_OUT:
        return STR_SIMOUT;
	case SIM_UP:
		return STR_READY;
    case SMS_SUCCESS:
    case SMS_FAILED:
        return STR_SMS;
    case CALL_IN:
        return STR_INDIAL;		
	default:
		return "Unknown";
	}
	
	return "Unknown";
}

static char *get_label_value(const char *label,int lab_len,char *src, char *dest,int maxLen)
{
    int i = 0;
    if(label == NULL || src == NULL || dest == NULL)
        return NULL;
    char *p = src;
    p = strstr(p, label);
    if(p != NULL)
        p=p+lab_len;
    if(p != NULL){
        while((*p!='\r'&&*p!='\n') && (i< maxLen))
            dest[i++] = *p++;
        dest[i] = '\0';
    }
    return dest;
}

static call_status_t check_sim_out(char *readbuff)
{
    char resultStr[64] = {0};
    
    get_label_value(RESULT_LABEL,RESULT_LABEL_LEN,readbuff,resultStr,sizeof(resultStr));
    char *str = (char *)"SIM Not inserted";
    
    if(strncmp(resultStr,str ,strlen(str)) != 0)
    {
        return CALL_UNKNOW;
    } 
    
    return SIM_OUT;
}

static call_status_t check_sms_succesfully(char *readbuff)
{
    char resultStr[64] = {0};
    char addr[64] = {0};
    char channels[3] = {0};
  //  int i = 0;
  //  char *p = readbuff;

    get_label_value(CHANNEL_LABEL,CHANNEL_LABEL_LEB,readbuff,channels,sizeof(channels));
//    print_debug("Channels:%s\n",channels);
    get_label_value(SMS_DES_LABEL,SMS_DES_LABEL_LEN,readbuff,addr,sizeof(addr));
//    print_debug("Destination:%s\n",addr);
    get_label_value(SMS_STATUS_LABEL,SMS_STATUS_LABEL_LEN,readbuff,resultStr,sizeof(resultStr));
//    print_debug("Status:%s\n",resultStr);
 
    if(!strncmp(resultStr,"SUCCESS" ,7) )
    {
        return SMS_SUCCESS;
    }else if (!strncmp(resultStr,"FAIL" ,4)){
        int n = atoi(channels)-1;
        if(!compare_destination(n,addr))
            return SMS_FAILED;
    }
	//区分锁卡短信和普通短信
	return SMS_SEND_FAILED;
}

static call_status_t check_sms_report_succesfully(char *readbuff)
{
    char resultStr[64] = {0};
    char addr[64] = {0};
    char channels[3] = {0};
 //   int i = 0;
  //  char *p = readbuff;

    get_label_value(CHANNEL_LABEL,CHANNEL_LABEL_LEB,readbuff,channels,sizeof(channels));
//    print_debug("Channels:%s\n",channels);
    get_label_value(SMS_REPORT_SENDER,SMS_REPORT_SENDER_LEN,readbuff,addr,sizeof(addr));
//    print_debug("Sender:%s\n",addr);
    get_label_value(SMS_STATUS_LABEL,SMS_STATUS_LABEL_LEN,readbuff,resultStr,sizeof(resultStr));
//    print_debug("Status:%s\n",resultStr);
 
    if(!strncmp(resultStr,"SUCCESS" ,7) )
    {
        return SMS_REPORT_SUCCESS;
    }else if (!strncmp(resultStr,"FAIL" ,4)){
        int n = atoi(channels)-1;
        if(!compare_destination(n,addr))
            return SMS_REPORT_FAILED;
    }
    return CALL_UNKNOW;
}

static int set_lastcall_state(char *readbuff)
{
    char resultStr[64] = {0};
    char channels[3] = {0};

    get_label_value(CHANNEL_LABEL,CHANNEL_LABEL_LEB,readbuff,channels,sizeof(channels));
 //   print_debug("Channels:%s\n",channels);
    get_label_value(RESULT_LABEL,RESULT_LABEL_LEN,readbuff,resultStr,sizeof(resultStr));
//    print_debug("Result:%s\n",resultStr);
 
    if (!strncmp(resultStr,"Failed",6)){
        int n = atoi(channels)-1;
        put_call_limit_event(n, CALL_NO_CARRIER);
        return CALL_NO_CARRIER;
    }
    return CALL_HANDUP;
}


static int parsing_event(char *readbuff)
{

	char *p = readbuff;
	call_status_t call_status;
	char callsta[CALL_STR_LEN] = {0};
	char channels[3] = {0};
	int i = 0;
	int j = 0;

	if(!p) {
		log_printf(LOG_ERROR, "input fail\n");
		return -1;
	}

	p = strstr(p, EVENT_LABEL);
	while(p) {
		p=p+EVENT_LABEL_LEN;
		while(*p!='\n' && (i<CALL_STR_LEN))
			callsta[i++] = *p++;
		callsta[i] = '\0';
		call_status = str_to_call_status(callsta);
		
		if(call_status == SIM_OUT){
		    call_status = check_sim_out(p);
		}else if(call_status == SMS_SUCCESS){
		    call_status = check_sms_succesfully(p);
		}else if(call_status == SMS_REPORT_SUCCESS){
		    call_status = check_sms_report_succesfully(p);
		}else if(call_status == CALL_HANDUP){
		    set_lastcall_state(p);
		}else if(call_status == SIM_IN) {
			get_label_value(CHANNEL_LABEL,CHANNEL_LABEL_LEB,p,channels,sizeof(channels));
			reset_limit_chanel_state(atoi(channels)-1);
			memset(callsta, 0, sizeof(callsta));
			i = 0;
			p = strstr(p, EVENT_LABEL);
			continue;
		}

		if(call_status != CALL_UNKNOW) {
			p = strstr(p, CHANNEL_LABEL);
			if(p) {
				p=p+CHANNEL_LABEL_LEB;
				while(*p!='\n' && (j<3))
					channels[j++] = *p++;
				channels[j] = '\0';
				put_call_limit_event(atoi(channels)-1, call_status);
				memset(channels, 0, sizeof(channels));
				j = 0;
			}
		}
		memset(callsta, 0, sizeof(callsta));
		i = 0;
		p = strstr(p, EVENT_LABEL);
	}
	
	return 0;
}

static int event_socket_create(void)
{
	int sockfd = -1;
	int size = 0;	
	int flags;
	struct sockaddr_in saddr; 	
	
	size = sizeof(struct sockaddr_in); 	
	bzero(&saddr,sizeof(saddr));	
	saddr.sin_family = AF_INET;	
	saddr.sin_port = htons(SERVER_PORT); //服务器绑定的端口	
	saddr.sin_addr.s_addr = inet_addr(SERVER_IP);//服务器的IP地址 	
	sockfd = socket(AF_INET,SOCK_STREAM,0); //创建一个套接字用于连接服务器，并且这个套接字可用作对服务器操作的文件描述符	
	
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
	log_printf(LOG_INFO, "event sock recreate succs!\n");
	sleep(2);
	reset_limit_state();
	return sockfd;
}


void *check_callevent_thread_cb_handler(void * data)
{
	char buf[READ_BUFF_LEN] = {0};
	struct pollfd eventfds;
	int sockfd = -1;
	int res = -1;

	sockfd = event_socket_create();
	event_manager_login(sockfd);
	
	while(1) {
		eventfds.fd = sockfd;
		eventfds.events |= POLLIN;
		eventfds.revents = 0;
		res = poll(&eventfds, 1, EVENT_TIMEOUTMS);	
		if(eventfds.revents & POLLIN) {
			res = read(sockfd, buf, sizeof(buf));
			if(res == 0) {
				sockfd = event_sock_recreate();
			} else if (res > 0) {
				parsing_event(buf);
				memset(buf, 0, sizeof(buf));
			}
		}
	}
	return 0;
}


int event_thread_create(void)
{
	pthread_t event_tid;

	calllimit_pthread_create(&event_tid, NULL, check_callevent_thread_cb_handler, NULL);

	return 0;	
}





