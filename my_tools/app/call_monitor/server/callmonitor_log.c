#include <stdarg.h>
#include "callmonitor_log.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

UINT32  g_iLogLevel      = 0;    // 日志等级
UINT32  g_iLogsw         = 0;    // 日志开关
UINT8   g_szLogFile[100] = {0};  // 带路径的日志文件名
FILE *log_handle = NULL;

//#define WRITELOGFILE(level, msg)  WriteLogFile(__FILE__, __FUNCTION__, __LINE__, level, msg)

/*
const static char *str_replace(char *path)
{
	int i;
	int len = strlen(path);
	for(i=0; i<len; i++) {
		if(path[i] == '/') {
			path[i] = '_';
		}
	}
	return path;
}

static int lock_file(const char* path)
{
	char temp_path[PATH_SIZE];
	char lock_path[PATH_SIZE];
	int fd;

	snprintf(temp_path, PATH_SIZE, "%s", path);
	snprintf(lock_path, PATH_SIZE, "/tmp/lock/%s.lock", str_replace(temp_path));

	fd = open(lock_path, O_WRONLY|O_CREAT);
	if(fd <= 0) {
		printf("%s Can't open %s\n",__FUNCTION__, lock_path);
		return -1;
	}

	//Lock
	flock(fd, LOCK_EX);

	return fd;
}

static int unlock_file(int fd)
{
	if(fd <= 0) {
		return -1;
	} else {
		//UnLock
		flock(fd,LOCK_UN);
		close(fd);
		return 0;
	}
}


static int is_file_exist(const char *file_path)
{
	if(NULL == file_path) {
		return -1;
	} 
	if(0 == access(file_path, F_OK)) {
		return 0;
	} 
	return -1;
}
*/
UINT8 *LogLevel(UINT32 iLogLevel)
{
    switch (iLogLevel)
    {
        case LOG_ERROR:   
        {
            return "ERROR";
        }
 
        case LOG_WARN :
        {
            return "WARN";
        }
 
        case LOG_INFO :
        {
            return "INFO";
        }

        case LOG_DEBUG:   
        {
            return "DEBUG";
        }
 
        case LOG_ALL:   
        {
            return "ALL";
        }
 
        default: 
        {
            return "OTHER";
        }
    }
}
 

void GetTime(char  *pszTimeStr)
{
    struct tm      tSysTime     = {0};
    struct timeval tTimeVal     = {0};
    time_t         tCurrentTime = {0};
 
    UINT8  szUsec[20] = {0};    // 微秒
    UINT8  szMsec[20] = {0};    // 毫秒
 
    if (pszTimeStr == NULL)
    {
        return;
    }
 
    tCurrentTime = time(NULL);
    localtime_r(&tCurrentTime, &tSysTime);   // localtime_r是线程安全的
 
    gettimeofday(&tTimeVal, NULL);    
    sprintf(szUsec, "%06d", tTimeVal.tv_usec);  // 获取微秒
    strncpy(szMsec, szUsec, 3);                // 微秒的前3位为毫秒(1毫秒=1000微秒)
 
    sprintf(pszTimeStr, "[%04d.%02d.%02d %02d:%02d:%02d.%3.3s]", 
            tSysTime.tm_year+1900, tSysTime.tm_mon+1, tSysTime.tm_mday,
            tSysTime.tm_hour, tSysTime.tm_min, tSysTime.tm_sec, szMsec);
}


void set_log_default(int level)
{
	g_iLogLevel = level;
	g_iLogsw = 1;
	sprintf(g_szLogFile, "%s", LOG_FILE); 
	log_handle = fopen(g_szLogFile, "w+");
	return;
}

int log_init(int level)
{
	set_log_default(level);
	return 0; 
}

void _log_printf(UINT8 *pszFileName, UINT8 *pszFunctionName, UINT32 iCodeLine, UINT32 iLogLevel, char *format, ...)
{
	char time_str[MAX_LOG_LEN] = {0};
	char tmp[MAX_LOG_LEN] = {0};
	va_list argPtr;

	if(g_iLogsw == 0)
	{
		return;
	}
 
    if (iLogLevel > g_iLogLevel)
    {
	return;
    }
	
	va_start(argPtr, format);
	vsnprintf(tmp, MAX_LOG_LEN,format, argPtr);
	va_end(argPtr);
	GetTime(time_str);
	if(log_handle){
		fprintf(log_handle, "%s [%s]%s", time_str,LogLevel(iLogLevel), tmp);
		fflush(log_handle);
	}else{
		fprintf(stdout, "[%s]%s", LogLevel(iLogLevel), tmp);
		fflush(stdout);
	}
	
}








