#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <alloca.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/stat.h>   
#include <termios.h>
#include <pthread.h>
#define POSIX_TERMIOS

#include "chn_upgrade.h"
#include "bsp_api.h"
#include "rri_api.h"

//#define _DEBUG
#define _INFO

#ifdef _DEBUG
#define DBG(format,...) printf("%s/L:%d: " format "\n",__func__,__LINE__,##__VA_ARGS__)
#else
#define DBG(format,...) 
#endif

#ifdef _INFO
#ifdef _DEBUG
#define INFO(format,...) printf("%s/L:%d: "format "\n",__func__,__LINE__,##__VA_ARGS__)
#else
#define INFO(format,...) printf(format "\n",##__VA_ARGS__)
#endif
#else
#define INFO(format,...) 
#endif


#define FRAME_FLAG              (0x7E) /* 帧头帧尾标识 */
#define ESCAPE_WORD_0           (0x7F) /* 转义字符 */
#define ESCAPE_WORD_1           (0x7C) /* 同上 */


extern FILE* log_handle;
int last_size=0;

#ifdef _DEBUG
void SHOW_HEX(unsigned char *cmd, int len)
{
    int i;
    for(i = 0;i < len;i++)
    {
        if(i % 10 == 0 )
            printf("\n>>>> : ");
        printf("[%c] ",cmd[i]);
    }
}
#else
void SHOW_HEX(char *cmd, int len){}
#endif
static void dump_data(unsigned char *data, int len)
{
    int i;

    for ( i = 0; i < len; i++ )
    {
        if ( i % 8 == 0 )
            printf("\r\n");

        printf("%02x ", data[i]);
    }

    printf("\r\n");
}

#define SYNC_WORD_S1        0xb5
#define SYNC_WORD_S1_ACK    0x5b
#define SYNC_WORD_S2        0xa9
#define SYNC_WORD_S2_ACK    0x9a

#define CMD_DL_BEGIN        0x0001
#define CMD_DL_BEGIN_RSP    0x0002

#define CMD_DL_DATA         0x0003
#define CMD_DL_DATA_RSP     0x0004

#define CMD_DL_END          0x0005
#define CMD_DL_END_RSP      0x0006

#define CMD_RUN_GSMSW       0x0007
#define CMD_RUN_GSMSW_RSP   0x0008

#define CMD_HEAD 0xaa

#define STATUS_SUCCESS                  0
#define STATUS_CRC16_ERR                1
#define STATUS_FLASH_ERR                2
#define STATUS_MODULE_IN_DOWNLOAD_MODE  3
#define STATUS_DATA_PACKAGE_ERR         4

//#define _MTU 1024  
//unsigned short _MTU = 48;  
unsigned short _MTU = 1024;  
unsigned int BUF_SIZE = 48;

#define FAULT 0
#define TURE 1

//{{{   sync_MCU_and_module
int write_sync_word(int fd, char sync_word)
{
    char sync_buf[2];
    sync_buf[0] = sync_word;
    sync_buf[1] = '\n';
    return write(fd,sync_buf, 1);
    // return write(fd, "AT\n", 3);
}

int read_sync_word_ack(int fd, char sync_word_ack)
{
    DBG("Enter");
    #define READ_BUF_SIZE 100
    int ret;
    char buf[READ_BUF_SIZE];
    int i;

    ret = read(fd,buf,READ_BUF_SIZE);
    DBG("read return %d",ret);
    DBG(">> %c : %c<<",buf[0],sync_word_ack);
    if(ret >= 0)
    {
        printf("read sync len = %d, buf :\n", ret);
        dump_data(buf, ret);
    }

    if(ret < 0)
        return ret;
    if(ret == 0)
        return 1;

    for(i = 0; i < ret; i++)
        if(buf[i] == sync_word_ack)
            return 0;

    return -2;
}

int sync_word(int *fds, char sync_word, char sync_word_ack, int retry)
{
    int ret;

    while(retry-- > 0)
    {
        DBG("\033[33m Retry : %d\033[m",retry);
        DBG("Sync [0x%2X] ...",sync_word);
        ret = write_sync_word(fds[PIFD_AT_W], sync_word);
        DBG("write_sync_word return %d",ret);
        if(ret < 0)
        {
            continue;
            INFO("write SYNC_WORD[0x%2x] err: %s",sync_word,strerror(errno));
            return ret;
        }
        usleep(20 * 1000);
        ret = read_sync_word_ack(fds[PIFD_AT_R], sync_word_ack);
        DBG("read_sync_word_ack return %d",ret);
        if(ret < 0)
        {
            continue;
            INFO("read SYNC_WORD_ACK[0x%2X] err: %s",sync_word_ack,strerror(errno));
            continue;
            return ret;
        }
        if(ret == 1)
            continue;
        
        if(ret == 0)
            break;
    }

    if(retry <= 0)
        return -1;
    DBG("Sync [%d] OK",sync_word);
    return 0;
}

int sync_MCU_and_module(int *fds)
{
    int ret;

    ret = sync_word(fds,SYNC_WORD_S1,SYNC_WORD_S1_ACK, 6000);
    /*ret = sync_word(fd,SYNC_WORD_S1,SYNC_WORD_S1_ACK,3000);*/
    printf("sync SYNC_WORD_S1(0x%x)\n", SYNC_WORD_S1);
    if(ret < 0)
        return -1;
   
    ret = sync_word(fds,SYNC_WORD_S2,SYNC_WORD_S2_ACK, 10);
    /*ret = sync_word(fd,SYNC_WORD_S2,SYNC_WORD_S2_ACK,10);*/
    printf("sync SYNC_WORD_S2(0x%x)\n", SYNC_WORD_S2);
    if(ret < 0)
        return -2;

    DBG("Sync OK ...");
    return 0;
}
//}}}

//{{{   get_crc16
#if 1
unsigned short get_crc16(unsigned char *data, int n)
{
	unsigned int CRC16Table[256]={               
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
	};
	
	unsigned int crc16 = 0;
    unsigned char crcregister;
	unsigned char *pData_In = data;
	int i;

	if(n <= 0)
		return 0;

    for(i = 0; i < n; i++)
	{
        crcregister = (unsigned int)crc16 >> 8;
        crc16 <<= 8;
        crc16 ^= CRC16Table[crcregister ^ *pData_In];
        pData_In++;
    }
	
    return (crc16 & 0xFFFF);
}
#else
#define CRC_SEED   0xFFFF   // 该位称为预置值，使用人工算法（长除法）时 需要将除数多项式先与该与职位 异或 ，才能得到最后的除数多项式
#define POLY16 0x1021  // 该位为简式书写 实际为0x11021
short get_crc16(unsigned char *buf,unsigned short length)
{
    short shift,data,val;
    int i;

    shift = CRC_SEED;

    for(i=0;i<length;i++) 
    {
        if((i % 8) == 0)
            data = (*buf++)<<8;
        val = shift ^ data;
        shift = shift<<1;
        data = data <<1;
        if(val&0x8000)
            shift = shift ^ POLY16;
    }
    return shift;
} 
#endif
//}}}

//{{{   begin_download
void encode_cmd_dl_begin(int version, char *cmd)
{
    #define VERSION_LENGTH 4
    char *p = cmd;
    short crc16;
    //Head
    p[0] = CMD_HEAD;

    //type
    p++;
    p[0] = (CMD_DL_BEGIN >> 8) & 0xff;
    p[1] = CMD_DL_BEGIN & 0xff;

    //length
    p += 2;
    p[0] = (VERSION_LENGTH >> 8) & 0xff;
    p[1] = VERSION_LENGTH & 0xff;

    //data
    p += 2;
    p[0] = (version >> 24) & 0xff;
    p[1] = (version >> 16) & 0xff;
    p[2] = (version >> 8) & 0xff;
    p[3] = version & 0xff;

    //crc16
    p += 4;
    crc16 = get_crc16((unsigned char *)(&cmd[1]), 2 + 2 + VERSION_LENGTH);
    p[0] = (crc16 >> 8) & 0xff;
    p[1] = crc16 & 0xff;
}

int decode_cmd_dl_begin_rsp(unsigned char *rsp)
{
    int i;
    char *p;
    short type;
    short length;
    short status;
    short MTU;

    for(i = 0;rsp[i] == CMD_HEAD; i++)
        break;
    p = &(rsp[i]);

    p++;
    type = (p[0] << 8) + p[1];
    if(type != CMD_DL_BEGIN_RSP)
    {
        INFO("Not CMD_DL_BEGIN_RSP:%d",type);
        return -1;
    }

    p += 2;
    length = (p[0] << 8) + p[1];
    if(length != 4)
    {
        INFO("Length != 4 : %d",length);
        return -2;
    }

    p += 2;
    status = (p[0] << 8) + p[1];
    //...

    p+=2;
    MTU = (p[0] << 8) + p[1];
    DBG("MTU is %d",MTU);

    /*_MTU = MTU;*/

    //CRC...
    
    return status;
}

int begin_download(int *fds)
{
    #define CMD_DL_BEGIN_LENGTH 11
    #define CMD_DL_BEGIN_RSP_LENGTH 11
    #define FW_VERSION 0x4d4d4d01 
    unsigned char cmd[CMD_DL_BEGIN_LENGTH + 1],rsp[CMD_DL_BEGIN_LENGTH + 1];
    int ret,count = 0;

    encode_cmd_dl_begin(FW_VERSION,cmd);

    SHOW_HEX(cmd, CMD_DL_BEGIN_LENGTH);

    ret = write(fds[PIFD_AT_W],cmd,CMD_DL_BEGIN_LENGTH);
    DBG("write return %d",ret);
    if(ret < 0)
    {
        INFO("write CMD_DL_BEGIN err: %s",strerror(errno));
        return ret;
    }

    while(1)
    {
        ret = read(fds[PIFD_AT_R],rsp + count,CMD_DL_BEGIN_RSP_LENGTH);
        if(ret < 0)
        {
            continue;
            INFO("read CMD_DL_BEGIN_RSP err:%s",strerror(errno));
            return ret;
        }
        count += ret;
        if(count < CMD_DL_BEGIN_RSP_LENGTH)
            continue;

        SHOW_HEX(rsp,count);
        
        int i = 0;
        while(i < count)
        {
            DBG("rsp[%d]:%d,CMD_HEAD:%d",i,rsp[i],CMD_HEAD);
            if(rsp[i] == CMD_HEAD)
                break;
            i++;
        }
        DBG("find head:%d/%d",count,i);
        if(count - i < CMD_DL_BEGIN_RSP_LENGTH)
            continue;

        ret = decode_cmd_dl_begin_rsp(rsp);
        DBG("decode_cmd_dl_begin_rsp return %d",ret);
        switch(ret)
        {
            case STATUS_SUCCESS:
                return ret;
            break;
            case STATUS_CRC16_ERR:
                INFO("CRC16 err");
                return -ret;
            break;
            case STATUS_FLASH_ERR:
                INFO("Flash err");
                return -ret;
            break;
            case STATUS_MODULE_IN_DOWNLOAD_MODE:
                INFO("Module is in download mode");
                return -ret;
            break;
            case STATUS_DATA_PACKAGE_ERR:
                INFO("Data package err");
                return -ret;
            break;
            default:
                INFO("decode_cmd_dl_begin_rsp return status: %d should never happen",ret);
            return -ret;
            break;
        }
    }

    return 0;
}
//}}}

//{{{   downloading
int open_fw(char *fw_path)
{
    if(fw_path == NULL)
        return -1;
    DBG("fw_path:%s",fw_path);
    return (open(fw_path,O_RDWR));
}

int read_fw(int fw_fd, char *buf, int count)
{
    return (read(fw_fd,buf,count));
}

void encode_cmd_dl_data(char *cmd, char *buf, int count,int seq)
{
    char *p = cmd;
    short crc16;
    DBG("enter");

    if(cmd == NULL)
        DBG("cmd null");
    if(buf == NULL)
        DBG("buf null");

    p[0] = CMD_HEAD;

    p++;
    p[0] = (CMD_DL_DATA >> 8) & 0xff;
    p[1] = CMD_DL_DATA & 0xff;

    p += 2;
    p[0] = ((count + 4) >> 8) & 0xff;
    p[1] = (count + 4) & 0xff;

    p += 2;
    p[0] = (seq >> 24) & 0xff;
    p[1] = (seq >> 16) & 0xff;
    p[2] = (seq >> 8) & 0xff;
    p[3] = seq & 0xff;

    p += 4;
    memcpy(p,buf,count);
	//除开帧头之外的信息
    crc16 = get_crc16((unsigned char *)(cmd + 1),8 + count);//(2 + 2 + 4 + count);
    p += count;
    p[0] = (crc16 >> 8) & 0xff;
    p[1] = crc16 & 0xff;
}

int write_cmd_dl_data(int fd, char *buf, int count, int seq)
{
    char *cmd = NULL;
    //head=1 type-2 crc=2 length=2 seq=4  crc=2
    cmd = (char *)alloca(11 + count);//(1 + 2 + 2 + 4 + count + 2);
    if(cmd == NULL)
    {
        INFO("alloca return NULL");
        return -1;
    }

    encode_cmd_dl_data(cmd,buf,count,seq);
    DBG("encode_cmd_dl_data out(len = %d)", 11 + count);

    return (write(fd,cmd,11 + count));
}

int decode_cmd_dl_data_rsp(char *rsp,int *ack_seq)
{
    int i;
    unsigned char *p;
    short type;
    short length;
    short status;

    for(i = 0;rsp[i] == CMD_HEAD; i++)
        break;
    p = &(rsp[i]);

    p++;
    type = (p[0] << 8) + p[1];
    if(type != CMD_DL_DATA_RSP)
    {
        INFO("Not CMD_DL_DATA_RSP:%d",type);
        return -1;
    }

    p += 2;
    length = (p[0] << 8) + p[1];
    if(length != 6)
    {
        INFO("Length != 6 : %d",length);
        return -2;
    }

    p += 2;
    status = (p[0] << 8) + p[1];
    //...

    p+=2;
    *ack_seq = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];

    //CRC...
    
    return status;
}

int read_cmd_dl_data_ack(int fd, int *ack_seq)
{
    #define CMD_DL_DATA_RSP_LENGTH 13
    unsigned char rsp[CMD_DL_DATA_RSP_LENGTH];
    int count = 0;
    int ret;

    while(1)
    {
        ret = read(fd,rsp + count,CMD_DL_DATA_RSP_LENGTH);
        if(ret < 0)
        {
            continue;
            INFO("read CMD_DL_DATA_RSP err:%s",strerror(errno));
            return ret;
        }
        count += ret;
        DBG("count = %d\n",count);
        if(count < CMD_DL_DATA_RSP_LENGTH)
            continue;
        DBG("%d/%d",count,CMD_DL_DATA_RSP_LENGTH);    

        int i = 0;
        while(i < count)
        {
            DBG("rsp[%d] = %d CMD_HEAD = %d",i,rsp[i],CMD_HEAD);
            if(rsp[i] == CMD_HEAD)
                break;
            i++;
        }
        DBG("find head %d/%d/%d",count,i,CMD_DL_DATA_RSP_LENGTH);
        if(count - i < CMD_DL_DATA_RSP_LENGTH)
            continue;

        ret = decode_cmd_dl_data_rsp(rsp,ack_seq);
        return ret;
    }

    return 0;
}

static int get_file_size(int fd) 
{  
        struct stat tbuf;  
        fstat(fd, &tbuf);  
        return tbuf.st_size;  
}  

#if 0
#define PRINT_PROGRESS_BAR(progress,total) do{\
    int i;\
    for(i = 0; i < progress * 100 / total; i++)\
        printf("\033[?25l\033[42m ");\
    for(i = 0;i < 100 - progress * 100 / total; i++)\
        printf("\033[41m ");\
    printf("\033[m\033[35m %d%% \033[m\r",progress * 100 / total);\
}while(0)
#else
#if 0
#define PRINT_PROGRESS_BAR(progress,total) do{\
    int i;\
    for(i = 0; i < progress * 50 / total; i++)\
        printf("*");\
    for(i = 0;i < 50 - progress * 50 / total; i++)\
        printf("-");\
    printf(" %d%% \r",progress * 100 / total);\
}while(0)
#endif
#define PRINT_PROGRESS_BAR(progress, total) do{\
    fseek(log_handle,0-last_size, SEEK_CUR);\
    last_size = fprintf(log_handle, "%d",progress * 100/total);\
}while(0)
#endif

int downloading(int *fds, char *fw_path)
{
    char buf[BUF_SIZE];
    int fw_fd;
    int data_len;
    int seq = 0,ack_seq;
    int reach_end = 0;
    int ret;
    unsigned int fw_size;
    
    data_len = BUF_SIZE > (_MTU - 4) ? (_MTU -4) : BUF_SIZE;

    fw_fd = open_fw(fw_path);
    if(fw_fd < 0)
    {
        INFO("open %s err: %s",fw_path,strerror(errno));
        return fw_fd;
    }
    DBG("Open fw_path success");

    fw_size = get_file_size(fw_fd);
    DBG("\033[m33 fw_size = %d\033[m",fw_size);

    while(1)
    {
        if(reach_end == 1)
            break;
		//每次读取48字节
        ret = read_fw(fw_fd,buf,data_len);
        if(ret < 0)
            return ret;
        if(ret == 0)
            break;
        if(ret < data_len)
        {
            reach_end = 1;
            data_len = ret;
        }

retry:
        DBG("write fw");
        ret = write_cmd_dl_data(fds[PIFD_AT_W], buf, data_len, seq);
        DBG("data_len = %d, write_cmd_dl_data return %d", data_len, ret);
        if(ret < 0)
            return ret;

      //  PRINT_PROGRESS_BAR(seq,(int)(fw_size * 1.0 / BUF_SIZE));
        PRINT_PROGRESS_BAR(seq,(int)(fw_size * 1.0 / BUF_SIZE));
    //    fflush(stdout);

        ret = read_cmd_dl_data_ack(fds[PIFD_AT_R],&ack_seq);
        DBG("\r\rSeq:%d, Wrote Data Length:%ld, Total:%d",seq,(long)seq * BUF_SIZE + data_len,fw_size);
        DBG("read_cmd_dl_data_ack return %d",ret);
        switch(ret)
        {
            case STATUS_SUCCESS:
            #if 0
            if(ack_seq == ++seq)
                continue;
            else
                return -5;
            #else
                ++seq;
                continue;
            #endif
            break;
            case STATUS_CRC16_ERR:
                INFO("CRC16 err");
                goto retry;
                return -ret;
            break;
            case STATUS_FLASH_ERR:
                INFO("Flash err");
                return -ret;
            break;
            case STATUS_MODULE_IN_DOWNLOAD_MODE:
                INFO("Module is in download mode");
                return -ret;
            break;
            case STATUS_DATA_PACKAGE_ERR:
                INFO("Data package err");
                goto retry;
                return -ret;
            break;
            default:
                INFO("read_data_dl_ack return status: %d should never happen",ret);
                return -ret;
            break;
        }
    }

    DBG("exit");
    return 0;
}
//}}}

//{{{  end_download
void encode_cmd_dl_end(unsigned char *cmd)
{
    unsigned char *p = cmd;
    short crc16;
    //Head
    p[0] = CMD_HEAD;

    //type
    p++;
    p[0] = (CMD_DL_END >> 8) & 0xff;
    p[1] = CMD_DL_END & 0xff;

    //length
    p += 2;
    p[0] = 0x00;
    p[1] = 0x00;

    //crc16
    p += 2;
    crc16 = get_crc16((unsigned char *)cmd + 1,2 + 2);
    p[0] = (crc16 >> 8) & 0xff;
    p[1] = crc16 & 0xff;
}

int decode_cmd_dl_end_rsp(unsigned char *rsp)
{
    int i;
    unsigned char *p;
    short type;
    short length;
    short status;

    for(i = 0;rsp[i] == CMD_HEAD; i++)
        break;
    p = &(rsp[i]);

    p++;
    type = (p[0] << 8) + p[1];
    if(type != CMD_DL_END_RSP)
    {
        INFO("Not CMD_DL_END_RSP:%d",type);
        return -1;
    }

    p += 2;
    length = (p[0] << 8) + p[1];
    if(length != 2)
    {
        INFO("Length != 2 : %d",length);
        return -2;
    }

    p += 2;
    status = (p[0] << 8) + p[1];
    //...

    //CRC...
    
    return status;
}

int end_download(int *fds)
{
    #define CMD_DL_END_LENGTH 7
    #define CMD_DL_END_RSP_LENGTH 9
    unsigned char cmd[CMD_DL_END_LENGTH + 1],rsp[CMD_DL_END_RSP_LENGTH + 1];
    int ret,count = 0;

    DBG("Enter");
    encode_cmd_dl_end(cmd);
    DBG("encode_cmd_dl_end out");

    SHOW_HEX(cmd,CMD_DL_END_LENGTH);
    ret = write(fds[PIFD_AT_W],cmd,CMD_DL_END_LENGTH);
    if(ret < 0)
    {
        INFO("write CMD_DL_END err: %s",strerror(errno));
        return ret;
    }

    while(1)
    {
        ret = read(fds[PIFD_AT_R],rsp + count,CMD_DL_END_RSP_LENGTH);
        if(ret < 0)
        {
            continue;
            INFO("read CMD_DL_END_RSP err:%s",strerror(errno));
            return ret;
        }
        DBG("count = %d",count);
        count += ret;
        if(count < CMD_DL_END_RSP_LENGTH)
            continue;
        DBG("count = %d CMD_DL_END_RSP = %d",count,CMD_DL_END_RSP_LENGTH);
        
        int i = 0;
        while(i < count)
        {
            DBG("rsp[%d] = %d CMD_HEAD = %d",i,rsp[i],CMD_HEAD);
            if(rsp[i] == CMD_HEAD)
                break;
            i++;
        }
        DBG("%d/%d/%d",count,i,CMD_DL_END_RSP_LENGTH);
        if(count - i < CMD_DL_END_RSP_LENGTH)
            continue;

        ret = decode_cmd_dl_end_rsp(rsp);
        DBG("decode_cmd_dl_end_rsp return %d",ret);
        switch(ret)
        {
            case STATUS_SUCCESS:
                return 0;
            break;
            case STATUS_CRC16_ERR:
                INFO("CRC16 err");
                return -ret;
            break;
            case STATUS_FLASH_ERR:
                INFO("Flash err");
                return -ret;
            break;
            case STATUS_MODULE_IN_DOWNLOAD_MODE:
                INFO("Module is in download mode");
                return -ret;
            break;
            case STATUS_DATA_PACKAGE_ERR:
                INFO("Data package err");
                return -ret;
            break;
            default:
                INFO("decode_cmd_dl_end_rsp return status: %d should never happen",ret);
            return -ret;
            break;
        }
    }

    return 0;
}
//}}}

//{{{   run_new_fw
void encode_cmd_run_gsmsw(unsigned char *cmd)
{
    unsigned char *p = cmd;
    short crc16;
    //Head
    p[0] = CMD_HEAD;

    //type
    p++;
    p[0] = (CMD_RUN_GSMSW >> 8) & 0xff;
    p[1] = CMD_RUN_GSMSW & 0xff;

    //length
    p += 2;
    p[0] = 0x00;
    p[1] = 0x00;

    //crc16
    p += 2;
    crc16 = get_crc16((unsigned char *)(cmd + 1),2 + 2);
    p[0] = (crc16 >> 8) & 0xff;
    p[1] = crc16 & 0xff;
}

int decode_cmd_run_gsmsw_rsp(unsigned char *rsp)
{
    int i;
    unsigned char *p;
    short type;
    short length;
    short status;

    for(i = 0;rsp[i] == CMD_HEAD; i++)
        break;
    p = &(rsp[i]);

    p++;
    type = (p[0] << 8) + p[1];
    if(type != CMD_RUN_GSMSW_RSP)
    {
        INFO("Not CMD_RUN_GSMSW_RSP:%d",type);
        return -1;
    }

    p += 2;
    length = (p[0] << 8) + p[1];
    if(length != 2)
    {
        INFO("Length != 2 : %d",length);
        return -2;
    }

    p += 2;
    status = (p[0] << 8) + p[1];
    //...

    //CRC...
    
    return status;
}

int run_new_fw(int *fds)
{
    #define CMD_RUN_GSMSW_LENGTH 7
    #define CMD_RUN_GSMSW_RSP_LENGTH 9
    unsigned char cmd[CMD_RUN_GSMSW_LENGTH + 1],rsp[CMD_RUN_GSMSW_LENGTH + 1];
    int ret,count = 0;

    DBG("enter");
    encode_cmd_run_gsmsw(cmd);
    SHOW_HEX(cmd, CMD_RUN_GSMSW_LENGTH);

    ret = write(fds[PIFD_AT_W],cmd,CMD_RUN_GSMSW_LENGTH);
    if(ret < 0)
    {
        INFO("write CMD_RUN_GSMSW err: %s",strerror(errno));
        return ret;
    }

    while(1)
    {
        ret = read(fds[PIFD_AT_R], rsp + count,CMD_RUN_GSMSW_RSP_LENGTH);
        if(ret < 0)
        {
            continue;
            INFO("read CMD_RUN_GSMSW_RSP_LENGTH err:%s",strerror(errno));
            return ret;
        }
        count += ret;
        DBG("count = %d CMD_RUN_GSMSW_RSP_LENGTH = %d",count,CMD_RUN_GSMSW_RSP_LENGTH);
        if(count < CMD_RUN_GSMSW_RSP_LENGTH)
            continue;
        
        int i = 0;
        while(i < count)
        {
            DBG("rsp[%d] = %d CMD_HEAD = %d",i,rsp[i],CMD_HEAD);
            if(rsp[i] == CMD_HEAD)
                break;
            i++;
        }
        DBG("%d/%d/%d",count,i,CMD_RUN_GSMSW_RSP_LENGTH);
        if(count - i < CMD_RUN_GSMSW_RSP_LENGTH)
            continue;

        ret = decode_cmd_run_gsmsw_rsp(rsp);
        DBG("decode_cmd_run_gsmsw_rsp return %d",ret);
        switch(ret)
        {
            case STATUS_SUCCESS:
                return 0;
            break;
            case STATUS_CRC16_ERR:
                INFO("CRC16 err");
                return -ret;
            break;
            case STATUS_FLASH_ERR:
                INFO("Flash err");
                return -ret;
            break;
            case STATUS_MODULE_IN_DOWNLOAD_MODE:
                INFO("Module is in download mode");
                return -ret;
            break;
            case STATUS_DATA_PACKAGE_ERR:
                INFO("Data package err");
                return -ret;
            break;
            default:
                INFO("decode_cmd_run_gsmsw_rsp return status: %d should never happen",ret);
            return -ret;
            break;
        }
    }

    return 0;
}
//}}}
void *powerkey_down_pthread(void *channel){

	sleep(4);
	
	int ret = module_powerkey_hign_low(*(int *)channel, 0, "upgrade");
	if(ret < 0){
		printf("set powerkey to low level failed!");
		return (void *)-1;
	}
	printf("set powerkey to low level successed!\n");
	return (void *)0;
}

int m35f_process_update(int *fds, int channel, char *fw_path)
{
    int ret;

    INFO("Start Sync ...");
	pthread_t u_thread;
	pthread_create(&u_thread, NULL, powerkey_down_pthread, (void *)&channel);
    ret = sync_MCU_and_module(fds);
    if(ret < 0)
    {
        fprintf(log_handle, "sync_MCU_and_module err:%d", ret);
        INFO("sync_MCU_and_module err:%d", ret);
        return -1;
    }
    INFO("Sync OK");
    INFO("Start Download ...");
    ret = begin_download(fds);
    if(ret < 0)
    {
        fprintf(log_handle,"begin_download err:%d", ret);
        INFO("begin_download err:%d", ret);
        return -2;
    }
    DBG("begin_download OK");

    ret = downloading(fds,fw_path);
    if(ret < 0)
    {
        fprintf(log_handle, "\ndownloading err:%d", ret);
        INFO("downloading err:%d", ret);
        return -3;
    }
    DBG("downloading OK");

    ret = end_download(fds);
    if(ret < 0)
    {
        fprintf(log_handle,"\nend_download err:%d", ret);
        INFO("end_download err:%d", ret);
        return -4;
    }
    DBG("end_download OK");
    INFO("\nDownload OK");

    INFO("Run New Firmware ...");
    ret = run_new_fw(fds);
    if(ret < 0)
    {
        fprintf(log_handle, "\nrun_new_fw err:%d", ret);
        INFO("run_new_fw err:%d", ret);
        return 1;
    }
    DBG("run_new_fw OK");
    
    return 0;
}

