#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/mms_debug.h"

static int log_level = 0;
static FILE *log_stream = NULL;
#define MMS_LOG_FILE "/tmp/log/mms.log"

static const char *get_log_level_str(int level){
	switch (level)
	{
		case LOG_ERROR:
			return "ERROR";
			break;
		case LOG_WARNING:
			return "WANING";
			break;
		case LOG_INFO:
			return "INFO";
			break;
		case LOG_DEBUG:
			return "DEBUG";
			break;
	}
	return "";
}

int mms_log_print(int line, const char *file, const char *func, int level, char *format, ...){
	if(level <= log_level ){
		va_list va_args;
		char buf[1024];
		va_start(va_args, format);
		vsnprintf(buf, 1024, format, va_args);
		if(log_stream){
			fprintf(log_stream, "[%s %s %d] %s : %s\n", file, func, line, get_log_level_str(level), buf);
			fflush(log_stream);
		}else
			printf("[%s %s %d] %s : %s\n", file, func, line, get_log_level_str(level), buf);
	}
	return 0;
}


void mms_log_init(int level){
	log_stream = fopen(MMS_LOG_FILE, "w+");
	log_level = level;
}

void mms_log_deinit(){
	fclose(log_stream);
}


