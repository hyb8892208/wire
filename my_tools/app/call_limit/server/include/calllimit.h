#ifndef _CALLLIMIT_H
#define _CALLLIMIT_H

#define    MAX_CHAN     44

#define    CALL_TIME_LIMIT          0//3600          //ͨ��ʱ������ s
#define    DAY_CALL_LIMIT           0//100           //�պ��д�������
#define    DAY_ANSWER_LIMIT         0//80           //�ս�ͨ��������
#define    HOUR_CALL_LIMIT          0//50           //Сʱ���д�������
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
	CALL_TIME_CLEAN_TYPE_DAY,//��
	CALL_TIME_CLEAN_TYPE_WEEK,//����
	CALL_TIME_CLEAN_TYPE_MON,//��
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
	REGISTERING,      //ע����
	REGISTERED,       //ע��ɹ�
	REGISTERFAIL,     //ע��ʧ��
}sim_register_t;


typedef struct calllock_info_s{
	unsigned int call_failed_count;           //Cumulative count of consecutive call failures
	unsigned int call_fail_send_sms_times; //Cumulative send sms failed times 	
	unsigned char call_fail_mark_status;
	unsigned char call_fail_lock_status;		
}calllock_info_t;


typedef struct calllimit_info_s{
	struct tm day_call_last_clean_date;        //��һ���պ������������
	struct tm day_answer_last_clean_date;     //��һ���ս�ͨ���������
	struct tm hour_call_last_clean_date;     //��һ��ʱ�������������
	unsigned int hour_cur_calls;		        // ��ǰСʱ�ۼƺ��д���
	unsigned int day_cur_calls; 		      // �����ۼƺ��д���	
	unsigned int day_cur_answers;		       // �����ۼƽ�ͨ���� 
	unsigned char daycall_generation_flag;	//�պ��в�����־	
	unsigned char hourcall_generation_flag;	//Сʱ���в�����־	
	unsigned char call_answer_flag;		 //���н�ͨ��־ 
	unsigned char day_call_limit_flag;	  //�պ��д������Ʊ�־
	unsigned char day_answer_limit_flag;   //�ս�ͨ�������Ʊ�־
	unsigned char hour_call_limit_flag;	  //ʱ���д������Ʊ�־	
}calllimit_info_t;


typedef struct calltime_info_s{
	struct tm call_time_clean_date;    //���ʱ��	
	struct timeval start_call_time;     // ��ʼͨ��ʱ��	
	pthread_t tid;
	pthread_cond_t cond;
	pthread_mutex_t cond_lock;	
	int call_time_count;
	int call_time_remain;	
	unsigned char call_time_limit_flag;	
	unsigned char call_time_redis_flag;    //redis д����
	unsigned char call_answer_flag;		 //���н�ͨ��־ 
	unsigned char sms_warning_flag;         //ʱ�����Ÿ澯���
    unsigned char handup_flag;         //�Ҷ�ͨ����־
	unsigned char cond_flag;	
	unsigned char call_time_warning_flag;  //ʱ�����Ÿ澯���
}calltime_info_t;


typedef struct smslimit_info_s{
	struct tm day_last_date;                  //��һ�����������
	struct tm mon_last_date;                  //��һ�����������
	unsigned int day_cur_sms;
	unsigned int mon_cur_sms;
	unsigned char mon_sms_limit_flag;	       //���������Ʊ�־
	unsigned char day_sms_limit_flag;	      //���������Ʊ�־	
	unsigned char sms_limit_warning_flag;  //ʱ�����Ÿ澯���
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
	int call_time_total;                  //��������ʱ������
	int call_time_free;                   //��������ʱ��������С��ʱʱ��
	int call_time_step;                   //��������ʱ������
	int call_time_warning_num;	
	unsigned int call_time_settings;     // ����ͨ���趨����ʱ��	
	unsigned char call_time_sw;
	unsigned char call_time_single_sw;    //����ͨ��ʱ�����ƿ���
	unsigned char call_time_total_sw;     //ͨ����ʱ�����ƿ���
	unsigned char call_time_clean_sw;	
	unsigned char call_time_clean_type;  //����������������		
}calltime_conf_t;



typedef struct calllimit_conf_s {
	unsigned int day_calls_settings;	 // �պ������ƴ����趨
	unsigned int day_answer_setting;	 //�ս�ͨ�����趨	
	unsigned int hour_calls_settings;	 //Сʱ�������ƴ����趨
	unsigned char calllimit_switch;    //ͨ���ܿ���	
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
	unsigned int sim_reg_timeout;      //simע�ᳬʱʱ��(s)
	unsigned int total_using_time;     //ʹ��ʱ�� (min)
	unsigned int total_callout_time;   //����ʱ�� (min)
	unsigned int total_callout_count;  //��������
	unsigned int total_sms_count;      //���ŷ��ʹ���
	unsigned char sim_policy;          //sim�л�ģʽ
	unsigned char sim_switch_sw;       //�п�����
}simswitch_conf_t;


typedef struct chan_conf_s {
	calllock_conf_t calllock_cfg;      //������������
	calllimit_conf_t calllimit_cfg;    //������������ 
	calltime_conf_t calltime_cfg;      //����ͨ������
	smslimit_conf_t smslimit_cfg;      //������������
	simswitch_conf_t simswitch_cfg;    //ͨ���п�����
}chan_conf_t;


typedef struct simcard_info_s {
	calllock_info_t calllock_info;     //����������Ϣ
	calllimit_info_t calllimit_info;   //����������Ϣ
	calltime_info_t calltime_info;     //����ͨ����Ϣ
	smslimit_info_t smslimit_info;     //����������Ϣ
	char sim_ping_code[PING_CODE_LEN];  //PING��
	unsigned char sim_slot_sta;
	unsigned char sim_slot_laststa;
	unsigned char sim_sta;
}simcard_info_t;


typedef struct chan_simswitch_info_s{
	struct timeval start_call_time;     // ��ʼͨ��ʱ��	
	struct timeval last_using_time;   // ��һ��ͳ��ʹ��ʱ��	
	unsigned int curr_using_time;      //ʹ��ʱ�� 
	unsigned int using_time_count; 
	unsigned int curr_callout_time;    //����ʱ�� 
	unsigned int curr_callout_count;  //��������
	unsigned int curr_sms_count;       //���ŷ��ʹ���
	unsigned char call_generation_flag;
	unsigned char call_answer_flag;
	unsigned char sim_switch_flag;     //
	unsigned char using_time_flag;
	unsigned char reopen_switch_flag;
	unsigned char sim_switch_fail_flag;
}chan_simswitch_info_t;


typedef  struct running_chan_status_s{
	call_status_t call_sta;                //��ǰͨ������״̬
	call_status_t last_sta;               //��ǰͨ����һ�κ���״̬
	call_status_t sms_sta;                //��ǰͨ������״̬	
	unsigned char sms_limit_sta;         //��ǰͨ����������״̬
	unsigned char sys_limit_sta;    	   //��ǰͨ����������״̬	
	unsigned char call_time_limit_sta;	//ͨ��ʱ������״̬
	unsigned char call_lock_sta;          //��������״̬
	unsigned char call_mark_sta;          //�����״̬
	unsigned char sim_reg_sta;            //��ǰѡ�е�sim��ע��״̬
}running_chan_status_t;


typedef  struct  calllimit_chan_s {	
	simcard_info_t sim[SIM_NUM];	       //sim card״̬��Ϣ
	chan_conf_t cfg;                      //ͨ��������Ϣ
	chan_simswitch_info_t simswitch;      //ͨ���п�״̬ͳ�ƽṹ
	running_chan_status_t status;         //��ǰͨ�����״̬
    struct tm cur_date;                  //ϵͳ��ǰ������
    struct timeval cur_time;             //ϵͳ��ǰʱ��
    struct timeval sim_reg_time;         //���ѡ��simע��״̬�Ŀ�ʼʱ��
	pthread_t tid;                         //ͨ���߳�id
	pthread_mutex_t lock;                  //ͨ���߳���    
	unsigned int id;                       //channel id	
	void *equeue; 	                        //ͨ������ָ��
	unsigned char sim_idx;                //��ǰͨ��ʹ�õ�simcard	
}calllimit_chan_t;


typedef struct sys_info_cfg_s {
	unsigned int total_chans;            //��ͨ����
	unsigned char sys_type;               //ϵͳ����
	unsigned char IsRefleshFile;         //�Ƿ�ˢ��״̬�ļ�		
}sys_info_cfg_t;


typedef struct calllimit_s {	
	calllimit_chan_t chans[MAX_CHAN];           //ͨ�����ݽṹ	
	sys_info_cfg_t sys_info;       	         //ϵͳ��Ϣ
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
}calllimit_settings_t;


typedef struct calllimit_status_s {
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



