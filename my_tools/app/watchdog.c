#include <stdio.h>  
#include <unistd.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <signal.h>
#include <linux/watchdog.h>  
  
int main(int argc, char **argv){  
	int fd;  
	pid_t pid;
	int fdtablesize;
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	if(pid = fork())
		exit(0);
	else if(pid < 0)
		exit(-1);
	if(setsid() < 0)
		exit(1);
	if(pid = fork())
		exit(0);
	else if(pid < 0)
		exit(-1);
	for (fd = 0, fdtablesize = getdtablesize(); fd < fdtablesize; fd++)
		close(fd);
	umask(0);
	signal(SIGCHLD, SIG_IGN);
	fd = open("/dev/comcerto_wdt",O_RDWR);  
	if(fd < 0){  
		perror("/dev/comcerto_wdt");  
		return -1;  
	}

	for(;;){  
		ioctl(fd, WDIOC_KEEPALIVE);  
		sleep(1);  
	}
	  
	close(fd);  
	return 0;  
}
