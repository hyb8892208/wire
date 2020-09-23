#ifndef __MMS_QUEUE_H_
#define __MMS_QUEUE_H_
#include "mms_redis_if.h"
#include "mms.h"
#include <sys/types.h>


//extern queue_t *queue_create(size_t queue_size);
extern mms_queue_t *queue_init(mms_queue_t *queue);
extern void queue_destroy(mms_queue_t *queue);
extern int queue_put(mms_queue_t *queue, mms_queue_item_t *item);
extern int queue_put_sort(mms_queue_t *queue, mms_queue_item_t *item);
extern int queue_pop(mms_queue_t *queue, mms_queue_item_t *item);
extern void queue_flush(mms_queue_t *queue);
extern mms_queue_item_t *queue_get_next(mms_queue_t *queue, mms_queue_item_t *item);

extern mms_result_queue_t *mms_result_queue_init(mms_result_queue_t *queue);
extern void mms_result_queue_destroy(mms_result_queue_t *queue);
extern int mms_result_queue_put(mms_result_queue_t *queue, mms_result_queue_item_t *item);
extern int mms_result_queue_pop(mms_result_queue_t *queue, mms_result_queue_item_t *item);
#endif


