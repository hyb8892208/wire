#ifndef __DEBUG_H
#define __DEBUG_H
#if 0
extern pthread_mutex_t zlog_lock;
extern zlog_category_t *c;
#define WarryPrintf(fmt, ...) \
    pthread_mutex_lock(&zlog_lock); \
    zlog_warn(c,fmt, ##__VA_ARGS__); \
    pthread_mutex_unlock(&zlog_lock)

#define DebugPrintf(fmt, ...) \
    pthread_mutex_lock(&zlog_lock); \
    zmq_debug_printf(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);\
    pthread_mutex_unlock(&zlog_lock)

    //zlog_debug(c, fmt, ##__VA_ARGS__); 
#define ErrorPrintf(fmt, ...) \
    pthread_mutex_lock(&zlog_lock); \
    zlog_error(c, fmt, ##__VA_ARGS__); \
    pthread_mutex_unlock(&zlog_lock)
#endif

#define WarryPrintf(fmt, ...) \
    warning_printf(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);

#define DebugPrintf(fmt, ...) \
    debug_printf(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);

#define ErrorPrintf(fmt, ...) \
    error_printf(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);

#define NoticePrintf(fmt, ...) \
    notice_printf(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);

#define InfoPrintf(fmt, ...) \
    info_printf(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__);

void error_printf(const char *filename, const char *func, int size, const char *format, ...);
void warning_printf(const char *filename, const char *func, int size, const char *format, ...);
void notice_printf(const char *filename, const char *func, int size, const char *format, ...);
void info_printf(const char *filename, const char *func, int size, const char *format, ...);
void debug_printf(const char *filename, const char *func, int size, const char *format, ...);
#endif
