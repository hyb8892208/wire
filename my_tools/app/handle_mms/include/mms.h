#ifndef	__MMS_H__
#define	__MMS_H__


#define   MAX_USERNAME_LEN 32
#define   MAX_PASSOWRD_LEN MAX_USERNAME_LEN
#define	MAX_URL_LEN		128
#define	MAX_SMS_LEN		2048

#define	_MAX_CHANNEL_COUNT_		44
#define	MAX_TAG_LEN		16
#define	MAX_VALUE_LEN		128
#define	MAX_CONTENT_LEN	1024
#define   MMS_MOREORY_DEAULT 4096

#define MMS_VER_STR              10
#define MMS_STR_DIM           10240

#include "mms_inc.h"
#include "hiredis.h"

typedef enum _mms_id_e {
	MMS_ID_TYPE = 0,
	MMS_ID_PORT,
	MMS_ID_URL,
	MMS_ID_MAX
} mms_id_e;

typedef struct _mms_redis_tv_s{
	const char *tag;
	char value[MAX_VALUE_LEN];
}mms_redis_tv_t;

typedef struct _mms_pthread_info_s {
	redisContext *redis_c;
	mms_redis_tv_t mms_tv[MMS_ID_MAX];
	pthread_t tid;
} mms_pthread_info_t;

typedef struct _mms_dialup_info_s{
	char apn[MAX_URL_LEN];
	char username[MAX_USERNAME_LEN];
	char password[MAX_PASSOWRD_LEN];
	char dns[MAX_VALUE_LEN];
	char device[MAX_VALUE_LEN];
}mms_dialup_info_t;

typedef enum _mms_download_type_e {
	MMS_DL_URL_OFF = 0,
	MMS_DL_URL_TIME,	/*use the time to download the url*/
	MMS_DL_URL_IMME,		/*immediately download the url*/
} mms_download_type_t;


typedef struct _mms_config_s {
	int save_url_flag;//
	int save_url_timeout;//url timeout
	int download_flag;//download flag
	int dl_timeout;//download time out
	mms_download_type_t dl_type;//download type. 0 means timer, 1 means without delay
	struct tm dl_time;
	mms_dialup_info_t dialup_info[_MAX_CHANNEL_COUNT_];
} mms_config_t;

typedef struct _mms_info_s {
	char dl_url[MAX_URL_LEN];//download url
	char long_sms[MAX_SMS_LEN];//long sms
	unsigned int save_flash_on:1;//save flag
	unsigned int save;//
} mms_info_t;

typedef struct _mms_memory_s{//use to store mms message from url 
       int size;//use size
	int total;//default size
       char *buf;//content
}mms_memory_t;

typedef struct {
    char *ctype;    /* content type */
    char *name;     /* content name */
    int size;       /* content size */
    char *path;     /* content file path */
} mms_part;

typedef struct {
    char version[MMS_VER_STR];           /* mms version */
    char *msg_type;                      /* message type string */
    char *cont_type;                     /* content type */
    char *from;                          /* from */
    char *to;                            /* to */
    char *cc;                            /* cc */
    char *bcc;                           /* bcc */
    short nparts;                        /* number of part */
    mms_part *part;                      /* parts */
    char *content;
    long sec;
} mms_message;

typedef struct mms_queue_item {
	mms_redis_tv_t tv[MMS_ID_MAX];//mms info
	struct mms_queue_item *next;//pointer to next mms_queue_item
	int try_count;
} mms_queue_item_t;

// the queue that save vapi msg.
typedef struct __mms_queue {
	mms_queue_item_t  *head;//pointer to head
	mms_queue_item_t  *tail;//pointer to tail
	pthread_mutex_t queue_lock;
} mms_queue_t;

typedef struct mms_result_buf_s{
	int port;
	char phonenumber[32];
	char time[32];
	char buf[MMS_MOREORY_DEAULT];
}mms_result_buf;

typedef struct mms_result_queue_item{
	mms_result_buf result_buf;
	int finish_flag;//finish flag
	struct mms_result_queue_item *next;
}mms_result_queue_item_t;

typedef struct _mms_result_queue{
	mms_result_queue_item_t  *head;
	mms_result_queue_item_t  *tail;
}mms_result_queue_t;


typedef struct _mms_dl_pthread_info_s{
	mms_memory_t        *mms_memory;
	mms_message          *mms_msg;
	mms_queue_t          *wait_queue;
	mms_queue_t          *fail_queue;
	pthread_t                  dl_thread_id;
	pthread_mutex_t       mms_lock;
	pthread_cond_t         mms_cond;
}mms_dl_pthread_info_t;

typedef struct _mms_result_pthread_info_s{
	mms_result_queue_t     *result_queue;
	redisContext              *redis_c;
	pthread_t                   result_thread_id;
	pthread_mutex_t        result_lock;
	pthread_cond_t          result_cond;
}mms_result_pthread_info_t;


typedef struct _mms_s{
	mms_pthread_info_t        mms_pthread;//redis pthread
	mms_dl_pthread_info_t    mms_dl_pthread;//download pthread
	mms_result_pthread_info_t  mms_result_pthread;//result pthread
	mms_config_t                *mms_config;//config
	pthread_t                        sava_thread_id;//save thread
	int                                   run_flag;
}mms_t;

#endif

