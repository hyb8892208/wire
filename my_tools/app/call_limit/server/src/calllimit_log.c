
#include "../include/header.h"
#include "../include/calllimit_log.h"


UINT32  g_iLogLevel      = 0;    // 日志等级
UINT32  g_iLogsw         = 0;    // 日志开关
UINT8   g_szLogFile[100] = {0};  // 带路径的日志文件名


//#define WRITELOGFILE(level, msg)  WriteLogFile(__FILE__, __FUNCTION__, __LINE__, level, msg)


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

static int get_option_value(const char * file_path, const char * context_name, const char * option_name, char * out_value)
{
	char buf[LINE_SIZE];
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	FILE* fp;
	char name[NAME_SIZE];
	int lock;

	lock=lock_file(file_path);

	if( NULL == (fp=fopen(file_path,"r")) ) {
		
		unlock_file(lock);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0== strcmp(name,context_name) ) {
						finded=1;
					} else {
						if(finded) 
							finish=1;
					}
				}
				break;
			case '=':
				if(finded && !finish) {
					memcpy(name,buf,i);
					name[i] = '\0';
					if(0==strcmp(name,option_name)) {
						memcpy(name,buf+i+1,len-i-1-1);
						name[len-i-1-1] = '\0';
						fclose(fp);
						unlock_file(lock);
						memcpy(out_value,name,len-i-1);
						return 0;
					}
				}
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	unlock_file(lock);
	return -1;
}

 
 
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
 
 
void GetTime(UINT8 *pszTimeStr)
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
 

static int create_filepath(const char * filepath)
{
	struct stat file_stat;
	int ret = -1;

	if(!filepath) {
		return -1;
	}

	ret = stat(filepath, &file_stat);
	if(ret < 0) {
		if(errno == ENOENT) {		 
			ret = mkdir(filepath, 0775);
			printf("creat dir %s\n", filepath);
			if(ret < 0) {
				printf("Could not create directory  %s\n",
					filepath);
				return -1;
			}
		} else {
			printf("bad file path\n");
			return -1;
		}
	}
	return 0;
}


void create_log_file()
{
	FILE *fp;

	create_filepath(CFG_FILE_PATH);
	
	if( NULL == (fp = fopen(LOG_CONF, "w+")) ) {
		printf("%s Can't create %s\n", __FUNCTION__, LOG_CONF);
		return -1;
	}

	fprintf(fp, "[general]\n");
	fprintf(fp, "switch=%s\n", "on");
	fprintf(fp, "level=%s\n", "error");
	fclose(fp);
	return 0;
}



void read_log_file(void)
{
	int res = -1;
	char tmp[256] = {0};

	if(!(res = get_option_value(LOG_CONF, "general", "switch", tmp))) {
	    if(!strcmp(tmp, "on")) {
			g_iLogsw = 1;
		}else if(!strcmp(tmp, "off")) {
			g_iLogsw = 0;
		}else {
			g_iLogsw = 0;
		}
//		 printf("%s g_iLogsw=%s\n", __FUNCTION__, g_iLogsw?"on":"off");
	} else {
	    printf("%s get logfile general switch fail.\n", __FUNCTION__);
	}

	if(!(res = get_option_value(LOG_CONF, "general", "level", tmp))) {
	    if(!strcmp(tmp, "all")) {
			g_iLogLevel = LOG_ALL;
		}else if(!strcmp(tmp, "debug")) {
			g_iLogLevel = LOG_DEBUG;
		}else if(!strcmp(tmp, "info")){
			g_iLogLevel = LOG_INFO;
		}else if(!strcmp(tmp, "warning")) {
			g_iLogLevel = LOG_WARN;
		}else if(!strcmp(tmp, "error")){
			g_iLogLevel = LOG_ERROR;
		}else {
			g_iLogLevel = LOG_ERROR;
		}
//		printf("%s g_iLogLevel=%s\n", __FUNCTION__, LogLevel(g_iLogLevel));
	} else {
	    printf("%s get logfile general switch level fail.\n", __FUNCTION__);
	}	
}
 
void GetConfigValue(void)
{
    UINT8  szLogDir[256] = {0};

 	if(is_file_exist(LOG_CONF)) {
		create_log_file();		
	} else {
		read_log_file();
	}
}
 

void WriteLogFile(UINT8 *pszFileName, UINT8 *pszFunctionName, UINT32 iCodeLine, UINT32 iLogLevel, UINT8 *pszContent) 
{
    FILE  *fp                 = NULL;
	int lock;
    UINT8  szLogContent[2048] = {0};
    UINT8  szTimeStr[128]     = {0};
 
    if (pszFileName == NULL || pszContent == NULL)
    {
        return;
    }

	lock = lock_file(g_szLogFile);
	
    fp = fopen(g_szLogFile, "at+");    
    if (fp == NULL)
    {
    	unlock_file(lock);
        return;
    }
 
    // 写入日志时间
    GetTime(szTimeStr);
    fputs(szTimeStr, fp);
    // 写入日志内容
    snprintf(szLogContent, sizeof(szLogContent)-1, "[%s][%s][%04d][%s]%s", pszFileName, pszFunctionName, iCodeLine, LogLevel(iLogLevel), pszContent);
    fputs(szLogContent, fp);
 
    fflush(fp);    
    fclose(fp);   
    fp = NULL;     
	unlock_file(lock);
//	printf("%s", szLogContent);
    return;
}
 

void set_log_default(void)
{
	g_iLogLevel = LOG_ERROR;
	g_iLogsw = 1;
	sprintf(g_szLogFile, "%s", LOG_FILE); 
	return;
}

int log_init(void)
{
	set_log_default();
    GetConfigValue();
    return 0; 
}

void _log_printf(UINT8 *pszFileName, UINT8 *pszFunctionName, UINT32 iCodeLine, UINT32 iLogLevel, char *format, ...)
{
	char print_info[MAX_LOG_LEN] = {0};
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
	
	snprintf(print_info, MAX_LOG_LEN ,"%s", tmp);
	WriteLogFile(pszFileName, pszFunctionName, iCodeLine, iLogLevel, print_info);
}
















