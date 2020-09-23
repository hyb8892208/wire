#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

#include "platform_def.h"

typedef struct  
{
  int RestoreEanble;
  int BackupEanble;
  int EraseEanble;
  target_current_state  TargetState;//����������ģ���״̬
  target_platform       TargetPlatform;//ģ���ƽ̨
  char                  ImagePath[MAX_PATH];  //image���·��
  int                   ComPortNumber;
}dload_cfg_type;

/*****
��������Ϣ
msgtype: MFC�е����Ի��������(1,ֻ����һ���Ի�����ʾ��2, ��Ҫѡ���Ƿ����)
msg1: �Ի�������
msg2:�Ի�����⣬������ߴ���
*******/
typedef void (*qdl_msg_cb)(int msgtype,char *msg1,char * msg2); 
/*****
��ʾ����
writesize: �Ѿ�д�Ĵ�С
size:�ܴ�С
clear:MFC���������
*******/
typedef void (*qdl_prog_cb)(uint32 writesize,uint32 size,int clear);  
typedef void (*qdl_log_cb)(char *msg);
typedef struct
{
  dload_cfg_type pDloadCfg;	// download config
  qdl_text_cb text_cb;	
  qdl_msg_cb msg_cb;	
  qdl_prog_cb prog_cb;	
  qdl_log_cb logfile_cb;
}qdl_context, *p_qdl_context;

extern qdl_context *QdlContext;
#define QDL_LOGFILE_NAME "qdl.txt"

int ProcessInit(dload_cfg_type *pDload,qdl_text_cb trace_cb,qdl_msg_cb msg_cb,qdl_log_cb log_cb, qdl_prog_cb prog_cb);
int save_log(char *fmt,...);
int ProcessUninit(void);
void Processing(char *filename, qdl_context *pQdlContext);
#endif /*__DOWNLOAD_H__*/
