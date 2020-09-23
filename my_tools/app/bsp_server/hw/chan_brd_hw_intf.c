/**********************************************************************************
Description : 实现与通道板(channel board)通信接口
Author      : junyu.yang@openvox.cn
Time        : 2018.01.31
Note        : 
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bsp_tools.h"

#define FRAME_FLAG                  (0x7E) /* 帧头帧尾标识 */
#define ESCAPE_WORD_0               (0x7F) /* 转义字符 */
#define ESCAPE_WORD_1               (0x7C) /* 同上 */
#define MOD_NUM                     (2)    /* 一个模块板有两个模块 */

#define BUF_SIZE               (1024)

/* 枚举管道的种类 */
typedef enum tagPIPE_TYPE_EN
{
    PTN_SND = 0, /* 音频 */
    PTN_AT,      /* AT指令 */
    PTN_CMD,     /* 与mcu收发命令 */
    PTN_NUM
}PIPE_TYPE_EN;


#define StartBlock()	(code_ptr = dst++, code = 1)
#define FinishBlock()	(*code_ptr = code)
/*cobs协议编码*/
int cobs_decrypt(unsigned char *src, int len_src, unsigned char *dst, int *len_dst){
	unsigned char *start = dst, *end = src + len_src;
	unsigned char code = 0xFF, copy = 0;
	//如果第一个字节是0，就认为是错误帧
	if(*src == 0){
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
	*len_dst = dst-start-1;//去掉最后一个分隔符0
	return 0;
}

/*cobs协议解码*/
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
	*len_dst = dst - start ;
	return 0;
}

/**********************************************************
函数描述 : 从模块串口读内容，自动去封装头尾
输入参数 : fd -- 串口文件句柄
输出参数 : buf -- 
           mod_id -- 模块id
返回值   : 成功返回>0代表读取到内容长度, 失败返回0
作者/时间: zhongwei.peng / 2017.03.03
************************************************************/
void dump_data(unsigned char *data, unsigned int len)
{
    unsigned int i;

    for ( i = 0; i < len; i++ )
    {
        if ( (i != 0) && (i % 4 == 0) )
        {
            if ( i % 8 == 0 )
                printf("\r\n");
            else
                printf("   ");
        }

        printf("%02x ", data[i] & 0xFF);
    }

    printf("\r\n");
}

/**********************************************************
函数描述 : 获取命令通道板命令返回值，基于cobs协议
输入参数 : fd --  通道板文件句柄
           buf -- 命令返回值缓冲区
           len -- 命令内容长度
           mod_id -- 通道模块板上的通道ID。
返回值   : 无
作者/时间: junyu.yang@openvox.cn / 2018.01.31
************************************************************/
int chan_brd_rx_cmd_ret_cobs(int fd, int *mod_id, char *ret_buf, int len, int *actual_len, int timeout)
{
    int ret;
    int try_count;
    int ptn_id = PTN_NUM;
    char *p;
    char *p_buf;
    char *p_end;
    char buf_read[BUF_SIZE] = {0};
	char buf_drc[BUF_SIZE] = {0};
	int drc_len = 0;
	int pos = 0;
	int success_flag = 0;
	int read_pos = 0;
	
    try_count = 0;

    /* 最多等待20ms */
    while ( try_count < timeout )
    {
		usleep(5000);
		try_count += 5;

        ret = read(fd, buf_read + read_pos, BUF_SIZE - read_pos - 1);
        if ( ret <= 0 ){
			if(success_flag == 1)
				break;
			else
				continue;
		}else{
			//check again after 5ms
			read_pos += ret;
			success_flag = 1;
		}
    }

	/*
    if ( ret <= 0 )
    {
        return -1;
    }
	*/
	if(success_flag == 0)
		return -1;
//	printf("\n dump read data:");
//	dump_data(buf_read, ret);
	//the 1st byte set to 0x00
	
	do{
		drc_len = 0;
		cobs_decrypt((unsigned char *)buf_read+pos, read_pos - pos, (unsigned char *)&buf_drc[1] , &drc_len);
		p = buf_drc;
		p_end = buf_drc + drc_len + 1;
		pos = pos + drc_len + 2;
		while(p < p_end){
			if(*p == 0x00)
			{
				p++;
				*mod_id = (*p >> 4) & 0x0F;
				ptn_id = *p & 0x0F;
				if ( (*mod_id < MOD_NUM) && (ptn_id == PTN_CMD) ){
					break;
				}else
					continue;
			}
			p++;
		}
		if(ptn_id == PTN_CMD)
			break;
    }while(pos < read_pos);

    if ( ptn_id != PTN_CMD ){
		return -1;
	}

    p++;
    p_buf = ret_buf;

    *actual_len = 0;
    while ( *p && (*p != 0x00) && (len-- >0 ))
    {
        *p_buf++ = *p++;
        (*actual_len)++;
    }

    return 0;
}

/**********************************************************
函数描述 : 往通道板发送命令。基于cobs协议
输入参数 : fd --  通道板文件句柄
           buf -- 命令
           len -- 命令内容长度
           mod_id -- 通道模块板上的通道ID。
返回值   : 无
作者/时间: junyu.yang@openvox.cn / 2018.01.31
************************************************************/
/*void com_module_write(int fd, unsigned char *buf, int len, int mod_id)*/
void chan_brd_tx_cmd_cobs(int fd, char *cmd, int len, int mod_id)
{
    int i, j, ret;
    int len_w = 0;
	unsigned char tmp[BUF_SIZE];
	//前面放1个无效指令，让单片机把前面的数据丢掉
    unsigned char buf_write[BUF_SIZE] = {0x03,0xff,0xff, 0x00}; 

    /* 先读空 */
    while ( (ret = read(fd, tmp, BUF_SIZE)) > 0 ) 
    {
        tmp[0] = '\0';
    }
	memset(tmp, 0, BUF_SIZE);
	
	tmp[0] = (mod_id << 4) | PTN_CMD;
	strcpy(&tmp[1], cmd);
    //printf("\n dump write data:");
    //dump_data(buf_write, len_w);
	cobs_encrypt(tmp, strlen(tmp), buf_write+4, &len_w);
    (void)write(fd, buf_write, len_w+4 );

    return;
}


/**********************************************************
函数描述 : 往通道板发送at命令，基于cobs协议
输入参数 : fd --  通道板文件句柄
           buf -- 命令
           len -- 命令内容长度
           mod_id -- 通道模块板上的通道ID。
返回值   : 无
作者/时间: wengang.mu@openvox.cn / 2018.04.19
************************************************************/
void chan_brd_tx_at_cobs(int fd, char *cmd, int len, int mod_id)
{
    int i, j, ret;
    int len_w;
    unsigned char buf_write[BUF_SIZE];
	unsigned char tmp[BUF_SIZE] = {0};

    /* 先读空 */
    while ( (ret = read(fd, buf_write, BUF_SIZE)) > 0 ) 
    {
        buf_write[ret] = '\0';
    }
	memset(buf_write, 0, BUF_SIZE);
    for ( i = 2, j = 0; j < len; i++, j++ )
        buf_write[i] = cmd[j];

    tmp[0] = (mod_id << 4) | PTN_AT;
	strcpy(&tmp[1], cmd);
    //printf("\n dump write data:");
    //dump_data(buf_write, len_w);
	cobs_encrypt(tmp, strlen(tmp), buf_write, &len_w);

    (void)write(fd, buf_write, len_w);

    return;
}

/**********************************************************
函数描述 : 获取命令通道板命令返回值,基于cobs协议
输入参数 : fd --  通道板文件句柄
           buf -- 命令返回值缓冲区
           len -- 命令内容长度
           mod_id -- 通道模块板上的通道ID。
返回值   : 无
作者/时间: weng.mu@openvox.cn / 2018.04.19
************************************************************/
int chan_brd_rx_at_ret_cobs(int fd, int *mod_id, char *ret_buf, int len, int *actual_len)
{
    int ret;
    int try_count;
    int ptn_id = PTN_NUM;
    char *p;
    char *p_buf;
    char *p_end;
    char buf_read[BUF_SIZE] = {0};
	char buf_drc[BUF_SIZE] = {0};
	int drc_len;

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
        return 0;
    }

    //printf("\n dump read data:");
    //dump_data(buf_read, ret);
	cobs_decrypt(buf_read, ret, &buf_drc[1] , &drc_len);
	
    p = buf_drc;
    p_end = buf_drc + drc_len+1;
    while ( p < p_end )
    {
        if ( *p == 0x00 )
        {
            p++;
            *mod_id = (*p >> 4) & 0x0F;
            ptn_id = *p & 0x0F;
            if ( (*mod_id < MOD_NUM) && (ptn_id == PTN_AT) )
                break;
            else
                continue;
        }
        p++;
    }

    if ( ptn_id != PTN_AT )
        return -1;

    p++;
    p_buf = ret_buf;

    *actual_len = 0;
    while ( *p && (*p != 0x00) && (len-- >0 ))
    {
        *p_buf++ = *p++;
        (*actual_len)++;
    }

    return 0;
}



/**********************************************************
函数描述 : 获取命令通道板命令返回值
输入参数 : fd --  通道板文件句柄
           buf -- 命令返回值缓冲区
           len -- 命令内容长度
           mod_id -- 通道模块板上的通道ID。
返回值   : 无
作者/时间: junyu.yang@openvox.cn / 2018.01.31
************************************************************/
int chan_brd_rx_cmd_ret_hdlc(int fd, int *mod_id, char *ret_buf, int len, int *actual_len, int timeout)
{
    int ret;
    int try_count;
    int ptn_id = PTN_NUM;
    char *p;
    char *p_buf;
    char *p_end;
    char buf_read[BUF_SIZE] = {0};
    int read_pos = 0;
    int success_flag = 0;

    try_count = 0;

    /* 最多等待40ms */
	while ( try_count < timeout )
	{
		usleep(5000);
		try_count += 5;

		ret = read(fd, buf_read + read_pos, BUF_SIZE - read_pos - 1);
		if ( ret <= 0 ){
			if(success_flag == 1)
				break;
			else
				continue;
		}else{
			success_flag = 1;
			read_pos += ret;
		}
	}

	/*
    if ( ret <= 0 )
    {
        return 0;
    }
	*/
	if(success_flag == 0)
		return -1;
//    printf("\n dump read data:");
//    dump_data(buf_read, read_pos);

    p = buf_read;
    p_end = buf_read + read_pos;
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
    p_buf = ret_buf;

    *actual_len = 0;
    while ( *p && (*p != FRAME_FLAG) && (len-- >0 ))
    {
        *p_buf++ = *p++;
        (*actual_len)++;
    }

    return 0;
}

/**********************************************************
函数描述 : 往通道板发送命令。
输入参数 : fd --  通道板文件句柄
           buf -- 命令
           len -- 命令内容长度
           mod_id -- 通道模块板上的通道ID。
返回值   : 无
作者/时间: junyu.yang@openvox.cn / 2018.01.31
************************************************************/
/*void com_module_write(int fd, unsigned char *buf, int len, int mod_id)*/
void chan_brd_tx_cmd_hdlc(int fd, char *cmd, int len, int mod_id)
{
    int i, j, ret;
    int len_w;
    unsigned char buf_write[BUF_SIZE];

    /* 先读空 */
    while ( (ret = read(fd, buf_write, BUF_SIZE)) > 0 ) 
    {
        buf_write[ret] = '\0';
    }

    for ( i = 2, j = 0; j < len; i++, j++ )
        buf_write[i] = cmd[j];

    len_w = len + 3;
    buf_write[0] = FRAME_FLAG;
    buf_write[len_w-1] = FRAME_FLAG;
    buf_write[1] = (mod_id << 4) | PTN_CMD;

    //printf("\n dump write data:");
    //dump_data(buf_write, len_w);

    (void)write(fd, buf_write, len_w);

    return;
}


/**********************************************************
函数描述 : 往通道板发送at命令。
输入参数 : fd --  通道板文件句柄
           buf -- 命令
           len -- 命令内容长度
           mod_id -- 通道模块板上的通道ID。
返回值   : 无
作者/时间: wengang.mu@openvox.cn / 2018.04.19
************************************************************/
void chan_brd_tx_at_hdlc(int fd, char *cmd, int len, int mod_id)
{
    int i, j, ret;
    int len_w;
    unsigned char buf_write[BUF_SIZE];

    /* 先读空 */
    while ( (ret = read(fd, buf_write, BUF_SIZE)) > 0 ) 
    {
        buf_write[ret] = '\0';
    }
	memset(buf_write, 0, BUF_SIZE);
    for ( i = 2, j = 0; j < len; i++, j++ )
        buf_write[i] = cmd[j];

    len_w = len + 3;
    buf_write[0] = FRAME_FLAG;
    buf_write[len_w-1] = FRAME_FLAG;
    buf_write[1] = (mod_id << 4) | PTN_AT;

    //printf("\n dump write data:");
    //dump_data(buf_write, len_w);

    (void)write(fd, buf_write, len_w);

    return;
}

/**********************************************************
函数描述 : 获取命令通道板命令返回值
输入参数 : fd --  通道板文件句柄
           buf -- 命令返回值缓冲区
           len -- 命令内容长度
           mod_id -- 通道模块板上的通道ID。
返回值   : 无
作者/时间: weng.mu@openvox.cn / 2018.04.19
************************************************************/
int chan_brd_rx_at_ret_hdlc(int fd, int *mod_id, char *ret_buf, int len, int *actual_len)
{
    int ret;
    int try_count;
    int ptn_id = PTN_NUM;
    char *p;
    char *p_buf;
    char *p_end;
    char buf_read[BUF_SIZE] = {0};

    try_count = 0;

    /* 最多等待20ms */
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
        return -1;
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
            if ( (*mod_id < MOD_NUM) && (ptn_id == PTN_AT) )
                break;
            else
                continue;
        }
        p++;
    }

    if ( ptn_id != PTN_AT )
        return -1;

    p++;
    p_buf = ret_buf;

    *actual_len = 0;
    while ( *p && (*p != FRAME_FLAG) && (len-- >0 ))
    {
        *p_buf++ = *p++;
        (*actual_len)++;
    }

    return 0;
}




