/***********************************************
FUNCTION: Update system time form The Base Station
***********************************************/
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "hiredis.h"
#define MAX_CHANNEL_COUNT (44)

#define DEFAULT_REDIS_IP		"127.0.0.1"
#define DEFAULT_REDIS_PORT	6379
#define CMD_GSMREG_STAT		"app.asterisk.simstatus.channel"

#define MAX_AST_CMD_LEN     2048
#define MAX_AST_RES_LEN     256
#define AT_CMD_GET_TIME     "AT+QLTS=2"
#define AST_AT_CMD              "asterisk -rx \"gsm send sync at %d \\\"%s\\\" %d\"" 
#define AST_CHECK_AT_RES        "+QLTS:"
#define AST_PIDOF_CMD           "pidof asterisk"

static int my_exec(const char *cmd, char *result)
{
	int res = 0;
	FILE *stream = NULL;

	if (cmd == NULL)
		return -1;
	stream = popen(cmd, "r");
	if(stream == NULL) {
		printf("popen %s failed. \n", cmd);
		return -1;
	}
	if (result != NULL) {
		res = fread(result, 1, MAX_AST_RES_LEN-1, stream);
		if (res > 0)	// read success
			res = 0;
//		printf("goto fread, res=%d, str[%s]\n", res, result);
	}
	pclose(stream);

	return res;
}

int get_redis_gsmreg_status(unsigned char status[])
{
	int i;
	int ret = 0;
	int val = 0;
	int chanid = 0;
	redisReply *reply = NULL;
	char *command = CMD_GSMREG_STAT;
	char *ptr = NULL;
	redisContext *redis_c = NULL;

	redis_c = redisConnect(DEFAULT_REDIS_IP, DEFAULT_REDIS_PORT);
	if (redis_c == NULL || redis_c->err) {
		printf("goto redisConnect failed.\n");
		return -1;
	}
	if (status == NULL || redis_c == NULL)
		return -1;
	reply = redisCommand(redis_c, "hgetall %s", command);
	if (NULL == reply) {
		printf("Goto redisCommand failed. reply is NULL\n");
		return -1;
	}

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

int ast_send_at(int chan_id, char *nowstr)
{   
	char ast_cmd[MAX_AST_CMD_LEN];
	char res_buf[MAX_AST_RES_LEN];
	int ast_timeout = 1200;
	int count = 10;
	int ret = -1;
	int i;
	char *ptr;

	if (chan_id <= 0 || nowstr == NULL)
		return -1;
	snprintf(ast_cmd, sizeof(ast_cmd), AST_AT_CMD, chan_id, AT_CMD_GET_TIME, ast_timeout);
//	printf("[%d] send at [%s].\n", chan_id, ast_cmd);
	for (i = 0; i < count; i++) {
		res_buf[0] = '\0';
		ret = my_exec((const char *)ast_cmd, res_buf);
		if (ret == 0) {
//			printf("[%d] res=[%s].\n", chan_id, res_buf);
			if (strstr(res_buf, AST_CHECK_AT_RES) != NULL) {
				//finish send sync at.
				ptr = strstr(res_buf, AST_CHECK_AT_RES) + strlen(AST_CHECK_AT_RES);
				while (*ptr == ' ' || *ptr == '\"')		// delete space and "
					ptr++;
				strncpy(nowstr, ptr, 32);
				printf("[%d] send at to asterisk success \n", chan_id);
				return ret;
			}
		} else {
			break;
		}
		ast_timeout += 500;
		snprintf(ast_cmd, sizeof(ast_cmd), AST_AT_CMD, chan_id, AT_CMD_GET_TIME, ast_timeout);
		sleep(1);
//		printf("[%d] %d goto asterisk [%s].\n", chan_id, i, ast_cmd);
	}
	printf("[%d] send at to asterisk failed \n", chan_id);

	return -1;
}

int ast_is_stop()
{
	int res;
	char val[256];

	val[0] = '\0';
	res= my_exec(AST_PIDOF_CMD, val);
	if (res < 0)
		return 0;
	if (strlen(val) == 0) {	// asterisk is stop
		return 1;
	}

	return 0;
}

static int get_astat_time(struct tm *nowtime)
{
	int i;
	int res;
	char timestr[32];
	unsigned char reg_status[MAX_CHANNEL_COUNT];

	if (nowtime == NULL)
		return -1;
	if (ast_is_stop())
		return -1;
	memset(reg_status, 0, sizeof(unsigned char) * MAX_CHANNEL_COUNT);
	res = get_redis_gsmreg_status(reg_status);
	if (res < 0) {
		printf("get_astat_time failed. res=%d.\n", res);
		return res;
	}

	for(i = 0; i < res && i < MAX_CHANNEL_COUNT; i++) {
		if (0 == reg_status[i])
			continue;
		res = ast_send_at(i+1, timestr);
		if (0 == res) {
			if(timestr[0] == '\r')
				continue;
			strptime(timestr, "%Y/%m/%d,%H:%M:%S", nowtime);
			if(nowtime->tm_year < 0){ // 19/03/05,10:00:00 => 2019/03/05,10:00:00
				nowtime->tm_year += 2000;
			}
			return 0;
		}
	}

	return -1;
}

static int save_local_time(struct tm *nowtime)
{
	int ret=-1;
	struct timeval tv;
	if (nowtime == NULL)
		return -1;
	tv.tv_sec = mktime(nowtime);
	tv.tv_usec = 0;
	/* need root user. */
	ret=settimeofday(&tv, NULL);
	if(ret < 0){
		printf("set system time failed!\n");
	}

	return ret;
}

static void try_save_local_time(struct tm *nowtime)
{
	int res;
	struct tm *local_time;

	if (nowtime == NULL)
		return;
	save_local_time(nowtime);
}

int main(int argc, char **argv){
	struct tm now_time;
	if(get_astat_time(&now_time) < 0){
		printf("get station time failed.\n");
		return -1;
	}
	try_save_local_time(&now_time);
	return 0;
}
