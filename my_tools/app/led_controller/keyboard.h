#ifndef _KEYBOARD_H
#define _KEYBOARD_H



#define KEY_READ_LEN    20
#define KEY_QUEUESIZE   20

#define KEY_UP_INFO     "key3=0\n"
#define KEY_OK_INFO     "key2=0\n"
#define KEY_DOWN_INFO   "key1=0\n"



#define    key_print_error(format,...)  printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define    key_print_debug(format,...)	printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)


typedef enum {
	KEY_UP,
	KEY_OK,
	KEY_DOWN,
	KEY_UNKNOW,
}key_type_t;


typedef struct keybaord_s {
	void *equeue;
	int key_read_fd;
	int nokey_time;
}keyboard_t;


int get_keyboard_event(void);
int put_keyboard_event(int key_event);
int check_keyboard_create(int confd);


#endif

