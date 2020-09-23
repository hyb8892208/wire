#include "platform_def.h"
#include "download.h"
#include "quectel_log.h"
#include "file.h"
#include "serialif.h"
#include "os_linux.h"
#include "quectel_common.h"
#include "atchannel.h"
#include "ril-daemon.cpp"
#include "openvox_version_record.h"

extern int dump;
extern download_context *QdlContext;
extern int g_hCom; 
unsigned char boot_tmp_crc_table[1024*4]={0};
unsigned char *boot_tmp=boot_tmp_crc_table;

int boot_tmp_datasize = sizeof(boot_tmp_crc_table);


static const char* not_support_at_fastboot = 
"AT+qfastboot execute failed.\n"
"May be current firmware not support this method.\n"
"Please try QFlash -f firmware_path -m [0,1]\n";

int do_streaming_download(download_context *pQdlContext);
int do_fastboot_download_direct(download_context *pQdlContext);
int do_fastboot_download(download_context *pQdlContext);



int ProcessInit(download_context *pQdlContext) {
    if (!image_read(pQdlContext)) {
        QFLASH_LOGD("Parse file error\n");
        return 0;
    }
    return 1;
}

int ProcessUninit(download_context *pQdlContext) {
    image_close(pQdlContext);
    return 1;
}

int module_state(download_context *pQdlContext)
{
    printf("Module status detect\n");
    int timeout = 10;
    while (timeout--) {
        pQdlContext->TargetState = send_sync();
        if (pQdlContext->TargetState == STATE_UNSPECIFIED) {
            if (timeout == 0) {
                printf("Module status is unspecified, download failed!\n");
                return false;
            }
            sleep(2);
        } else {
            break;
        }
    }
    return true;
}

static int ql_pclose(FILE *iop)
{
	(void)fclose(iop);
	return 0;
}

static int do_flash_mbn(const char *partion, const char *filepath) {
    char *program = (char *) malloc(MAX_PATH + MAX_PATH + 32);
    int result;
    unsigned char *filebuf;
    uint32 filesize;
    FILE * fp = NULL;
    
    if (!program) {
        QFLASH_LOGD("fail to malloc memory for %s %s\n", partion, filepath);
        return 0;
    }

    sprintf(program, "flash %s %s", partion, filepath);
    QFLASH_LOGD("%s\n", program);
    if(strstr(filepath, "invalid-boot") != NULL)
	{
		filebuf = boot_tmp;
		filesize = boot_tmp_datasize;
	}
    else
    {
	    if (!partion || !filepath || !filepath[0] || access(filepath, R_OK)) 
	    {
	        free(program);
	        return 0;
	    }

#if 0
	    filebuf = open_file(filepath, &filesize);
	    if (filebuf == NULL) {
	        free(program);
	        return false;
	    }
#else
	    filebuf = (unsigned char *)malloc(4*1024);
	    if (filebuf == NULL) {
	        free(program);
	        return false;
	    }

	    fp = fopen(filepath, "r");
	    if (fp == NULL) {
	        QFLASH_LOGD("%s(%s) failed to fopen errno: %d (%s)\n", __func__, filepath, errno, strerror(errno));
	        return 0;
	    }
	    
	    fseek(fp, 0, SEEK_END);
	    filesize = ftell(fp);
	    fseek(fp, 0, SEEK_SET);
#endif
    }

    strcpy(program, partion);
    result = handle_openmulti(strlen(partion) + 1, (unsigned char *)program);
    if (result == false) {
        QFLASH_LOGD("%s open failed\n", partion);
    	fclose(fp); free(filebuf); filebuf = NULL;
        goto __fail;
    }

    sprintf(program, "sending '%s' (%dKB)", partion, (int)(filesize/1024));
    QFLASH_LOGD("%s\n", program);

    result = handle_write(fp, filebuf, filesize);
    if(fp != NULL){
        fclose(fp); free(filebuf); filebuf = NULL;
	}
    if (result == false) {
        printf("%s download failed\n", partion);
	char err_buf[256] = {0};
	sprintf(err_buf, "download %s err: ret = false", partion);
	transfer_statistics::getInstance()->process_bar(err_buf);
        goto __fail;
    }

    result = handle_close();
    if (result == false) {
        printf("%s close failed", partion);
        goto __fail;
    }

    QFLASH_LOGD("OKAY\n");

    free(program);
    if(fp != NULL){
		free_file(filebuf, filesize);
	}
    return true;

__fail:
    free(program);
    if(fp!=NULL){
		free_file(filebuf, filesize);
	}
    return false;
}

static int do_fastboot(const char *cmd, const char *partion, const char *filepath) {
	char *program = (char *) malloc(MAX_PATH + MAX_PATH + 32);
	char *line = (char *) malloc(MAX_PATH);
	char *self_path = (char *) malloc(MAX_PATH);
	FILE * fpin;
	
	int self_count = 0;
	int recv_okay = 0;
	int recv_9607 = 0;
	
#define FREE_SOURCE  do{if(program) free(program); if(line) free(line);if(self_path) free(self_path);}while(0);

	if (!program || !line || !self_path) {
		QFLASH_LOGD("fail to malloc memory for %s %s %s\n", cmd, partion, filepath);
		FREE_SOURCE
		return 0;
	}

	self_count = readlink("/proc/self/exe", self_path, MAX_PATH - 1);
	if (self_count > 0) {
		self_path[self_count] = 0;
	} else {
		QFLASH_LOGD("fail to readlink /proc/self/exe for %s %s %s\n", cmd, partion, filepath);
		FREE_SOURCE
		return 0;
	}
	
	if (!strcmp(cmd, "flash")) {
		if (!partion || !partion[0] || !filepath || !filepath[0] || access(filepath, R_OK)) {
			FREE_SOURCE
			return 0;
		}
		sprintf(program, "%s fastboot %s %s \"%s\"", self_path, cmd, partion, filepath);
	    //sprintf(program, "%s fastboot %s %s %s 1>2 2>./rfastboot", self_path, cmd, partion, filepath);
		
	} else {
		sprintf(program, "%s fastboot %s", self_path, cmd);
		//sprintf(program, "%s fastboot %s 1>./rfastboot", self_path, cmd);
	}

	QFLASH_LOGD("%s\n", program);
	strcat(program, " 2>&1");
	fpin = popen(program, "r");


	if (!fpin) {
		QFLASH_LOGD("popen failed\n");
		QFLASH_LOGD("popen strerror: %s\n", strerror(errno));
		FREE_SOURCE
		return 0;
	}

	while (fgets(line, MAX_PATH - 1, fpin) != NULL) {
		QFLASH_LOGD("%s", line);
		if (strstr(line, "OKAY")) {
			recv_okay++;
		} else if (strstr(line, "fastboot")) {
			recv_9607++;
		}
	}
	
	ql_pclose(fpin);
	FREE_SOURCE
	if (!strcmp(cmd, "flash"))
	{	
		return (recv_okay == 2);
	}
	else if (!strcmp(cmd, "devices"))
	{
		return (recv_9607 == 1);
	}
   	else if (!strcmp(cmd, "continue"))
	{
		return (recv_okay == 1);
	}
   	else
	{
		return (recv_okay > 0);
	}
	
	return 0;
}

int BFastbootModel() {
	return do_fastboot("devices", NULL, NULL);
}



int downloadfastboot(download_context *pQdlContext) {
	int ret = 0;
	for (std::vector<Ufile>::iterator iter = pQdlContext->ufile_list.begin();iter!=pQdlContext->ufile_list.end();iter++) 
	{
		if(strcmp("0:MIBIB",((Ufile)*iter).name)!=0)
		{			
			strToLower((*iter).partition_name);
			ret = do_fastboot("flash", (*iter).partition_name, ((Ufile)*iter).img_name);
			if(1 != ret)
			{										
				QFLASH_LOGD("fastboot flash error!, upgrade process interrupt.  exit!\n");
				transfer_statistics::getInstance()->process_bar((char *)"fastboot err:ret=-1\n");
				return 1;				
			}
			transfer_statistics::getInstance()->set_write_bytes(get_file_size(((Ufile)*iter).img_name));
			transfer_statistics::getInstance()->process_bar();
		}
			
	}
	do_fastboot("reboot", NULL, NULL);
	return 0;
}

static void ignore_sahara_stage_files(download_context *pQdlContext)
{
	for (std::vector<Ufile>::iterator iter = pQdlContext->ufile_list.begin();
		iter != pQdlContext->ufile_list.end();/*iter++*/) 
	{
		if(strcmp("0:MIBIB",((Ufile)*iter).name)!=0)
		{

			if(strstr(((Ufile)*iter).name,"0:aboot") || strstr(((Ufile)*iter).name,"0:SBL")   ||strstr(((Ufile)*iter).name,"0:RPM")  || strstr(((Ufile)*iter).name,"0:TZ") )
			{
				iter = pQdlContext->ufile_list.erase(iter);
			}
			else
			{
				iter++;
			}
		}
		else
		{
			iter++;
		}
	}       
}
/*
1. wait port disconnect
2. wait port connect
3. open
*/
static int close_and_reopen(int ioflush)
{
	closeport();
    if(wait_diag_port_disconnect(DETECT_DEV_TIMEOUT) == 0)
    {
    	printf("Diagnose port disconnect\n");
    }
    else
    {
    	printf("Warning: Diagnose port may be exist always.\n");
    }
    if(detect_diag_port_timeout(DETECT_DEV_TIMEOUT) == 0)
    {
    	sleep(1);
	    if(open_port_once(ioflush) != 0)
	    {
	        printf("Start to open port, Failed!\n");
	        return false;
	    }
	    return 0;
    }else
    {
    	printf("Can't find diagnose port. upgrade interrupt.\n");
    	return -1;
    }
    return -2;
}
/*
*/
static int close_and_reopen_without_wait(int ioflush)
{
/*
some other 9x07 platform, host send done packet to module, the module have not shutdown usb port,
the port (ttyUSB0) will not disconnect, so wait is wasted time. simple sleep 5 seconds
*/
#if 0
	closeport();
	if(wait_diag_port_disconnect(DETECT_DEV_TIMEOUT) == 0)
	{
		printf("Diagnose port disconnect\n");
	}
	else
	{
		printf("Warning: Diagnose port may be exist always.\n");
	}
#endif
	closeport();
	sleep(5);
	if(detect_diag_port_timeout(DETECT_DEV_TIMEOUT) == 0)
	{
		sleep(1);
		if(open_port_once(ioflush) != 0)
		{
			printf("Start to open port, Failed!\n");
			return false;
		}
		return 0;
	}else
	{
		printf("Can't find diagnose port. upgrade interrupt.\n");
		return -1;
	}
	return -2;
}
int process_at_fastboot_upgrade(download_context* ctx_ptr)
{
	int modem_fd;
	ATResponse *p_response = NULL;
	int err;
	char dev_path[MAX_PATH];


	if(detect_modem_port(&ctx_ptr->modem_port) == 0)
	{		
		printf("Auto detect Quectel modem port = %s\n", ctx_ptr->modem_port);
		sleep(1);
	}else
	{
		printf("Auto detect Quectel modem port failed.\n");
		return false;
	}
	sprintf(dev_path, "/dev/%s", ctx_ptr->modem_port);
	modem_fd = serial_open(dev_path);
	if (modem_fd < 0)
	{
		printf("Fail to open %s, errrno : %d (%s)\n", dev_path, errno, strerror(errno));
		return false;
	}
	
	at_set_on_reader_closed(onATReaderClosed);
	at_set_on_timeout(onATTimeout, 15000);

	at_send_command("ATE0Q0V1", NULL);
	err = at_send_command_multiline("ATI;+CSUB;+CVERSION", "\0", &p_response);
	if (err < 0 || p_response == NULL || p_response->success == 0) {
		printf("Fail to send cmd  ATI, errrno : %d (%s)\n", errno, strerror(errno));
		return false;
	} 
	if (!err && p_response && p_response->success) {
		ATLine *p_cur = p_response->p_intermediates;
		while (p_cur) {
			p_cur = p_cur->p_next;
		}
	}
	at_response_free(p_response);
	if(AT_ERROR_CHANNEL_CLOSED == at_send_command("AT+qfastboot", NULL))
	{
		close(modem_fd);			
		printf("going to fastboot modle ...");
		sleep(3);
	}else
	{
		printf("%s\n",not_support_at_fastboot);
	}

	if(wait_adb(DETECT_DEV_TIMEOUT) == 0)
	{
		sleep(3);
		if(do_fastboot_download_direct(ctx_ptr) != 0)
		{
			return false;
		}else
		{
			return true;
		}
	}else
	{
		printf("Can't find adb port, upgrade failed.\n");
		return false;
	}
	return false;
}

int vertifyAllnum(char* ch)
{
    int re=1;
    int i;
    for (i=0;i<strlen(ch);i++)
    {
        if(isdigit(*(ch+i))==0)
        {
            return 0;
        }
    }
    return re;
}


void Resolve_port(char *chPort,int* nPort )
{
    *nPort=-1;
    char string[7];
    char chPortNum[10];
    strncpy(string,chPort,(sizeof("ttyUSB")-1));
    string[(sizeof("ttyUSB")-1)]='\0';

    if(strlen(chPort)<sizeof("ttyUSB**"))
    {
        if(strcmp(string,"ttyUSB")==0)
        {
            memset(chPortNum,0,sizeof(chPortNum));
            memcpy(chPortNum,chPort+(sizeof("ttyUSB")-1),(strlen(chPort)-(sizeof("ttyUSB")-1)));
            if(vertifyAllnum(chPortNum)&&*chPortNum!=0)
            {
                *nPort=atoi(chPortNum);
            }
        }
    }
}


int process_streaming_fastboot_upgarde(download_context *ctx_ptr)
{
	int sync_timeout=15;
	int timeout = 10;
	int get_hello_packet = 0;
	int re;
	int direct_fastboot = 0;
	int ret = 0;
	int emergency_mode = 0;
	int emergency_diag_port = 0;
	int try_count = 10;
	
    if (ctx_ptr->update_method == 0 && !detect_adb()) {
		if(!do_fastboot_download_direct(ctx_ptr))
		{
			transfer_statistics::getInstance()->process_bar((char *)"upgrade success!\n");
			return true;
		}else
		{
			return false;
		}		
	}
_detect_diag_port_:
	//auto detect diagnose port
	if(detect_diag_port(&ctx_ptr->diag_port) == 0)
	{		
		Resolve_port(ctx_ptr->diag_port, &g_default_port);
		if(g_default_port == -1)
		{
			QFLASH_LOGD("Auto detect quectel diagnose port failed!");
			module_info::getInstance()->set_upgrade_state(COM_ERR);
			return -1;
		}else
		{
			QFLASH_LOGD("Auto detect quectel diagnose port = %s\n", ctx_ptr->diag_port);
			//success
		}
	}else
	{
		printf("Cannot find Quectel diagnoese and adb port.\n");
		module_info::getInstance()->set_upgrade_state(COM_ERR);
		if(try_count > 0){
			sleep(1);
			try_count--;
			goto _detect_diag_port_;
		}
		return -2;
	}

__normal_download_:	
    //open port without ioflush    
    if(open_port_once(0) != 0)
    {
        printf("Start to open port, Failed!\n");
		module_info::getInstance()->set_upgrade_state(COM_ERR);
        return false;
    }
    if(!is_emergency_diag_port())
    {
    	emergency_diag_port = 1;
    }else
    {
    	emergency_diag_port = 0;
    }
    emergency_diag_port == 1?printf("Use emergency diag port\n"):printf("Use normal diag port\n");
    if(ctx_ptr->ignore_zero_pkt)
    {
    	emergency_diag_port = 1;
    	printf("Ignore USB zero packet.\n");
    }
	printf("Get sahara hello packet!\n");
	if(get_sahara_hello_packet() == 0)
	{
		/*
		Note: some kernel , the kernel will send 5E and other byte to ttyUSB0, module will response with hello packet.
		read it and clear rx buffer.
		*/
		get_hello_packet = 1;
		sleep(3);
		ignore_dirty_data();
		goto sahara_get_hello;
	}else
	{
		printf("Get sahara hello packet failed.\n");
	}

	printf("Detect module status!\n");
 
    if (module_state(ctx_ptr) == 0)
    {
		module_info::getInstance()->set_upgrade_state(BEGIN_ERR);
        return false;
    }

    if (ctx_ptr->TargetState == STATE_NORMAL_MODE) {

		retrieve_soft_revision();    
        printf("Switch to PRG status\n");
        if (switch_to_dload() != 0) {
            printf("Switch to PRG status failed\n");
			module_info::getInstance()->set_upgrade_state(BEGIN_ERR);
            return false;
        }
        
        if( close_and_reopen(0) != 0)
        {
        	return false;
        }
    }
    else if(ctx_ptr->TargetState == STATE_SAHARA_MODE)
    {
        goto sahara_download;
    }
    else if (ctx_ptr->TargetState == STATE_GOING_MODE)
    {
        goto stream_download;
    }
    else
    {
        printf("Get sahara hello packet failed!\n");
		module_info::getInstance()->set_upgrade_state(SYNC_ERR);
        return false;
    }
    
sahara_download:
	printf("Try get sahara hello packet!\n");
	if(get_sahara_hello_packet() == 0)
	{
		printf("Get sahara hello packet successfully!\n");
	}else
	{
		printf("Get sahara hello packet failed!\n");
	}
    //2.send hello response packet

sahara_get_hello:
    
	if(get_hello_packet == 1)
	{
		get_hello_packet = 0;
		printf("Send sahara hello response packet(1)!\n");
		if(SendHelloPacketTest(emergency_diag_port)==false)
		{
			printf("Send sahara hello response packet failed!\n");
			module_info::getInstance()->set_upgrade_state(SYNC_ERR);
			return false;
		}
	}
    else{
		printf("Send sahara hello response packet(2)!\n");
		if(SendHelloPacketTest(emergency_diag_port)==false)
		{
			printf("Send sahara hello response packet failed!\n");
			module_info::getInstance()->set_upgrade_state(SYNC_ERR);
			return false;
		}
	}

    printf("Start Read Data!\n");
	re = GetReadDataPacket(&emergency_mode);
	if(re == 2)
	{
		get_hello_packet = 1;
		goto sahara_get_hello;
	}

    if(re == false)
    {
        return false;
    }
    
    printf("Send sahara do packet!\n");
    if(send_sahara_do_packet() != 0)
    {
        printf("Send Do packet failed!\n");
	module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR);
        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    
    printf("Module Status Detection\n");
    emergency_mode == 1?printf("Upgrade in emergency mode\n"):printf("Upgrade in normal mode\n");

    if( emergency_mode != 1 && close_and_reopen_without_wait(1) != 0)
    {
		module_info::getInstance()->set_upgrade_state(COM_ERR);
    	return false;
    }
    
    if (module_state(ctx_ptr) == 0)
    {
    	module_info::getInstance()->set_upgrade_state(COM_ERR);
        return false;
    }

stream_download:
    if (ctx_ptr->TargetState == STATE_GOING_MODE) {
        printf("Start to download firmware\n");
        if (handle_hello() == false) {
	    module_info::getInstance()->set_upgrade_state(SYNC_ERR);
            printf("Send hello command fail\n");
            return false;
        } 
		/*
		hello packet will set dload flag in module, when upgrade interrup, restart module,module will enter dm(quectel sbl)
		*/
        if (handle_security_mode(1) == false) {
            printf("Send trust command fail\n");
	   module_info::getInstance()->set_upgrade_state(SYNC_ERR);
            return false;
        }

        if (handle_parti_tbl(0) == false) {
            printf("----------------------------------\n");
            printf("Detect partition mismatch.\n");
            printf("Download parition with override.\n");
            printf("----------------------------------\n");
            if(handle_parti_tbl(1) == false)
            {
            	printf("override failed. \n");
		module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR);
            	return false;
            }
            /*
            partition is not match, the download flag will be clear, so set it again, reset will clear it
            */
            if(handle_quectel_download_flag(1) == false)
            {
            	printf("Set Quectel download flag failed\n");
            }else
            {
            	printf("Set Quectel download flag successfully\n");
            }
            
        }
        dump = 0;

        if(ctx_ptr->update_method == 0)  //fastboot module
		{		           
            if(do_fastboot_download(ctx_ptr) != 0)
            {
            	 module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR);
            	return false;
            }
		}
        else if(ctx_ptr->update_method == 1)
        {
			if(do_streaming_download(ctx_ptr) != 0)
			{
				return false;
			}
        }

    }else if(STATE_NORMAL_MODE == ctx_ptr->TargetState)
    {
    	printf("Module in normal state, do download again.\n");
    	closeport();
    	//return false;
    	goto __normal_download_;
    }else
    {
    	printf("Module state is invalid, upgrade failed.\n");
	module_info::getInstance()->set_upgrade_state(UNKOWN);
    	return false;
    }
	transfer_statistics::getInstance()->process_bar((char *)"upgrade success\n");
	module_info::getInstance()->set_upgrade_state(SUCCESS);
       printf("The device restart...\n");
	printf("Welcome to use the Quectel module!!!\n");
    return true;
}
int do_streaming_download(download_context *pQdlContext)
{
	int ret = 0;
	for (std::vector<Ufile>::iterator iter = pQdlContext->ufile_list.begin();iter != pQdlContext->ufile_list.end();iter++)  
    {  
        if(strcmp("0:MIBIB",((Ufile)*iter).name)!=0)
		{
			//gettimeofday(&start,NULL);
			ret=do_flash_mbn(((Ufile)*iter).name, ((Ufile)*iter).img_name);
			//gettimeofday(&end,NULL);
			if(ret==false)
			{
				printf("down file:%s is faliled\n",((Ufile)*iter).name);
				return -2;
			}
		}
    }

    if (handle_reset() == false) {
        printf("Send reset command failed\n");
        return -1;
    }
    return 0;
}
int do_fastboot_download_direct(download_context *pQdlContext)
{
	ignore_sahara_stage_files(pQdlContext);
	if(downloadfastboot(pQdlContext) != 0)
	{
		return 1;
	}
	return 0;
}
int do_fastboot_download(download_context *pQdlContext)
{
	int ret = 0;
	for (std::vector<Ufile>::iterator iter = pQdlContext->ufile_list.begin();
            		iter!=pQdlContext->ufile_list.end();/*iter++*/) 
    {
    	if(strcmp("0:MIBIB",((Ufile)*iter).name)!=0)
		{
		
			if(strstr(((Ufile)*iter).name,"0:TZ") || strstr(((Ufile)*iter).name,"0:RPM") || strstr(((Ufile)*iter).name,"0:SBL") || strstr(((Ufile)*iter).name,"0:aboot"))
			{
				ret = do_flash_mbn(((Ufile)*iter).name, ((Ufile)*iter).img_name);
		    	if(ret != 1)
		    	{
		    		return -1;
		    	}
			    free_ufile((*iter));
			    iter = pQdlContext->ufile_list.erase(iter);
			}								
	        else
	        {
	        	iter++;
	        }
		}else
	    {
	    	iter++;
	    }
    }            

    printf("Change to fastboot mode...\n"); 
    do_flash_mbn("0:boot", "invalid-boot\n");  //write invalid boot for run fastboot
    sleep(1);
	if (handle_reset() == false) {
		printf("Send reset command failed\n");
		return -2;
	}

	closeport();
	if(wait_adb(DETECT_DEV_TIMEOUT) == 0)
	{
		if(downloadfastboot(pQdlContext) != 0)
		{
			return -3;
		}else
		{
		//upgrade success
			return 0;
		}
	}else
	{
		printf("Can't find adb port, upgrade failed.\n");
		return 1;
	}
	return 0;
}

void free_ufile(Ufile ufile)
{
	if( ufile.name != NULL)
	{
		free(ufile.name);
	}
	if(ufile.img_name != NULL)
	{
		free(ufile.img_name);
	}
	if(ufile.partition_name != NULL)
	{
		free(ufile.partition_name);
	}
}

