#ifndef	__MMS_REDIS_IF_H__
#define	__MMS_REDIS_IF_H__

void mms_redis_init(void *data);

void mms_redis_deinit(void *data);

void create_redis_pthread(void *data);

void mms_create_result_thread(void *data);

void mms_result_init(void *data);

void mms_result_deinit(void *data);

#endif

