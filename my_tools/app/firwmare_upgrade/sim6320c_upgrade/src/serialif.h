
#ifndef __SERIALIF_H__
#define __SERIALIF_H__

#include "platform_def.h"

extern byte  dloadbuf[]; //临时缓存的一个buffer
extern byte g_Receive_Buffer[];     //收到的buf
extern int g_Receive_Bytes;							//收到buffer的大小

extern byte g_Transmit_Buffer[];	//发送的代码
extern int g_Transmit_Length;							//发送的buffer 大小

target_current_state send_sync(void);
int read_buildId(void);
int read_version(void);
int read_flash_info(void);
int handle_erase_efs(void);
int normal_reset(void);
int handle_switch_target_offline(void);
int handle_send_sp_code(void);
int switch_to_dload(void);
/******nop命令*******/
int send_nop(void);
/******preq命令*******/
int preq_cmd(void);
/******发送hex文件*******/
int write_32bit_cmd(void);
/******dload模式切换到hex引导模式*******/
int go_cmd(void);

/******hello握手*******/
int handle_hello(void);
/******hex 引导go 命令*******/
int go_cmd_6085(void);

/******6085的hello要发2次*******/
int handle_hello_6085(void);
/******安全模式*******/
int handle_security_mode(void);
/******发送分区表，erase=0 :不擦，erase=1:擦除*******/
int handle_parti_tbl(int erase);
/******升级完成reset*******/
int handle_reset(void);
/******打开分区*******/
int handle_openmulti(int mode);
/******写分区*******/
int handle_write(int mode);
/******关闭PARTITION*******/
int handle_close(void);

int qdl_atoi(const byte *num_str, int len);
#endif /*__SERIALIF_H__*/

