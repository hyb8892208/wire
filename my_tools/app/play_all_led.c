#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PROC_LED1 "/proc/gsm_module_led_key"
#define PROC_LED2 "/proc/gsm_module_led-0"

int fd_led1 = -1;
int fd_led2 = -2;

void led1_off()
{
	write(fd_led1,"0",2);
	//lseek(fd_led1,0,SEEK_SET);
}

void led2_off()
{
	write(fd_led2,"00",3);
	//lseek(fd_led2,0,SEEK_SET);
	write(fd_led2,"10",3);
	//lseek(fd_led2,0,SEEK_SET);
	write(fd_led2,"20",3);
	//lseek(fd_led2,0,SEEK_SET);
	write(fd_led2,"30",3);
	//lseek(fd_led2,0,SEEK_SET);
}

void led1_on()
{
	write(fd_led1,"1",2);
	//lseek(fd_led1,0,SEEK_SET);
}

void led2_on()
{
	write(fd_led2,"03",3);
	//lseek(fd_led2,0,SEEK_SET);
	write(fd_led2,"13",3);
	//lseek(fd_led2,0,SEEK_SET);
	write(fd_led2,"23",3);
	//lseek(fd_led2,0,SEEK_SET);
	write(fd_led2,"33",3);
	//lseek(fd_led2,0,SEEK_SET);
}

void all_ledoff()
{
	led1_off();
	led2_off();
}

void all_ledon()
{
	led1_on();
	led2_on();
}

int main(int argc, char* argv[])
{
	int interval_time=300; //millisecond
	int times=3;
	int i;

	switch (argc) {
	case 2:
		interval_time = atoi(argv[1]);
		break;
	case 3:
		interval_time = atoi(argv[1]);
		times = atoi(argv[2]);
		break;
	default:
		break;
	}

	if(interval_time < 0 || interval_time >= 20000) {
		interval_time = 300;
	}

	if(times < 1 || times >=100 ) {
		times = 3;
	}


	fd_led1 = open(PROC_LED1,O_RDWR);
	if(fd_led1 < 0) {
		printf("Open %s failed\n",PROC_LED1);
		exit(255);
	}

	fd_led2 = open(PROC_LED2,O_RDWR);
	if(fd_led2 < 0) {
		printf("Open %s failed\n",PROC_LED2);
		exit(255);
	}


	for(i=1; i<=times; i++) {
		all_ledon();
		usleep(interval_time*1000);
		all_ledoff();
		usleep(interval_time*1000);
	}

	close(fd_led1);
	close(fd_led2);
}

