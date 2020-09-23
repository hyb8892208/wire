#ifndef __MMS_LOG_DEBUG_H
#define __MMS_LOG_DEBUG_H

typedef enum log_level_e{
	LOG_ERROR = 0,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
}log_level_t;

#define MMS_LOG_PRINT(level, format, args...) \
	mms_log_print(__LINE__, __FILE__, __func__, level, format,##args)
int mms_log_print(int line, const char *file,const char *func, int level, char *format, ...);

void mms_log_init(int level);

void mms_log_deinit();

#endif
