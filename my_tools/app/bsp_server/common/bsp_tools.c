/**********************************************************************************
�ļ�����  : ���õĴ���
����/ʱ�� : zhongwei.peng@openvox.cn/2017.12.8
***********************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdarg.h>
#include "public.h"
#include "czmq.h"
#include "bsp_tools.h"

/**********************************************************
�������� : 16�����ַ�ת��ֵ
������� : c -- �ַ��������������
������� : 
�� �� ֵ : ��ֵ��
��   ��  : zhongwei.peng
ʱ   ��  : 2016.11.25
************************************************************/
unsigned char char_to_val(unsigned char c)
{
    if ( (c >= '0') && (c <= '9') )
        return c - '0';
    else if ( (c >= 'A') && (c <= 'Z') )
        return c - 'A' + 10;
    else if ( (c >= 'a') && (c <= 'z') )
        return c - 'a' + 10;
    else 
        return 0;
}

/**********************************************************
�������� : �ַ���ת��ֵ
������� : str -- �ַ���
������� : 
�� �� ֵ : ��ֵ��
��   ��  : zhongwei.peng
ʱ   ��  : 2016.11.25
************************************************************/
int str_to_int(unsigned char *str)
{
    int flag;
    int value = 0;
    unsigned char *p = str;

    if ( NULL == str )
        return 0;

    /* 16���� */
    if ( (str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')) )
    {
        value = 0;
        p = &str[2];
        while ( *p )
        {
            value = value * 16 + char_to_val(*p);
            p++;
        }
        return value;
    }

    /* 10���� */
    if ( str[0] == '-' )
        flag = -1;
    else
        flag = 1;

    value = 0;
    p = str;
    while ( *p ) 
        value = value * 10 + char_to_val(*p++);

    value = value * flag;

    return value;
}

#if T_DESC("�������")

typedef unsigned char			uint8;
typedef signed char				sint8;
typedef unsigned short			uint16;
typedef signed short			sint16;
typedef unsigned int			uint32;
typedef signed int 				sint32;
typedef unsigned long long		uint64;
typedef signed long long	    sint64;
typedef float					float32;
typedef double			    	float64;
typedef uint16					bool16;
typedef uint8					bool8;

#define SERIAL_SUCC		0
#define SERIAL_ERR		-1
#define SERIAL_SELECT_TIMEOUT	-2

#define SERIAL_WR_TIMEOUT 1

// serial attr struct
typedef struct serial_attr_s
{
	uint32 rate;
	uint8 parity;
    uint8 databits;
    uint8 stopbits;
    uint8 streamcontrol;
}serial_attr_t;

// baud map struct
typedef struct baudmap_s
{
	uint32 baud;
	uint32 flag;
}baudmap_t;

// baud map table
baudmap_t baud_table[] = 
{
	{ 1200,   B1200 },
	{ 2400,   B2400 },
	{ 4800,   B4800 },
	{ 9600,   B9600 },
	{ 19200,  B19200 },
	{ 38400,  B38400 },
	{ 57600,  B57600 },
	{ 115200, B115200 },
	{ 230400, B230400 },
	{ 460800, B460800 },
	{ 921600, B921600 },
};

#define NMBAUDS (sizeof(baud_table) / sizeof(baudmap_t))

static sint32 baud2flag(uint32 baud);
sint32 open_serial(char *serial_file, uint32 rate, uint8 parity, uint8 databits, uint8 stopbits, uint8 streamcontrol);
void close_serial(uint32 serial_fd);
sint32 get_termios(uint32 serial_fd, struct termios *old_term);
sint32 set_termios(uint32 serial_fd, struct termios *old_term);
sint32 set_serial_attr(uint32 serial_fd, serial_attr_t *serial_attr);

// find relevant map flag
static sint32 baud2flag(uint32 baud)
{
	uint32	i = 0;
	for (; i < NMBAUDS; ++i)
	{
		if (baud == baud_table[i].baud)
		{
			return baud_table[i].flag;
		}
	}
	return SERIAL_ERR;
}

/******************************************************************************
Function	:	open_serial
Description	:	�򿪴���
Input		:	serial_file		�����ļ�
Return		:	�ɹ������ش��������� ʧ�ܣ����أ�-1
******************************************************************************/
int open_serial
(
    char *serial_file, 
    unsigned int rate, 
    unsigned char parity, 
    unsigned char databits, 
    unsigned char stopbits, 
    unsigned char streamcontrol
)
{
	int fd = -1;
	int ret = -1;
	serial_attr_t attr;
	struct termios term;

	fd = open(serial_file, (O_RDWR | O_NDELAY | O_NONBLOCK));
	if (fd < 0)
	{
		return SERIAL_ERR;
	}

	//
	memset(&term, 0, sizeof(term));
	ret = get_termios(fd, &term);
	if (ret != 0)
	{
		close_serial(fd);
		return -1;
	}

	//
	attr.rate = rate;
	attr.parity = parity;
	attr.databits = databits;
	attr.stopbits = stopbits;
	attr.streamcontrol = streamcontrol;

	ret = set_serial_attr(fd, &attr);
	if (ret != 0)
	{
		printf("set_serial_attr error\n");
		close_serial(fd);
		return -1;
	}

	return fd;
}

/******************************************************************************
Function	:	close_serial
Description	:	�رմ���
Input		:	serial_fd		�����ļ�������
Return		:	�ɹ�������0 ʧ�ܣ����أ�-1
******************************************************************************/
void close_serial(unsigned int serial_fd)
{
	close(serial_fd);
}

/******************************************************************************
Function	:	get_termios
Description	:	��ȡ����termios
Input		:	serial_fd		�����ļ�������
				old_term		����termios���ݽṹ
Return		:	�ɹ�������0 ʧ�ܣ����أ�-1
******************************************************************************/
sint32 get_termios(uint32 serial_fd, struct termios *old_term)
{
	sint32 ret = 0;
	ret = tcgetattr(serial_fd, old_term);
	if (ret < 0)
	{
		return SERIAL_ERR;
	}
	return SERIAL_SUCC;
}

/******************************************************************************
Function	:	set_termios
Description	:	���ô���termios
Input		:	serial_fd		�����ļ�������
				old_term		����termios���ݽṹ
Return		:	�ɹ�������0 ʧ�ܣ����أ�-1
******************************************************************************/
sint32 set_termios(uint32 serial_fd, struct termios *old_term)
{
	sint32 ret = 0;
	ret = tcsetattr(serial_fd, TCSAFLUSH, old_term);
	if (ret < 0)
	{
		return SERIAL_ERR;
	}
	return SERIAL_SUCC;
}

/******************************************************************************
Function	:	set_serial_attr
Description	:	���ô�������
Input		:	serial_fd		�����ļ�������
				serial_attr		�����������ݽṹ
Return		:	�ɹ�������0 ʧ�ܣ����أ�-1
******************************************************************************/
sint32 set_serial_attr(uint32 serial_fd, serial_attr_t *serial_attr)
{
	uint32 clocal = 1;
	struct termios tio;

	if (serial_attr == NULL)
	{
		return SERIAL_ERR;
	}
	
	memset(&tio, 0, sizeof(tio));
	
	tio.c_cflag = CREAD | HUPCL | baud2flag(serial_attr->rate);	
	tio.c_cflag |= CBAUDEX;
	if (clocal)
	{
		tio.c_cflag |= CLOCAL;
	}
	switch (serial_attr->parity)
	{
	case 1:
		{
			tio.c_cflag |= PARENB | PARODD;
		}
		break;
	case 2:
		{
			tio.c_cflag |= PARENB;
		}
	default:
		break;
	}
	switch (serial_attr->databits)
	{
	case 5:
		{
			tio.c_cflag |= CS5;
		}
		break;
	case 6:
		{
			tio.c_cflag |= CS6;
		}
		break;
	case 7:
		{
			tio.c_cflag |= CS7;
		}
		break;
	default:
		{
			tio.c_cflag |= CS8;
		}
		break;
	}

	if (serial_attr->streamcontrol == 2)
	{
		tio.c_iflag |= IXON | IXOFF;
	}
	if (serial_attr->streamcontrol == 1)
	{
		tio.c_cflag |= CRTSCTS;
	}

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;
	if (tcsetattr(serial_fd, TCSANOW, &tio) < 0)
	{
		return SERIAL_ERR;
	}
	return SERIAL_SUCC;
}

#endif

#if T_DESC("��ʱ��")

/* ���֧��16������ */
#define MAX_BS_TIMER_NUM              (16)

/* ��ʱ���ڵ����ݽṹ */
typedef struct tagBS_TIMER_NODE
{
    BS_TIMER_FUNC func;           /* Ҫ���õĺ��� */
    struct timeval next_time;     /* ��һ�����������ʱ�� */
    struct timeval interval;      /* �����ʱ���� */
    unsigned int state;           /* =0����������=1������������Ҫִ�� */
}BS_TIMER_NODE;

/* ��ʱ��ȫ�ֱ��� */
BS_TIMER_NODE g_bsp_timers[MAX_BS_TIMER_NUM];

int g_bs_timer_run_flag = 0;
pthread_t g_bs_timer_tid;
pthread_attr_t g_bs_timer_attr; // �̲߳���

/*************************************************
  �������� : bsp server timer ��ʼ��
  ������� : 
  ������� : 
  �������� : 
  ��    ע : ��
  ����/ʱ��: zhongwei.peng / 2017.12.20
*************************************************/
void bs_timer_init(void)
{
    int i;

    for ( i = 0; i < MAX_BS_TIMER_NUM; i++ )
        g_bsp_timers[i].state = 0;

    pthread_attr_init(&g_bs_timer_attr);
    pthread_attr_setinheritsched(&g_bs_timer_attr, PTHREAD_INHERIT_SCHED);
    pthread_attr_setstacksize(&g_bs_timer_attr, 262144); /* ��ջ��С256k */

    pthread_create(&g_bs_timer_tid, &g_bs_timer_attr, bs_timer_run, NULL);
}

/*************************************************
  �������� : bsp server timer ��Դ����
  ������� : 
  ������� : 
  �������� : 
  ��    ע : ��
  ����/ʱ��: zhongwei.peng / 2017.12.20
*************************************************/
void bs_timer_destroy(void)
{
    g_bs_timer_run_flag = 0;
}

/*************************************************
  �������� : bsp server timer ����ע��
  ������� : fun -- �ص�����
             interval -- ʱ����(����)
  ������� : 
  �������� : �ɹ�����0������-1
  ��    ע : ��
  ����/ʱ��: zhongwei.peng / 2017.12.20
*************************************************/
int bs_timer_add(BS_TIMER_FUNC func, unsigned int interval)
{
    int i;
    unsigned int us;

    for ( i = 0; i < MAX_BS_TIMER_NUM; i++ )
    {
        if ( g_bsp_timers[i].state == 0 )
        {
            g_bsp_timers[i].interval.tv_sec = interval / 1000;
            g_bsp_timers[i].interval.tv_usec = ((long long)interval * 1000) % 1000000;

            gettimeofday(&(g_bsp_timers[i].next_time), NULL);
            us = g_bsp_timers[i].next_time.tv_usec + g_bsp_timers[i].interval.tv_usec;
            if ( us >= 1000000 )
            {
                g_bsp_timers[i].next_time.tv_sec += 1 + g_bsp_timers[i].interval.tv_sec;
                g_bsp_timers[i].next_time.tv_usec = us - 1000000;
            }
            else
            {
                g_bsp_timers[i].next_time.tv_sec += g_bsp_timers[i].interval.tv_sec;
                g_bsp_timers[i].next_time.tv_usec = us;
            }

            g_bsp_timers[i].func = func;
            g_bsp_timers[i].state = 1;
                
            return 0;
        }
    }

    return -1;
}

/*************************************************
  �������� : bsp server timer ��������
  ������� : fun -- �ص�����
  ������� : 
  �������� : 
  ��    ע : ��
  ����/ʱ��: zhongwei.peng / 2017.12.20
*************************************************/
void bs_timer_del(BS_TIMER_FUNC func)
{
    int i;

    for ( i = 0; i < MAX_BS_TIMER_NUM; i++ )
    {
        if ( g_bsp_timers[i].func == func )
        {
            g_bsp_timers[i].state = 0;
            return;
        }
    }
}

/*************************************************
  �������� : bsp server timer ��פ����
  ������� : 
  ������� : 
  �������� : �ɹ�����0������-1
  ��    ע : ��
  ����/ʱ��: zhongwei.peng / 2017.12.20
*************************************************/
void bs_timer_run(void)
{
    int i;
    unsigned int us;
    struct timeval curr_time;

    g_bs_timer_run_flag = 1;

    while ( g_bs_timer_run_flag )
    {
        usleep(100000);

        gettimeofday(&curr_time, NULL);

        for ( i = 0; i < MAX_BS_TIMER_NUM; i++ )
        {
            if ( g_bsp_timers[i].state )
            {
                if ( (g_bsp_timers[i].next_time.tv_sec < curr_time.tv_sec) 
                    || ((g_bsp_timers[i].next_time.tv_sec == curr_time.tv_sec) 
                        && (g_bsp_timers[i].next_time.tv_usec < curr_time.tv_usec)) )
                {
                    g_bsp_timers[i].func();

                    us = g_bsp_timers[i].next_time.tv_usec + g_bsp_timers[i].interval.tv_usec;
                    if ( us >= 1000000 )
                    {
                        g_bsp_timers[i].next_time.tv_sec += 1 + g_bsp_timers[i].interval.tv_sec;
                        g_bsp_timers[i].next_time.tv_usec = us - 1000000;
                    }
                    else
                    {
                        g_bsp_timers[i].next_time.tv_sec += g_bsp_timers[i].interval.tv_sec;
                        g_bsp_timers[i].next_time.tv_usec = us;
                    }
                }
            }
        }
    }
}

int bsp_print(int level, const char *fmt, ... ){
	char buf[1024];
	va_list argPtr;
	va_start(argPtr, fmt);
	vsnprintf(buf, 1024,fmt, argPtr);
	va_end(argPtr);
	switch(level){
		case 1<<0:
			zsys_error(buf);
			break;
		case 1<< 1:
			zsys_warning(buf);
			break;
		case 1<< 2:
			zsys_info(buf);
			break;
		case 1<< 3:
			zsys_debug(buf );
			break;
		case 1 << 4:
		case 1 << 5:
		case 1 << 6:
			zsys_notice(buf);
			break;
		default:
			break;
	}
	return 0;
}
#endif


