/**********************************************************************************
Description : 产品led/lcd显示控制程序。处理整个单板所有端口的状态，
              并把这些状态通过led/lcd显示出来
Author      : zhongwei.peng@openvox.cn
Time        : 2016.12.10
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include<pthread.h>

#include "ledhdl.h"

#include "keyboard.h"
#include "menudisplay.h"
#include "menuctrl.h"
#include "redis_if.h"


/* 目前asterisk最大端口个数 */
#define AST_SPAN_MAX             (32)

#define SYS_STATE_READY          (0)  /* 系统就绪，asterisk未启动 */
#define SYS_STATE_AST_OK         (1)  /* asterisk已启动 */
#define SYS_STATE_REBOOT         (2)  /* 系统重启状态 */

#define BAUD_RATE "115200"

#define PROG_SHUTDOWN_MODE 3
#define PROG_RUNNING_MODE 2
#define PROG_START_MODE 1
#define PROG_REBOOT_MODE 0

/* LED/LCD操作指令 */
#define SYS_RUNNING_CMD         "state=3 "      /* 系统运行状态命令 */
#define SYS_START_CMD           "state=2\n"     /* 系统开机命令 */
#define SYS_REBOOT_CMD          "state=1\n"     /* 系统重启命令 */
#define SYS_SHUTDOWN_CMD        "state=0\n"     /* 系统关机命令 */
#define SYS_PARAM_CMD       "param=3 "
#define SYS_DESCR_CMD        "descr=3 "
#define SYS_RUNLED_CMD  "led=1\n"
#define SYS_LCD_VERSION         "ver\n"         

#define LIMIT_SH_PATH               "/my_tools/set_calllimit.sh"
/* 枚举asterisk端口状态 */ 
typedef enum tagAST_PORT_STATE_ENUM 
{ 
    AST_PORT_STATE_EMPTY = 0,      /* 没有模块 */ 
    AST_PORT_STATE_OFF,            /* 模块不可用 */ 
    AST_PORT_STATE_SIM_ABS,        /* 模块OK，sim卡不在位 */ 
    AST_PORT_STATE_NET_REQ,        /* 模块OK，sim卡在位，网络请求中(注册中) */ 
    AST_PORT_STATE_REG_OK_1,       /* 注册OK，信号质量为等级 - 1 */ 
    AST_PORT_STATE_REG_OK_2,       /* 注册OK，信号质量为等级 - 2 */ 
    AST_PORT_STATE_REG_OK_3,       /* 注册OK，信号质量为等级 - 3 */ 
    AST_PORT_STATE_REG_OK_4,       /* 注册OK，信号质量为等级 - 4 */ 
    AST_PORT_STATE_REG_OK_5,       /* 注册OK，信号质量为等级 - 5 */ 
    AST_PORT_STATE_CALL_1,         /* 通话中，信号质量为等级 - 低 */
	AST_PORT_STATE_CALL_2,         /* 通话中，信号质量为等级 - 中 */
	AST_PORT_STATE_CALL_3          /* 通话中，信号质量为等级 - 高 */
}AST_PORT_STATE_ENUM;

/* 描述系统当前状态 */
int g_sys_state = SYS_STATE_READY;

/* 当前asterisk物理端口状态 */
int g_ast_port_state[AST_SPAN_MAX] = {AST_PORT_STATE_EMPTY};

int g_ast_limit_state[AST_SPAN_MAX] = {AST_PORT_STATE_EMPTY};
/* 全局的16/32的lcd操作符 */
int g_fd_32 = -1;

//int g_sys_menu_state = 0;


/**********************************************************
函数描述 : 判断asterisk是否运行中
输入参数 : 
输出参数 : 
返回值   : 运行中返回1，否则返回0
作者/时间: zhongwei.peng / 2016.12.12
************************************************************/
int ast_is_running(void)
{
    FILE *fp;
    int count = 0;
    char buf[1024];
    char command[] = "pidof asterisk";

    if ( (fp = popen(command, "r")) == NULL )
        return 0;

    if ( (fgets(buf, 1024, fp)) != NULL )
        count = atoi(buf);

    pclose(fp);

	return count;
}

/**********************************************************
函数描述 : 检查指定文件是否超过 指定时间没有被修改过
输入参数 : filepath -- 文件名称及路径
           s -- 时间，秒数
输出参数 : 
返回值   : 端口正常返回1，否则返回0
作者/时间: zhongwei.peng / 2016.12.12
************************************************************/
int ast_port_is_ok(char *filepath, int s)
{
    struct stat st;

    if ( stat(filepath, &st) != 0 )
        return 0;

    if ( (time(NULL) - st.st_mtime) > s )
        return 0;

    return 1;
}

/**********************************************************
函数描述 : 从字符串中获取物理端口号
输入参数 : buf -- 端口状态字符串
输出参数 : 
返回值   : 返回物理端口号
作者/时间: zhongwei.peng / 2016.12.12
************************************************************/
int ast_port_get_num(char *buf)
{
    int port_num = 0;
    char *p;

    if ( (p = strstr(buf, "Hardware Port: ")) == NULL )
        return -1;

    while ( *p )
    {
        if ( (*p >= '0') && (*p <= '9') )
        {
            while ( *p )
            {
                if ( ('\r' == *p) || ('\n' == *p) )
                {
                    return port_num;
                }
                else
                {
                    port_num = port_num * 10 + (*p - '0');
                }
                p++;
            }

            return port_num;
        }
        else if ( ('\r' == *p) || ('\n' == *p) )
        {
            return -1;
        }
        else
        {
            p++;
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 从字符串中获取端口状态
输入参数 : buf -- 端口状态字符串
输出参数 : 
返回值   : 返回状态值
作者/时间: zhongwei.peng / 2016.12.12
************************************************************/
int ast_port_state_parse(char *buf)
{
    char *p, *end;
    int state;

    if ( (p = strstr(buf, "Port Status:")) == NULL )
    {
        printf("error while reading \"Port Status:\" \r\n");
        return AST_PORT_STATE_OFF;
    }

    end = p;
    while ( *end )
    {
        if ( (*end == '\r') || (*end == '\n') )
        {
            *end = '\0';
            break;
        }
        else
        {
            end++;
        }
    }

    if ( strstr(p, "Module Off") != NULL )
        state = AST_PORT_STATE_OFF;
    else if ( strstr(p, "Undetected SIM Card") != NULL )
        state = AST_PORT_STATE_SIM_ABS;
    else if ( strstr(p, "Un Registered") != NULL )
        state = AST_PORT_STATE_NET_REQ;
    else if ( strstr(p, "Registered-1") != NULL )
        state = AST_PORT_STATE_REG_OK_1;
    else if ( strstr(p, "Registered-2") != NULL )
        state = AST_PORT_STATE_REG_OK_2;
    else if ( strstr(p, "Registered-3") != NULL )
        state = AST_PORT_STATE_REG_OK_3;
    else if ( strstr(p, "Registered-4") != NULL )
        state = AST_PORT_STATE_REG_OK_4;
    else if ( strstr(p, "Registered-5") != NULL )
        state = AST_PORT_STATE_REG_OK_5;
    else if ( strstr(p, "Calling-1") != NULL )
        state = AST_PORT_STATE_CALL_1;
	else if ( strstr(p, "Calling-2") != NULL )
        state = AST_PORT_STATE_CALL_2;
	else if ( strstr(p, "Calling-3") != NULL )
        state = AST_PORT_STATE_CALL_3;
    else
        state = AST_PORT_STATE_OFF;

    /* 恢复 */
    *end = '\n';

    return state;
}

static int get_limit_state_value(int port_num,const char *label)
{
    char *p;
    char *n;
    int ret = 0;
    char buffer[255] = {0};
    FILE *fp;
    
    char cmdline[READ_BUFF_LEN] = {0};
    sprintf(cmdline, "%s  status %d", LIMIT_SH_PATH,port_num);
    fp = popen(cmdline,"r");
    if(fp == NULL)
        return 0;
    
    fgets(buffer, sizeof(buffer), fp);
    p = buffer;
    p = strstr(p, label);
    while(p == NULL)
    {
        n = fgets(buffer, sizeof(buffer), fp);
        if(n == NULL)
        {
            pclose(fp);
            return 0;
        }
        p= buffer;
        p = strstr(p, label);
    }
    p += strlen(label);
    while(p != NULL)
    {
        if(*p == ' ' || *p == '\t')
            p++;
        else
            break;
    }

    if(p == NULL)
    {
        pclose(fp);
        return 0;
    }

    
    if(*p <= '9' && *p >= '0')
    {
        ret = *p - '0';
    }else{
        if(0 == strcmp(p,"LIMIT\n"))
            ret = 1;
    }
    
    pclose(fp);
    return ret;
}

int ast_port_limit_get(int port_num)
{
    int limit,mark,lock,sms = 0;
    unsigned int state = 0;
    char* n = 0;
    
    limit = get_limit_state_value(port_num,"limit_sta:");
    mark = get_limit_state_value(port_num,"call_fail_mark_status:");
    lock = get_limit_state_value(port_num,"call_fail_lock_status:");
    sms = get_limit_state_value(port_num,"sms_limit_sta:");
    state = 0x0|(sms<<3)|(limit<<2)|(mark <<1)|lock;

    return state;
}

/**********************************************************
函数描述 : 判断asterisk是否运行中
输入参数 : 
输出参数 : 
返回值   : 运行中返回1，否则返回0
作者/时间: zhongwei.peng / 2016.12.12
************************************************************/
void ast_port_state_update(void)
{
    int i;
    int fd;
    int len;
    int port_num;
    char filepath[32];
    char buf[1024];

    for ( i = 1; i <= AST_SPAN_MAX; i++ )
    {
        snprintf(filepath, sizeof(filepath), "/tmp/gsm/%d", i);
        if ( access(filepath, F_OK) == -1 )
            continue;

        if ( (fd = open(filepath, O_RDONLY)) < 0 )
        {
            printf("span[%d] open file fail: %s \r\n", i, filepath);
            continue;
        }

        flock(fd, LOCK_EX);
        len = read(fd, buf, sizeof(buf));
        flock(fd, LOCK_UN);
        close(fd);

        if ( len > 0 )
            buf[len] = '\0';
        else
            continue;

        port_num = ast_port_get_num(buf);

        /* 6 秒没更新就代表fail */
        if ( !ast_port_is_ok(filepath, 6) )
        {
            g_ast_port_state[port_num -1] = AST_PORT_STATE_EMPTY;
            continue;
        }

        /* 物理端口从1-32 */
        if ( (port_num < 1) || (port_num > 32) )
            continue;

        g_ast_port_state[port_num - 1] = ast_port_state_parse(buf);
        g_ast_limit_state[port_num - 1] = ast_port_limit_get(port_num);
    }
}

/**********************************************************
函数描述 : 发送系统及端口状态到mcu
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2016.12.12
************************************************************/
int sys_state_output(char *result)
{
    int i;
    char *p;
    char buf[256];

    if ( SYS_STATE_READY == g_sys_state )
    {
		sprintf(buf, SYS_START_CMD);
		memcpy(result, buf, strlen(buf));
        return 1;
    }

    sprintf(buf, SYS_RUNNING_CMD);
    p = &buf[strlen(buf)];

    for ( i = 0; i < AST_SPAN_MAX; i++ )
    {
        if ( g_ast_port_state[i] >= 10 )
            *p = 'A' + g_ast_port_state[i] - 10;
        else 
            *p = '0' + g_ast_port_state[i];
        p++;
    }
    
    *p++ = '\n';
    *p = '\0';

	memcpy(result, buf, strlen(buf));
	return 0;
}

int sys_param_output(char *result)
{
    int i;
    char *p;
    char buf[256];

    sprintf(buf, SYS_PARAM_CMD);
    p = &buf[strlen(buf)];

    for ( i = 0; i < AST_SPAN_MAX; i++ )
    {
        if(g_ast_limit_state[i] > 0)
            *p = '1';
        else
            *p = '0';
        p++;
    }
    
    *p++ = '\n';
    *p = '\0';

	memcpy(result, buf, strlen(buf));
	return 0;
}

int sys_descr_output( char *result){
	char buf[256] = {0};
	char *p;
	sprintf(buf, SYS_DESCR_CMD);
	p = &buf[strlen(buf)];
	get_sim_slot_info( p);
	buf[strlen(buf)] = '\n';
	memcpy(result, buf, strlen(buf));
	return 0;
}

void get_mcu_dev(char *dev)
{
	FILE *fp;
	char buf[32] = {0};
	char *cmd = "/my_tools/set_config  /tmp/hw_info.cfg get option_value lcd lcd_1";
	if ( (fp = popen(cmd, "r")) == NULL )
        return;

    if ( (fgets(buf, 1024, fp)) != NULL ) {
		printf("buf is %s\n", buf);
		pclose(fp);
		strcpy(dev, buf);
	}
	return;
}

int send_control_cmd(char *buf, int type) {
	int ret = 0;
	char result[128];
//	printf("send control type is %d, command (%s)\n", type, buf);
	if ( 8 == type ) {
		//8口: 控制led灯的闪烁,显示模块的状态
	} else if ( 16 == type || 32 == type ) {
		// 16/32口: 控制LCD显示屏,显示模块的状态,LCD是独立的MCU
		if (g_fd_32 < 0 ) {
			printf("get lcd fd failed\n");
			return -1;
		}
		pthread_mutex_lock(&menu_ctrl.lock);	
		ret = com_write(g_fd_32, buf, strlen(buf));
		pthread_mutex_unlock(&menu_ctrl.lock);		
	} else {
		printf("board type get error.....\n");
		return -1;
	}
	return 0;
}

void lcd_menu_init(int confd)
{
	menudisplay_init(confd);
	check_keyboard_create(confd);
	menu_ctrl_pthread_create();
}

void usage(int argc, char *argv[])
{
    char *file_name = argv[0];

    printf("Usage:\n");
    printf("%s start|stop|reboot\n", file_name);
}

int main(int argc, char *argv[])
{
	int ret = 0;
	char buf[3][128];
	int i = 0;
	int type = -1;
	int boardtype = -1;
       int interval = 0;
       int sys_type = -1;
    
	if (argc < 2) {
		usage(argc, argv);
		return -1;
	} else {
		if ( 0 == strcmp(argv[1], "start")) {
			printf("in start mode\n");
			type = PROG_START_MODE;
		} else if ( 0 == strcmp(argv[1], "stop")) {
			printf("in stop mode\n");
			type = PROG_SHUTDOWN_MODE;
		} else if ( 0 == strcmp(argv[1], "reboot")) {
			printf("in reboot mode\n");
			type = PROG_REBOOT_MODE;
		} else {
			usage(argc, argv);
			return -1;
		}
	}
	memset(buf, 0, sizeof(buf));
	sys_type = get_sys_type();
	/* 获取型号：8口 16口 32口  */
	boardtype = get_total_channel();
	if(boardtype < 16)
		boardtype = 16;
	else if(boardtype > 16)
		boardtype = 32;
	if ( 16 == boardtype || 32 == boardtype ) {
		/* 16/32口: 获取lcd的描述符 */
		char dev[32] = {0};
		while( 1 ) {
			get_mcu_dev(dev);
			g_fd_32 = com_open(dev);
			if ( g_fd_32 < 0 ){
				printf("Open dev(%s) error.try again\n",dev);
			} else {
				printf("Open dev(%s) success\n",dev);
				break;
			}
			i++;
			if ( i >= 10 ) {
				printf("retry times too many, return....\n");
				return -1;
			}
			sleep(1);
		}
		m_setparms(g_fd_32, BAUD_RATE, "N", "8", "1", 1, 0);
		lcd_menu_init(g_fd_32);
		led_create_redis_thread(sys_type);
	}
	if ( type == PROG_SHUTDOWN_MODE ) {
		/* 关机模式 */
		sprintf(buf[0], SYS_SHUTDOWN_CMD);
		ret = send_control_cmd(buf[0], boardtype);
		sleep(1);   /* 持续1秒 */
		return 0;
	} else if ( type == PROG_REBOOT_MODE) {
		/* 重启模式 */
		sprintf(buf[0], SYS_REBOOT_CMD);
		ret = send_control_cmd(buf[0], boardtype);
		sleep(1);   /* 持续1秒 */
		return 0;
	} else if ( type == PROG_START_MODE) {
		/* 开机模式 */
		sprintf(buf[0], SYS_START_CMD);
		ret = send_control_cmd(buf[0], boardtype);
		sleep(1);   /* 持续1秒 */
		while ( 1 ) {
			/* 1秒钟扫描一次状态，发送一次系统运行命令 */
			sleep(1);
			if ( ast_is_running() ){
				g_sys_state = SYS_STATE_AST_OK;
				ast_port_state_update();
			} else {
				g_sys_state = SYS_STATE_READY;
			}

			if(interval %3 == 0)
			{
				ret = send_control_cmd(SYS_RUNLED_CMD, boardtype);
				interval = 1;
			}else{
				interval ++;
			}

			memset(buf[0], 0, sizeof(buf[0]));
			memset(buf[1], 0, sizeof(buf[1]));
			memset(buf[2], 0, sizeof(buf[2]));
			ret = sys_state_output(buf[0]);
			ret |= sys_param_output(buf[1]);
			ret |= sys_descr_output(buf[2]);
			if ( ret < 0 ) {
				printf("get date error\n");
				continue;
			}

			if(!menu_ctrl.MenuState)
			{         
				ret = send_control_cmd(buf[1], boardtype);
				ret = send_control_cmd(buf[0], boardtype);
				ret = send_control_cmd(buf[2], boardtype);
			}
		}
		if ( (32 == boardtype || 16 == boardtype ) && g_fd_32 > 0 ) {
			com_close(g_fd_32);
		}
		return 0;
	} else {
		usage(argc, argv);
		return -1;
	}

    return 0;
}
