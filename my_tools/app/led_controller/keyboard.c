#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>

#include "keyboard.h"
#include "queue.h"
#include "ledhdl.h"
#include "menudisplay.h"

keyboard_t g_key;


int put_keyboard_event(int key_event)
{
	queue_item_t item;
	keyboard_t * key = &g_key;
	int res = 0;
	
	if(!key) {
		key_print_error("key input error.\n");
		return -1;
	}
	
	if(!key->equeue) {
		key_print_error("equeue input error.\n");
		return -1;		
	}

	item.event = key_event;
	res = queue_put(key->equeue, item);
//	key_print_debug("put keyboard event: %d\n", key_event);	
	return res;
}


int get_keyboard_event(void)
{
	queue_item_t *pitem = NULL;
	keyboard_t * key =  &g_key;
	
	if(!key) {
		key_print_error("input error.\n");
		return -1;
	}

	if(!key->equeue) {
		key_print_error("equeue input error.\n");
		return -1;		
	}

	pitem = queue_get(key->equeue);
	if(!pitem) {
		return -1;
	}

	return pitem->event;
}


int keyboard_init(int confd)
{
	keyboard_t * key = &g_key;
	
	if(!key) {
		key_print_error("input error.\n");
		return -1;
	}

	key->equeue = queue_create(KEY_QUEUESIZE);
	key->key_read_fd = confd;
	key->nokey_time = 0;
	return 0;
}


void *check_keyboard_thread_cb_handler(void * data)
{
	keyboard_t * key = data;
	int res = 0;
	char revbuff[KEY_READ_LEN] = {0};
	struct pollfd keyfds;


	if(!key) {
		key_print_error("input error.\n");
		return NULL;
	}

	while(1) 
	{	
		keyfds.fd = key->key_read_fd;
		keyfds.events |= POLLIN;
		keyfds.revents = 0;
		res = poll(&keyfds, 1, 1000);

		if(keyfds.revents & POLLIN) {
			memset(revbuff, 0, KEY_READ_LEN);
			res = com_read(key->key_read_fd, revbuff, KEY_READ_LEN);
			if(res > 0)
			{
				if(!strcmp(revbuff, KEY_UP_INFO)) {
					put_keyboard_event(KEY_UP);	
				} else if(!strcmp(revbuff, KEY_OK_INFO)) {
					put_keyboard_event(KEY_OK);	
				} else if(!strcmp(revbuff, KEY_DOWN_INFO)) {
					put_keyboard_event(KEY_DOWN);
				}
			}
		}
	}
	return NULL;
}

int check_keyboard_create(int confd)
{
	pthread_t key_tid;

	keyboard_init(confd);
	led_pthread_create(&key_tid, NULL, check_keyboard_thread_cb_handler, &g_key);

	return 0;	
}
























