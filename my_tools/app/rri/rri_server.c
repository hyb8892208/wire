/**********************************************************************************
Description : gsm模块接口程序，在gsm模块(硬件)与asterisk之间透传数据
Author      : zhongwei.peng@openvox.cn
Time        : 2016.11.24(thanksgiving day)
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
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include "czmq.h"

#define POSIX_TERMIOS
#define USB_COM_BAUD             "460800"//"115200" /* USB串口波特率 */

#define FRAME_FLAG_HDLC              (0x7E) /* hdlc帧头帧尾标识 */
#define ESCAPE_WORD_0           (0x7F) /* 转义字符 */
#define ESCAPE_WORD_1           (0x7C) /* 同上 */

#define FRAME_FLAG_COBS         (0x00) /*cobs转义字符*/

#define MODULE_PIPE_DIR          "/tmp/module_pipe/"    /* module interface pipe文件存放目录 */
#define FILE_PIPE_WRITE          "-w.pipe"              /* 写pipe文件名后缀 */
#define FILE_PIPE_READ           "-r.pipe"              /* 读pipe文件名后缀 */
#define FILE_DEBUG_READ          "-r.pcm"               /* 调试文件，存放读数据 */
#define FILE_DEBUG_WRITE         "-w.pcm"               /* 调试文件，存放写数据 */
#define FILE_DEBUG_AT            "-at.log"              /* 调试文件，存放at指令 */
#define FILE_DEBUG_CMD           "-cmd.log"             /* 调试文件，存放命令 */

#define SERVER_DEBUG_FILE        "module_interface.log" /* 调试文件，当前程序log */
#define MODULE_MAP_FILE          "/tmp/hw_info.cfg"  /* 模块映射文件 */

#define LOG_LEVEL_FILE              "/etc/asterisk/gw.conf"

#define NAME_LEN                 (32)
#define BUF_SIZE                 (2048)
#define HW_PORT_MAX              (64)    /* 最大物理端口号个数(实际上目前最多32个) */
#define MOD_NUM                  (2)     /* 一个模块板有两个模块 */
#define UPGRAD_BUF_SIZE   (48)
/* 每次上传的alaw格式的语音数据是64，传给asterisk是160 */
#define RX_SND_BUF_SIZE          (160)
#define RX_AT_BUF_SIZE           (1024)

/* 收发数据长度定义 */
#define TX_SND_MAX               (320)  /* alaw 160 */

int ctrl_thr_flag = 0;
/* 枚举poll fd */
typedef enum tagPIPE_FD_EN
{
    PIFD_R = 0, /* read */
    PIFD_W,     /* write */
    PIFD_P,     /* pseudo */
    PIFD_NUM
}PIPE_FD_EN;

/* 枚举管道的种类 */
typedef enum tagPIPE_TYPE_EN
{
    PTN_SND = 0, /* 音频 */
    PTN_AT,      /* AT指令 */
    PTN_CMD,     /* 与mcu收发命令 */
    PTN_NUM
}PIPE_TYPE_EN;

/* 管道控制数据结构 */
typedef struct tagPIPE_CTRL_ST
{
    char name[NAME_LEN];           /* pipe文件名前面部分，后缀有另外的定义 */
    int fds[PIFD_NUM];             /* 读fds[0], 写fds[1], 伪管道fds[2] */
                                   /* pseudo(伪管道)，以读的方式打开fw_write同一个文件，不真正读数据 */
                                   /* 因为没有先以读方式open的管道文件是不能以写方式open */
}PIPE_CTRL_ST;

/* usb串口相关数据结构 */
typedef struct tagUSB_COM_ST
{
    char dev_name[NAME_LEN];          /* 串口设备名 */
    int fd;                           /* 设备文件句柄 */
    unsigned char buf_rx[BUF_SIZE];   /* 输入通道buf */
    unsigned int  buf_rx_len;         /* 当前buf有效内容长度 */
    unsigned int  reopen_flag;        /* 重新打开串口标志位 */
}USB_COM_ST;

struct  rrishced{
    struct timeval when;
    void (*callback)(void *data);
    void *data;
};


struct SndBuf{
    int in;
    int out;
    int size;
    unsigned char *data;
};


/* 模块通道信息数据结构 */
typedef struct tagMODULE_CHAN_ST
{
    unsigned int hw_port;             /* 模块物理端口号 */
    int mod_id;
    int state;                        /* =0代表非通话状态，丢弃语音数据 */
    PIPE_CTRL_ST pipes[PTN_NUM];      /* 每种数据类型对应一组读写通道 */

    /***** debug相关 ******/
    int dbg_flag_at;                     /* 调试模块标志，1代表调试模式 */
    int dbg_flag_snd;
    int dbg_fd_snd_rx;                /* 调试用文件句柄，保存从串口读到的语音数据 */
    int dbg_fd_snd_tx;                /* 调试用文件句柄，保存要写到串口的语音数据 */
    int dbg_fd_at;                    /* 调试用文件句柄 -- at指令 */
    int dbg_fd_cmd;                   /* 调试用文件句柄 -- 命令 */

    /***** 从串口接收到的数据相关 *****/
    struct timeval rx_time;           /* 当前接收数据的时间点 */
    int rx_at_state;					/*接收AT数据的状态机*/
    int rx_snd_len;                   /* 当前buf有效数据长度 */
    int rx_at_len;                    /* 当前buf有效数据长度 */
    unsigned char rx_snd_buf[RX_SND_BUF_SIZE+2];  /* 输入通道buf */
    unsigned char rx_at_buf[RX_AT_BUF_SIZE];    /* 输入通道buf */
    int upgrade_flag;                 /* =0代表非升级状态,1代表升级状态，升级状态不可被打断*/
    unsigned int tx_snd_speed;//???ͼ???
    unsigned int tx_snd_delay; //??ʼ??ʱ
    int tx_event_flag;//ʱ?䴥??????λ??=1??ʾ?ӹܵ��??յ?????????ʱ??Ҫ?????¼?
    int snd_buf_full;//ͨ???????У???????????
    int snd_buf_empty;//ͨ????????,????Ϊ?ռ???
    struct SndBuf tx_snd_buf;//?ܵ????????ݻ???
    struct rrishced sched;//??ʱ????
    void *mod;//ָ??ģ??????????
}MODULE_CHAN_ST;

typedef enum tagENCODE_PROTOCOL_TYPE_EN{
	ENCODE_PROTOCOL_TYPE_UNKOWN = 0,
	ENCODE_PROTOCOL_TYPE_HDLC,
	ENCODE_PROTOCOL_TYPE_COBS,
	ENCODE_PROTOCOL_TYPE_NUM,
}ENCODE_PROTOCOL_TYPE_EN;
	
typedef struct tagENCODE_INTERFACE_ST{
	ENCODE_PROTOCOL_TYPE_EN protocol;
	unsigned char fram_flag;
	int (*encrypt_len_limit)(unsigned char *src, int len_src, unsigned char *dst, int *len_dst, unsigned char ctrl_byte, int max_len);
	int (*encrypt)(unsigned char *src, int len_src, unsigned char *dst, int *len_dst);
	int (*decrypt)(unsigned char *src, int len_src, unsigned char *dst, int *len_dst);
}ENCODE_INTERFACE_ST;


/* 模块组数据结构，硬件上一个单片机带两个模块 */
/* 软件上一个usb串口与两个通道对应，每个通道有三种数据类型 */
typedef struct tagMODULE_GROUP_ST
{
    int alive;                        /* 模块组是否有效标志 */
    struct timeval usb_rx_time;
    int rx_timeout_cnt;
    pthread_mutex_t g_lock;
    USB_COM_ST usb_com;               /* usb 串口 */
    MODULE_CHAN_ST chans[MOD_NUM];    /* 2组模块通道 */
	ENCODE_INTERFACE_ST *encode;
}MODULE_GROUP_ST;

typedef enum rriDEBUG_LEVEL_EN{
	DEFAULT=0,
	ERROR = 1,
	WARN = 2,
	DEBUG =4,
	INFO = 8,
	NOTICE = 16,
}DEBUG_LEVEL_EN;


enum AT_RECEIVE_STATE
{
	NULL_AT = 0,
	RECEIVED_AT,
	RECEIVED_AT_END,
	RECEIVED_SMS_PRE,
	RECEIVED_SMS_PRE_END,
	RECEIVED_SMS_DATA,
	RECEIVED_SMS_DATA_END,
	RECEIVED_USSD_DATA,
	RECEIVED_USSD_DATA_END,
};


/* 不同类型管道名字 */
const char *module_pipe_names[PTN_NUM] = {"sound", "at", "mcu"};

/* server 通道，负责创建和管理其它通道 */
PIPE_CTRL_ST g_pipe_svr;                           

/* gsm 模块组数据 */
MODULE_GROUP_ST **g_module_groups;

/* 当前有效模块组个数 */
unsigned int g_mod_grp_num;

/* 物理通道个数 */
unsigned int g_hwport_num;

/* 全局文件句柄 */
struct pollfd *g_fds;

/* 要poll的文件句柄个数 */
unsigned char g_fds_num;

/* 本程序调试开关，0为关闭，>0打开 */
int g_server_debug = 0x1F;
FILE *g_fd_svr_dbg = NULL;

/* 线程开关 */
int g_thread_switch;

/* 测试用方波 *//* FOR_DEBUG */
unsigned char g_voice_alaw[RX_SND_BUF_SIZE] = 
{
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, /* 10个 */
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,       /* 9个，下同 */
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09
};



extern int soap_server_start();
int svr_chan_voice_stop(int );
int hdlc_encrypt_len_limit(unsigned char *src, int len_src, unsigned char *dst, int *len_dst, unsigned char ctrl_byte, int max_len);
int hdlc_encrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst);
int hdlc_decrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst);
int cobs_encrypt_len_limit(unsigned char *src, int len_src, unsigned char *dst, int *len_dst, unsigned char ctrl_byte, int max_len);
int cobs_encrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst);
int cobs_decrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst);

ENCODE_INTERFACE_ST encode_hdlc = {
	.protocol = ENCODE_PROTOCOL_TYPE_HDLC,
	.fram_flag = FRAME_FLAG_HDLC,
	.encrypt_len_limit = hdlc_encrypt_len_limit,
	.encrypt = hdlc_encrypt,
	.decrypt = hdlc_decrypt,
};

ENCODE_INTERFACE_ST encode_cobs = {
	.protocol = ENCODE_PROTOCOL_TYPE_COBS,
	.fram_flag = FRAME_FLAG_COBS,
	.encrypt_len_limit = cobs_encrypt_len_limit,
	.encrypt = cobs_encrypt,
	.decrypt = cobs_decrypt,
};

/**********************************************************
函数描述 : 打印封装函数
输入参数 : 
输出参数 : 
返回值   : 无
作者/时间: zhongwei.peng / 2016.09.07
************************************************************/
static void m_print(DEBUG_LEVEL_EN level, const char *fmt, ...)
{
    char buf[2048];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
	if(g_server_debug & level )
	{
		switch(level){
			case ERROR:
				zsys_error(buf);
				break;
			case WARN:
				zsys_warning(buf);
				break;
			case DEBUG:
				zsys_debug(buf);
				break;
			case INFO:
				zsys_info(buf);
				break;
			case NOTICE:
				zsys_notice(buf);
				break;
			default:
				fwrite(buf, 1, strlen(buf), g_fd_svr_dbg);
		}
	}
		
}

/**********************************************************
函数描述 : 获取配置文件内容
输入参数 : 
输出参数 : 
返回值   : -1 文件不存在或无法读取。
			0 文件里没有option内容
			1 获取option内容成功
作者/时间: zhongwei.peng / 2018.02.05
************************************************************/
int get_config(char* file_path, char* context_name, char* option_name,char *content)
{
	char buf[1024] = {0};
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	char name[256];
	FILE* fp;

	if( NULL == (fp=fopen(file_path,"r")) ) {
		m_print(ERROR, "[ERROR]Can't open %s",file_path);
		return -1;
	}

	while(fgets(buf,1024,fp)) {
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
						strcpy(content,name);
						fclose(fp);
						return 1;
					}
				}
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	return 0;
}


/**********************************************************
函数描述 : 获取超时时间(ms)
输入参数 : t -- 参考时间
输出参数 : 无
返回值   : 返回超时时间数
作者/时间: zhongwei.peng / 2016.11.28
************************************************************/
int get_time_out(struct timeval *t)
{
    int ms;
    struct timeval t_now;

    gettimeofday(&t_now, NULL);

    ms = ((long long)(t_now.tv_sec - t->tv_sec) * 1000000 + t_now.tv_usec - t->tv_usec) / 1000;

    return ms;
}

/**********************************************************
函数描述 : 向log文件写当前时间
输入参数 : fd -- 文件句柄
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
void write_time(int fd) 
{ 
   char buf[64]; 
   int size; 
   time_t now; 
   struct tm *timenow; 
   time(&now); 
   timenow = localtime(&now); 

   size = snprintf(buf, 
                    sizeof(buf), 
                    "\n%04d-%02d-%02d %02d:%02d:%02d   ",
                    timenow->tm_year+1900, 
                    timenow->tm_mon+1, 
                    timenow->tm_mday, 
                    timenow->tm_hour,
                    timenow->tm_min, 
                    timenow->tm_sec); 
   
   write(fd, buf, size); 
}

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
    if(g_server_debug & ERROR){
        for ( i = 0; i < len; i++ )
        {
            if ( i % 8 == 0 )
               fwrite("\r\n", 1, 2, g_fd_svr_dbg);

            fprintf(g_fd_svr_dbg,"%02x ", data[i]);
        }

    	 fprintf(g_fd_svr_dbg, "\r\n");
    }
}


/**********************************************************
函数描述 : cobs数据内容编码，第一个字节记录数据内容中第一个0的位置，0的位置记录下一个0的位置。最后一个自己为0，结束标志
		   如：0x00, 0x00 => 0x01,0x01,0x01,0x00
输入参数 : src -- 源数据内容
           len_src -- 源数据内容长度
输出参数 : dst -- 输出数据内容， 用户自己保证dst缓存比src至少大2个字节
           len_dst -- 输出数据内容长度
返回值   : =0代表成功，<0代表失败
作者/时间: wengang.mu / 2018.9.27
************************************************************/

#define StartBlock()	(code_ptr = dst++, code = 1)
#define FinishBlock()	(*code_ptr = code)

int cobs_encrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst){
	unsigned char *start = dst, *end = src + len_src;
	unsigned char code, *code_ptr; /* Where to insert the leading count */
	
	StartBlock();
	while (src < end) {
		if (code != 0xFF) {
			unsigned char c = *src++;
			if (c != 0) {
				*dst++ = c;
				code++;
				continue;
			}
		}
		FinishBlock();
		StartBlock();
	}
	FinishBlock();
	*dst++ = 0;//最后一个字节补为0，作为分隔符
	*len_dst = dst - start;
//	return dst - start;
	return 0;
}
/**********************************************************
函数描述 : cobs数据内容编码(限制帧长)，第一个字节记录数据内容中第一个0的位置，0的位置记录下一个0的位置。每帧最后一个字节为0，结束标志
		   如：0x00, 0x00 => 0x01,0x01,0x01,0x00
输入参数 : src -- 源数据内容
           len_src -- 源数据内容长度
           ctrl_byte -- 帧头，用于区分指令和通道
           max_len -- 最大帧长度
输出参数 : dst -- 输出数据内容， 用户自己保证dst缓存比src至少大帧数*3个字节
           len_dst -- 输出数据内容长度
返回值   : =0代表成功，<0代表失败
作者/时间: wengang.mu / 2018.9.27
************************************************************/

int cobs_encrypt_len_limit(unsigned char *src, int len_src, unsigned char *dst, int *len_dst, unsigned char ctrl_byte, int max_len) 
{ 
    int i, j, cnt,num,residue; 
    int end_1, data_len; 
    unsigned char *p = dst; 
//	unsigned char code = 0xFF;
	unsigned char c;
	unsigned char *start = dst, *end = src + len_src;
	unsigned char code = 0xFF, *code_ptr;
    /* 参数检查 */ 
    if ( (NULL == src) || (0 == len_src) || (NULL == dst) || (NULL == len_dst) ) 
        return -1; 
    
   end_1 = max_len - 3; 
   num = len_src / end_1; 
   residue = len_src % end_1; 
    cnt = 0; 
    data_len = max_len - 2;
	
	for ( i = 0; i < num; i++ ) 
	{
		int ctry_byte_flag = 1;
		StartBlock();
		for( j = 0; j < data_len; j++) 
		{
			if(ctry_byte_flag){
				c = ctrl_byte;
				ctry_byte_flag = 0;
			}else{
				c = *src++;
			}
			if(c != 0){
				*dst++ = c;
				code++;
				continue;
			}
			FinishBlock();
			StartBlock();
		}
		FinishBlock();
		*dst++ = 0;//最后一个字节补为0，作为分隔符
   } 
   if( residue != 0 ) 
   { 
		int ctry_byte_flag = 1;
		data_len = residue + 1;
		StartBlock();
		for( j = 0; j < data_len; j++) 
		{
			if(ctry_byte_flag){
				c = ctrl_byte;
				ctry_byte_flag = 0;
			}else{
				c = *src++;
			}
			if(c != 0){
				*dst++ = c;
				code++;
				continue;
			}
			FinishBlock();
			StartBlock();
		}
		FinishBlock();
		*dst++ = 0;//最后一个字节补为0，作为分隔符
   	}
    *len_dst = dst - start;
    return 0; 
}

/**********************************************************
函数描述 : cobs数据内容解码
		   如：0x01,0x01,0x01,0x00 => 0x00, 0x00
输入参数 : src -- 源数据内容
           len_src -- 源数据内容长度
输出参数 : dst -- 输出数据内容， 用户自己保证dst缓存比src至多大2个字节
           len_dst -- 输出数据内容长度
返回值   : =0代表成功，<0代表失败
作者/时间: wengang.mu / 2018.9.27
************************************************************/
int cobs_decrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst){
	unsigned char *start = dst, *end = src + len_src;
	unsigned char code = 0xFF, copy = 0;
	//如果第一个字节是0
	if(*src == 0 ){
		return -1;
	}
	for (; src < end; copy--) {
		if (copy != 0) {
			*dst++ = *src++;
		} else {
			if (code != 0xFF)//不是第一个字节，就填充为0；第一个字节就跳过
				*dst++ = 0;
			copy = code = *src++;
			if (code == 0)
				break; /* Source length too long */
		}
	}
	*len_dst = dst-start - 1;//去掉最后一个分隔符0
	return 0;
}


/**********************************************************
函数描述 : hdlc数据内容编码，FRAME_FLAG(0x7E) 作为帧头，
           ESCAPE_WORD_0(0x7F) 作为转义字符。0x7E用0x7F 0x7C两个字节表示。
           0x7F用0x7F 0x7F两个字节表示。
输入参数 : src -- 源数据内容
           len_src -- 源数据内容长度
           ctrl_byte -- 加在开关的控制字
           max_len -- 指定每个帧最大长度(包括帧头帧尾的0x7E)
输出参数 : dst -- 输出数据内容， 用户自己保证dst缓存是src的两倍，否则有溢出的可能
           len_dst -- 输出数据内容长度
返回值   : =0代表成功，<0代表失败
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
int hdlc_encrypt_len_limit(unsigned char *src, int len_src, unsigned char *dst, int *len_dst, unsigned char ctrl_byte, int max_len) 
{ 
    int i, j, cnt,num,residue; 
    int end_1; 
    unsigned char *p = dst; 

    /* 参数检查 */ 
    if ( (NULL == src) || (0 == len_src) || (NULL == dst) || (NULL == len_dst) ) 
        return -1; 
    
   end_1 = max_len - 3; 
   num = len_src / end_1; 
   residue = len_src % end_1; 
    cnt = 0; 
    
   for ( i = 0; i < num; i++ ) 
   { 
      /* 加帧头 */ 
      *p++ = FRAME_FLAG_HDLC; 
        *p++ = ctrl_byte; 
      cnt += 2; 
       
      for( j = 0; j < end_1; j++) 
      {
         *p++ = src[i*end_1+j]; 
         cnt ++; 
      } 
       
      /* 加帧尾 */ 
      *p++ = FRAME_FLAG_HDLC; 
      cnt ++; 
       
   } 
   if( residue != 0 ) 
   { 
      /* 加帧头 */ 
      *p++ = FRAME_FLAG_HDLC; 
        *p++ = ctrl_byte; 
      cnt += 2; 
       
      for(j=0; j < residue; j++) 
      { 
         *p++ = src[i*end_1+j]; 
         cnt ++; 
      } 
       
      /* 加帧尾 */ 
      *p++ = FRAME_FLAG_HDLC; 
      cnt ++; 
   } 
    
    *len_dst = cnt; 
    return 0; 
}

/**********************************************************
函数描述 : hdlc数据内容编码，FRAME_FLAG(0x7E) 作为帧头，
           ESCAPE_WORD_0(0x7F) 作为转义字符。0x7E用0x7F 0x7C两个字节表示。
           0x7F用0x7F 0x7F两个字节表示。
输入参数 : src -- 源数据内容
           len_src -- 源数据内容长度
输出参数 : dst -- 输出数据内容， 用户自己保证dst缓存是src的两倍，否则有溢出的可能
           len_dst -- 输出数据内容长度
返回值   : =0代表成功，<0代表失败
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
int hdlc_encrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst)
{
    int i, j;
    unsigned char *p = dst;

    /* 参数检查 */
    if ( (NULL == src) || (0 == len_src) || (NULL == dst) || (NULL == len_dst) )
        return -1;

    /* 帧头 */
    *p++ = FRAME_FLAG_HDLC;
    j = 1;

    for ( i = 0; i < len_src; i++ )
    {
        if ( FRAME_FLAG_HDLC == src[i] )
        {
            *p++ = ESCAPE_WORD_0;
            *p++ = ESCAPE_WORD_1;
            j += 2;
        }
        else if ( ESCAPE_WORD_0 == src[i] )
        {
            *p++ = ESCAPE_WORD_0;
            *p++ = ESCAPE_WORD_0;
            j += 2;
        }
        else
        {
            *p++ = src[i];
            j++;
        }
    }

    /* 帧尾 */
    *p++ = FRAME_FLAG_HDLC;
    j++;

    *len_dst = j;
    return 0;
}

/**********************************************************
函数描述 : hdlc数据内容解码
输入参数 : src -- 源数据内容
           len_src -- 源数据内容长度
输出参数 : dst -- 输出数据内容， 用户自己保证dst缓存>=src的，否则有溢出的可能
           len_dst -- 输出数据内容长度
返回值   : =0代表成功，<0代表失败
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
int hdlc_decrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst)
{
    int i, j;
    unsigned char *p = dst;
    int flag = 0;

    /* 参数检查 */
    if ( (NULL == src) || (len_src < 2) || (NULL == dst) || (NULL == len_dst) )
        return -1;

    /* 帧头帧尾检查 */
    if ( (src[0] != FRAME_FLAG_HDLC) || (src[len_src - 1] != FRAME_FLAG_HDLC) )
    {
        m_print(ERROR, "decrypt error: src[0]=0x%02x, src[%d]=0x%02x\n", src[0], len_src-1, src[len_src-1]);
        dump_data(src, len_src);
        return -1;
    }

    /* 去掉帧头帧尾 */
    len_src -= 1;
    i = 1;
    j = 0;

    while ( i < len_src )
    {
        if ( FRAME_FLAG_HDLC == src[i] )
        {
            m_print(ERROR, "decrypt error: src[%d]=0x%02x, src[%d]=0x%02x, len_src[%d]\n", i, src[i], i+1, src[i+1], len_src);
            flag = 1;
        }
        else if ( ESCAPE_WORD_0 == src[i] )
        {
            if ( ESCAPE_WORD_1 == src[i+1] )
            {
                *p++ = FRAME_FLAG_HDLC;
                i += 2;
                j++;
                continue;
            }
            else if ( ESCAPE_WORD_0 == src[i+1] )
            {
                *p++ = ESCAPE_WORD_0;
                i += 2;
                j++;
                continue;
            }
            else
            {
                /* 来到这里代表是有错误发生 */
                m_print(ERROR, "decrypt error 2: src[%d]=0x%02x, src[%d]=0x%02x\n", i, src[i], i+1, src[i+1]);
                flag = 1;
                *p++ = src[i];
                j++;
            }
        }
        else
        {
            *p++ = src[i];
            j++;
        }
        i++;
    }

    *len_dst = j;

    if ( flag ){
	 m_print(ERROR, "decrypt \n");
        dump_data(src, len_src+1);
    	}
    return 0;
}


/*
 * Set baudrate, parity and number of bits.
 */
void com_setparms(int fd, char *baudr, char *par, char *bits, char *stopb, int hwf, int swf)
{
  int spd = -1;
  int newbaud;
  int bit = bits[0];

#ifdef POSIX_TERMIOS
  struct termios tty;
#else /* POSIX_TERMIOS */
  struct sgttyb tty;
#endif /* POSIX_TERMIOS */

#ifdef POSIX_TERMIOS
  tcgetattr(fd, &tty);
#else /* POSIX_TERMIOS */
  ioctl(fd, TIOCGETP, &tty);
#endif /* POSIX_TERMIOS */

  /* We generate mark and space parity ourself. */
  if (bit == '7' && (par[0] == 'M' || par[0] == 'S'))
    bit = '8';

    /* Check if 'baudr' is really a number */
  if ((newbaud = (atol(baudr) / 100)) == 0 && baudr[0] != '0')
    newbaud = -1;

  switch (newbaud) {
    case 0:
#ifdef B0
      spd = B0;
#else
      spd = 0;
#endif
      break;
    case 3:	spd = B300;	break;
    case 6:	spd = B600;	break;
    case 12:	spd = B1200;	break;
    case 24:	spd = B2400;	break;
    case 48:	spd = B4800;	break;
    case 96:	spd = B9600;	break;
#ifdef B19200
    case 192:	spd = B19200;	break;
#else /* B19200 */
#  ifdef EXTA
    case 192:	spd = EXTA;	break;
#   else /* EXTA */
    case 192:	spd = B9600;	break;
#   endif /* EXTA */
#endif	 /* B19200 */
#ifdef B38400
    case 384:	spd = B38400;	break;
#else /* B38400 */
#  ifdef EXTB
    case 384:	spd = EXTB;	break;
#   else /* EXTB */
    case 384:	spd = B9600;	break;
#   endif /* EXTB */
#endif	 /* B38400 */
#ifdef B57600
    case 576:	spd = B57600;	break;
#endif
#ifdef B115200
    case 1152:	spd = B115200;	break;
#endif
#ifdef B230400
    case 2304:	spd = B230400;	break;
#endif
#ifdef B460800
    case 4608: spd = B460800; break;
#endif
#ifdef B500000
    case 5000: spd = B500000; break;
#endif
#ifdef B576000
    case 5760: spd = B576000; break;
#endif
#ifdef B921600
    case 9216: spd = B921600; break;
#endif
#ifdef B1000000
    case 10000: spd = B1000000; break;
#endif
#ifdef B1152000
    case 11520: spd = B1152000; break;
#endif
#ifdef B1500000
    case 15000: spd = B1500000; break;
#endif
#ifdef B2000000
    case 20000: spd = B2000000; break;
#endif
#ifdef B2500000
    case 25000: spd = B2500000; break;
#endif
#ifdef B3000000
    case 30000: spd = B3000000; break;
#endif
#ifdef B3500000
    case 35000: spd = B3500000; break;
#endif
#ifdef B4000000
    case 40000: spd = B4000000; break;
#endif
  }

#ifdef POSIX_TERMIOS

  if (spd != -1) {
    cfsetospeed(&tty, (speed_t)spd);
    cfsetispeed(&tty, (speed_t)spd);
  }

  tty.c_cflag = 0;

  switch (bit) {
    case '5':
      tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
      break;
    case '6':
      tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
      break;
    case '7':
      tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
      break;
    case '8':
    default:
      tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
      break;
  }
  
  /* Set into raw, no echo mode */
  tty.c_iflag =  IGNBRK;
  tty.c_lflag = 0;
  tty.c_oflag = 0;
  tty.c_cflag |= CLOCAL | CREAD;
#ifdef _DCDFLOW
  tty.c_cflag &= ~CRTSCTS;
#endif
  tty.c_cc[VMIN] = 0;//1
  tty.c_cc[VTIME] = 0;//5

  if (swf)
    tty.c_iflag |= IXON | IXOFF;
  else
    tty.c_iflag &= ~(IXON|IXOFF|IXANY);

  tty.c_cflag &= ~(PARENB | PARODD);
  if (par[0] == 'E')
    tty.c_cflag |= PARENB;
  else if (par[0] == 'O')
    tty.c_cflag |= (PARENB | PARODD);

  if (stopb[0] == '2')
    tty.c_cflag |= CSTOPB;
  else
    tty.c_cflag &= ~CSTOPB;

  tcsetattr(fd, TCSANOW, &tty);
#endif /* POSIX_TERMIOS */
}

/**********************************************************
函数描述 : 创建串口对象
输入参数 : dev_name -- 串口设备名
           baud -- 波特率
输出参数 : 无
返回值   : 成功返回fd, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.24
************************************************************/
int com_new(char *dev_name, char *baud)
{
    int fd;

    /* 参数检查 */
    if ( (NULL == dev_name) || (NULL == baud) )
        return -1;

    fd = open(dev_name, O_RDWR|O_NDELAY|O_NOCTTY);
    if ( fd < 0 )
        return -1;

    com_setparms(fd, baud, "N", "8", "1", 0, 0);

    return fd;
}

/**********************************************************
函数描述 : 串口接收超时的处理，主要作用是往pipe塞点数据，结束已挂断的通话
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.06.25
************************************************************/
void com_recv_timeout(void)
{
    int i, j;
    int ms;
    unsigned char tmp = 0;
    MODULE_GROUP_ST *mod_grp;
    MODULE_CHAN_ST *pchan;
    struct timeval t_now;

    gettimeofday(&t_now, NULL);    

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        mod_grp = g_module_groups[i];

        for ( j = 0; j < MOD_NUM; j++ )
        {
            pchan = &mod_grp->chans[j];

            /* 如果在通话状态，大于20ms算超时 */
            if ( pchan->state )
            {
                ms = ((long long)(t_now.tv_sec - pchan->rx_time.tv_sec) * 1000000 + (t_now.tv_usec - pchan->rx_time.tv_usec)) / 1000;

                if ( ms > 30 )
                {
                    m_print(WARN, "chan[%d] com_recv_timeout, rx_snd_len=%d\n", pchan->hw_port, pchan->rx_snd_len);
                    /* 发点数据，让asterisk结束通话 */
                    if ( pchan->rx_snd_len > 0 ) 
                        (void)write(pchan->pipes[PTN_SND].fds[PIFD_W], &(pchan->rx_snd_buf[0]), pchan->rx_snd_len);
                    else
                        (void)write(pchan->pipes[PTN_SND].fds[PIFD_W], &tmp, 1);

                    pchan->rx_snd_len = 0;
                    /* 通知单片机不要再上发语音数据 */
                    //svr_chan_voice_stop(i * MOD_NUM + j + 1);
                }
            }
        }
    }
}

/**********************************************************
函数描述 : 上送at
输入参数 : fd -- pipe 文件句柄
           pchan -- 模块通道数据结构首指针
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.8.3
************************************************************/
void com_recv_deliver_at(int fd, MODULE_CHAN_ST *pchan)
{
    char buf_print[1024] = {0};
    
    /* 上送 asterisk */
    (void)write(fd, pchan->rx_at_buf, pchan->rx_at_len);

    /* 调试信息 */
    if ( pchan->dbg_flag_at )
    {
        write_time(pchan->dbg_fd_at);
        pchan->rx_at_buf[pchan->rx_at_len] = '\0';
        sprintf(buf_print, "RX:%s", pchan->rx_at_buf);
        (void)write(pchan->dbg_fd_at, buf_print, strlen(buf_print));
    }

    /* 上送数据后重置变量 */
    pchan->rx_at_len = 0;
	pchan->rx_at_state = NULL_AT;
}

/**********************************************************
函数描述 : 串口接收到数据后分发到各个pipe
输入参数 : mod_grp -- 模块板(组)首指针
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
void com_recv_deliver(MODULE_GROUP_ST *mod_grp, unsigned char *buf_dst, int len_dst)
{
    int i;
    int fd;
    int mod_id;
    int ptn_id;
    int res, count;
    char buf_print[1024];
    MODULE_CHAN_ST *pchan;

    /* 获取模块号及数据类型并检查 */
    mod_id = (buf_dst[0] >> 4) & 0x0F;
    ptn_id = buf_dst[0] & 0x0F;

    if ( (mod_id >= MOD_NUM) || (ptn_id >= PTN_NUM) )
    {
        m_print(ERROR,"mod_id or ptn_id invalid: 0x%02x, len[%d] \n", buf_dst[0], len_dst);
        dump_data(buf_dst, len_dst);
        return;
    }
    pchan = &mod_grp->chans[mod_id];
    fd = pchan->pipes[ptn_id].fds[PIFD_W];

    /* 去掉第一字节长度 */
    len_dst -= 1;

    /* 如果是音频数据，特殊处理 */
    if ( PTN_SND == ptn_id )
    {
        /* 当前不在通话状态就丢弃 */
        if ( 0 == pchan->state )
            return;

        /* 缓存到buf */
        gettimeofday(&(pchan->rx_time), NULL);  /* 每次接收到数据都获取时间 */
        for ( i = 1; i <= len_dst; i++ )
        {
            pchan->rx_snd_buf[pchan->rx_snd_len] = buf_dst[i];
            pchan->rx_snd_len++;

            if ( pchan->rx_snd_len >= RX_SND_BUF_SIZE )
            {
                /* 分发数据 */
                res = write(fd, &(pchan->rx_snd_buf[0]), RX_SND_BUF_SIZE);
                if (res < 0) {
                    m_print(ERROR, "chan[%d] write sound to pipe failed, res = %d!\n", pchan->hw_port, res);
                } else if(res < RX_SND_BUF_SIZE) {
                	count = RX_SND_BUF_SIZE - res;
                    res = write(fd, &(pchan->rx_snd_buf[res]), count);
                    if (res != count ) {
                        m_print(WARN, "chan[%d] write %d bytes to sound pipe, count=%d \n", pchan->hw_port, res, count);
                    }
                }
                pchan->rx_snd_len = 0;
                /* FOR_DEBUG */
                //test_send_voice(mod_grp, buf_dst[0]);
            }
        }

        /* 调试信息 */
        if ( pchan->dbg_flag_snd)
            (void)write(pchan->dbg_fd_snd_rx, &buf_dst[1], len_dst);

        return;
    }
    else if ( PTN_CMD == ptn_id )
    {
        /* 分发数据 */
        (void)write(fd, &buf_dst[1], len_dst);

        buf_dst[len_dst+1] = '\0';

        /* 调试信息 */
        if ( pchan->dbg_flag_at )
        {
            write_time(pchan->dbg_fd_cmd);
            sprintf(buf_print, "RX:%s\n", &buf_dst[1]);
            //m_print("%s", buf_print);
            res = write(pchan->dbg_fd_cmd, buf_print, strlen(buf_print));
            if (res < 0) {
                m_print(ERROR, "chan[%d] write data to cmd pipe failed!\n", pchan->hw_port);
            }
        }

        return;
    }
    else if ( PTN_AT == ptn_id )
    {
    	if(pchan->upgrade_flag){//如果是升级模式，则上报所有信息
		pchan->rx_at_len = (len_dst < RX_AT_BUF_SIZE  ? len_dst : RX_AT_BUF_SIZE);
              memcpy(pchan->rx_at_buf, buf_dst + 1, pchan->rx_at_len);
		if(pchan->dbg_flag_at)
	    	{
		        write_time(pchan->dbg_fd_at);
		        pchan->rx_at_buf[pchan->rx_at_len] = '\0';
		        sprintf(buf_print, "RX(drop):%s", pchan->rx_at_buf);
		        (void)write(pchan->dbg_fd_at, buf_print, strlen(buf_print));
		}
              com_recv_deliver_at(fd, pchan);
	       return;
	    }
        /* 如果超过缓存大小就丢弃数据 */
        if ( (pchan->rx_at_len + len_dst) >= RX_AT_BUF_SIZE )
        {
            /* 调试信息 */
            if ( pchan->dbg_flag_at )
            {
                write_time(pchan->dbg_fd_at);
                pchan->rx_at_buf[pchan->rx_at_len] = '\0';
                sprintf(buf_print, "RX(drop):%s", pchan->rx_at_buf);
                (void)write(pchan->dbg_fd_at, buf_print, strlen(buf_print));
            }

            pchan->rx_at_len = 0;
        }

        /* 先接收到缓存里面，遇到'>'上送，遇到回车上送('+'开头要是否短信) */
        i = 1;
        while ( i <= len_dst )
        {
            /* '>'特殊处理 */
            if ( '>' == buf_dst[i] )
            {
                pchan->rx_at_buf[pchan->rx_at_len++] = buf_dst[i++];
                if(buf_dst[i] == ' ');//
		    pchan->rx_at_buf[pchan->rx_at_len++] = buf_dst[i++];
                com_recv_deliver_at(fd, pchan);
                continue;
            }

			/**************************************************************************
			NULL_AT:判断是否收到非\r\n数据，如果是，进入RECEIVED_AT。
			RECEIVED_AT: 当收到\r\n结束符时判断是否为短信AT，如果是，进入RECEIVED_SMS_PRE_END，如果不是进入RECEIVED_AT_END
			RECEIVED_AT_END: 收到非\r\n字符时直接发送前面缓存的buff,回到NULL_AT
			//RECEIVED_AT_END 收到的短信AT前缀还不完整，判断是否收到\r\n数据，如果是，进入RECEIVED_SMS_PRE_END
			RECEIVED_SMS_PRE_END 已经收到了完整的短信前缀，判断是否收到非\r\n数据，如果收到，进入到RECEIVED_SMS_DATA
			RECEIVED_SMS_DATA 已经开始接收短信数据AT，判断是否收到\r\n数据，如果收到，进入到阶段RECEIVED_SMS_DATA_END
			RECEIVED_SMS_DATA_END 短信AT已经接收完整，判断是否收到非\r\n数据，如果收到，发送已缓存buff，回到NULL_AT
			****************************************************************************/
			if ('\r' == buf_dst[i] || '\n' == buf_dst[i])
			{
				switch(pchan->rx_at_state)
				{
					case RECEIVED_AT:
						pchan->rx_at_buf[pchan->rx_at_len] = '\0';
						if(strstr(pchan->rx_at_buf, "+CMT") || strstr(pchan->rx_at_buf, "CDS") \
							|| strstr(pchan->rx_at_buf,"^HCMT")){
							pchan->rx_at_state = RECEIVED_SMS_PRE_END;
						}else if(strstr(pchan->rx_at_buf, "+CUSD") && strstr(pchan->rx_at_buf, ",\"") && strstr(pchan->rx_at_buf, "\",") == NULL) {
							pchan->rx_at_state = RECEIVED_USSD_DATA;
						}else{
							pchan->rx_at_state = RECEIVED_AT_END;
						}
						break;
					case RECEIVED_SMS_PRE:
						pchan->rx_at_state = RECEIVED_SMS_PRE_END;
						break;
					case RECEIVED_SMS_DATA:
						pchan->rx_at_state = RECEIVED_SMS_DATA_END;
						break;
					case RECEIVED_USSD_DATA:
						pchan->rx_at_buf[pchan->rx_at_len] = '\0';
						if(strstr(pchan->rx_at_buf, "\",")){
							pchan->rx_at_state = RECEIVED_USSD_DATA_END;
						}
						break;
					default:
						break;
				}
			}
			else{
				switch(pchan->rx_at_state)
				{
					case NULL_AT:
						pchan->rx_at_state = RECEIVED_AT;
						break;
					case RECEIVED_AT_END:
                        com_recv_deliver_at(fd, pchan);
						continue;
					case RECEIVED_SMS_PRE_END:
						pchan->rx_at_state = RECEIVED_SMS_DATA;
						break;
					case RECEIVED_SMS_DATA_END:
                        			com_recv_deliver_at(fd, pchan);
						continue;
					case RECEIVED_USSD_DATA_END:
						com_recv_deliver_at(fd, pchan);
						continue;
					default:
						break;
				}
			}

            pchan->rx_at_buf[pchan->rx_at_len++] = buf_dst[i++];
        } // end while

		/*如果接收完整，就发送出去*/
		if(RECEIVED_AT_END == pchan->rx_at_state || RECEIVED_SMS_DATA_END == pchan->rx_at_state || RECEIVED_USSD_DATA_END == pchan->rx_at_state){
			com_recv_deliver_at(fd, pchan);
		}
        return;
    }/* end: if (PTN_AT == ptn_id) */
}

/**********************************************************
函数描述 : 串口接收到数据的处理
输入参数 : mod_grp -- 模块板(组)首指针
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
void com_recv_handle(MODULE_GROUP_ST *mod_grp)
{
    int ret;
    int i;
    int len_dst;
    unsigned char buf_src[BUF_SIZE] = {0};
    unsigned char buf_dst[BUF_SIZE] = {0};
    USB_COM_ST *pcom = &mod_grp->usb_com;

    /* 检查是否要重新打开串口 */
    if ( pcom->reopen_flag )
    {
        /* 重新打开串口设备 */
        if ( pcom->fd > 0 )
        {
            close(pcom->fd);
            pcom->fd = 0;
        }

        if ( (pcom->fd = com_new(pcom->dev_name, USB_COM_BAUD)) < 0 )
        {
            m_print(ERROR, "reopen com fail! name = %s\n", pcom->dev_name);
            return;
        }
        else
        {
            m_print(INFO, "reopen com %s ok!\n", pcom->dev_name);
        }
        pcom->reopen_flag = 0;
    }

    /* 从串口读数据 */
    if ( (ret = read(pcom->fd, buf_src, BUF_SIZE)) <= 0 )
        return;
    gettimeofday(&mod_grp->usb_rx_time, NULL);
    //m_print("\n\n com[%s] recv[%d]", mod_grp->usb_com.dev_name, ret);
    //dump_data(buf_src, ret);

	/*如果前面收到的数据是半个包/只剩最后一个字节？*/
	//前面收到的数据已经存储到buf_rx中，把后面的数据继续copy到buf_rx即可
    /* 如果第一个字节是标识符，就认定前面的是错误帧 */
    if ( mod_grp->encode->protocol == ENCODE_PROTOCOL_TYPE_HDLC 
			&& (FRAME_FLAG_HDLC == buf_src[0]) 
			&& (buf_src[1] != FRAME_FLAG_HDLC) ){
		if(pcom->buf_rx_len > 0)
                m_print(ERROR, "Not complete frame, discard!\n");
		pcom->buf_rx_len = 0;
	}

    for ( i = 0; i < ret; i++ )
    {
	    if(mod_grp->encode->fram_flag == buf_src[i] && pcom->buf_rx_len > 0)
	    {
		    pcom->buf_rx[pcom->buf_rx_len++] = buf_src[i];
		    if ( mod_grp->encode->decrypt(pcom->buf_rx, pcom->buf_rx_len, buf_dst, &len_dst) == 0 ){
			    com_recv_deliver(mod_grp, buf_dst, len_dst);
		    }
		    pcom->buf_rx_len= 0;
		    //			printf("com decrypt:\n");
		    //			dump_data(buf_dst, len_dst);
		    if(mod_grp->encode->protocol == ENCODE_PROTOCOL_TYPE_COBS){
			    if ( ((i + 1) < ret) && (buf_src[i+1] == mod_grp->encode->fram_flag) )
			    {
				    m_print(ERROR,"drop frame! dev_name=%s\n", pcom->dev_name);
				    dump_data(buf_src, ret);
				    return;
			    }
			    else
			    {
				    continue;
			    }
		    }else{
			    if ( ((i + 1) < ret) && (buf_src[i+1] != mod_grp->encode->fram_flag) )
			    {
				    m_print(ERROR,"drop frame! dev_name=%s\n", pcom->dev_name);
				    dump_data(buf_src, ret);
				    return;
			    }
			    else
			    {
				    continue;
			    }
		    }
	    }

        pcom->buf_rx[pcom->buf_rx_len++] = buf_src[i];
    }
    return;
}

/**********************************************************
函数描述 : 向单片机发送命令或者数据
输入参数 : hwport -- 物理端口号
           data -- 要发送的数据内容
           len -- 要发送的长度
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
int com_cmd_send(int hwport, unsigned char *data, int len)
{
    int i, j;
    int len_dst;
    unsigned char ctrl_byte;
    char buf_src[BUF_SIZE];
    unsigned char buf_dst[BUF_SIZE];

    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    /* 检查长度, 输出buf的一半，另外还有帧头帧尾，标识字节共3个字节 */
    if ( len >= (BUF_SIZE / 2 - 3) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
                /* 标识字节 */
                ctrl_byte = (((unsigned char)j) & 0x0F) << 4;
                ctrl_byte |= (PTN_CMD & 0x0F);

                /* 编码 */
                if ( g_module_groups[i]->encode->encrypt_len_limit(data, len, buf_dst, &len_dst, ctrl_byte, 64) < 0 )
                    return -1;


                (void)write(g_module_groups[i]->usb_com.fd, buf_dst, len_dst);

                /* 调试信息 */
                if ( g_module_groups[i]->chans[j].dbg_flag_at )
                {
                    write_time(g_module_groups[i]->chans[j].dbg_fd_cmd);
                    sprintf(buf_src, "TX:%s\n", data);
                    (void)write(g_module_groups[i]->chans[j].dbg_fd_cmd, buf_src, strlen(buf_src));
                }
                return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 打开一个管道文件，文件不存在就创建
输入参数 : file_name -- 文件名，绝对路径
           mode -- O_RDONLY 或者 O_WRONLY (一定是O_NONBLOCK方式)
输出参数 : 
返回值   : 成功>=0, 失败<0
作者/时间: zhongwei.peng / 2016.09.07
************************************************************/
int pipe_open(char *file_name, int mode)
{
    int res;

    if ( NULL == file_name )
    {
        m_print(ERROR, "file_name == NULL\n");
        return -1;
    }

    if ( (mode != O_RDONLY) && (mode != O_WRONLY) )
    {
        m_print(ERROR, "mode[%d] != O_RDONLY or O_WRONLY\n", mode);
        return -1;
    }

    if ( access(file_name, F_OK) == -1 )
    {
        res = mkfifo(file_name, 0777);  
        if ( res != 0 )
        {  
            m_print(ERROR, "create fifo %s fail\n", file_name);
            return -1;
        }
    }

    res = open(file_name, mode|O_NONBLOCK);
    if ( res < 0 )
    {  
        m_print(ERROR, "open fifo %s fail, fd = %d\n", file_name, res);
        return -1;
    }

    return res;
}

/**********************************************************
函数描述 : 创建一个新的pipe节点
输入参数 : name -- 文件名
输出参数 : ppipe -- pipe 指针
返回值   : 成功>=0, 失败<0
作者/时间: zhongwei.peng / 2016.09.07
************************************************************/
int pipe_new(PIPE_CTRL_ST *ppipe, char *name)
{
    int res;
    char file_name[BUF_SIZE];

    if ( (NULL == ppipe) || (NULL == name) )
        return -1;

    /* 保存名字 */
    memset(ppipe->name, 0, NAME_LEN);
    res = strlen(name);
    if ( (res > 0) && (res < NAME_LEN) )
        strncpy(ppipe->name, name, res);
    else
        return -1;

    /* 打开读pipe文件 */
    sprintf(file_name, "%s%s%s", MODULE_PIPE_DIR, name, FILE_PIPE_READ);
	if ( (ppipe->fds[PIFD_R] = pipe_open(file_name, O_RDONLY)) < 0 )
	{
		m_print(ERROR, "open read pipe file fail! file_name = %s, fd = %d\n", file_name, ppipe->fds[PIFD_R]);
		return -1;
	}

    /* 先用读打开，放在fd pseudo，这样就能用写打开 */
    sprintf(file_name, "%s%s%s", MODULE_PIPE_DIR, name, FILE_PIPE_WRITE);
	if ( (ppipe->fds[PIFD_P] = pipe_open(file_name, O_RDONLY)) < 0 )
	{
	    close(ppipe->fds[PIFD_R]);
		m_print(ERROR, "open read pipe file fail! file_name = %s, fd = %d\n", file_name, ppipe->fds[PIFD_P]);
		return -1;
	}

    /* 打开写pipe文件 */
	if ( (ppipe->fds[PIFD_W] = pipe_open(file_name, O_WRONLY)) < 0 )
	{
	    close(ppipe->fds[PIFD_P]);
           close(ppipe->fds[PIFD_R]);
		m_print(ERROR, "open write pipe file fail! file_name = %s, fd = %d\n", file_name, ppipe->fds[PIFD_W]);
		return -1;
	}

    return 0;
}

/**********************************************************
函数描述 : 关闭文件
输入参数 : fds -- 文件节点
输出参数 : 无
返回值   : 无
作者/时间: zhongwei.peng / 2016.09.08
************************************************************/
void pipe_close_fd(PIPE_CTRL_ST *ppipe)
{
    if ( ppipe->fds[PIFD_W] >= 0 )
    {
        close(ppipe->fds[PIFD_W]);
        ppipe->fds[PIFD_W] = -1;
    }

    if ( ppipe->fds[PIFD_P] >= 0 )
    {
        close(ppipe->fds[PIFD_P]);
        ppipe->fds[PIFD_P] = -1;
    }

    if ( ppipe->fds[PIFD_R] >= 0 )
    {
        close(ppipe->fds[PIFD_R]);
        ppipe->fds[PIFD_R] = -1;
    }
    
}

/**********************************************************
函数描述 : 从pipe接收语音数据
输入参数 : mod_grp -- 模块板(组)首指针
           mod_id -- 模块id，0或者1
           ptn_id -- 传输类型

输出参数 : 无
返回值   : 无
***********************************************************/
#define min(a, b) (((a) < (b)) ? (a) : (b))


/*??ʼ?????����???*/
int tx_snd_data_init(struct SndBuf *buf){
    if(buf->size == 0)
        buf->size = BUF_SIZE;
    
    if(buf->data != NULL){
        free(buf->data);
        buf->data = NULL;
    }
    
    buf->data = (unsigned char *)malloc(buf->size);
    if(buf->data == NULL){
        m_print(ERROR, "malloc memory failed!\n");
        return -1;
    }
    buf->in = 0;
    buf->out = 0;
    return 0;
}
/*ȥ??ʼ?????����???*/
void tx_snd_data_deinit(struct SndBuf *buf){
    if(buf->data != NULL){
        free(buf->data);
        buf->data = NULL;
   }
    buf->in = 0;
    buf->out = 0;
}

/*????:ȥ??ʼ????????
*????:??ȡ?????ֽ???
*/

int get_sound_from_sndbuf(MODULE_CHAN_ST *mod, void *buffer, unsigned int size ){
    unsigned int len = 0;
    struct SndBuf *buf = &mod->tx_snd_buf;
    size  = min(size, buf->in - buf->out);
    len = min(size, buf->size - (buf->out & (buf->size - 1)));
    memcpy(buffer, buf->data + (buf->out & (buf->size - 1)), len);
    memcpy(buffer + len, buf->data, size - len);
    buf->out += size;
    return size;
}
/*???????????ݵ???????
*????:ʹ?õĻ???????С

*/
unsigned int put_sound_to_sndbuf(MODULE_CHAN_ST *chan, void *buffer, unsigned int size){
    unsigned int len = 0;
    unsigned int buf_len;
    struct SndBuf *buf = &chan->tx_snd_buf;
    struct timeval start_tv;
    buf_len = buf->in - buf->out;
    if(buf_len + size >  buf->size){
        chan->snd_buf_full++;
        gettimeofday(&start_tv, NULL);
        m_print(ERROR, "port[%d] Mem overflow, give up, snd_buf_full_count = %d, put time is %dms\n", chan->hw_port, chan->snd_buf_full,start_tv.tv_usec/1000);
        return -1;
    }
    size = min(size, buf->size - buf->in + buf->out);
    len  = min(size, buf->size - (buf->in & (buf->size - 1)));
    memcpy(buf->data + (buf->in & (buf->size - 1)), buffer, len);
    memcpy(buf->data, buffer + len, size - len);
    buf->in += size;
     if(g_server_debug & DEBUG){
        m_print(DEBUG, "port[%d] buf total size %dbytes, have used %dbytes, \n ", chan->hw_port, buf->size, buf->in - buf->out);
        
     }
    return size;
}


/*???????????ݷ??͸?ģ??*/
void write_sndbuf_to_module(void *data){
	unsigned char buf_src[RX_SND_BUF_SIZE] = {0};
	unsigned char buf_dst[BUF_SIZE];
	unsigned char frame_head;

	struct timeval start_tv;
	int len_dst;
	int len_src;
	int res;
	MODULE_CHAN_ST *chan = (MODULE_CHAN_ST*)data;
	MODULE_GROUP_ST *mod_grp = (MODULE_GROUP_ST *)chan->mod;
	int mod_id = chan->mod_id;
	int ptn_id = PTN_SND;

	rri_schedule_event(chan, chan->tx_snd_speed, write_sndbuf_to_module, chan);
	if(chan->state != 0){
		len_src = get_sound_from_sndbuf(chan, buf_src, chan->tx_snd_speed << 3);
		if(len_src <= 0){
			len_src = chan->tx_snd_speed << 3;
			//????buf????û?????ݣ?????һ?ξ???????
			memset(buf_src, 0xD5, chan->tx_snd_speed<<3);
			chan->snd_buf_empty++;
			gettimeofday(&start_tv, NULL);
			m_print(ERROR, "port[%d] read snd data failed, have no data, snd_buf_empty_count = %d, now tiem is %dms!\n", chan->hw_port, chan->snd_buf_empty,start_tv.tv_usec/1000);
			return;
		}

		frame_head = (((unsigned char)mod_id) & 0x0F) << 4;
		frame_head |= (ptn_id & 0x0F);
		/*д?뵽pcm?ļ???*/
		if(chan->dbg_flag_snd){
			write(chan->dbg_fd_snd_tx, buf_src, len_src);
		}

		if(g_server_debug &DEBUG){
			gettimeofday(&start_tv, NULL);
			m_print(DEBUG, "port[%d] write data to usb serial, now time is %dms\n", chan->hw_port, start_tv.tv_usec/1000); 
		}

		if ( mod_grp->encode->encrypt_len_limit(buf_src, len_src, buf_dst, &len_dst, frame_head, 64) < 0 )
		{
			m_print(ERROR, "port[%d] encrypt error:\n", chan->hw_port);
			dump_data(buf_src, len_src);
			return;
		}
		pthread_mutex_lock(&mod_grp->g_lock);
		res =write(mod_grp->usb_com.fd, buf_dst, len_dst);
		pthread_mutex_unlock(&mod_grp->g_lock);
		if(res < 0) {
			m_print(ERROR, "port[%d] write sound data to com serial failed!\n", chan->hw_port);
		}
	}
}
/*?????????????????¼?*/
int  rri_schedule_event(MODULE_CHAN_ST *chan, int ms, void (*function)(void *data), void *data) {

	struct timeval tv;
     
	/* Get current time */
	gettimeofday(&tv, NULL);
        int delay_s = ms/1000;
        int delay_ms = (ms % 1000)*1000;
        //?????ǵ?һ?δ????ص??????õ?ǰʱ??????ʱ??????????һ?η???ʱ??????ʱ
        if(chan->tx_event_flag == 1)
            gettimeofday(&tv, NULL);
        else
            tv= chan->sched.when;     

	/* Get the schedule end time */
	tv.tv_sec += delay_s;
	tv.tv_usec += delay_ms;
	if (tv.tv_usec > 1000000) {
		tv.tv_usec -= 1000000;
		tv.tv_sec += 1;
	}
	/* Set schedule */
	chan->sched.when = tv;			/* end time */
	chan->sched.callback = function;	/* callback function */
	chan->sched.data = data;			/* data */

	/* return schedule id */
	return 0;
}
/*??ʱ????rri_schedule_event?????õĻص?ʱ??*/
int __rri_schedule_run(MODULE_CHAN_ST *chan, struct timeval *tv){
    void (*callback)( void *);
    void *data;

    if(chan->sched.callback &&
            ((chan->sched.when.tv_sec < tv->tv_sec) ||
            ((chan->sched.when.tv_sec == tv->tv_sec) &&
            (chan->sched.when.tv_usec <= tv->tv_usec)))){
        
                    callback = chan->sched.callback;
                    data = chan->sched.data; 

                    chan->sched.callback = NULL;
                    chan->sched.data = NULL;
                    callback( data);
        }
 
   return 0;
}

/*??ʱ????????????*/
int rri_schedule_run(MODULE_CHAN_ST *chan) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return __rri_schedule_run(chan,  &tv);
}

/*?ӹܵ��??????ݲ?д?뵽??????*/
void pipe_file_recv_sound(MODULE_GROUP_ST *mod_grp, int mod_id, int ptn_id){
	
	unsigned char buf_src[BUF_SIZE] = {0};
	int fd = 0;
	int ret = -1;
       unsigned int size = 0;
	MODULE_CHAN_ST *pchan = &mod_grp->chans[mod_id];

	fd = pchan->pipes[ptn_id].fds[PIFD_R];
    	if ( (ret = read(fd, buf_src, BUF_SIZE)) <= 0 )
    		return;
       if(mod_grp->chans[mod_id].state == 0)
            return ;
            //д????????
       size = put_sound_to_sndbuf(pchan, buf_src, ret);
       if(size < 0){
            m_print(ERROR, "put data to snd buf failed!\n");
       }else if(mod_grp->chans[mod_id].tx_event_flag == 1){
            rri_schedule_event(pchan, pchan->tx_snd_delay, write_sndbuf_to_module, pchan);
            //???ô????¼?Ϊ0
            mod_grp->chans[mod_id].tx_event_flag = 0;
       }
	return;

}
/*??????ʱ?????߳?*/
void *module_group_audio_thread(void *param)
{
    int i, j;

    MODULE_GROUP_ST *mod_grp;

    (void)param;

    while ( g_thread_switch )
	{
	    if ( g_fds_num > 0 )
        {   
            //module_group_fds_init();
            usleep(10000);
        }
        else
        {
    	    sleep(5);
            continue;
        }
    
	    for ( i = 0; i < g_mod_grp_num; i++ )
        {
            mod_grp = g_module_groups[i];

            if ( mod_grp->alive == 0 )
                continue;
            for ( j = 0; j < MOD_NUM; j++ )
            {
                    if(mod_grp->chans[j].state == 0)//????ͨ??״̬??ֱ???˳?
                        continue;
                    
                   rri_schedule_run(&mod_grp->chans[j]);
                   
            }
        }
        
	}// end while
	return (void *)0;
}

/**********************************************************
函数描述 : pipe管理接收到数据的处理
输入参数 : mod_grp -- 模块板(组)首指针
           mod_id -- 模块id，0或者1
           ptn_id -- 传输类型
输出参数 : 无
返回值   : 无
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
void pipe_file_recv_handle(MODULE_GROUP_ST *mod_grp, int mod_id, int ptn_id)
{
    int fd;
    int ret, res;
    int len_dst;
	unsigned char crtl_byte;
    unsigned char buf_src[BUF_SIZE] = {0};
    unsigned char buf_dst[BUF_SIZE];
    char buf_print[1024];
    MODULE_CHAN_ST *pchan;

    pchan = &mod_grp->chans[mod_id];
    fd = pchan->pipes[ptn_id].fds[PIFD_R];

    if ( PTN_AT == ptn_id )
    {
        /* 从pipe读数据, 每次下发最多64， 还有3个字节的帧头帧尾，剩61，多一个字节预留给0x7E转换 */
        if ( (ret = read(fd, &buf_src[1], 60)) <= 0 )
            return;

        /* 写到debug文件去 */
        if ( pchan->dbg_flag_at )
        {
            write_time(pchan->dbg_fd_at);
            sprintf(buf_print, "%s[%d] TX:%s", mod_grp->usb_com.dev_name, mod_id, &buf_src[1]);
            (void)write(pchan->dbg_fd_at, buf_print, strlen(buf_print));
        }
        
    }
    else if ( PTN_CMD == ptn_id )
    {       
        /* 从pipe读数据 */
        if ( (ret = read(fd, &buf_src[1], BUF_SIZE)) <= 0 )
            return;

        /* 写到debug文件去 */
        if ( pchan->dbg_flag_at )
        {
            write_time(pchan->dbg_fd_cmd);
            sprintf(buf_print, "TX:%s\n", &buf_src[1]);
            (void)write(pchan->dbg_fd_cmd, buf_print, strlen(buf_print));
        }
		
    }
    else
    {
		pipe_file_recv_sound(mod_grp, mod_id, ptn_id);
        return;
    }

    /* 检查长度, 输出buf的一半，另外还有帧头帧尾，标识字节共3个字节 */
    if ( ret >= (BUF_SIZE / 2 - 3) )
    {
        m_print(ERROR, " len error: ret[%d], BUF_SIZE[%d]\n", ret, BUF_SIZE);
        return;
    }

    /* 标识字节 */
    buf_src[0] = (((unsigned char)mod_id) & 0x0F) << 4;
    buf_src[0] |= (ptn_id & 0x0F);

    /* 编码, 从端usb buf为64字节  */
    if ( mod_grp->encode->encrypt_len_limit(&buf_src[1], ret, buf_dst, &len_dst, buf_src[0], 64) < 0 )
    {
        m_print(ERROR, " encrypt error:\n");
        dump_data(buf_src, ret+1);
        return;
    }
//	printf("pipe encrypte data:\n");
//	dump_data(buf_dst, len_dst);
    pthread_mutex_lock(&mod_grp->g_lock);
    /* 写串口 */
    res = write(mod_grp->usb_com.fd, buf_dst, len_dst);
    pthread_mutex_unlock(&mod_grp->g_lock);
    if(res < 0) {
        m_print(ERROR, "[%d]write cmd or at data to com serial failed!\n", pchan->hw_port);
    }
    return;
}

/**********************************************************
函数描述 : 打开调试信息输出文件
输入参数 : mod_grp -- 模块组首指针
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.28
************************************************************/
int module_group_debug_close(MODULE_GROUP_ST *mod_grp)
{
    int i;
    MODULE_CHAN_ST *pchan;

    for ( i = 0; i < MOD_NUM; i++ )
    {
        pchan = &mod_grp->chans[i];

        if ( pchan->dbg_flag_at == 0  )
            continue;
        else{
            pchan->dbg_flag_at = 0;
        }
        if ( pchan->dbg_fd_at > 0 )
        {
            close(pchan->dbg_fd_at);
            pchan->dbg_fd_at = -1;
        }

        if ( pchan->dbg_fd_cmd > 0 )
        {
            close(pchan->dbg_fd_cmd);
            pchan->dbg_fd_cmd = -1;
        }
    }

    return 0;
}


/**********************************************************
函数描述 : 打开调试信息输出文件
输入参数 : mod_grp -- 模块组首指针
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.28
************************************************************/
int module_group_debug_snd_close(MODULE_GROUP_ST *mod_grp)
{
    int i;
    MODULE_CHAN_ST *pchan;

    for ( i = 0; i < MOD_NUM; i++ )
    {
        pchan = &mod_grp->chans[i];

        if ( pchan->dbg_flag_snd == 0  )
            continue;
        else
            pchan->dbg_flag_snd = 0;
        
	
        if ( pchan->dbg_fd_snd_rx > 0 )
        {
            close(pchan->dbg_fd_snd_rx);
            pchan->dbg_fd_snd_rx = -1;
        }

        if ( pchan->dbg_fd_snd_tx > 0 )
        {
            close(pchan->dbg_fd_snd_tx);
            pchan->dbg_fd_snd_tx = -1;
        }

    }

    return 0;
}
/**********************************************************
函数描述 : 打开调试信息输出文件
输入参数 : mod_grp -- 模块组首指针
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.28
************************************************************/
int module_group_debug_open(MODULE_GROUP_ST *mod_grp)
{
    int i;
    char *p;
    MODULE_CHAN_ST *pchan;
    char path[BUF_SIZE];
    char dbg_name[NAME_LEN];

    /* 逆初始化 */
    (void)module_group_debug_close(mod_grp);

    /* 设备名称使用串口名称 */
    p = &mod_grp->usb_com.dev_name[strlen(mod_grp->usb_com.dev_name) - 1];

    while ( (*p != '/') && (p >= (&mod_grp->usb_com.dev_name[0])) )
        p--;

    p++;
    i = 0;
    memset(dbg_name, 0, sizeof(dbg_name));

    while ( *p )
        dbg_name[i++] = *p++;

    /* 创建调试文件 */
    for ( i = 0; i < MOD_NUM; i++ )
    {
        pchan = &mod_grp->chans[i];
        /* at debug 文件 */
        sprintf(path, "%s%s-%d%s", MODULE_PIPE_DIR, dbg_name, pchan->hw_port, FILE_DEBUG_AT);
        if ( (pchan->dbg_fd_at = open(path, O_CREAT|O_WRONLY|O_APPEND)) < 0 )
    	{
    		m_print(ERROR, "open debug at file fail! path = %s, fd = %d\n", path, pchan->dbg_fd_at);
    		return -1;
    	}

        /* cmd debug 文件 */
        sprintf(path, "%s%s-%d%s", MODULE_PIPE_DIR, dbg_name, pchan->hw_port, FILE_DEBUG_CMD);
        if ( (pchan->dbg_fd_cmd = open(path, O_CREAT|O_WRONLY|O_APPEND)) < 0 )
    	{
    		m_print(ERROR, "open debug cmd file fail! path = %s, fd = %d\n", path, pchan->dbg_fd_cmd);
    		return -1;
    	}
    }

    return 0;
}

/**********************************************************
函数描述 : 打开调试信息输出文件
输入参数 : mod_grp -- 模块组首指针
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: wengang.mu / 2018.04.25
************************************************************/
int module_group_debug_snd_open(MODULE_GROUP_ST *mod_grp)
{
    int i;
    char *p;
    MODULE_CHAN_ST *pchan;
    char path[BUF_SIZE];
    char dbg_name[NAME_LEN];

    /* 逆初始化 */
    (void)module_group_debug_snd_close(mod_grp);

    /* 设备名称使用串口名称 */
    p = &mod_grp->usb_com.dev_name[strlen(mod_grp->usb_com.dev_name) - 1];

    while ( (*p != '/') && (p >= (&mod_grp->usb_com.dev_name[0])) )
        p--;

    p++;
    i = 0;
    memset(dbg_name, 0, sizeof(dbg_name));

    while ( *p )
        dbg_name[i++] = *p++;

    /* 创建调试文件 */
    for ( i = 0; i < MOD_NUM; i++ )
    {
        pchan = &mod_grp->chans[i];
        /* 语音debug文件 */
        sprintf(path, "%s%s-%d%s", MODULE_PIPE_DIR, dbg_name, pchan->hw_port, FILE_DEBUG_READ);
        if ( (pchan->dbg_fd_snd_rx = open(path, O_CREAT|O_WRONLY|O_APPEND)) < 0 )
    	{
    		m_print(ERROR, "open debug read file fail! path = %s, fd = %d\n", path, pchan->dbg_fd_snd_rx);
    		return -1;
    	}

        sprintf(path, "%s%s-%d%s", MODULE_PIPE_DIR, dbg_name, pchan->hw_port, FILE_DEBUG_WRITE);
        if ( (pchan->dbg_fd_snd_tx = open(path, O_CREAT|O_WRONLY|O_APPEND)) < 0 )
    	{
    		m_print(ERROR, "open debug write file fail! path = %s, fd = %d\n", path, pchan->dbg_fd_snd_tx);
    		return -1;
    	}

    }

    return 0;
}

/**********************************************************
函数描述 : 为所有要读写的文件句柄创建fds，用于线程中poll
输入参数 : 
输出参数 : 无
返回值   : 
作者/时间: zhongwei.peng / 2017.06.24
************************************************************/
void module_group_fds_init(void)
{
    int i;

    for ( i = 0; i < g_fds_num; i++ )
    {
        g_fds[i].events  = POLLIN|POLLPRI;
        g_fds[i].revents = 0;
    }
}

/**********************************************************
函数描述 : 为所有要读写的文件句柄创建fds，用于线程中poll
输入参数 : 
输出参数 : 无
返回值   : 
作者/时间: zhongwei.peng / 2017.06.24
************************************************************/
void module_group_fds_create(void)
{
    int i, j, k, num;
    unsigned int alive_group_num = 0;
    MODULE_GROUP_ST *mod_grp;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        if ( g_module_groups[i]->alive ) 
            alive_group_num++;
    }

    /* 申请空间 */
    /* fd个数 = 有效模块组个数 * (1个串口读通道 + 2个gsm模块 * 3种通道类型 * 1个读fd) */
    g_fds_num = alive_group_num * (1 + MOD_NUM * PTN_NUM);
    if ( g_fds_num == 0 )
        return;

    g_fds = (struct pollfd *)malloc(sizeof(struct pollfd) * g_fds_num);
    if ( NULL == g_fds )
        return;

    num = 0;
    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        mod_grp = g_module_groups[i];

        if ( mod_grp->alive == 0 )
            continue;

        /* 串口 */
        g_fds[num].fd = mod_grp->usb_com.fd;
        g_fds[num].events  = POLLIN|POLLPRI;
        g_fds[num].revents = 0;
        num++;

        /* pipe 文件 */
        for ( j = 0; j < MOD_NUM; j++ )
        {
            for ( k = 0; k < PTN_NUM; k++ )
            {
                g_fds[num].fd = mod_grp->chans[j].pipes[k].fds[PIFD_R];
                g_fds[num].events  = POLLIN|POLLPRI;
                g_fds[num].revents = 0;
                num++;
            }
        }
    } 

    return;
}

/**********************************************************
函数描述 : 模块板数据透传处理线程
输入参数 : param -- 模块组首指针
输出参数 : 无
返回值   : 无
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
void *module_group_thread(void *param)
{
    int i, j, k;
    //int res;
    MODULE_GROUP_ST *mod_grp;

    (void)param;

    while ( g_thread_switch )
	{
	    if ( g_fds_num > 0 )
        {   
            //module_group_fds_init();
            usleep(15000);
        }
        else
        {
    	    sleep(5); /* 释放cpu */
            continue;
        }
#if 0
        res = poll(g_fds, g_fds_num, 30); /* 超时时间30ms */
        if ( res < 0 )
        {
            m_print("poll error: res = %d\n", res);
            continue;
        }
#endif
	    for ( i = 0; i < g_mod_grp_num; i++ )
        {
            mod_grp = g_module_groups[i];

            if ( mod_grp->alive == 0 )
                continue;

            /* 串口数据接收处理 */
            com_recv_handle(mod_grp);

            /* pipe数据接收处理 */
            for ( j = 0; j < MOD_NUM; j++ )
            {
                for ( k = 0; k < PTN_NUM; k++ )
                {
                    pipe_file_recv_handle(mod_grp, j, k);
                }
            }
        }

        /* 检查超时 */
        com_recv_timeout();

	}// end while
	return (void *)0;
}

/**********************************************************
函数描述 : 检查module节点是否存在
输入参数 : name -- 串口设备名(没使用)
           p_hw_port -- 物理端口号(字符形式)
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2017.06.25
************************************************************/
int module_group_check(char *name, char *p_hw_port)
{
    int i, j;
    MODULE_GROUP_ST *mod_grp = NULL;
    int hwport = 0;

    (void)name;

    if ( (NULL == p_hw_port) || (g_mod_grp_num == 0) )
        return -1;

    /* 物理端口是从1开始的 */
    hwport = atoi(p_hw_port);
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        mod_grp = g_module_groups[i];

        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( mod_grp->chans[j].hw_port == hwport )
            {
                if ( mod_grp->alive )
                    return 0;
                else
                    return -1;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 重新打开指定串口
输入参数 : hwport -- 物理端口号
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2017.09.22
************************************************************/
int module_group_com_reopen(int hwport)
{
    int i, j;
    MODULE_GROUP_ST *mod_grp = NULL;

    if ( g_mod_grp_num == 0 )
        return -1;

    /* 物理端口是从1开始的 */
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        mod_grp = g_module_groups[i];

        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( mod_grp->chans[j].hw_port == hwport )
            {
                mod_grp->usb_com.reopen_flag = 1;
                return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 初始化模块板(组)数据结构
输入参数 : mod_grp -- 模块板(组)首指针，内部调用不检查参数
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
void module_group_init(MODULE_GROUP_ST *mod_grp)
{
    int i, j, k;

    mod_grp->alive = 0;

    memset(mod_grp->usb_com.dev_name, 0, sizeof(mod_grp->usb_com.dev_name));
    mod_grp->usb_com.fd = -1;
    mod_grp->usb_com.buf_rx_len = 0;
    mod_grp->usb_com.reopen_flag = 0;
    mod_grp->rx_timeout_cnt = 0;
    memset(&mod_grp->usb_rx_time,0, sizeof(mod_grp->usb_rx_time));
    pthread_mutex_init(&mod_grp->g_lock, NULL);
    for ( i = 0; i < MOD_NUM; i++ )
    {
        for ( j = 0; j < PTN_NUM; j++ )
        {
            for ( k = 0; k < PIFD_NUM; k++ )
            {
                mod_grp->chans[i].pipes[j].fds[k] = -1;
                memset(mod_grp->chans[i].pipes[j].name, 0, sizeof(mod_grp->chans[i].pipes[j].name));
            }

            mod_grp->chans[i].dbg_flag_at = 0;
            mod_grp->chans[i].dbg_flag_snd = 0;
            mod_grp->chans[i].dbg_fd_snd_rx   = -1;
            mod_grp->chans[i].dbg_fd_snd_tx   = -1;
            mod_grp->chans[i].dbg_fd_at  = -1;
            mod_grp->chans[i].dbg_fd_cmd = -1;
            
            mod_grp->chans[i].rx_snd_len = 0;
            mod_grp->chans[i].rx_at_len = 0;
            mod_grp->chans[i].rx_at_state = 0;
            mod_grp->chans[i].upgrade_flag = 0;
	     mod_grp->chans[i].state = 0;
            mod_grp->chans[i].tx_snd_speed = 20;
            mod_grp->chans[i].tx_snd_delay = 40;
//            mod_grp->chans[i].tx_data_buf.size = BUF_SIZE;
            tx_snd_data_init(&mod_grp->chans[i].tx_snd_buf);
            mod_grp->chans[i].mod_id = i;
            mod_grp->chans[i].mod = mod_grp;
        }
    }
}

/**********************************************************
函数描述 : 释放模块板(组)数据结构
输入参数 : mod_grp -- 模块板(组)首指针，内部调用不检查参数
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
void module_group_deinit(MODULE_GROUP_ST *mod_grp)
{
    int i, j;
    
    for ( i = 0; i < MOD_NUM; i++ )
    {
        for ( j = 0; j < PTN_NUM; j++ )
        {
            pipe_close_fd(&mod_grp->chans[i].pipes[j]);
        }

        mod_grp->chans[i].rx_at_len = 0;
        mod_grp->chans[i].rx_snd_len = 0;
        mod_grp->chans[i].rx_at_state = 0;
        tx_snd_data_deinit(&mod_grp->chans[i].tx_snd_buf);
    }

    if ( mod_grp->usb_com.fd >= 0 )
    {
        close(mod_grp->usb_com.fd);
        mod_grp->usb_com.fd = -1;
    }
    pthread_mutex_destroy(&mod_grp->g_lock);

    module_group_debug_close(mod_grp);
    module_group_debug_snd_close(mod_grp);
}

/**********************************************************
函数描述 : 模块组释放空间
输入参数 : 
输出参数 : 
返回值   : 
作者/时间: zhongwei.peng / 2017.06.23
************************************************************/
void module_group_device_deinit(void)
{
    int i;

    if ( g_module_groups == NULL )
        return;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        module_group_deinit(g_module_groups[i]);
        free(g_module_groups[i]);
    }
    
    free(g_module_groups);
    g_module_groups = NULL;
}


/**********************************************************
函数描述 : 注册编码，解码回调
输入参数 : 
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: wenggang.mu / 2018.10.29
************************************************************/
int module_group_encode_init(MODULE_GROUP_ST *mod_grp){
	char context[32] = {0};
	char domain[16] = {0};
	sprintf(context, "chan_%d_protocol", mod_grp->chans[0].hw_port );
	if(1 == get_config( MODULE_MAP_FILE, "channel", context, domain)){
		if(strcmp(domain, "cobs") == 0){
			mod_grp->encode = &encode_cobs;
		}else{
			mod_grp->encode = &encode_hdlc;
		}
	}else{//default: hdlc encrypt
		mod_grp->encode = &encode_hdlc;
	}
	return 0;
}

/**********************************************************
函数描述 : 装载所有module group节点
输入参数 : 
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2017.06.24
************************************************************/
int module_group_setup(void)
{
    int i, j, k;
    char pipe_name[NAME_LEN];
    MODULE_GROUP_ST *mod_grp = NULL;
    MODULE_CHAN_ST *mod_chan = NULL;

    if ( g_mod_grp_num == 0 )
        return -1;

    for ( k = 0; k < g_mod_grp_num; k++ )
    {
        mod_grp = g_module_groups[k];
        /* 打开串口设备 */
        if ( (mod_grp->usb_com.fd = com_new(mod_grp->usb_com.dev_name, USB_COM_BAUD)) < 0 )
        {
            m_print(WARN, "open com fail! k = %d, name = %s\n", k, mod_grp->usb_com.dev_name);
            mod_grp->alive = 0;
            //continue;
        }
        else
        {
            m_print(INFO, "open com %s[%d] ok!\n", mod_grp->usb_com.dev_name, mod_grp->usb_com.fd);
            mod_grp->alive = 1;
        }

        /* 创建与asterisk之间通信管道 */
        for ( i = 0; i < MOD_NUM; i++ )
        {
            mod_chan = &mod_grp->chans[i];
            mod_chan->state = 0;

            for ( j = 0; j < PTN_NUM; j++ )
            {
                memset(pipe_name, 0, sizeof(pipe_name));
                sprintf(pipe_name, "%s-%d", module_pipe_names[j], mod_chan->hw_port);

                if ( pipe_new(&mod_chan->pipes[j], pipe_name) < 0 )
                {
                    module_group_deinit(mod_grp);
                    mod_grp->alive = 0;
                    return -1;  /* pipe都创建不成功，严重错误 */
                }
            }
        }

        /* 创建调试文件 */
		module_group_debug_open(mod_grp);
		module_group_debug_snd_open(mod_grp);
		module_group_encode_init(mod_grp);
    }

    return 0;
}

/**********************************************************
函数描述 : 从配置文件读模块设备名和物理端口号
输入参数 : 
输出参数 : 
返回值   : 成功返回0，失败返回-1
作者/时间: liyezhen / 2018.2.5
************************************************************/
int module_group_read_device(void)
{
    int hwport;
    int mnum;
    int hwport_num = 0;
	char context[32] = {0};
	char domain[16] = {0};
    int i, j;

	 /* 获取当前端口数 */
	if(1 != get_config(MODULE_MAP_FILE,"sys","total_chan_count",context))
	{
        m_print(ERROR, "read file[%s] fail\n", MODULE_MAP_FILE);
		return -1;
	}

	/* 不为0，偶数个，不超64  */
	hwport_num = atoi(context);
    if ( (0 == hwport_num ) || ((hwport_num & 1) != 0) || (hwport_num >= HW_PORT_MAX)  )
    {
        m_print(ERROR, "error: hw port sum = %d!\n", hwport_num);
        return -1;
    }

	g_hwport_num = hwport_num;
	m_print(INFO, "hwport num = %d\n", hwport_num);

	/* 申请空间 */
	g_mod_grp_num = hwport_num / 2;
	g_module_groups = (MODULE_GROUP_ST **)malloc(sizeof(MODULE_GROUP_ST *) * g_mod_grp_num);
	if ( NULL == g_module_groups )
	{
		m_print(ERROR, "%s: malloc module groups fail!\n", __FUNCTION__);
		return -1;
	}

	for ( i = 0; i < g_mod_grp_num; i++ )
	{
		g_module_groups[i] = (MODULE_GROUP_ST *)malloc(sizeof(MODULE_GROUP_ST));
		if ( NULL == g_module_groups[i] )
		{
			m_print(ERROR, "%s: malloc module group fail! i = %d\n", __FUNCTION__, i);
			for ( j = 0; j < i; j++ )
				free(g_module_groups[j]);
			free(g_module_groups);
			return -1;
		}
		module_group_init(g_module_groups[i]);
	}

	/* 获取物理端口号 */
	for(i = 0;i < hwport_num;i++)
	{
		sprintf(domain,"chan_%d",i+1);
		memset(context,0,sizeof(context));
		if(1 == get_config(MODULE_MAP_FILE,"channel",domain,context))
		{
			j = i / 2;			/* 组下标 */
			mnum = i % MOD_NUM; /* 模块下标 */
			hwport = i + 1;
			g_module_groups[j]->chans[mnum].hw_port = hwport;
			memset(g_module_groups[j]->usb_com.dev_name,0,NAME_LEN);
			memcpy(g_module_groups[j]->usb_com.dev_name,context,strlen(context));
			m_print(INFO, "dev-%d: hwport = %d, dev=%s\n", i+1, g_module_groups[j]->chans[mnum].hw_port, g_module_groups[j]->usb_com.dev_name);
			
		}
		else{
			g_module_groups[j]->chans[mnum].hw_port = 0;
			memset(g_module_groups[j]->usb_com.dev_name,0,NAME_LEN);
		}
	}
	m_print(INFO, "\n%s: complete!\n", __FUNCTION__);
	return 0;
}

/**********************************************************
函数描述 : 启动指定chan的语音数据通讯
输入参数 : hwport -- 物理端口号
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
int svr_chan_voice_start(int hwport)
{
    int i, j;
    char buf_src[BUF_SIZE];

    /* 物理端口是从1开始的 */
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
		    g_module_groups[i]->chans[j].rx_snd_len = 0;
		    gettimeofday(&g_module_groups[i]->chans[j].rx_time, NULL);
		    g_module_groups[i]->chans[j].rx_time.tv_sec += 61;  /* 刚开始，61秒超时 */
		    g_module_groups[i]->chans[j].state = 1;

		    sprintf(buf_src, "voice start");
		    com_cmd_send(hwport, (unsigned char *)buf_src, strlen(buf_src));
		    m_print(INFO, "chn[%d]voice start\n", hwport);
		    g_module_groups[i]->chans[j].tx_event_flag= 1;
		    g_module_groups[i]->chans[j].tx_snd_buf.in = 0;
		    g_module_groups[i]->chans[j].tx_snd_buf.out = 0;
		    g_module_groups[i]->chans[j].snd_buf_empty = 0;
		    g_module_groups[i]->chans[j].snd_buf_full = 0;
		    return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 停止指定chan的语音数据通讯
输入参数 : hwport -- 物理端口号
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
int svr_chan_voice_stop(int hwport)
{
    int i, j;
  //  int fd;
    char buf_src[BUF_SIZE];

    /* 物理端口是从1开始的 */
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
                if ( g_module_groups[i]->chans[j].state != 0 )
                {                   
			sprintf(buf_src, "voice stop");
			g_module_groups[i]->chans[j].state = 0;
			com_cmd_send(hwport, (unsigned char *)buf_src, strlen(buf_src));
			m_print(INFO, "chn[%d]voice stop\n", hwport);
			g_module_groups[i]->chans[j].tx_event_flag= 0;
                }
                return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 启动指定chan的语音数据通讯
输入参数 : hwport -- 物理端口号
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
int svr_chan_update_start(int hwport)
{
    int i, j;
    char buf_src[BUF_SIZE];

    /* 物理端口是从1开始的 */
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {

		    sprintf(buf_src, "updata start");
		    com_cmd_send(hwport, (unsigned char *)buf_src, strlen(buf_src));
		    m_print(INFO, "chn[%d]updata start\n", hwport);

                return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 停止指定chan的语音数据通讯
输入参数 : hwport -- 物理端口号
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.25
************************************************************/
int svr_chan_update_stop(int hwport)
{
    int i, j;
  //  int fd;
    unsigned char buf_src[BUF_SIZE];

    /* 物理端口是从1开始的 */
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
                                   
		    sprintf(buf_src, "updata stop");
		    com_cmd_send(hwport, (unsigned char *)"updata stop", strlen(buf_src));
		    m_print(INFO, "chn[%d]updata stop\n", hwport);

                return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 设置指定chan的调试标志
输入参数 : hwport -- 物理端口号
           val -- 要设置的值
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.10.18
************************************************************/
int svr_chan_debug_set(int hwport, int val)
{
    int i, j;

    if ( (val != 0) && (val != 1) )
        return -1;

    /* 物理端口是从1开始的 */
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
		    g_module_groups[i]->chans[j].dbg_flag_at = val;
		    m_print(INFO, "set at debug ok, hw_port = %d, val = %d\n", hwport, val);
		    return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 设置指定chan的调试标志
输入参数 : hwport -- 物理端口号
           val -- 要设置的值
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: wengang.mu / 2018.04.25
************************************************************/
int svr_chan_debug_snd_set(int hwport, int val)
{
    int i, j;

    if ( (val != 0) && (val != 1) )
        return -1;

    /* 物理端口是从1开始的 */
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
		    g_module_groups[i]->chans[j].dbg_flag_snd = val;
		    m_print(INFO, "set sound debug ok, hw_port = %d, val = %d\n", hwport, val);
		    return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : server通道写消息
输入参数 : msg -- 消息
           len -- 消息长度
输出参数 : 无
返回值   : 退出程序标志，0代表退出
作者/时间: zhongwei.peng / 2016.11.24
************************************************************/
void svr_write_msg(char *msg, int len)
{
    (void)write(g_pipe_svr.fds[PIFD_W], msg, len);
}

/**********************************************************
函数描述 : server通道消息处理
输入参数 : msg -- 消息
输出参数 : 无
返回值   : 退出程序标志，0代表退出
作者/时间: zhongwei.peng / 2016.11.24
************************************************************/
int svr_msg_handle(viod)
{
    int res;
    int val;
    int argc;
    char *argv[10];
    char *p;
    static char s_buf[BUF_SIZE];

    memset(s_buf, 0, BUF_SIZE);
	res = read(g_pipe_svr.fds[PIFD_R], s_buf, BUF_SIZE-1);
	if ( res <= 0 )
        return -1;

    /* 格式化字符串，把空格分开的词放入多个字符串 */
    p = s_buf;
    while ( *p && (' ' == *p) ) p++;

    if ( *p == '\0' )
        return -1;

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

            if ( argc >= 10 )
                return -1;
        }

        p++;
    }

    /* 处理消息 */
    if ( strcmp("new", argv[0]) == 0 )
    {
        if ( argc == 3 )
        {
            m_print(INFO,"msg: %s %s %s\n", argv[0], argv[1], argv[2]);

            if ( module_group_check(argv[1], argv[2]) == 0 )
                svr_write_msg("OK", 2);
            else
                svr_write_msg("fail", 4);
        }
    }
    else if ( strcmp("start", argv[0]) == 0 )
    {
        if ( argc == 2 )
        {
//            write_time(g_fd_svr_dbg);
            m_print(INFO, "msg: %s %s\n", argv[0], argv[1]);

            if ( svr_chan_voice_start(atoi(argv[1])) >= 0 )
                svr_write_msg("OK", 2);
            else
                svr_write_msg("fail", 4);
        }
    }
    else if ( strcmp("stop", argv[0]) == 0 )
    {
        if ( argc == 2 )
        {
//            write_time(g_fd_svr_dbg);
            m_print(INFO, "msg: %s %s\n", argv[0], argv[1]);

            if ( svr_chan_voice_stop(atoi(argv[1])) >= 0 )
                svr_write_msg("OK", 2);
            else
                svr_write_msg("fail", 4);
        }
    }
    else if ( strcmp("svr_debug", argv[0]) == 0 )
    {
        if ( argc == 2 )
        {
            m_print(INFO, "msg: %s %s\n", argv[0], argv[1]);
            val = atoi(argv[1]);

            if ( (val >= 0) && (val <= 1) )
            {
                g_server_debug = val;
                svr_write_msg("OK", 2);
            }
            else
            {
                svr_write_msg("fail", 4);
            }
        }
    }
    else if ( strcmp("set_debug", argv[0]) == 0 )
    {
        if ( argc == 3 )
        {
            m_print(INFO,"msg: %s %s %s\n", argv[0], argv[1], argv[2]);

            if ( svr_chan_debug_set(atoi(argv[1]), atoi(argv[2])) >= 0 )
                svr_write_msg("OK", 2);
            else
                svr_write_msg("fail", 4);
        }
    }
    else if ( strcmp("reopen_com", argv[0]) == 0 )
    {
        if ( argc == 2 )
        {
            m_print(INFO,"msg: %s %s\n", argv[0], argv[1]);

            if ( module_group_com_reopen(atoi(argv[1])) >= 0 )
                svr_write_msg("OK", 2);
            else
                svr_write_msg("fail", 4);
        }
    }
    else if ( strcmp("quit", argv[0]) == 0 )
    {
        return 0; /* 退出程序 */
    }
    else
    {
        m_print(WARN, "unknown msg: %s\n", argv[0]);
    }

    return 1;
}

/**********************************************************
函数描述 : 初始化 server 调试信息输出文件
输入参数 : 
输出参数 : 无
返回值   : 成功返回0, 失败返回-1
作者/时间: zhongwei.peng / 2016.11.28
************************************************************/
int debug_init()
{
#define RRI_LOG_FILE "/tmp/log/rri_server.log"
//    char path[BUF_SIZE];

    if(g_fd_svr_dbg != NULL){
	fclose(g_fd_svr_dbg);
	g_fd_svr_dbg = NULL;
   }
   g_fd_svr_dbg = fopen(RRI_LOG_FILE, "a+");
   if(g_fd_svr_dbg == NULL)
   	return -1;

   zsys_set_logstream(g_fd_svr_dbg);
#if 0   
    /* 逆初始化 */
    if ( g_fd_svr_dbg >= 0 )
    {
        close(g_fd_svr_dbg);
        g_fd_svr_dbg = -1;
    }

    sprintf(path, "%s%s", MODULE_PIPE_DIR, SERVER_DEBUG_FILE);
    if ( (g_fd_svr_dbg = open(path, O_CREAT|O_WRONLY|O_APPEND)) < 0 )
		return -1;
    else
        return 0;
#endif
	return 0;
}

/**********************************************************
函数描述 : 获取物理通道个数
输入参数 : 
输出参数 : 无
返回值   : 通道个数
作者/时间: zhongwei.peng / 2017.12.28
************************************************************/
unsigned int svr_get_hwport_num(void)
{
    return g_hwport_num;
}

void svr_global_debug_set(int val)
{
    g_server_debug = val;
}

int svr_get_pipe_names
(
    int hwport, 
    char *audio_r, 
    char *audio_w, 
    char *at_r, 
    char *at_w, 
    char *cmd_r, 
    char *cmd_w
)
{
    int i, j;
    PIPE_CTRL_ST *ppipe = NULL;
//    char buf_src[BUF_SIZE];

    /* 物理端口是从1开始的 */
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        if ( g_module_groups[i]->alive == 0 )
            continue;

        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
                ppipe = &(g_module_groups[i]->chans[j].pipes[PTN_SND]);
                sprintf(audio_r, "%s%s%s", MODULE_PIPE_DIR, ppipe->name, FILE_PIPE_WRITE); /* 读跟写反过来，下同 */
                sprintf(audio_w, "%s%s%s", MODULE_PIPE_DIR, ppipe->name, FILE_PIPE_READ);

                ppipe = &(g_module_groups[i]->chans[j].pipes[PTN_AT]);
                sprintf(at_r, "%s%s%s", MODULE_PIPE_DIR, ppipe->name, FILE_PIPE_WRITE);
                sprintf(at_w, "%s%s%s", MODULE_PIPE_DIR, ppipe->name, FILE_PIPE_READ);

                ppipe = &(g_module_groups[i]->chans[j].pipes[PTN_CMD]);
                sprintf(cmd_r, "%s%s%s", MODULE_PIPE_DIR, ppipe->name, FILE_PIPE_WRITE);
                sprintf(cmd_w, "%s%s%s", MODULE_PIPE_DIR, ppipe->name, FILE_PIPE_READ);
                return 0;
            }
        }
    }

    return -1;
}

/**********************************************************
函数描述 : 获取通道AT指令信息
输入参数 : 
输出参数 : bound--波特率,如果通道不存在,返回0
           x_on_off--奇偶校验位
返回值   : 无
作者/时间: wengang.mu / 2018.03.26
************************************************************/

int svr_get_at_port_info(int hwport, int *bound, unsigned char *x_on_off){
	//如果通道不存在，直接返回，否则返回默认的信息
	if(hwport < 0 || hwport > HW_PORT_MAX){
		*bound = 0;
		*x_on_off = 0;
		return -1;
	}
	
	*bound = atoi(USB_COM_BAUD);
	*x_on_off = 0;
	return 0;
}

/**********************************************************
函数描述 : 获取通道调试信息
输入参数 : chn--通道号
输出参数 : status--1代表调试模式
返回值   : 0--成功
        -1--失败
作者/时间: wengang.mu / 2018.03.26
************************************************************/

int svr_get_debug_port_info(int hwport, int *bound, unsigned char *x_on_off){
	int i, j;

	*bound = 0;
	
	*x_on_off = 0;
	
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
            //如果是调试状态，返回预先设置的波特率，以及是否校验的信息
            	if(g_module_groups[i]->chans[j].dbg_flag_at == 1 || g_module_groups[i]->chans[j].dbg_flag_snd == 1){
                	*bound = atoi(USB_COM_BAUD);
					*x_on_off = 0;
                	return 0;
            	}
				return -1;
            }
        }
    }
	return -1;

}

/**********************************************************
函数描述 : 获取通道信息
输入参数 : chn--通道号
输出参数 : status--1代表调试模式
返回值   : 0--成功
        -1--失败
作者/时间: wengang.mu / 2018.03.26
************************************************************/

int svr_get_upgrade_port_info(int hwport, int *bound, unsigned char *x_on_off){
	int i, j;

	*bound = 0;
	*x_on_off = 0;
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
                //如果是在升级模式下，则返回预先设置的串口的波特率和校验位信息，否则直接返回错误
            	if(g_module_groups[i]->chans[j].upgrade_flag == 1){
                	*bound = atoi(USB_COM_BAUD);
			*x_on_off = 0;
                	return 0;
            	}
			return -1;
            }
        }
    }
	return -1;
}

/**********************************************************
函数描述 : 获取通道信息
输入参数 : chn--通道号
输出参数 : audioStatus--通道语音状态，0表示不再通信状态
		   atStatus--通道at指令状态
		   debugSatus--通道调试状态，1表示调试状态
		   upgradStatus--升级状态，1表示升级状态
返回值   : 0--成功
        -1--失败
作者/时间: wengang.mu / 2018.03.26
************************************************************/

int svr_get_channel_connect_state(int hwport, int *audioStatus, int *atStatus, int *debugStatus, int *upgradStatus){
	int i, j;

    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
            	//语音通话状态，=0表示非通话状态
            	*audioStatus = g_module_groups[i]->chans[j].state;
				//at指令状态，< 3表示不在at短信状态
				*atStatus = g_module_groups[i]->chans[j].rx_at_state;
				//通道调试状态，=0代表不在调试状态
		*debugStatus = g_module_groups[i]->chans[j].dbg_flag_at | g_module_groups[i]->chans[j].dbg_flag_snd;
				//通道升级状态
		*upgradStatus = g_module_groups[i]->chans[i].upgrade_flag;
            	return 0;
            }
        }
    }
	return -1;
}

/**********************************************************
函数描述 : 设置通道升级状态
输入参数 : hwport--物理端口号
           flag--升级标志，1表示设置为升级模式，0表示设置为非升级模式
返回值   : 0--成功
           -1--失败
作者/时间: wengang.mu / 2018.04.18
*********************************************************/
int svr_chan_upgrade_set(int hwport, int flag){
	int i, j;
	if((hwport < 1)||(hwport > HW_PORT_MAX)){
		m_print(ERROR, "hwport input error!\n");
		return -1;
	}
	if(flag != 0 && flag != 1){
		m_print(ERROR, "flag error, flag = %d\n", flag);
		return -1;
	}
	for(i = 0; i < g_mod_grp_num; i++)
	{
		for(j = 0; j < MOD_NUM; j++)
		{
			if(g_module_groups[i]->chans[j].hw_port == hwport)
			{
                //如果是在通信状态或者发送短信状态，则不能设置为升级模式
				if(g_module_groups[i]->chans[j].state > 0 || g_module_groups[i]->chans[j].rx_at_state > RECEIVED_AT_END)
				{
					m_print(ERROR, "can't set upgrade mode, state = %d, rx_at_state = %d\n", g_module_groups[i]->chans[j].state, g_module_groups[i]->chans[j].rx_at_state);
					return -1;
				}
				else
				{
					if(flag == 0)
						svr_chan_update_stop(hwport);
					else
						svr_chan_update_start(hwport);
					g_module_groups[i]->chans[j].upgrade_flag = flag;
					return 0;
				}
			}
		}
	}
	return -1;
}
/*???û???????С????С??2??n?η?*/
int svr_set_channel_txsnd_buf_size(int hwport, int bufsize){
    int i = 0, j = 0;
  
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) || bufsize <= 0|| ((bufsize & (bufsize-1)) != 0) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
           //??????ͨ??״̬?£?????????buf??С
            	if(g_module_groups[i]->chans[j].state != 0){
                    return -1;
            	}else{
                    g_module_groups[i]->chans[j].tx_snd_buf.size = bufsize;
                    return tx_snd_data_init(&g_module_groups[i]->chans[j].tx_snd_buf);
               }
            }
        }
    }
	return -1;

}
/*??ȡ????????С*/
int svr_get_channel_txsnd_buf_size(int hwport, int *bufsize){
    int i = 0, j = 0;
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
                *bufsize = g_module_groups[i]->chans[j].tx_snd_buf.size;
                return 0;
            }
        }
    }
	return -1;
}
/*???ô???????ʱ??*/
int svr_set_channel_txsnd_speed(int hwport, int speed){
    int i = 0, j = 0;
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) || speed <= 0)
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
                g_module_groups[i]->chans[j].tx_snd_speed = speed;
                return 0;
            }
        }
    }
    return -1;
}
/*??ȡ????????ʱ??*/
int svr_get_channel_txsnd_speed(int hwport, int *speed){
    int i = 0, j = 0;
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
               *speed =  g_module_groups[i]->chans[j].tx_snd_speed;
                return 0;
            }
        }
    }
    return -1;
}

/*???õ?һ??????????ʱ*/
int svr_set_channel_txsnd_delay(int hwport, int delay){
    int i = 0, j = 0;
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) || delay <= 0)
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
                g_module_groups[i]->chans[j].tx_snd_delay = delay;
                return 0;
            }
        }
    }
    return -1;
}
/*??ȡ??һ??????????ʱ*/
int svr_get_channel_txsnd_delay(int hwport, int *delay){
    int i = 0, j = 0;
    if ( (hwport < 1) || (hwport > HW_PORT_MAX) )
        return -1;

    for ( i = 0; i < g_mod_grp_num; i++ )
    {
        for ( j = 0; j < MOD_NUM; j++ )
        {
            if ( g_module_groups[i]->chans[j].hw_port == hwport )
            {
               *delay =  g_module_groups[i]->chans[j].tx_snd_delay;
                return 0;
            }
        }
    }
    return -1;
}

static void rri_get_log_level(char *level_file){
//	char buf[1024] = {0};
	char *line = NULL;
	int level = 0;
	size_t len = 0;
	if(level_file == NULL){
		g_server_debug= 0;
		return;
	}
	if(access(level_file, F_OK) != 0){
		g_server_debug = 0;
		return;
	}

	FILE *fp = fopen(level_file, "r");
	if(fp == NULL){
		g_server_debug = 0;
		return;
	}

	while(getline(&line, &len, fp) > 0){
		switch(line[0]){
			case '\n':
			case '\0':
			case '#':
				continue;
			case '[':
				if(strncmp(line, "[rri-log]", 9) == 0){
					if(getline(&line, &len, fp) < 0)
						return ;
					if(strstr(line, "notice")){
						level |= NOTICE;
					}
					if(strstr(line, "info")){
						level |= INFO;
					}
					if(strstr(line, "debug")){
						level |= DEBUG;
					}
					if(strstr(line, "warning")){
						level |= WARN;
					}
					if(strstr(line, "error")){
						level |= ERROR;
					}
					break;
				}else
					continue;
			default:
				continue;
		}
		break;
	}
	g_server_debug = level;
	
	if(line)
		free(line);
	fclose(fp);
	
}

void *mcu_detect(void *data){
	int i = 0;
	struct timeval time_now;
	char cmd_buf[128];
	MODULE_GROUP_ST *p_mod;
	while(g_thread_switch){
		gettimeofday(&time_now, NULL);
		for(i = 0; i < g_mod_grp_num;i++){
			p_mod = g_module_groups[i];
			if(!p_mod->alive)//not detect
				continue;
			if(p_mod->rx_timeout_cnt >= 2){//rx_timeout_cnt need to clear zero 
			       m_print(ERROR, "mcu reset\n");
				com_cmd_send(p_mod->chans[0].hw_port, (unsigned char *)"mcu_reset", strlen("mcu_reset"));
				p_mod->rx_timeout_cnt = 0;
				sleep(4);
				sprintf(cmd_buf, "/my_tools/gen_mcu_device_link.sh %d", p_mod->chans[0].hw_port);
				system(cmd_buf);
				p_mod->usb_com.reopen_flag = 1;
				continue;
			}
			
			if(time_now.tv_sec - p_mod->usb_rx_time.tv_sec > 60){//need test ver control cmd
				com_cmd_send(p_mod->chans[0].hw_port, (unsigned char *)"ver", strlen("ver"));
				p_mod->rx_timeout_cnt++;
			}else
				p_mod->rx_timeout_cnt = 0;
		}
		sleep(50);// bigger than (time_now.tv_sec - p_mod->usb_rx_time.tv_sec)
	}
	return (void *)NULL;	
}

void sig_handle(int sig){
	if(sig == SIGTERM|| sig == SIGINT || sig == SIGQUIT)
		ctrl_thr_flag = 0;
}

int main(int argc, char *argv[])
{
    //int i;
    int res;
    char file_name[BUF_SIZE];
    unsigned char buf[BUF_SIZE];
    struct pollfd fds;
    pthread_t mod_thd_id, mod_audio_thd_id, mcu_detect_id;
    pthread_attr_t mod_thd_attr; // 线程参数

    g_mod_grp_num = 0;
    g_module_groups = NULL;

    if ( debug_init() < 0 )
        return -1;

    /* 读配置文件，申请空间 */
    if ( module_group_read_device() < 0 )
    {
        m_print(ERROR, "module_group_read_device fail\n");
        return -1;
    }

    /* 装载module group, 包括打开设备，创建pipe等 */
    if ( module_group_setup() < 0 )
    {
        module_group_device_deinit();
        m_print(ERROR, "module_group_device_deinit fail\n");
        return -1;
    }

    module_group_fds_create();

    /* 创建线程 */
    pthread_attr_init(&mod_thd_attr);
    pthread_attr_setinheritsched(&mod_thd_attr, PTHREAD_INHERIT_SCHED);
    pthread_attr_setstacksize(&mod_thd_attr, 262144); /* 堆栈大小256k */
    
    g_thread_switch = 1;
    if ( pthread_create(&mod_thd_id, &mod_thd_attr, module_group_thread, NULL) != 0 )
    {
        m_print(ERROR, "create thread fail\n");
        module_group_device_deinit();
        return -1;
    }

    if ( pthread_create(&mod_audio_thd_id, &mod_thd_attr, module_group_audio_thread, NULL) != 0 )
    {
        m_print(ERROR, "create audio thread fail\n");
        module_group_device_deinit();
        return -1;
    }
    
    if( pthread_create(&mcu_detect_id, &mod_thd_attr, mcu_detect, NULL) != 0 ){
        m_print(ERROR, "create mcu detect error\n");
        module_group_device_deinit();
        return -1;
    }
    /* 创建server pipe */
    if ( pipe_new(&g_pipe_svr, "server") < 0 )
    {
        m_print(ERROR, "create server pipe fail!\n");
        module_group_device_deinit();
        return -1;
    }

    /* 创建rri server */
    soap_server_start();

    fds.events  = POLLIN|POLLPRI;
    fds.revents = 0;
    fds.fd      = g_pipe_svr.fds[PIFD_R];

    rri_get_log_level(LOG_LEVEL_FILE);
    ctrl_thr_flag = 1;
    zsys_handler_set(sig_handle);
    while ( ctrl_thr_flag )
	{
		res = poll(&fds, 1, -1);
		if ( res > 0 )
		{
			if ( fds.revents & (POLLIN|POLLPRI) )
			{
                ctrl_thr_flag = svr_msg_handle(buf);
			}
            else
            {
                /* 有异常，重新打开一次 */
                m_print(ERROR, "fd error: res[%d], r.revents[0x%x], w.revents[0x%x]\n", 
                    res, fds.revents, fds.revents);
                close(fds.fd);
                g_pipe_svr.fds[PIFD_R] = -1;
                sprintf(file_name, "%s%s%s", MODULE_PIPE_DIR, g_pipe_svr.name, FILE_PIPE_READ);
            	if ( (g_pipe_svr.fds[PIFD_R] = pipe_open(file_name, O_RDONLY)) < 0 )
            	{
            		m_print(ERROR, "reopen read pipe file fail! file_name = %s, fd = %d\n", file_name, g_pipe_svr.fds[PIFD_R]);
            		break;
            	}
                else
                {
                    fds.events  = POLLIN|POLLPRI;
                    fds.revents = 0;
                    fds.fd      = g_pipe_svr.fds[PIFD_R];
                }
            }
		}
		else
		{
			m_print(ERROR, "poll fail: res = %d\n", res);
		}
	}

    g_thread_switch = 0;
    sleep(1);

    module_group_device_deinit();

    pipe_close_fd(&g_pipe_svr);
    fclose(g_fd_svr_dbg);
//    fclose(rri_svr_log_fd);
    return 0;
}


