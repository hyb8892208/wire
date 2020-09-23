#ifndef __Z_PRINT_H__
#define __Z_PRINT_H__


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include "debug.h"

enum LOG_CLASS
{
	INFO = 1,
	WARN = 2,
	ERROR = 4, 
};


typedef struct log_mon_dhl_param_s
{
	int stop;
	int intv;
	int file_size;
	char file_name[256];
}log_mon_dhl_param_t;

#define zprintf( flags,...)    z_printf(( flags ),(__FILE__),(__LINE__),__VA_ARGS__)


int openlogfile(char *filename);
int openhexlogfile(char *filename);

void z_printf(unsigned int flags,const char *file,const int line,const char *format, ...);

char * get_proc_name(pid_t pid, char *proc_name, int len);
//char *str_timestamp(time_t tt, char *tstr, int tstrlen);

void * logMonHdl(void *pParam);

int open_apdu_log_file(int bank_nbr,int slot_nbr);

int logDataToHex(int fd,unsigned char *buff, unsigned short buff_len,const char *dire);

#endif


