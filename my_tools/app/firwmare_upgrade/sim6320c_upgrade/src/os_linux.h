
#ifndef __OS_LINUX_H__
#define __OS_LINUX_H__

#include "platform_def.h"

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_ANDROID)

char   *itoa(   int   val,   char   *buf,   unsigned   radix   );

int openport(int *com_port);
int closeport(HANDLE com_fd);

int WriteABuffer(HANDLE file, unsigned char * lpBuf, int dwToWrite);
int ReadABuffer(HANDLE file, unsigned char * lpBuf, int dwToRead);

int det_ttyusb_device(int interface);

void show_log(const char *msg, ...);

void qdl_sleep(int millsec);

void qdl_pre_download(char *filename);

void qdl_post_download(void);
#endif
#endif  /*TARGET_OS_LINUX*/

