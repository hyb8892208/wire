#include "../include/mms_redis_if.h"
#include "../include/mms.h"
#include "../include/mms_queue.h"
#include "../include/mms_debug.h"

#include <pthread.h>
#include <time.h>

#define AST_COMMOND_LEN 4096

#define DEFAULT_REDIS_IP		"127.0.0.1"
#define DEFAULT_REDIS_PORT	6379
#define RECEIVE_MMS_LIST		"app.asterisk.mmsnotify.list"
#define RESULT_MMS_LIST              "app.handle.smsrecv.list"
#define _redis_info_tag_type_	"type"
#define _redis_info_tag_port_		"port"
#define _redis_info_tag_url_		"url"
#define _redis_info_value_type_	"notification"

mms_redis_tv_t g_mms_redis[] = {
	{_redis_info_tag_type_,	_redis_info_value_type_},
	{_redis_info_tag_port_,		""},
	{_redis_info_tag_url_,		""},
	{"",		""}
};

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
char *trim_cr(char *src, char *dst)
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
** "type":"notification"
** token is :
** tag is type
** value is notification
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
		}
		if (value && right && strlen(right) < MAX_VALUE_LEN) {
			trim_quote(right, value);
		}
	}

	return right;
}

/*********************************************************
** function: get tag and value
** "type\":\"notification\",\"port\":\"1\"
** info_token is ,
** tv_token is :
** tag is type
** value is notification
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
		//This is lresult info
		get_redis_tv((const char *)ptr, tv_token, tag, value);
	}

	return right;
}


static int get_redis_mms_info(mms_pthread_info_t *ptr_info)
{
	int ret = -1;
	int i;
	redisReply *reply = NULL;
	char *command = RECEIVE_MMS_LIST;
	char *ptr = NULL;
	char left[MAX_TAG_LEN];
	char right[MAX_VALUE_LEN];


	if(ptr_info->redis_c == NULL)
		return -1;

	reply = redisCommand(ptr_info->redis_c, "blpop %s 0", command);
	if (NULL == reply) {
		MMS_LOG_PRINT(LOG_ERROR, "goto redisCommand failed. reply is NULL\n");
		return -1;
	}

	if (REDIS_REPLY_ARRAY == reply->type && 2 == reply->elements ) {
		if (reply->element[1] != NULL && reply->element[1]->str != NULL) {
			ptr = reply->element[1]->str;
			while (ptr) {
				left[0] = '\0';
				right[0] = '\0';
				ptr = get_redis_info_token((const char *)ptr, ',', ':', left, right);
				if (strlen(left) == 0)
					break;
				for (i = 0; i < MMS_ID_MAX; i++) {
					if (strcmp(left, g_mms_redis[i].tag) == 0) {
						strncpy(ptr_info->mms_tv[i].value, right, MAX_VALUE_LEN);
						MMS_LOG_PRINT(LOG_DEBUG,"tag=%s,value=%s\n", g_mms_redis[i].tag, ptr_info->mms_tv[i].value);
					}
				}
			}
			if (strcmp(ptr_info->mms_tv[MMS_ID_TYPE].value, g_mms_redis[MMS_ID_TYPE].value) == 0)
				ret = 0;
			else
				ret = -1;
		}
	}

	freeReplyObject(reply);
	MMS_LOG_PRINT(LOG_DEBUG,"finish get_redis_mms_info %s. ret=%d\n", ret?"failed":"succ", ret);

	return ret;
}

static void *mms_redis_pthread_cb_handle(void *data)
{
	int res; 
	int waikup_flag = 0;
	mms_t *ptr_mms = (mms_t *)data;
	mms_pthread_info_t *ptr_info =&ptr_mms->mms_pthread;
	mms_config_t *ptr_config = ptr_mms->mms_config;
	mms_queue_t *wait_queue = ptr_mms->mms_dl_pthread.wait_queue;
	int mmstv_size = sizeof(mms_redis_tv_t) * MMS_ID_MAX;
	mms_queue_item_t item;
	item.try_count = 0;
	int old_state;
	item.next = NULL;
	if (data == NULL) {
		MMS_LOG_PRINT(LOG_ERROR,"callback func data is NULL.\n");
		return NULL;
	}
	while(ptr_info->redis_c != 	NULL) {
		res = get_redis_mms_info(ptr_info);
		if (!res) {
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
			memcpy(item.tv, ptr_info->mms_tv, mmstv_size);
			if(ptr_config->dl_type == MMS_DL_URL_IMME){
				res = queue_put_sort(wait_queue, &item);
				if(res != 0){
					//put to log
				}else{
					pthread_cond_signal(&ptr_mms->mms_dl_pthread.mms_cond);
				}
			}else{
				res = queue_put(wait_queue, &item);
				if(res != 0){

				}
			}
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
		}
	
		usleep(20*1000);
		//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
	}

	if(ptr_info->redis_c != NULL)
		redisFree(ptr_info->redis_c);
	ptr_info->redis_c = NULL;

	return NULL;
}

void create_redis_pthread(void *data)
{
	int ret;
	mms_t *p_mms = (mms_t *)data;
	mms_pthread_info_t *ptr_info = NULL;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		MMS_LOG_PRINT(LOG_ERROR,"goto pthread_attr_setstacksize failed.\n");
		return ;
	}

	ptr_info = &p_mms->mms_pthread;
	ptr_info->redis_c = conn_redis();
	if (ptr_info->redis_c == NULL) {
		MMS_LOG_PRINT(LOG_ERROR,"goto conn_redis failed.\n");
		return ;
	}
	ret = pthread_create(&ptr_info->tid, &attr, mms_redis_pthread_cb_handle, data);
	if(ret != 0)
		MMS_LOG_PRINT(LOG_ERROR,"goto pthread_create failed.\n");
	else
		MMS_LOG_PRINT(LOG_DEBUG,"pthread_create succ. ret=%d\n", ret);
	
}

void mms_redis_init(void *data)
{
	create_redis_pthread(data);
}

void mms_redis_deinit(void *data){
	void *status;
	mms_t *p_mms = (mms_t *)data;
	mms_pthread_info_t *ptr_info = &p_mms->mms_pthread;
	pthread_cancel(ptr_info->tid);
	pthread_join(ptr_info->tid,&status);
}

static int modify_mms_text(char *sms_text,int len)
{
	char sms_tmp[MMS_MOREORY_DEAULT] = {0};
	int i,j=0;
	int max_len = MMS_MOREORY_DEAULT - 4;
	for(i=0; i < len && j < max_len; i++) 
	{
		switch(sms_text[i])
		{
			case '"':
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '"';
				break;
			case '\\':
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				break;
			case '`':
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '`';
				break;
			case '$':
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '\\';
				sms_tmp[j++] = '$';
				break;
			default:
				sms_tmp[j++]=sms_text[i];
				break;
		}
	}
	memcpy(sms_text,sms_tmp,j);
	return j;
}


/*******************result hander*******************************/
static void *mms_result_handler(void *data)
{
	mms_t *p_mms = (mms_t*)data;
	int old_state;
	mms_result_pthread_info_t *result_pthread_info = &p_mms->mms_result_pthread;
	mms_result_queue_t *result_queue = result_pthread_info->result_queue;
	mms_result_queue_item_t result_item;
	redisReply *reply = NULL;
	char buf[AST_COMMOND_LEN];
	const char *port_tag = "port";
	const char *sender_tag = "sender";
	const char *time_tag = "time";
	const char *msg_tag = "msg";
	pthread_mutex_lock(&result_pthread_info->result_lock);
	while(1){
		while(mms_result_queue_pop(result_queue, &result_item) == -1){
			pthread_cond_wait(&result_pthread_info->result_cond,&result_pthread_info->result_lock);
		}
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
		modify_mms_text(result_item.result_buf.buf, strlen(result_item.result_buf.buf));
		snprintf(buf, sizeof(buf), "{\"%s\":\"%d\","
			                             "\"%s\":\"%s\","
			                             "\"%s\":\"%s\","
			                             "\"%s\":\"%s\"}",
			                             port_tag, result_item.result_buf.port,
			                             sender_tag, result_item.result_buf.phonenumber,
			                             time_tag, result_item.result_buf.time,
			                             msg_tag, result_item.result_buf.buf);

		reply = redisCommand(result_pthread_info->redis_c, "rpush %s %s", RESULT_MMS_LIST, buf);
		if (NULL != reply) {
			freeReplyObject(reply);
		}
		MMS_LOG_PRINT(LOG_DEBUG, "rpush result to redis success\n");
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
	}
	pthread_mutex_unlock(&result_pthread_info->result_lock);
	return (void *)NULL;
}

void mms_create_result_thread(void *data)
{
	int ret;
	pthread_attr_t attr;
	mms_t *p_mms = (mms_t*)data;
	mms_result_pthread_info_t *ptr_info = &p_mms->mms_result_pthread;
	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		MMS_LOG_PRINT(LOG_ERROR,"goto pthread_attr_setstacksize failed.\n");
		return ;
	}
	
	ptr_info->redis_c = conn_redis();
	if (ptr_info->redis_c == NULL) {
		MMS_LOG_PRINT(LOG_ERROR,"goto conn_redis failed.\n");
		return ;
	}
	
	ret = pthread_create(&ptr_info->result_thread_id, &attr, mms_result_handler, data);
	if(ret != 0)
		MMS_LOG_PRINT(LOG_ERROR, "goto pthread_create failed.\n");
	else
		MMS_LOG_PRINT(LOG_DEBUG,"pthread_create succ. ret=%d\n", ret);
}

void mms_result_init(void *data)
{
	mms_t *p_mms = (mms_t*)data;
	mms_result_pthread_info_t *ptr_info = &p_mms->mms_result_pthread;
	pthread_mutex_init(&ptr_info->result_lock, NULL);
	pthread_cond_init(&ptr_info->result_cond, NULL);
	ptr_info->result_queue = NULL;
	ptr_info->result_queue = mms_result_queue_init(ptr_info->result_queue);
	
}

void mms_result_deinit(void *data)
{
	mms_t *p_mms = (mms_t*)data;
	void *status;
	mms_result_pthread_info_t *ptr_info = &p_mms->mms_result_pthread;
	pthread_cancel(ptr_info->result_thread_id);
	pthread_join(ptr_info->result_thread_id, &status);
	pthread_mutex_destroy(&ptr_info->result_lock);
	pthread_cond_destroy(&ptr_info->result_cond);
	mms_result_queue_destroy(ptr_info->result_queue);
}

