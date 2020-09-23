#include "../include/sms_inc.h"
#include <sys/file.h>
#include "sms_ami_if.h"
#include "log_debug.h"
#define	__LOCK_FILE__		"/var/lock/async_sms.lock"
extern void sms_redis_init();

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
	log_init();
	if (check_lock_process())
		return -1;
	sms_ami_init();
	sms_redis_init();
	sched_task();
	log_deinit();
	return 0;
}

