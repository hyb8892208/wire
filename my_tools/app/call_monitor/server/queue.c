#include "queue.h"

#include <malloc.h>


/* queue related functions */
queue_t *queue_create(size_t queue_size)
{
	queue_t *q = (queue_t *)malloc(sizeof(queue_t) + queue_size * sizeof(queue_item_t));
	if (!q)
		return NULL;

	q->queue_size = queue_size;
	q->last = q->buf + (queue_size - 1);
	q->n_items = 0;
	q->head = q->buf;
	q->tail = q->buf;

	return q;
}

void queue_destroy(queue_t *queue)
{
	if (!queue) {
		return;
	}
	free(queue);
}

int queue_put(queue_t *queue, queue_item_t item)
{
	if (!queue) {
		return 1;
	}

	if (queue->n_items >= queue->queue_size) {
		return 1;
	}
	*queue->tail = item;
	++(queue->n_items);
	if (queue->tail == queue->last) {
		queue->tail = queue->buf;
	} else {
		queue->tail++;
	}

	return 0;
}

queue_item_t *queue_get(queue_t *queue)
{
	queue_item_t *pitem;

	if (!queue) {
		return NULL;
	}

	if (queue->n_items <= 0) {
		return NULL;
	}
	pitem = (queue_item_t *) queue->head;
	--(queue->n_items);
	if (queue->head == queue->last) {
		queue->head = queue->buf;
	} else {
		(queue->head)++;
	}

	return pitem;
}

void queue_flush(queue_t *queue)
{
	if (!queue) {
		return;
	}

	queue->last = queue->buf + (queue->queue_size - 1);
	queue->n_items = 0;
	queue->head = queue->buf;
	queue->tail = queue->buf;
	return;
}





