

#define __OS_LINUX_CPP_H__

#include "platform_def.h"

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_ANDROID)
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "os_linux.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "os_linux.h"
#include "stdarg.h"
#include "download.h"
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include "openvox_version_record.h"
#define MAX_TRACE_LENGTH      (256)

const char PORT_NAME_PREFIX[] = "/dev/ttyUSB";

static char log_trace[MAX_TRACE_LENGTH];
extern int at_processing(char *filename);
	
void show_log(const char *msg, ...)
{
    va_list ap;
        
    va_start(ap, msg);
    vsnprintf(log_trace, MAX_TRACE_LENGTH, msg, ap);
    va_end(ap);
	
    printf("%s\r\n", log_trace);
}

void prog_log(uint32 writesize,uint32 size,int clear)
{
    uint32 progress = (uint32)(writesize * 100)/ size;
	
    printf( "progress : %d%% finish\r", progress);
}

void qdl_msg_log(int msgtype,char *msg1,char * msg2)
{
}

void qdl_log(char *msg)
{
}

static int config_uart(int fd)
{
	/*set UART configuration*/
	struct termios newtio;
	
	if(tcgetattr(fd, &newtio) != 0)
		return -1;
	
	cfmakeraw(&newtio);
	
	//newtio.c_cflag &= ~CIGNORE;
	/*set baudrate*/
	cfsetispeed(&newtio, B115200);
	cfsetospeed(&newtio, B115200);
	
	/*set char bit size*/
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS8;
	
	/*set check sum*/
	//newtio.c_cflag &= ~PARENB;
	//newtio.c_iflag  &= ~INPCK;
	
	/*set stop bit*/
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag |= CLOCAL |CREAD;
	newtio.c_cflag &= ~(PARENB | PARODD);
	
	newtio.c_iflag &= ~(INPCK | BRKINT |PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
	newtio.c_iflag |= IGNBRK;
	newtio.c_iflag &= ~(IXON|IXOFF|IXANY);
	//newtio.c_iflag |= (INPCK | ISTRIP);
	
	newtio.c_lflag = 0;
	newtio.c_oflag = 0;
	
	//newtio.c_lflag &= ~(ECHO | ECHONL |ICANON|ISIG|IEXTEN);
	//newtio.c_iflag |= (INPCK | ISTRIP);
	
	/*set wait time*/
	newtio.c_cc[VMIN] = 0;
	newtio.c_cc[VTIME] = 10;
	
	tcflush(fd, TCIFLUSH);
	tcflush(fd, TCOFLUSH);

	if(tcsetattr(fd, TCSANOW, &newtio) !=0 )
		return -1;
	
	return 0;
}

/*   This   program   is   part   of   Microsoft   C   Run-time   Library.
      This   program   is   not   free   software;   you   can 't   redistribute   it
      and/or   modify   it   due   to   the   terms   of   the   Microsoft   License   as
      published   by   the   Microsoft   Corporation;

      This   program   is   distributed   in   the   hope   that   it   will   be   useful,
      but   WITHOUT   ANY   WARRANTY;   without   even   the   implied   warranty   of
      MERCHANTABILITY   or   FITNESS   FOR   A   PARTICULAR   PURPOSE.     See   the
      Microsoft   License   for   more   details.

      You   should   have   received   a   copy   of   the   Microsft   License   along
      with   this   program;   if   not,   write   to   the   Microsoft   Corporation.
      WA   One   Microsoft   Way,   Redmond,   WA   98052*/

char   *itoa(   int   val,   char   *buf,   unsigned   radix   )
{
          char   *p;                                 /*   pointer   to   traverse   string   */
          char   *firstdig;                   /*   pointer   to   first   digit   */
          char   temp;                             /*   temp   char   */
          unsigned   digval;                 /*   value   of   digit   */
          
          p   =   buf;
          
          if   (val <0)   {
	          /*   negative,   so   output   '- '   and   negate   */
	          *p++   =   '- ';
	          val   =   (unsigned   long)(-(long)val);
          }
          
          firstdig   =   p;                       /*   save   pointer   to   first   digit   */
          
          do   {
	          digval   =   (unsigned)   (val   %   radix);
	          val   /=   radix;               /*   get   next   digit   */
	          
	          /*   convert   to   ascii   and   store   */
	          if   (digval   >   9)
	         	 *p++   =   (char)   (digval   -   10   +   'a ');     /*   a   letter   */
	          else
	         	 *p++   =   (char)   (digval   +   '0 ');               /*   a   digit   */
          }   while   (val   >   0);
          
          /*   We   now   have   the   digit   of   the   number   in   the   buffer,   but   in   reverse
          order.     Thus   we   reverse   them   now.   */
          
          *p--   =   '\0 ';                         /*   terminate   string;   p   points   to   last   digit   */
          
          do   {
	          temp   =   *p;
	          *p   =   *firstdig;
	          *firstdig   =   temp;       /*   swap   *p   and   *firstdig   */
	          --p;
	          ++firstdig;                   /*   advance   to   next   two   digits   */
          }   while   (firstdig   <   p);   /*   repeat   until   halfway   */
        
	return   buf;
}
int det_ttyusb_device(int interface){
	struct dirent* ent = NULL;
	DIR* pDir;
	char dir[255] = "/sys/bus/usb/devices";
	pDir = opendir(dir);
	int ret = 1;
	int tmp_port = -1;
	if(pDir){	
		while((ent = readdir(pDir)) != NULL){
			struct dirent* subent = NULL;
			DIR *psubDir;
			char subdir[255];
			char dev[255];
			char idVendor[4 + 1] = {0};
			char idProduct[4 + 1] = {0};
			char number[10] = {0};
			int fd = 0;
			char diag_port[32] = "\0";
			snprintf(subdir, sizeof(subdir), "%s/%s/idVendor", dir, ent->d_name);
			fd = open(subdir, O_RDONLY);
			if (fd > 0) {
				read(fd, idVendor, 4);
				close(fd);
			}else{
					continue;
			}
			snprintf(subdir, sizeof(subdir), "%s/%s/idProduct", dir, ent->d_name);
			fd  = open(subdir, O_RDONLY);
			if (fd > 0) {
				read(fd, idProduct, 4);
				close(fd);
			}else{ 
				continue;
			}
//			printf("idVendor = %s, idProduct = %s, dir = %s/%s\n", idVendor, idProduct, dir, ent->d_name); 
			if (!strncasecmp(idVendor, "05c6", 4) )
				;
			else
				continue;                        //snprintf(subdir, sizeof(subdir), "%s/%s:1.0",dir, ent->d_name);
			snprintf(subdir, sizeof(subdir), "%s/%s:1.%d",dir, ent->d_name, interface);
			psubDir = opendir(subdir);
			if(psubDir == NULL){
				continue;
			}
			while((subent = readdir(psubDir)) != NULL){
				if(subent->d_name[0] == '.')
					continue;
				if(!strncasecmp(subent->d_name, "ttyUSB", 6))
				{
					strcpy(diag_port, subent->d_name+6);
					break;
				}
			}
			closedir(psubDir);
			closedir(pDir);
			if(strlen(diag_port) > 0)
				tmp_port = atoi(diag_port);
			}
		}
	return tmp_port;
}

int openport(int *com_port)
{
	int tmp_port = 7;
	int retry = 6;
	char pc_comport[32];	
        if(hCom) 
	{
		QdlContext->text_cb("in openport, but already opened!");
		return TRUE;  /*already opened*/
        }
	
	
	tmp_port = det_ttyusb_device(0);
	if(tmp_port < 0)
		return false;
start_probe_port:
	memset(pc_comport,0,sizeof(pc_comport));

	sprintf(pc_comport, "%s%d", PORT_NAME_PREFIX, tmp_port);

	if(access(pc_comport, F_OK))
	{
		QdlContext->text_cb("start open com port: %s, but no such device, try again[%d]", pc_comport, retry);

		tmp_port++;
		retry--;

		if(retry > 0)
			goto start_probe_port;
		else
			return FALSE;
	}
		
	QdlContext->text_cb("start open com port: %s", pc_comport);
		
	hCom = (HANDLE) open(pc_comport, O_RDWR | O_NOCTTY);
	
	if(hCom == (HANDLE)-1)
	{
		hCom = NULL;
		return FALSE;
	}
	else
		config_uart((int)hCom) ;

	*com_port = tmp_port;
	
	return TRUE;
}

int closeport(HANDLE com_fd)
{
	QdlContext->text_cb("start close com port");
	
	if(!com_fd)
		return 1;
	
	close(com_fd);

	hCom = NULL;
	
	return 1;
}

int WriteABuffer(HANDLE file, unsigned char * lpBuf, int dwToWrite)
{
	int written = 0;
	
	assert(file != (HANDLE) -1);
	assert(lpBuf);
	
	if(dwToWrite <= 0)
		return dwToWrite;
	
	//QdlContext->text_cb("start write :%d\r\n", dwToWrite);
	/*just write data to device*/
	written = write(file, lpBuf, dwToWrite);
	//QdlContext->text_cb("after write :%d / %d", dwToWrite, written);

	if(written < 0)   
	{
		qdl_text_cb("write error!");
		return 0;
	}
	else 
		return written;
}

int ReadABuffer(HANDLE file, unsigned char * lpBuf, int dwToRead)
{
	int read_len = 0;
	
	assert(lpBuf);
	
	if(dwToRead <= 0)
		return 0;
	
	/*just write data to device*/
	//QdlContext->text_cb("start read :%d\r\n",dwToRead);
	read_len = read(hCom, lpBuf, dwToRead);
	//QdlContext->text_cb("after read :%d\r\n",read_len);
	if(read_len < 0)
        {
	     QdlContext->text_cb("read com error :%d", read_len);
	     read_len = 0;
	}
		
	return read_len;
}

void qdl_flush_fifo(HANDLE fd, int tx_flush, int rx_flush)
{
	if(tx_flush)
		tcflush(fd, TCOFLUSH);

	if(rx_flush)
		tcflush(fd, TCIFLUSH);
}

void qdl_sleep(int millsec)
{
	/*linux sleep function's unit is second*/
	int second = millsec / 1000;
	
	if(millsec % 1000)
		second += 1;
	
	sleep(second);
}

extern int get_cfg_file_data(char * aszSession,        
                        char * aszKey,        
                        char * aszDefault,        
                        char *  aszText,  
                        int   nSize,            
                        char * aszFileName);
	
void qdl_pre_download(char *filename)
{
	time_t tm;
	time(&tm);
	int opt;
	/*
	  * this point QdlContext->text_cb is NULL,
	  * so we can only use the real print function
	  */
	show_log("SIMTech download tool, %s", ctime(&tm));
	
	dload_cfg_type  cfg_info;

	cfg_info.ComPortNumber = 0;

#if 0
  	while( (opt = getopt(argc, argv, "f:c:h")) > 0){
		switch (opt){
			case 'f':
				printf("f=%s\n", optarg);
				if(access(optarg, F_OK) == 0)
					strcpy(cfg_info.ImagePath, optarg);
				else
					strcpy(cfg_info.ImagePath, "./");
				break;
			case 'c':
				set_upgrade_channel(atoi(optarg));
//				printf("optarg = %s\n", optarg);
				break;
			case 'h':
				printf("usage:%s [-f dir] [-c channel]\n", argv[0]);
				break;
		}
	}
#endif
	strcpy(cfg_info.ImagePath, filename);
	cfg_info.TargetPlatform = TARGET_PLATFORM_6085; 
	
	char profile_value[255];
	char  g_dir[MAX_PATH]; 
	
	strcpy(g_dir, "./config.ini");
	
  /*get config file data*/
	get_cfg_file_data("QCN","backup","0",profile_value,255,g_dir);
	cfg_info.BackupEanble = atoi(profile_value);
	
	memset(profile_value, 0, sizeof(profile_value));
	get_cfg_file_data("QCN","restore","0",profile_value,255,g_dir);
	cfg_info. RestoreEanble = atoi(profile_value);
	
	memset(profile_value, 0, sizeof(profile_value));
	get_cfg_file_data("FILE SYSTEM","Erase_EFS","0",profile_value,255,g_dir);
	cfg_info.EraseEanble = atoi(profile_value);	
		 
	ProcessInit(&cfg_info, show_log, qdl_msg_log, qdl_log, prog_log);
}

void qdl_post_download(void)
{
	time_t tm;
	time(&tm);

	ProcessUninit();

	/*
	  * this point QdlContext->text_cb is NULL,
	  * so we can only use the real print function
	  */
	show_log("SIMTech download finished, %s", ctime(&tm));
}

int main(int argc, char *argv[])           
{	
	char filename[256] = {0};
	int mode = -1;
	int opt = 0;
	int channel = -1;
	
	while( (opt = getopt(argc, argv, "f:c:m:h")) > 0){
		switch (opt){
			case 'f':
				if(access(optarg, F_OK) == 0){
					strcpy(filename, optarg);
				}
				else
					return -1;
				break;
			case 'c':
				set_upgrade_channel(atoi(optarg));
				break;
			case 'm':
				mode = atoi(optarg);
				break;
			case 'h':
				printf("usage:%s [-f firmware] [-c channel]\n", argv[0]);
				break;
		}
	}
	if(strlen(filename) == 0){
		printf("usage:%s [-f firmware] [-c channel] [-m mode]");
		return 1;
	}

	if(mode == 1)
		Processing(filename, NULL);
	else
		at_processing(filename);

	return 0;
}
#endif/*TARGET_OS_LINUX*/

