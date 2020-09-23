#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <sys/termios.h>

#include "ahead.h"
#include "typedef.h"


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



/******************************************************************************
Function	:	open_serial
Description	:	打开串口
Input		:	serial_file		串口文件
Return		:	成功：返回串口描述符 失败：返回：-1
******************************************************************************/
sint32 open_serial(char *serial_file);

/******************************************************************************
Function	:	open_serial
Description	:	打开串口
Input		:	serial_file		串口文件
Return		:	成功：返回串口描述符 失败：返回：-1
******************************************************************************/
sint32 open_serial(char *serial_file, uint32 rate, uint8 parity, uint8 databits, uint8 stopbits, uint8 streamcontrol);

/******************************************************************************
Function	:	close_serial
Description	:	关闭串口
Input		:	serial_fd		串口文件描述符
Return		:	成功：返回0 失败：返回：-1
******************************************************************************/
sint32 close_serial(uint32 serial_fd);

/******************************************************************************
Function	:	get_termios
Description	:	获取串口termios
Input		:	serial_fd		串口文件描述符
				old_term		串口termios数据结构
Return		:	成功：返回0 失败：返回：-1
******************************************************************************/
sint32 get_termios(uint32 serial_fd, struct termios *old_term);

/******************************************************************************
Function	:	set_termios
Description	:	设置串口termios
Input		:	serial_fd		串口文件描述符
				old_term		串口termios数据结构
Return		:	成功：返回0 失败：返回：-1
******************************************************************************/
sint32 set_termios(uint32 serial_fd, struct termios *old_term);

/******************************************************************************
Function	:	set_serial_attr
Description	:	设置串口属性
Input		:	serial_fd		串口文件描述符
				serial_attr		串口属性数据结构
Return		:	成功：返回0 失败：返回：-1
******************************************************************************/
sint32 set_serial_attr(uint32 serial_fd, serial_attr_t *serial_attr);

/******************************************************************************
Function	:	serial_read_n
Description	:	读串口
Input		:	serial_fd		串口文件描述符
				buf				数据缓存
				len				读出长度
				timeout			读超时
Return		:	成功：返回读出数据长度 失败：返回：-1
******************************************************************************/
sint32 serial_read_n(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout);

/******************************************************************************
Function	:	serial_write_n
Description	:	写串口
Input		:	serial_fd		串口文件描述符
				buf				数据缓存
				len				写入长度
				timeout			写超时
Return		:	成功：返回写入数据长度 失败：返回：-1
******************************************************************************/
sint32 serial_write_n(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout);

/******************************************************************************
Function	:	serial_wr
Description	:	读写串口
Input		:	serial_fd		串口文件描述符
				buf				数据缓存
				len				写入长度
				timeout			写超时
Return		:	成功：返回写入数据长度 失败：返回：-1
******************************************************************************/
sint32 serial_wr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout);
sint32 serial_wr_atr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout);
sint32 serial_read_n_atr(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout);

int read_from_mini51(uint32 serial_fd, uint8 *buf_r, uint32 len_r, uint32 timeout);
int write_to_mini51(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint32 timeout);
int serial_wr_mini51(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout);





#endif
