#include "redis_if.h"
#include "./liblicense/liblicense.h"

#define __LOCK_FILE__		"/var/lock/anlslicense.lock"

static int g_check_license_flag = 0;
static int g_check_license_res = 0;

extern int check_endtime(int *unlimited_flag);
extern void ast_stop();
extern void ast_start();

static int check_lock_process()
{
	int fd, rc;
	const char *filename = __LOCK_FILE__;

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

char *get_endtime_str()
{
	return get_license_endtime();
}

static int check_license()
{
	int ret;
	char *lic_endtime = NULL;

	ret = decode_license();
	lic_endtime = get_endtime_str();
	redis_set_license_endtime(lic_endtime);

	return ret;
}

static void recheck_license()
{
	g_check_license_res = check_license();
	g_check_license_flag = 1;
}

static void init_anls()
{
	redis_sap_t tmp_redis_sap;

	tmp_redis_sap.redis_check_license = recheck_license;
	redis_init(&tmp_redis_sap);

	return;
}

int main()
{
	int res;
	int check_res;
	int limit_on = 0;
	int stop_flag = 0;
	int unlimited_flag = 0;

	if (check_lock_process())
		return -1;
	init_anls();
	check_res = check_license();
	while (1) {
		if (unlimited_flag == 0) {
			if (check_res == 0) {	// pass check
				limit_on = 0;
				// check_time 
				if((res=check_endtime(&unlimited_flag)) < 0) {	// -1: limit; 0: unlimit, >0: other
					limit_on = 1;
				} else {
					if (unlimited_flag) {
						redis_set_license_status(LICENSE_STATUS_UNLIMITED);
					} else {
						if (res) 
							redis_set_license_status(LICENSE_STATUS_NEARLY);
						else
							redis_set_license_status(LICENSE_STATUS_VALID);
					}
				}
				printf("goto check_endtime. limit_on=%d.\n", limit_on);
			} else {	// no pass  check
				limit_on = 1;
			}
			if (limit_on) {
				stop_flag = 1;
				ast_stop();
				redis_set_license_status(LICENSE_STATUS_EXPIRED);
			} else {
				if (stop_flag) {
					stop_flag = 0;
					ast_start();
				}
			}
		}

		if (g_check_license_flag) {
			g_check_license_flag = 0;
			check_res = g_check_license_res;
			limit_on = 0;
			unlimited_flag = 0;
		}
		sleep(5);
	}

	return check_res;
}

