
#ifndef _CALLLIMIT_LOG_H
#define _CALLLIMIT_LOG_H

// 重定义数据类型
typedef signed   int    INT32;
typedef unsigned int    UINT32;
typedef unsigned char   UINT8;
 
// 函数宏定义
#define MAX_LOG_LEN      1024
#define LOG_FILE          "/tmp/calllimit.log"
#define CFG_FILE_PATH    "/etc/asterisk/gw/call_limit"
#define LOG_CONF          "/etc/asterisk/gw/call_limit/log.conf"

#define PATH_SIZE 256
#define LINE_SIZE 128
#define NAME_SIZE 256

 
// 日志级别定义
typedef enum log_level_s {       
	LOG_ERROR = 0,              
	LOG_WARN = 1,
	LOG_INFO = 2,
	LOG_DEBUG = 3,
	LOG_ALL = 4,
}log_level_t;


#define log_printf(level, format,...) _log_printf(__FILE__, __FUNCTION__, __LINE__, level, format, ##__VA_ARGS__)
 
// 函数声明
int log_init(void);
void read_log_file(void);

#endif



