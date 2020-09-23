/*****************************************

完成升级过程，对各个平台兼容支持

*****************************************/

#include "download.h"
#include "file.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined(TARGET_OS_LINUX ) ||defined(TARGET_OS_ANDROID)
#include <stdlib.h>
#include "os_linux.h"
#endif 

#include "serialif.h"
#include "qcn.h"
#include "openvox_version_record.h"
#include "openvox_process_bar.h"
qdl_context *QdlContext = NULL;
FILE *log_file;  //保存log文件
int create_log(void)
{
	log_file = fopen(QDL_LOGFILE_NAME,"wt+");
	if(log_file == NULL){
		return (int) NULL;
	}
	//fclose(log_file);
	return TRUE;
}

int close_log(void)
{
	fclose(log_file);
	log_file = NULL;
	return TRUE;
}
int save_log(char *fmt,...)
{
	//log_file = fopen("QDL.log","at+");
	va_list args;
	char *buffer=new char[200];
	int result=false;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);
	
	if(buffer == NULL)
		return  result;
	if(log_file != NULL){
		int result =fwrite((void *)buffer,sizeof(char),strlen((const char *)buffer)-1,log_file);
		 result = true;		
	}
	delete[] buffer;
	return result;
}

int set_hw_path_manually(void)
{
	char hw_path[50] = {0};
	
	QdlContext->text_cb("please enter the directory where firmware resides in:");

	scanf("%s", hw_path);

	strcat(QdlContext->pDloadCfg.ImagePath, hw_path);

	return true;
}

int ProcessInit(dload_cfg_type *pDload, qdl_text_cb trace_cb, qdl_msg_cb msg_cb,qdl_log_cb log_cb, qdl_prog_cb prog_cb)
{
	if(!pDload)
		return NULL;
	QdlContext = (p_qdl_context)malloc(sizeof(qdl_context));
	if(!QdlContext)
	{
		return NULL;
	}
	memset(QdlContext, 0, sizeof(qdl_context));
	memcpy((void *)(&QdlContext->pDloadCfg),  pDload, sizeof(dload_cfg_type));

	QdlContext->msg_cb= msg_cb;

	QdlContext->logfile_cb = log_cb;
	QdlContext->text_cb = trace_cb;
	QdlContext->prog_cb = prog_cb;
	QdlContext->msg_cb = msg_cb;
	QdlContext->logfile_cb = log_cb;

	QdlContext->pDloadCfg.BackupEanble = pDload->BackupEanble;
	QdlContext->pDloadCfg.RestoreEanble = pDload->RestoreEanble;
	QdlContext->pDloadCfg.EraseEanble = pDload->EraseEanble;

	QdlContext->text_cb("config data, QCN: backup[%d], restore[%d], FILE SYSTEM: erase efs[%d]", 
			QdlContext->pDloadCfg.BackupEanble, QdlContext->pDloadCfg.RestoreEanble, QdlContext->pDloadCfg.EraseEanble);
	
	create_log();
  
	return 1;
}

int ProcessUninit(void)
{
	free(QdlContext);
	QdlContext = NULL;
	
	close_log();

	image_close();
	
	return 1;
}

extern int downloading(void);

void Processing(char *filename, qdl_context *pQdlContext)
{	
	int result;
	int msgresult;
	int timeout=60;
	int state;
	char StrBuff[50];
	long int time,time1;
	
	qdl_pre_download(filename);
	
	module_info::getInstance()->set_start_time(NULL);
	module_info::getInstance()->set_old_version(NULL);
	module_info::getInstance()->set_mod_version(NULL);
	
	downloading();
	
	qdl_post_download();
	
	module_info::getInstance()->set_new_version(NULL);
	module_info::getInstance()->set_end_time(NULL);
	module_info::getInstance()->record_info_to_file();
	
}

int downloading(void)
{
	int result;
	int msgresult;
	int timeout=60;
	char StrBuff[50];
	long int time,time1;
	
	/*open diag com port*/
	timeout=5;
	while(timeout--)
	{
		if(openport(&QdlContext->pDloadCfg.ComPortNumber))
		{	
			break;
		}	

		if(timeout > 0)
		{	
			sprintf(StrBuff,"open com error, retry again [%d]", timeout);
			QdlContext->text_cb(StrBuff);
			qdl_sleep(1000);
		}		
		else 
		{
			UpgradeCount::getInstance()->process_bar("OpenCom err");
			module_info::getInstance()->set_upgrade_state(COM_ERR);
			QdlContext->text_cb( "open comport fail"); 
			return FALSE;
		}		
	}

	/*get module status*/
	timeout=2;
	while(timeout--)
	{
		//QdlContext->text_cb( "start send sync");
		QdlContext->pDloadCfg.TargetState=send_sync();
		if(QdlContext->pDloadCfg.TargetState == STATE_UNSPECIFIED)	
		{
			if(timeout == 0)
			{
				QdlContext->text_cb( "module state is unspecified, download failed!"); 
				UpgradeCount::getInstance()->process_bar("module state err");
				module_info::getInstance()->set_upgrade_state(MODULE_ERR);
				return FALSE;
			}
			QdlContext->text_cb( "module state is unspecified, try again"); 
			qdl_sleep(1000);
		}
		else 
		{
			break;  
		}
	}
	
	/*
	   * if module is in normal mode, then we should 
	   * switch it to download mode.
	   */
	if(QdlContext->pDloadCfg.TargetState == STATE_NORMAL_MODE)
	{	
		QdlContext->text_cb( "read version:");
		result=read_version();
		if(result==false) 
		{
			QdlContext->text_cb( "read fail");
		}
		else
		{
			QdlContext->text_cb("Version:%s\r\n",&g_Receive_Buffer[1]);
		}
		qdl_sleep(50);	

		/*read flash informaiton for compatible*/
		if(QdlContext->pDloadCfg.TargetPlatform == TARGET_PLATFORM_6290 ||
			QdlContext->pDloadCfg.TargetPlatform == TARGET_PLATFORM_6270_SIM5215) 
		{
			result = read_flash_info();

			if(result == false)
			{
				QdlContext->text_cb( "can't detect flash type");
				module_info::getInstance()->set_upgrade_state(FLASH_ERR); 
				UpgradeCount::getInstance()->process_bar("flash type err");
				if(!set_hw_path_manually())
					return FALSE;
			}
			else
			{
				/*get maker id and device id for the flash*/
				int maker_id = 0;
				int device_id = 0;

				maker_id = qdl_atoi(&g_Receive_Buffer[24], 4);
				device_id = qdl_atoi(&g_Receive_Buffer[28], 4);	

				QdlContext->text_cb( "flash maker_id :%x, device_id :%x", maker_id, device_id);
				
				if(maker_id == 0xAD && device_id == 0x76)   /*old memory*/
					strcat(QdlContext->pDloadCfg.ImagePath, "H1/");
				else if(maker_id == 0xAD && device_id == 0xA1)   /*new memory*/
					strcat(QdlContext->pDloadCfg.ImagePath, "H2/");
				else if(maker_id == 0xAD && device_id == 0x36)   /*old memory*/
					strcat(QdlContext->pDloadCfg.ImagePath, "H1/");
				else if(maker_id == 0xAD && device_id == 0xB1)   /*new memory*/
					strcat(QdlContext->pDloadCfg.ImagePath, "H2/");
				else
				{
					QdlContext->text_cb( "can't detect flash type");
					
					if(!set_hw_path_manually())
						return FALSE;
				}
			}
		}
		
		/*backup QCN*/
		if(QdlContext->pDloadCfg.BackupEanble)
		{
			QdlContext->text_cb( "backup QCN...");
			SaveQdnThreadFunc(QdlContext);
		}
		
		QdlContext->text_cb( "switch to download mode");
		result = switch_to_dload();
		if(result==false)
		{
			
			module_info::getInstance()->set_upgrade_state(BEGIN_ERR); 
			UpgradeCount::getInstance()->process_bar("begin download err");
			QdlContext->text_cb( "switch to download mode failed");
			return FALSE;
		}

		/*wait module to reboot to download mode*/
		qdl_sleep(5000);     

          	/*
          	  * maybe diag port is switch to another port, so we 
          	  * need to reopen the port.
          	  */		
          	timeout=10;
          	while(timeout--)
          	{         		
          		closeport(hCom);
          		
          		qdl_sleep(1000);
          		
          		if(!openport(&QdlContext->pDloadCfg.ComPortNumber))
          		{
          			qdl_sleep(2000);
          			continue;
          		}
          
          		QdlContext->pDloadCfg.TargetState=send_sync();
          
          		if (QdlContext->pDloadCfg.TargetState==STATE_DLOAD_MODE)
          		{	
          			break;
          		}	      
          		else if(timeout > 0)
          		{	
          			sprintf(StrBuff,"switch to download mode failed try again[%d]", timeout);
          			QdlContext->text_cb( StrBuff);
          		}	
          		else if(timeout==0) 
          		{
				module_info::getInstance()->set_upgrade_state(BEGIN_ERR); 
				UpgradeCount::getInstance()->process_bar("begin download err");
          			QdlContext->text_cb( "switch to download mode failed, download error!" ); 
          			return FALSE;
          		}				
          	}
	}
	else
	{
		/*
		  * in download mode, we can't get flash type this stage,
		  * so we need to set firmware path manually!
		  */
		if(QdlContext->pDloadCfg.TargetPlatform == TARGET_PLATFORM_6290 ||
			QdlContext->pDloadCfg.TargetPlatform == TARGET_PLATFORM_6270_SIM5215) 
		{
		       if(!set_hw_path_manually())
				return FALSE;
		}
	}

	if(QdlContext->pDloadCfg.TargetPlatform == TARGET_PLATFORM_6270_SIM5320)
		strcat(QdlContext->pDloadCfg.ImagePath, "KPRBL/");


	QdlContext->text_cb( "start reading firmware :%s", QdlContext->pDloadCfg.ImagePath);
	if(!image_read(&QdlContext->pDloadCfg))
	{
		QdlContext->text_cb( "can't get firmware, failed");
		return FALSE;
	}
	UpgradeCount::getInstance()->set_total_size(image_size());
	/*prepare to downloading*/
	if(QdlContext->pDloadCfg.TargetState==STATE_DLOAD_MODE)
	{
		QdlContext->text_cb( "nop");
		result=send_nop();
		if(result==false) 
		{
			module_info::getInstance()->set_upgrade_state(SYNC_ERR); 
			UpgradeCount::getInstance()->process_bar("sync err");
			QdlContext->text_cb( "nop fail"); 
			return FALSE;
		}

		QdlContext->text_cb( "preq");
		if(!preq_cmd())
		{
			module_info::getInstance()->set_upgrade_state(SYNC_ERR); 
			UpgradeCount::getInstance()->process_bar("sync err");
			QdlContext->text_cb( "preq fail!");
			return FALSE;
		}

		qdl_sleep(1000);
		
		QdlContext->text_cb( "hex");
		result=write_32bit_cmd();
		if(result==false) 
		{
			module_info::getInstance()->set_upgrade_state(SYNC_ERR); 
			UpgradeCount::getInstance()->process_bar("sync err");
			QdlContext->text_cb( "hex fail"); 
			return FALSE;
		}

		QdlContext->text_cb( "go");
		result=go_cmd();
		if(result==false)
		{
			module_info::getInstance()->set_upgrade_state(SYNC_ERR); 
			UpgradeCount::getInstance()->process_bar("sync err");
			QdlContext->text_cb( "go fail"); 
			return FALSE;
		}

		/*module is rebooted*/
		qdl_sleep(3000);
		
		/*reopen the diag port*/
		timeout = 10;
		while(timeout--)
		{
			closeport(hCom);
				
			qdl_sleep(1000);
				
			if(!openport(&QdlContext->pDloadCfg.ComPortNumber))
			{	
				continue;
			}	
			
			QdlContext->pDloadCfg.TargetState=send_sync();
     			 QdlContext->pDloadCfg.TargetState=STATE_GOING_MODE; //who change?
			if (QdlContext->pDloadCfg.TargetState==STATE_GOING_MODE)
			{	
				break;
			}	
			else if(timeout > 0)
			{	
				sprintf(StrBuff,"go failed, try again[%d]", timeout);
				QdlContext->text_cb( StrBuff);
			}		
			else if(timeout==0) 
			{
				module_info::getInstance()->set_upgrade_state(SYNC_ERR); 
				UpgradeCount::getInstance()->process_bar("sync err");
				QdlContext->text_cb( "timeout, go failed"); 
				return FALSE;
			}				
		}
	}

	/*start downing*/
	if(QdlContext->pDloadCfg.TargetState==STATE_GOING_MODE)
	{	
		QdlContext->text_cb( "hello");

		result=handle_hello();
		if(result==false) 
		{
			QdlContext->text_cb( "hello fail");
			module_info::getInstance()->set_upgrade_state(BEGIN_ERR); 
			UpgradeCount::getInstance()->process_bar("hello err");
			return FALSE;	
		}
		else
		{
			char string1[64];
			int size;
			memcpy(string1,&g_Receive_Buffer[1],32);
			string1[32] = 0;
			size = (g_Receive_Buffer[36]<<8) | g_Receive_Buffer[35];
			size = g_Receive_Buffer[43];
			memcpy(string1,&g_Receive_Buffer[44],size);
			string1[size] = 0;
			QdlContext->text_cb("Flash_type:%s, len :%d, size :%d",string1, g_Receive_Bytes, size);
		}

		result=handle_security_mode();
		if(result==false)
		{
			QdlContext->text_cb( "trust fail"); 
			module_info::getInstance()->set_upgrade_state(BEGIN_ERR); 
			UpgradeCount::getInstance()->process_bar("trust err");
			return FALSE;
		} 

		if(handle_parti_tbl(0)==false) 
		{
			/*
			  * WARNING!!! It's very important to backup the QDN
			  * before the new firmware download, if the partition
			  * table is different between the new and old firmware.
			  */
			QdlContext->text_cb( "partitbl mismatched");
			if(!QdlContext->pDloadCfg.BackupEanble)
			{
				QdlContext->text_cb( "please backup QCN first!");
				module_info::getInstance()->set_upgrade_state(BEGIN_ERR); 
				UpgradeCount::getInstance()->process_bar("sync err");
				return FALSE;
			}
			QdlContext->text_cb( "QDL will erase QCN");
		}
		UpgradeCount::getInstance()->set_write_size(partition_length);
		UpgradeCount::getInstance()->process_bar(NULL);
		/*erase EFS*/
		if(QdlContext->pDloadCfg.EraseEanble)
		{
			QdlContext->text_cb( "Erase EFS...");

			/*open the efs partition and erased it*/
			result = handle_erase_efs();
			if(result == false) 
			{
				QdlContext->text_cb( "Erase EFS fail"); 
				//return FALSE;
			}
			else
				QdlContext->text_cb( "Erase EFS finished!");
			
			/*close efs partition*/
			handle_close();
		}

		if((QdlContext->pDloadCfg.TargetPlatform==TARGET_PLATFORM_6290) 
			||(QdlContext->pDloadCfg.TargetPlatform==TARGET_PLATFORM_6085))//5218 or 6216
		{     
		
			QdlContext->text_cb( "qcsblhdcfg downloading...");
			
			result=handle_openmulti(OPEN_MULTI_MODE_QCSBLHDCFG);
			
			if(result == false) 
			{
				
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("open qcsblhdcfg err");
				QdlContext->text_cb( "qcsblhdcfg open fail"); 
				return FALSE;
			}
			
			result=handle_write(OPEN_MULTI_MODE_QCSBLHDCFG);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("write qcsblhdcfg err");
				QdlContext->text_cb( "write fail"); 
				return FALSE;
			}
			
			result=handle_close();	
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("close qcsblhdcfg err");
				QdlContext->text_cb( "close fail"); 
				return FALSE;
			}
			
			QdlContext->text_cb( "");

			QdlContext->text_cb( "qcsbl downloading...");
			
			result  =handle_openmulti(OPEN_MULTI_MODE_QCSBL);
			
			if(result == false)
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("oepn qcsbl err");
				QdlContext->text_cb( "qcsbl open fail"); 
				return FALSE; 
			}
			
			result = handle_write(OPEN_MULTI_MODE_QCSBL);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("write qcsbl err");
				QdlContext->text_cb( "write fail"); 
				return FALSE;
			}
			
			result = handle_close();	
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("close qcsbl err");
				QdlContext->text_cb( "close fail"); 
				return FALSE;
			} 
			
			QdlContext->text_cb( "");

			QdlContext->text_cb( "oemsbl downloading...");
			
			result = handle_openmulti(OPEN_MULTI_MODE_OEMSBL);
			
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("open oemsbl err");
				QdlContext->text_cb( "oemsbl open fail"); 
				return FALSE;
			}
			
			result = handle_write(OPEN_MULTI_MODE_OEMSBL);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("write oemsbl err");
				QdlContext->text_cb( "write fail"); 
				return FALSE;
			}
			
			result = handle_close();	
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("close oemsbl err");
				QdlContext->text_cb( "close fail"); 
				return FALSE;
			}
			
			QdlContext->text_cb( "");

			QdlContext->text_cb( "amss downloading...");
			
			result = handle_openmulti(OPEN_MULTI_MODE_AMSS);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("open amss err");
				QdlContext->text_cb( "amss open fail"); 
				return FALSE;
			}
			
			result = handle_write(OPEN_MULTI_MODE_AMSS);
			if(result == false) 
			{ 
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("write amss err");
				QdlContext->text_cb( "write fail"); 
				return FALSE;
			}
			
			qdl_sleep(2000);

			result = handle_close();	
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("close amss err");
				QdlContext->text_cb( "close fail"); 
				return FALSE;
			} 
			QdlContext->text_cb( "");
		}

		if(QdlContext->pDloadCfg.TargetPlatform == TARGET_PLATFORM_6270_SIM5215 ||
			QdlContext->pDloadCfg.TargetPlatform == TARGET_PLATFORM_6270_SIM5320) 
		{    
			QdlContext->text_cb( "dbl downloading...");
			
			result = handle_openmulti(OPEN_MULTI_MODE_DBL);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("open dbl err");
				QdlContext->text_cb( "dbl open fail"); 
				return FALSE;
			}
			
			result=handle_write(OPEN_MULTI_MODE_DBL);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("write dbl err");
				QdlContext->text_cb( "write fail"); 
				return FALSE;
			}
			
			result = handle_close();	
			if(result == false)
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("close dbl err");
				QdlContext->text_cb( "close fail"); 
				return FALSE;
			}
			
			QdlContext->text_cb( "");

			QdlContext->text_cb( "fsbl downloading...");
			
			result = handle_openmulti(OPEN_MULTI_MODE_FSBL);
			if(result == false)
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("open fsbl err");
				QdlContext->text_cb( "fsbl open fail"); 
				return FALSE;
			}
			
			result = handle_write(OPEN_MULTI_MODE_FSBL);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("write fsbl err");
				QdlContext->text_cb( "write fail"); 
				return FALSE;
			}
			
			result = handle_close();	
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("closs fsbl err");
				QdlContext->text_cb( "close fail"); 
				return FALSE;
			}
			
			QdlContext->text_cb( "");

			QdlContext->text_cb( "osbl downloading...");
			result = handle_openmulti(OPEN_MULTI_MODE_OSBL);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("open osbl err");
				QdlContext->text_cb( "osbl open fail"); 
				return FALSE;
			}
			
			result = handle_write(OPEN_MULTI_MODE_OSBL);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("white osbl err");
				QdlContext->text_cb( "write fail"); 
				return FALSE;
			}
			
			result = handle_close();	
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("close osbl err");
				QdlContext->text_cb( "close fail"); 
				return FALSE;
			}
			
			QdlContext->text_cb( "");
			QdlContext->text_cb( "amss downloading...");
			
			result = handle_openmulti(OPEN_MULTI_MODE_AMSS);
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("open amss err");
				QdlContext->text_cb( "amss open fail"); 
				return FALSE;
			}
			
			result = handle_write(OPEN_MULTI_MODE_AMSS);
			if(result = false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("download amss err");
				QdlContext->text_cb( "write fail"); 
				return FALSE;
			}

#ifdef FEATURE_FAST_DOWNLOAD
			qdl_sleep(2000);
#endif
			result = handle_close();	
			if(result == false) 
			{
				module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR); 
				UpgradeCount::getInstance()->process_bar("close amss err");
				QdlContext->text_cb( "close fail"); 
				return FALSE;
			} 
			QdlContext->text_cb( "");
		}

	}
	
	handle_reset();    /*reset the module*/

	if(QdlContext->pDloadCfg.RestoreEanble)
	{
		/*restore QCN*/
		
		/*wait module to reboot*/
		qdl_sleep(8000);     

          	/*
          	  * maybe diag port is switch to another port, so we 
          	  * need to reopen the port.
          	  */		
          	timeout=10;
          	while(timeout--)
          	{         		
          		closeport(hCom);
          		
          		qdl_sleep(1000);
          		
          		if(!openport(&QdlContext->pDloadCfg.ComPortNumber))
          		{
          			qdl_sleep(2000);
          			continue;
          		}
          
          		QdlContext->pDloadCfg.TargetState=send_sync();
          
          		if (QdlContext->pDloadCfg.TargetState==STATE_NORMAL_MODE)
          		{	
          			QdlContext->text_cb( "restore QCN...");
				RestoreQdnThreadFunc(QdlContext);
          			break;
          		}	
			else
			{
          			QdlContext->text_cb( "can't restore QCN, module is not in normal mode");
				break;
			}
          	}
	}
	module_info::getInstance()->set_upgrade_state(SUCCESS); 
	UpgradeCount::getInstance()->process_bar("upgrade success");
	return TRUE;
}


