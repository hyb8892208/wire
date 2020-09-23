#ifndef _GSOAP_SERVER_H_
#define _GSOAP_SERVER_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <errno.h>
//#include <string.h>
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <sys/socket.h>
//#include <sys/wait.h>


#define GSOAP_MAX_THR	(3) // Size of thread pool 
#define GSOAP_MAX_QUEUE	(100) // Max. size of request queue 

#define	GSOAP_BACK_LOG		100
#define	UDP_IP_MAX_LEN		16

#define GSOAP_SERVER_PORT        9801
#define GSOAP_SERVER_IP          "127.0.0.1"

#define    MAX_CHAN     44

#define    SIM_NUM      4

#define gsoap_print(format,...)			printf("[File:%s, Line:%d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define gsoap_print_error(format,...)		printf("[ERROR][File:%s, Line:%d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define gsoap_print_warn(format,...)		printf("[WARN][File:%s, Line:%d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define gsoap_print_info(format,...)		printf("[INFO][File:%s, Line:%d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define gsoap_print_debug(format,...)		printf("[DEBUG][File:%s, Line:%d] "format, __FILE__, __LINE__, ##__VA_ARGS__)



typedef struct _gsoap_params_s
{
	unsigned short gsoap_port;
	unsigned short count;
}gsoap_params_t;


typedef int (*gsoap_callback_get_channel_settings_t)(int channel_id, calllimit_settings_t *settings);
typedef int (*gsoap_callback_get_channel_status_t)(int channel_id, calllimit_status_t *status);
typedef int (*gsoap_callback_get_channel_sim_status_t)(int channel_id, int sim_idx, calllimit_status_t *status);

typedef int (*gsoap_callback_get_all_status_t)(calllimit_getall_status_t *status);


typedef int (*gsoap_callback_set_channel_calltime_t)(int channel_id, unsigned int calltime);
typedef int (*gsoap_callback_set_channel_daycalls_t)(int channel_id, unsigned int daycalls);
typedef int (*gsoap_callback_set_channel_dayanswers_t)(int channel_id, unsigned int dayanswers);
typedef int (*gsoap_callback_set_channel_hourcalls_t)(int channel_id, unsigned int hourcalls);
typedef int (*gsoap_callback_set_channel_switch_t)(int channel_id, unsigned int call_switch);
typedef int (*gsoap_callback_clean_channel_day_calls_t)(int channel_id);
typedef int (*gsoap_callback_clean_channel_day_answers_t)(int channel_id);
typedef int (*gsoap_callback_clean_channel_hour_calls_t)(int channel_id);
typedef int (*gsoap_callback_clean_channel_limit_t)(int channel_id);
typedef int (*gsoap_callback_set_filewrite_switch_t)(unsigned int file_switch);
typedef int (*gsoap_callback_set_channel_unlock_t)(int channel_id);
typedef int (*gsoap_callback_set_channel_unmark_t)(int channel_id);

typedef int (*gsoap_callback_set_param_t)(int channel_id, unsigned int param);
typedef int (*gsoap_callback_set_param_str)(int channel_id, char* param);

typedef int (*gsoap_callback_reload_t)(void);
typedef int (*gsoap_callback_reflesh_status_t)(void);


typedef int (*gsoap_callback_clena_channel_sms_limit_t)(int channel_id);
typedef int (*gsoap_callback_set_day_sms_limit_settings_t)(int channel_id, unsigned int sms_settings);
typedef int (*gsoap_callback_set_mon_sms_limit_settings_t)(int channel_id, unsigned int sms_settings);
typedef int (*gsoap_callback_set_sms_limit_switch_t)(int channel_id, unsigned int sms_switch);
typedef int (*gsoap_callback_set_sms_limit_success_flag_t)(int channel_id, unsigned int flag);
typedef int (*gsoap_callback_set_day_sms_cnt_t)(int channel_id, unsigned int cur_sms);
typedef int (*gsoap_callback_set_mon_sms_cnt_t)(int channel_id, unsigned int cur_sms);
typedef int (*gsoap_callback_set_call_time_cnt_t)(int channel_id, unsigned int call_time_cnt);
typedef int (*gsoap_callback_set_call_time_total_t)(int channel_id, unsigned int total);


typedef int (*gsoap_callback_set_sim_limitsta_t)(int channel_id, int sim_idx, unsigned int calllimit_flag, unsigned int calllock_flag, unsigned int callmark_flag, unsigned int smslimit_flag);
typedef int (*gsoap_callback_set_sim_unlimit_t)(int channel_id, int sim_idx);
typedef int (*gsoap_callback_set_sim_unlock_t)(int channel_id, int sim_idx);
typedef int (*gsoap_callback_set_sim_unmark_t)(int channel_id, int sim_idx);

typedef int (*gsoap_callback_all_simcard_unlimit_t)(void);
typedef int (*gsoap_callback_all_simcard_unlock_t)(void);
typedef int (*gsoap_callback_all_simcard_unmark_t)(void);
typedef int (*gsoap_callback_all_simcard_unsmslimit_t)(void);
typedef int (*gsoap_callback_switch_chan_simcard_t)(int channel_id, int sim_idx);


typedef int (*gsoap_callback_set_sim_daysmscnt_t)(int channel_id, int sim_idx, unsigned int day_sms);
typedef int (*gsoap_callback_set_sim_monsmscnt_t)(int channel_id, int sim_idx, unsigned int mon_sms);
typedef int (*gsoap_callback_set_sim_calltimecnt_t)(int channel_id, int sim_idx, unsigned int calltimecnt);
typedef int (*gsoap_callback_set_sim_callfailedcnt_t)(int channel_id, int sim_idx, unsigned int callfailedcnt);
typedef int (*gsoap_callback_set_sim_calllimitcnt_t)(int channel_id, int sim_idx, unsigned int daycalls, unsigned int dayanswers, unsigned int hourcalls);
typedef int (*gsoap_callback_set_sim_pincode_t)(int channel_id, int sim_idx, char *pincode);
typedef int (*gsoap_callback_get_sim_pincode_t)(int channel_id, int sim_idx, char *pincode);
typedef int (*gsoap_callback_get_channel_pincode_t)(int channel_id, char *pincode);


typedef int (*gsoap_callback_get_remain_info_t)(int channel_id, struct calllimit_get_remain_info_s *remain_calls);

typedef struct _gsoap_sap_s
{
	gsoap_callback_get_channel_settings_t gsoap_cb_get_channel_settings;
	gsoap_callback_get_channel_status_t gsoap_cb_get_channel_status;
	gsoap_callback_get_channel_sim_status_t gsoap_cb_get_channel_sim_status;
	gsoap_callback_get_all_status_t gsoap_cb_get_all_status;
	gsoap_callback_set_channel_calltime_t gsoap_cb_set_channel_calltime;
	gsoap_callback_set_channel_daycalls_t gsoap_cb_set_channel_daycalls;
	gsoap_callback_set_channel_dayanswers_t gsoap_cb_set_channel_dayanswers;
	gsoap_callback_set_channel_hourcalls_t gsoap_cb_set_channel_hourcalls;
	gsoap_callback_set_channel_switch_t gsoap_cb_set_channel_switch;	
	gsoap_callback_clean_channel_day_calls_t gsoap_cb_clean_channel_day_calls;
	gsoap_callback_clean_channel_day_answers_t gsoap_cb_clean_channel_day_answers;
	gsoap_callback_clean_channel_hour_calls_t gsoap_cb_clean_channel_hour_calls;
	gsoap_callback_clean_channel_limit_t gsoap_cb_clean_channel_limit;
	gsoap_callback_set_filewrite_switch_t gsoap_cb_set_filewrite_switch;	
	gsoap_callback_set_channel_unlock_t gsoap_cb_set_channel_unlock;
	gsoap_callback_set_channel_unmark_t gsoap_cb_set_channel_unmark;
	gsoap_callback_reload_t gsoap_cb_reload_cfg;
	gsoap_callback_reflesh_status_t gsoap_cb_reflesh_status_cfg;
	   
	gsoap_callback_set_param_t gsoap_cb_set_channel_check_flag;
	gsoap_callback_set_param_t gsoap_cb_set_channel_mark_flag;
	gsoap_callback_set_param_t gsoap_cb_set_channel_lock_flag;
	gsoap_callback_set_param_t gsoap_cb_set_channel_mark_count;
	gsoap_callback_set_param_t gsoap_cb_set_channel_lock_count;
	gsoap_callback_set_param_t gsoap_cb_set_channel_sms_flag;
	gsoap_callback_set_param_t gsoap_cb_set_channel_sms_report_flag;
	gsoap_callback_set_param_t gsoap_cb_set_channel_sms_failed_count;
	gsoap_callback_set_param_str gsoap_cb_set_channel_sms_sender;
	gsoap_callback_set_param_str gsoap_cb_set_channel_sms_msg;
	
	gsoap_callback_clena_channel_sms_limit_t gsoap_cb_channel_sms_limit;
	gsoap_callback_set_day_sms_limit_settings_t gsoap_cb_set_day_sms_limit_settings;
	gsoap_callback_set_mon_sms_limit_settings_t gsoap_cb_set_mon_sms_limit_settings;
	gsoap_callback_set_sms_limit_switch_t gsoap_cb_set_sms_limit_switch;
	gsoap_callback_set_sms_limit_success_flag_t gsoap_cb_set_sms_limit_success_flag;
	gsoap_callback_set_day_sms_cnt_t gsoap_cb_set_day_sms_cnt;
	gsoap_callback_set_mon_sms_cnt_t gsoap_cb_set_mon_sms_cnt;
	gsoap_callback_set_call_time_cnt_t gsoap_cb_set_call_time_cnt;
	gsoap_callback_set_call_time_total_t gsoap_cb_set_call_time_total;

	gsoap_callback_set_sim_limitsta_t gsoap_cb_set_sim_limitsta;
	gsoap_callback_set_sim_unlimit_t gsoap_cb_set_sim_unlimit;
	gsoap_callback_set_sim_unlock_t gsoap_cb_set_sim_unlock;
	gsoap_callback_set_sim_unmark_t gsoap_cb_set_sim_unmark;
	gsoap_callback_all_simcard_unlimit_t gsoap_cb_all_simcard_unlimit;
	gsoap_callback_all_simcard_unlock_t gsoap_cb_all_simcard_unlock;
	gsoap_callback_all_simcard_unmark_t gsoap_cb_all_simcard_unmark;
	gsoap_callback_all_simcard_unsmslimit_t gsoap_cb_all_simcard_unsmslimit;
	gsoap_callback_switch_chan_simcard_t gsoap_cb_switch_chan_simcard;

	gsoap_callback_set_sim_daysmscnt_t gsoap_cb_set_sim_daysmscnt;
	gsoap_callback_set_sim_monsmscnt_t gsoap_cb_set_sim_monsmscnt;
	gsoap_callback_set_sim_calltimecnt_t gsoap_cb_set_sim_calltimecnt;
	gsoap_callback_set_sim_callfailedcnt_t gsoap_cb_set_sim_callfailedcnt;
	gsoap_callback_set_sim_calllimitcnt_t gsoap_cb_set_sim_calllimitcnt;
	gsoap_callback_set_sim_pincode_t gsoap_cb_set_sim_pincode;
	gsoap_callback_get_sim_pincode_t gsoap_cb_get_sim_pincode;
	gsoap_callback_get_channel_pincode_t gsoap_cb_get_channel_pincode;
	
	gsoap_callback_get_remain_info_t gsoap_cb_get_remain_info;
}gsoap_sap_t;


void gsoap_init();
int gsoap_bind(gsoap_sap_t *sap);
int gsoap_thread_create(void);


extern int calllimit_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *data);


#endif







