#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0


int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300,
	    38400,  19200,  9600, 4800, 2400, 1200,  300, };

void set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if  (status != 0)
				perror("tcsetattr fd1");
			return;
		}
		tcflush(fd,TCIOFLUSH);
	}
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if ( tcgetattr( fd,&options)  !=  0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) {
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return (FALSE);
	}

	switch (parity) {
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 'S':
		case 's':
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
	}

	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
	}

	if (parity != 'n')
		options.c_iflag |= INPCK;

//	options.c_cc[VTIME] = 150; // 15 seconds
	options.c_cc[VTIME] = 15; // 1.5 seconds
	options.c_cc[VMIN] = 0;

	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/

	tcflush(fd,TCIFLUSH);
	if (tcsetattr(fd,TCSANOW,&options) != 0) {
		perror("SetupSerial 3");
		return (FALSE);
	}

	return (TRUE);

}


int OpenDev(char *Dev)
{
	int	fd = open(Dev, O_RDWR);         //| O_NOCTTY | O_NDELAY
	if (-1 == fd) {
		perror("Can't Open Serial Port");
		return -1;
	} else {
		return fd;
	}
}

int main(int argc, char **argv)
{
#define SEND_DATA "backport"

	int fd;
	int nread;
	char buff[64+1];
	int i;
	char *dev ="/dev/ttyS1";
	fd = OpenDev(dev);

	if (fd>0) {
		set_speed(fd,1200);
	} else {
		perror("Can't Open Serial Port!\n");
		exit(1);
	}

	if( !set_Parity(fd,8,1,'N') ) {
		perror("Set Parity Error\n");
		exit(1);
	}

	write(fd,SEND_DATA,sizeof(SEND_DATA));
	//usleep(500*1000);
	if((nread = read(fd,buff,64)) > 0) {
		switch(buff[0]) {
		case 0x00:
			printf("1");
			break;
		case 0x01:
			printf("2");
			break;
		case 0x02:
			printf("3");
			break;
		case 0x04:
			printf("4");
			break;
		case 0x05:
			printf("5");
			break;
		case 0x06:
			printf("6");
			break;
		case 0x07:
			printf("7");
			break;
		case 0x08:
			printf("8");
			break;
		case 0x09:
			printf("9");
			break;
		case 0x0a:
			printf("10");
			break;
		case 0x0b:
			printf("11");
			break;
		default:
			fprintf(stderr,"Get backport failed(0x%X)\n",buff[0]);
			break;
		}
	}

	close(fd);
	return 0;
}
