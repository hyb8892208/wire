#include <iostream>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <termios.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "openvox_version_record.h"
#include "openvox_process_bar.h"
#include "os_linux.h"
char *baud = (char *)"115200";
//查询版本
#define VERSION_ACK "AT+CGMR\r\n"
//关闭回显
#define ECHO_ACK "ATE0\r\n"
//设置存放路径
#define UPLOAD_ACK "AT+CFTRANRX=\"C:/%s\",%d\r\n"
//查看模块文件列表
#define FSLS_ACK "AT+FSLS\r\n"
//设置某个文件为升级固件
#define CDELTA_ACK "AT+CDELTA=\"%s\"\r\n"
//模块重启
#define RESET_MODULE_ACK "AT+CRESET\r\n"
//调用rri接口删除上传到模块的文件
#define FSDEL_ACK "/my_tools/rri_cli at %d AT+fsDEL=%s"

#define POSIX_TERMIOS

struct firmware_t {
	int size;
	char filepath[256];
	char filename[256];
	
	char *content;
};

struct firmware_t firmware;
//设置串口参数
void com_setparms(int fd, char *baudr, char *par, char *bits, char *stopb, int hwf, int swf)
{
  int spd = -1;
  int newbaud;
  int bit = bits[0];

#ifdef POSIX_TERMIOS
  struct termios tty;
#else /* POSIX_TERMIOS */
  struct sgttyb tty;
#endif /* POSIX_TERMIOS */

#ifdef POSIX_TERMIOS
  tcgetattr(fd, &tty);
#else /* POSIX_TERMIOS */
  ioctl(fd, TIOCGETP, &tty);
#endif /* POSIX_TERMIOS */

  /* We generate mark and space parity ourself. */
  if (bit == '7' && (par[0] == 'M' || par[0] == 'S'))
    bit = '8';

    /* Check if 'baudr' is really a number */
  if ((newbaud = (atol(baudr) / 100)) == 0 && baudr[0] != '0')
    newbaud = -1;

  switch (newbaud) {
    case 0:
#ifdef B0
      spd = B0;
#else
      spd = 0;
#endif
      break;
    case 3:	spd = B300;	break;
    case 6:	spd = B600;	break;
    case 12:	spd = B1200;	break;
    case 24:	spd = B2400;	break;
    case 48:	spd = B4800;	break;
    case 96:	spd = B9600;	break;
#ifdef B19200
    case 192:	spd = B19200;	break;
#else /* B19200 */
#  ifdef EXTA
    case 192:	spd = EXTA;	break;
#   else /* EXTA */
    case 192:	spd = B9600;	break;
#   endif /* EXTA */
#endif	 /* B19200 */
#ifdef B38400
    case 384:	spd = B38400;	break;
#else /* B38400 */
#  ifdef EXTB
    case 384:	spd = EXTB;	break;
#   else /* EXTB */
    case 384:	spd = B9600;	break;
#   endif /* EXTB */
#endif	 /* B38400 */
#ifdef B57600
    case 576:	spd = B57600;	break;
#endif
#ifdef B115200
    case 1152:	spd = B115200;	break;
#endif
#ifdef B230400
    case 2304:	spd = B230400;	break;
#endif
#ifdef B460800
    case 4608: spd = B460800; break;
#endif
#ifdef B500000
    case 5000: spd = B500000; break;
#endif
#ifdef B576000
    case 5760: spd = B576000; break;
#endif
#ifdef B921600
    case 9216: spd = B921600; break;
#endif
#ifdef B1000000
    case 10000: spd = B1000000; break;
#endif
#ifdef B1152000
    case 11520: spd = B1152000; break;
#endif
#ifdef B1500000
    case 15000: spd = B1500000; break;
#endif
#ifdef B2000000
    case 20000: spd = B2000000; break;
#endif
#ifdef B2500000
    case 25000: spd = B2500000; break;
#endif
#ifdef B3000000
    case 30000: spd = B3000000; break;
#endif
#ifdef B3500000
    case 35000: spd = B3500000; break;
#endif
#ifdef B4000000
    case 40000: spd = B4000000; break;
#endif
  }

#ifdef POSIX_TERMIOS

  if (spd != -1) {
    cfsetospeed(&tty, (speed_t)spd);
    cfsetispeed(&tty, (speed_t)spd);
  }

  tty.c_cflag = 0;

  switch (bit) {
    case '5':
      tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
      break;
    case '6':
      tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
      break;
    case '7':
      tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
      break;
    case '8':
    default:
      tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
      break;
  }
  
  /* Set into raw, no echo mode */
  tty.c_iflag =  IGNBRK;
  tty.c_lflag = 0;
  tty.c_oflag = 0;
  tty.c_cflag |= CLOCAL | CREAD;
#ifdef _DCDFLOW
  tty.c_cflag &= ~CRTSCTS;
#endif
  tty.c_cc[VMIN] = 0;//1
  tty.c_cc[VTIME] = 0;//5

  if (swf)
    tty.c_iflag |= IXON | IXOFF;
  else
    tty.c_iflag &= ~(IXON|IXOFF|IXANY);

  tty.c_cflag &= ~(PARENB | PARODD);
  if (par[0] == 'E')
    tty.c_cflag |= PARENB;
  else if (par[0] == 'O')
    tty.c_cflag |= (PARENB | PARODD);

  if (stopb[0] == '2')
    tty.c_cflag |= CSTOPB;
  else
    tty.c_cflag &= ~CSTOPB;

  tcsetattr(fd, TCSANOW, &tty);
#endif /* POSIX_TERMIOS */
}
//获取文件大小
int get_file_size(char *filename){
	struct stat tbuf;
	int fd = open(filename, O_RDONLY);
	if(fd < 0){
		printf("open file error!\n");
		return -1;
	}
	fstat(fd, &tbuf);
	close(fd);
	return tbuf.st_size;	
}
//打开串口
int com_new(int comport)
{
    int fd;
	
	char dev_name[32]={0};
	sprintf(dev_name, "/dev/ttyUSB%d", comport);
	printf("open com:%s\n", dev_name);	
    fd = open(dev_name, O_RDWR|O_NDELAY|O_NOCTTY);
    if ( fd < 0 )
        return -1;

    com_setparms(fd, baud, (char *)"N", (char *)"8", (char *)"1", 0, 0);

    return fd;
}
//关闭回显
int close_echo(int ttyfd ){
	char tmp_buf[1024] = {0};
	if(write(ttyfd, (char *)ECHO_ACK, strlen(ECHO_ACK)) < 0){
		printf("close echo failed!\n");
		return -1;
	}
	usleep(20*1000);
	read(ttyfd, tmp_buf, 1024);
	return 0;
}
//读取固件
int read_fw(void){
	struct stat tbuf;
	int real_size;
	int fd = open(firmware.filepath, O_RDONLY);
	if(fd < 0){
		printf("open file error!\n");
		return -1;
	}
	
	fstat(fd, &tbuf);
	if(tbuf.st_size <= 0){
		printf("get filesize error!\n");
		return -1;
	}
	
	firmware.size = tbuf.st_size;
	
	firmware.content = (char *)malloc(tbuf.st_size + 1);
	if(firmware.content == NULL){
		printf("malloc memory fialed!\n");
		return -1;
	}
	
	real_size = read(fd, firmware.content, firmware.size);
	if(real_size != firmware.size){
		printf("read fireware error\n");
		return -1;
	}
	
	close(fd);
	
	return real_size;
}

//上传固件到模块
int upload_file(int ttyfd){
	char write_buf[1024] = {0};
	int write_total = 0;
	
	sprintf(write_buf, UPLOAD_ACK, firmware.filename, firmware.size );
	write(ttyfd, write_buf, strlen(write_buf));
	
	while(write_total < firmware.size){
		int real_size = write(ttyfd, firmware.content + write_total, 1024);
		if(real_size < 0){
			printf("write error\n");
			return -1;
		}
		usleep(30 * 1000);
		write_total += real_size;
		UpgradeCount::getInstance()->set_write_size(real_size);
		UpgradeCount::getInstance()->process_bar(NULL);
	}
	
	memset(write_buf, 0, 1024);
	
	usleep(300 * 1000);
	read(ttyfd, write_buf, 1024);
	if(strstr(write_buf, "OK")){
		printf("upload file ok!\n");
	}else{
		printf("upload file failed, result = %s\n", write_buf);
		return -1;
	}
	return 0;
}
//获取模块固件列表
int get_file_list(int ttyfd){
	char tmp_buf[1024] = {0};
	write(ttyfd, (char *)FSLS_ACK, strlen(FSLS_ACK));
	usleep(300 * 1000);
	read(ttyfd, tmp_buf, 1024);
	
	if(strstr(tmp_buf, firmware.filename)){
		printf("the file has upload\n");
	}else{
		printf("the file hasn't upload, %s\n", tmp_buf);
		return -1;
	}
	return 0;
}
//选择模块文件列表中的文件作为升级固件
int select_firmware(int ttyfd){
	char write_buf[1024] = {0};
	sprintf(write_buf, CDELTA_ACK, firmware.filename );
	write(ttyfd, write_buf, strlen(write_buf));
	
	memset(write_buf, 0, 1024);
	sleep(5);
	read(ttyfd, write_buf, 1024);
	
	// printf("cdelta:%s\n", buf);
	
	if(strstr(write_buf, "+CDELTA: 1")){
		printf("select upgrade file success!\n");
	}else{
		printf("select upgrade file failed, result=%s\n", write_buf);
		return -1;
	}
	return 0;
}
//重启模块，升级固件
int reset_module(int ttyfd){
	write(ttyfd, (char *)RESET_MODULE_ACK, strlen(RESET_MODULE_ACK));
	usleep(300*1000);
	return 0;
}

//删除模块列表中的某个固件
int fs_del(void){
	char tmp_buf[1024] = {0};
	sprintf(tmp_buf, FSDEL_ACK, module_info::getInstance()->get_channel(), firmware.filename);
	system(tmp_buf);
	return 0;
}
#if 0
//检测idVendor为05c6的第三个端口(at端口)
int det_ttyusb_device(){
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
			snprintf(subdir, sizeof(subdir), "%s/%s:1.%d",dir, ent->d_name, 3);//3 means at port
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
#endif
int at_processing(char *filename){
	// char filename[256] = {0};
	int ttyfd;
	char com[32] = {0};
	int opt;
	char *pos;

	if(access(filename, F_OK) == 0){
		strcpy(firmware.filepath, filename);
	}else{
		printf("firmware is not exist!\n");
		return -1;
	}
	pos = strrchr(firmware.filepath, '/');
	if(pos){
		strcpy(firmware.filename, pos + 1);
	}else{
		strcpy(firmware.filename, firmware.filepath);
	}
	
	module_info::getInstance()->set_start_time(NULL);
	module_info::getInstance()->set_old_version(NULL);
	module_info::getInstance()->set_mod_version(NULL);
	//检测升级串口
	int tmp_port = det_ttyusb_device(3);
	if(tmp_port < 0){
		UpgradeCount::getInstance()->process_bar((char *)"open com err");
		module_info::getInstance()->set_upgrade_state(COM_ERR);
		return -1;
	}
	//打开升级串口
	ttyfd = com_new(tmp_port);
	
	if(ttyfd < 0){
		printf("open com err\n");
		UpgradeCount::getInstance()->process_bar((char *)"open com err");
		module_info::getInstance()->set_upgrade_state(COM_ERR);
		return -1;
	}
	//关闭回显，避免影响上传固件后的返回追判断
	if(close_echo(ttyfd) < 0){
		printf("close echo failed!\n");
	}
	//读取固件内容
	int file_size = read_fw();
	if(file_size < 0){
		UpgradeCount::getInstance()->process_bar((char *)"read firmware err");
		module_info::getInstance()->set_upgrade_state(LOAD_ERR);
		return -1;
	}
	//设置升级文件总大小
	UpgradeCount::getInstance()->set_total_size(file_size);
	//上传固件到模块
	if(upload_file(ttyfd) < 0){
		free(firmware.content);
		UpgradeCount::getInstance()->process_bar((char *)"download file err");
		module_info::getInstance()->set_upgrade_state(DOWNLOAD_ERR);
		return -1;
	}
	//释放掉存储文件内容的缓存
	free(firmware.content);
	//获取模块文件列表
	if(get_file_list(ttyfd) < 0 ){
		UpgradeCount::getInstance()->process_bar((char *)"download end err");
		module_info::getInstance()->set_upgrade_state(END_ERR);
		return -1;
	}
	//指定升级固件
	if(select_firmware(ttyfd) < 0){
		UpgradeCount::getInstance()->process_bar((char *)"run fireware err");
		module_info::getInstance()->set_upgrade_state(RUN_ERR);
		return -1;
	}
	//重启模块
	reset_module(ttyfd);
	//通知页面可以切换到其他通道升级
	UpgradeCount::getInstance()->process_bar((char *)"upload ok");
	close(ttyfd);
	sleep(2);
	
	module_info::getInstance()->set_new_version(NULL);
	module_info::getInstance()->set_end_time(NULL);
	module_info::getInstance()->record_info_to_file();
	//通知页面升级成功
	UpgradeCount::getInstance()->process_bar((char *)"upgrade success");
	//删除上传到模块的固件
	fs_del();
	return 0;
}
