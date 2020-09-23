#include "../include/ussd_redis_if.h"
#include <pthread.h>
#include <time.h>

#define	_MAX_CHANNEL_COUNT_		44

ussd_redis_info_t ussd_redis_info[_MAX_CHANNEL_COUNT_];
ussd_config_t ussd_config;

#define DEFAULT_REDIS_IP		"127.0.0.1"
#define DEFAULT_REDIS_PORT	6379
#define _ASYNC_USSD_LIST_		"app.asterisk.async.ussdlist"
#define _BOARD_SPAN_			"local.product.board.span"

#define _redis_info_tag_type_		"type"
#define _redis_info_tag_msg_		"msg"
#define _redis_info_tag_timeout_	"timeout"
#define _redis_info_tag_id_		"id"
#define _redis_info_value_type_	"asyncussd"

ussd_info_t g_ussd_info[] = {
	{_redis_info_tag_type_,	_redis_info_value_type_},
	{_redis_info_tag_msg_,		""},
	{_redis_info_tag_timeout_,	""},
	{_redis_info_tag_id_,		""},
	{"",		""}
};

static int g_total_chan_count = _MAX_CHANNEL_COUNT_;

#define _USSD_FINISH_LIST_		"app.ussd.resulttohttp.list"
#define _redis_ussd_res_tag_port_		"port"
#define _redis_ussd_res_tag_status_	"status"
#define _redis_ussd_res_tag_coding_	"coding"
#define _redis_ussd_res_tag_content_	"content"
#define _redis_ussd_res_tag_id_		_redis_info_tag_id_
#define _redis_ussd_res_tag_type_		"type"
#define _redis_ussd_res_value_type_	"USSD-Result"

#define ast_ussd_res_status_pre		"responses:"
#define ast_ussd_res_coding_pre		"code:"
#define ast_ussd_res_content_pre		"Text:"

extern int send_ussd(int chan_id, char *msg, char *timeout, char *uuid, char *result);

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

/*
gsm send ussd 12 "*#301#"
1:Recive USSD on span 12,responses:0,code:15
Text:UNKNOWN APPLICATION
*/
static int decode_str_to_ussd_res(ussd_redis_info_t *ptr_info, const char *str)
{
	int status;
	int coding;
	char *tmp = NULL;
	char *value = NULL;
	char *content = NULL;

	if (ptr_info == NULL || str == NULL)
		return -1;

	// status
	tmp = strstr(str, ast_ussd_res_status_pre);
	if (tmp == NULL) {
		printf("[%d] str=[%s] is no included [%s]\n", ptr_info->chan_id, str, ast_ussd_res_status_pre);
		return -1;
	}
	value = tmp+strlen(ast_ussd_res_status_pre);
	if (value == NULL)
		return -1;

	tmp = strstr(str, ast_ussd_res_coding_pre);
	if (tmp == NULL) {
		printf("[%d] str=[%s] is no included [%s]\n", ptr_info->chan_id, str, ast_ussd_res_coding_pre);
		return -1;
	}

	content = strstr(str, ast_ussd_res_content_pre);
	if (content == NULL) {
		printf("[%d] str=[%s] is no included [%s].\n", ptr_info->chan_id, str, ast_ussd_res_content_pre);
		return -1;
	}

	*(tmp-1) = '\0';
	status = atoi(value);
//	printf("status = %d, [%s].\n", status, value);

	// coding
	value = tmp+strlen(ast_ussd_res_coding_pre);
	if (value == NULL)
		return -1;
	*(content-1) = '\0';
	coding = atoi(value);
//	printf("coding = %d, [%s].\n", coding, value);

	// content 
	value = content+strlen(ast_ussd_res_content_pre);
	if (value == NULL)
		return -1;
//	tmp = strchr(value, '\n');
	tmp = value + strlen(value) - 1;
	if (tmp != NULL) {
		if (*tmp != '\n')
			printf("warning [%d] decode text.\n", ptr_info->chan_id);
		*tmp = '\0';
	}
//	printf("content = [%s].\n", value);

	ptr_info->ussd_res.status = status;
	ptr_info->ussd_res.coding = coding;
	strcpy(ptr_info->ussd_res.content, value);
//	printf("[%d] decode str to ussd result success!\n", ptr_info->chan_id);
	
	return 0;
}

static int finish_ussd_to_http(ussd_redis_info_t *ptr_info)
{
	char buf[1280];
	char msg[MAX_CONTENT_LEN] = {0};
	redisReply *reply = NULL;
	char *command = _USSD_FINISH_LIST_;
	ussd_res_info_t *ptr_res = NULL;
	char *port_tag = _redis_ussd_res_tag_port_;
	char *status_tag = _redis_ussd_res_tag_status_;
	char *coding_tag = _redis_ussd_res_tag_coding_;
	char *content_tag = _redis_ussd_res_tag_content_;
	char *content_value = NULL;
	char *id_tag = _redis_ussd_res_tag_id_;
	char *id_value = NULL;	
	char *type_tag = _redis_ussd_res_tag_type_;
	char *type_value = _redis_ussd_res_value_type_;

	if(ptr_info == NULL || ptr_info->redis_c == NULL)
		return -1;
	ptr_res = &ptr_info->ussd_res;

	trim_cr(ptr_res->content, msg);
	content_value = msg;
	id_value = ptr_info->ussd_info[USSD_ID_ID].value;
	snprintf(buf, sizeof(buf), "{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", \
		port_tag, ptr_info->chan_id, status_tag, ptr_res->status, coding_tag, ptr_res->coding, content_tag, content_value, \
		id_tag, id_value, type_tag, type_value);

//	printf("[%d] finish_ussd_to_http rpush [%s] [%s] \n", ptr_info->chan_id, command, buf);
	reply = redisCommand(ptr_info->redis_c, "rpush %s %s", command, buf);
	if ( reply != NULL )
		freeReplyObject(reply);
//	printf("[%d] ussd to http : [%s] \n", ptr_info->chan_id, ptr_res->content);

	return 0;
}

inline int filter_ussd(char *ussd_src,char *ussd_dst,int len)
{
	int i,j=0;
	
	for(i=0; i<len; i++) 
	{
		switch(ussd_src[i])
		{	
			case '\'':
				ussd_dst[j++] = '\\';
				ussd_dst[j++] = '\'';
				break;
			case '"':
				ussd_dst[j++] = '\\';
				ussd_dst[j++] = '"';
				break;
			case '\\':
				ussd_dst[j++] = '\\';
				ussd_dst[j++] = '\\';
				break;
			case '`':
				ussd_dst[j++] = '\\';
				ussd_dst[j++] = '`';
				break;
			case '$':
				ussd_dst[j++] = '\\';
				ussd_dst[j++] = '$';
				break;
			default:
				ussd_dst[j++]=ussd_src[i];
				break;
		}
	}
	ussd_dst[j] = '\0';
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
** "type":"asyncussd"
** token is :
** tag is type
** value is asyncussd
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
//			printf("tag=[%s]\n", tag);
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
** "type\":\"asyncussd\",\"msg\":\"helloworld-0\"
** info_token is ,
** tv_token is :
** tag is type
** value is asyncussd
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


static int get_redis_ussd_info(ussd_redis_info_t *ptr_info)
{
	int ret = -1;
	int i;
	redisReply *reply = NULL;
	char *command = _ASYNC_USSD_LIST_;
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
				for (i = 0; i < USSD_ID_MAX; i++) {
					if (strcmp(left, g_ussd_info[i].tag) == 0) {
						strncpy(ptr_info->ussd_info[i].value, right, MAX_VALUE_LEN);
					}
				}
			}
			if (strcmp(ptr_info->ussd_info[USSD_ID_TYPE].value, g_ussd_info[USSD_ID_TYPE].value) == 0)
				ret = 0;
			else
				ret = -1;
		}
	}

	freeReplyObject(reply);
//	printf("[%d] finish get_redis_ussd_info %s. ret=%d\n", ptr_info->chan_id, ret?"failed":"succ", ret);

	return ret;
}

static int redis_check_ussd(ussd_redis_info_t *ptr_info)
{
	int ret = 0;
	int res = 0;
	char msg[256];
	char result[1024];

//	printf("[%d] enter redis_check_ussd succ. val=[%s]\n", ptr_info->chan_id, ptr_info->ussd_info[USSD_ID_TYPE].value);
	if (strcmp(ptr_info->ussd_info[USSD_ID_TYPE].value, g_ussd_info[USSD_ID_TYPE].value) == 0) {
		filter_ussd(ptr_info->ussd_info[USSD_ID_MSG].value,msg,strlen(ptr_info->ussd_info[USSD_ID_MSG].value));
//		printf("[%d] goto filter_ussd succ. val=[%s], msg=[%s]\n", ptr_info->chan_id, ptr_info->ussd_info[USSD_ID_MSG].value, msg);
		ret = send_ussd(ptr_info->chan_id, msg,  ptr_info->ussd_info[USSD_ID_TIMEOUT].value, ptr_info->ussd_info[USSD_ID_ID].value, result);
		sleep(1);
	}

	if (ret) {
		res = decode_str_to_ussd_res(ptr_info, (const char *)result);
		if (res == 0) {
			res |= finish_ussd_to_http(ptr_info);
		}
	}

	return ret;
}

static void *ussd_redis_pthread_cb_handle(void *data)
{
	int res; 
	ussd_redis_info_t *ptr_info = (ussd_redis_info_t *)data;

	if (data == NULL) {
		printf("[%d] callback func data is NULL.\n", ptr_info->chan_id);
		return NULL;
	}
	ptr_info->redis_c = conn_redis();

	while(ptr_info->redis_c != 	NULL) {
		res = get_redis_ussd_info(ptr_info);
		if (!res) {
			redis_check_ussd(ptr_info);
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
	ussd_redis_info_t *ptr_info = NULL;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 100);
	if(0 != ret){
		printf("[%d] goto pthread_attr_setstacksize failed.\n", ptr_info->chan_id);
		return ;
	}

	for (i = 0; i < g_total_chan_count; i++)
	{
		ptr_info = &ussd_redis_info[i];

		do {
			ret = pthread_create(&ptr_info->tid, &attr, ussd_redis_pthread_cb_handle, ptr_info);
			if(ret != 0)
				printf("[%d] goto pthread_create failed.\n", ptr_info->chan_id);
			usleep(20*1000);
		} while(ret != 0);
		printf("[%d] pthread_create succ. ret=%d\n", ptr_info->chan_id, ret);
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

static void ussd_redis_info_init()
{
	int i;
	ussd_redis_info_t *ptr_info = NULL;

	memset(&ussd_config, 0, sizeof(ussd_config_t));
	for (i = 0; i < _MAX_CHANNEL_COUNT_; i ++)
	{
		ptr_info = &ussd_redis_info[i];
		ptr_info->chan_id = i+1;
		ptr_info->redis_c = NULL;
	}
}

void ussd_redis_init()
{
	get_total_channel();
	ussd_redis_info_init();
	create_redis_pthread();
}

