
//#define QUEUE_TEST

#ifndef QUEUE_TEST

#include "mms.h"
#include "mms_queue.h"
#include "mms_debug.h"
#else
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#endif
#ifdef QUEUE_TEST
typedef enum _mms_id_e {
	MMS_ID_TYPE = 0,
	MMS_ID_PORT,
	MMS_ID_URL,
	MMS_ID_MAX
} mms_id_e;

typedef struct _mms_redis_tv_s{
	const char *tag;
	char value[128];
}mms_redis_tv_t;

typedef struct mms_queue_item{
	struct mms_queue_item *next;
	mms_redis_tv_t tv[MMS_ID_MAX];
	int try_count;
} mms_queue_item_t;


// the queue that save vapi msg.
typedef struct __mms_queue {
	mms_queue_item_t  *head;
	mms_queue_item_t  *tail;
	pthread_mutex_t queue_lock;
} mms_queue_t;

#endif

mms_queue_t *queue_init(mms_queue_t *queue)
{
	if(queue == NULL){
		queue = (mms_queue_t *)malloc(sizeof(mms_queue_t));
	}
	queue->head = queue->tail = NULL;
	pthread_mutex_init(&queue->queue_lock, NULL);
	return queue;
}

void queue_destroy(mms_queue_t *queue)
{
	mms_queue_item_t *item;
	if (!queue) {
		return;
	}
	if(queue->head != NULL){
		while(queue->head){
			item =queue->head ;
			queue->head = queue->head->next;
			free(item);
			item = NULL;
		}
	}
	pthread_mutex_destroy(&queue->queue_lock);
	free(queue);
}

int queue_put(mms_queue_t *queue, mms_queue_item_t *item)
{
	if (!queue) {
		MMS_LOG_PRINT(LOG_ERROR, "mms_queue head is null\n");
		return 1;
	}

	mms_queue_item_t *tmp = (mms_queue_item_t *)malloc(sizeof(mms_queue_item_t));

	memcpy(tmp, item, sizeof(mms_queue_item_t));
	tmp->next = NULL;
	if(queue->head == NULL){
		queue->head = tmp;
		queue->tail = tmp;
	}else{
		queue->tail->next = tmp;
		queue->tail = tmp;
	}
	return 0;
}
int queue_put_sort(mms_queue_t *queue, mms_queue_item_t *item)
{
	if (!queue) {
		return 1;
	} 
	mms_queue_item_t *tmp = (mms_queue_item_t *)malloc(sizeof(mms_queue_item_t));
	tmp->next = NULL;
	tmp->try_count = item->try_count;
	memcpy(tmp, item, sizeof(mms_queue_item_t));
	if(queue->head == NULL){
		queue->head = tmp; 
		queue->tail = tmp;
	}else{
		int cur_port ,item_port = atoi(item->tv[MMS_ID_PORT].value);
		mms_queue_item_t *head = queue->head;
		mms_queue_item_t *temp;
		for( ;head; head = head->next){
			if(head->next == NULL){//is tail
				queue->tail->next = tmp;
				queue->tail = tmp;
				break;
			}
			cur_port = atoi(head->tv[MMS_ID_PORT].value); 
			if(cur_port < item_port)
				continue;
			else if(cur_port > item_port)//port maybe is not sort
				continue;
			else{
				temp = head->next;
				if(atoi(temp->tv[MMS_ID_PORT].value) == item_port)
					continue;
				head->next = tmp;
				tmp->next = temp;
				break;
			}
		} 
	} 
	return 0;
}

int queue_pop(mms_queue_t *queue, mms_queue_item_t *item)
{
	mms_queue_item_t *pitem;

	if (!queue||!item) {
		MMS_LOG_PRINT(LOG_ERROR, "mms_queue is null, or item is null\n");
		return -1;
	}
	if(!queue->head){
		MMS_LOG_PRINT(LOG_DEBUG, "mms_queue head is null\n");
		return -1;
	}
	memcpy(item, queue->head, sizeof(mms_queue_item_t));
	pitem = queue->head;
	
	queue->head = queue->head->next;
	free(pitem);
	pitem = NULL; 
	return 0;
}

mms_queue_item_t *queue_get_next(mms_queue_t *queue, mms_queue_item_t *item)
{
	if(item == NULL)
		return queue->head;
	return item->next;
}
void queue_flush(mms_queue_t *queue)
{
	if (!queue) {
		return;
	}

	return;
}


mms_result_queue_t *mms_result_queue_init(mms_result_queue_t *queue){
	if(queue == NULL){
		queue = (mms_result_queue_t *)malloc(sizeof(mms_result_queue_t));
	}
	queue->head = queue->tail = NULL;
	return queue;
}

void mms_result_queue_destroy(mms_result_queue_t *queue){
	mms_result_queue_item_t *item;
	if (!queue) {
		return;
	}
	if(queue->head != NULL){
		while(queue->head){
			item =queue->head ;
			queue->head = queue->head->next;
			free(item);
			item = NULL;
		}
	}
	//pthread_mutex_destroy(&queue->queue_lock);
	free(queue);
}
int mms_result_queue_put(mms_result_queue_t *queue, mms_result_queue_item_t *item){
	if (!queue) {
		return 1;
	}

	mms_result_queue_item_t *tmp = (mms_result_queue_item_t *)malloc(sizeof(mms_result_queue_item_t));
	memcpy(tmp, item, sizeof(mms_result_queue_item_t));
	tmp->next = NULL;
	if(queue->head == NULL){
		queue->head = tmp;
		queue->tail = tmp;
	}else{
		queue->tail->next = tmp;
		queue->tail = tmp;
	}
	return 0;
}

int mms_result_queue_pop(mms_result_queue_t *queue, mms_result_queue_item_t *item){
	mms_result_queue_item_t *pitem;

	if (!queue||!item) {
		return -1;
	}
	if(!queue->head){
		MMS_LOG_PRINT(LOG_ERROR, "result_queue head is null\n");
		return -1;
	}
	memcpy(item, queue->head, sizeof(mms_result_queue_item_t));
	pitem = queue->head;
	
	queue->head = queue->head->next;
	free(pitem);
	pitem = NULL; 
	return 0;
}

#ifdef QUEUE_TEST
int load_config(const char *filename, mms_queue_t *queue)
{
	char *linebuf;
	size_t size;
	mms_queue_item_t item;
	char type[64];
	char port[64];
	char url[128];
	item.next = NULL;
	//memset();
	if(access(filename,F_OK) != 0){
		printf("file is not exist\n");
		return -1;
	}
	FILE *handle  = fopen(filename, "r");
	if(handle == NULL){
		printf("open file filed\n");
		return -1;
	}

	while (getline(&linebuf, &size, handle) > 0){
		memset(&item, 0, sizeof(mms_queue_item_t));
		sscanf(linebuf, "%s %s %s", item.tv[MMS_ID_TYPE].value, item.tv[MMS_ID_PORT].value, item.tv[MMS_ID_URL].value );
		//queue_put(queue, item);
		queue_put_sort(queue,item);
	}
	
	return 0;
}

int main(int argc, char**argv)
{
	mms_queue_t *queue = NULL;
	mms_queue_item_t item;
	 queue_init(queue);
	if(argc < 2){
		printf("usage:%s filename\n", argv[0]);
	}
	if(queue == NULL){
		printf("init queue failed\n");
		return -1;
	}
	load_config(argv[1], queue);
	while(queue_pop(queue, &item) >= 0){
		printf("type:%s,port:%s,url:%s\n", item.tv[MMS_ID_TYPE].value, item.tv[MMS_ID_PORT].value, item.tv[MMS_ID_URL].value);
	}
	queue_destroy(queue);
	return 0;
}

#endif


