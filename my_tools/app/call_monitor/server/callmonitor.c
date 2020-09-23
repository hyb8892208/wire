#include "config.h"
#include "callmonitor_log.h"
#include "callmonitor.h"
#include "ami_interface.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>


#define QUEUESIZE 20
#define HW_INFO_FILE "/tmp/hw_info.cfg"
#define CALL_MONITOR_CONF "/etc/asterisk/gw/call_monitor.conf"
#define CALL_MONITOR_STATUS "/data/log/call_monitor_status.conf"

struct call_monitor_s g_call_monitor;

extern int gsoap_thread_create(void);
extern int get_total_channel();
static void sys_info_init(struct call_monitor_s *p_call_monitor)
{
	p_call_monitor->total_chan = get_total_channel();
	LOG_PRINT(LOG_INFO, "total_chan = %d\n", p_call_monitor->total_chan);
}

static enum sim_state_e get_chan_sim_state(int chan){
	char buf[32] = {0};
	char cmd[128] = {0};
	FILE *handle = NULL;
	char filepath[64] = {0};
	sprintf(filepath, "/tmp/gsm/%d", chan);
	if(access(filepath, F_OK) != 0)
		return SIM_STATE_UNKOWN;
	sprintf(cmd, "cat /tmp/gsm/%d|grep State |awk '{print $2}'", chan);
	handle = popen(cmd, "r");
	if(handle == NULL)
		return SIM_STATE_UNKOWN;
	if(fread(buf, 1, sizeof(buf), handle) < 0)
		return SIM_STATE_UNKOWN;
	if(strncmp(buf, "READY", strlen("READY")) == 0){
		LOG_PRINT(LOG_INFO, "%d is ready\n", chan);
		return SIM_STATE_READY;
	}
	LOG_PRINT(LOG_INFO, "%d is %s", chan, buf);
	return SIM_STATE_UNKOWN;
}

static int call_monitor_check_config(struct call_monitor_s *p_call_monitor){
	int i = 0;
	int total_chan = p_call_monitor->total_chan;
	if(p_call_monitor->glb_conf.call_monitor_switch == 0){
		p_call_monitor->glb_conf.fixed_time_switch = 0;
		for(i = 0; i < total_chan; i++){
			p_call_monitor->chans[i].conf.total_call_answers = 0;
			p_call_monitor->chans[i].conf.total_call_times = 0;
			p_call_monitor->chans[i].conf.total_call_dur = 0;
			
			p_call_monitor->chans[i].data.cur_call_answers = 0;
			p_call_monitor->chans[i].data.cur_call_times = 0;
			p_call_monitor->chans[i].data.cur_call_answers = 0;
			p_call_monitor->chans[i].conf.handle_type = 0;
		}
	}else{
		for(i = 0; i < total_chan; i++){
			if(p_call_monitor->chans[i].conf.total_call_answers == 0)
				p_call_monitor->chans[i].data.cur_call_answers = 0;
			if(p_call_monitor->chans[i].conf.total_call_times== 0)
				p_call_monitor->chans[i].data.cur_call_times = 0;
			if(p_call_monitor->chans[i].conf.total_call_dur = 0)
				p_call_monitor->chans[i].data.cur_call_dur = 0;
		}
	}
}
static int call_monitor_init(struct call_monitor_s *p_call_monitor, int reload_flag)
{
	int i = 0;
	char value[128] = {0};
	char context[12] = {0};
	struct config call_monitor_conf, call_monitor_status;
	load_config(CALL_MONITOR_CONF, &call_monitor_conf);
	load_config(CALL_MONITOR_STATUS, &call_monitor_status);
	
	if(reload_flag == 0){
		p_call_monitor->chans = (struct chan_info_s *)malloc(sizeof(struct chan_info_s)*p_call_monitor->total_chan);

		if(!p_call_monitor->chans)
			return -1;
		memset(p_call_monitor->chans, 0, sizeof(struct chan_info_s)*p_call_monitor->total_chan);
	}

	if(get_line_value(&call_monitor_conf, "general", "callmonitor_switch", value) == 0){
		if(strcmp(value, "yes") == 0){
			p_call_monitor->glb_conf.call_monitor_switch = 1;
		}else{
			p_call_monitor->glb_conf.call_monitor_switch = 0;
		}
	}else{
		p_call_monitor->glb_conf.call_monitor_switch = 0;
	}
	
	if(get_line_value(&call_monitor_conf, "general", "reset_flag", value) == 0){
		if(strstr(value, "yes"))
		       p_call_monitor->glb_conf.reset_flag = RESET_FLAG_ON;
		else
			p_call_monitor->glb_conf.reset_flag = RESET_FLAG_OFF;
	}else
		 p_call_monitor->glb_conf.reset_flag = RESET_FLAG_OFF;

	if(get_line_value(&call_monitor_conf, "general", "start_time", value) == 0){
		strptime(value, "%H:%M:%S", &p_call_monitor->glb_conf.start_time);
	}else{
		memset(&p_call_monitor->glb_conf.start_time, 0, sizeof(p_call_monitor->glb_conf.start_time));
	}

	if(get_line_value(&call_monitor_conf, "general", "end_time", value) == 0){
		strptime(value, "%H:%M:%S", &p_call_monitor->glb_conf.end_time);
	}else{
		memset(&p_call_monitor->glb_conf.end_time, 0, sizeof(p_call_monitor->glb_conf.end_time));
	}

	if(get_line_value(&call_monitor_conf, "general", "call_max_time", value) == 0)
		p_call_monitor->glb_conf.max_time = atoi(value);
	else
		p_call_monitor->glb_conf.max_time = 50;

	if(get_line_value(&call_monitor_conf, "general", "call_min_time", value) == 0)
		p_call_monitor->glb_conf.min_time = atoi(value);
	else
		p_call_monitor->glb_conf.min_time = 30;

	if(get_line_value(&call_monitor_conf, "general", "fixed_time_call_switch",value) == 0){
		if(strcmp(value, "yes") == 0){
			p_call_monitor->glb_conf.fixed_time_switch = 1;
		}else{
			p_call_monitor->glb_conf.fixed_time_switch = 0;
		}
	}else{
		p_call_monitor->glb_conf.fixed_time_switch = 0;
	}

	if(get_line_value(&call_monitor_conf, "general", "fixed_time_call_type",value) == 0){
		if(strcmp(value, "day") == 0){
			p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_DAYS;
		}else if(strcmp(value, "2days") == 0){
			p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_2DAYS;
		}else if(strcmp(value, "3days") == 0){
			p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_3DAYS;
		}else if(strcmp(value, "4days") == 0){
			p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_4DAYS;
		}else if(strcmp(value, "5days") == 0){
			p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_5DAYS;
		}else if(strcmp(value, "week") == 0){
			p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_WEEK;
		}else if(strcmp(value, "month") == 0){
			p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_MONTH;
		}else
			p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_UNKOWN;
	}else{
		p_call_monitor->glb_conf.fixed_time_type = FIXED_TIME_TYPE_UNKOWN;
	}


	if(get_line_value(&call_monitor_conf, "general", "fixed_time_calltime",value) == 0){
		struct tm fixed_time;
		time_t now;
		time_t t;
		time(&now);
		strptime(value, "%Y-%m-%d %H:%M:%S", &fixed_time);
		t = mktime(&fixed_time);
		if( (t+3600 * 2)  >= now){//date if less than 1 days, will use
			memcpy(&p_call_monitor->glb_conf.fixed_time_calltime, &fixed_time, sizeof(struct tm));
			printf("fixed_time_calltime=%s\n", value);
		}
		//printf("fixed_time_calltime=%s\n", value);
	}else{
		memset(&p_call_monitor->glb_conf.fixed_time_calltime, 0, sizeof(p_call_monitor->glb_conf.fixed_time_calltime));
	}
	//if reload
	if(reload_flag == 0){
		if(get_line_value(&call_monitor_status, "general", "next_fixed_time",value) == 0){
			strptime(value, "%Y-%m-%d %H:%M:%S", &p_call_monitor->glb_conf.fixed_time_calltime);
			//printf("next_fixed_time=%s\n", value);
		}
	}
	
	for( i = 0; i < p_call_monitor->total_chan; i++){
		if(reload_flag == 0){
			p_call_monitor->chans[i].call_flag = 0;
			p_call_monitor->chans[i].id = i+1;
			p_call_monitor->chans[i].equeue = queue_create(QUEUESIZE);
		}

		sprintf(context, "%d", i+1);

		if(get_line_value(&call_monitor_conf, context, "call_dur", value) == 0)
			p_call_monitor->chans[i].conf.total_call_dur = atoi(value);
		else
			p_call_monitor->chans[i].conf.total_call_dur = 0;

		if(get_line_value(&call_monitor_conf, context, "call_times", value) == 0)
			p_call_monitor->chans[i].conf.total_call_times = atoi(value);
		else 
			p_call_monitor->chans[i].conf.total_call_times = 0;

		if(get_line_value(&call_monitor_conf, context, "call_answers", value) == 0)
			p_call_monitor->chans[i].conf.total_call_answers = atoi(value);
		else
			p_call_monitor->chans[i].conf.total_call_answers = 0;

		if(get_line_value(&call_monitor_conf, context, "online_time",value) == 0){
			p_call_monitor->chans[i].conf.online_time = atoi(value) * 60;//minitues convert to seconds
		}else{
			p_call_monitor->chans[i].conf.online_time = 0;
		}
		
		if(get_line_value(&call_monitor_conf, context, "handle_type", value) == 0){
			if(strstr(value, "call_internal"))
				p_call_monitor->chans[i].conf.handle_type |= HANDLE_CALL_INTERNAL;
			if(strstr(value, "call_external"))
				p_call_monitor->chans[i].conf.handle_type |= HANDLE_CALL_EXTERNAL;
			if(strstr(value, "sms_internal"))
				p_call_monitor->chans[i].conf.handle_type |= HANDLE_SMS_INTERNAL;
			if(strstr(value, "sms_external"))
				p_call_monitor->chans[i].conf.handle_type |= HANDLE_SMS_EXTENAL;
			if(strstr(value, "internet"))
				p_call_monitor->chans[i].conf.handle_type |= HANDLE_INTERNET;
		}else{
			p_call_monitor->chans[i].conf.handle_type = 0;
		}
		
		if(reload_flag == 0){
			p_call_monitor->chans[i].data.cur_call_dur = 0;
			p_call_monitor->chans[i].data.cur_call_times = 0;
			p_call_monitor->chans[i].data.cur_call_answers = 0;
			memset(&p_call_monitor->chans[i].data.call_begin, 0, sizeof(struct timeval));
			memset(&p_call_monitor->chans[i].data.call_end, 0, sizeof(struct timeval));
			p_call_monitor->chans[i].data.call_sta = CALL_UNKNOW;
			p_call_monitor->chans[i].data.last_sta = CALL_UNKNOW;
			p_call_monitor->chans[i].data.last_online_time.tv_sec = 0;
			p_call_monitor->chans[i].data.last_online_time.tv_nsec = 0;
			if(get_line_value(&call_monitor_status, context, "cur_dur", value) == 0)
				p_call_monitor->chans[i].data.cur_call_dur = atoi(value);
			if(get_line_value(&call_monitor_status, context, "cur_answers", value) == 0)
				p_call_monitor->chans[i].data.cur_call_answers= atoi(value);
			if(get_line_value(&call_monitor_status, context, "cur_times", value) == 0)
				p_call_monitor->chans[i].data.cur_call_times = atoi(value);
			p_call_monitor->chans[i].data.sim_state = get_chan_sim_state(i+1);
		}
	}

	config_deinit(&call_monitor_conf);
	config_deinit(&call_monitor_status);
	
	return 0;
}

static int call_monitor_deinit(struct call_monitor_s *p_call_monitor)
{
	int i = 0;
	
	for(i = 0; i < p_call_monitor->total_chan; i++){
		if(p_call_monitor->chans[i].equeue)
			queue_destroy(p_call_monitor->chans[i].equeue);
	}
	
	if(p_call_monitor->chans)
		free(p_call_monitor->chans);
	p_call_monitor->chans = NULL;
	return 0;
}

static int set_last_sta(int channel_id,call_status_t call_sta)
{
	if(channel_id < 0){
		LOG_PRINT(LOG_ERROR, "channel_id is out of range, chan_id = %d\n", channel_id);
		return -1;
	}
	if(channel_id > g_call_monitor.total_chan){
		LOG_PRINT(LOG_ERROR, "channel_id is out of range, chan_id = %d\n", channel_id);
		return -1;
	}
	//g_call_monitor.chans[channel_id - 1].data.last_sta = call_sta;
	
	return 0;
}

int put_call_limit_event(int channel_id, call_status_t call_sta)
{
	int res;
	queue_item_t item;
	chan_info_t *p_chan = &g_call_monitor.chans[channel_id];
	if(channel_id < 0){
		LOG_PRINT(LOG_ERROR, "channel_id is out of range, chan_id = %d\n", channel_id);
		return -1;
	}
	if(channel_id > g_call_monitor.total_chan){
		LOG_PRINT(LOG_ERROR,"channel_id is out of range, chan_id = %d\n", channel_id);
		return -1;
	}

	if(!p_chan->equeue) {
		LOG_PRINT(LOG_INFO, "equeue is null\n");
		return -1;
	}

	item.event = (int)call_sta;
	pthread_mutex_lock(&p_chan->lock);
	res = queue_put((queue_t *)p_chan->equeue, item);
	pthread_mutex_unlock(&p_chan->lock);
	return 0;
}

int get_call_limit_event(chan_info_t *p_chan)
{
	queue_item_t *pitem = NULL;

	if(!p_chan) {
		LOG_PRINT(LOG_ERROR,"p_chan is null\n");
		return -1;
	}

	if(!p_chan->equeue) {
		LOG_PRINT(LOG_INFO,"equeue input error\n");
		return -1;
	}

	pitem = queue_get((queue_t *)p_chan->equeue);
	if(!pitem) {
		return -1;
	}

	pthread_mutex_lock(&p_chan->lock);
	p_chan->data.call_sta = (call_status_t)pitem->event;
	pthread_mutex_unlock(&p_chan->lock);
	return pitem->event;
}

static void set_call_begin_time(struct chan_info_s *p_chan)
{
	gettimeofday(&p_chan->data.call_begin, NULL);	
}

static void set_call_end_time(struct chan_info_s *p_chan)
{
	gettimeofday(&p_chan->data.call_end, NULL);
}

static int get_call_duration_time(struct chan_info_s *p_chan)
{
	return p_chan->data.call_begin.tv_sec - p_chan->data.call_end.tv_sec;
}

void *call_statistics_handler(void *data)
{
	int i = 0;
	struct call_monitor_s *p_call_monitor = (struct call_monitor_s *)data;
	struct chan_info_s *p_chan = NULL;
	int call_time = 0;
	while(1){
		for( i = 0; i < p_call_monitor->total_chan; i++){
			p_chan = &p_call_monitor->chans[i];	
			if(get_call_limit_event(p_chan) < 0)
				continue;
			
			if(p_chan->data.call_sta == SIM_OUT){
				p_chan->data.cur_call_times = 0;
				p_chan->data.cur_call_answers = 0;
				p_chan->data.cur_call_dur = 0;
				p_chan->data.sim_state = SIM_STATE_REMOVE;
				LOG_PRINT(LOG_INFO,"[%d] sim removed\n", i+1);
			}else if(p_chan->data.call_sta == SIM_UP){
				p_chan->data.sim_state = SIM_STATE_READY;
				LOG_PRINT(LOG_INFO,"[%d] sim ready\n", i+1);
			}else if(p_chan->data.call_sta == SIM_IN){
				p_chan->data.sim_state = SIM_STATE_INSERT;
				LOG_PRINT(LOG_INFO,"[%d] sim insert\n", i+1);
			}
			
			if(p_chan->call_flag == 1)
				continue;
			
			if(p_chan->data.call_sta >= CALL_IN)
				continue;

			if(p_chan->data.call_sta == CALL_DAIL){//outboud call
				if(p_chan->conf.total_call_times > 0)
					p_chan->data.cur_call_times++;
				p_chan->data.sim_state = SIM_STATE_CALL;
				LOG_PRINT(LOG_INFO,"[%d] sim call\n", i+1);
			}else if((CALL_ANSWER == p_chan->data.call_sta) && (p_chan->data.last_sta == CALL_DAIL || p_chan->data.last_sta == CALL_RING) ){
				if(p_chan->conf.total_call_answers>0){
					p_chan->data.cur_call_answers++;//outboud answers
					set_call_begin_time(p_chan);//update call process begin time
				}
			}else if(p_chan->data.call_sta == CALL_HANGUP && p_chan->data.last_sta == CALL_ANSWER){
				if(p_chan->conf.total_call_dur > 0){
					set_call_end_time(p_chan);//update call process end time 
					call_time = get_call_duration_time(p_chan);//calc call process duration time
					p_chan->data.cur_call_dur += call_time/60 + (call_time % 60)?1:0;//seconds => minitues
					call_time = 0;
				}
				p_chan->data.sim_state = SIM_STATE_READY;
				LOG_PRINT(LOG_INFO,"[%d] sim ready\n", i+1);
			}

			p_chan->data.last_sta = p_chan->data.call_sta;
			LOG_PRINT(LOG_INFO,"cur_call_times = %d\n"
					"cur_call_answers=%d\n"
					"cur_duration_time=%d\n",
					p_chan->data.cur_call_times,
					p_chan->data.cur_call_answers,
					p_chan->data.cur_call_dur);

		}
		usleep(200000);//200ms
	}
	return (void *)NULL;
}

 int call_monitor_create_thread(pthread_t *thred_id, void *data, void *(func)(void *data))
{
	int res;
	pthread_attr_t attr;
	//pthread_t thread_id;
	const int stack_size = 256 * 1024; /* 256 K */

	/* Initialize thread creation attributes */
	res = pthread_attr_init(&attr);
	if (0 != res)
	{
		LOG_PRINT(LOG_INFO,"Init thread attr failed, %s\n", strerror(errno));
		return -1;
	}

	res = pthread_attr_setstacksize(&attr, stack_size);
	if (0 != res)
	{
		LOG_PRINT(LOG_INFO, "Init thread stack size failed, %s\n", strerror(errno));
		return -2;
	}

	res = pthread_create(thred_id, &attr, func, data);
	if (0 != res)
	{
		LOG_PRINT( LOG_INFO,"create thread failed, %s\n", strerror(errno));
		return -3;
	}

	return 0;
}


static void get_local_time(struct tm *now_time)
{
	struct tm *ptr = NULL;
	time_t lt;
	lt = time(NULL);
	ptr = localtime(&lt);
	memcpy(now_time, ptr, sizeof(struct tm));
}

static int fixed_time_is_timeout(struct tm* ptm, int type)
{
	int res = 0;
	int add = 0;
	time_t now;
	time_t t;
	time_t set;

	time(&now);

	t = mktime(ptm);

	if(t == -1) {
		return 0;
	}
	set = t + add;
	while(now >= set) {
		res = 1;
		int curr_year = 0;
		int curr_month = 0;
		int is_leap_year = 0;   /* 是否是闰年 */
		int day_of_month = 0;   /* 某个月的天数 */
		curr_year = ptm->tm_year + 1900;   /* tm_year: 年份,等于当前年份减去1900 */
		curr_month = ptm->tm_mon + 1;      /*  tm_mon: 月份,0表示1月 */
		if (( (curr_year % 4 == 0) && (curr_year % 100 != 0)) || (curr_year % 400 == 0)) {
			is_leap_year = 1;
		}
		switch(curr_month) {
			case 1:  day_of_month = 31; break;
			case 2:  day_of_month = is_leap_year ? 29 : 28; break;
			case 3:  day_of_month = 31; break;
			case 4:  day_of_month = 30; break;
			case 5:  day_of_month = 31; break;
			case 6:  day_of_month = 30; break;
			case 7:  day_of_month = 31; break;
			case 8:  day_of_month = 31; break;
			case 9:  day_of_month = 30; break;
			case 10: day_of_month = 31; break;
			case 11: day_of_month = 30; break;
			case 12: day_of_month = 31; break;
		}
		switch(type) {
			case FIXED_TIME_TYPE_DAYS:   add = 3600 * 24; break;
			case FIXED_TIME_TYPE_2DAYS: add = 3600 * 24 * 2; break;
			case FIXED_TIME_TYPE_3DAYS: add = 3600 * 24 * 3; break;
			case FIXED_TIME_TYPE_4DAYS: add = 3600 * 24 * 4; break;
			case FIXED_TIME_TYPE_5DAYS: add = 3600 * 24 * 5; break;
			case FIXED_TIME_TYPE_WEEK:  add = 3600 * 24 * 7; break;
			case FIXED_TIME_TYPE_MONTH: add = 3600 * 24 * day_of_month; break;
			default: printf("wrong type\n");return 0;
		}
		set += add;
		memcpy(ptm, localtime(&set), sizeof(struct tm));
	}
	return res;
}


static void * call_monitor_run(void *data)
{
	int next_day_flag = 0;//用于判断是不是有跨0点的皮遏制
	struct tm now_time;
	int time_star;
	struct call_monitor_s *p_call_monitor = (struct call_monitor_s *)data;
	struct tm *start_time = &p_call_monitor->glb_conf.start_time;
	struct tm *end_time = &p_call_monitor->glb_conf.end_time;
	int start_time_s;//开始时间转换成秒
	int end_time_s;//结束时间转换成秒
	int now_time_s;//当前时间转换成秒

	start_time_s = start_time->tm_hour * 3600 + start_time->tm_min * 60 + start_time->tm_sec;
	end_time_s = end_time->tm_hour * 3600 + start_time->tm_min * 60 + end_time->tm_sec;

	if(start_time_s >= end_time_s){
		//eg: start_time=00:00:00, end_time=00:00:00 or start_time=23:00:00, end_time=01:00:00
		next_day_flag = 1;
	} 
	
	while(1){
		
		get_local_time(&now_time);
		now_time_s = now_time.tm_hour * 3600 + now_time.tm_min * 60 + now_time.tm_sec;
		
		if(next_day_flag == 1){
			if(now_time_s < end_time_s || now_time_s > end_time_s ){
				// run autocall
				check_calldata(p_call_monitor);
			}else{
				p_call_monitor->trigger_flag = 0;
			}
		}else{
			if(now_time_s > start_time_s && now_time_s < end_time_s){
				//run auto call
				check_calldata(p_call_monitor);
			}else{
				p_call_monitor->trigger_flag = 0;
			}
		}
		if(p_call_monitor->glb_conf.fixed_time_switch == 1 
			&& fixed_time_is_timeout(&p_call_monitor->glb_conf.fixed_time_calltime,p_call_monitor->glb_conf.fixed_time_type)){
			//run autocall
			check_fixed_time(p_call_monitor);
			flush_status_to_file();
		}
		sleep(10);
	}
}
/*************************GSOAP INTERFACE BEGIN *************************/
int config_reload()
{
	call_monitor_init(&g_call_monitor, 1);
	call_monitor_check_config(&g_call_monitor);
	return 0;
}

int get_chan_conf(int chan_id, struct chan_config *config)
{
	if(chan_id <= 0)
		return -1;
	if(chan_id > g_call_monitor.total_chan)
		return -1;
	if(config){
		config->total_call_dur = g_call_monitor.chans[chan_id-1].conf.total_call_dur;
		config->total_call_answers = g_call_monitor.chans[chan_id-1].conf.total_call_answers;
		config->total_call_times = g_call_monitor.chans[chan_id-1].conf.total_call_times;
		config->handle_type = g_call_monitor.chans[chan_id-1].conf.handle_type;
		config->online_time = g_call_monitor.chans[chan_id-1].conf.online_time/60;
	}
	return 0;
}

int get_chan_data(int chan_id, struct chan_data *data)
{
	if(chan_id <= 0)
		return -1;
	if(chan_id > g_call_monitor.total_chan)
		return -1;
	if(data){
		data->cur_call_dur = g_call_monitor.chans[chan_id-1].data.cur_call_dur;
		data->cur_call_answers = g_call_monitor.chans[chan_id-1].data.cur_call_answers;
		data->cur_call_times = g_call_monitor.chans[chan_id-1].data.cur_call_times;
		data->last_online_time = g_call_monitor.chans[chan_id-1].data.last_online_time.tv_sec;

	}
	return 0;
}

int flush_status_to_file(){
	int i = 0;
	int total_chan = g_call_monitor.total_chan;
	char buf[128] = {0};
	struct chan_info_s *p_chan = NULL;
	FILE *handle = fopen(CALL_MONITOR_STATUS, "wa+");
	if(handle == NULL){
		printf("open %s fail\n", CALL_MONITOR_CONF);
		return -1;
	}
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &g_call_monitor.glb_conf.fixed_time_calltime);
	fprintf(handle, "[general]\n");
	fprintf(handle, "next_fixed_time=%s\n", buf);
	for(i = 0; i < total_chan;i++){
		p_chan = &g_call_monitor.chans[i];
		fprintf(handle, "[%d]\n", i+1);
		fprintf(handle, "cur_times=%d\n", p_chan->data.cur_call_times);
		fprintf(handle, "cur_answers=%d\n", p_chan->data.cur_call_answers);
		fprintf(handle, "cur_dur=%d\n", p_chan->data.cur_call_dur);
	}
	fclose(handle);
	return 0;
}
/*************************GSOAP INTERFACE END *************************/
int main(int argc, char **argv)
{
	log_init(LOG_INFO);
	pthread_t call_monitor_id, event_id, run_id;
	sys_info_init(&g_call_monitor);
	call_monitor_init(&g_call_monitor, 0);

	call_monitor_check_config(&g_call_monitor);

	call_monitor_create_thread(&call_monitor_id, &g_call_monitor, call_statistics_handler);
	call_monitor_create_thread(&event_id, &g_call_monitor, check_callevent_thread_cb_handler);
	call_monitor_create_thread(&run_id, &g_call_monitor, call_monitor_run);
	gsoap_thread_create();
	//create 
	while(1){
		sleep(200);	
	}

	call_monitor_deinit(&g_call_monitor);
	return 0;
}
