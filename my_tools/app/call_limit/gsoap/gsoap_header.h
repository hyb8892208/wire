//gsoap calllimit service  name:	calllimit Configuring calllimit server tools and calllimit client tools parameters
//gsoap calllimit service  protocol:	SOAP
//gsoap calllimit service e style:	rpc
//gsoap calllimit service  encoding:	encoded
//gsoap calllimit service  location:	http://localhost:9801
//gsoap calllimit schema namespace:	urn:calllimit


typedef struct calllimit_settings_s {
    char call_fail_lock_sms_msg[128];           //message 	
	char smslimit_mon_warning_msg[128];    
	char call_time_warning_msg[128];
    char call_fail_lock_sms_callee[64];    //sms address setting	
	char call_time_warning_callee[64];	
	char smslimit_mon_warning_callee[64];
	unsigned char call_time_sw;
	unsigned char call_time_single_sw;    //单次通话时长限制开关
	unsigned char call_time_settings;     // 单次通话设定限制时间
	unsigned char call_time_total_sw;     //通话总时长限制开关
	unsigned char call_time_clean_sw;
	unsigned char call_time_clean_type;  //呼叫限制清理类型
	unsigned char calllimit_switch;       //通道总开关
	unsigned char sim_switch_sw;         //切卡开关		
	unsigned char sim_policy;             //sim切换模式	
    unsigned char call_detect_flag;       //check sim lock on: 1 off:0
    unsigned char call_fail_mark_flag;    //sim mark is required  on:1 off: 0
    unsigned char call_fail_lock_flag;   //sim lock mark is required on:1 off: 0
    unsigned char call_fail_lock_sms_flag; //switch to send sms
    unsigned char call_fail_lock_sms_report_flag; // switch to check sms report 
    unsigned char smslimit_switch;     //sms limit switch
	unsigned char smslimit_success_flag;//sms success flag
	unsigned char sms_warning_switch;
	unsigned char sms_clean_date;//sms month clean date
	int call_time_total;                  //呼叫限制时长限制
	int call_time_free;                   //呼叫限制时长限制最小计时时间
	int call_time_step;                   //呼叫限制时长步长
	int smslimit_mon_warning_num;
	int call_time_warning_num;
	unsigned int day_calls_settings;    // 日呼叫限制次数设定
	unsigned int day_answer_setting;    //日接通次数设定	
	unsigned int hour_calls_settings;   //小时呼叫限制次数设定
    unsigned int call_fail_mark_count;  //Mark count number setting
    unsigned int call_fail_lock_count;  //lock count number setting
    unsigned int call_fail_lock_sms_count; //send sms failed count setting
    unsigned int day_sms_settings;//day sms limit 
	unsigned int mon_sms_settings;//month sms limit
	unsigned int sim_reg_timeout;     //sim注册超时时间(s)
	unsigned int total_using_time;     //使用时长 (min)
	unsigned int total_callout_time;   //呼出时长 (min)
	unsigned int total_callout_count;  //呼出次数
	unsigned int total_sms_count;      //短信发送次数	
}calllimit_settings_t;


typedef struct calllimit_status_s {
	char call_time_clean_date[64];	
    unsigned int hour_cur_calls;          // 当前小时累计呼叫次数
	unsigned int day_cur_calls;          // 当日累计呼叫次数	
	unsigned int day_cur_answers;       // 当日累计接通次数	
    unsigned int call_failed_count;   //Cumulative count of consecutive call failures
    unsigned int call_fail_send_sms_times; //Cumulative send sms failed times 
	unsigned int day_cur_sms;
	unsigned int mon_cur_sms;
	unsigned int curr_using_time;      //使用时长 
	unsigned int curr_callout_time;    //呼出时长 
	unsigned int curr_callout_count;  //呼出次数
	unsigned int curr_sms_count;       //短信发送次数		
	int call_time_count;
	int call_time_remain;
	unsigned char call_sta;
	unsigned char call_time_limit_flag;
	unsigned char call_time_limit_sta;
    unsigned char call_answer_flag;       //呼叫接通标志	
    unsigned char call_generation_flag	;   //呼叫产生标志      
    unsigned char day_call_limit_flag;     //日呼叫次数限制标志
    unsigned char day_answer_limit_flag;   //日接通次数限制标志
    unsigned char hour_call_limit_flag;    //时呼叫次数限制标志
	unsigned char sys_limit_sta;    		//当前系统呼叫限制状态	    
	unsigned char mon_sms_limit_flag;    //短信月限制标志
	unsigned char day_sms_limit_flag;    //短信日限制标志
	unsigned char sms_limit_sta;
	unsigned char call_time_redis_flag;//redis 写入标记
	unsigned char call_fail_mark_status;
    unsigned char call_fail_lock_status;	
	unsigned char sim_idx; 
	unsigned char sim_sta;
}calllimit_status_t;



typedef struct calllimit_get_settings_s
{
	calllimit_settings_t settings;
	int result;
}calllimit_get_settings_t;

typedef struct calllimit_get_status_s
{
	calllimit_status_t status;
	int result;
}calllimit_get_status_t;


typedef struct calllimit_all_status_s
{
	calllimit_status_t *__ptr;
	int __size;
}calllimit_all_status_t;


typedef struct calllimit_getall_status_s
{
    calllimit_all_status_t status;
	int result;
}calllimit_getall_status_t;


typedef struct sim_pincode_s
{
    char pincode[128];
	int result;
}sim_pincode_t;

typedef struct calllimit_remain_info_s{
	unsigned int day_remain_calls;	
}calllimit_remain_info_t;

typedef struct calllimit_get_remain_info_s{
	struct calllimit_remain_info_s  __ptr[44];
	int __size;
}calllimit_get_remain_info_t;

int calllimit__get_channel_settings(int channel_id, calllimit_get_settings_t *settings);
int calllimit__get_channel_status(int channel_id, calllimit_get_status_t *status);
int calllimit__get_channel_sim_status(int channel_id, int sim_idx, calllimit_get_status_t *status);
int calllimit__get_all_status(calllimit_getall_status_t *status);
int calllimit__set_channel_calltime(int channel_id, unsigned int calltime, int *result);
int calllimit__set_channel_daycalls(int channel_id, unsigned int daycalls, int *result);
int calllimit__set_channel_dayanswers(int channel_id, unsigned int dayanswers, int *result);
int calllimit__set_channel_hourcalls(int channel_id, unsigned int hourcalls, int *result);
int calllimit__set_channel_call_swiitch(int channel_id, unsigned int call_swiitch, int *result);
int calllimit__clean_channel_day_calls(int channel_id, int *result);
int calllimit__clean_channel_day_answers(int channel_id, int *result);
int calllimit__clean_channel_hour_calls(int channel_id, int *result);
int calllimit__set_filewrite_switch(unsigned int file_switch, int *result);
int calllimit__clean_channel_limit(int channel_id, int *result);

int calllimit__set_channel_unlock(int channel_id, int *result);
int calllimit__set_channel_unmark(int channel_id, int *result);
int calllimit__reload_cfg(int *result);
int calllimit__reflesh_status_cfg(int *result);

int calllimit__set_channel_lock_detect(int channel_id, unsigned int flag, int *result);
int calllimit__set_channel_mark_flag(int channel_id,unsigned int flag, int *result);
int calllimit__set_channel_mark_count(int channel_id,unsigned int count,int *result);
int calllimit__set_channel_lock_flag(int channel_id, unsigned int flag,int *result);
int calllimit__set_channel_lock_count(int channel_id, unsigned int count,int *result);
int calllimit__set_channel_sms_flag(int channel_id, unsigned int flag, int *result);
int calllimit__set_channel_sms_report_flag(int channel_id, unsigned int flag, int *result);
int calllimit__set_channel_sms_count(int channel_id, unsigned int count, int *result);
int calllimit__set_channel_sms_sender(int channel_id, char *addr,int *result);
int calllimit__set_channel_sms_msg(int channel_id, char *msg,int *result);

int calllimit__clena_channel_sms_limit(int channel_id,int *result);
int calllimit__set_day_sms_limit_settings(int channel_id, unsigned int sms_settings, int *result);
int calllimit__set_mon_sms_limit_settings(int channel_id, unsigned int sms_settings, int *result);
int calllimit__set_sms_limit_switch(int channel_id, unsigned int sms_switch,int *result);
int calllimit__set_sms_limit_success_flag(int channel_id, unsigned int flag, int *result);
int calllimit__set_mon_sms_cnt(int channel_id, unsigned int cur_sms, int *result);
int calllimit__set_day_sms_cnt(int channel_id, unsigned int cur_sms,int *result);
int calllimit__set_call_time_cnt( int channel_id, unsigned int call_time_cnt,int *result);
int calllimit__set_call_time_total( int channel_id, unsigned int total,int *result);
int calllimit__set_sim_limitsta(int channel_id, int sim_idx, unsigned int calllimit_flag, unsigned int calllock_flag, unsigned int callmark_flag, unsigned int smslimit_flag, int *result);
int calllimit__set_sim_unlimit(int channel_id, int sim_idx, int *result);
int calllimit__set_sim_unlock(int channel_id, int sim_idx, int *result);
int calllimit__set_sim_unmark(int channel_id, int sim_idx, int *result);
int calllimit__all_simcard_unlimit(int *result);
int calllimit__all_simcard_unlock(int *result);
int calllimit__all_simcard_unmark(int *result);
int calllimit__all_simcard_unsmslimit(int *result);
int calllimit__switch_chan_simcard(int channel_id, int sim_idx, int *result);

int calllimit__set_sim_daysmscnt(int channel_id, int sim_idx, unsigned int day_sms, int *result);
int calllimit__set_sim_monsmscnt(int channel_id, int sim_idx, unsigned int mon_sms, int *result);
int calllimit__set_sim_calltimecnt(int channel_id, int sim_idx, unsigned int calltimecnt, int *result);
int calllimit__set_sim_callfailedcnt(int channel_id, int sim_idx, unsigned int callfailedcnt, int *result);
int calllimit__set_sim_calllimitcnt(int channel_id, int sim_idx, unsigned int daycalls, unsigned int dayanswers, unsigned int hourcalls, int *result);

int calllimit__set_sim_pincode(int channel_id, int sim_idx, char *pincode, int *result);
int calllimit__get_sim_pincode(int channel_id, int sim_idx, sim_pincode_t *result);
int calllimit__get_channel_pincode(int channel_id, sim_pincode_t *result);

int calllimit__get_remain_info( int channel_id, struct calllimit_get_remain_info_s *remain);
