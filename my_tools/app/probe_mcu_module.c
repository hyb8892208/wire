/**********************************************************************************
Description : 这个程序用来探测模块板上mcu的usb串口
Author      : zhongwei.peng@openvox.cn
Time        : 2016.12.07
Note        : 
***********************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
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

#define TRUE 1
#define FALSE 0

#define MODULE_NAME_M35        "Quectel_M35"
#define MODULE_NAME_SIM6320C   "SIMCOM_SIM6320C"
#define MODULE_NAME_NULL       "null"  /* 没有模块时显示的名字 */

#define FRAME_FLAG                  (0x7E) /* 帧头帧尾标识 */
#define ESCAPE_WORD_0               (0x7F) /* 转义字符 */
#define ESCAPE_WORD_1               (0x7C) /* 同上 */
#define MOD_NUM                     (2)     /* 一个模块板有两个模块 */

#define BUF_SIZE               (1024)
#define USB_NODE_MAX           (256)
#define USB_NODE_CHAN_MAX      (32)
#define USB_NODE_LCD           (33)
#define MY_PATH_MAX            (512)
#define MAP_INFO_MAX           (8192)
#define NAME_LEN               (64)
#define PORT_STR_MAX           (256)
#define INVALID_VALUE          (0xFFFF)

/* 枚举管道的种类 */
typedef enum tagPIPE_TYPE_EN
{
    PTN_SND = 0, /* 音频 */
    PTN_AT,      /* AT指令 */
    PTN_CMD,     /* 与mcu收发命令 */
    PTN_NUM
}PIPE_TYPE_EN;

/* usb设备在hub中的地址(枚举地址)与设备名称之间的对应关系数据结构 */
typedef struct tagST_NODE_TO_ACM_NUM
{
    char node_name[NAME_LEN];     /* usb node */
    unsigned int ttyACM_num;      /* /dev/ttyACMx */
    char module_type[NAME_LEN];   /* 模块类型 */
}ST_NODE_TO_ACM_NUM;

/* 枚举地址与设备名称对应关系数组，数组的下标代表物理端口号 */
ST_NODE_TO_ACM_NUM arr_node_dev_info[USB_NODE_MAX] = 
{ 
    {"", INVALID_VALUE, ""}, /* no use, because port start from 1 */
    {"1-1.2.3.1.1.3", INVALID_VALUE, ""}, /* 1: bottom board, first left slot */
	{"1-1.2.3.1.1.3", INVALID_VALUE, ""}, 
    {"1-1.2.3.1.2.2", INVALID_VALUE, ""}, 
	{"1-1.2.3.1.2.2", INVALID_VALUE, ""}, 
    {"1-1.2.3.2.1.1", INVALID_VALUE, ""}, 
	{"1-1.2.3.2.1.1", INVALID_VALUE, ""}, 
    {"1-1.2.3.2.1.4", INVALID_VALUE, ""}, 
	{"1-1.2.3.2.1.4", INVALID_VALUE, ""}, 
    {"1-1.2.3.1.4.3", INVALID_VALUE, ""}, 
	{"1-1.2.3.1.4.3", INVALID_VALUE, ""}, 
    {"1-1.2.3.1.3.3", INVALID_VALUE, ""}, 
	{"1-1.2.3.1.3.3",INVALID_VALUE, ""}, 
    {"1-1.2.3.3.2", INVALID_VALUE, ""},
	{"1-1.2.3.3.2", INVALID_VALUE, ""},
    {"1-1.2.3.4.1", INVALID_VALUE, ""},
	{"1-1.2.3.4.1", INVALID_VALUE, ""},
	{"1-1.2.4.1.1.3", INVALID_VALUE, ""}, /* 17: top board, first left slot */
    {"1-1.2.4.1.1.3", INVALID_VALUE, ""}, 
    {"1-1.2.4.1.2.2", INVALID_VALUE, ""}, 
    {"1-1.2.4.1.2.2",INVALID_VALUE, ""}, 
    {"1-1.2.4.2.1.1", INVALID_VALUE, ""}, 
    {"1-1.2.4.2.1.1", INVALID_VALUE, ""}, 
    {"1-1.2.4.2.1.4", INVALID_VALUE, ""}, 
    {"1-1.2.4.2.1.4", INVALID_VALUE, ""},
    {"1-1.2.4.1.4.3", INVALID_VALUE, ""}, 
    {"1-1.2.4.1.4.3", INVALID_VALUE, ""},
    {"1-1.2.4.1.3.3", INVALID_VALUE, ""}, 
    {"1-1.2.4.1.3.3", INVALID_VALUE, ""}, 
    {"1-1.2.4.3.2", INVALID_VALUE, ""}, 
    {"1-1.2.4.3.2", INVALID_VALUE, ""}, 
    {"1-1.2.4.4.1", INVALID_VALUE, ""}, 
    {"1-1.2.4.4.1", INVALID_VALUE, ""},   /* 32 */
	{"1-1.2.3.4.4", INVALID_VALUE, ""},    /* 33, LCD ctrl mcu */
	{"1-1.2.3.4.4", INVALID_VALUE, ""},    /* 34, LCD ctrl mcu */
    {"", 0, ""}, 
    {"", 0, ""}
};

/* loop this fold to find usb sound card */
static const char sys_bus_usb_devices[] = "/sys/bus/usb/devices";

/* record map info */
static const char mcu_module_map_path[] = "/tmp/mcu_module_map";

/* record module type */
static const char module_type_path[] = "/tmp/.module_type";

/* 端口名称 */
static char port_str[PORT_STR_MAX];
static unsigned int port_str_len;

/* 所有映射信息缓存(字符串) */
static char ttyACM_map_info[MAP_INFO_MAX];
static unsigned long map_info_len;

/* 所有映射信息缓存(字符串) */
static char hwport_map_info[MAP_INFO_MAX];
static unsigned long hwport_map_info_len;

int com_speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int com_name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,
	    38400,  19200,  9600, 4800, 2400, 1200,  300, };
#if 0
/**********************************************************
函数描述 : 数据以16进制形式导出，用于调试
输入参数 : data -- 数据内容
           len -- 数据内容长度
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
void dump_data(unsigned char *data, int len)
{
    int i;

    for ( i = 0; i < len; i++ )
    {
        if ( i % 8 == 0 )
            printf("\n");

        printf("%02x ", data[i]);
    }

    printf("\n");
}
#endif
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
    int ptn_id = PTN_NUM;
    unsigned char *p;
    unsigned char *p_buf;
    unsigned char *p_end;
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

    //printf("\n dump read data:");
    //dump_data(buf_read, ret);

    p = buf_read;
    p_end = buf_read + ret;
    while ( p < p_end )
    {
        if ( *p == FRAME_FLAG )
        {
            p++;
            *mod_id = (*p >> 4) & 0x0F;
            ptn_id = *p & 0x0F;
            if ( (*mod_id < MOD_NUM) && (ptn_id == PTN_CMD) )
                break;
            else
                continue;
        }
        p++;
    }

    if ( ptn_id != PTN_CMD )
        return -1;

    p++;
    p_buf = buf;

    while ( *p && (*p != FRAME_FLAG) )
        *p_buf++ = *p++;

    return 0;
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

    //printf("\n dump write data:");
    //dump_data(buf_write, len_w);

    (void)write(fd, buf_write, len_w);

    return;
}

/**************************************************
Description : inspect dir find all the ttyACM dirs
Input       : d_name -- dir name
Output      : 
Author      : zhongwei.peng
Time        : 2016.07.04
Note        : 
***************************************************/
static void inspect_bus_entry(const char *d_name)
{
    unsigned int i = 0;
    unsigned int l = 0;
    unsigned int card_id = 0;
    char dir_path[MY_PATH_MAX];
    char devname[MY_PATH_MAX];
    char nodename[MY_PATH_MAX + 1];
    DIR *sbud = NULL;
    
	if (d_name[0] == '.' && (!d_name[1] || (d_name[1] == '.' && !d_name[2])))
		return;

	if ( isdigit(d_name[0]) && strchr(d_name, ':') )
    {
        /* check if the tty dir exist, or return */
        snprintf(dir_path, MY_PATH_MAX, "%s/%s/tty", sys_bus_usb_devices, d_name);
        if ( sbud = opendir(dir_path) ) 
        {
            closedir(sbud);

            for ( i = 0; i < USB_NODE_MAX; i++ ) 
            {
                snprintf(dir_path, MY_PATH_MAX, "%s/%s/tty/ttyACM%d", sys_bus_usb_devices, d_name, i);
                if ( sbud = opendir(dir_path) ) 
                {
                    closedir(sbud);
                    card_id = i;
                    break;
                }
            }
            if ( i >= USB_NODE_MAX )
            {
                return;
            }
        }
        else
        {
            return;
        }
        
        /* if ttyACM%d exist, get usb node name */
        memset(nodename, 0, MY_PATH_MAX + 1);

        for ( i = 0; i < MY_PATH_MAX; i++ )
        {
            if ( ':' == d_name[i] )
            {
                break;
            }
            nodename[i] = d_name[i];
        }

        if ( i >= MY_PATH_MAX )
        {
            return;
        }

        /* fill the card id from the nodename */
        for ( i = 1; i < USB_NODE_MAX; i++ )
        {
            if ( strlen(arr_node_dev_info[i].node_name) == 0 )
            {
                break;
            }

            if ( strcmp(nodename, arr_node_dev_info[i].node_name) == 0 )
            {
                arr_node_dev_info[i].ttyACM_num   = card_id;
                arr_node_dev_info[i+1].ttyACM_num = card_id; /* 2 ports use the same chip */
                break;
            }
        }
    }
}

/**************************************************
Description : 从文件中读取模块类型
Input       : 
Output      : 
Return      : 读取成功返回0， 失败返回<0
Author      : zhongwei.peng
Time        : 2016.07.04
Note        : 
***************************************************/
int read_module_type(void)
{
    int i;
    int len_m35 = 0;
    int len_sim6320 = 0;
    int len_null = 0;
    FILE *fp_mtype;
    unsigned char buf[2048];
    unsigned char *p;

    fp_mtype = fopen(module_type_path, "r");
    if ( fp_mtype == NULL )
        return -1;

    printf("file %s exist\n", module_type_path);

    memset(buf, 0, 2048);
    if ( fread(buf, 1, 2047, fp_mtype) <= 0 )
    {
        printf("read file %s fail\n", module_type_path);
        fclose(fp_mtype);
        return -1;
    }

    len_m35 = strlen(MODULE_NAME_M35);
    len_sim6320 = strlen(MODULE_NAME_SIM6320C);
    len_null = strlen(MODULE_NAME_NULL);

    p = buf;
    i = 1;
    while ( 1 )
    {
        if ( strncmp(MODULE_NAME_M35, p, len_m35) == 0 )
        {
            strcpy(arr_node_dev_info[i].module_type, MODULE_NAME_M35);
            strcpy(arr_node_dev_info[i+1].module_type, MODULE_NAME_M35);
            p += len_m35;
            i += 2;
        }
        else if ( strncmp(MODULE_NAME_SIM6320C, p, len_sim6320) == 0 )
        {
            strcpy(arr_node_dev_info[i].module_type, MODULE_NAME_SIM6320C);
            strcpy(arr_node_dev_info[i+1].module_type, MODULE_NAME_SIM6320C);
            p += len_sim6320;
            i += 2;
        }
        else if ( strncmp(MODULE_NAME_NULL, p, len_null) == 0 )
        {
            p += len_null;
            i += 2;
        }
        else
        {
            printf("unknown module type:\n %s\n", buf);
            fclose(fp_mtype);
            return -1;
        }

        if ( (*p == 0) || (*p == '\n') || (*p == '\r') )
            break;
        else if ( *p == ',' )
            p++;
        else
        {
            printf("content error:\n %s\n", p);
            return -1;
        }
    }

    fclose(fp_mtype);
    printf("read from file %s complete!\n", module_type_path);
    
    return 0;
}

/**************************************************
Description : 模块类型写到文件去
Input       : 
Output      : 
Return      : 
Author      : zhongwei.peng
Time        : 2016.07.04
Note        : 
***************************************************/
void write_module_type(void)
{
    unsigned long i = 0;
    char buf_mtype[PORT_STR_MAX] = {0};
    FILE *fp_mtype;

    fp_mtype = fopen(module_type_path, "w+");
    if ( fp_mtype == NULL )
    {
        printf("%s: open file %s fail!\n", __FUNCTION__, module_type_path);
        return;
    }

    /* hwport=module_type */
    memset(buf_mtype, 0, sizeof(buf_mtype));
    for ( i = 1; i <= USB_NODE_CHAN_MAX; i++ )
    {
        if ( arr_node_dev_info[i].ttyACM_num != INVALID_VALUE )
        {
            if ( strlen(buf_mtype) )
                sprintf(buf_mtype, ",%s", arr_node_dev_info[i].module_type);
            else
                sprintf(buf_mtype, "%s", arr_node_dev_info[i].module_type);
        }
        else
        {
            if ( strlen(buf_mtype) )
                sprintf(buf_mtype, ",%s", MODULE_NAME_NULL);
            else
                sprintf(buf_mtype, "%s", MODULE_NAME_NULL);
        }

        fwrite(buf_mtype, 1, strlen(buf_mtype), fp_mtype);
    }

    fwrite("\n", 1, 1, fp_mtype);

    fclose(fp_mtype);
}

/**************************************************
Description : 检查模块类型
Input       : 
Output      : 
Author      : zhongwei.peng
Time        : 2016.07.04
Note        : 
***************************************************/
void check_module_type(void)
{
    //unsigned char frame_get_ver[6] = {0x7E, 0x02, 'v', 'e', 'r', 0x7E};
    unsigned char buf[1024];
    int fds[USB_NODE_MAX] = {0};
    int i;
    int mod_id;

    printf("check module type start...\n");

    /* 打开所有串口并发送查询版本命令 */
    for ( i = 1; i <= USB_NODE_CHAN_MAX; i += 2 )
    {
        if ( arr_node_dev_info[i].ttyACM_num != INVALID_VALUE )
        {
            sprintf(buf, "/dev/ttyACM%d", arr_node_dev_info[i].ttyACM_num);
            fds[i] = com_open(buf, 115200);
            if ( fds[i] <= 0 )
            {
                printf("open %s fail\n", buf);
                continue;
            }
            usleep(100 * 1000);
            com_module_write(fds[i], "ver", 3, 0);
        }
    }

    /* 等1秒 */
    sleep(1);

    /* 读取所有模块类型并关闭所有串口 */
    /* 打开所有串口并发送查询版本命令 */
    for ( i = 1; i <= USB_NODE_CHAN_MAX; i += 2 )
    {
        if ( arr_node_dev_info[i].ttyACM_num != INVALID_VALUE )
        {
            if ( fds[i] <= 0 )
            {
                strcpy(arr_node_dev_info[i].module_type, MODULE_NAME_M35);
                strcpy(arr_node_dev_info[i+1].module_type, MODULE_NAME_M35);
                continue;
            }

            memset(buf, 0, sizeof(buf));
            (void)com_module_read(fds[i], buf, &mod_id);
            printf("\n%d:\n%s\n", i, buf);

            if ( strstr(buf, MODULE_NAME_SIM6320C) )
            {
                strcpy(arr_node_dev_info[i].module_type, MODULE_NAME_SIM6320C);
                strcpy(arr_node_dev_info[i+1].module_type, MODULE_NAME_SIM6320C);
            }
            else
            {
                strcpy(arr_node_dev_info[i].module_type, MODULE_NAME_M35);
                strcpy(arr_node_dev_info[i+1].module_type, MODULE_NAME_M35);
            }

            close(fds[i]);
        }
    }

    printf("check module type complete!\n");
}

/**************************************************
Description : build "/tmp/mcu_module_map" 
Input       : 
Output      : 
Author      : zhongwei.peng
Time        : 2016.07.04
Note        : 生成以下内容

[mcu_module]
sum=6
port=17,18,21,22,23,24
dev-1=/dev/ttyACM2
dev-2=/dev/ttyACM2
dev-3=/dev/ttyACM0
dev-4=/dev/ttyACM0
dev-5=/dev/ttyACM1
dev-6=/dev/ttyACM1
***************************************************/
#if 1
void rebuild_mcu_module_map(void)
{
    unsigned long i = 0;
    unsigned long port_sum = 0;
    char buf[PORT_STR_MAX] = {0};
    int ret = 0;
	int dev_num;
    FILE *fp;
	
	dev_num = 1;

    memset(port_str, 0, PORT_STR_MAX);
    port_str_len = 0;
	
	memset(ttyACM_map_info, 0, MAP_INFO_MAX);
	map_info_len = 0;
	
	memset(hwport_map_info, 0, MAP_INFO_MAX);
	hwport_map_info_len = 0;

    /* 计算port_sum */
    for ( i = 1; i <= USB_NODE_CHAN_MAX; i += 2 )
    {
        if ( strlen(arr_node_dev_info[i].node_name) == 0 )
            break;

        if ( arr_node_dev_info[i].ttyACM_num == INVALID_VALUE )
            continue;
        else
            port_sum = i + 1;
    }

    if ( port_sum > 16 )
        port_sum = 32;
    else
        port_sum = 16;

    /* dev-1=/dev/ttyACM2 */
    for ( i = 1; i <= port_sum; i += 2, dev_num += 2 )
    {
        printf("arr_node_dev_info[%d].node_name: %s\n", i, arr_node_dev_info[i].node_name);
        printf("arr_node_dev_info[%d].ttyACM_num: %d \n", i, arr_node_dev_info[i].ttyACM_num);

        /* 生成port=x,y... */
        if ( i == 1 )
            ret = snprintf(port_str, PORT_STR_MAX, "port=%d,%d", i, i+1);
        else
            ret = snprintf(&port_str[port_str_len], PORT_STR_MAX - port_str_len, ",%d,%d", i, i+1);

        port_str_len += ret;

        /* dev-x=hwport */
		ret = snprintf(&hwport_map_info[hwport_map_info_len], MAP_INFO_MAX - hwport_map_info_len, 
				"dev-%d=%d\ndev-%d=%d\n", dev_num, i, dev_num+1, i+1);

        hwport_map_info_len += ret;

		/* dev-x=ttyACMx */
		ret = snprintf(&ttyACM_map_info[map_info_len], MAP_INFO_MAX - map_info_len, 
				"dev-%d=/dev/ttyACM%d\ndev-%d=/dev/ttyACM%d\n", 
				dev_num, arr_node_dev_info[i].ttyACM_num, dev_num+1, arr_node_dev_info[i+1].ttyACM_num);

        map_info_len += ret;
    }

    fp = fopen(mcu_module_map_path, "w+");
    if ( fp != NULL ) 
    {
		/* mcu_module */
        sprintf(buf, "[mcu_module]\nsum=%d\n", port_sum);
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);

		port_str[port_str_len++] = '\n';
        port_str[port_str_len] = '\0';
        fwrite(port_str, 1, port_str_len, fp);
        printf("%s", port_str);
		
		fwrite(ttyACM_map_info, 1, map_info_len, fp);
        printf("%s", ttyACM_map_info);
		
		/* hwport_map */
		sprintf(buf, "\n[hwport_map]\n");
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);
		
		fwrite(hwport_map_info, 1, hwport_map_info_len, fp);
        printf("%s", hwport_map_info);
		
		/* lcd */
		sprintf(buf, "\n[lcd]\n");
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);
		
		sprintf(buf, "lcd_mcu=/dev/ttyACM%d\n", arr_node_dev_info[USB_NODE_LCD].ttyACM_num);
		fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);

        /* module type */
        sprintf(buf, "\n[module_type]\n");
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);

        /* hwport=module_type */
        for ( i = 1; i <= USB_NODE_CHAN_MAX; i++ )
        {
            if ( arr_node_dev_info[i].ttyACM_num != INVALID_VALUE )
            {
                sprintf(buf, "%d=%s\n", i, arr_node_dev_info[i].module_type);
                fwrite(buf, 1, strlen(buf), fp);
                printf("%s", buf);
            }
        }

		fclose(fp);
	}
    else
    {
        printf("open %s or %s fail!\n", mcu_module_map_path, module_type_path);
    }
}
#else
void rebuild_mcu_module_map(void)
{
    unsigned long i = 0;
    unsigned long port_sum = 0;
    char buf[PORT_STR_MAX] = {0};
    int ret = 0;
	int dev_num;
    FILE *fp;
	
	dev_num = 1;

    memset(port_str, 0, PORT_STR_MAX);
    port_str_len = 0;
	
	memset(ttyACM_map_info, 0, MAP_INFO_MAX);
	map_info_len = 0;
	
	memset(hwport_map_info, 0, MAP_INFO_MAX);
	hwport_map_info_len = 0;

    for ( i = 1; i <= USB_NODE_CHAN_MAX; i += 2 )
    {
        if ( strlen(arr_node_dev_info[i].node_name) == 0 )
        {
            break;
        }

        if ( arr_node_dev_info[i].ttyACM_num != INVALID_VALUE )
        {
            printf("arr_node_dev_info[%d].node_name: %s\n", i, arr_node_dev_info[i].node_name);
            printf("arr_node_dev_info[%d].ttyACM_num: %d \n", i, arr_node_dev_info[i].ttyACM_num);

            /* /tmp/mcu_module_map port=x,y,z... */
            if ( port_str_len == 0 )
                ret = snprintf(port_str, PORT_STR_MAX, "port=%d,%d", i, i+1);
            else
                ret = snprintf(&port_str[port_str_len], PORT_STR_MAX - port_str_len, ",%d,%d", i, i+1);

            if ( ret > 0 )
            {
                port_str_len += ret;

                if ( port_str_len >= PORT_STR_MAX )
                {
                    printf("error: port_str_len[%d] exceed max[%d]\n", 
                    port_str_len, PORT_STR_MAX);
                    return;
                }
            }
            else
            {
                printf("error: create port str fail! ret = %d\n", ret);
                return;
            }

			/* dev-x=ttyACMx */
			ret = snprintf(&ttyACM_map_info[map_info_len], MAP_INFO_MAX - map_info_len, 
					"dev-%d=/dev/ttyACM%d\ndev-%d=/dev/ttyACM%d\n", 
					dev_num, arr_node_dev_info[i].ttyACM_num, dev_num+1, arr_node_dev_info[i+1].ttyACM_num);

			if ( ret > 0 )
            {
                map_info_len += ret;

                if ( map_info_len >= MAP_INFO_MAX )
                {
                    printf("error: map_info_len[%d] exceed max[%d]\n", 
                    map_info_len, MAP_INFO_MAX);
                    return;
                }
            }
            else
            {
                printf("error: create map str fail! ret = %d\n", ret);
                return;
            }
			
			/* dev-x=hwport */
			ret = snprintf(&hwport_map_info[hwport_map_info_len], MAP_INFO_MAX - hwport_map_info_len, 
					"dev-%d=%d\ndev-%d=%d\n", dev_num, i, dev_num+1, i+1);

			if ( ret > 0 )
            {
                hwport_map_info_len += ret;

                if ( hwport_map_info_len >= MAP_INFO_MAX )
                {
                    printf("error: map_info_len[%d] exceed max[%d]\n", 
                    hwport_map_info_len, MAP_INFO_MAX);
                    return;
                }
            }
            else
            {
                printf("error: create hwport_map str fail! ret = %d\n", ret);
                return;
            }
			
			dev_num += 2;
            port_sum += 2;
        }
    }

    fp = fopen(mcu_module_map_path, "w+");
    if ( fp != NULL ) 
    {
		/* mcu_module */
        sprintf(buf, "[mcu_module]\nsum=%d\n", port_sum);
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);

		port_str[port_str_len++] = '\n';
        port_str[port_str_len] = '\0';
        fwrite(port_str, 1, port_str_len, fp);
        printf("%s", port_str);
		
		fwrite(ttyACM_map_info, 1, map_info_len, fp);
        printf("%s", ttyACM_map_info);
		
		/* hwport_map */
		sprintf(buf, "\n[hwport_map]\n");
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);
		
		fwrite(hwport_map_info, 1, hwport_map_info_len, fp);
        printf("%s", hwport_map_info);
		
		/* lcd */
		sprintf(buf, "\n[lcd]\n");
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);
		
		sprintf(buf, "lcd_mcu=/dev/ttyACM%d\n", arr_node_dev_info[USB_NODE_LCD].ttyACM_num);
		fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);

        /* module type */
        sprintf(buf, "\n[module_type]\n");
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);

        /* hwport=module_type */
        for ( i = 1; i <= USB_NODE_CHAN_MAX; i++ )
        {
            if ( arr_node_dev_info[i].ttyACM_num != INVALID_VALUE )
            {
                sprintf(buf, "%d=%s\n", i, arr_node_dev_info[i].module_type);
                fwrite(buf, 1, strlen(buf), fp);
                printf("%s", buf);
            }
        }

		fclose(fp);
	}
    else
    {
        printf("open %s or %s fail!\n", mcu_module_map_path, module_type_path);
    }
}

#endif

int main(int argc,char *argv[])
{
    unsigned long i = 0;
    struct dirent *de;
    DIR *sbud = NULL;

    sbud = opendir(sys_bus_usb_devices);
	if (sbud) 
    {
    	while ( (de = readdir(sbud)) )
    		inspect_bus_entry(de->d_name);

        closedir(sbud);
    }

    if ( read_module_type() < 0 )
    {
        check_module_type();
        write_module_type();
    }

    rebuild_mcu_module_map();

    return 0;
}

