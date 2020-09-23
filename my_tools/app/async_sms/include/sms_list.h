#ifndef SMS_LIST_H
#define SMS_LIST_H
#include <time.h>
struct sms_message{
	int chan_id;
	char message[1024];
	char phonenumber[32];
	char uuid[32]; 
	unsigned char status;
	time_t last_time;
};

struct sms_list{
	struct sms_message sms;
	struct sms_list *_next;
};

int sms_list_init(struct sms_list *p_list);

int sms_list_find(struct sms_list *p_list, int chan_id, char *uuid, struct sms_message *sms);

int sms_list_remove(struct sms_list *p_list, int chan_id, char *uuid);

int sms_list_insert(struct sms_list *p_list, struct sms_message *sms);

int sms_list_destroy(struct sms_list *p_list);

struct sms_list *sms_list_get_next(struct sms_list *p_list);
#endif
