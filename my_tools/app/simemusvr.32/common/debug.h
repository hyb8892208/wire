/****************************************************************************
* 版权信息：
* 系统名称：SimServer
* 文件名称：debug.h 
* 文件说明：日志头文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

/**************************** 条件编译选项和头文件 ****************************/


#ifndef MSG_DEBUG_H
#define MSG_DEBUG_H

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/file.h>
#include <syslog.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>


/**************** First debug interface ****************************/
/* that is simaple, and function is not very good at all */

/**                    you can change here               */
// if define will use syslog
#define SYSLOG__
// use console to print log
//#define DEBUG_CONSOLE__
// use printf to print log
//#define DEBUG__

/**                  don't change ----- simply log implement              */
#ifdef SYSLOG__
 #define OPEN_LOG( name ) openlog( (name), LOG_PID|LOG_NDELAY, LOG_DAEMON )
 #define CLOSE_LOG() closelog()
 #define LOG(level, str, args...) syslog(level, str, ## args)
 #define DEBUG(level, str, args...) do {;} while(0)
#elif DEBUG_CONSOLE__
 #define OPEN_LOG(name) do {;} while(0)
 #define CLOSE_LOG() do {;} while(0)
 #define LOG(level, fmt, args...) \
    do { \
        FILE *fp = fopen("/dev/console", "w"); \
        fprintf(fp, fmt, ## args); \
        fprintf(fp, "\n"); \
        fclose(fp); \
    } while (0)
 #define DEBUG(level, str, args...) LOG(LOG_DEBUG, str, ## args)
#elif DEBUG__
 #define OPEN_LOG(name) openlog(name, LOG_PID|LOG_NDELAY, LOG_DAEMON)
 #define CLOSE_LOG() closelog()
 #define LOG(level, str, args...) \
    do { \
        printf("%s, ", level); \
		printf(str, ## args); \
		printf("\n"); \
    } while(0)
#endif



/**************** Second debug interface ****************************/

#define MSG_BUF_SIZE            (8192)//(512)
#define MSG_TIME_SIZE           (50)
#define MSG_FILE_MODE           (0644)

#define MSG_DEFAULT_FILE        "/var/log/msg"
#define MSG_DEFAULT_SIZE        40960-256
#define MSG_DEFAULT_ID          "General"
//#define MSG_DEBUG               1
//#define MSG_SYSLOG              (1<<1)
#define MSG_EASY                (1<<2)
#define MSG_NOSTDOUT            (1<<7)

/* log mode */
#define MSG_MODE_SHIFT          (8)
#define MSG_MODE_START          (1<<MSG_MODE_SHIFT)
#define MSG_SYSLOG              (1<<MSG_MODE_SHIFT) 
#define MSG_DEBUG               (2<<MSG_MODE_SHIFT) 
#define MSG_APP                 (3<<MSG_MODE_SHIFT) 
#define MSG_KERNEL              (4<<MSG_MODE_SHIFT) 
#define MSG_XMLP                (5<<MSG_MODE_SHIFT) 


#define M_INFO					(0)        // infomation log
#define M_WARN					(1)        // warning log
#define M_ERRO					(1<<1)     // error log
#define M_FATA					(1<<2)     // error log and will exit by caller
#define M_DEBU					(1<<3)     // debug log
#define M_MASK					(1<<4)
#define M_ERRNO					(1<<5)     // print the errno message
#define M_GENERAL			    (1<<6)
#define M_MALLOC			    (1<<7)
#define M_WARNING				( M_WARN | M_ERRNO )
#define M_ERROR					( M_ERRO | M_ERRNO )
#define M_FATAL                 ( M_FATA | M_ERRNO )
#define M_USAGE                 ( M_FATA )

#define msg( flags, ... )       debug_msg( ( flags ), ( __FILE__ ), ( __LINE__ ), __VA_ARGS__ )

/*  Description:  redirect the fd to NULL */
int set_std_files_to_null ( int stdout_only );
// initialize the msg function
void openmsg( int msgmode, const char *msgfile, const char *msgid, int msglen );
// release the msg function
void closemsg( void );
// message function
void debug_msg(const unsigned int flags,const char *file,int line,const char *format, ...);

char *str_timestamp(time_t tt, char *tstr, int tstrlen);



#ifdef  __cplusplus
    }
#endif  /* end of __cplusplus */


#endif



