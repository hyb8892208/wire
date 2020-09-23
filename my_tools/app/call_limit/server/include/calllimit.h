#ifndef _CALLLIMIT_H
#define _CALLLIMIT_H

#define    MAX_CHAN     44

#define    CALL_TIME_LIMIT          0//3600          //通话时长限制 s
#define    DAY_CALL_LIMIT           0//100           //日呼叫次数限制
#define    DAY_ANSWER_LIMIT         0//80           //日接通次数限制
#define    HOUR_CALL_LIMIT          0//50           //小时呼叫次数限制
#define    SIM_REG_TIMEOUT          120             //sim reg timeout     


#define    PER_HOUR_SEC           3600
#define    REFRESH_TIMEUS         20000

#define    WARNING_CALLEE_LEN   64
#define    WARNING_MSG_LEN      128

#define    HANDUP_CMD             "asterisk -rx \"channel request hangup EXTRA/%d-1\""
#define    CALLLIMIT_CMD          "asterisk -rx \"gsm set calllimit %d %d\""
#define    SMSLIMIT_CMD          "asterisk -rx \"gsm set smslimit %d %d\""

#define    STACKSIZE             (((sizeof(void *) * 8 * 32) - 16) * 1024)
#define    QUEUESIZE               20

#define    SEND_MSG                "asterisk -rx \"gsm send sms %d %s \\\"%s\\\"\""
#define    SET_CALLMARK           "asterisk -rx \"gsm set simmark %d %d\""
#define    SET_CALLLOCK            "asterisk -rx \"gsm set simlock %d %d\""

#define    SIM_RELOAD_CMD          "asterisk -rx \"sim reload span %d %d %s\""

#define    DEFAULT_REDIS_IP      "127.0.0.1"
#define    REDIS_PORT            (6379)

#define    PING_CODE_LEN		  128

#define    print_error(format,...)    printf("[ERROR][%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define    print_debug(format,...)	printf("[DEBUG][%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)


typedef enum call_status_s
{	
	CALL_DAIL,
	CALL_RING,
	CALL_ANSWER,
	CALL_HANDUP,
	CALL_NO_CARRIER,
	CALL_IN,
	SIM_UP,
	SIM_IN,
	SIM_OUT,
	SMS_SENDING,
	SMS_SUCCESS,
	SMS_FAILED,
	SMS_SEND_FAILED,
	SMS_REPORT_SUCCESS,
	SMS_REPORT_FAILED,
	AST_REBOOT,
	CALL_UNKNOW,
}call_status_t;


typedef enum limit_switch_s
{
	DISABLE = 0,
	ENABLE,		
}limit_switch_t;

typedef enum call_limit_status_s
{
	UNLIMIT = 0,
	CALLLIMIT = 1
}call_limit_status_t;

typedef enum sms_limit_status_s{
	SMS_UNLIMIT = 0,
	SMS_LIMIT = 1
}sms_limit_status_t;

typedef enum sim_limit_status_s{
	SIM_UNLIMIT = 0,
	SIM_LIMIT = 1
}sim_limit_status_t;


typedef enum limit_flag_s
{
	FLAG_NO,
	FLAG_YES,
}limit_flag_t;

typedef enum call_time_clean_type_s{
	CALL_TIME_CLEAN_TYPE_UNKOWN = 0,
	CALL_TIME_CLEAN_TYPE_DAY,//日
	CALL_TIME_CLEAN_TYPE_WEEK,//星期
	CALL_TIME_CLEAN_TYPE_MON,//月
}call_time_clena_type_t;


typedef enum simcard_index_s {
	SIM1 = 0,
	SIM2 = 1,
	SIM3 = 2,
	SIM4 = 3,
	SIM_NUM,
}simcard_index_t;

typedef enum system_type_s{
	UNKNOW_T = 0,
	CHAN_1SIM = 1,
	CHAN_4SIM = 2,
}system_type_t;


typedef enum sim_policy_s{
	SIM_ASC,
	SIM_DES,
}sim_policy_t;

typedef enum limit_type_s{
	LT_CALL,                   
	LT_SIMLOCK,                
	LT_SMS,              
}limit_type_t;

typedef enum sim_slot_sta_s{                   
	SIM_IN_SLOT = 0,                
	SIM_OUT_SLOT = 1,              
}sim_slot_sta_t;

typedef enum sim_status_s{                   
	SIM_NONE = 0,
	SIM_IDLE = 1,
	SIM_FREE = 2,
	SIM_BUSY = 3,
	SIM_ERROR = 4,
	SIM_REGING = 5,
}sim_status_t;

typedef enum sim_register_s
{
	REGISTERING,      //注册中
	REGISTERED,       //注册成功
	REGISTERFAIL,     //注册失败
}sim_register_t;


typedef struct calllock_info_s{
	unsigned int call_failed_count;           //Cumulative count of consecutive call failures
	unsigned int call_fail_send_sms_times; //Cumulative send sms failed times 	
	unsigned char call_fail_mark_status;
	unsigned char call_fail_lock_status;		
}calllock_info_t;


typedef struct calllimit_info_s{
	struct tm day_call_last_clean_date;        //上一次日呼叫清除的日期
	struct tm day_answer_last_clean_date;     //上一次日接通清除的日期
	struct tm hour_call_last_clean_date;     //上一次时呼叫清除的日期
	unsigned int hour_cur_calls;		        // 当前小时累计呼叫次数
	unsigned int day_cur_calls; 		      // 当日累计呼叫次数	
	unsigned int day_cur_answers;		       // 当日累计接通次数 
	unsigned char daycall_generation_flag;	//日呼叫产生标志	
	unsigned char hourcall_generation_flag;	//小时呼叫产生标志	
	unsigned char call_answer_flag;		 //呼叫接通标志 
	unsigned char day_call_limit_flag;	  //日呼叫次数限制标志
	unsigned char day_answer_limit_flag;   //日接通次数限制标志
	unsigned char hour_call_limit_flag;	  //时呼叫次数限制标志	
}calllimit_info_t;


typedef struct calltime_info_s{
	struct tm call_time_clean_date;    //清除时间	
	struct timeval start_call_time;     // 开始通话时间	
	pthread_t tid;
	pthread_cond_t cond;
	pthread_mutex_t cond_lock;	
	int call_time_count;
	int call_time_remain;	
	unsigned char call_time_limit_flag;	
	unsigned char call_time_redis_flag;    //redis 写入标记
	unsigned char call_answer_flag;		 //呼叫接通标志 
	unsigned char sms_warning_flag;         //时长短信告警标记
    unsigned char handup_flag;         //挂断通话标志
	unsigned char cond_flag;	
	unsigned char call_time_warning_flag;  //时长短信告警标记
}calltime_info_t;


typedef struct smslimit_info_s{
	struct tm day_last_date;                  //上一次清除的日期
	struct tm mon_last_date;                  //上一次清除的日期
	unsigned int day_cur_sms;
	unsigned int mon_cur_sms;
	unsigned char mon_sms_limit_flag;	       //短信月限制标志
	unsigned char day_sms_limit_flag;	      //短信日限制标志	
	unsigned char sms_limit_warning_flag;  //时长短信告警标记
}smslimit_info_t;


typedef struct calllock_conf_s{
    char call_fail_lock_sms_msg[WARNING_MSG_LEN];          //message 		
    char call_fail_lock_sms_callee[WARNING_CALLEE_LEN];    //sms address setting	
    unsigned int call_fail_mark_count;  //Mark count number setting
    unsigned int call_fail_lock_count;  //lock count number setting
    unsigned int call_fail_lock_sms_count; //send sms failed count setting    
    unsigned char call_detect_flag;       //check sim lock on: 1 off:0
    unsigned char call_fail_mark_flag;    //sim mark is required  on:1 off: 0
    unsigned char call_fail_lock_flag;   //sim lock mark is required on:1 off: 0
    unsigned char call_fail_lock_sms_report_flag; // switch to check sms report     
    unsigned char call_fail_lock_sms_flag; //switch to send sms    
}calllock_conf_t;


typedef struct calltime_conf_s {
	char call_time_warning_msg[WARNING_MSG_LEN];			
	char call_time_warning_callee[WARNING_CALLEE_LEN];
	int call_time_total;                  //呼叫限制时长限制
	int call_time_free;                   //呼叫限制时长限制最小计时时间
	int call_time_step;                   //呼叫限制时长步长
	int call_time_warning_num;	
	unsigned int call_time_settings;     // 单次通话设定限制时间	
	unsigned char call_time_sw;
	unsigned char call_time_single_sw;    //单次通话时长限制开关
	unsigned char call_time_total_sw;     //通话总时长限制开关
	unsigned char call_time_clean_sw;	
	unsigned char call_time_clean_type;  //呼叫限制清理类型		
}calltime_conf_t;



typedef struct calllimit_conf_s {
	unsigned int day_calls_settings;	 // 日呼叫限制次数设定
	unsigned int day_answer_setting;	 //日接通次数设定	
	unsigned int hour_calls_settings;	 //小时呼叫限制次数设定
	unsigned char calllimit_switch;    //通道总开关	
}calllimit_conf_t;


typedef struct smslimit_conf_s {
	char smslimit_mon_warning_msg[WARNING_MSG_LEN];	
	char smslimit_mon_warning_callee[WARNING_CALLEE_LEN];
	unsigned int day_sms_settings;        //day sms limit 
	unsigned int mon_sms_settings;        //month sms limit	
	int smslimit_mon_warning_num;
	unsigned char smslimit_switch;	         //sms limit switch
	unsigned char smslimit_success_flag;   //sms success flag
	unsigned char sms_clean_date;           //sms month clean date
	unsigned char sms_warning_switch;
}smslimit_conf_t;


typedef struct simswitch_conf_s{
	unsigned int sim_reg_timeout;      //sim注册超时时间(s)
	unsigned int total_using_time;     //使用时长 (min)
	unsigned int total_callout_time;   //呼出时长 (min)
	unsigned int total_callout_count;  //呼出次数
	unsigned int total_sms_count;      //短信发送次数
	unsigned char sim_policy;          //sim切换模式
	unsigned char sim_switch_sw;       //切卡开关
}simswitch_conf_t;


typedef struct chan_conf_s {
	calllock_conf_t calllock_cfg;      //呼叫锁卡配置
	calllimit_conf_t calllimit_cfg;    //呼叫限制配置 
	calltime_conf_t calltime_cfg;      //呼叫通话配置
	smslimit_conf_t smslimit_cfg;      //短信限制配置
	simswitch_conf_t simswitch_cfg;    //通道切卡配置
}chan_conf_t;


typedef struct simcard_info_s {
	calllock_info_t calllock_info;     //呼叫锁卡信息
	calllimit_info_t calllimit_info;   //呼叫限制信息
	calltime_info_t calltime_info;     //呼叫通话信息
	smslimit_info_t smslimit_info;     //短信限制信息
	char sim_ping_code[PING_CODE_LEN];  //PING码
	unsigned char sim_slot_sta;
	unsigned char sim_slot_laststa;
	unsigned char sim_sta;
}simcard_info_t;


typedef struct chan_simswitch_info_s{
	struct timeval start_call_time;     // 开始通话时间	
	struct timeval last_using_time;   // 上一次统计使用时间	
	unsigned int curr_using_time;      //使用时长 
	unsigned int using_time_count; 
	unsigned int curr_callout_time;    //呼出时长 
	unsigned int curr_callout_count;  //呼出次数
	unsigned int curr_sms_count;       //短信发送次数
	unsigned char call_generation_flag;
	unsigned char call_answer_flag;
	unsigned char sim_switch_flag;     //
	unsigned char using_time_flag;
	unsigned char reopen_switch_flag;
	unsigned char sim_switch_fail_flag;
}chan_simswitch_info_t;


typedef  struct running_chan_status_s{
	call_status_t call_sta;                //当前通道呼叫状态
	call_status_t last_sta;               //当前通道上一次呼叫状态
	call_status_t sms_sta;                //当前通道短信状态	
	unsigned char sms_limit_sta;         //当前通道短信限制状态
	unsigned char sys_limit_sta;    	   //当前通道呼叫限制状态	
	unsigned char call_time_limit_sta;	//通话时长限制状态
	unsigned char call_lock_sta;          //锁卡限制状态
	unsigned char call_mark_sta;          //卡标记状态
	unsigned char sim_reg_sta;            //当前选中的sim卡注册状态
}running_chan_status_t;


typedef  struct  calllimit_chan_s {	
	simcard_info_t sim[SIM_NUM];	       //sim card状态信息
	chan_conf_t cfg;                      //通道配置信息
	chan_simswitch_info_t simswitch;      //通道切卡状态统计结构
	running_chan_status_t status;         //当前通道相关状态
    struct tm cur_date;                  //系统当前的日期
    struct timeval cur_time;             //系统当前时间
    struct timeval sim_reg_time;         //检测选中sim注册状态的开始时间
	pthread_t tid;                         //通道线程id
	pthread_mutex_t lock;                  //通道线程锁    
	unsigned int id;                       //channel id	
	void *equeue; 	                        //通道队列指针
	unsigned char sim_idx;                //当前通道使用的simcard	
}calllimit_chan_t;


typedef struct sys_info_cfg_s {
	unsigned int total_chans;            //总通道数
	unsigned char sys_type;               //系统类型
	unsigned char IsRefleshFile;         //是否刷新状态文件		
}sys_info_cfg_t;


typedef struct calllimit_s {	
	calllimit_chan_t chans[MAX_CHAN];           //通道数据结构	
	sys_info_cfg_t sys_info;       	         //系统信息
	pthread_t tid;
	pthread_mutex_t lock;                   
}calllimit_t;



typedef struct calllimit_settings_s {
    char call_fail_lock_sms_msg[WARNING_MSG_LEN];           //message 	
	char smslimit_mon_warning_msg[WARNING_MSG_LEN];    
	char call_time_warning_msg[WARNING_MSG_LEN];
    char call_fail_lock_sms_callee[WARNING_CALLEE_LEN];    //sms address setting	
	char call_time_warning_callee[WARNING_CALLEE_LEN];	
	char smslimit_mon_warning_callee[WARNING_CALLEE_LEN];
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

typedef struct calllimit_remain_info_s{
	unsigned int day_remain_calls;	
}calllimit_remain_info_t;

typedef struct calllimit_get_remain_info_s{
	struct calllimit_remain_info_s  __ptr[44];
	int __size;
}calllimit_get_remain_info_t;

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


extern int parsing_call_status(int channel_id, call_status_t call_sta);
extern  int put_call_limit_event(int channel_id, call_status_t call_sta);
extern int set_reboot_state(void);
extern reset_limit_state(void);
extern int reset_limit_chanel_state(int channel_id);
extern int compare_destination(int channel_id, char *addr);
extern int start_reflesh_callstatus_conf(void);
extern void call_limit_gsoap_init(void);
extern int calllimit_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *data);

#endif



