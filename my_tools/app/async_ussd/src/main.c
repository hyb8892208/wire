#include "../include/ussd_inc.h"
#include <sys/file.h>

#define	__LOCK_FILE__		"/var/lock/async_ussd.lock"
extern void ussd_redis_init();

static int check_lock_process()
{
	int fd, rc;
	char *filename = __LOCK_FILE__;

	fd = open(filename, O_CREAT|O_RDWR, 0666);
	if (fd < 0)
		return -1;
	rc = flock(fd, LOCK_EX|LOCK_NB);
	if (rc < 0) {
		printf("%s is locked.\n", filename);
		close(fd);
		return -1;
	}

	return 0;
}

void sched_task()
{
	while(1) {
		sleep(1);
	}

	return ;
}

int main(void)
{
	if (check_lock_process())
		return -1;
	ussd_redis_init();
	sched_task();

	return 0;
}

