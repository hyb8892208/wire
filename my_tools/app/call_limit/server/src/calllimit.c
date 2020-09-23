
#include "../include/header.h"
#include "../include/calllimit.h"
#include "../include/callevent.h"
#include "../include/gsoap_server.h"
#include "../include/calllimit_cfg.h"
#include "../include/queue.h"
#include "hiredis.h"
#include "bsp_api.h"
#include "../include/calllimit_log.h"


#define CALL_LOCK_DEBUG 0
#define __LOCK_FILE__		"/var/lock/call_limit.lock"
#define ALL_CHANNEL 0xFFFF

calllimit_t g_call_limit;

static int is_valid_channel_id(int channel_id){
	if( (channel_id  == ALL_CHANNEL) || (channel_id < MAX_CHAN ) & (channel_id >= 0)) {
		return 0;
	}
	return 1;
}

static int set_last_sta(running_chan_status_t *p_status,call_status_t call_sta)
{
    if(p_status->last_sta != CALL_UNKNOW) {
        p_status->last_sta = p_status->call_sta;
    }
    if(call_sta == SMS_FAILED || call_sta==SMS_SUCCESS)
        p_status->last_sta = p_status->call_sta;
    return 0;
}

int compare_destination(int channel_id,char *addr)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

    if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]input fail\n", channel_id);
        return -1;
    }

    p_chan += channel_id;
    return strcmp(addr,p_chan->cfg.calllock_cfg.call_fail_lock_sms_callee);
}

int calllimit_pthread_create_stack(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *data)
{
	if (!attr) {
		attr = (pthread_attr_t *)malloc(sizeof(*attr));
		pthread_attr_init(attr);
	}

	pthread_attr_setstacksize(attr, STACKSIZE);
	return pthread_create(thread, attr, start_routine, data); /* We're in ast_pthread_create, so it's okay */
}


int calllimit_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *data)
{
	unsigned char attr_destroy = 0;
	int res;

	if (!attr) {
		attr = (pthread_attr_t *)malloc(sizeof(*attr));
		pthread_attr_init(attr);
		attr_destroy = 1;
	}

	pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
	res = calllimit_pthread_create_stack(thread, attr, start_routine, data);

	if (attr_destroy)
		pthread_attr_destroy(attr);

	return res;
}

int set_sms_sta(running_chan_status_t *p_status, call_status_t call_sta){
	if(call_sta == SMS_SUCCESS || call_sta == SMS_SEND_FAILED || call_sta == SMS_FAILED){
		p_status->sms_sta = call_sta;
	}
	return 0;
}
int parsing_call_status(int channel_id, call_status_t call_sta)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]input fail\n", channel_id);
		return -1;
	}

	p_chan += channel_id;
	
	pthread_mutex_lock(&p_chan->lock);	
    set_last_sta(&p_chan->status,call_sta);
	p_chan->status.call_sta = call_sta;
	set_sms_sta(&p_chan->status, call_sta);
	pthread_mutex_unlock(&p_chan->lock);	

//	print_debug("[%d]call: %s\n", channel_id, call_status_to_str(call_sta));
	return 0;
}

int put_call_limit_event(int channel_id, call_status_t call_sta)
{
	queue_item_t item;
	calllimit_chan_t *p_chan = g_call_limit.chans;
	int res = 0;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]input fail\n", channel_id);
		return -1;
	}
	p_chan += channel_id;

	if(!p_chan->equeue) {
		log_printf(LOG_ERROR, "equeue input error.\n");
		return -1;		
	}

	item.event = (int)call_sta;
	pthread_mutex_lock(&p_chan->lock);		
	res = queue_put((queue_t *)p_chan->equeue, item);
	pthread_mutex_unlock(&p_chan->lock);	
//	print_debug("[%d]put calllimit event: %s\n", channel_id+1, call_status_to_str(call_sta));	
	return res;
}


int get_call_limit_event(calllimit_chan_t *p_chan)
{
	queue_item_t *pitem = NULL;

	if(!p_chan) {
		log_printf(LOG_ERROR, "p_call_limit is null\n");
		return -1;
	}

	if(!p_chan->equeue) {
		log_printf(LOG_ERROR, "equeue input error.\n");
		return -1;		
	}

	pitem = queue_get((queue_t *)p_chan->equeue);
	if(!pitem) {
		return -1;
	}
	
	pthread_mutex_lock(&p_chan->lock);		
	set_last_sta(&p_chan->status, (call_status_t)pitem->event);
	p_chan->status.call_sta = (call_status_t)pitem->event;
	set_sms_sta(&p_chan->status, (call_status_t)pitem->event);
	pthread_mutex_unlock(&p_chan->lock);		
	log_printf(LOG_DEBUG, "[%d]get calllimit event: %s\n", p_chan->id, call_status_to_str((call_status_t)pitem->event)); 
	return pitem->event;
}


static int get_local_date(struct tm *out_date)
{
	struct tm *ptr = NULL;
	time_t lt;
	
	lt =time(NULL);
	ptr=localtime(&lt);
	memcpy((char *)out_date, (char *)ptr, sizeof(struct tm));

	return 0;
}

static void get_current_datetime(calllimit_chan_t *p_call_limit)
{
	get_local_date(&p_call_limit->cur_date);
	gettimeofday(&p_call_limit->cur_time, NULL);
	return;
}



static void calllock_cfg_init(calllock_conf_t *calllock_cfg)
{
	calllock_cfg->call_fail_mark_count = 0;
	calllock_cfg->call_fail_lock_count = 0;
	calllock_cfg->call_fail_lock_sms_count = 0;
	calllock_cfg->call_detect_flag = FLAG_NO;
	calllock_cfg->call_fail_mark_flag = FLAG_NO;
	calllock_cfg->call_fail_lock_flag = FLAG_NO;
	calllock_cfg->call_fail_lock_sms_report_flag = FLAG_NO;
	calllock_cfg->call_fail_lock_sms_flag = FLAG_NO;
	memset(calllock_cfg->call_fail_lock_sms_msg, 0, sizeof(calllock_cfg->call_fail_lock_sms_msg));
	memset(calllock_cfg->call_fail_lock_sms_callee, 0, sizeof(calllock_cfg->call_fail_lock_sms_callee));
 	return;
}

static void calllimit_cfg_init(calllimit_conf_t *calllimit_cfg)
{
	calllimit_cfg->calllimit_switch = DISABLE;
	calllimit_cfg->day_calls_settings = 0;
	calllimit_cfg->day_answer_setting = 0;
	calllimit_cfg->hour_calls_settings = 0;
	return;
}

static void calltime_cfg_init(calltime_conf_t *calltime_cfg)
{
	calltime_cfg->call_time_total = 0;
	calltime_cfg->call_time_free = 0;
	calltime_cfg->call_time_step = 0;
	calltime_cfg->call_time_warning_num = 0;
	calltime_cfg->call_time_settings = 0;
	calltime_cfg->call_time_sw = DISABLE;
	calltime_cfg->call_time_single_sw = DISABLE;
	calltime_cfg->call_time_total_sw = DISABLE;
	calltime_cfg->call_time_clean_sw = DISABLE;
	calltime_cfg->call_time_clean_type = CALL_TIME_CLEAN_TYPE_UNKOWN;
	memset(calltime_cfg->call_time_warning_msg, 0, sizeof(calltime_cfg->call_time_warning_msg));
	memset(calltime_cfg->call_time_warning_callee, 0, sizeof(calltime_cfg->call_time_warning_callee));
	return;
}

static void smslimit_cfg_init(smslimit_conf_t *smslimit_cfg)
{
	smslimit_cfg->day_sms_settings = 0;
	smslimit_cfg->mon_sms_settings = 0;
	smslimit_cfg->smslimit_mon_warning_num = 0;
	smslimit_cfg->smslimit_switch = DISABLE;
	smslimit_cfg->smslimit_success_flag = FLAG_NO;
	smslimit_cfg->sms_clean_date = 0;
	smslimit_cfg->sms_warning_switch = DISABLE;
	memset(smslimit_cfg->smslimit_mon_warning_msg, 0, sizeof(smslimit_cfg->smslimit_mon_warning_msg));
	memset(smslimit_cfg->smslimit_mon_warning_callee, 0, sizeof(smslimit_cfg->smslimit_mon_warning_callee));
	return;
}

static void simswitch_cfg_init(simswitch_conf_t *simswitch_cfg)
{
	simswitch_cfg->sim_reg_timeout = SIM_REG_TIMEOUT;
	simswitch_cfg->sim_switch_sw = DISABLE;
	simswitch_cfg->sim_policy = SIM_ASC;
	simswitch_cfg->total_callout_count = 0;
	simswitch_cfg->total_callout_time = 0;	
	simswitch_cfg->total_using_time = 0;
	simswitch_cfg->total_sms_count = 0;
	return;
}


static void chan_config_init(chan_conf_t *cfg)
{
	calllock_cfg_init(&cfg->calllock_cfg);
	calllimit_cfg_init(&cfg->calllimit_cfg);
	calltime_cfg_init(&cfg->calltime_cfg);
	smslimit_cfg_init(&cfg->smslimit_cfg);
	simswitch_cfg_init(&cfg->simswitch_cfg);
	return;
}

static void calllock_info_init(calllock_info_t *calllock_info)
{
	calllock_info->call_failed_count = 0;
	calllock_info->call_fail_send_sms_times = 0;
	calllock_info->call_fail_mark_status = FLAG_NO;
	calllock_info->call_fail_lock_status = FLAG_NO;
	return;
}

static void calllimit_info_init(calllimit_info_t *calllimit_info)
{
	calllimit_info->hour_cur_calls = 0;
	calllimit_info->day_cur_calls = 0;
	calllimit_info->day_cur_answers = 0;
	calllimit_info->daycall_generation_flag = FLAG_NO;
	calllimit_info->hourcall_generation_flag = FLAG_NO;
	calllimit_info->call_answer_flag = FLAG_NO;
	calllimit_info->day_call_limit_flag = FLAG_NO;
	calllimit_info->day_answer_limit_flag = FLAG_NO;
	calllimit_info->hour_call_limit_flag = FLAG_NO;
	memset((char *)&calllimit_info->day_call_last_clean_date, 0, sizeof(calllimit_info->day_call_last_clean_date));
	memset((char *)&calllimit_info->day_answer_last_clean_date, 0, sizeof(calllimit_info->day_answer_last_clean_date));
	memset((char *)&calllimit_info->hour_call_last_clean_date, 0, sizeof(calllimit_info->hour_call_last_clean_date));
	return;	
}

static void calltime_info_init(calltime_info_t *calltime_info)
{
	calltime_info->call_time_count = 0;
	calltime_info->call_time_remain = 0;
	calltime_info->call_time_limit_flag = FLAG_NO;
	calltime_info->call_time_redis_flag = FLAG_NO;
	calltime_info->call_answer_flag = FLAG_NO;
	calltime_info->sms_warning_flag = FLAG_NO;
	calltime_info->handup_flag = FLAG_NO;
	calltime_info->cond_flag = FLAG_NO;
	calltime_info->call_time_warning_flag = FLAG_NO;
    pthread_mutex_init(&calltime_info->cond_lock, NULL);
    pthread_cond_init(&calltime_info->cond, NULL);
	memset((char *)&calltime_info->call_time_clean_date, 0, sizeof(calltime_info->call_time_clean_date));
	memset((char *)&calltime_info->start_call_time, 0, sizeof(calltime_info->start_call_time));
	return;
}

static void smslimit_info_init(smslimit_info_t *smslimit_info)
{
	smslimit_info->day_cur_sms = 0;
	smslimit_info->mon_cur_sms = 0;
	smslimit_info->mon_sms_limit_flag = FLAG_NO;
	smslimit_info->day_sms_limit_flag = FLAG_NO;
	memset((char *)&smslimit_info->day_last_date, 0, sizeof(smslimit_info->day_last_date));
	memset((char *)&smslimit_info->mon_last_date, 0, sizeof(smslimit_info->mon_last_date));
	return;
}

static void chan_simcard_init(simcard_info_t *sim)
{
	calllimit_info_init(&sim->calllimit_info);
	calltime_info_init(&sim->calltime_info);
	calllock_info_init(&sim->calllock_info);
	smslimit_info_init(&sim->smslimit_info);
	memset(sim->sim_ping_code, 0, sizeof(sim->sim_ping_code));
	sim->sim_slot_sta = SIM_OUT_SLOT;
	sim->sim_slot_laststa = sim->sim_slot_sta;
	sim->sim_sta = SIM_NONE;
	return;
}

static void chan_running_status_init(running_chan_status_t *status)
{
	status->call_sta = CALL_HANDUP;	
	status->last_sta = CALL_HANDUP;
	status->sys_limit_sta = UNLIMIT;
	status->sms_limit_sta = SMS_UNLIMIT;
	status->sms_sta = CALL_UNKNOW;
	status->call_time_limit_sta = UNLIMIT;
	status->sim_reg_sta = REGISTERING;
	status->call_lock_sta = UNLIMIT;
	status->call_mark_sta = UNLIMIT;
	return;
}

static void chan_simswitch_info_init(chan_simswitch_info_t *simswitch_info)
{
	simswitch_info->curr_using_time = 0;
	simswitch_info->using_time_count = 0;
	simswitch_info->curr_callout_time = 0;
	simswitch_info->curr_callout_count = 0;
	simswitch_info->curr_sms_count = 0;
	simswitch_info->call_generation_flag = FLAG_NO;
	simswitch_info->call_answer_flag = FLAG_NO;
	simswitch_info->sim_switch_flag = FLAG_NO;
	simswitch_info->reopen_switch_flag = FLAG_NO;
	simswitch_info->sim_switch_fail_flag = FLAG_NO;
	return;
}


static int update_using_time(chan_simswitch_info_t *p)
{
	struct timeval now;    
	unsigned int duration_time = 0;

	if(p->using_time_flag == FLAG_NO) {
		gettimeofday(&p->last_using_time, NULL);
		p->using_time_flag = FLAG_YES;
	}		
	
	gettimeofday(&now, NULL);
	duration_time = now.tv_sec -  p->last_using_time.tv_sec;
	if(duration_time > 0)
		p->curr_using_time += duration_time;

	gettimeofday(&p->last_using_time, NULL);		
//	print_debug("curr_using_time=%d\n", p->curr_using_time);		
}

static void clean_sim_switch_info(chan_simswitch_info_t *p)
{
	p->curr_using_time = 0;
	p->curr_callout_time = 0;
	p->curr_callout_count = 0;
	p->curr_sms_count = 0;
	p->sim_switch_flag = FLAG_NO;
	p->using_time_flag = FLAG_NO;
//	update_using_time(p);
	return;
}

static void cmd_sim_register(unsigned int chan_id, unsigned char sim_id, char *pincode){
	char *cmd;
    asprintf(&cmd,SIM_RELOAD_CMD, chan_id, sim_id, pincode?pincode:"");
    system(cmd);
    free(cmd);
    return 0;
}

static unsigned char get_new_simidx(unsigned char cur_simidx, unsigned char policy)
{
	unsigned char sim_idx = SIM1;
	
	switch(policy) {
		case SIM_ASC:
			sim_idx = (unsigned char)(cur_simidx+1)%SIM_NUM;
			break;
		case SIM_DES:
			if(cur_simidx  == SIM1)
				sim_idx = SIM4; 
			else
				sim_idx = (unsigned char)(cur_simidx-1)%SIM_NUM;
			break;
		default:
			;
	}	
	return sim_idx;
}

static sim_limit_status_t check_simcard_is_available(simcard_info_t *sim_info)
{
	if(SIM_OUT_SLOT == sim_info->sim_slot_sta) {
		return SIM_LIMIT;
	}	

	if( sim_info->calllimit_info.day_call_limit_flag == FLAG_YES || 
		 sim_info->calllimit_info.day_answer_limit_flag == FLAG_YES ||
		 sim_info->calllimit_info.hour_call_limit_flag == FLAG_YES ||
		 sim_info->calltime_info.call_time_limit_flag == FLAG_YES ||
		 sim_info->calllock_info.call_fail_lock_status == FLAG_YES ||
//		 sim_info->calllock_info.call_fail_lock_status == 2 || 
		 sim_info->smslimit_info.day_sms_limit_flag == FLAG_YES ||
		 sim_info->smslimit_info.mon_sms_limit_flag == FLAG_YES) {
		return SIM_LIMIT;
	} else {
		return SIM_UNLIMIT;
	}		
	
	return SIM_UNLIMIT;
}

static int switch_chan_simcard(calllimit_chan_t *p_chan, int force)
{
	int i = 0;
	int res = -1;
	unsigned char sim_idx = p_chan->sim_idx;
	unsigned char sys_type = g_call_limit.sys_info.sys_type;

	if(CHAN_4SIM == sys_type) {	
		for(i=0; i<SIM_NUM; i++) {
			sim_idx = get_new_simidx(sim_idx, p_chan->cfg.simswitch_cfg.sim_policy);	
			if((sim_idx != p_chan->sim_idx) || (FLAG_YES == force)) {
				if(SIM_UNLIMIT == check_simcard_is_available(&p_chan->sim[sim_idx])) {
					clean_sim_switch_info(&p_chan->simswitch);
					res = select_simcard_from_chan(p_chan->id, sim_idx+1);
					if(res) {
						log_printf(LOG_ERROR, "[chan_%d][sim_%d]can't select simcard from chan\n", p_chan->id, sim_idx+1); 	
					} else {
						log_printf(LOG_INFO, "[chan_%d][sim_%d]switch to new simcard\n", p_chan->id, sim_idx+1); 
						p_chan->sim_reg_time = p_chan->cur_time;
						p_chan->status.sim_reg_sta = REGISTERING;
						p_chan->status.call_sta = CALL_HANDUP;
						p_chan->sim_idx = sim_idx;
						p_chan->sim[sim_idx].calltime_info.call_time_redis_flag = FLAG_NO;
						p_chan->simswitch.sim_switch_fail_flag = FLAG_NO;
						cmd_sim_register(p_chan->id, sim_idx+1, p_chan->sim[sim_idx].sim_ping_code);
						calllimit_redis_update_sim_slot(p_chan->id, sim_idx+1);
						return 0;				
					}
				}
			}
		}
	}
	return 1;
}


static void cmd_call_mark(unsigned char mode, unsigned int chan_id)
{
	char cmd[128] = {0};

	sprintf(cmd,SET_CALLMARK, chan_id, mode);
	system(cmd);
}

static void cmd_call_lock(unsigned char mode, unsigned int chan_id)
{
	char cmd[128] = {0};

	sprintf(cmd,SET_CALLLOCK, chan_id, mode);
	system(cmd);
}


static int Is_chan_mark(calllimit_chan_t *p_chan){	
	if( (p_chan->sim[SIM1].calllock_info.call_fail_mark_status == FLAG_YES || p_chan->sim[SIM1].sim_slot_sta == SIM_OUT_SLOT) &&
		 (p_chan->sim[SIM2].calllock_info.call_fail_mark_status == FLAG_YES || p_chan->sim[SIM2].sim_slot_sta == SIM_OUT_SLOT) &&
		(p_chan->sim[SIM3].calllock_info.call_fail_mark_status == FLAG_YES || p_chan->sim[SIM3].sim_slot_sta == SIM_OUT_SLOT) &&
		(p_chan->sim[SIM4].calllock_info.call_fail_mark_status == FLAG_YES || p_chan->sim[SIM4].sim_slot_sta == SIM_OUT_SLOT)) {
		return 1;
	}
	return 0;
}

static int call_mark_set(calllimit_chan_t *p_chan, int force)
{
    unsigned char sim_idx = p_chan->sim_idx;
	calllock_info_t *p = &p_chan->sim[sim_idx].calllock_info;
    
    if(p->call_fail_mark_status != FLAG_YES||force == 1){
        p->call_fail_mark_status = FLAG_YES;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]set call mark.\n", p_chan->id, sim_idx+1);
		if(Is_chan_mark(p_chan)) {
			cmd_call_mark(FLAG_YES, p_chan->id);
			p_chan->status.call_mark_sta = CALLLIMIT;
		}
        return 1;
    }
    return 0;
}

static int call_lock_set(calllimit_chan_t *p_chan, int mode,int force)
{
    unsigned char sim_idx = p_chan->sim_idx;
	calllock_info_t *p = &p_chan->sim[sim_idx].calllock_info;
	int res = -1;
	
    if(p->call_fail_lock_status != FLAG_YES||force == 1)
    {
        if(mode == 2) {
			log_printf(LOG_INFO, "[chan_%d][sim_%d]set call locked.\n", p_chan->id, sim_idx+1);
            p->call_fail_lock_status = FLAG_YES;
			res = switch_chan_simcard(p_chan, FLAG_NO);
			if(res) {
				cmd_call_lock(mode, p_chan->id);
				p_chan->status.call_lock_sta = CALLLIMIT;
			}
        } else if(mode == 1) {
            p->call_fail_lock_status = 2;
        }
        return 1;
    } 
    return 0;
}

static int call_mark_clean(calllimit_chan_t *p_chan)
{
	unsigned char sim_idx = p_chan->sim_idx;
	calllock_info_t *p = &p_chan->sim[sim_idx].calllock_info;
    p->call_failed_count = 0; 
    p->call_fail_send_sms_times = 0;

    if(p->call_fail_mark_status != FLAG_NO){
        if(p->call_fail_lock_status == 2)
        {
            p->call_fail_lock_status = FLAG_NO;
			if(p_chan->status.call_lock_sta == CALLLIMIT) {
		 		p_chan->status.call_lock_sta = UNLIMIT;
		 		cmd_call_lock(FLAG_NO, p_chan->id);
		 	}
        }
        p->call_fail_mark_status = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unmark.\n", p_chan->id, sim_idx+1);		
		if(CALLLIMIT == p_chan->status.call_mark_sta) {
			cmd_call_mark(FLAG_NO, p_chan->id);
			p_chan->status.call_mark_sta = UNLIMIT;
		}
        return 1;
    }
    return 0;
}

static int call_lock_clean(calllimit_chan_t *p_chan)
{
    unsigned char sim_idx = p_chan->sim_idx;
	calllock_info_t *p = &p_chan->sim[sim_idx].calllock_info;
    p->call_failed_count = 0; 
    p->call_fail_send_sms_times = 0;
    if(p->call_fail_lock_status != FLAG_NO)
    {
         p->call_fail_lock_status = FLAG_NO;
		 log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unlock.\n", p_chan->id, sim_idx+1);
		 if(p_chan->status.call_lock_sta == CALLLIMIT) {
		 	p_chan->status.call_lock_sta = UNLIMIT;
		 	cmd_call_lock(FLAG_NO, p_chan->id);
		 }
         return 1;
    }
    return 0;
}

static int call_sms_send(calllimit_chan_t *p_chan)
{
    char *cmd_sms_test;
	calllock_conf_t *p = &p_chan->cfg.calllock_cfg;
    asprintf(&cmd_sms_test,SEND_MSG,p_chan->id,p->call_fail_lock_sms_callee,p->call_fail_lock_sms_msg);
    system(cmd_sms_test);
//	print_debug("[%d]sms_send=%s\n", p_chan->id, cmd_sms_test);
    free(cmd_sms_test);
    return 0;
}

#if 0

static int  day_sms_limit_init(day_sms_limit_s *p_day_sms){
	p_day_sms->sms_settings = 0;
	p_day_sms->day_sms_limit_flag = SMS_UNLIMIT;
	p_day_sms->cur_sms = 0;
	return 0;
}

static int mon_sms_limit_init(mon_sms_limit_s *p_mon_sms){
	p_mon_sms->sms_settings = 0;
	p_mon_sms->mon_sms_limit_flag = SMS_UNLIMIT;
	p_mon_sms->cur_sms = 0;
	p_mon_sms->clean_date = 0;
	return 0;
}

static int show_date(struct tm *in_date)
{
	struct tm *p = in_date;
	char *wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

	printf("%d??%02d??%02d??",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday);
	printf(" %s ", wday[p->tm_wday]);
	printf("%02d:%02d:%02d\n", p->tm_hour,p->tm_min, p->tm_sec);

	return 0;
}

static int show_para(int idx, call_limit_t *p_call_limit)
{
	printf("==================chan%d======================\n", idx);
	printf("[call_time_limit]\n");
	printf("call_time_settings=%d\n", p_call_limit->CallTimeLimit.call_time_settings);
	printf("handup_flag=%s\n", p_call_limit->CallTimeLimit.handup_flag?"YES":"NO");
	printf("call_answer_flag=%s\n", p_call_limit->CallTimeLimit.call_answer_flag?"YES":"NO");
	printf("start_call_time: sec=%d, usec=%d\n", p_call_limit->CallTimeLimit.start_call_time.tv_sec, p_call_limit->CallTimeLimit.start_call_time.tv_usec);	

	printf("[day_call_limit]\n");
	printf("calls_settings=%d\n", p_call_limit->DayCallLimit.calls_settings);
	printf("day_call_limit_flag=%s\n", p_call_limit->DayCallLimit.day_call_limit_flag?"YES":"NO");
	printf("call_generation_flag=%s\n", p_call_limit->DayCallLimit.call_generation_flag?"YES":"NO");
	printf("cur_calls=%d\n", p_call_limit->DayCallLimit.cur_calls);
	printf("last_date: ");
	show_date(&p_call_limit->DayCallLimit.last_date);

	printf("[day_answer_limit]\n");
	printf("answer_setting=%d\n", p_call_limit->DayAnswerLimit.answer_setting);
	printf("day_answer_limit_flag=%s\n", p_call_limit->DayAnswerLimit.day_answer_limit_flag?"YES":"NO");
	printf("cur_answers=%d\n", p_call_limit->DayAnswerLimit.cur_answers);
	printf("call_answer_flag=%s\n", p_call_limit->DayAnswerLimit.call_answer_flag?"YES":"NO");
	printf("last_date: ");
	show_date(&p_call_limit->DayAnswerLimit.last_date);
	
	printf("[hour_call_limit]\n");
	printf("calls_settings=%d\n", p_call_limit->Hour_Call_Limit.calls_settings);
	printf("hour_call_limit_flag=%s\n", p_call_limit->Hour_Call_Limit.hour_call_limit_flag?"YES":"NO");
	printf("call_generation_flag=%s\n", p_call_limit->Hour_Call_Limit.call_generation_flag?"YES":"NO");
	printf("cur_calls=%d\n", p_call_limit->Hour_Call_Limit.cur_calls);

	printf("[global]\n");
	printf("call_sta=%s\n", call_status_to_str(p_call_limit->call_sta));
	printf("cur_date: ");
	show_date(&p_call_limit->cur_date);
	printf("cur_time: sec=%d, usec=%d\n", p_call_limit->cur_time.tv_sec, p_call_limit->cur_time.tv_usec);
	

	printf("\n");
	return 0;
}
#endif

static void call_limit_hangup_channel(int channel_id)
{
	char handup_cmd[128] = {0};

	sprintf(handup_cmd, HANDUP_CMD, channel_id*2-1);
	system(handup_cmd);
	return;
}
#ifdef DEBUG
#define call_limit_channel(channel_id,type) call_limit_channel_ext(channel_id,type,__LINE__)
static void call_limit_channel(int channel_id, call_limit_status_t type)
static void call_limit_channel_ext(int channel_id, call_limit_status_t type,const int line)
#else
static void call_limit_channel(int channel_id, call_limit_status_t type)
#endif
{
	char limit_cmd[128] = {0};

	sprintf(limit_cmd, CALLLIMIT_CMD, channel_id, type);
	system(limit_cmd);
	return;
}


static void sys_info_init(sys_info_cfg_t *sys_info)
{
	sys_info->IsRefleshFile = ENABLE;
	sys_info->total_chans = MAX_CHAN;
	sys_info->sys_type = CHAN_1SIM;
}

static int call_limit_para_init(void)
{
	int i = 0;
	int j = 0;
	calllimit_chan_t *p_chan = g_call_limit.chans;

	for(i=0; i<MAX_CHAN; i++) {
		chan_config_init(&p_chan->cfg);
		for(j=0; j<SIM_NUM; j++) {
			chan_simcard_init(&p_chan->sim[j]);
		}
		chan_running_status_init(&p_chan->status);
		chan_simswitch_info_init(&p_chan->simswitch);
		get_current_datetime(p_chan);
		gettimeofday(&p_chan->sim_reg_time, NULL);
		pthread_mutex_init(&p_chan->lock, NULL);		
		p_chan->id = i+1;
		p_chan->sim_idx = SIM1;
		p_chan->equeue = queue_create(QUEUESIZE);		
		p_chan++;
	}

	pthread_mutex_init(&g_call_limit.lock, NULL);
	sys_info_init(&g_call_limit.sys_info);
	call_limit_gsoap_init();
	return 0;
}



void calllimit_redis_dlremaintime_set(calllimit_chan_t *p_chan)
{
	redisContext *rc = NULL;
	char *command = (char *)"app.asterisk.dlremaintime.channel";
	redisReply *reply = NULL;
	unsigned char sim_idx = p_chan->sim_idx;
	calltime_info_t *p_calltime_info = &p_chan->sim[sim_idx].calltime_info;	

	rc = redisConnect(DEFAULT_REDIS_IP, REDIS_PORT);
	if (rc->err) {
		log_printf(LOG_ERROR, "can't connect to redis\n");
		return;
	}

	reply = (redisReply *)redisCommand(rc, "hset %s %d %d", command, p_chan->id, p_calltime_info->call_time_remain);
	if ( reply != NULL )
		freeReplyObject(reply);

	redisFree(rc);
	return;
}

void calllimit_redis_dlremaintime_del(calllimit_chan_t *p_chan)
{
	redisContext *rc = NULL;
	char *command = (char *)"app.asterisk.dlremaintime.channel";
	redisReply *reply = NULL;

	rc = redisConnect(DEFAULT_REDIS_IP, REDIS_PORT);
	if (rc->err) {
		log_printf(LOG_ERROR, "can't connect to redis\n");
		return;
	}

	reply = (redisReply *)redisCommand(rc, "hdel %s %d ", command, p_chan->id);
	if ( reply != NULL )
		freeReplyObject(reply);

	redisFree(rc);

	return;
}

void calllimit_redis_update_sim_slot(int chan_id, int slot_id){
	redisContext *rc = NULL;
	redisReply *reply = NULL;
	char *redis_simslot_list = (char *)"app.ledserver.simslot";
	char command[256] = {0};
	sprintf(command, "{\"sim\":\"%d\",\"slot\":\"%d\"}", chan_id, slot_id);
	rc = redisConnect(DEFAULT_REDIS_IP, REDIS_PORT);
	if(rc->err){
		log_printf(LOG_ERROR, "can't connect to redis\n");
		return;
	}
	printf("update sim slot to redis, command:%s\n", command);
	reply = (redisReply *)redisCommand(rc, "rpush %s %s", redis_simslot_list, command);
	if(reply != NULL){
		printf("push to redis fail\n");
		freeReplyObject(reply);
	}
	redisFree(rc);
	return;
}

void calllimit_redis_get_sysinfo( sys_info_cfg_t *sys_info_cfg)
{
	redisContext *rc = NULL;
	redisReply *reply = NULL;
	const char *sys_type_cmd = "get local.product.board.type";
	const char *total_chan_cmd = "get local.product.board.span";
	sys_info_cfg->sys_type = 0;
	sys_info_cfg->total_chans = 0;

	rc = redisConnect(DEFAULT_REDIS_IP, REDIS_PORT);
	if(rc->err){
		log_printf(LOG_ERROR, "can't connect to redis\n");
		return;
	}

	reply = (redisReply *)redisCommand(rc, total_chan_cmd);
	if (reply->type == REDIS_REPLY_ERROR){
		log_printf(LOG_ERROR, "can't connect to redis\n");
	}else{
		if (reply->str != NULL){
			sys_info_cfg->total_chans = atoi(reply->str) ;
			freeReplyObject(reply);
		}
	}

	reply = (redisReply *)redisCommand(rc, sys_type_cmd);
	if (reply->type == REDIS_REPLY_ERROR){
		log_printf(LOG_ERROR, "can't connect to redis\n");
	}else{
		if (reply->str != NULL){
			sys_info_cfg->sys_type = atoi(reply->str) ;
			freeReplyObject(reply);
		}
	}
	redisFree(rc);
}

static void call_time_update(calllimit_chan_t *p_chan){
	int duration_time;
	struct timeval now;

	
	unsigned char sim_idx = p_chan->sim_idx;
	calltime_info_t *p_calltime_info = &p_chan->sim[sim_idx].calltime_info;
	calltime_conf_t *p_calltime_cfg = &p_chan->cfg.calltime_cfg;
	simswitch_conf_t *p_simswitch_cfg = &p_chan->cfg.simswitch_cfg;
	

	gettimeofday(&now, NULL);
	duration_time = now.tv_sec -  p_calltime_info->start_call_time.tv_sec;
	if(duration_time < p_calltime_cfg->call_time_free)
		return;
	if( p_calltime_cfg->call_time_total > 0 && duration_time){

		if(duration_time < p_calltime_cfg->call_time_free)
			return;
		int duration = (duration_time/p_calltime_cfg->call_time_step) + ((duration_time%p_calltime_cfg->call_time_step == 0)? 0 : 1);
		if(duration > p_calltime_info->call_time_remain){//remain time is less than duration
			p_calltime_info->call_time_remain= 0;
			p_calltime_info->call_time_count = p_calltime_info->call_time_count;
		}else{
			p_calltime_info->call_time_remain  -= duration;
			p_calltime_info->call_time_count += duration;
		}
	}

	if( p_simswitch_cfg->sim_switch_sw == ENABLE && p_simswitch_cfg->total_callout_time > 0 && duration_time){
		
		int duration = (duration_time/60) + ((duration_time%60 == 0)? 0 : 1);
		p_chan->simswitch.curr_callout_time += duration*60;
	}
	
}



static void * call_time_limit_thread_func(void * p)
{
	calllimit_chan_t *p_chan = (calllimit_chan_t *)p;
	int timeout = 0;
	int remain = 0;
	struct timespec ts;

	unsigned char sim_idx = p_chan->sim_idx;
	running_chan_status_t *chan_status = &p_chan->status;
	calltime_info_t *p_calltime_info = &p_chan->sim[sim_idx].calltime_info;
	calltime_conf_t *p_calltime_cfg = &p_chan->cfg.calltime_cfg;
	simswitch_conf_t *p_simswitch_cfg = &p_chan->cfg.simswitch_cfg;
	

	pthread_mutex_lock(&p_chan->lock);	
	if(p_calltime_cfg->call_time_sw == ENABLE) {
		if( p_calltime_cfg->call_time_single_sw == ENABLE  && p_calltime_cfg->call_time_settings){
			timeout = p_calltime_cfg->call_time_settings*p_calltime_cfg->call_time_step ;
			p_calltime_info->cond_flag = FLAG_YES;
		}
		if(timeout == 0 ||( p_calltime_cfg->call_time_total_sw==ENABLE && timeout > p_calltime_info->call_time_remain* p_calltime_cfg->call_time_step)){
			timeout = p_calltime_info->call_time_remain*p_calltime_cfg->call_time_step;
			p_calltime_info->cond_flag = FLAG_YES;
		}
	}
	
	remain = p_simswitch_cfg->total_callout_time-p_chan->simswitch.curr_callout_time/60;
	if(timeout == 0 ||( p_simswitch_cfg->sim_switch_sw == ENABLE  && p_simswitch_cfg->total_callout_time > 0 && timeout > remain*60)){
		timeout = remain*60;
		p_calltime_info->cond_flag = FLAG_YES;
	}	

	if(p_calltime_info->cond_flag == FLAG_NO){
		pthread_mutex_unlock(&p_chan->lock);	
		return NULL;
	}
	ts.tv_sec = p_calltime_info->start_call_time.tv_sec + timeout;
	pthread_mutex_unlock(&p_chan->lock);

	pthread_mutex_lock(&p_calltime_info->cond_lock);	
	if( 0 != pthread_cond_timedwait(&p_calltime_info->cond, &p_calltime_info->cond_lock, &ts) ) { //Timeout, Hangup
		if(chan_status->call_sta == CALL_ANSWER){
			call_limit_hangup_channel(p_chan->id);
		}
	}
	pthread_mutex_unlock(&p_calltime_info->cond_lock);	

	return NULL;
}


static int start_call_time_limit(calllimit_chan_t *p_chan) 
{
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	calltime_info_t *p_calltime_info = &p_chan->sim[sim_idx].calltime_info;
	
	if(p_calltime_info->cond_flag == FLAG_NO) {
		calllimit_pthread_create(&p_calltime_info->tid, NULL, call_time_limit_thread_func, p_chan);
	}
	return 0;
}

static int stop_call_time_limit(calllimit_chan_t *p_chan)
{

	unsigned char sim_idx = p_chan->sim_idx;
	calltime_info_t *p_calltime_info = &p_chan->sim[sim_idx].calltime_info;

	pthread_mutex_lock(&p_chan->lock);

	if(p_calltime_info->cond_flag == FLAG_NO) {
		pthread_mutex_unlock(&p_chan->lock);
		return -1;
	}
	
	p_calltime_info->cond_flag = FLAG_NO;
	p_calltime_info->call_answer_flag = FLAG_NO;
	p_calltime_info->call_time_redis_flag = FLAG_NO;
	pthread_mutex_unlock(&p_chan->lock);
	pthread_cond_signal(&p_calltime_info->cond);
	call_time_update(p_chan);
	return 0;
}

static void sms_limit_channel(int channel_id, sms_limit_status_t type)
{
	char limit_cmd[128] = {0};
	sprintf(limit_cmd, SMSLIMIT_CMD, channel_id, type);
	system(limit_cmd);
	return;
} 

static int call_time_clean_date_is_timeout(struct tm* ptm, int type)
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
			case CALL_TIME_CLEAN_TYPE_DAY:   add = 3600 * 24; break;
			case CALL_TIME_CLEAN_TYPE_WEEK:  add = 3600 * 24 * 7; break;
			case CALL_TIME_CLEAN_TYPE_MON: add = 3600 * 24 * day_of_month; break;
			default: return 0;
		}
		set += add;
		memcpy(ptm, localtime(&set), sizeof(struct tm));
	}

	return res;
}

static int check_call_time_limit(calllimit_chan_t *p_chan)
{		
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null.\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	running_chan_status_t *chan_status = &p_chan->status;
	calltime_info_t *p_calltime_info = &p_chan->sim[sim_idx].calltime_info;
	calltime_conf_t *p_calltime_cfg = &p_chan->cfg.calltime_cfg;

	if(p_calltime_cfg->call_time_single_sw == DISABLE && p_calltime_cfg->call_time_total_sw == DISABLE){
		if(p_calltime_info->call_time_redis_flag == FLAG_NO){
			calllimit_redis_dlremaintime_del(p_chan);
			p_calltime_info->call_time_redis_flag = FLAG_YES;
		}
		return 0;
	}
	
 	if((chan_status->call_sta == CALL_ANSWER) && (p_calltime_info->call_answer_flag == FLAG_NO)) {
		p_calltime_info->call_answer_flag = FLAG_YES;
		gettimeofday(&p_calltime_info->start_call_time, NULL);
		start_call_time_limit(p_chan);
	} else if(chan_status->call_sta == CALL_HANDUP) {
		stop_call_time_limit(p_chan);
	}
	
	//call time limit check
	if(p_calltime_info->call_time_remain == 0 &&
		p_calltime_cfg->call_time_total > 0 && 
		p_calltime_info->call_time_limit_flag == FLAG_NO){
		p_calltime_info->call_time_limit_flag = FLAG_YES;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]call time limited.\n", p_chan->id, sim_idx+1);
		int res = -1;
		res = switch_chan_simcard(p_chan, FLAG_NO);
		if(res) {
			chan_status->call_time_limit_sta = CALLLIMIT;
			call_limit_channel(p_chan->id, CALLLIMIT);
		}
	}
	
	//update redis app.asterisk.dlremaintime.channel [chn] [value]
	if(p_calltime_info->call_time_redis_flag == FLAG_NO ){
//		if(p_call_limit->CallTimeLimit.call_time_total_sw == ENABLE && p_call_limit->call_sta != SIM_OUT)
		if(p_calltime_cfg->call_time_total_sw == ENABLE )//set redis app.asterisk.dlremaintime.channel [chn] [value]
			calllimit_redis_dlremaintime_set(p_chan);
		else//delete redis  app.asterisk.dlremaintime.channel [chn]
			calllimit_redis_dlremaintime_del(p_chan);
		p_calltime_info->call_time_redis_flag = FLAG_YES;
	}

	if( p_calltime_cfg->call_time_clean_sw == ENABLE
		&& p_calltime_cfg->call_time_clean_type != CALL_TIME_CLEAN_TYPE_UNKOWN 
		&& call_time_clean_date_is_timeout(&p_calltime_info->call_time_clean_date, p_calltime_cfg->call_time_clean_type)){

		p_calltime_info->call_time_remain = p_calltime_cfg->call_time_total;
		p_calltime_info->call_time_count = 0;
		p_calltime_info->sms_warning_flag = FLAG_NO;                 //clear call time send flag
		p_calltime_info->call_time_redis_flag = FLAG_NO;
		if(p_calltime_info->call_time_limit_flag == FLAG_YES) {
			p_calltime_info->call_time_limit_flag = FLAG_NO;            //clear call time limit flag
			log_printf(LOG_INFO, "[chan_%d][sim_%d]call time unlimit.\n", p_chan->id, sim_idx+1);
		}	
	}
	return 0;
}


static int cycle_clean_day_calls(calllimit_chan_t *p_chan)
{
	unsigned char sim_idx = p_chan->sim_idx;
	calllimit_info_t *p = &p_chan->sim[sim_idx].calllimit_info;
	int i = 0;

	for(i=0; i<SIM_NUM; i++){
		p = &p_chan->sim[i].calllimit_info;
		if( (p_chan->cur_date.tm_mday != p->day_call_last_clean_date.tm_mday) &&
			 (p_chan->cur_date.tm_hour == 0) && 
			  (p_chan->cur_date.tm_min == 0) &&
				(p_chan->cur_date.tm_sec == 0) ) {
			p->day_cur_calls = 0;
			if(FLAG_YES == p->day_call_limit_flag ) {
				p->day_call_limit_flag = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]day call unlimit.\n", p_chan->id, i+1);
			}
			p->daycall_generation_flag = FLAG_NO;
			p->day_call_last_clean_date = p_chan->cur_date;
		}
	}
	return 0;
}

static int check_day_call_limit(calllimit_chan_t *p_chan)
{
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null.\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	running_chan_status_t *chan_status = &p_chan->status;
	calllimit_info_t *p_calllimit_info = &p_chan->sim[sim_idx].calllimit_info;
	calllimit_conf_t *p_calllimit_cfg = &p_chan->cfg.calllimit_cfg;

	if(p_calllimit_cfg->day_calls_settings == 0) {
		return 0;
	}

	pthread_mutex_lock(&p_chan->lock);

	if( (chan_status->call_sta == CALL_DAIL) && (p_calllimit_info->daycall_generation_flag == FLAG_NO) ) {
		p_calllimit_info->daycall_generation_flag = FLAG_YES;
		p_calllimit_info->day_cur_calls++;		
	} 

	if((p_calllimit_info->daycall_generation_flag == FLAG_YES) && (chan_status->call_sta == CALL_HANDUP)) {
		p_calllimit_info->daycall_generation_flag = FLAG_NO;
	}

	if(p_calllimit_info->day_cur_calls >= p_calllimit_cfg->day_calls_settings && (p_calllimit_info->day_call_limit_flag == FLAG_NO)) {
		if( (chan_status->call_sta == CALL_HANDUP) && (chan_status->sys_limit_sta == UNLIMIT) ) {
			int res = -1;
			p_calllimit_info->day_call_limit_flag = FLAG_YES;
			log_printf(LOG_INFO, "[chan_%d][sim_%d]day call limited.\n", p_chan->id, sim_idx+1);
			res = switch_chan_simcard(p_chan, FLAG_NO);
			if(res) {
				chan_status->sys_limit_sta = CALLLIMIT;
				call_limit_channel(p_chan->id, CALLLIMIT);
			}
		}
	}

	cycle_clean_day_calls(p_chan);
	pthread_mutex_unlock(&p_chan->lock);

	return 0;
	
}


static int cycle_clean_hour_calls(calllimit_chan_t *p_chan)
{
	unsigned char sim_idx = p_chan->sim_idx;
	calllimit_info_t *p = &p_chan->sim[sim_idx].calllimit_info;
	int i = 0;

	for(i=0; i<SIM_NUM; i++){	
		p = &p_chan->sim[i].calllimit_info;
		if( (p_chan->cur_date.tm_hour != p->hour_call_last_clean_date.tm_hour) &&
			  (p_chan->cur_date.tm_min == 0) &&
			   (p_chan->cur_date.tm_sec == 0) ) {
			p->hour_cur_calls = 0;	
			if(FLAG_YES == p->hour_call_limit_flag) {
				p->hour_call_limit_flag = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]hour call unlimit.\n", p_chan->id, i+1);
			}
			p->hourcall_generation_flag = FLAG_NO;
			p->hour_call_last_clean_date = p_chan->cur_date;
		} 
	}
	return 0;
}


static int check_hour_call_limit(calllimit_chan_t *p_chan)
{
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null.\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	running_chan_status_t *chan_status = &p_chan->status;
	calllimit_info_t *p_calllimit_info = &p_chan->sim[sim_idx].calllimit_info;
	calllimit_conf_t *p_calllimit_cfg = &p_chan->cfg.calllimit_cfg;

	if(p_calllimit_cfg->hour_calls_settings == 0) {
		return 0;
	}

	pthread_mutex_lock(&p_chan->lock);

	if( (chan_status->call_sta == CALL_DAIL) && (p_calllimit_info->hourcall_generation_flag == FLAG_NO) ) {
		p_calllimit_info->hourcall_generation_flag = FLAG_YES;
		p_calllimit_info->hour_cur_calls++;
	} 

	if((p_calllimit_info->hourcall_generation_flag == FLAG_YES) && (chan_status->call_sta == CALL_HANDUP)) {
		p_calllimit_info->hourcall_generation_flag = FLAG_NO;
	}

	if(p_calllimit_info->hour_cur_calls >= p_calllimit_cfg->hour_calls_settings && (p_calllimit_info->hour_call_limit_flag == FLAG_NO)) {
		if( (chan_status->call_sta == CALL_HANDUP) && (chan_status->sys_limit_sta == UNLIMIT)) {
			int res = -1;
			p_calllimit_info->hour_call_limit_flag = FLAG_YES;
			log_printf(LOG_INFO, "[chan_%d][sim_%d]hour call limited.\n", p_chan->id, sim_idx+1);
			res = switch_chan_simcard(p_chan, FLAG_NO);
			if(res) {
				chan_status->sys_limit_sta = CALLLIMIT;
				call_limit_channel(p_chan->id, CALLLIMIT);
			}
		}		
	}

	cycle_clean_hour_calls(p_chan);
	pthread_mutex_unlock(&p_chan->lock);
	return 0;
}


static int cycle_clean_day_answers(calllimit_chan_t *p_chan)
{
	unsigned char sim_idx = p_chan->sim_idx;
	calllimit_info_t *p = &p_chan->sim[sim_idx].calllimit_info;
	int i = 0;
	
	for(i=0; i<SIM_NUM; i++){	
		p = &p_chan->sim[i].calllimit_info;
		if( (p_chan->cur_date.tm_mday != p->day_answer_last_clean_date.tm_mday) &&
			 (p_chan->cur_date.tm_hour == 0) && 
			   (p_chan->cur_date.tm_min == 0) &&
			     (p_chan->cur_date.tm_sec == 0) ) {
			p->day_cur_answers = 0;	
			if(FLAG_YES == p->day_answer_limit_flag) {
				p->day_answer_limit_flag = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]day answer unlimit.\n", p_chan->id, i+1);
			}
			p->call_answer_flag = FLAG_NO;
			p->day_answer_last_clean_date = p_chan->cur_date;
		} 
	}
	return 0;
}


static int check_day_answer_limit(calllimit_chan_t *p_chan)
{
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null.\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	running_chan_status_t *chan_status = &p_chan->status;
	calllimit_info_t *p_calllimit_info = &p_chan->sim[sim_idx].calllimit_info;
	calllimit_conf_t *p_calllimit_cfg = &p_chan->cfg.calllimit_cfg;

	if(p_calllimit_cfg->day_answer_setting == 0) {
		return 0;
	}

	pthread_mutex_lock(&p_chan->lock);

 	if((chan_status->call_sta == CALL_ANSWER) && (p_calllimit_info->call_answer_flag == FLAG_NO)) {
		p_calllimit_info->day_cur_answers++;
		p_calllimit_info->call_answer_flag = FLAG_YES;
//		refresh_call_limit_status_cfg(&g_call_limit, p_call_limit->id);
	} 

	if((p_calllimit_info->call_answer_flag == FLAG_YES) && (chan_status->call_sta == CALL_HANDUP)) {
		p_calllimit_info->call_answer_flag = FLAG_NO;
	}

	if(p_calllimit_info->day_cur_answers >= p_calllimit_cfg->day_answer_setting && (p_calllimit_info->day_answer_limit_flag == FLAG_NO)) {
		if( (chan_status->call_sta == CALL_HANDUP) && (chan_status->sys_limit_sta == UNLIMIT)) {
			int res = -1;
			p_calllimit_info->day_answer_limit_flag = FLAG_YES;
			log_printf(LOG_INFO, "[chan_%d][sim_%d]day answer limited.\n", p_chan->id, sim_idx+1);
			res = switch_chan_simcard(p_chan, FLAG_NO);
			if(res) {
				chan_status->sys_limit_sta = CALLLIMIT;
				call_limit_channel(p_chan->id, CALLLIMIT);
			}
		}		

	}

	cycle_clean_day_answers(p_chan);

	pthread_mutex_unlock(&p_chan->lock);

	return 0;
	
}

static void clean_all_call_limit(calllimit_chan_t *p_chan, unsigned char sim_idx)
{
	calllimit_info_t *p = &p_chan->sim[sim_idx].calllimit_info;
		
	p->day_cur_calls = 0;
	if(p->day_call_limit_flag == FLAG_YES) {
		p->day_call_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]day call unlimit.\n", p_chan->id, sim_idx+1);
	}
	
	p->day_cur_answers = 0;
	if(p->day_answer_limit_flag == FLAG_YES) {
		p->day_answer_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]day answer unlimit.\n", p_chan->id, sim_idx+1);
	}
	
	p->hour_cur_calls = 0;
	if(p->hour_call_limit_flag == FLAG_YES) {
		p->hour_call_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]hour call unlimit.\n", p_chan->id, sim_idx+1);
	}
	return;
}

static int clean_sys_call_limit_status(calllimit_chan_t *p_chan)
{
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null.\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	sys_info_cfg_t *sys_info = &g_call_limit.sys_info;
	running_chan_status_t *chan_status = &p_chan->status;
	calltime_info_t *p_calltime_info = &p_chan->sim[sim_idx].calltime_info;
	calllimit_info_t *p_calllimit_info = &p_chan->sim[sim_idx].calllimit_info;
	calllock_info_t *p_calllock_info = &p_chan->sim[sim_idx].calllock_info;

	pthread_mutex_lock(&p_chan->lock);
	if( (CALLLIMIT == chan_status->sys_limit_sta || CALLLIMIT == chan_status->call_time_limit_sta) &&
		(FLAG_NO ==  p_calltime_info->call_time_limit_flag) &&
		(FLAG_NO == p_calllimit_info->day_call_limit_flag) &&
		 (FLAG_NO == p_calllimit_info->day_answer_limit_flag) &&
		  (FLAG_NO == p_calllimit_info->hour_call_limit_flag) ) {
		chan_status->sys_limit_sta = UNLIMIT;
		chan_status->call_time_limit_sta = UNLIMIT;
		call_limit_channel(p_chan->id, UNLIMIT);
	} else if (SIM_OUT == chan_status->call_sta && sys_info->sys_type != CHAN_4SIM) {
		clean_all_call_limit(p_chan, p_chan->sim_idx);
		chan_status->sys_limit_sta = UNLIMIT;
		if(chan_status->call_time_limit_sta == UNLIMIT && FLAG_NO ==  p_calltime_info->call_time_limit_flag) {
			call_limit_channel(p_chan->id, UNLIMIT);
		} 
	}

	if( (UNLIMIT == chan_status->sys_limit_sta) && (
		 (FLAG_YES == p_calllimit_info->day_call_limit_flag) ||
		 (FLAG_YES == p_calllimit_info->day_answer_limit_flag) ||
		  (FLAG_YES == p_calllimit_info->hour_call_limit_flag) )) {
		chan_status->sys_limit_sta = CALLLIMIT;
		call_limit_channel(p_chan->id, CALLLIMIT);
	}

	if( (UNLIMIT == chan_status->call_time_limit_sta) && (FLAG_YES ==  p_calltime_info->call_time_limit_flag) ) {
		chan_status->call_time_limit_sta = CALLLIMIT;
		call_limit_channel(p_chan->id, CALLLIMIT);
	}	
		
	pthread_mutex_unlock(&p_chan->lock);
	return 0;
}

static int  check_call_lock(calllimit_chan_t *p_chan)
{
    int ret = 0;
	unsigned char sim_idx = SIM1;
    int need_refresh_flag = 0;
    call_status_t sms_status;
    
    if(!p_chan) {
        log_printf(LOG_ERROR, "p_chan is null.\n");
        return -1;
    }

	sim_idx = p_chan->sim_idx;
	running_chan_status_t *chan_status = &p_chan->status;
	calllock_info_t *p_calllock_info = &p_chan->sim[sim_idx].calllock_info;
	calllock_conf_t *p_calllock_cfg = &p_chan->cfg.calllock_cfg;
	
    pthread_mutex_lock(&p_chan->lock);

    if(p_calllock_cfg->call_detect_flag == FLAG_NO)
    {
        pthread_mutex_unlock(&p_chan->lock);
        return 0;
    }

	if( (CALLLIMIT == chan_status->call_lock_sta ) && (FLAG_NO == p_calllock_info->call_fail_lock_status) ) {
		chan_status->call_lock_sta = UNLIMIT;
		chan_status->call_mark_sta = UNLIMIT;
		cmd_call_mark(FLAG_NO, p_chan->id);
		cmd_call_lock(FLAG_NO, p_chan->id);
	} else if( (UNLIMIT == chan_status->call_lock_sta ) && (FLAG_YES == p_calllock_info->call_fail_lock_status) ) {
		chan_status->call_lock_sta = CALLLIMIT;
		chan_status->call_mark_sta = CALLLIMIT;
		cmd_call_mark(FLAG_YES, p_chan->id);
		cmd_call_lock(FLAG_YES, p_chan->id);			
	}

	if( (CALLLIMIT == chan_status->call_mark_sta ) && (FLAG_NO == p_calllock_info->call_fail_mark_status) ) {
		chan_status->call_mark_sta = UNLIMIT;
		cmd_call_mark(FLAG_NO, p_chan->id);
	} else if ( (UNLIMIT == chan_status->call_mark_sta ) && (FLAG_YES == p_calllock_info->call_fail_mark_status) ) {
		chan_status->call_mark_sta = CALLLIMIT;
		cmd_call_mark(FLAG_YES, p_chan->id);	
	} 

	if((chan_status->last_sta == CALL_DAIL && chan_status->call_sta == CALL_HANDUP)
        ||(chan_status->last_sta == CALL_RING && chan_status->call_sta == CALL_HANDUP))
    {
        if(p_calllock_info->call_fail_lock_status == FLAG_NO && chan_status->sys_limit_sta == UNLIMIT)
            p_calllock_info->call_failed_count ++;
        log_printf(LOG_DEBUG, "[chan_%d][sim_%d]call_failed_count = %d.\n", p_chan->id, sim_idx+1, p_calllock_info->call_failed_count);
        chan_status->last_sta = CALL_HANDUP; 
    }else if(chan_status->last_sta == chan_status->call_sta){
        pthread_mutex_unlock(&p_chan->lock);
        return 0;
    }

    if(p_calllock_cfg->call_fail_lock_sms_report_flag == FLAG_YES)
    {
        sms_status = SMS_REPORT_SUCCESS;
    }else{
        sms_status = SMS_SUCCESS;
    }
    
    if(chan_status->call_sta == CALL_ANSWER 
        || chan_status->call_sta == SIM_OUT
        || chan_status->call_sta == sms_status)
    {
        ret = 0;
        ret = call_mark_clean(p_chan);
        if(ret == 1)
        {
            need_refresh_flag = ret;
        }
    }

    if(chan_status->call_sta == SIM_OUT && p_calllock_cfg->call_fail_lock_flag == FLAG_YES)
    {
        ret = 0;
        ret =call_lock_clean(p_chan);
        if(ret == 1)
        {
            need_refresh_flag = ret;
        }
    }
    
    if(p_calllock_info->call_failed_count >= p_calllock_cfg->call_fail_mark_count && p_calllock_cfg->call_fail_mark_flag == FLAG_YES)
    {
        if(p_calllock_cfg->call_fail_mark_count > 0)
        {
            ret = 0;    
            ret = call_mark_set(p_chan,0);
            if(ret == 1)
            {
                need_refresh_flag = ret;
            }
        }
    }

    if(p_calllock_info->call_failed_count >= p_calllock_cfg->call_fail_lock_count && p_calllock_cfg->call_fail_lock_flag == FLAG_YES)
    {
        ret = 0;
        if(p_calllock_info->call_fail_lock_status != FLAG_YES && p_calllock_cfg->call_fail_lock_count > 0){
            if( chan_status->call_sta == SMS_FAILED)
                p_calllock_info->call_fail_send_sms_times ++;
            else if(p_calllock_cfg->call_fail_lock_sms_report_flag==FLAG_YES && chan_status->call_sta == SMS_REPORT_FAILED)
                p_calllock_info->call_fail_send_sms_times ++;
                        
            if(p_calllock_info->call_fail_send_sms_times > p_calllock_cfg->call_fail_lock_sms_count)
            {
                ret = call_lock_set(p_chan, 2, 0);
                chan_status->last_sta = chan_status->call_sta;
                if(ret == 1)
                {
                    need_refresh_flag = ret;
                }
            }else {
                if(p_calllock_cfg->call_fail_lock_sms_flag == FLAG_YES){
                    if( (chan_status->call_sta == SMS_FAILED) || (p_calllock_info->call_fail_lock_status == 0) )
                    {
                       log_printf(LOG_INFO, "[chan_%d][sim_%d]Send message test!\n", p_chan->id, sim_idx+1);
                        call_sms_send(p_chan);
                        chan_status->last_sta = chan_status->call_sta;
                        chan_status->call_sta = SMS_SENDING;
                    }
                    if(p_calllock_info->call_fail_lock_status != 2){
                        ret = call_lock_set(p_chan, 1, 0);
                        if(ret == 1)
                        {
                            need_refresh_flag = ret;
                        }
                     }
                     
                }else{
                    ret = call_lock_set(p_chan,2,0);
                    chan_status->last_sta = chan_status->call_sta;
                    if(ret == 1)
                    {
                        need_refresh_flag = ret;
                    }
                }
            }
        }
    }

    pthread_mutex_unlock(&p_chan->lock);
    return 0;
}

static int cycle_clean_day_sms(calllimit_chan_t *p_chan)
{
	unsigned char sim_idx = p_chan->sim_idx;
	int i = 0;
	running_chan_status_t *chan_status = &p_chan->status;
	smslimit_info_t *p_smslimit_info = &p_chan->sim[sim_idx].smslimit_info;	

	for(i=0; i<SIM_NUM; i++) {
		p_smslimit_info = &p_chan->sim[i].smslimit_info;
		if( (p_chan->cur_date.tm_mday != p_smslimit_info->day_last_date.tm_mday) &&
			 (p_chan->cur_date.tm_hour == 0) && 
			  (p_chan->cur_date.tm_min == 0) &&
			    (p_chan->cur_date.tm_sec == 0) ) {
			p_smslimit_info->day_cur_sms = 0;
			if(SMS_LIMIT== p_smslimit_info->day_sms_limit_flag && SMS_UNLIMIT == p_smslimit_info->mon_sms_limit_flag) {
				p_smslimit_info->day_sms_limit_flag = SMS_UNLIMIT;
				p_smslimit_info->day_last_date = p_chan->cur_date;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]day sms unlimit.\n", p_chan->id, sim_idx+1);	
				if(chan_status->sms_limit_sta == SMS_LIMIT) {
					chan_status->sms_limit_sta = SMS_UNLIMIT;
					sms_limit_channel(p_chan->id, SMS_UNLIMIT);
				}
			}
		}
	}
	return 0;
}

static int cycle_clean_mon_sms(calllimit_chan_t *p_chan)
{
	unsigned char sim_idx = p_chan->sim_idx;
	int i = 0;
	running_chan_status_t *chan_status = &p_chan->status;
	smslimit_info_t *p_smslimit_info = &p_chan->sim[sim_idx].smslimit_info;	

	for(i=0; i<SIM_NUM; i++) {	
		p_smslimit_info = &p_chan->sim[i].smslimit_info;	
		if(( p_chan->cur_date.tm_mon != p_smslimit_info->mon_last_date.tm_mon || p_smslimit_info->mon_last_date.tm_year != p_smslimit_info->mon_last_date.tm_year ) &&
			(p_chan->cur_date.tm_mday == (int)p_chan->cfg.smslimit_cfg.sms_clean_date) &&
			 (p_chan->cur_date.tm_hour == 0) && 
			  (p_chan->cur_date.tm_min == 0) &&
			    (p_chan->cur_date.tm_sec == 0) ) {
			p_smslimit_info->mon_cur_sms = 0;
			p_smslimit_info->day_cur_sms = 0;
			p_smslimit_info->mon_last_date = p_chan->cur_date;
			p_smslimit_info->day_last_date = p_chan->cur_date;
			if( SMS_LIMIT == p_smslimit_info->mon_sms_limit_flag ) {
				p_smslimit_info->mon_sms_limit_flag = SMS_UNLIMIT;
				if(p_smslimit_info->day_sms_limit_flag == SMS_LIMIT) {
					p_smslimit_info->day_sms_limit_flag = SMS_UNLIMIT;
					log_printf(LOG_INFO, "[chan_%d][sim_%d]day sms unlimit.\n", p_chan->id, sim_idx+1);
				}
				p_smslimit_info->sms_limit_warning_flag = FLAG_NO;  //clear smslimit sms warning send flag
				log_printf(LOG_INFO, "[chan_%d][sim_%d]mon sms unlimit.\n", p_chan->id, sim_idx+1);
				if(SMS_LIMIT == chan_status->sms_limit_sta) {
					chan_status->sms_limit_sta = SMS_UNLIMIT;
					sms_limit_channel(p_chan->id, SMS_UNLIMIT);
				}
			}
		}
	}
	return 0;
}
static int  check_sms_limit(calllimit_chan_t *p_chan){
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null.\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	running_chan_status_t *chan_status = &p_chan->status;
	smslimit_info_t *p_smslimit_info = &p_chan->sim[sim_idx].smslimit_info;
	smslimit_conf_t *p_smslimit_cfg = &p_chan->cfg.smslimit_cfg;
	
	
	if(p_smslimit_cfg->day_sms_settings == 0 && p_smslimit_cfg->mon_sms_settings == 0){//0=>????δ????
		return 0;
	}
	
	pthread_mutex_lock(&p_chan->lock);

	if((chan_status->sms_limit_sta == SMS_LIMIT) && (p_smslimit_info->day_sms_limit_flag == FLAG_NO) &&
		 (p_smslimit_info->mon_sms_limit_flag == FLAG_NO) ) {
		chan_status->sms_limit_sta = SMS_UNLIMIT;
		sms_limit_channel(p_chan->id, SMS_UNLIMIT);
	}else if((chan_status->sms_limit_sta == SMS_UNLIMIT) &&  ((p_smslimit_info->day_sms_limit_flag == FLAG_YES) ||
		 (p_smslimit_info->mon_sms_limit_flag == FLAG_YES)) ) {
		chan_status->sms_limit_sta = SMS_LIMIT;
		sms_limit_channel(p_chan->id, SMS_LIMIT);
	}

	if(chan_status->sms_sta == SMS_SUCCESS|| chan_status->sms_sta == SMS_SEND_FAILED || chan_status->sms_sta == SMS_FAILED){
		if(p_smslimit_cfg->smslimit_success_flag == DISABLE ){
			if(chan_status->sms_sta == SMS_SUCCESS){
				if(p_smslimit_cfg->day_sms_settings > 0)
					p_smslimit_info->day_cur_sms++;//day sms count
				if(p_smslimit_cfg->mon_sms_settings > 0)
					p_smslimit_info->mon_cur_sms++;
			}
		}else{
			if(p_smslimit_cfg->day_sms_settings > 0)
				p_smslimit_info->day_cur_sms++;//day sms count
			if(p_smslimit_cfg->mon_sms_settings > 0)
				p_smslimit_info->mon_cur_sms++;
		}
		chan_status->sms_sta = CALL_UNKNOW;
	}

	if(p_smslimit_cfg->day_sms_settings > 0 &&  p_smslimit_info->day_cur_sms >= p_smslimit_cfg->day_sms_settings &&
		 (SMS_UNLIMIT == p_smslimit_info->day_sms_limit_flag) ) {//day limit
		int res = -1;
		p_smslimit_info->day_sms_limit_flag = SMS_LIMIT;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]day sms limited.\n", p_chan->id, p_chan->sim_idx+1);		
		res = switch_chan_simcard(p_chan, FLAG_NO);
		if(res) {
			chan_status->sms_limit_sta = SMS_LIMIT;
			sms_limit_channel(p_chan->id, SMS_LIMIT);
		}
	}
	
	if( p_smslimit_cfg->mon_sms_settings > 0 && p_smslimit_info->mon_cur_sms >= p_smslimit_cfg->mon_sms_settings && 
	     (SMS_UNLIMIT == p_smslimit_info->mon_sms_limit_flag) ){//month limit
		int res = -1;
		p_smslimit_info->mon_sms_limit_flag = SMS_LIMIT;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]mon sms limited.\n", p_chan->id, sim_idx+1);	
		res = switch_chan_simcard(p_chan, FLAG_NO);
		if(res) {
			chan_status->sms_limit_sta = SMS_LIMIT;
			sms_limit_channel(p_chan->id, SMS_LIMIT);
		}
	}
	
	cycle_clean_day_sms(p_chan);
	cycle_clean_mon_sms(p_chan);
	
	pthread_mutex_unlock(&p_chan->lock);
	
	return 0;
}


static void send_warning_sms(char *warning_callee, char *warning_msg, unsigned int id ){
    char *cmd_sms_test;
    asprintf(&cmd_sms_test,SEND_MSG,id,warning_callee,warning_msg);
//	print_debug("send sms cmd=%s\n", cmd_sms_test);
    system(cmd_sms_test);
    free(cmd_sms_test);
	return;	
}

static int check_sms_warning(calllimit_chan_t *p_chan){	
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null.\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	calltime_info_t *pcalltime_info = &p_chan->sim[sim_idx].calltime_info;	 
	calltime_conf_t *pcalltime_cfg = &p_chan->cfg.calltime_cfg;	 
	
	if(pcalltime_cfg->call_time_warning_num >  0 && pcalltime_cfg->call_time_total > 0){
		if(pcalltime_info->sms_warning_flag == FLAG_NO && (int)pcalltime_cfg->call_time_warning_num >= pcalltime_info->call_time_remain){
			send_warning_sms(pcalltime_cfg->call_time_warning_callee, pcalltime_cfg->call_time_warning_msg, p_chan->id);
			pcalltime_info->sms_warning_flag  = FLAG_YES;
			log_printf(LOG_INFO, "send CallTime smslimit sms warning:%s to %s\n", pcalltime_cfg->call_time_warning_msg, pcalltime_cfg->call_time_warning_callee);
		}
	}
	return 0;
}

	
static int check_sim_switch(calllimit_chan_t *p_chan)
{
	if(!p_chan) {
		log_printf(LOG_ERROR, "p_chan is null.\n");
		return -1;
	}

	unsigned char sim_idx = p_chan->sim_idx;
	calltime_info_t *p_calltime_info = &p_chan->sim[sim_idx].calltime_info;	
	chan_simswitch_info_t *p = &p_chan->simswitch;
	simswitch_conf_t *p_cfg = &p_chan->cfg.simswitch_cfg;
	calltime_conf_t *p_calltime_cfg = &p_chan->cfg.calltime_cfg;
	int res = -1;

	if(p->sim_switch_fail_flag == FLAG_YES) {	
		return 0;
	}

	if(p_cfg->total_callout_count > 0) {
		if( (p_chan->status.call_sta == CALL_DAIL) && (p->call_generation_flag == FLAG_NO) ) {
			p->call_generation_flag = FLAG_YES;
			p->curr_callout_count++;
//			print_debug("curr_callout_count=%d\n", p->curr_callout_count);
		} else if((p->call_generation_flag == FLAG_YES) && (p_chan->status.call_sta == CALL_HANDUP)) {
			p->call_generation_flag = FLAG_NO;
		}
	}
	
	if(p_cfg->total_callout_time > 0) {
	 	if((p_chan->status.call_sta == CALL_ANSWER) && (p->call_answer_flag == FLAG_NO)) {
			if( (p_calltime_cfg->call_time_sw == DISABLE) || (p_calltime_cfg->call_time_single_sw == DISABLE && p_calltime_cfg->call_time_total_sw == DISABLE) ) {
				p->call_answer_flag = FLAG_YES;
				gettimeofday(&p_calltime_info->start_call_time, NULL);
				start_call_time_limit(p_chan);
			}
		} else if((p_chan->status.call_sta == CALL_HANDUP) && (p->call_answer_flag == FLAG_YES)) {
			p->call_answer_flag = FLAG_NO;
			stop_call_time_limit(p_chan);
		}
	}

	if(p_cfg->total_sms_count > 0) {	
		if(p_chan->status.sms_sta == SMS_SUCCESS|| p_chan->status.sms_sta == SMS_SEND_FAILED || p_chan->status.sms_sta == SMS_FAILED){
			p->curr_sms_count++;
			p_chan->status.sms_sta = CALL_UNKNOW;
//			print_debug("curr_sms_count=%d\n", p->curr_sms_count);
		}
	}

	if(p_cfg->total_using_time > 0 && p->using_time_count++ > 500) {
		update_using_time(p);
		p->using_time_count = 0;
	} else if ( (p_cfg->total_using_time == 0) && (p->using_time_flag != FLAG_NO) ) {
		p->using_time_flag = FLAG_NO;
		p->curr_using_time = 0;
	}

	if( (p_cfg->total_callout_count > 0 && p->curr_callout_count >= p_cfg->total_callout_count) ||
		  (p_cfg->total_callout_time > 0 && p->curr_callout_time >= p_cfg->total_callout_time*60) ||
		   (p_cfg->total_sms_count > 0 && p->curr_sms_count >= p_cfg->total_sms_count) || 
		    (p_cfg->total_using_time > 0 && p->curr_using_time >= p_cfg->total_using_time*60) ) {
		p->sim_switch_flag = FLAG_YES;
	}
	
	if( (p->sim_switch_flag == FLAG_YES) && ( p_chan->status.call_sta != CALL_DAIL && p_chan->status.call_sta != CALL_ANSWER && p_chan->status.call_sta != CALL_RING)  ) {
		res = switch_chan_simcard(p_chan, FLAG_NO);
		if(res) {
			p->sim_switch_fail_flag = FLAG_YES;
		}
	}

	return 0;
}


int set_reboot_state(void)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;
    int i = 0;
    for(i = 0; i < MAX_CHAN; i++)
    {
        p_chan[i].status.last_sta = AST_REBOOT;
    }
    return 0;
}

int reset_limit_state(void)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;
	int channum = g_call_limit.sys_info.total_chans;
	unsigned char sim_idx;
	int i = 0;

	for(i=0; i<channum; i++) {
		pthread_mutex_lock(&p_chan->lock);
		sim_idx = p_chan->sim_idx;
	    if(p_chan->status.call_mark_sta == CALLLIMIT) {
	        call_mark_set(p_chan,1);
			log_printf(LOG_INFO, "[chan_%d][sim_%d]reset call mark.\n", p_chan->id, sim_idx+1);
	    }

	    if(p_chan->status.call_lock_sta == CALLLIMIT) {
			log_printf(LOG_INFO, "[chan_%d][sim_%d]reset call lock.\n", p_chan->id, sim_idx+1);
	        call_lock_set(p_chan, 2,1);
	    }
	    
	    if(p_chan->status.sys_limit_sta == CALLLIMIT) {
	        call_limit_channel(p_chan->id,CALLLIMIT);
			log_printf(LOG_INFO, "[chan_%d][sim_%d]reset call out limited.\n", p_chan->id, sim_idx+1);
	    }
		
		if(p_chan->status.call_time_limit_sta == CALLLIMIT) {
			call_limit_channel(p_chan->id,CALLLIMIT);
			log_printf(LOG_INFO, "[chan_%d][sim_%d]reset call time limited.\n", p_chan->id, sim_idx+1);
		}
	    
	    if(p_chan->status.sms_limit_sta == SMS_LIMIT) {
	        sms_limit_channel(p_chan->id, SMS_LIMIT);
			log_printf(LOG_INFO, "[chan_%d][sim_%d]reset sms limited.\n", p_chan->id, sim_idx+1);
	    }

	    if( g_call_limit.sys_info.sys_type == CHAN_4SIM)
		    cmd_sim_register(p_chan->id, sim_idx+1, p_chan->sim[sim_idx].sim_ping_code);
	    p_chan->sim_reg_time = p_chan->cur_time;		
	    p_chan->status.last_sta = CALL_HANDUP;
	    p_chan->status.call_sta = CALL_HANDUP;
	    p_chan->status.sim_reg_sta = REGISTERING;
	    pthread_mutex_unlock(&p_chan->lock);	
	    p_chan ++;
	}
	
	return 0;
}

int reset_limit_chanel_state(int channel_id)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;
    unsigned char sim_idx;

    if((channel_id >= MAX_CHAN) || (channel_id <0)) {
        log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
        return -1;
    }

    p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);	
    sim_idx = p_chan->sim_idx;

	if(p_chan->status.call_mark_sta == CALLLIMIT) {
		call_mark_set(p_chan,1);
		log_printf(LOG_INFO, "[chan_%d][sim_%d]reset call mark.\n", p_chan->id, sim_idx+1);	
	}
	
	if(p_chan->status.call_lock_sta == CALLLIMIT) {
		call_lock_set(p_chan, 2,1);
		log_printf(LOG_INFO, "[chan_%d][sim_%d]reset call lock.\n", p_chan->id, sim_idx+1);	
	}
	
	if(p_chan->status.sys_limit_sta == CALLLIMIT) {
		call_limit_channel(p_chan->id,CALLLIMIT);
		log_printf(LOG_INFO, "[chan_%d][sim_%d]reset call out limited.\n", p_chan->id, sim_idx+1);
	}

	if(p_chan->status.call_time_limit_sta == CALLLIMIT) {
		call_limit_channel(p_chan->id,CALLLIMIT);
		log_printf(LOG_INFO, "[chan_%d][sim_%d]reset call time limited.\n", p_chan->id, sim_idx+1);
	}
	
	if(p_chan->status.sms_limit_sta == SMS_LIMIT) {
		sms_limit_channel(p_chan->id, SMS_LIMIT);
		log_printf(LOG_INFO, "[chan_%d][sim_%d]reset sms limited.\n", p_chan->id, sim_idx+1);	
	}
	
	if( g_call_limit.sys_info.sys_type == CHAN_4SIM)
		cmd_sim_register(p_chan->id, sim_idx+1, p_chan->sim[sim_idx].sim_ping_code);
	p_chan->sim_reg_time = p_chan->cur_time;		
	p_chan->status.last_sta = CALL_HANDUP;
	p_chan->status.sim_reg_sta = REGISTERING;
	pthread_mutex_unlock(&p_chan->lock);	

    return 0;
}



static int call_limit_sync_status2file(void)
{
	int i = 0;
    char *cmd;

	for(i=0; i<MAX_CHAN; i++) {
		refresh_call_limit_status_cfg(&g_call_limit, i+1);
	}

   	asprintf(&cmd,"cp -af %s %s",STATUS_FILE,STATUS_FILE_ORG);
	system(cmd);
    free(cmd);
	return 0;
}

void *reflesh_callstatus_conf_thread_cb_handler(void * data)
{
	calllimit_t *call_limit = &g_call_limit;
	struct tm cur_date, last_date;             	

	get_local_date(&last_date);
	call_limit_sync_status2file();
	
	while(1) {		
		get_local_date(&cur_date);
		if(call_limit->sys_info.IsRefleshFile != ENABLE) {
			log_printf(LOG_INFO, "stop reflesh callstatus config.\n");
			return 0;
		}
		
		if( (cur_date.tm_mday != last_date.tm_mday) &&
			 (cur_date.tm_hour == 0) && 
			  (cur_date.tm_min == 0) &&
				(cur_date.tm_sec == 0) ) {
			call_limit_sync_status2file();
			last_date = cur_date;
			log_printf(LOG_INFO, "reflesh callstatus config success!\n");
		}
		
		sleep(1);
	}
	return 0;
}

int start_reflesh_callstatus_conf(void)
{
	pthread_t tid;

	calllimit_pthread_create(&tid, NULL, reflesh_callstatus_conf_thread_cb_handler, NULL);
	return 0;	
}


void clean_sim_status(calllimit_chan_t *p_chan, unsigned char sim_idx)
{
	simcard_info_t *sim = &p_chan->sim[sim_idx];

	sim->calllimit_info.hour_cur_calls = 0;
	sim->calllimit_info.day_cur_calls = 0;
	sim->calllimit_info.day_cur_answers = 0;
	sim->calllimit_info.daycall_generation_flag = FLAG_NO;
	sim->calllimit_info.hourcall_generation_flag = FLAG_NO;
	sim->calllimit_info.call_answer_flag = FLAG_NO;
	if(sim->calllimit_info.day_call_limit_flag == FLAG_YES) {
		sim->calllimit_info.day_call_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]day call unlimit.\n", p_chan->id, sim_idx+1);
	}
	if(sim->calllimit_info.hour_call_limit_flag == FLAG_YES) {
		sim->calllimit_info.hour_call_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]hour call unlimit.\n", p_chan->id, sim_idx+1);
	}
	if(sim->calllimit_info.day_answer_limit_flag == FLAG_YES) {
		sim->calllimit_info.day_answer_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]day answer unlimit.\n", p_chan->id, sim_idx+1);
	}
	
	sim->calllock_info.call_failed_count = 0;
	sim->calllock_info.call_fail_send_sms_times = 0;
	if(sim->calllock_info.call_fail_mark_status == FLAG_YES) {
		sim->calllock_info.call_fail_mark_status = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unmark.\n", p_chan->id, sim_idx+1);
	}
	if(sim->calllock_info.call_fail_lock_status == FLAG_YES) {
		sim->calllock_info.call_fail_lock_status = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unlock.\n", p_chan->id, sim_idx+1);
	}
	
	sim->calltime_info.call_time_count = 0;
	sim->calltime_info.call_time_remain = p_chan->cfg.calltime_cfg.call_time_total;
	sim->calltime_info.call_time_redis_flag = FLAG_NO;
	if(sim->calltime_info.call_time_limit_flag == FLAG_YES) {
		sim->calltime_info.call_time_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]call time unlimit.\n", p_chan->id, sim_idx+1);
	}
	sim->calltime_info.sms_warning_flag = FLAG_NO;
	sim->calltime_info.handup_flag = FLAG_NO;
	sim->calltime_info.cond_flag = FLAG_NO;
	sim->calltime_info.call_time_warning_flag = FLAG_NO;
	return;	
}

static int Is_only_one_sim_from_channel(unsigned char sim_idx, calllimit_chan_t *p_chan)
{
	int i = 0;
	for(i=0; i<SIM_NUM; i++) {
		if(i == sim_idx) {
			if(p_chan->sim[i].sim_slot_sta == SIM_OUT_SLOT)
				return 0;
		}else {
			if(p_chan->sim[i].sim_slot_sta == SIM_IN_SLOT)
				return 0;
		}
	}
	return 1;
}

void check_chan_sim_slot_status(calllimit_chan_t *p_chan, int sim_slot[SIM_NUM])
{
	int i = 0;
	
	for(i=0; i<SIM_NUM; i++) {
		p_chan->sim[i].sim_slot_sta = sim_slot[i];
		if( (p_chan->sim[i].sim_slot_sta == SIM_OUT_SLOT) && (p_chan->sim[i].sim_slot_laststa == SIM_IN_SLOT)) {
			clean_sim_status(p_chan, i);
		}else if( (p_chan->sim[i].sim_slot_sta == SIM_IN_SLOT) && (p_chan->sim[i].sim_slot_laststa == SIM_OUT_SLOT)) {
			if(Is_only_one_sim_from_channel(i, p_chan)) {
				switch_chan_simcard(p_chan, FLAG_YES);
			}
		}
		p_chan->sim[i].sim_slot_laststa = p_chan->sim[i].sim_slot_sta;
	}
	return;
}

void check_switch_sim_status(calllimit_t *call_limit, int sim_slot_state[][SIM_NUM])
{
	int i = 0;
	unsigned char sim_idx;
	int chan_num = call_limit->sys_info.total_chans;
	calllimit_chan_t *p_chan;

	for(i=0; i<chan_num; i++) {
		p_chan =& call_limit->chans[i];
		pthread_mutex_lock(&p_chan->lock);
		sim_idx = p_chan->sim_idx;
		check_chan_sim_slot_status(p_chan, sim_slot_state[i]);		
		if( ((SIM_LIMIT == check_simcard_is_available(&p_chan->sim[sim_idx])) || (REGISTERFAIL == p_chan->status.sim_reg_sta) 
			||  (p_chan->simswitch.sim_switch_fail_flag == FLAG_YES)) && (p_chan->status.call_sta != CALL_DAIL && p_chan->status.call_sta != CALL_ANSWER && p_chan->status.call_sta != CALL_RING)) {
			switch_chan_simcard(p_chan, FLAG_NO);
		} 
		pthread_mutex_unlock(&p_chan->lock);
	}
}

	
void chan_select_simcard_init(calllimit_t *call_limit)
{
	int i = 0;
	int chan_num = call_limit->sys_info.total_chans;
	calllimit_chan_t *p_chan;
	
	for(i=0; i<chan_num; i++) {
		p_chan =& call_limit->chans[i];
		pthread_mutex_lock(&p_chan->lock);
		select_simcard_from_chan(p_chan->id, p_chan->sim_idx+1);	
		cmd_sim_register(p_chan->id, p_chan->sim_idx+1, p_chan->sim[p_chan->sim_idx].sim_ping_code);
		calllimit_redis_update_sim_slot(p_chan->id, p_chan->sim_idx+1);
//		print_debug("[%d][%d]switch to cur available sim card\n", p_chan->id, p_chan->sim_idx); 
		pthread_mutex_unlock(&p_chan->lock);
	}
}



void *check_sim_status_thread_cb_handler(void * data)
{
	calllimit_t *call_limit = &g_call_limit;    
	calllimit_chan_t *p_chan = call_limit->chans;
	int sim_slot_status[MAX_CHAN][SIM_NUM];
	int res = -1;
	
	bsp_api_init(NULL, 0);
	chan_select_simcard_init(call_limit);
	
	while(1) {		
		res = get_chan_all_sim_state(0xffff, sim_slot_status);
		if(!res)
			check_switch_sim_status(call_limit, sim_slot_status);
		sleep(2);
	}
}

static void check_sim_register_status(calllimit_chan_t *p_chan)
{
	if((SIM_UP == p_chan->status.call_sta) && (p_chan->status.sim_reg_sta != REGISTERED)) {
		p_chan->status.sim_reg_sta = REGISTERED;
//		log_printf(LOG_DEBUG, "[chan_%d][sim_%d] sim register succ.\n", p_chan->id, p_chan->sim_idx+1);
	}
	
	if((REGISTERING == p_chan->status.sim_reg_sta) && 
		(p_chan->cur_time.tv_sec - p_chan->sim_reg_time.tv_sec > p_chan->cfg.simswitch_cfg.sim_reg_timeout) ) {
		p_chan->status.sim_reg_sta = REGISTERFAIL;
//		log_printf(LOG_INFO, "[chan_%d][sim_%d] sim register fail.\n", p_chan->id, p_chan->sim_idx+1);	
	} 
	return;
}

void check_channel_sim_status(calllimit_chan_t *p_chan)
{
	unsigned char i = 0;
	unsigned char sim_idx = p_chan->sim_idx;

	pthread_mutex_lock(&p_chan->lock);
	check_sim_register_status(p_chan);
	for(i=0; i<SIM_NUM; i++) {
		if(SIM_OUT_SLOT == p_chan->sim[i].sim_slot_sta) {
			p_chan->sim[i].sim_sta = SIM_NONE;
		} else if( (SIM_IN_SLOT == p_chan->sim[i].sim_slot_sta) && (p_chan->sim_idx != i) ) {
			p_chan->sim[i].sim_sta = SIM_IDLE;
		} else if ( (SIM_IN_SLOT == p_chan->sim[i].sim_slot_sta) && (p_chan->sim_idx == i) ) {
			if(REGISTERED == p_chan->status.sim_reg_sta) {
				if( (p_chan->status.call_sta == CALL_DAIL) ||  (p_chan->status.call_sta == CALL_RING) 
					|| (p_chan->status.call_sta == CALL_ANSWER) || (p_chan->status.call_sta == CALL_IN) )
					p_chan->sim[i].sim_sta = SIM_BUSY;
				else
					p_chan->sim[i].sim_sta = SIM_FREE;
			} else if(REGISTERFAIL == p_chan->status.sim_reg_sta) {
				p_chan->sim[i].sim_sta = SIM_ERROR;
			} else if(REGISTERING == p_chan->status.sim_reg_sta) {
				p_chan->sim[i].sim_sta = SIM_REGING;
			}
		} 
	}
	pthread_mutex_unlock(&p_chan->lock);	
}

int start_check_sim_status(void)
{
	pthread_t tid;
	unsigned char sys_type = g_call_limit.sys_info.sys_type;

	if(CHAN_4SIM == sys_type) {
		calllimit_pthread_create(&tid, NULL, check_sim_status_thread_cb_handler, NULL);
	}
	return 0;	
}



void * call_limit_thread_cb_handler(void * data)
{
	int i = 0;
	int total_chans = 0;
	calllimit_t *ptr = (calllimit_t *)data;
	calllimit_chan_t *p_chans = ptr->chans;
	total_chans = ptr->sys_info.total_chans;
	calllimit_chan_t *p_chan = NULL;
	calllimit_conf_t *pcalllimit = NULL;
	calltime_conf_t *pcalltime = NULL;
	smslimit_conf_t *psmslimit = NULL;

	while(1) {
		for(i = 0; i < total_chans; i++){
			p_chan = p_chans + i;
			pcalllimit = &p_chan->cfg.calllimit_cfg;
			pcalltime = &p_chan->cfg.calltime_cfg;
			psmslimit = &p_chan->cfg.smslimit_cfg;

			get_call_limit_event(p_chan);
			get_current_datetime(p_chan);	
			check_call_lock(p_chan);

			if(ENABLE == pcalllimit->calllimit_switch) {
				check_day_call_limit(p_chan);
				check_hour_call_limit(p_chan);
				check_day_answer_limit(p_chan);
			}

			if(ENABLE == pcalltime->call_time_sw){
				check_call_time_limit(p_chan);
				check_sms_warning(p_chan);
			}else{ //call_time_switch == disable, clear redis call limit value
				if(p_chan->sim[p_chan->sim_idx].calltime_info.call_time_redis_flag ==FLAG_NO){
					calllimit_redis_dlremaintime_del(p_chan);
					p_chan->sim[p_chan->sim_idx].calltime_info.call_time_redis_flag  = FLAG_YES;
				}
			}

			if(ENABLE == pcalllimit->calllimit_switch ||ENABLE == pcalltime->call_time_sw ){
				clean_sys_call_limit_status(p_chan);
			}

			if(ENABLE == psmslimit->smslimit_switch){
				check_sms_limit(p_chan);
			}

			if(ENABLE == p_chan->cfg.simswitch_cfg.sim_switch_sw) {
				check_sim_switch(p_chan);
				if(p_chan->simswitch.reopen_switch_flag == FLAG_YES)
					p_chan->simswitch.reopen_switch_flag = FLAG_NO;
			} else {
				if(p_chan->simswitch.reopen_switch_flag == FLAG_NO) {
					clean_sim_switch_info(&p_chan->simswitch);
					p_chan->simswitch.reopen_switch_flag = FLAG_YES;
					//				print_debug("[%d][%d]clean_sim_switch_info\n", p_chan->id, p_chan->sim_idx); 
				}
			}

			check_channel_sim_status(p_chan);
		}
		usleep(REFRESH_TIMEUS);
	}

	return NULL;
}


//int call_limit_thread_create(calllimit_chan_t *p_chan)
int call_limit_thread_create(calllimit_t *calllimit)
{
#if 0
	int i = 0;
	int total_chans = calllimit->sys_info.total_chans;
	calllimit_chan_t *p_chan = calllimit->chans;
	for(i=0; i< total_chans; i++) {
		if(!p_chan) {
			log_printf(LOG_ERROR, " call_limit_thread_create p_call_limit is null.\n");
			break;
		}
		calllimit_pthread_create(&p_chan->tid, NULL, call_limit_thread_cb_handler, p_chan);	
		p_chan++;
	}
#endif
	calllimit_pthread_create(&calllimit->tid, NULL, call_limit_thread_cb_handler, calllimit);
	return 0;	
}


static int call_limit_get_channel_settings(int channel_id, calllimit_settings_t *settings)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if(!settings) {
		log_printf(LOG_ERROR, "[%d]settings is null\n", channel_id);
		return -1;		
	}
	
	p_chan += channel_id;
	calllock_conf_t *pcalllock_cfg = &p_chan->cfg.calllock_cfg;	
	calllimit_conf_t *pcalllimit_cfg = &p_chan->cfg.calllimit_cfg;   
	calltime_conf_t *pcalltime_cfg = &p_chan->cfg.calltime_cfg;	 
	smslimit_conf_t *psmslimit_cfg = &p_chan->cfg.smslimit_cfg;	  

	pthread_mutex_lock(&p_chan->lock);
	settings->call_time_sw = pcalltime_cfg->call_time_sw;
	settings->call_time_single_sw = pcalltime_cfg->call_time_single_sw;
	settings->call_time_settings = pcalltime_cfg->call_time_settings;
	settings->call_time_total_sw = pcalltime_cfg->call_time_total_sw;
	settings->call_time_total = pcalltime_cfg->call_time_total;
	settings->call_time_clean_sw = pcalltime_cfg->call_time_clean_sw;
	settings->call_time_free = pcalltime_cfg->call_time_free;
	settings->call_time_step = pcalltime_cfg->call_time_step;
	settings->call_time_clean_type= pcalltime_cfg->call_time_clean_type;

	settings->day_answer_setting =  pcalllimit_cfg->day_answer_setting;
	settings->day_calls_settings = pcalllimit_cfg->day_calls_settings;
	settings->hour_calls_settings = pcalllimit_cfg->hour_calls_settings;
	settings->calllimit_switch = pcalllimit_cfg->calllimit_switch;
	
    settings->call_fail_mark_count = pcalllock_cfg->call_fail_mark_count;
    settings->call_fail_lock_count = pcalllock_cfg->call_fail_lock_count;
    settings->call_detect_flag = pcalllock_cfg->call_detect_flag;
    settings->call_fail_lock_flag = pcalllock_cfg->call_fail_lock_flag;
    settings->call_fail_mark_flag = pcalllock_cfg->call_fail_mark_flag;
    settings->call_fail_lock_sms_flag = pcalllock_cfg->call_fail_lock_sms_flag;
    settings->call_fail_lock_sms_count = pcalllock_cfg->call_fail_lock_sms_count;
    settings->call_fail_lock_sms_report_flag = pcalllock_cfg->call_fail_lock_sms_report_flag;
    memcpy(settings->call_fail_lock_sms_callee,pcalllock_cfg->call_fail_lock_sms_callee,sizeof(pcalllock_cfg->call_fail_lock_sms_callee));
    memcpy(settings->call_fail_lock_sms_msg,pcalllock_cfg->call_fail_lock_sms_msg,sizeof(pcalllock_cfg->call_fail_lock_sms_msg));
	settings->smslimit_switch = psmslimit_cfg->smslimit_switch;
	settings->day_sms_settings  = psmslimit_cfg->day_sms_settings;
	settings->mon_sms_settings = psmslimit_cfg->mon_sms_settings;
	settings->sms_clean_date = psmslimit_cfg->sms_clean_date;

	settings->sms_warning_switch = psmslimit_cfg->sms_warning_switch;
	settings->smslimit_mon_warning_num = psmslimit_cfg->smslimit_mon_warning_num;
	strcpy(settings->smslimit_mon_warning_callee , psmslimit_cfg->smslimit_mon_warning_callee);
	strcpy(settings->smslimit_mon_warning_msg, psmslimit_cfg->smslimit_mon_warning_msg);
	settings->call_time_warning_num = pcalltime_cfg->call_time_warning_num;
	strcpy(settings->call_time_warning_callee, pcalltime_cfg->call_time_warning_callee);
	strcpy(settings->call_time_warning_msg, pcalltime_cfg->call_time_warning_msg);

	settings->sim_policy = p_chan->cfg.simswitch_cfg.sim_policy;
	settings->sim_switch_sw = p_chan->cfg.simswitch_cfg.sim_switch_sw;
	settings->sim_reg_timeout = p_chan->cfg.simswitch_cfg.sim_reg_timeout;
	settings->total_callout_count = p_chan->cfg.simswitch_cfg.total_callout_count;
	settings->total_callout_time = p_chan->cfg.simswitch_cfg.total_callout_time;
	settings->total_sms_count = p_chan->cfg.simswitch_cfg.total_sms_count;
	settings->total_using_time = p_chan->cfg.simswitch_cfg.total_using_time;
	
	pthread_mutex_unlock(&p_chan->lock);	
	
	return 0;
}

static int call_limit_get_channel_status(int channel_id, calllimit_status_t *status)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;
	char tmp[64];
	
	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if(!status) {
		log_printf(LOG_ERROR, "[%d]status is null\n", channel_id);
		return -1;		
	}
	
	p_chan += channel_id;
	unsigned char sim_idx = p_chan->sim_idx;
	calllock_info_t *pcalllock_info = &p_chan->sim[sim_idx].calllock_info;	  
	calllimit_info_t *pcalllimit_info = &p_chan->sim[sim_idx].calllimit_info;	
	calltime_info_t *pcalltime_info = &p_chan->sim[sim_idx].calltime_info;		  
	smslimit_info_t *psmslimit_info = &p_chan->sim[sim_idx].smslimit_info;	

//	print_debug("[%d]call_limit_get_channel_status\n", channel_id); 
	
	pthread_mutex_lock(&p_chan->lock);	
	status->sim_idx = p_chan->sim_idx;
	status->call_sta = p_chan->status.call_sta;
	status->call_time_count = pcalltime_info->call_time_count;
	status->call_time_remain = pcalltime_info->call_time_remain;
	if(p_chan->cfg.calltime_cfg.call_time_clean_sw == FLAG_NO){
		time_t now;
		struct tm *tmp_tm;
		time(&now);
		tmp_tm = localtime(&now);	
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", tmp_tm);
		strcpy(status->call_time_clean_date, tmp);
	}else{
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &pcalltime_info->call_time_clean_date);
		strcpy(status->call_time_clean_date, tmp);
	}
	status->call_time_limit_flag = pcalltime_info->call_time_limit_flag;
	status->call_time_limit_sta = p_chan->status.call_time_limit_sta;
	status->call_answer_flag = pcalltime_info->call_answer_flag;
	status->call_generation_flag = pcalllimit_info->daycall_generation_flag;
	status->day_answer_limit_flag = pcalllimit_info->day_answer_limit_flag;
	status->day_call_limit_flag = pcalllimit_info->day_call_limit_flag;
	status->day_cur_answers = pcalllimit_info->day_cur_answers;
	status->day_cur_calls = pcalllimit_info->day_cur_calls;
	status->hour_call_limit_flag = pcalllimit_info->hour_call_limit_flag;
	status->hour_cur_calls = pcalllimit_info->hour_cur_calls;
	status->sys_limit_sta = p_chan->status.sys_limit_sta;
	status->call_fail_mark_status = pcalllock_info->call_fail_mark_status;
	status->call_fail_lock_status = pcalllock_info->call_fail_lock_status;
	status->call_failed_count = pcalllock_info->call_failed_count;
	status->call_fail_send_sms_times =  pcalllock_info->call_fail_send_sms_times;
	status->day_cur_sms = psmslimit_info->day_cur_sms;
	status->mon_cur_sms = psmslimit_info->mon_cur_sms;
	status->sms_limit_sta = p_chan->status.sms_limit_sta;
	status->curr_callout_count = p_chan->simswitch.curr_callout_count;
	status->curr_callout_time = p_chan->simswitch.curr_callout_time;
	status->curr_sms_count = p_chan->simswitch.curr_sms_count;
	status->curr_using_time = p_chan->simswitch.curr_using_time;
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}


static int call_limit_get_channel_sim_status(int channel_id, int sim_idx, calllimit_status_t *status)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;
	char tmp[64];
	
	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx > SIM_NUM) || (sim_idx <0)) {
		log_printf(LOG_ERROR, "[%d]sim_idx out of range\n", sim_idx);
		return -1;
	}	

	if(!status) {
		log_printf(LOG_ERROR, "[%d][%d]status is null\n", channel_id, sim_idx);
		return -1;		
	}
	
	p_chan += channel_id;

	calllock_info_t *pcalllock_info = &p_chan->sim[sim_idx].calllock_info;	  
	calllimit_info_t *pcalllimit_info = &p_chan->sim[sim_idx].calllimit_info;	
	calltime_info_t *pcalltime_info = &p_chan->sim[sim_idx].calltime_info;		  
	smslimit_info_t *psmslimit_info = &p_chan->sim[sim_idx].smslimit_info;		   

//	print_debug("%s --new-get chan_%d sim_%d day_cur_sms=%d., status=0x%x\n", __FUNCTION__, channel_id+1, sim_idx+1, psmslimit_info->day_cur_sms, status);	
	pthread_mutex_lock(&p_chan->lock);	
	status->sim_idx = p_chan->sim_idx;
	status->sim_sta = p_chan->sim[sim_idx].sim_sta;
	status->call_sta = p_chan->status.call_sta;
	status->call_time_count = pcalltime_info->call_time_count;
	status->call_time_remain = pcalltime_info->call_time_remain;
	if(p_chan->cfg.calltime_cfg.call_time_clean_sw == FLAG_NO){
		time_t now;
		struct tm *tmp_tm;
		time(&now);
		tmp_tm = localtime(&now);	
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", tmp_tm);
		strcpy(status->call_time_clean_date, tmp);
	}else{
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &pcalltime_info->call_time_clean_date);
		strcpy(status->call_time_clean_date, tmp);
	}
	status->call_time_limit_flag = pcalltime_info->call_time_limit_flag;
	status->call_time_limit_sta = (pcalltime_info->call_time_limit_flag==FLAG_YES)?CALLLIMIT:UNLIMIT;
	status->call_answer_flag = pcalltime_info->call_answer_flag;
	status->call_generation_flag = pcalllimit_info->daycall_generation_flag;
	status->day_answer_limit_flag = pcalllimit_info->day_answer_limit_flag;
	status->day_call_limit_flag = pcalllimit_info->day_call_limit_flag;
	status->day_cur_answers = pcalllimit_info->day_cur_answers;
	status->day_cur_calls = pcalllimit_info->day_cur_calls;
	status->hour_call_limit_flag = pcalllimit_info->hour_call_limit_flag;
	status->hour_cur_calls = pcalllimit_info->hour_cur_calls;
	if( (pcalllimit_info->day_call_limit_flag == FLAG_YES) || 
		 (pcalllimit_info->day_answer_limit_flag == FLAG_YES) ||
		  (pcalllimit_info->hour_call_limit_flag == FLAG_YES)) {
		status->sys_limit_sta = CALLLIMIT;
	} else {
		status->sys_limit_sta = UNLIMIT;
	}
	status->call_fail_mark_status = pcalllock_info->call_fail_mark_status;
	status->call_fail_lock_status = pcalllock_info->call_fail_lock_status;
	status->call_failed_count = pcalllock_info->call_failed_count;
	status->call_fail_send_sms_times =  pcalllock_info->call_fail_send_sms_times;
	status->day_cur_sms = psmslimit_info->day_cur_sms;
	status->mon_cur_sms = psmslimit_info->mon_cur_sms;
	if( (psmslimit_info->day_sms_limit_flag == FLAG_YES) || (psmslimit_info->mon_sms_limit_flag== FLAG_YES))
		status->sms_limit_sta = SIM_LIMIT;
	else
		status->sms_limit_sta = SIM_UNLIMIT;

	if(sim_idx == p_chan->sim_idx) {
		status->curr_callout_count = p_chan->simswitch.curr_callout_count;
		status->curr_callout_time = p_chan->simswitch.curr_callout_time;
		status->curr_sms_count = p_chan->simswitch.curr_sms_count;
		status->curr_using_time = p_chan->simswitch.curr_using_time;
	} else {
		status->curr_callout_count = 0;
		status->curr_callout_time = 0;
		status->curr_sms_count = 0;
		status->curr_using_time = 0;		
	}
	
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}


static int call_limit_get_all_status(calllimit_getall_status_t *status)
{
	int i = 0;
	int j = 0;
	unsigned int total_chans = g_call_limit.sys_info.total_chans;

	if(!status) {
		log_printf(LOG_ERROR, "get_all_status is null\n");
		return -1;				
	}

	for(i=0; i<total_chans; i++) {
		for(j=0; j<SIM_NUM; j++)	
			call_limit_get_channel_sim_status(i, j, (calllimit_status_t *)status->status.__ptr+(i*SIM_NUM+j));
	}
	status->result = total_chans;
	return total_chans;
}


static int call_limit_set_channel_calltime(int channel_id, unsigned int calltime)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((calltime < 0) || (calltime > 40000)) {
		log_printf(LOG_ERROR, "calltime(%d) out of range\n", calltime);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calltime_cfg.call_time_settings = calltime;
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}


static int call_limit_set_channel_daycalls(int channel_id, unsigned int daycalls)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((daycalls < 0) || (daycalls > 100000)) {
		log_printf(LOG_ERROR, "daycalls(%d) out of range\n", daycalls);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllimit_cfg.day_calls_settings = daycalls;
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}


static int call_limit_set_channel_dayanswers(int channel_id, unsigned int dayanswers)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((dayanswers < 0) || (dayanswers > 100000)) {
		log_printf(LOG_ERROR, "dayanswers(%d) out of range\n", dayanswers);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllimit_cfg.day_answer_setting = dayanswers;
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}


static int call_limit_set_channel_hourcalls(int channel_id, unsigned int hourcalls)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((hourcalls < 0) || (hourcalls > 1000)) {
		log_printf(LOG_ERROR, "hourcalls(%d) out of range\n", hourcalls);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllimit_cfg.hour_calls_settings = hourcalls;
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}


static int call_limit_set_channel_switch(int channel_id, unsigned int call_switch)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((call_switch < 0) || (call_switch > 1)) {
		log_printf(LOG_ERROR, "call_switch(%d) out of range\n", call_switch);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllimit_cfg.calllimit_switch = call_switch;
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}

static int call_limit_set_filewrite_switch(unsigned int file_switch)
{
	calllimit_t *call_limit = &g_call_limit;
	unsigned int last_file_switch = call_limit->sys_info.IsRefleshFile;

	if((file_switch < 0) || (file_switch > 1)) {
		log_printf(LOG_ERROR, "value(%d) out of range\n", file_switch);
		return -1;		
	}
	
	pthread_mutex_lock(&call_limit->lock);
	call_limit->sys_info.IsRefleshFile = file_switch;
	pthread_mutex_unlock(&call_limit->lock);	

	if( (last_file_switch != file_switch) && (ENABLE == file_switch)) {
		start_reflesh_callstatus_conf();
	}

	return 0;
}


static int call_limit_clean_channel_day_calls(int channel_id)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->sim[SIM1].calllimit_info.day_cur_calls = 0;
	if(p_chan->sim[SIM1].calllimit_info.day_call_limit_flag == FLAG_YES) {
		p_chan->sim[SIM1].calllimit_info.day_call_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]day call unlimit.\n", p_chan->id, p_chan->sim_idx+1);
	}
//	refresh_call_limit_status_cfg(&g_call_limit, p_call_limit->id);	
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}

static int call_limit_clean_channel_day_answers(int channel_id)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->sim[SIM1].calllimit_info.day_cur_answers = 0;
	if(p_chan->sim[SIM1].calllimit_info.day_answer_limit_flag == FLAG_YES) {
		p_chan->sim[SIM1].calllimit_info.day_answer_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]day answer unlimit.\n", p_chan->id, p_chan->sim_idx+1);
	}
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}

static int call_limit_clean_channel_hour_calls(int channel_id)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->sim[SIM1].calllimit_info.hour_cur_calls = 0;
	if(p_chan->sim[SIM1].calllimit_info.hour_call_limit_flag == FLAG_YES) {
		p_chan->sim[SIM1].calllimit_info.hour_call_limit_flag = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]hour call unlimit.\n", p_chan->id, p_chan->sim_idx+1);
	}
	pthread_mutex_unlock(&p_chan->lock);	

	return 0;
}

static int call_limit_reload( )
{

    reload_call_limit_cfg(&g_call_limit);
    
    return 0;
}

static int call_limit_reflesh_status(void)
{
	call_limit_sync_status2file();
	return 0;
}


static int call_limit_set_channel_unmark(int channel_id)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);  

	if(p_chan->sim[SIM1].calllock_info.call_fail_lock_status != FLAG_NO)
	{
	    log_printf(LOG_ERROR, "[%d]channel status can not unmark!\n", channel_id);
		pthread_mutex_unlock(&p_chan->lock);
	    return -1;
	}

	if(p_chan->sim[SIM1].calllock_info.call_fail_mark_status == FLAG_YES) {	   
		p_chan->sim[SIM1].calllock_info.call_fail_mark_status = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unmark.\n", p_chan->id, p_chan->sim_idx+1);
	}
	p_chan->sim[SIM1].calllock_info.call_fail_send_sms_times = 0;
	p_chan->sim[SIM1].calllock_info.call_failed_count = 0;
	p_chan->status.call_mark_sta = UNLIMIT;

	char *unmark_cmd = NULL;
	asprintf(&unmark_cmd,SET_CALLMARK,p_chan->id,FLAG_NO);
	system(unmark_cmd);
	free(unmark_cmd);

//	print_debug("Manual set call unmark!\n");
	pthread_mutex_unlock(&p_chan->lock);
	
	return 0;   
}

static int call_limit_set_channel_unlock(int channel_id)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);  
#if 0
	p_chan->sim[SIM1].calllock_info.call_fail_lock_status = FLAG_NO;
	p_chan->sim[SIM1].calllock_info.call_fail_mark_status = FLAG_NO;
	p_chan->sim[SIM1].calllock_info.call_fail_send_sms_times = 0;
	p_chan->sim[SIM1].calllock_info.call_failed_count = 0;
	p_chan->status.call_lock_sta = UNLIMIT;
	p_chan->status.call_mark_sta = UNLIMIT;
	
	char *unlock_cmd = NULL;
	asprintf(&unlock_cmd,SET_CALLLOCK,p_chan->id,FLAG_NO);
	system(unlock_cmd);
	free(unlock_cmd);

	char *unmark_cmd = NULL;
	asprintf(&unmark_cmd,SET_CALLMARK,p_chan->id,FLAG_NO);
	system(unmark_cmd);
	free(unmark_cmd);
#endif
	call_lock_clean(p_chan);
	call_mark_clean(p_chan);
	pthread_mutex_unlock(&p_chan->lock);
	return 0;

}

static int call_limit_set_channel_check(int channel_id,  unsigned int check_flag)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((check_flag < 0) || (check_flag > 1)) {
		log_printf(LOG_ERROR, "check_flag(%d) out of range\n", check_flag);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllock_cfg.call_detect_flag = check_flag;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;

}

static int call_limit_set_mark_flag(int channel_id,  unsigned int failed_flag)
{
   	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((failed_flag < 0) || (failed_flag > 1)) {
		log_printf(LOG_ERROR, "check_flag(%d) out of range\n", failed_flag);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllock_cfg.call_fail_mark_flag = failed_flag;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_lock_flag(int channel_id,  unsigned int lock_flag)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((lock_flag < 0) || (lock_flag > 1)) {
		log_printf(LOG_ERROR, "check_flag(%d) out of range\n", lock_flag);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllock_cfg.call_fail_lock_flag = lock_flag;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_mark_count(int channel_id,  unsigned int count)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllock_cfg.call_fail_mark_count = count;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_lock_count(int channel_id,  unsigned int count)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllock_cfg.call_fail_lock_count = count;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_sms_flag(int channel_id,  unsigned int sms_flag)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sms_flag < 0) || (sms_flag > 1)) {
		log_printf(LOG_ERROR, "check_flag(%d) out of range\n", sms_flag);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllock_cfg.call_fail_lock_sms_flag = sms_flag;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_sms_count(int channel_id,  unsigned int count)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllock_cfg.call_fail_lock_sms_count = count;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_sms_report_flag(int channel_id,  unsigned int report_flag)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((report_flag < 0) || (report_flag > 1)) {
		log_printf(LOG_ERROR, "report_flag(%d) out of range\n", report_flag);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	p_chan->cfg.calllock_cfg.call_fail_lock_sms_report_flag = report_flag;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_sms_sender(int channel_id, char* addr)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((addr == NULL) || (strlen(addr) > 64)) {
		log_printf(LOG_ERROR, "addr(%s) out of range\n", addr);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	memcpy(p_chan->cfg.calllock_cfg.call_fail_lock_sms_callee, addr,strlen(addr));
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_sms_msg(int channel_id,  char* msg)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((msg == NULL) || (strlen(msg) > WARNING_MSG_LEN)) {
		log_printf(LOG_ERROR, "msg(%s) out of range\n", msg);
		return -1;		
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	memcpy(p_chan->cfg.calllock_cfg.call_fail_lock_sms_callee, msg,strlen(msg));
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}


static int call_limit_clean_channel_limit(int channel_id)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;

	pthread_mutex_lock(&p_chan->lock);	
	clean_all_call_limit(p_chan, p_chan->sim_idx);
	if(p_chan->status.sys_limit_sta == CALLLIMIT) {
		p_chan->status.sys_limit_sta = UNLIMIT;
		if(p_chan->status.call_time_limit_sta == UNLIMIT) {
			call_limit_channel(p_chan->id, UNLIMIT);
		}
	}
//	refresh_call_limit_status_cfg(&g_call_limit, p_call_limit->id);	
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_clena_channel_sms_limit(int channel_id){
	int chan_id = channel_id & ALL_CHANNEL; 
	int start_id, end_id, i;
	
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if(is_valid_channel_id(chan_id) != 0){
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	if(chan_id == ALL_CHANNEL){
		start_id = 0;
		end_id = g_call_limit.sys_info.total_chans;
	}else{
		start_id = chan_id;
		end_id = chan_id+1;
	}
	for(i = start_id; i < end_id; i++){
		p_chan += i;
		if(p_chan->status.sms_limit_sta == SMS_LIMIT) {
			p_chan->status.sms_limit_sta = SMS_UNLIMIT;
			sms_limit_channel(p_chan->id, SMS_UNLIMIT);
			log_printf(LOG_INFO, "[chan_%d][sim_%d]send sms unlimit.\n", p_chan->id, p_chan->sim_idx+1);
		}
	}
//	refresh_call_limit_status_cfg(&g_call_limit, p_call_limit->id);	
	return 0;
}

static int call_limit_set_day_sms_limit_settings(int channel_id, unsigned int sms_settings){
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);
	p_chan->cfg.smslimit_cfg.day_sms_settings = sms_settings;
	pthread_mutex_unlock(&p_chan->lock);
	return 0;
}

static int call_limit_set_mon_sms_limit_settings(int channel_id, unsigned int sms_settings){
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);
	p_chan->cfg.smslimit_cfg.mon_sms_settings = sms_settings;
	pthread_mutex_unlock(&p_chan->lock);
	return 0;
}

static int call_limit_set_sms_limit_switch(int channel_id, unsigned int sms_switch){
	int start_id, end_id, i;
	int chan_id = channel_id & ALL_CHANNEL;
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if(is_valid_channel_id(chan_id)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	
	if(chan_id == ALL_CHANNEL){
		start_id = 0;
		end_id = g_call_limit.sys_info.total_chans;
	}else{
		start_id = chan_id;
		end_id = chan_id+1;
	}
	
	for(i = start_id; i < end_id; i++){
		p_chan = &g_call_limit.chans[i];
		pthread_mutex_lock(&p_chan->lock);
		p_chan->cfg.smslimit_cfg.smslimit_switch= sms_switch;
		pthread_mutex_unlock(&p_chan->lock);
	}
	return 0;
}

static int call_limit_set_sms_limit_success_flag(int channel_id, unsigned int flag){
	int start_id, end_id, i;
	int chan_id = channel_id & ALL_CHANNEL;
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if(is_valid_channel_id(chan_id)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	if(chan_id == ALL_CHANNEL){
		start_id = 0;
		end_id = g_call_limit.sys_info.total_chans;
	}else{
		start_id = chan_id;
		end_id = chan_id+1;
	}
	for(i = start_id; i < end_id; i++){
		p_chan = &g_call_limit.chans[i];
		pthread_mutex_lock(&p_chan->lock);
		p_chan->cfg.smslimit_cfg.smslimit_success_flag= flag;
		pthread_mutex_unlock(&p_chan->lock);
	}
	return 0;
}

static int call_limit_set_day_sms_cnt(int channel_id, unsigned int cur_sms){
	int start_id, end_id, i;
	int chan_id = channel_id & ALL_CHANNEL;
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if(is_valid_channel_id(chan_id)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	if(chan_id == ALL_CHANNEL){
		start_id = 0;
		end_id = g_call_limit.sys_info.total_chans;
	}else{
		start_id = chan_id;
		end_id = chan_id+1;
	}
	for(i = start_id; i < end_id; i++){
		p_chan = &g_call_limit.chans[i];
		pthread_mutex_lock(&p_chan->lock);
		p_chan->sim[SIM1].smslimit_info.day_cur_sms = cur_sms;
		if(cur_sms == 0){
			if(p_chan->sim[SIM1].smslimit_info.day_sms_limit_flag == SMS_LIMIT) {
				p_chan->sim[SIM1].smslimit_info.day_sms_limit_flag = SMS_UNLIMIT;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]day sms unlimit.\n", p_chan->id, p_chan->sim_idx+1);
			}
			if(p_chan->sim[SIM1].smslimit_info.mon_sms_limit_flag == SMS_UNLIMIT) {
				p_chan->status.sms_limit_sta = SMS_UNLIMIT;
				sms_limit_channel(p_chan->id, SMS_UNLIMIT);
			}
		}
		pthread_mutex_unlock(&p_chan->lock);
	}
	return 0;
}

static int call_limit_set_mon_sms_cnt(int channel_id, unsigned int cur_sms){
	int start_id, end_id, i;
	int chan_id = channel_id & ALL_CHANNEL;
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if(is_valid_channel_id(chan_id)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	if(chan_id == ALL_CHANNEL){
		start_id = 0;
		end_id = g_call_limit.sys_info.total_chans;
	}else{
		start_id = chan_id;
		end_id = chan_id+1;
	}
	for(i = start_id; i < end_id; i++){
		p_chan = &g_call_limit.chans[i];
		pthread_mutex_lock(&p_chan->lock);
		p_chan->sim[SIM1].smslimit_info.mon_cur_sms = cur_sms;
		if(cur_sms == 0){
			p_chan->sim[SIM1].smslimit_info.day_cur_sms = cur_sms;
			if(p_chan->sim[SIM1].smslimit_info.mon_sms_limit_flag == SMS_LIMIT) {
				p_chan->sim[SIM1].smslimit_info.mon_sms_limit_flag = SMS_UNLIMIT;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]mon sms unlimit.\n", p_chan->id, p_chan->sim_idx+1);
			}
			if(p_chan->sim[SIM1].smslimit_info.day_sms_limit_flag == SMS_LIMIT) {
				p_chan->sim[SIM1].smslimit_info.day_sms_limit_flag = SMS_UNLIMIT;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]day sms unlimit.\n", p_chan->id, p_chan->sim_idx+1);
			}
			p_chan->status.sms_limit_sta = SMS_UNLIMIT;
			sms_limit_channel(p_chan->id, SMS_UNLIMIT);
		}
		pthread_mutex_unlock(&p_chan->lock);
	}
	return 0;
}

static int call_limit_set_call_time_cnt(int channel_id, unsigned int call_time_cnt){
	int start_id, end_id, i;
	int chan_id = channel_id & ALL_CHANNEL;
	unsigned char sim_idx = 0;
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if(is_valid_channel_id(chan_id)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	if(chan_id == ALL_CHANNEL){
		start_id = 0;
		end_id = g_call_limit.sys_info.total_chans;
	}else{
		start_id = chan_id;
		end_id = chan_id+1;
	}
	for(i = start_id; i < end_id; i++){
		p_chan = &g_call_limit.chans[i];
		sim_idx = p_chan->sim_idx;
		pthread_mutex_lock(&p_chan->lock);
		p_chan->sim[sim_idx].calltime_info.call_time_count = call_time_cnt;
		p_chan->sim[sim_idx].calltime_info.call_time_remain = p_chan->cfg.calltime_cfg.call_time_total - call_time_cnt;
		p_chan->sim[sim_idx].calltime_info.call_time_redis_flag = FLAG_NO;
		if(call_time_cnt == 0){
			if(p_chan->sim[sim_idx].calltime_info.call_time_limit_flag == FLAG_YES) {
				p_chan->sim[sim_idx].calltime_info.call_time_limit_flag = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]call time unlimit.\n", p_chan->id, sim_idx+1);
			}
			p_chan->sim[sim_idx].calltime_info.sms_warning_flag = FLAG_NO;
		}
		pthread_mutex_unlock(&p_chan->lock);
	}
	return 0;
}

static int call_limit_set_call_time_total(int channel_id, unsigned int total){
	int start_id, end_id, i;
	int chan_id = channel_id & ALL_CHANNEL;
	calllimit_chan_t *p_chan = g_call_limit.chans;
	
	if(is_valid_channel_id(chan_id)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	if(chan_id == ALL_CHANNEL){
		start_id = 0;
		end_id = g_call_limit.sys_info.total_chans;
	}else{
		start_id = chan_id;
		end_id = chan_id+1;
	}
	for(i = start_id; i < end_id; i++){
		p_chan = &g_call_limit.chans[i];
		pthread_mutex_lock(&p_chan->lock);
		p_chan->sim[SIM1].calltime_info.call_time_count = total;
		p_chan->sim[SIM1].calltime_info.call_time_remain = p_chan->cfg.calltime_cfg.call_time_total - total;
		
		if(p_chan->sim[SIM1].calltime_info.call_time_remain <= 0){
			p_chan->sim[SIM1].calltime_info.call_time_remain = 0;
			p_chan->sim[SIM1].calltime_info.call_time_limit_flag = FLAG_YES;
			log_printf(LOG_INFO, "[chan_%d][sim_%d]call time limited.\n", p_chan->id, p_chan->sim_idx+1);
		}
		if(total == 0){
			p_chan->sim[SIM1].calltime_info.sms_warning_flag = FLAG_NO;
			if(p_chan->sim[SIM1].calltime_info.call_time_limit_flag == FLAG_YES) {
				p_chan->sim[SIM1].calltime_info.call_time_limit_flag = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]call time unlimit.\n", p_chan->id, p_chan->sim_idx+1);
			}
		}
		p_chan->sim[SIM1].calltime_info.call_time_redis_flag = FLAG_NO;
		pthread_mutex_unlock(&p_chan->lock);
	}
	return 0;
}


static int call_limit_set_sim_limitsta(int channel_id, int sim_idx, unsigned int calllimit_flag, unsigned int calllock_flag, unsigned int callmark_flag, unsigned int smslimit_flag)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_limitsta\n", channel_id, sim_idx);
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);	
	p_chan->sim[sim_idx].calllimit_info.day_call_limit_flag = calllimit_flag;
	p_chan->sim[sim_idx].calllimit_info.day_answer_limit_flag = calllimit_flag;
	p_chan->sim[sim_idx].calllimit_info.hour_call_limit_flag = calllimit_flag;
	p_chan->sim[sim_idx].calltime_info.call_time_limit_flag = calllimit_flag;
	p_chan->sim[sim_idx].calllock_info.call_fail_lock_status = calllock_flag;
	p_chan->sim[sim_idx].calllock_info.call_fail_mark_status = callmark_flag;
	p_chan->sim[sim_idx].smslimit_info.day_sms_limit_flag = smslimit_flag;
	p_chan->sim[sim_idx].smslimit_info.mon_sms_limit_flag = smslimit_flag;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;	
}

static int call_limit_set_sim_unlimit(int channel_id, int sim_idx)
{
	calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_unlimit\n", channel_id, sim_idx);
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);	
	clean_all_call_limit(p_chan, sim_idx);
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;	
}

static int call_limit_set_sim_unlock(int channel_id, int sim_idx)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_unlock\n", channel_id, sim_idx);
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  
	if(p_chan->sim[sim_idx].calllock_info.call_fail_lock_status == FLAG_YES) {
		p_chan->sim[sim_idx].calllock_info.call_fail_lock_status = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unlock.\n", p_chan->id, sim_idx+1);
	}
	if(p_chan->sim[sim_idx].calllock_info.call_fail_mark_status == FLAG_YES) {
		p_chan->sim[sim_idx].calllock_info.call_fail_mark_status = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unmark.\n", p_chan->id, sim_idx+1);
	}
	p_chan->sim[sim_idx].calllock_info.call_fail_send_sms_times = 0;
	p_chan->sim[sim_idx].calllock_info.call_failed_count = 0;

	pthread_mutex_unlock(&p_chan->lock);
	return 0;

}

static int call_limit_set_sim_unmark(int channel_id, int sim_idx)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_unmark\n", channel_id, sim_idx);
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  

	if(p_chan->sim[sim_idx].calllock_info.call_fail_lock_status != FLAG_NO)
	{
	    log_printf(LOG_ERROR, "[%d][%d]channel sim_idx status can not unmark!\n", channel_id, sim_idx+1);
		pthread_mutex_unlock(&p_chan->lock);
	    return -1;
	}

	if(p_chan->sim[sim_idx].calllock_info.call_fail_mark_status == FLAG_YES) {
		p_chan->sim[sim_idx].calllock_info.call_fail_mark_status = FLAG_NO;
		log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unmark.\n", p_chan->id, sim_idx+1);
	}
	p_chan->sim[sim_idx].calllock_info.call_fail_send_sms_times = 0;
	p_chan->sim[sim_idx].calllock_info.call_failed_count = 0;
	
	pthread_mutex_unlock(&p_chan->lock);
	
	return 0;   
}

static int call_limit_all_simcard_unlimit(void)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;
	int channum = g_call_limit.sys_info.total_chans;
	int i = 0, j = 0;
	
//	print_debug("call_limit_all_simcard_unlimit\n");

	for(i=0; i<channum; i++) {	
		pthread_mutex_lock(&p_chan->lock);  
		for(j=0; j<SIM_NUM; j++) {
			clean_all_call_limit(p_chan, j);
			p_chan->sim[j].calltime_info.call_time_count = 0;
			p_chan->sim[j].calltime_info.call_time_remain = p_chan->cfg.calltime_cfg.call_time_total;
			p_chan->sim[j].calltime_info.call_time_redis_flag = FLAG_NO;
			if(p_chan->sim[j].calltime_info.call_time_limit_flag == FLAG_YES) {
				p_chan->sim[j].calltime_info.call_time_limit_flag = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]call time unlimit.\n", p_chan->id, j+1);
			}
			p_chan->sim[j].calltime_info.sms_warning_flag = FLAG_NO;
		}
		pthread_mutex_unlock(&p_chan->lock);
		p_chan ++;
	}
	return 0;   
}


static int call_limit_all_simcard_unlock(void)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;
	int channum = g_call_limit.sys_info.total_chans;
	int i = 0, j = 0;
	
//	print_debug("call_limit_all_simcard_unlock\n");

	for(i=0; i<channum; i++) {	
		pthread_mutex_lock(&p_chan->lock);  
		for(j=0; j<SIM_NUM; j++) {
			if(p_chan->sim[j].calllock_info.call_fail_lock_status == FLAG_YES) {
				p_chan->sim[j].calllock_info.call_fail_lock_status = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unlock.\n", p_chan->id, j+1);
			}
			if(p_chan->sim[j].calllock_info.call_fail_mark_status == FLAG_YES) {
				p_chan->sim[j].calllock_info.call_fail_mark_status = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unmark.\n", p_chan->id, j+1);
			}
			p_chan->sim[j].calllock_info.call_fail_send_sms_times = 0;
			p_chan->sim[j].calllock_info.call_failed_count = 0;	
		}
		pthread_mutex_unlock(&p_chan->lock);
		p_chan ++;
	}
	return 0;   
}


static int call_limit_all_simcard_unmark(void)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;
	int channum = g_call_limit.sys_info.total_chans;
	int i = 0, j = 0;
	
//	print_debug("call_limit_all_simcard_unmark\n");

	for(i=0; i<channum; i++) {	
		pthread_mutex_lock(&p_chan->lock);  
		for(j=0; j<SIM_NUM; j++) {
			if(p_chan->sim[j].calllock_info.call_fail_lock_status != FLAG_NO) {
				log_printf(LOG_ERROR, "[%d][%d]channel sim_idx status can not unmark!\n", p_chan->id, j+1);
				continue;
			}	   
			if(p_chan->sim[j].calllock_info.call_fail_mark_status == FLAG_YES){
				p_chan->sim[j].calllock_info.call_fail_mark_status = FLAG_NO;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]set call unmark.\n", p_chan->id, j+1);
			}
			p_chan->sim[j].calllock_info.call_fail_send_sms_times = 0;
			p_chan->sim[j].calllock_info.call_failed_count = 0;
		}
		pthread_mutex_unlock(&p_chan->lock);
		p_chan ++;
	}
	return 0;   
}


static int call_limit_all_simcard_unsmslimit(void)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;
	int channum = g_call_limit.sys_info.total_chans;
	int i = 0, j = 0;
	
//	print_debug("call_limit_all_simcard_unsmslimit\n");

	for(i=0; i<channum; i++) {	
		pthread_mutex_lock(&p_chan->lock);  
		for(j=0; j<SIM_NUM; j++) {
			p_chan->sim[j].smslimit_info.mon_cur_sms = 0;
			p_chan->sim[j].smslimit_info.day_cur_sms = 0;
			if(p_chan->sim[j].smslimit_info.mon_sms_limit_flag == SMS_LIMIT) {
				p_chan->sim[j].smslimit_info.mon_sms_limit_flag = SMS_UNLIMIT;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]mon sms unlimit.\n", p_chan->id, j+1);
			}
			if(p_chan->sim[j].smslimit_info.day_sms_limit_flag == SMS_LIMIT) {
				p_chan->sim[j].smslimit_info.day_sms_limit_flag = SMS_UNLIMIT;
				log_printf(LOG_INFO, "[chan_%d][sim_%d]day sms unlimit.\n", p_chan->id, j+1);
			}
		}
		pthread_mutex_unlock(&p_chan->lock);
		p_chan ++;
	}
	return 0;   
}


static int call_limit_switch_chan_simcard(int channel_id, int sim_idx)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;
	int res = -1;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return -1;		
	}  
	
//	print_debug("[chan_%d][sim_%d]call_limit_switch_chan_simcard\n", channel_id, sim_idx);
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);
	if(sim_idx != p_chan->sim_idx) {
		if(SIM_UNLIMIT == check_simcard_is_available(&p_chan->sim[sim_idx])) {
			res = select_simcard_from_chan(p_chan->id, sim_idx+1);
			if(res) {
				log_printf(LOG_ERROR, "[chan_%d][sim_%d]can't select simcard from chan\n", p_chan->id, sim_idx+1); 	
			} else {
				log_printf(LOG_INFO, "[chan_%d][sim_%d]switch to new simcard\n", p_chan->id, sim_idx+1); 
				cmd_sim_register(p_chan->id, sim_idx+1, p_chan->sim[sim_idx].sim_ping_code);		
				clean_sim_switch_info(&p_chan->simswitch);
				p_chan->sim[sim_idx].calltime_info.call_time_redis_flag = FLAG_NO;
				p_chan->sim_reg_time = p_chan->cur_time;
				p_chan->status.call_sta = CALL_HANDUP;
				p_chan->status.sim_reg_sta = REGISTERING;
				p_chan->sim_idx = sim_idx;	
				p_chan->simswitch.sim_switch_fail_flag = FLAG_NO;
				calllimit_redis_update_sim_slot(p_chan->id, sim_idx+1);
			}
		}
	}
	pthread_mutex_unlock(&p_chan->lock);
	return 0;   
}



static int call_limit_set_sim_daysmscnt(int channel_id, int sim_idx, unsigned int day_sms)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_daysmscnt, daysmscnt=%d\n", channel_id, sim_idx, day_sms);
	
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  
	p_chan->sim[sim_idx].smslimit_info.day_cur_sms = day_sms;
	if(day_sms == 0) {
		if(SMS_LIMIT == p_chan->sim[sim_idx].smslimit_info.day_sms_limit_flag ) {
			p_chan->sim[sim_idx].smslimit_info.day_sms_limit_flag = SMS_UNLIMIT;
			log_printf(LOG_INFO, "[chan_%d][sim_%d]day sms unlimit.\n", p_chan->id, sim_idx+1);
		}
	}
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;

}

static int call_limit_set_sim_monsmscnt(int channel_id, int sim_idx, unsigned int mon_sms)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_monsmscnt, monsmscnt=%d\n", channel_id, sim_idx, mon_sms);
	
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  
	p_chan->sim[sim_idx].smslimit_info.mon_cur_sms = mon_sms;
	if(mon_sms == 0) {
		p_chan->sim[sim_idx].smslimit_info.day_cur_sms = mon_sms;
		if(p_chan->sim[sim_idx].smslimit_info.mon_sms_limit_flag == SMS_LIMIT) {
			p_chan->sim[sim_idx].smslimit_info.mon_sms_limit_flag = SMS_UNLIMIT;
			log_printf(LOG_INFO, "[chan_%d][sim_%d]mon sms unlimit.\n", p_chan->id, sim_idx+1);
		}
		if(p_chan->sim[sim_idx].smslimit_info.day_sms_limit_flag == SMS_LIMIT) {
			p_chan->sim[sim_idx].smslimit_info.day_sms_limit_flag = SMS_UNLIMIT;
			log_printf(LOG_INFO, "[chan_%d][sim_%d]day sms unlimit.\n", p_chan->id, sim_idx+1);
		}
	}
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;

}

static int call_limit_set_sim_calltimecnt(int channel_id, int sim_idx, unsigned int calltimecnt)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_calltimecnt, calltimecnt=%d\n", channel_id, sim_idx, calltimecnt);
	
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  
	
	p_chan->sim[sim_idx].calltime_info.call_time_count = calltimecnt;
	p_chan->sim[sim_idx].calltime_info.call_time_remain = p_chan->cfg.calltime_cfg.call_time_total - calltimecnt;
	p_chan->sim[sim_idx].calltime_info.call_time_redis_flag = FLAG_NO;

	if(calltimecnt == 0){
		if(p_chan->sim[sim_idx].calltime_info.call_time_limit_flag == FLAG_YES) {
			p_chan->sim[sim_idx].calltime_info.call_time_limit_flag = FLAG_NO;
			p_chan->sim[sim_idx].calltime_info.sms_warning_flag = FLAG_NO;
			log_printf(LOG_INFO, "[chan_%d][sim_%d]call time unlimit.\n", p_chan->id, sim_idx+1);
		}
	}
	
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}

static int call_limit_set_sim_callfailedcnt(int channel_id, int sim_idx, unsigned int callfailedcnt)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_callfailedcnt, callfailedcnt=%d\n", channel_id, sim_idx, callfailedcnt);
	
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  
	p_chan->sim[sim_idx].calllock_info.call_failed_count = callfailedcnt;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;

}


static int call_limit_set_sim_calllimitcnt(int channel_id, int sim_idx, unsigned int daycalls, unsigned int dayanswers, unsigned int hourcalls)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

//	print_debug("[chan_%d][sim_%d]call_limit_set_sim_calllimitcnt, daycalls=%d, dayansnwers=%d, hourcalls=%d\n", channel_id, sim_idx, daycalls, dayanswers, hourcalls);
	
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  
	p_chan->sim[sim_idx].calllimit_info.day_cur_calls = daycalls;
	p_chan->sim[sim_idx].calllimit_info.day_cur_answers = dayanswers;
	p_chan->sim[sim_idx].calllimit_info.hour_cur_calls = hourcalls;
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;

}


static int call_limit_set_sim_pincode(int channel_id, int sim_idx, char *pincode)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);
	memset(p_chan->sim[sim_idx].sim_ping_code, 0, sizeof(p_chan->sim[sim_idx].sim_ping_code));
	memcpy(p_chan->sim[sim_idx].sim_ping_code, pincode, strlen(pincode));
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;

}


static int call_limit_get_sim_pincode(int channel_id, int sim_idx, char *pincode)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}

	if((sim_idx >= SIM_NUM) || (sim_idx < 0)) {
		log_printf(LOG_ERROR, "sim_idx(%d) out of range.\n", channel_id);
		return 0;		
	}  

	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  
	memcpy(pincode, p_chan->sim[sim_idx].sim_ping_code, strlen(p_chan->sim[sim_idx].sim_ping_code));
//	print_debug("[chan_%d][sim_%d]call_limit_get_sim_pincode:%s\n", channel_id, sim_idx, pincode);		
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;

}


static int call_limit_get_channel_pincode(int channel_id, char *pincode)
{
    calllimit_chan_t *p_chan = g_call_limit.chans;
	unsigned char sim_idx = 0;

	if((channel_id >= MAX_CHAN) || (channel_id <0)) {
		log_printf(LOG_ERROR, "[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	
	p_chan += channel_id;
	pthread_mutex_lock(&p_chan->lock);  
	sim_idx = p_chan->sim_idx;
	memcpy(pincode, p_chan->sim[sim_idx].sim_ping_code, strlen(p_chan->sim[sim_idx].sim_ping_code));
//	print_debug("[chan_%d][sim_%d]call_limit_get_channel_pincode:%s\n", channel_id, sim_idx, pincode);	
	pthread_mutex_unlock(&p_chan->lock);	
	return 0;
}



static int call_limit_get_remain_info(int channel_id, calllimit_get_remain_info_t *remain){
	int start_id, end_id, i;
	int chan_id = channel_id & ALL_CHANNEL;
	int sim_idx;
	calllimit_chan_t *p_call_limit = g_call_limit.chans;	
	if(is_valid_channel_id(chan_id)) {
		print_error("[%d]channel_id out of range\n", channel_id);
		return -1;
	}
	if(chan_id == ALL_CHANNEL){
		start_id = 0;
		end_id = g_call_limit.sys_info.total_chans;
		remain->__size = g_call_limit.sys_info.total_chans;
	}else{
		start_id = chan_id-1;
		end_id = chan_id;
		remain->__size = 1;
	}
	if(chan_id != ALL_CHANNEL) {
		p_call_limit +=  start_id;	
		sim_idx = p_call_limit->sim_idx;
		if(p_call_limit->cfg.calllimit_cfg.calllimit_switch != ENABLE)
			remain->__ptr[0].day_remain_calls = 0;
		else
			remain->__ptr[0].day_remain_calls = p_call_limit->cfg.calllimit_cfg.day_calls_settings - p_call_limit->sim[sim_idx].calllimit_info.day_cur_calls;
	}
	else{
		for(i = start_id; i < end_id; i++){
			p_call_limit = &g_call_limit.chans[i];
			sim_idx = p_call_limit->sim_idx;
			pthread_mutex_lock(&p_call_limit->lock);
			if(p_call_limit->cfg.calllimit_cfg.calllimit_switch != ENABLE)
				remain->__ptr[i].day_remain_calls = 0;
			else
				remain->__ptr[i].day_remain_calls =  p_call_limit->cfg.calllimit_cfg.day_calls_settings - p_call_limit->sim[sim_idx].calllimit_info.day_cur_calls;
			pthread_mutex_unlock(&p_call_limit->lock);
		}
	}
	return 0;
}

void call_limit_gsoap_init(void)
{
	gsoap_sap_t sap;
	memset(&sap, 0, sizeof(gsoap_sap_t));

	gsoap_init();
	sap.gsoap_cb_get_channel_settings = call_limit_get_channel_settings;
	sap.gsoap_cb_get_channel_status = call_limit_get_channel_status;
	sap.gsoap_cb_get_channel_sim_status = call_limit_get_channel_sim_status;
	sap.gsoap_cb_get_all_status = call_limit_get_all_status;
	sap.gsoap_cb_set_channel_calltime = call_limit_set_channel_calltime;
	sap.gsoap_cb_set_channel_dayanswers = call_limit_set_channel_dayanswers;
	sap.gsoap_cb_set_channel_daycalls = call_limit_set_channel_daycalls;
	sap.gsoap_cb_set_channel_hourcalls = call_limit_set_channel_hourcalls;
	sap.gsoap_cb_set_channel_switch = call_limit_set_channel_switch;
	sap.gsoap_cb_clean_channel_day_calls = call_limit_clean_channel_day_calls;
	sap.gsoap_cb_clean_channel_day_answers = call_limit_clean_channel_day_answers;
	sap.gsoap_cb_clean_channel_hour_calls = call_limit_clean_channel_hour_calls;
	sap.gsoap_cb_clean_channel_limit = call_limit_clean_channel_limit;
	sap.gsoap_cb_set_filewrite_switch = call_limit_set_filewrite_switch;
	sap.gsoap_cb_set_channel_unlock = call_limit_set_channel_unlock;
    sap.gsoap_cb_set_channel_unmark = call_limit_set_channel_unmark;
    sap.gsoap_cb_reload_cfg = call_limit_reload;
	sap.gsoap_cb_reflesh_status_cfg = call_limit_reflesh_status;
       
    sap.gsoap_cb_set_channel_check_flag = call_limit_set_channel_check;
    sap.gsoap_cb_set_channel_mark_flag = call_limit_set_mark_flag;
    sap.gsoap_cb_set_channel_lock_flag = call_limit_set_lock_flag;
    sap.gsoap_cb_set_channel_mark_count = call_limit_set_mark_count;
    sap.gsoap_cb_set_channel_lock_count = call_limit_set_lock_count;
    sap.gsoap_cb_set_channel_sms_flag = call_limit_set_sms_flag;
    sap.gsoap_cb_set_channel_sms_report_flag = call_limit_set_sms_report_flag;
    sap.gsoap_cb_set_channel_sms_failed_count = call_limit_set_sms_count;
    sap.gsoap_cb_set_channel_sms_sender = call_limit_set_sms_sender;
    sap.gsoap_cb_set_channel_sms_msg = call_limit_set_sms_msg;

	sap.gsoap_cb_channel_sms_limit = call_limit_clena_channel_sms_limit;
	sap.gsoap_cb_set_day_sms_limit_settings = call_limit_set_day_sms_limit_settings;
	sap.gsoap_cb_set_mon_sms_limit_settings = call_limit_set_mon_sms_limit_settings;
	sap.gsoap_cb_set_sms_limit_switch = call_limit_set_sms_limit_switch;
	sap.gsoap_cb_set_sms_limit_success_flag = call_limit_set_sms_limit_success_flag;
	sap.gsoap_cb_set_day_sms_cnt = call_limit_set_day_sms_cnt;
	sap.gsoap_cb_set_mon_sms_cnt = call_limit_set_mon_sms_cnt;
	sap.gsoap_cb_set_call_time_cnt = call_limit_set_call_time_cnt;
	sap.gsoap_cb_set_call_time_total = call_limit_set_call_time_total;

	sap.gsoap_cb_set_sim_limitsta = call_limit_set_sim_limitsta;
	sap.gsoap_cb_set_sim_unlimit = call_limit_set_sim_unlimit;
	sap.gsoap_cb_set_sim_unlock = call_limit_set_sim_unlock;
	sap.gsoap_cb_set_sim_unmark = call_limit_set_sim_unmark;
	sap.gsoap_cb_all_simcard_unlimit = call_limit_all_simcard_unlimit;
	sap.gsoap_cb_all_simcard_unlock = call_limit_all_simcard_unlock;
	sap.gsoap_cb_all_simcard_unmark = call_limit_all_simcard_unmark;
	sap.gsoap_cb_all_simcard_unsmslimit = call_limit_all_simcard_unsmslimit;
	sap.gsoap_cb_switch_chan_simcard = call_limit_switch_chan_simcard;
	sap.gsoap_cb_set_sim_daysmscnt = call_limit_set_sim_daysmscnt;
	sap.gsoap_cb_set_sim_monsmscnt = call_limit_set_sim_monsmscnt;
	sap.gsoap_cb_set_sim_calltimecnt = call_limit_set_sim_calltimecnt;
	sap.gsoap_cb_set_sim_callfailedcnt = call_limit_set_sim_callfailedcnt;
	sap.gsoap_cb_set_sim_calllimitcnt = call_limit_set_sim_calllimitcnt;
	sap.gsoap_cb_set_sim_pincode = call_limit_set_sim_pincode;
	sap.gsoap_cb_get_sim_pincode = call_limit_get_sim_pincode;
	sap.gsoap_cb_get_channel_pincode = call_limit_get_channel_pincode;
	
	sap.gsoap_cb_get_remain_info = call_limit_get_remain_info;
	gsoap_bind(&sap);
}

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


int main(void)
{
	if (check_lock_process())
		return -1;
	log_init();
	call_limit_para_init();
	event_thread_create();
	//get_sys_info_cfg(&g_call_limit.sys_info);
	calllimit_redis_get_sysinfo(&g_call_limit.sys_info);
	read_call_limit_cfg(&g_call_limit);
	gsoap_thread_create();
	start_check_sim_status();
	sleep(2);
	call_limit_thread_create(&g_call_limit);

	while(1) {
		sleep(200);
	}
	return 0;
}














