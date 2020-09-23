
#ifndef __SERIALIF_H__
#define __SERIALIF_H__

#include "platform_def.h"

extern byte  dloadbuf[]; //��ʱ�����һ��buffer
extern byte g_Receive_Buffer[];     //�յ���buf
extern int g_Receive_Bytes;							//�յ�buffer�Ĵ�С

extern byte g_Transmit_Buffer[];	//���͵Ĵ���
extern int g_Transmit_Length;							//���͵�buffer ��С

target_current_state send_sync(void);
int read_buildId(void);
int read_version(void);
int read_flash_info(void);
int handle_erase_efs(void);
int normal_reset(void);
int handle_switch_target_offline(void);
int handle_send_sp_code(void);
int switch_to_dload(void);
/******nop����*******/
int send_nop(void);
/******preq����*******/
int preq_cmd(void);
/******����hex�ļ�*******/
int write_32bit_cmd(void);
/******dloadģʽ�л���hex����ģʽ*******/
int go_cmd(void);

/******hello����*******/
int handle_hello(void);
/******hex ����go ����*******/
int go_cmd_6085(void);

/******6085��helloҪ��2��*******/
int handle_hello_6085(void);
/******��ȫģʽ*******/
int handle_security_mode(void);
/******���ͷ�����erase=0 :������erase=1:����*******/
int handle_parti_tbl(int erase);
/******�������reset*******/
int handle_reset(void);
/******�򿪷���*******/
int handle_openmulti(int mode);
/******д����*******/
int handle_write(int mode);
/******�ر�PARTITION*******/
int handle_close(void);

int qdl_atoi(const byte *num_str, int len);
#endif /*__SERIALIF_H__*/

