#ifndef _QUEUE_H_
#define  _QUEUE_H_

#include <sys/types.h>


typedef struct queue_item {
	int event;
} queue_item_t;


// the queue that save vapi msg.
typedef struct queue {
	size_t                  queue_size;
	queue_item_t           *last;
//	volatile queue_item_t  *head;
//	volatile queue_item_t  *tail;
//	volatile unsigned int   n_items;
	queue_item_t  *head;
	queue_item_t  *tail;
	unsigned int	n_items;	
	queue_item_t            buf[];
} queue_t;



extern queue_t *queue_create(size_t queue_size);
extern void queue_destroy(queue_t *queue);
extern int queue_put(queue_t *queue, queue_item_t item);
extern queue_item_t *queue_get(queue_t *queue);
extern void queue_flush(queue_t *queue);



#endif


