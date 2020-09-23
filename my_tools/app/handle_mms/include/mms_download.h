#ifndef __MMS_DOWNLOAD_H_
#define __MMS_DOWNLOAD_H_

void *mms_create_dl_thread(void *data);

void mms_dl_deinit(void *data);

void mms_dl_init(void *data);

#endif

