#ifndef	__USSD_REDIS_IF_H__
#define	__USSD_REDIS_IF_H__

#include "ussd_inc.h"
#include "hiredis.h"

#define	MAX_TAG_LEN		16
#define	MAX_VALUE_LEN		128
#define	MAX_CONTENT_LEN	1024

#define USSD_FLAG_RANGE	0x100

typedef enum _ussd_id_e{
	USSD_ID_TYPE = 0,
	USSD_ID_MSG,
	USSD_ID_TIMEOUT,
	USSD_ID_ID,
	USSD_ID_MAX
}ussd_id_e;

typedef enum _ussd_res_e{
	USSD_RES_PORT = 0,
	USSD_RES_STATUS,		/*status id*/
	USSD_RES_CODING,
	USSD_RES_TEXT,
	USSD_RES_ID,
	USSD_RES_TYPE,
	USSD_RES_MAX
}ussd_res_e;

typedef struct _ussd_info_s{
	const char *tag;
	char value[MAX_VALUE_LEN];
}ussd_info_t;

typedef struct _ussd_res_info_s {
	int status;
	int coding;
	char content[MAX_CONTENT_LEN];
} ussd_res_info_t;

typedef struct _ussd_redis_info_s {
	int chan_id;
	redisContext *redis_c;
	ussd_info_t ussd_info[USSD_ID_MAX];
	ussd_res_info_t ussd_res;
	pthread_t tid;
//	pthread_mutex_t lock;
} ussd_redis_info_t;

typedef struct _ussd_config_s {
	int async_flag;
	int max_channel;
	
} ussd_config_t;


#endif

