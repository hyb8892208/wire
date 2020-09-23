/****************************************************************************
* 版权信息：
* 系统名称：SimServer
* 文件名称：debug.c
* 文件说明：日志实现文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/




#include "debug.h"



/* msg code string */
static const char *msglevel_string[] = {
    "<INFO>",    /* M_INFO */
    "<WARNING>", /* M_WARN */
    "<ERROR>",   /* M_ERRO */
    "",
    "<FATAL>",   /* M_FATA */
    "",
    "",
    "",
    "<DEBUG>",   /* M_DEUB */
    NULL };
/* msg code to log level */
static const int   msg_loglevel[] = {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERR,
    0,
    LOG_EMERG,
    0,
    0,
    0,
    LOG_DEBUG,
    0 };

/* message status info */
static struct {
    int msgmode;
    char *msgfile;
    char *msgid;
    int msgsize;
} msg_status;


/* get the time string */
#if 1
char *str_timestamp(time_t tt, char *tstr, int tstrlen)
{
	struct timeval tv;
	struct tm *ptm = NULL;
	
	gettimeofday(&tv, NULL);
	ptm = localtime(&tv.tv_sec);

	sprintf(tstr, "%04d-%02d-%02d %02d:%02d:%02d'%03d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, (int)(tv.tv_usec/1000));
	
	return tstr;
}

#else
inline char *str_timestamp( time_t  tt, char *tstr, int tstrlen )
{
	struct tm t;

	if( !tt )
	{
	    tt = time( NULL );
	}
    /* transfor the time_t to structure named tm */
	localtime_r( &tt, &t );
	strftime( tstr, tstrlen, "%Y-%m-%d %X", &t );
	return ( tstr );
}
#endif
inline void reset_file_len( char *file, int msglen )
{
    struct stat st;

    if ( stat( file, &st ) == 0 )
    {
        if ( st.st_size >= msglen )
        {
            unlink( file );
        }
    }
}
/*  Description:  redirect the fd to NULL */
int set_std_files_to_null( int stdout_only )
{
    /* Set standard file descriptors to /dev/null */
	int fd;
    int err;

    /* open the NULL device */
	if ( ( fd = open( "/dev/null", O_RDWR, 0 ) ) >= 0 )
	{
		if ( dup2( fd, STDOUT_FILENO ) < 0 )
        {
            err = errno;
            close( fd );
            errno = err;
            return -1;
        }
		if ( !stdout_only )
		{
			dup2( fd, STDERR_FILENO );
		}
		if ( 2 < fd )
		{
			close( fd );
		}
        return 0;
	}
    return fd;
}


/*  Description:  debug reset/init */
void openmsg( int msgmode, const char *msgfile, const char *msgid, int msgsize )
{
    closemsg( );

    msg_status.msgmode = msgmode;
    if ( NULL != msgfile )
    {
        msg_status.msgfile = strdup( msgfile );
    }
    else
    {
        msg_status.msgfile = strdup( MSG_DEFAULT_FILE );
    }
    if ( NULL != msgid )
    {
        msg_status.msgid = strdup( msgid );
    }
    else
    {
        msg_status.msgid = strdup( MSG_DEFAULT_ID );
    }
    if ( msgsize > 0 )
    {
        msg_status.msgsize = msgsize;
    }
    else
    {
        msg_status.msgsize = MSG_DEFAULT_SIZE;
    }
    if ( msgmode & MSG_NOSTDOUT )
    {
        set_std_files_to_null( 0 );
    }
    switch( msg_status.msgmode )
    {
        case MSG_SYSLOG:
            openlog ( msg_status.msgid, LOG_PID, LOG_DAEMON );
    }
}
/*  Description:  release the debug info */
void closemsg( void )
{
    if ( NULL != msg_status.msgfile )
    {
        free( msg_status.msgfile );
    }
    if ( NULL != msg_status.msgid )
    {
        free( msg_status.msgid );
    }
    switch( msg_status.msgmode )
    {
        case MSG_SYSLOG:
            closelog();
    }
    msg_status.msgmode = MSG_DEBUG;
}

/*  Description:  message */
void debug_msg( unsigned int flags, const char *file, int line, const char *format, ... )
{
    char *msgfile;
    int msgsize;
	va_list arglist;
	char buf1[ MSG_BUF_SIZE ];
	char buf2[ MSG_BUF_SIZE ];
	char timestr[MSG_TIME_SIZE];
    int len = 0;
    int errnum = errno;
    int msgfd = STDOUT_FILENO;

	va_start ( arglist, format );
	vsnprintf ( buf1, MSG_BUF_SIZE, format, arglist );
	va_end ( arglist );

    switch ( msg_status.msgmode )
    {
        case MSG_SYSLOG:
            /* mutex_lock_static (T_MSG); */
            if ( flags & M_GENERAL )
            {
                closelog();
                openlog( MSG_DEFAULT_FILE, LOG_PID, LOG_DAEMON );
            }
            if ( flags & M_MALLOC )
            { /* Fail memory allocation.
                 Don't use more operate because it tries to allocate memory as part of its operation. */
                syslog ( LOG_CRIT, "<CRITICAL>out of memory" );
                syslog( msg_loglevel[ ( flags&M_MASK ) ], "%s {%s->%d}", buf1, file, line );
            }
            else if ( flags & M_ERRNO )
            {
                syslog( msg_loglevel[ ( flags&M_MASK ) ], "%s {%s->%d}(errno=%d)", buf1, file, line, errnum );
            }
            else
            {
                syslog( msg_loglevel[ ( flags&M_MASK ) ], "%s {%s->%d}", buf1, file, line );
            }
            if ( flags & M_GENERAL )
            {
                openlog ( msg_status.msgid, LOG_PID, LOG_DAEMON );
            }
            /* nlf_mutex_unlock_static (T_MSG); */
            break;
        case MSG_EASY:
            if ( flags & M_GENERAL )
            {
                msgfile = MSG_DEFAULT_FILE;
                msgsize = MSG_DEFAULT_SIZE;
            }
            else
            {
                msgfile = msg_status.msgfile;
                msgsize = msg_status.msgsize;
            }
            reset_file_len( msgfile, msgsize );
            msgfd = open( msgfile, O_CREAT|O_WRONLY|O_APPEND, MSG_FILE_MODE );
            if( msgfd < 0 )
            {
                msg_status.msgmode = MSG_SYSLOG;
                msg( M_ERRNO, "Error open message file(%s) for record", msg_status.msgfile );
                break;
            }
        case MSG_DEBUG:
        default:
            if ( flags & M_MALLOC )
            {
                write( msgfd, "<CRITICAL>out of memory\n", 24 );
                len = snprintf( buf2, MSG_BUF_SIZE, "[%s]%s%s:%s {%s->%d}\n", str_timestamp( 0, timestr, sizeof( timestr ) ),
                    msglevel_string[ ( flags & M_MASK ) ], msg_status.msgid, buf1, file, line );
            }
            else if ( flags & M_ERRNO )
            {
                len = snprintf( buf2, MSG_BUF_SIZE, "[%s]%s%s:%s {%s->%d}(err=%d)\n", str_timestamp( 0, timestr, sizeof( timestr ) ),
                    msglevel_string[ ( flags & M_MASK ) ], msg_status.msgid, buf1, file, line, errnum );
            }
            else
            {
                len = snprintf( buf2, MSG_BUF_SIZE, "[%s]%s%s:%s {%s->%d}\n", str_timestamp( 0, timestr, sizeof( timestr ) ),
                    msglevel_string[ ( flags & M_MASK ) ], msg_status.msgid, buf1, file, line );
            }
            write( msgfd, buf2, len );
            if ( MSG_EASY == msg_status.msgmode )
            {
                close( msgfd );
            }
    }

	if ( flags & M_FATA )
	{
        closemsg( );
		exit( -1 ); /* exit point */
	}
}

