#include <pthread.h>
#include "redis_if.h"
#include "hiredis.h"


#define DEFAULT_REDIS_IP		"127.0.0.1"
#define DEFAULT_REDIS_PORT	6379

#define MAX_TAG_LEN			16
#define MAX_VALUE_LEN			1024
#define CMD_LICENSE_FLAG		"app.license.flag"
#define CMD_GSMREG_STAT		"app.asterisk.simstatus.channel"
#define CMD_LICENSE_ENDTIME	"app.license.endtime"
#define CMD_LICENSE_STATUS		"app.license.status"
#define REDIS_RESULS			"OK"

static redisContext *g_redis_c = NULL;
static redis_sap_t  redis_sap;

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

int get_redis_gsmreg_status(unsigned char status[])
{
	int i;
	int ret = 0;
	int val = 0;
	int chanid = 0;
	redisContext *redis_c = g_redis_c;
	redisReply *reply = NULL;
	char *command = CMD_GSMREG_STAT;
	char *ptr = NULL;

	if (status == NULL || redis_c == NULL)
		return -1;
	// todo
	reply = redisCommand(redis_c, "hgetall %s", command);
	if (NULL == reply) {
		printf("Goto redisCommand failed. reply is NULL\n");
		return -1;
	}

//	printf("get_redis_gsmreg_status : type=%d, elements=%d\n", reply->type, reply->elements);
	if (REDIS_REPLY_ARRAY == reply->type) {
		for (i = 0; i < reply->elements && i < MAX_CHANNEL_COUNT*2; i++) {
			if (NULL == reply->element[i])
				continue;
			ptr = reply->element[i]->str;
			if (NULL == ptr)
				continue;
			if (i%2 == 0) {
				val = atoi(ptr);
				if (val > 0)
					chanid = val -  1;
				else
					i += 1;
			} else {
				val = atoi(ptr);
				if (val == 0 || val == 1)
					status[chanid] = val ^ 1;
			}
		}
		ret = i/2;
	}
	freeReplyObject(reply);

	return ret;
}                      

static int get_redis_license_flag(redisContext *redis_c, int *res_license_flag)
{
	int ret = -1;
	int flag;
	redisReply *reply = NULL;
	char *command = CMD_LICENSE_FLAG;
	char *ptr = NULL;

	if(redis_c == NULL)
		return -1;

//	printf("blpop %s 0\n", command);
	reply = redisCommand(redis_c, "blpop %s 0", command);
	if (NULL == reply) {
		printf("Goto redisCommand failed. reply is NULL\n");
		return -1;
	}
//	printf("reply type=%d, integer=%lld, len=%d, elements=%d\n", reply->type, reply->integer, reply->len, reply->elements);

	if (REDIS_REPLY_ARRAY == reply->type && 2 == reply->elements ) {
		if (reply->element[1] != NULL && reply->element[1]->str != NULL) {
			printf("Goto redisCommand succ. str=[%s]\n", reply->element[1]->str);
			ptr = reply->element[1]->str;
			flag = atoi(ptr);
			if (res_license_flag != NULL)
				*res_license_flag = flag;
			ret = 0;
		}
	}

	freeReplyObject(reply);

	return ret;
}

int redis_set_license_endtime(char *endtime)
{
	int ret = -1;
	redisContext *redis_c = g_redis_c;
	redisReply *reply = NULL;
	char *command = CMD_LICENSE_ENDTIME;

	if (endtime == NULL || redis_c == NULL)
		return -1;
	reply = redisCommand(redis_c, "set %s %s", command, endtime);
	if (NULL == reply || NULL == reply->str) {
		printf("Goto redisCommand failed. reply is NULL\n");
		return -1;
	}
//	printf("reply str: [%s]\n", reply->str);
	if (strncmp(reply->str, REDIS_RESULS, 2) == 0)
		ret = 0;
	freeReplyObject(reply);

	return ret;
}

int redis_set_license_status(char *status)
{
	int ret = -1;
	redisContext *redis_c = g_redis_c;
	redisReply *reply = NULL;
	char *command = CMD_LICENSE_STATUS;

	if (status == NULL || redis_c == NULL)
		return -1;
	reply = redisCommand(redis_c, "set %s %s", command, status);
	if (NULL == reply || NULL == reply->str) {
		printf("Goto redisCommand failed. reply is NULL\n");
		return -1;
	}
//	printf("reply str: [%s]\n", reply->str);
	if (strncmp(reply->str, REDIS_RESULS, 2) == 0)
		ret = 0;
	freeReplyObject(reply);

	return ret;
}

static void *redis_pthread_cb_handle()
{
	int res;
	int license_flag = 0;
	redisContext *redis_c = NULL;

	redis_c = conn_redis();
	while(redis_c != 	NULL) {
		res = get_redis_license_flag(redis_c, &license_flag);
		if (!res) {
			printf("Get redis license.\n");
			// handle check license
			if (license_flag) {
				redis_sap.redis_check_license();
				license_flag = 0;
			}
		}

		sleep(10);
	}

	if(redis_c != NULL)
		redisFree(redis_c);

	return NULL;
}

static void create_redis_pthread()
{
	int ret;
	pthread_t tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		printf("Goto pthread_attr_setstacksize failed.\n");
		return ;
	}
	ret = pthread_create(&tid, &attr, redis_pthread_cb_handle, NULL);
	if(ret != 0) 
		printf("Goto pthread_create failed.\n");
	else
		printf("Goto pthread_create succ. ret=%d\n", ret);

	return;
}

void redis_init(redis_sap_t *psap)
{
	redisContext *redis_c = NULL;

	if (psap == NULL)
		return ;
	memcpy(&redis_sap, psap, sizeof(redis_sap_t));
	do {
		redis_c = conn_redis();
		if (redis_c != NULL)
			break;
	} while(1);
	g_redis_c = redis_c;

	create_redis_pthread();
}

