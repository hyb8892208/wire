#ifndef __MMS_FILE_H
#define __MMS_FILE_H
#include "mms_queue.h"
int load_mms_queue_conf( mms_queue_t *queue);

int flush_mms_queue_conf( mms_t *p_mms);

int load_mms_conf(mms_config_t *config);

void mms_config_deinit(mms_config_t *config);

int mms_config_init(mms_t *p_mms);

void mms_flush_file_deinit(void *data);

void *mms_flush_file_hander(void *data);

void mms_flush_file_init(void *data);
#endif
