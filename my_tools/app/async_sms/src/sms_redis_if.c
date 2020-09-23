#include "../include/sms_redis_if.h"
#include "sms_ami_if.h"
#include "log_debug.h"
#include <pthread.h>
#include <time.h>
#define	_MAX_CHANNEL_COUNT_		44

sms_redis_info_t sms_redis_info[_MAX_CHANNEL_COUNT_];
sms_config_t sms_config;

#define DEFAULT_REDIS_IP		"127.0.0.1"
#define DEFAULT_REDIS_PORT	6379
#define CHANNEL_STATUS_PATH     "/tmp/gsm"
#define _ASYNC_SMS_LIST_		"app.asterisk.async.smslist"
#define _SMS_FINISH_LIST_		"app.asterisk.smssend.list"
#define _BOARD_SPAN_			"local.product.board.span"

#define _redis_info_tag_type_		"type"
#define _redis_info_tag_num_		"num"
#define _redis_info_tag_msg_		"msg"
#define _redis_info_tag_flash_		"flash"
#define _redis_info_tag_id_		"id"
#define _redis_info_value_type_	"asyncsms"

sms_info_t g_sms_info[] = {
	{_redis_info_tag_type_,		_redis_info_value_type_},
	{_redis_info_tag_msg_,		""},
	{_redis_info_tag_num_,		""},
	{_redis_info_tag_flash_,	""},
	{_redis_info_tag_id_,		""},
	{"",		""}
};

static int g_total_chan_count = _MAX_CHANNEL_COUNT_;

static redisContext *ami_redis_cli = NULL; 

extern int send_sms(int chan_id, char *msg, char *num, int flashsms, char *id);
extern int send_csms(int chan_id, char *msg, char *num, int flashsms, char *id,
	unsigned int flag,int smscount,int smssequence);

extern int splite_sms(int chan_id, char *msg, char **sms);

static redisContext *conn_redis()
{
	redisContext *c = NULL;

	c = redisConnect(DEFAULT_REDIS_IP, DEFAULT_REDIS_PORT);
	if (c == NULL || c->err) {
		printf("goto redisConnect failed.\n");
		return NULL;
	}

	return c;
}

/*********************************************************
** function: replace sms out list \r\n symbol.
*********************************************************/
static char *trim_cr(char *src, char *dst)
{
	for(;'\0' != *src;src++)
	{
		if(*src == '\r'){
			*dst++ = '\\';
			*dst++ = 'r';
			continue;
		}
		else if(*src == '\n'){
			*dst++ = '\\';
			*dst++ = 'n';
		}else{
			*dst++ = *src;
		}
	}
	return dst;
}

int exec_pipe(const char *cmd, char *outbuf, int outlen)
{
	FILE *stream;
//	time_t stime;
	char *p = NULL;
	int rest = 0, len;

	if(cmd == NULL || cmd[0] == '\0'){
		return -1;
	}    

//	stime = time(NULL);

	if( (stream = popen(cmd, "r")) == NULL ) {
		return -1;
	}    

	if(outbuf != NULL && outlen > 0){
		p = outbuf;
		rest = outlen;
		len = fread(p,1,rest,stream);
		if(len < 0) {
			pclose(stream);
			return -1;
		}
	}
	pclose(stream);
	return 0;
}

const char *get_imsi(int port, char *imsi, int len)
{
	char cmd[128] = {0};
	char buf[64] = {0};
	if(!port)
		return "";
	sprintf(cmd, "cat %s/%d |grep \"SIM IMSI\"|awk '{print $3}'",CHANNEL_STATUS_PATH, port );
	if(exec_pipe(cmd, buf, sizeof(buf)) < 0)
		return "";
	strncpy(imsi, buf,len);
	return imsi;
}

static int finish_sms_to_outbox(sms_redis_info_t *ptr_info, int sms_res)
{
	char buf[1280];
	char msg[1280] = {0};
	char imsi_value[64];
	redisReply *reply = NULL;
	char *command = _SMS_FINISH_LIST_;
	char *num_tag = "number";
	char *num_value = NULL;
	char *chan_tag = "port";
	char *date_tag = "date";
	char date_value[64];
	char *content_tag = "content";
	char *content_value = NULL;
	char *status_tag = "status";
	char *uuid_tag = "uuid";
	char *uuid_value = NULL;
	char *imsi_tag = "imsi";
	int status_value;
	time_t now;
	struct tm *tm_now;

	if(ptr_info == NULL || ptr_info->redis_c == NULL)
		return -1;

	if (sms_res > 0)
		status_value = 1;
	else
		status_value = 0;
	num_value = ptr_info->sms_info[SMS_ID_NUM].value;
	trim_cr(ptr_info->sms_info[SMS_ID_MSG].value,msg);//adjust /my_tools/lua/sms_receive/sms_send
	content_value = msg;
	time(&now);
	tm_now = localtime(&now);
	uuid_value = ptr_info->sms_info[SMS_ID_ID].value;
	get_imsi(ptr_info->chan_id, imsi_value, sizeof(imsi_value));
	strftime(date_value, sizeof(date_value), "%Y-%m-%d %H:%M:%S", tm_now);
	snprintf(buf, sizeof(buf), "{\"%s\":\"%s\",\"%s\":\"%d\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%d\",\"%s\":\"%s\",\"%s\":\"%s\"}", num_tag, num_value, chan_tag, ptr_info->chan_id, date_tag, \
			date_value, content_tag, content_value, status_tag, status_value, uuid_tag, uuid_value, imsi_tag, imsi_value);
	log_print(INFO, "insert to outbox:%s\n",buf);
	reply = redisCommand(ptr_info->redis_c, "rpush %s %s", command, buf);
	if ( reply != NULL )
		freeReplyObject(reply);
//	printf("[%d] sms to outbox : [%s] \n", ptr_info->chan_id, date_value);

	return 0;
}

int insert_sms_to_outbox(int chan_id, char *msg, int status, char *uuid, char *imsi, char *phonenumber)
{
	char *num_tag = "number";
	char *chan_tag = "port";
	char *date_tag = "date";
	char *content_tag = "content";
	char *status_tag = "status";
	char *uuid_tag = "uuid";
	char *imsi_tag = "imsi";
	char *command = _SMS_FINISH_LIST_;
	char date_value[64];
	redisReply *reply = NULL;
	char buf[1280];
	struct tm *tm_now;
	time_t now;
	time(&now);
	tm_now = localtime(&now);
	strftime(date_value, sizeof(date_value), "%Y-%m-%d %H:%M:%S", tm_now);
	snprintf(buf, sizeof(buf), "{\"%s\":\"%s\",\"%s\":\"%d\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%d\",\"%s\":\"%s\",\"%s\":\"%s\"}", num_tag, phonenumber, chan_tag, chan_id, date_tag, \
			date_value, content_tag, msg, status_tag, status, uuid_tag, uuid, imsi_tag, imsi);
	log_print(INFO, "update [%d] to outbox:%s\n", chan_id, buf);
	reply = redisCommand(ami_redis_cli, "rpush %s %s", command, buf);
	if ( reply != NULL )
		freeReplyObject(reply);
	return 0;
}

inline int filter_sms(char *sms_src,char *sms_dst,int len)
{
	int i,j=0;
	
	for(i=0; i<len; i++) 
	{
		switch(sms_src[i])
		{	
			case '\'':
				sms_dst[j++] = '\\';
				sms_dst[j++] = '\'';
				break;
			case '"':
				sms_dst[j++] = '\\';
				sms_dst[j++] = '"';
				break;
			case '\\':
				sms_dst[j++] = '\\';
				sms_dst[j++] = '\\';
				break;
			case '`':
				sms_dst[j++] = '\\';
				sms_dst[j++] = '`';
				break;
			case '$':
				sms_dst[j++] = '\\';
				sms_dst[j++] = '$';
				break;
			default:
				sms_dst[j++]=sms_src[i];
				break;
		}
	}
	sms_dst[j] = '\0';
	return j;
}

/*********************************************************
** function: remove " symbol.
*********************************************************/
static char *trim_quote(char *src, char *dst)
{
	int len;
	int tail;
	int head = 0;

	if (src == NULL || dst == NULL)
		return NULL;

	len = strlen(src);
	tail = len;
	if (src[0] == '"')
		head = 1;
	if (src[len-1] == '"')
		tail -= 1;
	len = tail - head;
	strncpy(dst, &src[head], len);
	dst[len] = '\0';

	return dst;
}

/*********************************************************
** function: get tag and value
** "type":"asyncsms"
** token is :
** tag is type
** value is asyncsms
*********************************************************/
static char *get_redis_tv(const char *buf, char token, char *tag, char *value)		// token is =
{
	int opt = 0;
	char *find = NULL;
	char *left = NULL;
	char *right = NULL;
	char *ptr = (char *)buf;
	char key_str[4] = {'"',token,'"',0};
	
	find = strstr(ptr, key_str);
	if (find) {
		opt = find - ptr;
		right = find + 2;
		*(find+1) = '\0';
		left = ptr;
		if (tag && left && opt < MAX_TAG_LEN) {
			trim_quote(left, tag);
//			printf("tag=[%s]", tag);
		}
		if (value && right && strlen(right) < MAX_VALUE_LEN) {
			trim_quote(right, value);
//			printf("value=[%s]\n", value);
		}
	}

	return right;
}

/*********************************************************
** function: get tag and value
** "type\":\"asyncsms\",\"send_num\":\"13412345678\",\"context\":\"helloworld-0\"
** info_token is ,
** tv_token is :
** tag is type
** value is asyncsms
*********************************************************/
char* get_redis_info_token(const char *start, char info_token, char tv_token, char *tag, char *value)
{
	int  opt = 0;
	char *find = NULL;
	char *left = NULL;
	char *right = NULL;
	char *ptr = (char *)start;
	char key_str[4] = {'"',info_token,'"',0};
	
	if(start == NULL)
		return NULL;
	opt = strlen(start);
	if(start[opt-1] == '}')
		ptr[opt-1] = '\0';
	if(start[0] == '{')
		ptr += 1;

	find = strstr(ptr,key_str);
	if(find) {
		opt = find - ptr;
		right = find + 2;
		*(find+1) = '\0';
		left = ptr;
		get_redis_tv((const char *)left, tv_token, tag, value);
	} else {
		//This is last info
		get_redis_tv((const char *)ptr, tv_token, tag, value);
	}

	return right;
}


static int get_redis_sms_info(sms_redis_info_t *ptr_info)
{
	int ret = -1;
	int i;
	redisReply *reply = NULL;
	char *command = _ASYNC_SMS_LIST_;
	char *ptr = NULL;
	char left[MAX_TAG_LEN];
	char right[MAX_VALUE_LEN];


	if(ptr_info->redis_c == NULL)
		return -1;

//	printf("blpop %s.%d 0\n", command, ptr_info->chan_id);
	reply = redisCommand(ptr_info->redis_c, "blpop %s.%d 0", command, ptr_info->chan_id);
//	reply = redisCommand(ptr_info->redis_c, "lpop %s.%d", command, ptr_info->chan_id);
	if (NULL == reply) {
		printf("[%d] goto redisCommand failed. reply is NULL\n", ptr_info->chan_id);
		return -1;
	}
//	printf("reply type=%d, integer=%lld, len=%d, elements=%d\n", reply->type, reply->integer, reply->len, reply->elements);

	if (REDIS_REPLY_ARRAY == reply->type && 2 == reply->elements ) {
		if (reply->element[1] != NULL && reply->element[1]->str != NULL) {
//			printf("[%d] goto redisCommand succ. str=[%s]\n", ptr_info->chan_id, reply->element[1]->str);
			ptr = reply->element[1]->str;
			while (ptr) {
				left[0] = '\0';
				right[0] = '\0';
				ptr = get_redis_info_token((const char *)ptr, ',', ':', left, right);
				if (strlen(left) == 0)
					break;
				for (i = 0; i < SMS_ID_MAX; i++) {
					if (strcmp(left, g_sms_info[i].tag) == 0) {
						strncpy(ptr_info->sms_info[i].value, right, MAX_VALUE_LEN);
					}
				}
			}
			ret = 0;
		}
	}

	freeReplyObject(reply);

	return ret;
}

static int redis_check_sms(sms_redis_info_t *ptr_info)
{
	int ret = 0,i = 0,res = 0;
	int flashsms;
	char sms[20][320] ;
	char msg[320];
	char *p[20];
	unsigned int flag = 0;

	for(i = 0;i < 20;i++){
		p[i] = sms[i];
	}

	if (strcmp(ptr_info->sms_info[SMS_ID_TYPE].value, g_sms_info[SMS_ID_TYPE].value) == 0) {
		ret = splite_sms( ptr_info->chan_id,ptr_info->sms_info[SMS_ID_MSG].value,p);
		flashsms = atoi(ptr_info->sms_info[SMS_ID_FLASH].value);

		if(1 == ret){
			filter_sms(ptr_info->sms_info[SMS_ID_MSG].value,msg,strlen(ptr_info->sms_info[SMS_ID_MSG].value));
			res = send_sms(ptr_info->chan_id, msg, ptr_info->sms_info[SMS_ID_NUM].value,\
				flashsms, ptr_info->sms_info[SMS_ID_ID].value);
			sleep(1);
		}else{
			srand(time(NULL));
			flag = rand()%SMS_FLAG_RANGE;
			
			for(i = 0;i < ret;i++){
				filter_sms(sms[i],msg,strlen(sms[i]));
				res += send_csms(ptr_info->chan_id, msg, ptr_info->sms_info[SMS_ID_NUM].value, \
					flashsms, ptr_info->sms_info[SMS_ID_ID].value,flag,ret,i + 1);				
				sleep(1);
			}
		}
		if(res < 0){
			insert_to_fail_list(ptr_info->chan_id , ptr_info->sms_info[SMS_ID_ID].value, ptr_info->sms_info[SMS_ID_MSG].value, ptr_info->sms_info[SMS_ID_NUM].value);
		}else{
			ret = (res == ret? 1:0);
			finish_sms_to_outbox(ptr_info, ret);
		}
	}

	return ret;
}

static void *sms_redis_pthread_cb_handle(void *data)
{
	int res; 
	sms_redis_info_t *ptr_info = (sms_redis_info_t *)data;

	if (data == NULL) {
		log_print(ERROR, "[%d] callback func data is NULL.\n", ptr_info->chan_id);
		return NULL;
	}
	ptr_info->redis_c = conn_redis();

	while(ptr_info->redis_c != 	NULL) {
		res = get_redis_sms_info(ptr_info);
		if (!res) {
			redis_check_sms(ptr_info);
		}

		usleep(20*1000);
	}

	if(ptr_info->redis_c != NULL)
		redisFree(ptr_info->redis_c);
	ptr_info->redis_c = NULL;

	return NULL;
}



void create_redis_pthread()
{
	int i;
	int ret;
	sms_redis_info_t *ptr_info = NULL;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 100);
	if(0 != ret){
		printf("[%d] goto pthread_attr_setstacksize failed.\n", ptr_info->chan_id);
		return ;
	}

	for (i = 0; i < g_total_chan_count; i++)
	{
		ptr_info = &sms_redis_info[i];
		do {
			ret = pthread_create(&ptr_info->tid, &attr, sms_redis_pthread_cb_handle, ptr_info);
			if(ret != 0)
				printf("[%d] goto pthread_create failed.\n", ptr_info->chan_id);
			usleep(20*1000);
		} while(ret != 0);
		log_print(INFO, "[%d] pthread_create succ. ret=%d\n", ptr_info->chan_id, ret);
	}
}

static int get_total_channel()
{
	int total_channel = 0;
	redisContext *redis_c = NULL;
	redisReply *reply = NULL;
	char *command = _BOARD_SPAN_;

	redis_c = conn_redis();
	if (redis_c == NULL)
		return 0;

	reply = redisCommand(redis_c, "GET %s", command);
	if (NULL == reply) {
		printf("Goto redisCommand failed. reply is NULL\n");
		redisFree(redis_c);
		return 0;
	}
	if (reply->type != REDIS_REPLY_ERROR && reply->str != NULL) {
		total_channel = atoi(reply->str);
		if (total_channel > 0 && total_channel <= _MAX_CHANNEL_COUNT_)
			g_total_chan_count = total_channel;
	}
	freeReplyObject((void *)reply);
	redisFree(redis_c);

	return total_channel;
}

static void sms_redis_info_init()
{
	int i;
	sms_redis_info_t *ptr_info = NULL;

	memset(&sms_config, 0, sizeof(sms_config_t));
	for (i = 0; i < _MAX_CHANNEL_COUNT_; i ++)
	{
		ptr_info = &sms_redis_info[i];
		ptr_info->chan_id = i+1;
		ptr_info->redis_c = NULL;
	}
}

void sms_redis_init()
{
	get_total_channel();
	sms_redis_info_init();
	create_redis_pthread();
}

void init_ami_redis_cli()
{
	ami_redis_cli = conn_redis();
}
