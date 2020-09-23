#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
//#include <zmq.h>
#include "czmq.h"
#include "debug.h"
int LogLevel = 4;
#define DEBUG_BUF_MAX (2048)
void error_printf(const char *filename, const char *func, int size, const char *format, ...){
	if(LogLevel >= 0){
		char print_info[DEBUG_BUF_MAX]  = {0};
		char tmp[DEBUG_BUF_MAX] = {0};
		va_list argPtr;
		
		va_start(argPtr, format);
		vsnprintf(tmp, DEBUG_BUF_MAX,format, argPtr);
		va_end(argPtr);
		
		snprintf(print_info, DEBUG_BUF_MAX ,"[%s:%s:%d] %s", filename, func, size, tmp);
		zsys_error(print_info);
	}

}

void warning_printf(const char *filename, const char *func, int size, const char *format, ...){
	if(LogLevel >= 1){
		char print_info[DEBUG_BUF_MAX] = {0};
		char tmp[DEBUG_BUF_MAX] = {0};
		va_list argPtr;
		
		va_start(argPtr, format);
		vsnprintf(tmp, DEBUG_BUF_MAX,format, argPtr);
		va_end(argPtr);
		
		snprintf(print_info, DEBUG_BUF_MAX ,"[%s:%s:%d] %s", filename, func, size, tmp);
		zsys_warning(print_info);
	}
}

void notice_printf(const char *filename, const char *func, int size, const char *format, ...){
	if(LogLevel >=2){
		char print_info[DEBUG_BUF_MAX] = {0};
		char tmp[DEBUG_BUF_MAX] = {0};
		va_list argPtr;
		
		va_start(argPtr, format);
		vsnprintf(tmp, DEBUG_BUF_MAX,format, argPtr);
		va_end(argPtr);
		
		snprintf(print_info, DEBUG_BUF_MAX ,"[%s:%s:%d] %s", filename, func, size, tmp);
		zsys_notice(print_info);
	}
}

void info_printf(const char *filename, const char *func, int size, const char *format, ...){
	if(LogLevel >=3){
		char print_info[DEBUG_BUF_MAX] = {0};
		char tmp[DEBUG_BUF_MAX] = {0};
		va_list argPtr;
		
		va_start(argPtr, format);
		vsnprintf(tmp, DEBUG_BUF_MAX,format, argPtr);
		va_end(argPtr);
		
		snprintf(print_info, DEBUG_BUF_MAX ,"[%s:%s:%d] %s", filename, func, size, tmp);
		zsys_info(print_info);
	}
}


void debug_printf(const char *filename, const char *func, int size, const char *format, ...){
	if(LogLevel >=4){
		char print_info[DEBUG_BUF_MAX] = {0};
		char tmp[DEBUG_BUF_MAX] = {0};
		va_list argPtr;
		
		va_start(argPtr, format);
		vsnprintf(tmp, DEBUG_BUF_MAX,format, argPtr);
		va_end(argPtr);
		
		snprintf(print_info, DEBUG_BUF_MAX ,"[%s:%s:%d] %s", filename, func, size, tmp);
		zsys_debug(print_info);
	}
}

