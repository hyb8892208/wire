/**********************************************************************************
Description : gsm模块初始化
Author      : zhongwei.peng@openvox.cn
Time        : 2017.03.03
Note        : 
***********************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <termios.h>
#include <errno.h>
//#include <pthread.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0

/* 单板控制mcu个数 */
#define BOARD_MCU_NUM               (2)

/* 定义模块类型 */
#define MODULE_TYPE_M35             (1)
#define MODULE_TYPE_SIM6320C        (2)

#define BUF_SIZE                    (1024)
#define MAX_PORT                    (128)   /* 系统最大端口数 */
#define DEV_NAME_LEN                (32)    /* usb串口设备名称长度 */

#define FRAME_FLAG                  (0x7E) /* 帧头帧尾标识 */
#define ESCAPE_WORD_0               (0x7F) /* 转义字符 */
#define ESCAPE_WORD_1               (0x7C) /* 同上 */
#define MOD_NUM                     (2)     /* 一个模块板有两个模块 */

/* 枚举管道的种类 */
typedef enum tagPIPE_TYPE_EN
{
    PTN_SND = 0, /* 音频 */
    PTN_AT,      /* AT指令 */
    PTN_CMD,     /* 与mcu收发命令 */
    PTN_NUM
}PIPE_TYPE_EN;

static const char mcu_module_map_path[] = "/tmp/mcu_module_map";
static const char board_mcu_dev_path[] = "/tmp/mcu_info";
static const char hw_version_path[] = "/tmp/hardware_version";

/* usb串口设备名称 */
char g_com_dev_names[MAX_PORT][DEV_NAME_LEN] = {0};
char g_brd_mcu_names[BOARD_MCU_NUM][DEV_NAME_LEN] = {0};

/* 模块类型 */
int g_com_dev_type[MAX_PORT] = {0};

/* 当前usb串口设备个数 */
int g_com_num = 0;

/* 当前单板控制mcu设备个数 */
int g_brd_mcu_num = 0;

int com_speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int com_name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,
	    38400,  19200,  9600, 4800, 2400, 1200,  300, };

void com_set_speed(int fd, int speed)
{
	int i;
	int status;
	struct termios Opt;
	tcgetattr(fd, &Opt);
	for ( i= 0; i < sizeof(com_speed_arr)/sizeof(int); i++) {
		if (speed == com_name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, com_speed_arr[i]);
			cfsetospeed(&Opt, com_speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0)
				perror("tcsetattr fd1");
			return;
		}
		tcflush(fd,TCIOFLUSH);
	}
}

int com_set_parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if ( tcgetattr( fd,&options)  !=  0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) {
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return (FALSE);
	}

	switch (parity) {
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 'S':
		case 's':
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
	}

	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
	}

	if (parity != 'n')
		options.c_iflag |= INPCK;

//	options.c_cc[VTIME] = 150; // 15 seconds
//	options.c_cc[VTIME] = 30;
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;

	options.c_cflag |= (CLOCAL|CREAD);
	options.c_cflag &= ~CRTSCTS;
	options.c_iflag  &= ~(IXON | IXOFF | IXANY);
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/

	//Ignore CR.
	//Map CR to NL on input.
	options.c_iflag &= ~(ICRNL|IGNCR);

	tcflush(fd,TCIFLUSH);
	if (tcsetattr(fd,TCSANOW,&options) != 0) {
		perror("SetupSerial 3");
		return (FALSE);
	}

	return (TRUE);

}

static int com_open(const char *devpath, int baud)
{
	int	fd = open(devpath, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd) {
        printf("Can't Open Serial Port (%s), %s(%d)\n",devpath, strerror(errno), errno);
		return -1;
	}

    com_set_speed(fd, baud);
    com_set_parity(fd, 8, 1, 'N');

	return fd;
}

/**********************************************************
函数描述 : 从模块串口读内容，自动去封装头尾
输入参数 : fd -- 串口文件句柄
输出参数 : buf -- 
           mod_id -- 模块id
返回值   : 成功返回>0代表读取到内容长度, 失败返回0
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
int com_module_read(int fd, unsigned char *buf, int *mod_id)
{
    int ret;
    int try_count;
    int ptn_id;
    int i, j;
    unsigned char buf_read[BUF_SIZE] = {0};

    try_count = 0;

    /* 最多等待2秒 */
    while ( ++try_count < 2000 )
    {
        usleep(1000);

        ret = read(fd, buf_read, BUF_SIZE - 1);
        if ( ret <= 0 )
            continue;
        else
            break;
    }

    if ( ret <= 0 )
    {
        printf("read time out!\n");
        return 0;
    }

    /* 校验 */
    if ( (buf_read[0] != FRAME_FLAG) || (buf_read[ret-1] != FRAME_FLAG) )
    {
        printf("frame error!\n");
        return -1;
    }

    *mod_id = (buf_read[1] >> 4) & 0x0F;
    ptn_id = buf_read[1] & 0x0F;

    if ( (*mod_id >= MOD_NUM) || (ptn_id != PTN_CMD) )
    {
        printf("mod_id or ptn_id invalid: 0x%02x, len[%d] \n", buf_read[1], ret);
        return -1;
    }

    ret -= 3;
    for ( i = 2, j = 0; j < ret; i++, j++ )
        buf[j] = buf_read[i];

    return ret;
}

/**********************************************************
函数描述 : 向模块串口写内容，自动封装头尾
输入参数 : fd -- 串口文件句柄
           buf -- 要写出的内容
           len -- 要写出的内容长度
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
void com_module_write(int fd, unsigned char *buf, int len, int mod_id)
{
    int i, j, ret;
    int len_w;
    unsigned char buf_write[BUF_SIZE];

    /* 先读空 */
    while ( (ret = read(fd, buf_write, BUF_SIZE)) > 0 ) 
    {
        buf_write[ret] = '\0';
        printf("RX: %s\n", buf_write);
    }

    for ( i = 2, j = 0; j < len; i++, j++ )
        buf_write[i] = buf[j];

    len_w = len + 3;
    buf_write[0] = FRAME_FLAG;
    buf_write[len_w-1] = FRAME_FLAG;
    buf_write[1] = (mod_id << 4) | PTN_CMD;

    (void)write(fd, buf_write, len_w);

    return;
}

/**********************************************************
函数描述 : 获取gsm模块的端口名称
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
void gsm_get_com_names(void)
{
    FILE *fp;
    int i, j;
    int ret;
    unsigned char *p;
    unsigned char buf[8192];

    fp = fopen(mcu_module_map_path, "r");
    if ( NULL == fp ) 
    {
        printf("open [%s] fail\n", mcu_module_map_path);
        return;
    }

    ret = (int)fread(buf, 1, sizeof(buf), fp);

    fclose(fp);

    if ( ret <= 0 )
    {
        printf("read file fail\n");
        return;
    }

    /* 获取当前端口数 */
    p = strstr(buf, "[mcu_module]");
    p = strstr(p, "sum=");
    if ( NULL == p )
    {
        printf("get sum fail!\n");
        return;
    }

    while ( *p && (*p++ != '=') );
    if ( '\0' == *p )
        return;

    while ( *p && (*p != '\n') )
    {
        g_com_num = g_com_num * 10 + (*p - '0');
        p++;
    }

    if ( (0 == g_com_num ) || (g_com_num >= MAX_PORT) )
    {
        printf("com sum = %d!\n", g_com_num);
        return;
    }

    /* 获取端口名字 */
    p = strstr(p, "dev-1");
    if ( NULL == p )
        return;

    printf("\n");

    for ( i = 0; i < g_com_num; i++ )
    {
        while ( *p && (*p++ != '=') );
        if ( '\0' == *p )
            return;

        j = 0;
        while ( *p && (*p != '\n') )
        {
            g_com_dev_names[i][j] = *p;
            j++;
            p++;
        }
        g_com_dev_names[i][j] = '\0';
        printf("dev-%d: %s\n", i+1, g_com_dev_names[i]);
    }

    return;
}

/**********************************************************
函数描述 : 所有模块开机
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.03.17
************************************************************/
void gsm_modules_on(void)
{
    int i;
    int fd;
    int ret;
    int mod_id;
    unsigned char buf[64];

    /* 模块开机 */
    for ( i = 0; i < g_com_num; i += 2 )
    {
        if ( (fd = com_open(g_com_dev_names[i], 115200)) < 0 )
        {
            printf("open [%s] fail 1\n", g_com_dev_names[i]);
            continue;
        }

        printf("power on %s\n", g_com_dev_names[i]);

        com_module_write(fd, "power on", 8, 0);
        com_module_write(fd, "power on", 8, 1);

        close(fd);
    }

    /* 开机要等3秒 */
    sleep(3);

    /* 模块开机结果查询 */
    for ( i = 0; i < g_com_num; i += 2 )
    {
        if ( (fd = com_open(g_com_dev_names[i], 115200)) < 0 )
        {
            printf("open [%s] fail 1\n", g_com_dev_names[i]);
            continue;
        }

        printf("power on %s result: ", g_com_dev_names[i]);

        if ( (ret = com_module_read(fd, buf, &mod_id)) > 0 )
        {
            buf[ret] = '\0';
            printf("%s\n", buf);
        }

        close(fd);
    }
}

/**********************************************************
函数描述 : 所有模块关机
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.03.17
************************************************************/
void gsm_modules_off(void)
{
    int i;
    int fd;
    int ret;
    int mod_id;
    unsigned char buf[64];

    /* 模块关机 */
    for ( i = 0; i < g_com_num; i += 2 )
    {
        if ( (fd = com_open(g_com_dev_names[i], 115200)) < 0 )
        {
            printf("open [%s] fail 1\n", g_com_dev_names[i]);
            continue;
        }

        printf("power off %s\n", g_com_dev_names[i]);

        com_module_write(fd, "power off", 9, 0);
        com_module_write(fd, "power off", 9, 1);

        close(fd);
    }

    /* 关机要等700毫秒 */
    usleep(700 * 1000);

    /* 模块关机结果查询 */
    for ( i = 0; i < g_com_num; i += 2 )
    {
        if ( (fd = com_open(g_com_dev_names[i], 115200)) < 0 )
        {
            printf("open [%s] fail 1\n", g_com_dev_names[i]);
            continue;
        }

        printf("power off %s result: ", g_com_dev_names[i]);

        if ( (ret = com_module_read(fd, buf, &mod_id)) > 0 )
        {
            buf[ret] = '\0';
            printf("%s\n", buf);
        }
        else
        {
            printf("\n", buf);
        }

        close(fd);
    }
}

/**********************************************************
函数描述 : 从单板控制串口读内容
输入参数 : fd -- 串口文件句柄
输出参数 : buf -- 
返回值   : 成功返回>0代表读取到内容长度, 失败返回0
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
int brd_mcu_read(int fd, unsigned char *buf, int buf_len)
{
    int ret;
    int try_count;

    try_count = 0;
    memset(buf, 0, buf_len);

    /* 最多等待2秒 */
    while ( ++try_count < 2000 )
    {
        usleep(1000);

        ret = read(fd, buf, buf_len - 1);
        if ( ret <= 0 )
            continue;
        else
            break;
    }

    if ( ret <= 0 )
        return 0;

    return ret;
}

/**********************************************************
函数描述 : 向单板控制串口写内容
输入参数 : fd -- 串口文件句柄
           buf -- 要写出的内容
           len -- 要写出的内容长度
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
void brd_mcu_write(int fd, unsigned char *buf, int len)
{
    unsigned char buf_write[BUF_SIZE];

    /* 先读空 */
    while ( read(fd, buf_write, BUF_SIZE) > 0 ) ;

    (void)write(fd, buf, len);

    return;
}

/**********************************************************
函数描述 : 获取单板控制mcu的设备名称
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
void brd_mcu_get_names(void)
{
    FILE *fp;
    int i, j;
    int ret;
    unsigned char *p;
    unsigned char buf[8192];

    fp = fopen(board_mcu_dev_path, "r");
    if ( NULL == fp ) 
    {
        printf("open [%s] fail\n", board_mcu_dev_path);
        return;
    }

    ret = (int)fread(buf, 1, sizeof(buf), fp);

    fclose(fp);

    if ( ret <= 0 )
    {
        printf("read [%s] fail\n", board_mcu_dev_path);
        return;
    }

    /* 获取端口名字 */
    p = strstr(buf, "dev=");
    if ( NULL == p )
        return;

    printf("\n");

    for ( i = 0; i < BOARD_MCU_NUM; i++ )
    {
        while ( *p && (*p++ != '=') );
        if ( '\0' == *p )
            return;

        j = 0;
        while ( *p && (*p != '\n') )
        {
            g_brd_mcu_names[i][j] = *p;
            j++;
            p++;
        }
        g_brd_mcu_names[i][j] = '\0';
        printf("dev-%d: %s\n", i+1, g_brd_mcu_names[i]);
        g_brd_mcu_num++;
    }

    return;
}

/**********************************************************
函数描述 : 初始化单板硬件版本
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
void brd_mcu_init_hw_version(void)
{
    int i, j;
    int fd;
    int ret;
    unsigned char *p;
    unsigned char buf[BUF_SIZE];
    unsigned char ver[BUF_SIZE];
    FILE *fp;

    j = 0;

    for ( i = 0; i < g_brd_mcu_num; i++ )
    {
        if ( (fd = com_open(g_brd_mcu_names[i], 9600)) < 0 )
        {
            printf("open [%s] fail 6\n", g_brd_mcu_names[i]);
            continue;
        }

        brd_mcu_write(fd, "ver\n", 4);

        if ( (ret = brd_mcu_read(fd, buf, BUF_SIZE)) <= 0 )
        {
            printf("read version fail!\n");
            close(fd);
            continue;
        }
        close(fd);

        p = strstr(buf, "HwVer :");
        if ( p )
        {
            p += 8;

            j = 0;
            while ( *p && (*p != '\n') )
            {
                ver[j] = *p;
                j++;
                p++;
            }
            ver[j] = '\0';
            printf("hw version: %s\n", ver);
            break;
        }
    }

    if ( 0 == j )
        return;

    if ( NULL == (fp = fopen(hw_version_path, "w")) ) 
    {
        printf("open file %s error", hw_version_path);
        return; 
    }
    fwrite(ver, 1, j, fp);
    fclose(fp);
}

/**********************************************************
函数描述 : 使能sim 卡
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
void brd_mcu_sim_enable(void)
{
    int i;
    int fd;

    for ( i = 0; i < g_brd_mcu_num; i++ )
    {
        if ( (fd = com_open(g_brd_mcu_names[i], 9600)) < 0 )
        {
            printf("open [%s] fail 7\n", g_brd_mcu_names[i]);
            continue;
        }

        brd_mcu_write(fd, "write 0=ffh\n", 12);
        sleep(1);
        brd_mcu_write(fd, "write 1=ffh\n", 12);
        close(fd);
    }
}

int main(int argc, char *argv[])
{
    if ( argc == 2 )
    {
        if ( strcmp(argv[1], "module_on") == 0 )
        {
            gsm_get_com_names();
            gsm_modules_on();
        }
        else if ( strcmp(argv[1], "module_off") == 0 )
        {
            gsm_get_com_names();
            gsm_modules_off();
        }
    }
    else
    {
        brd_mcu_get_names();
        brd_mcu_init_hw_version();
        brd_mcu_sim_enable();
    }

    return 0;
}





