#ifndef __GSOAP_CLI_H_
#define __GSOAP_CLI_H_


typedef struct calllimit_setting_s {
    char call_fail_lock_sms_msg[128];           //message 	
	char smslimit_mon_warning_msg[128];    
	char call_time_warning_msg[128];
    char call_fail_lock_sms_callee[64];    //sms address setting	
	char call_time_warning_callee[64];	
	char smslimit_mon_warning_callee[64];
	unsigned char call_time_sw;
	unsigned char call_time_single_sw;    //����ͨ��ʱ�����ƿ���
	unsigned char call_time_settings;     // ����ͨ���趨����ʱ��
	unsigned char call_time_total_sw;     //ͨ����ʱ�����ƿ���
	unsigned char call_time_clean_sw;
	unsigned char call_time_clean_type;  //����������������
	unsigned char calllimit_switch;       //ͨ���ܿ���
	unsigned char sim_switch_sw;         //�п�����		
	unsigned char sim_policy;             //sim�л�ģʽ	
    unsigned char call_detect_flag;       //check sim lock on: 1 off:0
    unsigned char call_fail_mark_flag;    //sim mark is required  on:1 off: 0
    unsigned char call_fail_lock_flag;   //sim lock mark is required on:1 off: 0
    unsigned char call_fail_lock_sms_flag; //switch to send sms
    unsigned char call_fail_lock_sms_report_flag; // switch to check sms report 
    unsigned char smslimit_switch;     //sms limit switch
	unsigned char smslimit_success_flag;//sms success flag
	unsigned char sms_warning_switch;
	unsigned char sms_clean_date;//sms month clean date
	int call_time_total;                  //��������ʱ������
	int call_time_free;                   //��������ʱ��������С��ʱʱ��
	int call_time_step;                   //��������ʱ������
	int smslimit_mon_warning_num;
	int call_time_warning_num;
	unsigned int day_calls_settings;    // �պ������ƴ����趨
	unsigned int day_answer_setting;    //�ս�ͨ�����趨	
	unsigned int hour_calls_settings;   //Сʱ�������ƴ����趨
    unsigned int call_fail_mark_count;  //Mark count number setting
    unsigned int call_fail_lock_count;  //lock count number setting
    unsigned int call_fail_lock_sms_count; //send sms failed count setting
    unsigned int day_sms_settings;//day sms limit 
	unsigned int mon_sms_settings;//month sms limit
	unsigned int sim_reg_timeout;     //simע�ᳬʱʱ��(s)
	unsigned int total_using_time;     //ʹ��ʱ�� (min)
	unsigned int total_callout_time;   //����ʱ�� (min)
	unsigned int total_callout_count;  //��������
	unsigned int total_sms_count;      //���ŷ��ʹ���	
}calllimit_setting_t;


typedef struct calllimit_statu_s {
	char call_time_clean_date[64];	
    unsigned int hour_cur_calls;          // ��ǰСʱ�ۼƺ��д���
	unsigned int day_cur_calls;          // �����ۼƺ��д���	
	unsigned int day_cur_answers;       // �����ۼƽ�ͨ����	
    unsigned int call_failed_count;   //Cumulative count of consecutive call failures
    unsigned int call_fail_send_sms_times; //Cumulative send sms failed times 
	unsigned int day_cur_sms;
	unsigned int mon_cur_sms;
	unsigned int curr_using_time;      //ʹ��ʱ�� 
	unsigned int curr_callout_time;    //����ʱ�� 
	unsigned int curr_callout_count;  //��������
	unsigned int curr_sms_count;       //���ŷ��ʹ���		
	int call_time_count;
	int call_time_remain;
	unsigned char call_sta;
	unsigned char call_time_limit_flag;
	unsigned char call_time_limit_sta;
    unsigned char call_answer_flag;       //���н�ͨ��־	
    unsigned char call_generation_flag	;   //���в�����־      
    unsigned char day_call_limit_flag;     //�պ��д������Ʊ�־
    unsigned char day_answer_limit_flag;   //�ս�ͨ�������Ʊ�־
    unsigned char hour_call_limit_flag;    //ʱ���д������Ʊ�־
	unsigned char sys_limit_sta;    		//��ǰϵͳ��������״̬	    
	unsigned char mon_sms_limit_flag;    //���������Ʊ�־
	unsigned char day_sms_limit_flag;    //���������Ʊ�־
	unsigned char sms_limit_sta;
	unsigned char call_time_redis_flag;//redis д����
	unsigned char call_fail_mark_status;
    unsigned char call_fail_lock_status;	
	unsigned char sim_idx; 
	unsigned char sim_sta;
}calllimit_statu_t;


typedef struct calllimit_all_statu_s
{
	calllimit_statu_t *__ptr;
	int __size;
}calllimit_all_statu_t;


typedef struct calllimit_getall_statu_s
{
    calllimit_all_statu_t status;
	int result;
}calllimit_getall_statu_t;


typedef struct sim_pincode__s
{
    char pincode[128];
	int result;
}sim_pincode__t;

struct calllimit_remain_info{
	unsigned int day_remain_calls;	
};

struct calllimit_get_remain_info{
	struct calllimit_remain_info  __ptr[44];
	int __size;
};

int calllimit_get_channel_settings(int channel_id, calllimit_setting_t *settings);
int calllimit_get_channel_status(int channel_id, calllimit_statu_t *status);
int calllimit_get_channel_sim_status(int channel_id, int sim_idx, calllimit_statu_t *status);
int calllimit_get_all_status(calllimit_statu_t *status);
int calllimit_set_channel_calltime(int channel_id, unsigned int calltime);
int calllimit_set_channel_daycalls(int channel_id, unsigned int daycalls);
int calllimit_set_channel_dayanswers(int channel_id, unsigned int dayanswers);
int calllimit_set_channel_hourcalls(int channel_id, unsigned int hourcalls);
int calllimit_set_channel_call_switch(int channel_id, unsigned int call_switch);
int calllimit_set_filewrite_switch(unsigned int file_switch);
int calllimit_clean_channel_day_answers(int channel_id);
int calllimit_clean_channel_hour_calls(int channel_id);
int calllimit_clean_channel_day_calls(int channel_id);
int calllimit_clean_channel_limit(int channel_id);


int calllimit_set_channel_unlock(int channel_id);
int calllimit_set_channel_unmark(int channel_id);
int calllimit_reload_cfg(void);
int calllimit_set_channel_detect(int channel_id, unsigned int flag);
int calllimit_reflesh_status_cfg(void);
int calllimit_set_channel_mark_flag(int channel_id, unsigned int flag);
int calllimit_set_channel_mark_count(int channel_id, unsigned int count);
int calllimit_set_channel_lock_flag(int channel_id, unsigned int flag);
int calllimit_set_channel_lock_count(int channel_id, unsigned int count);
int calllimit_set_channel_lock_sms_flag(int channel_id, unsigned int flag);
int calllimit_set_channel_lock_sms_report_flag(int channel_id, unsigned int flag);
int calllimit_set_channel_lock_sms_count(int channel_id, unsigned int count);
int calllimit_set_channel_lock_sms_callee(int channel_id, char *addr);
int calllimit_set_channel_lock_sms_msg(int channel_id, char *msg);

int calllimit_clena_channel_sms_limit(int channel_id);
int calllimit_set_day_sms_limit_settings(int channel_id, unsigned int sms_settings);
int calllimit_set_mon_sms_limit_settings(int channel_id, unsigned int sms_settings);
int calllimit_set_sms_limit_switch(int channel_id, unsigned int sms_switch);
int calllimit_set_sms_limit_success_flag(int channel_id, unsigned int flag);
int calllimit_set_mon_sms_cnt(int channel_id, unsigned int cur_sms);
int calllimit_set_day_sms_cnt(int channel_id, unsigned int cur_sms);
int calllimit_set_call_time_cnt( int channel_id, unsigned int call_time_cnt);
int calllimit_set_call_time_total( int channel_id, unsigned int call_time_cnt);
int calllimit_set_sim_callfailedcnt(int channel_id, int sim_idx, unsigned int callfailedcnt);
int calllimit_set_sim_calllimitcnt(int channel_id, int sim_idx, unsigned int daycalls, unsigned int dayanswers, unsigned int hourcalls);


int calllimit_set_sim_unlimit(int channel_id, int sim_idx);
int calllimit_set_sim_unlock(int channel_id, int sim_idx);
int calllimit_set_sim_unmark(int channel_id, int sim_idx);
int calllimit_all_simcard_unlimit(void);
int calllimit_all_simcard_unlock(void);
int calllimit_all_simcard_unmark(void);
int calllimit_all_simcard_unsmslimit(void);
int calllimit_switch_chan_simcard(int channel_id, int sim_idx);
int calllimit_set_sim_daysmscnt(int channel_id, int sim_idx, unsigned int daysmscnt);
int calllimit_set_sim_monsmscnt(int channel_id, int sim_idx, unsigned int monsmscnt);
int calllimit_set_sim_calltimecnt(int channel_id, int sim_idx, unsigned int calltimecnt);
int calllimit_set_sim_pincode(int channel_id, int sim_idx, char *pincode);
int calllimit_get_sim_pincode(int channel_id, int sim_idx, sim_pincode__t *pincode);
int calllimit_get_channel_pincode(int channel_id, sim_pincode__t *pincode);

int calllimit_get_remain_info(int channel_id, struct calllimit_get_remain_info *remain);
#endif

