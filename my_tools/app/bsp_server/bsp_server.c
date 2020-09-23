/*
    bsp server 主入口
    api 实现文件
    bsp 状态维护
*/
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

#include "soapH.h"
#include "ns.nsmap"

#include "public.h"
#include "czmq.h"

#include "bsp_tools.h"
#include "bsp_server.h"

#include "swg_intf.h"

/* 主函数运行标志位 */
static int g_main_run_flag = 0;

#define LOG_FILE "/tmp/log/bsp_server.log"
#define LOG_LEVEL_FILE "/etc/asterisk/gw.conf"

#define HOST_IP "127.0.0.1"

#if T_DESC("soap server")

typedef struct queue_handle_param_s
{
	int nbr;
    int thd_flag;         // 线程运行标志，=1运行中，=0已退出
    pthread_attr_t attr;  // 线程参数
	struct soap *psoap;
}queue_handle_param_t;

pthread_mutex_t queue_lock;		// 队列锁
pthread_cond_t  queue_cond;		// 队列条件变量
SOAP_SOCKET queue[MAX_QUEUE];	// 数组队列
int g_queue_head = 0;			// 队列头
int g_queue_tail = 0;			// 队列尾

struct soap *server_soap = NULL;
zpoller_t *poller = NULL;
int       g_soap_server_run_flag;  // server 线程运行标志位
pthread_attr_t g_soap_server_attr; // 线程参数
pthread_t g_soap_server_tid;       // server 线程id
pthread_t g_queue_tid[MAX_THR];    // 处理调用队列线程id
queue_handle_param_t hdl_params[MAX_THR];
struct soap *soap_thr[MAX_THR];



/* bsp print unmask, control log output level, 
 * one bit one level, =1 output 0 not output
 * bit 0 to bit 6 represent error, warning, information, 
 * debug, track, verbose, test respectively */
unsigned int bsp_prt_unmask = BSP_ERR | BSP_WARN | BSP_INFO|BSP_DBG;
FILE *bsp_log_fd = NULL ; 
static char log_file[PATH_MAX] = {0};

static void display_help(char *app_name)
{
	printf("Usage: %s [OPTION]\n"
			"  -h, --help\n"
			"  -l, --loglevle    Set log output level, MUST BE in base 16, default 0x7\n"
			"  -f, --logfile     specify log file, absulote path, default STDOUT\n"
			"\n", 
            app_name
	      );
	exit(0);
}

void process_options(int argc, char * argv[])
{
	for (;;) {
		int option_index = 0;
		static const char *short_options = "hf:l:";
		static const struct option long_options[] = {
			{"help", no_argument, 0, 0},
			{"logfile", required_argument, 0, 'f'},
			{"loglevel", required_argument, 0, 'l'},
			{0,0,0,0},
		};

		int c = getopt_long(argc, argv, short_options,
				long_options, &option_index);

		if (c == EOF) {
			break;
		}

		switch (c) {
		case 'l':
            bsp_prt_unmask = (unsigned int)strtol(optarg, NULL, 16);
			break;

        case 'f':
            bzero(log_file, sizeof(log_file));
            strcpy(log_file, optarg);
            break;

		case 'h': 
        default:
			display_help(argv[0]);
		}
	}
}

/*************************************************
  函数描述 : 打开bsp log 文件
  输入参数 : log_file -- 指定log文件
  函数返回 : 打开文件成功, 返回log文件(FILE *)FD,
             如果打开失败，返回标准输出。
  作者/时间 : junyu.yang@openvox.cn/2018.03.12
*************************************************/
static FILE *bsp_log_open(char *log_file)
{
    FILE *fd = NULL;

    if(NULL == log_file 
       || 0 == strlen(log_file)) //使用标准输出
    {
        return  stdout;
    }

    fd = fopen(log_file, "a+");
    if(NULL == fd)
    {
        printf("open %s failed : %s\n", log_file, strerror(errno));
        printf("So write log to STDOUT\n");
        return stdout;
    }
//	zsys_set_logsystem(0);
	zsys_set_logstream(fd);

    return fd;
}

/*************************************************
  函数描述 : 关闭 bsp log 文件
  输入参数 : log_fd -- 指定log文件 fd
  函数返回 : 无
  作者/时间 : junyu.yang@openvox.cn/2018.03.12
*************************************************/
static void bsp_log_close(FILE *log_fd)
{
   if(stdout != log_fd
      && NULL != log_fd)
       fclose(log_fd);
}

static void bsp_get_log_level(char *level_file){
	char *line = NULL;
	int level = 0;
	size_t len = 0;
	if(level_file == NULL){
		bsp_prt_unmask = 0;
		return;
	}
	if(access(level_file, F_OK) != 0){
		bsp_prt_unmask = 0;
		return;
	}

	FILE *fp = fopen(level_file, "r");
	if(fp == NULL){
		bsp_prt_unmask = 0;
		return;
	}

	while(getline(&line, &len, fp) > 0){
		switch(line[0]){
			case '\n':
			case '\0':
			case '#':
				continue;
			case '[':
				if(strncmp(line, "[bsp-log]", 9) == 0){
					if(getline(&line, &len, fp) < 0)
						return;
					if(strstr(line, "error")){
						level |= 1 << 0;
					}
					if(strstr(line, "warning")){
						level |= 1 << 1;
					}
					if(strstr(line, "info")){
						level |= 1 << 2;
					}
					if(strstr(line, "debug")){
						level |= 1<< 3;
					}
					if(strstr(line, "verbose")){
						level |= 1 << 4;
					}
					if(strstr(line, "track")){
						level |= 1 << 5;
					}
					
					if(strstr(line, "test")){
						level |= 1 << 6;
					}
					break;
				}else
					continue;
			default:
				continue;
		}
		break;
	}
	bsp_prt_unmask = level|1;//默认为error等级
	
	if(line)
		free(line);
	fclose(fp);
	
}
int soap_enqueue(SOAP_SOCKET sock)
{
	int status = SOAP_OK;
	int next = 0;
	//zsys_debug("enqueue: Starting[head:%03d-tail:%03d] ...", g_queue_head, g_queue_tail);
	pthread_mutex_lock(&queue_lock);
	next = g_queue_tail + 1;
	if (next >= MAX_QUEUE)
	{
		next = 0;
	}
	if (next == g_queue_head)
	{
		status = SOAP_EOM;
	}
	else
	{
		queue[g_queue_tail] = sock;
		g_queue_tail = next;
	}
	pthread_cond_signal(&queue_cond);
	pthread_mutex_unlock(&queue_lock);
	//zsys_debug("enqueue: done [head:%03d-tail:%03d]status:%d", g_queue_head, g_queue_tail, status);
	return status;
}

SOAP_SOCKET soap_dequeue(void)
{
	SOAP_SOCKET sock;
	//zsys_debug("dequeue: begin[head:%03d-tail:%03d]", g_queue_head, g_queue_tail);
	pthread_mutex_lock(&queue_lock);
	while (g_queue_head == g_queue_tail)
	{
		pthread_cond_wait(&queue_cond, &queue_lock);
	}
	sock = queue[g_queue_head++];
	if (g_queue_head >= MAX_QUEUE)
	{
		g_queue_head = 0;
	}
	pthread_mutex_unlock(&queue_lock);
	
	//zsys_debug("dequeue: done [head:%03d-tail:%03d]sock:%d", g_queue_head, g_queue_tail, sock);
	return sock;
}

void * soap_queue_handle(void *arg)
{
	queue_handle_param_t *param = (queue_handle_param_t *)arg;

	BSP_PRT( INFO,"queue_handle[%d]: Starting ...", param->nbr);

	while ( param->thd_flag )
	{
		param->psoap->socket = soap_dequeue();
		if (!soap_valid_socket(param->psoap->socket))
		{
			zsys_error("queue_handle[%d]: socket(%d) from queue not valid!!!", param->nbr, param->psoap->socket);
			break;
		}

		soap_serve(param->psoap);
		soap_destroy(param->psoap);
		soap_end(param->psoap);
	}
	BSP_PRT( INFO,"queue_handle[%d]: End", param->nbr);
	return NULL;
}

/*
desc      : soap server
param in  : 
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
void * soap_server_run(void *param)
{
    void *res = NULL;
    SOAP_SOCKET s = 0;

    g_soap_server_run_flag = 1;
	while ( g_soap_server_run_flag )
	{
		res = zpoller_wait(poller, 2000);
		if (res != &server_soap->master)
		{
			continue;
		}

		s = soap_accept(server_soap);
		if (!soap_valid_socket(s))
		{
			if (server_soap->errnum)
			{
				soap_print_fault(server_soap, stderr);
				continue;
			}
			else
			{
				BSP_PRT( ERR,"Server time out!!!");
				break;
			}
		}
		/*zsys_info("Accept connection from IP = %d.%d.%d.%d, socket = %d", \
			((server_soap.ip)>>24)&0xFF, ((server_soap.ip)>>16)&0xFF, ((server_soap.ip)>>8)&0xFF, \
			(server_soap.ip)&0xFF, server_soap.socket);*/

		while (soap_enqueue(s) == SOAP_EOM)
		{
			sleep(1);
		}
	}
    
	soap_server_stop();

    return param;
}

/*
desc      : start soap server, it will create some threads
param in  : 
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int soap_server_start(void)
{
	int ret = 0;
    int i = 0;
	SOAP_SOCKET m = 0;

    if ( ((server_soap = (struct soap*)malloc(sizeof(struct soap))) == NULL) )
        return RET_ERROR;

	soap_init(server_soap);
	server_soap->bind_flags |= SO_REUSEADDR;

    poller = zpoller_new(NULL);
	if ( poller == NULL )
    {
        soap_done(server_soap);
        free(server_soap);
		return -1;
    }
    
	ret = zpoller_add(poller, (void *)&(server_soap->master));
	if ( ret < 0 )
    {
        zpoller_destroy (&poller);
        soap_done(server_soap);
        free(server_soap);
		return -1;
    }

	m = soap_bind(server_soap, HOST_IP, soap_get_port(), BACKLOG);
	if (!soap_valid_socket(m))
	{
	    zpoller_remove(poller, (void *)server_soap->master);
	    zpoller_destroy(&poller);
		soap_print_fault(server_soap, stderr);
        soap_done(server_soap);
        free(server_soap);
		return -1;
	}
	BSP_PRT( INFO, "Socket connection successful: master socket = %d", m);

    pthread_mutex_init(&queue_lock, NULL);
	pthread_cond_init(&queue_cond, NULL);
    
	for (i = 0; i < MAX_THR; i++)
	{
	    pthread_attr_init(&(hdl_params[i].attr));
        pthread_attr_setinheritsched(&(hdl_params[i].attr), PTHREAD_INHERIT_SCHED);
        pthread_attr_setstacksize(&(hdl_params[i].attr), 262144); /* 堆栈大小256k */
    
		soap_thr[i] = soap_copy(server_soap);
        hdl_params[i].thd_flag = 1;
		hdl_params[i].nbr = i;
		hdl_params[i].psoap = soap_thr[i];
		pthread_create(&g_queue_tid[i], &(hdl_params[i].attr), soap_queue_handle, (void *)&hdl_params[i]);
	}

    pthread_attr_init(&g_soap_server_attr);
    pthread_attr_setinheritsched(&g_soap_server_attr, PTHREAD_INHERIT_SCHED);
    pthread_attr_setstacksize(&g_soap_server_attr, 262144); /* 堆栈大小256k */

    pthread_create(&g_soap_server_tid, &g_soap_server_attr, soap_server_run, NULL);
    
    return 0;
}

/*
desc      : stop soap server, it will kill all threads which create by soap_server_start
param in  : 
param out : 
return    : 
history   : create by zhongwei.peng/2017.11.24
*/
void soap_server_stop(void)
{
    int i;

    if ( server_soap == NULL )
        return;

    zpoller_remove(poller, (void *)server_soap->master);
	zpoller_destroy (&poller);

    for (i = 0; i < MAX_THR; i++)
        //pthread_cancel(g_queue_tid[i]);
        hdl_params[i].thd_flag = 0;

	for (i = 0; i < MAX_THR; i++)
	{
		BSP_PRT(INFO, "Waiting for thread %d to terminate ...", i);
		pthread_join(g_queue_tid[i], NULL);
		BSP_PRT(INFO,"Thread %d terminate", i);
		soap_done(soap_thr[i]);
		soap_free(soap_thr[i]);
	}
	pthread_mutex_destroy(&queue_lock);
	pthread_cond_destroy(&queue_cond);

	soap_done(server_soap);
    free(server_soap);
    server_soap = NULL;
}

/*
desc      : get server port
param in  : 
param out : 
return    : server port
history   : create by zhongwei.peng/2017.11.24
*/
int soap_get_port(void)
{
	int val = 8809;
	return val;
}

#endif


#if T_DESC("ZeroMQ log")

FILE *fd_log = NULL;

int czmq_init_log(char *logident)
{
	zconfig_t *root = NULL;
	char *buf = NULL;

	root = zconfig_load("/etc/asterisk/bsp_server.conf");
	if (root == NULL)
	{
		printf("init_log: load %s error\n", "/etc/asterisk/bsp_server.conf");
		return -1;
	}

	buf = zconfig_get(root, "/log/logsystem", NULL);
	if (buf == NULL || strlen(buf) <= 0)
	{
		zsys_set_logsystem(0);
	}
	else
	{
		zsys_error("init_log: logsystem = %s", buf);
		zsys_set_logsystem(atoi(buf));
	}

	//
	zsys_info("init_log: logident = %s", logident);
	zsys_set_logident(logident);
	//
	buf = zconfig_get(root, "/log/logfile", NULL);
	if (buf != NULL && strlen(buf) > 0)
	{
		zsys_info("init_log: logfile = [%s]", buf);
		fd_log = fopen(buf, "a+");
		if (fd_log == NULL)
		{
			zsys_error("init_log: open %s error(%d:%s)", buf, errno, strerror(errno));
		}
		else
		{
			zsys_set_logstream (fd_log);
		}
	}

	zconfig_destroy(&root);
	return 0;
}

#endif

#if T_DESC("命令行")

/* 最多10个参数 */
#define PARAM_MAX         (10)

/* 一行最多支持256字节 */
#define CLI_MAX_LEN       (256)

/* 输入缓存 */
char g_input_buf[CLI_MAX_LEN];
unsigned int g_input_idx = 0;

void cmd_usage(void)
{
    printf("Usage:\n");
    printf("quit -- program exit\n");
}

void cmd_proc(char *input_buf, unsigned int len)
{
    int argc;
    char *argv[PARAM_MAX]; /* 最多10个参数 */
    char *p;

    /* 格式化字符串，把空格分开的词放入多个字符串 */
    p = input_buf;
    while ( *p && (' ' == *p) ) p++;

    if ( *p == '\0' )
        return;

    argc = 0;
    argv[argc++] = p;

    while ( *p )
    {
        if ( ' ' == *p ) 
        {
            *p++ = '\0';

            while ( *p && (' ' == *p) ) p++;

            if ( *p == '\0' )
                break;
            else
                argv[argc++] = p;

            if ( argc >= PARAM_MAX )
                return;
        }

        p++;
    }

    /* 处理消息 */
    if ( strcmp("ver", argv[0]) == 0 )
        printf("version: 1.1\r\n"); 
    else if ( strcmp("quit", argv[0]) == 0 )
    {
        printf("program exit...\r\n");
        g_main_run_flag = 0;
    }
    else
    {
        printf("unknown cmd: %s\r\n\r\n", argv[0]);
        cmd_usage();
    }

    return;
}

void cmd_parse(char c)
{
    switch ( c )
    {
        case '\n': /* 0x0A, 回车的处理 */
            //g_input_buf[g_input_idx++] = '\r';
			//g_input_buf[g_input_idx++] = c;
            g_input_buf[g_input_idx] = '\0';
            printf("%c", '\r');
			printf("%c", c);
			cmd_proc(g_input_buf, g_input_idx);
            g_input_idx = 0;
            g_input_buf[g_input_idx] = '\0';
            break;

        //case '\r': /* 0x0D, 只处理'\n'，'-r'被忽略 */
        //    break;

        //case 0x09: /* TAB 键 */
        //    break;

        case '\b': /* 0x08, 退格键 */
            if ( g_input_idx > 0 )
            {
                g_input_idx--;
                printf("\b \b");
            }
            g_input_buf[g_input_idx] = 0;
            break;   

        default:   
            /* 添加可见字符，其它忽略 */
            if ( g_input_idx < (CLI_MAX_LEN-1) )
                g_input_buf[g_input_idx++] = c;
            else
                g_input_idx = 0; 
			//printf("%c", c);
            break;
    }
}

#endif

#if T_DESC("bsp")

/*
    通讯模块初始化(包括sim card)
*/
/*
 *void tel_module_init(void)
 *{
 *    int ALL_CHN = -1;
 *
 *    [> 彻底重启模块 <]
 *    brd_mcu_module_turn_off(ALL_CHN);
 *    brd_mcu_simcard_disable(ALL_CHN);
 *    sleep(2);
 *    brd_mcu_module_power_off(ALL_CHN);
 *    sleep(1);
 *    brd_mcu_simcard_enable(ALL_CHN);
 *    brd_mcu_module_power_on(ALL_CHN);
 *    sleep(1);
 *    brd_mcu_module_turn_on(ALL_CHN);
 *}
 */

/*
desc      : init the whole board and software
param in  : 
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.12.05
*/
/*
 *static int bsp_init(struct swg_device *dev)
 *{
 *    module_board_init(dev);
 *    
 *    [>probe_mcu_module(dev);<]
 *    swg_channel_init(dev);
 *    brd_info_init();
 *    [>tel_module_init();<]
 *    bs_timer_init();
 *    bs_timer_add(brd_mcu_event_check, 2000); [> 2秒检查一次事件 <]
 *    return 0;
 *}
 */

#endif

void sig_handle(int sig){
	g_main_run_flag = 0;
}

int main(int argc, char **argv)
{
    int res;
    process_options(argc, argv);

    bsp_log_fd = bsp_log_open(LOG_FILE);

    if ((res = swg_device_init()) < 0 ){
		BSP_PRT(ERR,"[%s %d] swg_device_init failed, res = %d\n", __FILE__, __LINE__, res);
		return -1;
    }
    if ( soap_server_start() < 0 ){
		BSP_PRT(ERR,"[%s %d] soap_server_start failed!\n", __FILE__, __LINE__);
		return -1;
    }
	
    bsp_get_log_level(LOG_LEVEL_FILE);
    g_main_run_flag = 1;
    zsys_handler_set(sig_handle);
    while ( g_main_run_flag )
    {
  //      len = read(STDIN_FILENO, &c, 1);
    //    if ( len < 0 )
      //      continue;

//        cmd_parse(c);
        usleep(100000);
    }

    g_soap_server_run_flag = 0;
    sleep(3);
    pthread_cancel(g_soap_server_tid);
    pthread_join(g_soap_server_tid, NULL);

    swg_device_deinit();
    bsp_log_close(bsp_log_fd);
	return 0;
}




