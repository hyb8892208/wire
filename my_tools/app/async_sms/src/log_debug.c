#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "log_debug.h"
#define LOG_FILE "/tmp/log/async_sms.log"

FILE *log_handle = NULL;
static const char *get_level_str(int level){
	switch(level){
		case INFO:
			return "INFO";
			break;
		case DEBUG:
			return "DEBUG";
			break;
		case WARNING:
			return "WARING";
			break;
		case ERROR:
			return "ERROR";
	}
	return "";
}

void log_init(void){
	log_handle = fopen(LOG_FILE, "w+");	
	if(log_handle == NULL)
		printf("init log fail\n");
}

static char *get_time_str(char *str_time){
	time_t t = time(NULL);
	strftime(str_time,24,"%Y-%m-%d %X",localtime(&t));
	return str_time;
}
void log_print(int level, const char *format, ...){
	char buf[2048] = {0};
	char *pbuf = buf;
	va_list va_args;
	char str_time[24] = {0};
	va_start(va_args, format);
	pbuf += sprintf(buf,"[%s] ", get_level_str(level));
	vsnprintf(pbuf, 2048, format, va_args);
	if(log_handle == NULL){
		fprintf(stdout, "%s %s", buf, get_time_str(str_time));
	}else{
		fprintf(log_handle, "%s %s", buf, get_time_str(str_time));
		fflush(log_handle);
	}
}
void log_deinit(void){
	fclose(log_handle);
}

