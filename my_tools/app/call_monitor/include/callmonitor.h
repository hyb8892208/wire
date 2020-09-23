#ifndef __CALL_MONITOR_H
#define __CALL_MONITOR_H
#include "queue.h"
#include <time.h>
#include <pthread.h>
typedef enum flag_e{
	FLAG_OFF=0,
	FLAG_ON=1
}flag_t;

typedef enum handle_type_e{
	HANDLE_CALL_INTERNAL = 1 << 0,//内部呼叫
	HANDLE_CALL_EXTERNAL = 1 << 1,//外部呼叫
	HANDLE_SMS_INTERNAL = 1 << 2,//内部短信
	HANDLE_SMS_EXTENAL = 1 << 3,//外部短信
	HANDLE_INTERNET = 1 << 4,//拨号上网
}handle_type_t;

typedef enum call_status_e
{
	CALL_DAIL,
	CALL_RING,
	CALL_ANSWER,
	CALL_HANGUP,
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

 typedef enum fixed_time_type_e{
 	FIXED_TIME_TYPE_UNKOWN = 0,
	FIXED_TIME_TYPE_DAYS,
	FIXED_TIME_TYPE_2DAYS,
	FIXED_TIME_TYPE_3DAYS,
	FIXED_TIME_TYPE_4DAYS,
	FIXED_TIME_TYPE_5DAYS,
	FIXED_TIME_TYPE_WEEK,
	FIXED_TIME_TYPE_MONTH
 }fixed_tiem_type_t;

typedef enum sim_state_e{
	SIM_STATE_INSERT,
	SIM_STATE_READY,
	SIM_STATE_CALL,
	SIM_STATE_REMOVE,
	SIM_STATE_UNKOWN,
}sim_state_t;

//呼叫统计数据
typedef struct chan_call_data_s{
	unsigned int cur_call_dur;//当前已呼出时长
	unsigned int cur_call_times;//当前已呼出次数
	unsigned int cur_call_answers;//当前已呼出应答次数
	enum call_status_e call_sta;
	enum call_status_e last_sta;//记录上一次的呼叫状态，用于判断呼叫是否被接通
	enum sim_state_e sim_state;//sim card state.
	struct timeval call_begin;//记录呼叫接通时间
	struct timeval call_end;//记录呼叫结束时间
	struct timespec last_online_time;
}chan_call_date_t;

typedef struct chan_config_s{
	unsigned int total_call_dur;//总呼出时长
	unsigned int total_call_times;//总呼叫测试
	unsigned int total_call_answers;//总呼出次数
	time_t online_time;//system runtime,config is minutus, but there is seconds
	unsigned char handle_type;//以何种方式处理, 可以同时存在多种处理方式
}chan_config_t;

typedef enum reset_flag_e{
	RESET_FLAG_OFF = 0,
	RESET_FLAG_ON,
}reset_flag_t;


/*typedef enum trigger_type_e{
	TRIGGER_TYPE_CALLDATA,//根据统计数据触发
	TRIGGER_TYPE_ONTIME,//定时触发
}trigger_type_t;
*/
typedef enum trigger_type_e{
	TRIGGER_TYPE_UNKOWN = 0,
	TRIGGER_TYPE_CALL_DUR = 1 << 0,
	TRIGGER_TYPE_CALL_ANSWERS = 1 << 1,
	TRIGGER_TYPE_CALL_TIMES = 1 << 2,
	TRIGGER_TYPE_ONLINE_TIME = 1 << 3,
}trigger_type_t;
typedef struct call_config_s{
	enum reset_flag_e reset_flag;//判断0点是否清除数据
	struct tm start_time;//自动功能时间段设置
	struct tm end_time;//自动功能时间段设置
	unsigned int max_time;//最小工作时长
	unsigned int min_time;//最大工作时长
	enum fixed_time_type_e fixed_time_type;
	struct tm fixed_time_calltime;
	unsigned char fixed_time_switch;
	unsigned char call_monitor_switch;
}call_config_t;

typedef struct chan_info_s{
	struct queue * equeue;//存放事件
	int id;
	struct chan_config_s conf;
	struct chan_call_data_s data;
	enum trigger_type_e trigger_type;
	pthread_mutex_t lock;
	unsigned char call_flag;//call flag
}chan_info_t;

typedef struct call_monitor_s{
	int sys_type;
	int total_chan;
	int trigger_flag;
	struct call_config_s glb_conf;//全局配置
	struct chan_info_s *chans;//通道信息
	pthread_mutex_t lock;
}call_monit_t;

/********** GSOAP BEGIN **********/
struct chan_data{
	unsigned int cur_call_dur;
	unsigned int cur_call_times;
	unsigned int cur_call_answers;
	time_t last_online_time;
};

struct chan_config{
	unsigned int total_call_dur;
	unsigned int total_call_times;
	unsigned int total_call_answers;
	time_t online_time;
	unsigned char handle_type;
};
int reload_config();

int flush_status_to_file();

int get_chan_conf(int chan_id, struct chan_config *config);

int get_chan_data(int chan_id, struct chan_data *data);

int call_monitor_create_thread(pthread_t *thred_id, void *data, void *(func)(void *data));
/*********GSOAP END***********/
#endif
