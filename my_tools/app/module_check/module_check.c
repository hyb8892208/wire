/*
 *function: check module_state
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT            5038
#define SERVER_IP              "127.0.0.1"
#define WRITE_BUFF_LEN         256
#define BUFF_LEN               2048
#define EVENT_TIMEOUTMS        1000
#define SMS_RECEVIE_STR        "Event: SMSRecevied"
#define EVENT_LABEL            "Event: "
//#define RESPONS_LABEL          "Response: "
#define RESPONS_LABEL           "Response: Follows"
#define ACTIONID_LABEL         "ActionID: module-check-"
#define ENDCOMMAND_LABEL       "--END COMMAND--"
#define CALL_STR_LEN           32
#define LOG_FILE               "/tmp/log/module_check.log"

//Username end Secret config by /etc/asterisk/manager.conf
#define LOGIN_STR              "Action:Login\nUsername:admin\nSecret:NS1hPs2d\n\n"
//#define LOGIN_STR "Action:Login\nUsername:admin\nKey:62577330f898a1ebd844ac0570978340\nAuthType:md5\n\n"

typedef enum sim_state_e{
	SIM_STATE_UNKOWN,
	SIM_STATE_REMOVE,
	SIM_STATE_INSERT,
	SIM_STATE_READY,
}sim_state_t;

typedef struct chan_info_s{
	int chan_id;
	int check_count;
	enum sim_state_e sim_state;
}chan_info_t;

typedef struct hw_info_s{
	int total_chan;
}hw_info_t;

typedef struct event_buf_s{
	int len;
	char buf[1024];
}event_buf_t;

struct module_check_info_s{
	struct hw_info_s hwinfo;
	struct chan_info_s* chans;
	struct event_buf_s event_buf;
	int sockfd;
	pthread_t event_id;//控制线程启动的id
	pthread_t detect_id;
	pthread_mutex_t socket_lock;
};

struct module_check_info_s module_check_info;

FILE *log_handle = NULL;

void get_timestr(char *pszTimeStr)
{
	struct tm      tSysTime     = {0};
	struct timeval tTimeVal     = {0};
	time_t         tCurrentTime = {0};

	char  szUsec[20] = {0};
	char  szMsec[20] = {0};

	if (pszTimeStr == NULL)
	{
		return;
	}

	tCurrentTime = time(NULL);
	localtime_r(&tCurrentTime, &tSysTime);

	gettimeofday(&tTimeVal, NULL);
	sprintf(szUsec, "%06d", tTimeVal.tv_usec);
	strncpy(szMsec, szUsec, 3);

	sprintf(pszTimeStr, "%04d.%02d.%02d %02d:%02d:%02d.%3.3s",
			tSysTime.tm_year+1900, tSysTime.tm_mon+1, tSysTime.tm_mday,
			tSysTime.tm_hour, tSysTime.tm_min, tSysTime.tm_sec, szMsec);
}

void record_log(char *level,char *format,...)
{

	va_list va_args;
	char buf[1024] = {0};
	char time_str[128] = {0};
	char* pbuf = buf;
	time_t t = time(NULL);
	get_timestr(time_str);
	va_start(va_args, format);

	vsnprintf(buf, 1024, format, va_args);
	if(NULL == log_handle){
		printf("[%s] %s %s\n",level, time_str, buf);
	}else{
		fprintf(log_handle,"[%s] %s %s\n",level, time_str, buf);
		fflush(log_handle);
	}
	va_end(va_args);

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
	char *auther_type = "Action: Challenge\nAuthType: md5\n";
	char wbuf[WRITE_BUFF_LEN] = LOGIN_STR;
	char read_buf[1024] = {0};
	int res = -1;
	int try_cnt = 3;
	res = write(sockfd, wbuf, strlen(wbuf));
	if(res < 0){
		return -1;
	}
	return res;

}

static int event_sock_recreate(void)
{
    int sockfd = -1;
	int i = 0;
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

static int parsing_event(struct module_check_info_s *module_check_info,char *readbuff)
{
	char *p = readbuff;
	int i = 0;
	int j = 0;
	char callsta[32] = {0};
	int state = -1;
	char chan_id_str[32] = {0};
	char chan_result[32] = {0};
	char *res_p;
	int chan_id = -1;
	if(!p) {
		record_log("ERROR","input fail\n");
		return -1;
	}
	p = strstr(p, EVENT_LABEL);
	if(!p)
		return -1;
	while(p){
		state = -1;
		i = 0;
		p = p + strlen(EVENT_LABEL);
		while(*p!='\r' && (i<CALL_STR_LEN))
			callsta[i++] = *p++;
		callsta[i] = '\0';
		if(strncmp(callsta, "ExtraReady", strlen("ExtraReady")) == 0){
			state = SIM_STATE_READY;
//			record_log("INFO","ExtraReady:");
		}else if(strncmp(callsta, "Dial", strlen("Dial")) == 0){
			state = SIM_STATE_READY;
//			record_log("INFO","Dial:");
		}else if(strncmp(callsta, "ExtraDown", strlen("ExtraDown")) == 0){
			i = 0;
			res_p = strstr(p, "Result: ");
			if(res_p){
				res_p += strlen("Result: ");
				while (*res_p != '\r' && (i < 32)){
					chan_result[i++] = (*res_p++);
				}
				chan_result[i] = '\0';
				if(strcmp(chan_result, "SIM Not inserted") == 0)
					state = SIM_STATE_REMOVE;
			}
//			record_log("INFO", "ExtraDown:");
		}else if(strncmp(callsta, "SimInserted", strlen("SimInserted")) == 0){
			state = SIM_STATE_INSERT;
//			record_log("INFO","SIM_STATE_INSERT:");
		}

		if(state > 0){
			j = 0;
			if(!strstr(p, "Channel:")){
				p = strstr(p, EVENT_LABEL);
				continue;
			}
			p = strstr(p, "Channel:");
			p += strlen("Channel:");
			while(*p != '\n' && (j < CALL_STR_LEN))
				chan_id_str[j++] = *p++;
			chan_id_str[j] = '\0';
			chan_id = atoi(chan_id_str);
			module_check_info->chans[chan_id - 1].sim_state = state;
//			record_log("INFO","%d, %s\n", chan_id, callsta);
		}
		p = strstr(p, EVENT_LABEL);
	}
	return 0;
}

static int get_at_result(char *respons)
{
	const char *success_lable = "OK";
	const char *fail_lable = "TIMEOUT";
	if(strstr(respons, success_lable)){
		return 0;
	}else if(strstr(respons, fail_lable)){
		return 1;
	}else
		return 2;
	return -1;
}

static int get_state_result(char *respons){
	const char *state_label = "State: ";
	char state[256] = {0};
	char *pos = NULL;
	char *p = state;
	pos = strstr(respons, state_label);
	if(pos){
		pos += strlen(state_label);
		while(*pos != '\n'){
			*p++ = *pos++;
		}
		*p = '\0';
		if(strcmp(state, "INIT") == 0)
			return 1;
	}

	return 0;
}

static int get_chan_id(char *respons)
{
	char *p = NULL;
	
	p = strstr(respons, ACTIONID_LABEL);
	if(!p){
		return -1; 
	}
	
	p = p + strlen(ACTIONID_LABEL);
	
	return atoi(p);
}

static int parsing_response(struct module_check_info_s *module_check_info, char *readbuff)
{
	char *p = readbuff;
	int i = 0;
	int j = 0;
	int chan_id = -1;
	int at_result = -1;
	char respons[1024] = {0};
	char *start, *end;
	start = strstr(p, RESPONS_LABEL);
	end = strstr(p, ENDCOMMAND_LABEL);
	if(start && end && start > end ){
		p = end+strlen(ENDCOMMAND_LABEL);
		start = strstr(p, RESPONS_LABEL);
		end = strstr(p, ENDCOMMAND_LABEL);
	}
	while(start && end && p){
		memset(respons, 0, sizeof(respons));
		strncpy(respons, start, end-start);	
		chan_id = get_chan_id(respons);
		if(chan_id > 0){
//			at_result = get_at_result(respons);
			at_result = get_state_result(respons);
			pthread_mutex_lock(&module_check_info->socket_lock);
			if(at_result == 0){
//				printf("chan_%d recive ok\n", chan_id);
				module_check_info->chans[chan_id - 1].check_count = 0;	
			}else if(at_result == 1){
//				record_log("INFO","chan_%d recive fail\n", chan_id);
				record_log("INFO", "chan_%d init\n", chan_id);
				module_check_info->chans[chan_id - 1].check_count += 1;
			}else{
//				printf("chan_%d recive other\n", chan_id);
				module_check_info->chans[chan_id - 1].check_count = 0;	
			}
			pthread_mutex_unlock(&module_check_info->socket_lock);
		}
		p = end+strlen(ENDCOMMAND_LABEL);
		start = strstr(p, RESPONS_LABEL);
		end = strstr(p, ENDCOMMAND_LABEL);
	}

	if(start != NULL && end == NULL&& p){
		module_check_info->event_buf.len = strlen(start);
		strncpy(module_check_info->event_buf.buf, start, module_check_info->event_buf.len );
	}
}
/*
static int get_sim_state(struct module_check_info_s *module_check_info)
{
	if()
}
*/
static int send_at(int fd, int chan, char *at, int timeout)
{
	char ast_command[256]= {0};
	if(fd < 0)
		return 0;
//	sprintf(ast_command, "Action:Command\nCommand:gsm send sync at %d %s %d\nActionID:module-check-%d\nServer:127.0.0.1\n\n", chan, at, timeout, chan);
	sprintf(ast_command, "Action:Command\nCommand:gsm show span %d\nActionID:module-check-%d\nServer:127.0.0.1\n\n", chan, chan);
	return write(fd, ast_command, strlen(ast_command));
}

static int exec_system_cmd(char *cmd, char *result, int bufsize)
{
	int res = 0;
	FILE *handle = NULL;

	handle = popen(cmd, "r");
	if(handle == NULL){
		printf("exec %s fail\n", cmd);
		return -1;
	}
	memset(result, 0, bufsize);
	res = fread(result, 1, bufsize, handle);
	fclose(handle);

	return res;
}

static int log_init( struct module_check_info_s *p_info){
	log_handle = fopen(LOG_FILE, "w+");
	if(log_handle < 0)
		return -1;
	return 0;
}

static void log_deinit(struct module_check_info_s *p_info){
	if(log_handle)
		fclose(log_handle);
}


static int module_check_init_hwinfo(struct hw_info_s *hwinfo)
{
	char total_chan_str[32];
        
	exec_system_cmd("cat /tmp/hw_info.cfg |grep ^total_chan_count |awk -F '=' '{print $2}'", total_chan_str, sizeof(total_chan_str) );
	hwinfo->total_chan = atoi(total_chan_str);
	return 0;
}

static int module_check_init(struct module_check_info_s *module_check_info)
{
	int i = 0;
	struct chan_info_s* chan;

	log_init(module_check_info);
	module_check_init_hwinfo(&module_check_info->hwinfo);
	if(module_check_info->hwinfo.total_chan <= 0)
		return -1;
	
	module_check_info->chans = (struct chan_info_s *)malloc((sizeof(struct chan_info_s) * module_check_info->hwinfo.total_chan));
	if(module_check_info->chans == NULL)
		return -1;

	for(i = 0; i < module_check_info->hwinfo.total_chan; i++){
		chan = &module_check_info->chans[i];
		chan->chan_id = i+1;
		chan->check_count = 0;
		chan->sim_state = SIM_STATE_UNKOWN;
	}
	pthread_mutex_init(&module_check_info->socket_lock, NULL);
	return 0; 
}

static void module_check_deinit(struct module_check_info_s *module_check_info){
	if(module_check_info->chans)
		free(module_check_info->chans);

	log_deinit(module_check_info);
	module_check_info->chans = NULL;
}

static void * event_handler(void *data){
	struct pollfd eventfds;
	struct module_check_info_s *p_info = (struct module_check_info_s *)data;
	char buf[2048] = {0};
	int len = 0;
	int res;
	while(1){
		eventfds.fd = p_info->sockfd;
		eventfds.events |= POLLIN;
		eventfds.revents = 0;
		res = poll(&eventfds, 1, EVENT_TIMEOUTMS);
		if(res < 0) {
			fprintf(stderr, "poll(%d) error : %s\n",eventfds.fd, strerror(errno));
		}
		if(eventfds.revents & POLLIN) {
			if(p_info->event_buf.len > 0){
				strncpy(buf, p_info->event_buf.buf, p_info->event_buf.len);
				len = p_info->event_buf.len;
				p_info->event_buf.len = 0;
				memset(p_info->event_buf.buf, 0, 1024);
			}
			pthread_mutex_lock(&p_info->socket_lock);
			res = read(p_info->sockfd, buf+len, sizeof(buf) - len);
			pthread_mutex_unlock(&p_info->socket_lock);
			if(res == 0) {
				p_info->sockfd = event_sock_recreate();
			} else if (res > 0) {
				parsing_response(p_info, buf);
				parsing_event(p_info, buf);
				memset(buf, 0, sizeof(buf));
				len = 0;
			}
		}
	}
	return (void *)NULL;
}

int create_event_thread(void *data)
{
	int ret = -1;
	pthread_attr_t attr;
	struct module_check_info_s *p_info = (struct module_check_info_s *)data;
	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		return -1;
	}
	ret = pthread_create(&p_info->event_id, &attr, event_handler, data);
	if(ret != 0){
		printf("check fail\n");
		return -1;
	}
	return 0;
}

static int bsp_restart_module(int chan_id){
	char vbat_off_cmd[128] = {0};
	char vbat_on_cmd[128] = {0};
	char turn_on_cmd[128] = {0};
	sprintf(vbat_off_cmd, "/my_tools/bsp_cli module power off %d > /dev/null", chan_id);
	sprintf(vbat_on_cmd, "/my_tools/bsp_cli module power on %d > /dev/null", chan_id);
	sprintf(turn_on_cmd, "/my_tools/bsp_cli module turn on %d > /dev/null", chan_id);

	system(vbat_off_cmd);
	usleep(200000);//200ms
	system(vbat_on_cmd);
	usleep(200000);
	system(turn_on_cmd);
	record_log("INFO","restart module %d\n", chan_id);
}

void *detect_handler(void *data){
	int i = 0;
	struct module_check_info_s *p_info = (struct module_check_info_s *)data;
	while(1){
		for(i = 0; i < p_info->hwinfo.total_chan; i++){
			if(p_info->chans[i].check_count > 3){
				bsp_restart_module(i+1);	
				pthread_mutex_lock(&p_info->socket_lock);
				p_info->chans[i].check_count = 0;
				pthread_mutex_unlock(&p_info->socket_lock);
			}
		}
		
		sleep(60);
	}

	return (void *)NULL;
}

int create_reset_detect_thread(void *data){
	pthread_attr_t attr;
	int ret;
	struct module_check_info_s *p_info = (struct module_check_info_s *)data;
	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		return -1;
	}

	ret = pthread_create(&p_info->detect_id, &attr, detect_handler, data);
	if(ret != 0){
		record_log("ERROR", "check fail\n");
		return -1;
	}
	return 0;
}


int main(int argc, char **argv)
{
	int sockfd;
	int i = 0;
	module_check_init(&module_check_info);
	module_check_info.sockfd = event_sock_recreate();
	sockfd = module_check_info.sockfd;

	create_event_thread(&module_check_info);
	create_reset_detect_thread(&module_check_info);

	while(1) {
		for(i = 1; i <= module_check_info.hwinfo.total_chan; i++){
			if(module_check_info.chans[i-1].sim_state < SIM_STATE_INSERT )
				continue;
			//模块不是注册状态
			if(SIM_STATE_READY != module_check_info.chans[i-1].sim_state){
				pthread_mutex_lock(&module_check_info.socket_lock);
				send_at(module_check_info.sockfd, i , "AT", 2000);
				pthread_mutex_unlock(&module_check_info.socket_lock);
			}
		}
		sleep(60);
	}
	module_check_deinit(&module_check_info);
	return 0;
}

