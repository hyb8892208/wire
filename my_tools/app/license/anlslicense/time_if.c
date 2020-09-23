#include <time.h>
#include "redis_if.h"

#define _test_

#define LINCESE_OUTPUT_RES			"/tmp/license_result.txt"
#define LICENSE_ENDTIME_FORMAT	"%Y-%m-%d %H:%M:%S"
#define LICENSE_ENDTIME_UNLIMIT	"1970-01-01 00:00:00"

extern int ast_is_stop();
extern int ast_send_at(int chan_id, char *nowstr);
extern char *get_endtime_str();

// write nowtime to system time.
static int save_local_time(struct tm *nowtime)
{
	int ret=-1;
	struct timeval tv;
	if (nowtime == NULL)
		return -1;
	tv.tv_sec = mktime(nowtime);
	tv.tv_usec = 0;
	/* need root user. */
	ret=settimeofday(&tv, NULL);
	if(ret < 0){
		printf("set system time failed!\n");
	}

	return ret;
}

static struct tm *get_local_time()
{
	time_t tv;
	struct tm *nowtime = NULL;

	time(&tv);
	nowtime = localtime(&tv);
#ifdef _test_
	char tmp_str[64];
	strftime(tmp_str, 64, "%Y-%m-%d %H:%M:%S", nowtime);
	printf("localtime=%s\n", tmp_str);
#endif
	return nowtime;
}

static int get_astat_time(struct tm *nowtime)
{
	int i;
	int res;
	char timestr[32];
	unsigned char reg_status[MAX_CHANNEL_COUNT];

	if (nowtime == NULL)
		return -1;
	if (ast_is_stop())
		return -1;
	memset(reg_status, 0, sizeof(unsigned char) * MAX_CHANNEL_COUNT);
	res = get_redis_gsmreg_status(reg_status);
	if (res < 0) {
		printf("get_astat_time failed. res=%d.\n", res);
		return res;
	}

	for(i = 0; i < res && i < MAX_CHANNEL_COUNT; i++) {
		if (0 == reg_status[i])
			continue;
		res = ast_send_at(i+1, timestr);
		if (0 == res) {
			if(timestr[0] == '\r')
				continue;
			strptime(timestr, "%Y/%m/%d,%H:%M:%S", nowtime);
			if(nowtime->tm_year < 0){ // 19/03/05,10:00:00 => 2019/03/05,10:00:00
				nowtime->tm_year += 2000;
			}
#ifdef _test_
	char tmp_str[64];
	strftime(tmp_str, 64, "%Y-%m-%d %H:%M:%S", nowtime);
	printf("asterisk time=%s\n", tmp_str);
#endif
			return 0;
		}
	}

	return -1;
}

static int get_endtime(struct tm *endtime)
{
	char *lic_endtime = NULL;

	lic_endtime = get_endtime_str();
	if (lic_endtime == NULL)
		return -1;
//	printf("license_endtime: [%s]\n", lic_endtime);
	strptime(lic_endtime, LICENSE_ENDTIME_FORMAT, endtime);
#ifdef _test_
	char tmp_str[64];
//	printf("lic_endtime=%s\n", lic_endtime);
	strftime(tmp_str, 64, "%Y-%m-%d %H:%M:%S", endtime);
	printf("license endtime=%s\n", tmp_str);
#endif

	return 0;
}

static int is_unlimited_time(struct tm *endtime)
{
	struct tm ulimit;

	strptime(LICENSE_ENDTIME_UNLIMIT, LICENSE_ENDTIME_FORMAT, &ulimit);
	if (ulimit.tm_year == endtime->tm_year && 
		ulimit.tm_mon == endtime->tm_mon &&
		ulimit.tm_mday == endtime->tm_mday &&
		ulimit.tm_hour == endtime->tm_hour &&
		ulimit.tm_min == endtime->tm_min &&
		ulimit.tm_sec == endtime->tm_sec){
		return 1;
	}

	return 0;
}

static int is_expired_time(struct tm *endtime, struct tm *nowtime)
{
	if (nowtime->tm_year > endtime->tm_year){
		return 1;
	} else if (nowtime->tm_year == endtime->tm_year) {
		if (nowtime->tm_mon > endtime->tm_mon) {
			return 1;
		} else if (nowtime->tm_mon == endtime->tm_mon) {
			if (nowtime->tm_mday > endtime->tm_mday) {
				return 1;
			} else if(nowtime->tm_mday == endtime->tm_mday) {
				if (nowtime->tm_hour > endtime->tm_hour) {
					return 1;
				} else if (nowtime->tm_hour == endtime->tm_hour) {
					if ((nowtime->tm_min > endtime->tm_min) || \
						(nowtime->tm_min == endtime->tm_min && nowtime->tm_sec >= endtime->tm_sec))
						return 1;
				}
			}
		}
	}

	return 0;
}

static int is_nearEnd_time(struct tm *endtime, struct tm *nowtime)
{
	int offset = 3 * 24 * 60 * 60; // 3 day => 3*24*60*60s
	struct timeval endtime_tv, nowtime_tv;                                                                                                                                                                         
	endtime_tv.tv_sec = mktime(endtime);
	nowtime_tv.tv_sec = mktime(nowtime);
	int differ = endtime_tv.tv_sec - nowtime_tv.tv_sec;
	if(differ <= offset && differ >= -offset){
		//printf("is near time, offset=%d,differ=%d!\n", offset,differ);
		return 1;
	}
	//printf("is not near time, differ=%d!\n", differ);

	return 0;
}

static int is_near_time(struct tm *local_time, struct tm *nowtime)
{
	int offset = 5 * 60; // 5 min
	struct timeval local_tv, nowtime_tv;                                                                                                                                                                         
	local_tv.tv_sec = mktime(local_time);
	nowtime_tv.tv_sec = mktime(nowtime);
	int differ = local_tv.tv_sec - nowtime_tv.tv_sec;
	if(differ <= offset && differ >= -offset){
		//printf("is near time, differ=%d!\n", differ);
		return 1;
	}
	//printf("is not near time, differ=%d!\n", differ);
	return 0;
}

static void try_save_local_time(struct tm *nowtime)
{
	int res;
	struct tm *local_time;

	if (nowtime)
		return;
	local_time = get_local_time();
	if (local_time == NULL)
		return;
	res = is_near_time(local_time, nowtime);
	if (0 == res) {
		save_local_time(nowtime);
	}
}


int check_endtime(int *unlimited_flag)
{
	int res;
	struct tm endtime;
	struct tm nowtime;
	struct tm *ptrtime = &nowtime;

	res = get_endtime(&endtime);
	if (res ) {
		printf("goto get_endtime failed.\n");
		return -1;
	}
	res = is_unlimited_time(&endtime);
	if (res) { //unlimited
		if (unlimited_flag)
			*unlimited_flag = 1;
//		printf("unlimited\n");
		return 0;
	}
	res = get_astat_time(&nowtime);
	if (res < 0) {
		ptrtime = get_local_time();
		if (ptrtime == NULL) {
			return -1;
		}
	} else {
		try_save_local_time(&nowtime);
	}

	res = is_expired_time(&endtime, ptrtime);
	if (res) {
		return -1;
	}
	res = is_nearEnd_time(&endtime, ptrtime);

	if(res == 1){
		printf("is near end time\n");
	}

	return res;
}


