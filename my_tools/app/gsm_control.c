/**********************************************************************************
Description : this program use to test com
Author      : zhongwei.peng@openvox.cn
Time        : 2016.07.18
Note        : 
***********************************************************************************/
#include <sys/types.h>
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
#include <errno.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0

#define BAUD_RATE "115200"

#define SIM_ENABLE_ON_L "write 0=FFh\n"   // 前8个模块的SIM使能--开
#define SIM_ENABLE_OFF_L "write 0=00h\n"  // 前8个模块的SIM使能--关
#define SIM_ENABLE_ON_H "write 1=FFh\n"   // 后8个模块的SIM使能--开
#define SIM_ENABLE_OFF_H "write 1=00h\n"  // 后8个模块的SIM使能--开

#define POWER_ON_L "write 2=00h\n"  // 前8个模块的供电控制--开机
#define POWER_OFF_L "write 2=FFh\n" // 前8个模块的供电控制--关机
#define POWER_ON_H "write 3=00h\n"  // 后8个模块的供电控制--开机
#define POWER_OFF_H "write 3=FFh\n" // 后8个模块的供电控制--关机

#define MODULE_DOWN_L "write 6=00h\n"  // 前8个模块的模块开关控制--拉低
#define MODULE_UP_L "write 6=FFh\n" // 前8个模块的模块开关控制--拉高
#define MODULE_DOWN_H "write 7=00h\n"  // 后8个模块的模块开关控制--拉低
#define MODULE_UP_H "write 7=FFh\n" // 后8个模块的模块开关控制--拉高


#define CLI_MAX_LEN (256)

#define POSIX_TERMIOS

unsigned char g_input_buf[CLI_MAX_LEN];
unsigned int g_input_idx = 0;

int	g_com_fd = -1;
int g_thd_flag = 0;

int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,
	    38400,  19200,  9600, 4800, 2400, 1200,  300, };

/*
 * Set baudrate, parity and number of bits.
 */
void m_setparms(int fd, char *baudr, char *par, char *bits, char *stopb,
                int hwf, int swf)
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

int com_open(char *dev_name)
{
	int n = 0;
	//g_com_fd = open(dev_name, O_RDWR|O_NOCTTY);
	
        g_com_fd = open(dev_name, O_RDWR|O_NDELAY|O_NOCTTY);
	
	if ( g_com_fd < 0 )
	{
		printf("can not open dev %s, ret = %d\n", dev_name, g_com_fd);
		return -1;
	}
	
	return g_com_fd;
}

void com_close()
{
	if ( g_com_fd > 0 )
	{
		close(g_com_fd);
		g_com_fd = -1;
	}
}

int com_write(unsigned char *buf, unsigned int len)
{
	int ret = -1;
	
	ret = write(g_com_fd, buf, len);
	return ret;
	//return write(g_com_fd, buf, len);
	
}
void check_io(unsigned char *read_buf, unsigned int read_len)
{
  int fd1 = g_com_fd;
  int n = 0, i,j;
  struct timeval tv;
  fd_set fds;
  int tmout = 1000;
  int ret = 0;

  tv.tv_sec = tmout / 1000;
  tv.tv_usec = (tmout % 1000) * 1000L;
  
  i = fd1;

  FD_ZERO(&fds);
  FD_SET(fd1, &fds);

  if ( select(i + 1, &fds, NULL, NULL, &tv) > 0 ) {
	  if ( FD_ISSET(fd1, &fds) > 0 ) {
		ret = read(g_com_fd, read_buf, read_len);
		  
		if ( ret > 0 ) {
			read_buf[ret] = 0;
			printf("%s", read_buf);
		}
	  }
  }
 
  return;
}

void proc_cmd()
{
	if ( strstr(g_input_buf, "quit") != NULL )
	{
		g_thd_flag = 0;
		sleep(3);
		com_close();
		exit(0);
	}
	
	if ( strncmp("pdu:", g_input_buf, 4) == 0 )
	{
		com_write(g_input_buf + 4, g_input_idx - 4);
		g_input_buf[0] = 0x1a;
		com_write(g_input_buf, 1);
	}
	else
	{
		com_write(g_input_buf, g_input_idx);
	}

	//printf("input,strings[%d]:(%s)\n",g_input_idx,g_input_buf);
	memset(g_input_buf, 0, CLI_MAX_LEN);
	g_input_idx = 0;
	
	return;
}

// 发送模块供电和模块开关控制
void send_cli(char *cmd)
{
	strncpy(g_input_buf, cmd, strlen(cmd));
	g_input_idx = strlen(cmd);
	proc_cmd();
	sleep(1);
	return ;
}

// 发送read和write操作指令
void send_command(char *type, char *data)
{
	strncpy(g_input_buf, type, strlen(type));
	strcat(g_input_buf, " ");
	strcat(g_input_buf, data);
	g_input_idx = strlen(g_input_buf);
	g_input_buf[g_input_idx++] = '\n';
	proc_cmd();
	return ;
}

void clean_buf()
{
	unsigned char read_buf[1024];
	unsigned int read_len = 1024;

	while(read(g_com_fd, read_buf, read_len) > 0 );
}

void operate(char *type, char *data)
{
	unsigned char read_buf[1024];
	unsigned int read_len = 1024;
	if (strstr(type, "run") != NULL ) {
		if (strstr(data, "reboot") != NULL ) {
			//printf("run reboot.\n");
			send_cli(POWER_OFF_H);
			send_cli(POWER_OFF_L);
			send_cli(POWER_ON_H);
			send_cli(POWER_ON_L);
			send_cli(SIM_ENABLE_ON_L);
			send_cli(SIM_ENABLE_ON_H);
			// 模块开机，首先拉低电平，持续0.6——1s，然后拉高电平
			send_cli(MODULE_DOWN_L);
			send_cli(MODULE_DOWN_H);
			usleep(700000);
			send_cli(MODULE_UP_L);
			send_cli(MODULE_UP_H);
		}
		else if (strstr(data, "stop") != NULL ) {
			//printf("run stop.\n");
			send_cli(POWER_OFF_H);
			send_cli(POWER_OFF_L);
		}
		else if (strstr(data, "start") != NULL ) {
			//printf("run start.\n");
			//前8个和后8个模块供电
			send_cli(POWER_ON_H);
			send_cli(POWER_ON_L);
			//前8个和后8个SIM卡使能
			send_cli(SIM_ENABLE_ON_L);
			send_cli(SIM_ENABLE_ON_H);
			// 模块开机，首先拉低电平，持续0.6——1s，然后拉高电平
			send_cli(MODULE_DOWN_L);
			send_cli(MODULE_DOWN_H);
			usleep(700000);
			send_cli(MODULE_UP_L);
			send_cli(MODULE_UP_H);
		}
	} else if (strstr(type, "read") != NULL ) {
		clean_buf();
		send_command(type, data);
		memset(read_buf, 0, 1024);
		read_len = 1024;
		check_io(read_buf, read_len);
	} else if (strstr(type, "write") != NULL ) { 
		clean_buf();
		send_command(type, data);
		memset(read_buf, 0, 1024);
		read_len = 1024;
		check_io(read_buf, read_len);
	} else {
		printf("Unknown command type.Please check out.\n");
		return ;
	}
}


void usage(int argc, char *argv[])
{
	char *file_name = argv[0];
	
	printf("Usage:\n");
	printf("%s <devname> run start|stop|reboot\n", file_name);
	printf("%s <devname> read <0-206>\n", file_name);
	printf("%s <devname> write <0-9|15-206>=XXh\n", file_name);
}

int main(int argc, char *argv[])
{
	if ( argc < 4 ) {
		usage(argc,argv);
		return -1;
	}
	
	if ( com_open(argv[1]) < 0 )
	{
		printf("Open dev(%s) error.\n",argv[1]);
		return -1;
	}
	m_setparms(g_com_fd, BAUD_RATE, "N", "8", "1", 1, 0);
	
	memset(g_input_buf, 0, CLI_MAX_LEN);
 	g_input_idx = 0;

	operate(argv[2], argv[3]);

	printf("Operate finish, close the MCU COM\n");
	com_close();
	return 0;
	
}

