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
Description	:	�򿪴���
Input		:	serial_file		�����ļ�
Return		:	�ɹ������ش��������� ʧ�ܣ����أ�-1
******************************************************************************/
sint32 open_serial(char *serial_file);

/******************************************************************************
Function	:	open_serial
Description	:	�򿪴���
Input		:	serial_file		�����ļ�
Return		:	�ɹ������ش��������� ʧ�ܣ����أ�-1
******************************************************************************/
sint32 open_serial(char *serial_file, uint32 rate, uint8 parity, uint8 databits, uint8 stopbits, uint8 streamcontrol);

/******************************************************************************
Function	:	close_serial
Description	:	�رմ���
Input		:	serial_fd		�����ļ�������
Return		:	�ɹ�������0 ʧ�ܣ����أ�-1
******************************************************************************/
sint32 close_serial(uint32 serial_fd);

/******************************************************************************
Function	:	get_termios
Description	:	��ȡ����termios
Input		:	serial_fd		�����ļ�������
				old_term		����termios���ݽṹ
Return		:	�ɹ�������0 ʧ�ܣ����أ�-1
******************************************************************************/
sint32 get_termios(uint32 serial_fd, struct termios *old_term);

/******************************************************************************
Function	:	set_termios
Description	:	���ô���termios
Input		:	serial_fd		�����ļ�������
				old_term		����termios���ݽṹ
Return		:	�ɹ�������0 ʧ�ܣ����أ�-1
******************************************************************************/
sint32 set_termios(uint32 serial_fd, struct termios *old_term);

/******************************************************************************
Function	:	set_serial_attr
Description	:	���ô�������
Input		:	serial_fd		�����ļ�������
				serial_attr		�����������ݽṹ
Return		:	�ɹ�������0 ʧ�ܣ����أ�-1
******************************************************************************/
sint32 set_serial_attr(uint32 serial_fd, serial_attr_t *serial_attr);

/******************************************************************************
Function	:	serial_read_n
Description	:	������
Input		:	serial_fd		�����ļ�������
				buf				���ݻ���
				len				��������
				timeout			����ʱ
Return		:	�ɹ������ض������ݳ��� ʧ�ܣ����أ�-1
******************************************************************************/
sint32 serial_read_n(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout);

/******************************************************************************
Function	:	serial_write_n
Description	:	д����
Input		:	serial_fd		�����ļ�������
				buf				���ݻ���
				len				д�볤��
				timeout			д��ʱ
Return		:	�ɹ�������д�����ݳ��� ʧ�ܣ����أ�-1
******************************************************************************/
sint32 serial_write_n(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout);

/******************************************************************************
Function	:	serial_wr
Description	:	��д����
Input		:	serial_fd		�����ļ�������
				buf				���ݻ���
				len				д�볤��
				timeout			д��ʱ
Return		:	�ɹ�������д�����ݳ��� ʧ�ܣ����أ�-1
******************************************************************************/
sint32 serial_wr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout);
sint32 serial_wr_atr(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout);
sint32 serial_read_n_atr(uint32 serial_fd, uint8 *buf, uint32 len, uint32 timeout);

int read_from_mini51(uint32 serial_fd, uint8 *buf_r, uint32 len_r, uint32 timeout);
int write_to_mini51(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint32 timeout);
int serial_wr_mini51(uint32 serial_fd, uint8 *buf_w, uint32 len_w, uint8 *buf_r, uint32 len_r, uint32 timeout);





#endif
