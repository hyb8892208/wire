#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "hiredis.h"

#define REDIS_KEY_SIMPHONENUM "app.simquery.phonenum.channel"
#define REDIS_KEY_BOARD_SPAN  "local.product.board.span"

#define DEFAULT_REDIS_IP		"127.0.0.1"
#define DEFAULT_REDIS_PORT	6379

static redisContext *c;

int conn_redis()
{
	int ret = 0;

	c = redisConnect(DEFAULT_REDIS_IP, DEFAULT_REDIS_PORT);
	if (c == NULL || c->err) {
		printf("goto redisConnect failed.\n");
		ret = -1;
	}

	return ret;
}

int get_phonenumber(int chan, char *phonenumber)
{
	int ret = -1;
	redisReply *reply = NULL;
	reply = redisCommand(c, "hget %s %d", REDIS_KEY_SIMPHONENUM, chan );
	if (reply && REDIS_REPLY_STRING == reply->type && reply->str ) {
		strcpy(phonenumber, reply->str);
		freeReplyObject(reply);
		ret = 0;
	}else{
		if(reply)
			freeReplyObject((void *)reply);
	}
	return ret;
}

void redis_disconnect(void)
{
	if(c)
		redisFree(c);
	c = NULL;
}

int get_total_channel()
{
	int total_channel = 0;
	int ret = -1;
	redisReply *reply = NULL;
	char *command = REDIS_KEY_BOARD_SPAN;

	ret = conn_redis();
	if (ret != 0){
		return 0;
	}

	reply = redisCommand(c, "GET %s", command);
	if (NULL == reply) {
		printf("Goto redisCommand failed. reply is NULL\n");
		redis_disconnect();
		return 0;
	}
	if (reply->type != REDIS_REPLY_ERROR && reply->str != NULL) {
		total_channel = atoi(reply->str);
	}
	freeReplyObject((void *)reply);
	redis_disconnect();

	return total_channel;
}

