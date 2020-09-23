#ifndef	__SMS_REDIS_IF_H__
#define	__SMS_REDIS_IF_H__

#include "sms_inc.h"
#include "hiredis.h"

#define	MAX_TAG_LEN		16
#define	MAX_VALUE_LEN		1024

#define SMS_FLAG_RANGE	0x100

typedef enum _sms_id_e{
	SMS_ID_TYPE = 0,
	SMS_ID_MSG,
	SMS_ID_NUM,
	SMS_ID_FLASH,
	SMS_ID_ID,
	SMS_ID_MAX
}sms_id_e;

typedef struct _sms_info_s{
	const char *tag;
	char value[MAX_VALUE_LEN];
}sms_info_t;

typedef struct _sms_redis_info_s {
	int chan_id;
	redisContext *redis_c;
//	char sms_msg[_MAX_MSG_LEN_];
//	char sms_dest[_MAX_PHONE_LEN_];
	sms_info_t sms_info[SMS_ID_MAX];
	pthread_t tid;
} sms_redis_info_t;

typedef struct _sms_config_s {
	int async_flag;
	int max_channel;
	
} sms_config_t;

int insert_sms_to_outbox(int chan_id, char *msg, int status, char *uuid, char *imsi, char *phonenumber);
void init_ami_redis_cli();
#endif

