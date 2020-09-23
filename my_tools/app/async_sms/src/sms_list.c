#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sms_list.h"

int sms_list_init(struct sms_list *p_list){
	if(p_list == NULL)
		return -1;
	p_list->_next = NULL;
	return 0;
}

int sms_list_find(struct sms_list *p_list, int chan_id, char *uuid, struct sms_message *sms){
	struct sms_list *prev = p_list;
	struct sms_list *temp = p_list->_next;
	while(temp){
		if(temp->sms.chan_id == chan_id && strcmp(temp->sms.uuid, uuid) == 0){
			memcpy(sms, &temp->sms, sizeof(struct sms_message));
			prev->_next = temp->_next;
			free(temp);
			break;
		}
		prev = temp;
		temp = temp->_next;
	}
	if(temp == NULL)
		return -1;
	return 0;
}

int sms_list_remove(struct sms_list *p_list, int chan_id, char *uuid){
	struct sms_list *temp = p_list->_next;
	while(temp){
		if(temp->sms.chan_id == temp->sms.chan_id && strcmp(temp->sms.uuid, uuid) == 0){
			p_list->_next = temp->_next;
			free(temp);
		}else{
			p_list = temp;
			temp = temp->_next;
		}
	}
	return 0;
}

int sms_list_insert(struct sms_list *p_list, struct sms_message *sms){
	if(p_list == NULL)
		return -1;
	struct sms_list *temp;
	struct sms_list *head = p_list;
	temp = (struct sms_list *)malloc(sizeof(struct sms_list));
	if(temp == NULL)
		return -1;
	memcpy(&temp->sms, sms, sizeof(struct sms_message));
	temp->_next = NULL;
	while(head->_next){
		head = head->_next;	
	}
	head->_next = temp;
	return 0;
}

int sms_list_destroy(struct sms_list *p_list){
	struct sms_list *head = p_list;
	struct sms_list *temp = p_list->_next;
	while(temp){
		head->_next = temp->_next;
		free(temp);
		temp = head->_next;
	}
	return 0;
}

struct sms_list *sms_list_get_next(struct sms_list *p_list){
	return p_list->_next;
}


