#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>


#define LED_KEY_CODE	0xC5

#define SET_KEY_LED_ON			_IO(LED_KEY_CODE, 1)
#define SET_KEY_LED_OFF			_IO(LED_KEY_CODE, 2)
#define LED_KEY_POLL_PERIOD		HZ/2

int main(int argc,char *argv[])
{
	char buf[20];
	int fd;
	fd_set rfds;
	int retval;
	//struct timeval tv;
	if(argc!=2){
		printf("argv error\n");
		return -1;
	}
	fd = open("/dev/led_key", O_RDWR);
	if(fd < 0)
	{
		perror("open default key failse");
		return -1;
	}
	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		//tv.tv_sec = 5;
		//tv.tv_usec = 0;
		//retval = select(fd+1, &rfds, NULL, NULL, &tv);
		retval = select(fd+1, &rfds, NULL, NULL, NULL);
		if (retval == -1)
			perror("select()");
		else if (retval) {
			read(fd, buf, 10);
			if(buf[0]==0x31)
			{
				//ioctl(fd,SET_KEY_LED_ON,0);
			}
			else if(buf[0]==0x32)
			{
				system(argv[1]);
			}
		}
		/*else
			printf("No data within five seconds.\n");*/
	}
	close(fd);
	return 0;
}
