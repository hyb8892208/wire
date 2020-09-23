#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

#include "platform_def.h"

typedef struct  
{
  int RestoreEanble;
  int BackupEanble;
  int EraseEanble;
  target_current_state  TargetState;//升级过程中模块的状态
  target_platform       TargetPlatform;//模块的平台
  char                  ImagePath[MAX_PATH];  //image存放路径
  int                   ComPortNumber;
}dload_cfg_type;

/*****
弹出累消息
msgtype: MFC中弹出对话框的类别澹(1,只弹出一个对话框提示，2, 需要选择是否继续)
msg1: 对话框内容
msg2:对话框标题，警告或者错误
*******/
typedef void (*qdl_msg_cb)(int msgtype,char *msg1,char * msg2); 
/*****
显示进度
writesize: 已经写的大小
size:总大小
clear:MFC进度条清空
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
