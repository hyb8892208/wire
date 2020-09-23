#ifndef LOG_DEBUG_H
#define LOG_DEBUG_H
#include <stdarg.h>
enum LOG_LEVEL{
	DEBUG=0,
	INFO,
	WARNING,
	ERROR
};
void log_init(void);
void log_deinit(void);

void log_print(int level, const char *format, ... );
#endif
