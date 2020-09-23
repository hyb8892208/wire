#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>

#include "redis_interface.h"
#include "callmonitor.h"
#include "callmonitor_log.h"
//#include <hiredis.h>
#define MAX_CHN 44
//#define MAKE_CALL_CMD "asterisk -rx \"gsm send sync at %d ATD%s; 2000\""

#define MAKE_CALL_CMD "asterisk -rx \"channel originate extra/%d/%s extension 400@auto-call\""

typedef struct call_info_s{
	int chan;
	char phonenumber[32];
}call_info_t;

//struct call_info *call_list;

//产生一个范围内的随机数
static int get_range_srandnum(int max, int min)
{
	int num = 0;
	struct timeval tv;
	gettimeofday(&tv,NULL);	
	if(max == min){
		num = max;
	}else{
		num = tv.tv_usec % (max - min) + min;
	}
	return num;	
}

static int get_sys_runtime()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return 0;
}

//calc inside channels count .
static int get_inside_calls_count(struct call_monitor_s *p_call_monitor, struct call_info_s *call_list)
{
	int count = 0;
	int total_chan = p_call_monitor->total_chan;
	int i = 0;
	struct chan_info_s *p_chan = NULL;
	memset(call_list, 0, sizeof(call_list));
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	
	for(i = 0; i < total_chan; i++){
		p_chan = &p_call_monitor->chans[i];
		if(!(p_chan->conf.handle_type & HANDLE_CALL_INTERNAL))
			continue;
	//	if(p_chan->data.call_sta == CALL_ANSWER || p_chan->data.call_sta == CALL_RING || p_chan->data.call_sta == CALL_DAIL)//there status, calls fialure
		if(p_chan->data.sim_state != SIM_STATE_READY)
			continue;
		p_chan->trigger_type = TRIGGER_TYPE_UNKOWN;
		//check device run time.
		if(p_chan->conf.online_time > 0 && now.tv_sec - p_chan->data.last_online_time.tv_sec >  p_chan->conf.online_time ){
			p_chan->trigger_type |= TRIGGER_TYPE_ONLINE_TIME;
			LOG_PRINT(LOG_INFO,"online time\n");
		}
		if (p_chan->conf.total_call_dur > 0 && p_chan->conf.total_call_dur <= p_chan->data.cur_call_dur){
			p_chan->trigger_type |= TRIGGER_TYPE_CALL_DUR;
			LOG_PRINT(LOG_INFO,"call dur\n");
		}
		if(p_chan->conf.total_call_times > 0 && p_chan->conf.total_call_times <= p_chan->data.cur_call_times){
			p_chan->trigger_type |= TRIGGER_TYPE_CALL_TIMES;
			LOG_PRINT(LOG_INFO, "call times\n");
		}
		if(p_chan->conf.total_call_answers > 0 && p_chan->conf.total_call_answers <= p_chan->data.cur_call_answers){
			p_chan->trigger_type |= TRIGGER_TYPE_CALL_ANSWERS;
			LOG_PRINT(LOG_INFO,"call answers");
		}
		if(p_chan->trigger_type == TRIGGER_TYPE_UNKOWN)
			continue;

		if(0 != get_phonenumber(p_chan->id, call_list[count].phonenumber) )
			continue;

		p_chan->call_flag = 1;
		call_list[count++].chan = p_chan->id;
	}
	return count;
}


static get_fixed_time_call_count(struct call_monitor_s *p_call_monitor, struct call_info_s *p_call_list)
{	
	int count = 0;
	int total_chan = p_call_monitor->total_chan;
	int i = 0;
	struct chan_info_s *p_chan = NULL;
	memset(p_call_list, 0, sizeof(p_call_list));
	for(i = 0; i < total_chan; i++){
		p_chan = &p_call_monitor->chans[i];
		if(!(p_chan->conf.handle_type & HANDLE_CALL_INTERNAL))
			continue;
		if(p_chan->data.sim_state != SIM_STATE_READY){
			LOG_PRINT(LOG_INFO, "[%d] is not ready\n", i+1);
			continue;
		}
		if(0 != get_phonenumber(p_chan->id, p_call_list[count].phonenumber) )
			continue;
		
		p_chan->call_flag = 1;
		p_call_list[count++].chan = p_chan->id;
		
	}
	return count;
}

//make call by asterisk channel originate command
static void channel_make_call(struct call_info_s *caller, struct call_info_s *callee)
{
	char call_cmd[128] = {0};
	//extra/1/[phonember], extra/3/[phonenumber],extra/5/[phonenumber]...
	sprintf(call_cmd, MAKE_CALL_CMD, caller->chan * 2 - 1, callee->phonenumber);
	LOG_PRINT(LOG_INFO, "%s\n", call_cmd);
	system(call_cmd);
}

static void set_call_duration(int max, int min)
{
	char buf[32] = {0};
	int duration = get_range_srandnum(max, min);
	sprintf(buf, "%d", duration);
	FILE * handle = fopen("/tmp/answer_time.conf", "w+");
	if(handle == NULL)
		return;
	fwrite(buf, 1, strlen(buf), handle);
	fclose(handle);	
}

static void unset_call_duration(){
	const char *filename = "/tmp/answer_time.conf";
	if(access(filename, F_OK) == 0){
		unlink(filename);	
	}
}

static void set_auto_answer_flag(int chan)
{
	char filename[64] = {0};
	sprintf(filename, "/tmp/answer_flag_%d.conf", chan);
	FILE * handle = fopen(filename, "w+");
	if(handle == NULL)
		return;
	fwrite("yes",1, strlen("yes"), handle);
	fclose(handle);
}

static void unset_auto_answer_flag(int chan){
	char filename[64] = {0};
	sprintf(filename, "/tmp/answer_flag_%d.conf", chan);
	if(access(filename, F_OK) == 0)
		unlink(filename);
	
}

static void srand_call_chan_order(struct call_info_s *p_call_list, int count)
{
	int i = 0;
	int index = 0;
	int half_count = count/2;
	struct call_info_s tmp;
	for(i = 0; i < half_count; i++){
		index = get_range_srandnum(count, half_count);
		tmp = p_call_list[i];
		p_call_list[i] = p_call_list[index];
		p_call_list[index] = tmp;
	}
}

static void make_calls(struct call_info_s *p_call_list, struct call_monitor_s *p_call_monitor, int chan_count)
{
	int half_count = 0;
	int i = 0;
	struct chan_info_s *p_chan = NULL;
	struct call_info_s *callee = NULL, *caller = NULL;
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	
	half_count = chan_count/2;
	set_call_duration(p_call_monitor->glb_conf.max_time, p_call_monitor->glb_conf.min_time);
	//srand channel order
	srand_call_chan_order(p_call_list, chan_count);

	//first time, The first half of the port calls the second half
	for(i = 0; i < half_count;i++){
		LOG_PRINT(LOG_INFO, "first call\n");
		caller = &p_call_list[i];
		callee = &p_call_list[i+half_count];
		set_auto_answer_flag(callee->chan);
		channel_make_call(caller, callee);	
	}
	//
	sleep(p_call_monitor->glb_conf.max_time + 30);//wait for calls end
	//The second half of the port calls the first half
	for(i = 0; i < half_count; i++){
		LOG_PRINT(LOG_INFO, "second call\n");
		callee = &p_call_list[i];
		caller = &p_call_list[i+half_count];
		set_auto_answer_flag(callee->chan);
		channel_make_call(caller, callee);	
	}
	//wait for calls end 
	sleep(p_call_monitor->glb_conf.max_time + 15);//wait for calls end
	if(chan_count % 2){//maybe port is 
		LOG_PRINT(LOG_INFO, "third call\n");
		callee = &p_call_list[0];
		caller = &p_call_list[chan_count - 1];
		set_auto_answer_flag(callee->chan);
		channel_make_call(caller, callee);
		sleep(p_call_monitor->glb_conf.max_time + 60);//wait calls end.
	}

	for(i = 0; i < chan_count; i++){
		p_chan = &p_call_monitor->chans[p_call_list[i].chan - 1];
		
		if(p_chan->trigger_type & TRIGGER_TYPE_CALL_TIMES || p_chan->trigger_type & TRIGGER_TYPE_CALL_ANSWERS || p_chan->trigger_type & TRIGGER_TYPE_CALL_DUR){
			p_call_monitor->chans[(p_call_list[i].chan-1)].data.cur_call_dur = 0;
			p_call_monitor->chans[(p_call_list[i].chan-1)].data.cur_call_times = 0;
			p_call_monitor->chans[(p_call_list[i].chan-1)].data.cur_call_answers = 0;

		}
		if(p_chan->trigger_type & TRIGGER_TYPE_ONLINE_TIME){
			//udate last call time
			p_chan->data.last_online_time.tv_sec = (now.tv_sec/p_chan->conf.online_time ) * p_chan->conf.online_time;
		}
		p_chan->call_flag = 0;
		//delete answer flag file
		unset_auto_answer_flag(p_call_list[i].chan);
	}
	unset_call_duration();
}

//创建端口呼叫和局外呼叫的时长
void check_calldata(void *data)
{
	int chan_count = 0;//所有需要发起呼叫的通道总数

	struct call_monitor_s *p_call_monitor = (struct call_monitor_s *)data;
	struct call_info_s *call_list = NULL;
	
	conn_redis();
	LOG_PRINT(LOG_INFO, "check calldata\n");
	call_list = (struct call_info_s *)malloc(sizeof(struct call_info_s) * p_call_monitor->total_chan);

	chan_count = get_inside_calls_count(p_call_monitor, call_list);
	if(chan_count <= 1){//通道数小于1，不能发起内部端口互打
		LOG_PRINT(LOG_INFO, "chan_count=%d\n", chan_count);
		free(call_list);
		redis_disconnect();
		return ;
	}

	make_calls(call_list, p_call_monitor, chan_count);

	free(call_list);
	call_list = NULL;


	redis_disconnect();
	
}


void check_fixed_time(void *data)
{
	int chan_count = 0;//所有需要发起呼叫的通道总数

	struct call_monitor_s *p_call_monitor = (struct call_monitor_s *)data;
	struct call_info_s *call_list = NULL;
	
	conn_redis();
	LOG_PRINT(LOG_INFO,"check fixed time\n");
	call_list = (struct call_info_s *)malloc(sizeof(struct call_info_s) * p_call_monitor->total_chan);

	chan_count = get_fixed_time_call_count(p_call_monitor, call_list);
	if(chan_count <= 1){//通道数小于1，不能发起内部端口互打
		LOG_PRINT(LOG_INFO,"chan_count=%d\n", chan_count);
		free(call_list);
		redis_disconnect();
		return ;
	}

	make_calls(call_list, p_call_monitor, chan_count);

	free(call_list);
	call_list = NULL;

	redis_disconnect();

}

