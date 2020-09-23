
#include "../include/header.h"
#include "../../gsoap/xml_client/soapH.h"
#include "../include/calllimit_api.h"
#include "../include/cli.h"
#include "json.h"


#define   STR_DIAL               "ExtraOutDial"
#define   STR_RING               "ExtraRingBack"
#define   STR_ANSWER             "ExtraConnect"
#define   STR_HANGUP             "ExtraHangup"
#define   STR_INDIAL             "ExtraDial"


#define   STR_SIMIN              "SimInserted"
#define   STR_SIMOUT             "ExtraDown"
#define   STR_SMS                 "SMSSendStatus"
#define   STR_SMS_REPORT         "SMSStatusReport"
#define   STR_RESTART            "FullyBooted"
#define   STR_READY           	  "ExtraReady"

 
#define    MAX_CHAN     44
#define    MAX_JSON_LEN  1024*80

typedef enum simcard_index_s {
	SIM1 = 0,
	SIM2 = 1,
	SIM3 = 2,
	SIM4 = 3,
	SIM_NUM,
}simcard_index_t;

typedef enum sim_status_s{                   
	SIM_NONE = 0,
	SIM_IDLE = 1,
	SIM_FREE = 2,
	SIM_BUSY = 3,
	SIM_ERROR = 4,
	SIM_REGING = 5,
}sim_status_t;


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
	SIM_OUT,//sim card remove
	SMS_SENDING,//sms is sending
	SMS_SUCCESS,//sms send success
	SMS_FAILED,//send call lock test sms failed
	SMS_SEND_FAILED,//send sms failed
    SMS_REPORT_SUCCESS,
    SMS_REPORT_FAILED,
    AST_REBOOT,
	CALL_UNKNOW,
}call_status_t;


const char * call_status_to_str(call_status_t call_status)
{
	switch (call_status) {
	case CALL_DAIL:
		return STR_DIAL;
	case CALL_RING:
		return STR_RING;
	case CALL_ANSWER:
		return STR_ANSWER;
	case CALL_HANDUP:
		return STR_HANGUP;
    case SIM_UP:
        return STR_READY;
	case SIM_IN:
		return STR_SIMIN;
    case SIM_OUT:
        return STR_SIMOUT;
    case SMS_SUCCESS:
        return STR_SMS;
    case CALL_IN:
        return STR_INDIAL;			
	default:
		return "Unknown";
	}
	
	return "Unknown";
}


const char * sim_status_to_str(unsigned char sim_status)
{
	switch (sim_status) {
	case SIM_NONE:
		return "None";
	case SIM_IDLE:
		return "Idle";
	case SIM_FREE:
		return "Free";
	case SIM_BUSY:
		return "Busy";
    case SIM_ERROR:
        return "Fail";	
    case SIM_REGING:
        return "Registing";
	default:
		return "Unknown";
	}	
	return "Unknown";
}


const char * sim_idx_to_str(unsigned char sim_idx)
{
	switch (sim_idx) {
	case SIM1:
		return "SIM1";
	case SIM2:
		return "SIM2";
	case SIM3:
		return "SIM3";
	case SIM4:
		return "SIM4";
	default:
		return "Unknown";
	}	
	return "Unknown";
}


int cli_usage_main(int argc, char **argv)
{
    printf("[?|help] -- show this menu\n");
	printf("show allstatus -- show allstatus\n");
    printf("show chn settings [chn] -- show channel settings\n");
    printf("show chn status [chn] -- show channel callstatus\n");
	printf("show sim status [chn] [sim_idx] -- show channel sim callstatus\n");
    printf("show chn dayremaincalls [chn] -- show channel day remain calls\n");	
    printf("set chn calltime [chn] [value] -- set channel calltime limit\n");
    printf("set chn daycalls [chn] [value] -- set channel day calls limit\n");
    printf("set chn dayanswers [chn] [value] -- set channel day answers limit\n");
    printf("set chn hourcalls [chn] [value] -- set channel hour calls limit\n");
    printf("set chn switch [chn] [ENABLE|DISABLE] -- set channel call limit swtich\n");
	printf("set filewrite switch [ENABLE|DISABLE] -- set status file write swtich\n");	
    printf("clean chn daycalls [chn] -- clean channel day calls limit\n");
    printf("clean chn dayanswers [chn] -- clean channel day answers limit\n");
    printf("clean chn hourcalls [chn] -- clean channel hour calls limit\n");
    printf("clean chn limited [chn] -- clean channel all call limit status\n");
	printf("clean chn smslimit [chn] -- clean channel sms limit status\n");
    printf("set chn unlock [chn] -- unlock channel\n");
    printf("set chn unmark [chn] -- unmark channel\n");
    printf("set chn reload -- reload config file\n");
	printf("set chn reflesh -- reload reflesh file\n");
    printf("set chn detect [chn] [ENABLE|DISABLE] -- set channel lock detect\n");
    printf("set chn markflag [chn] [ENABLE|DISABLE] -- set channel mark swtich\n");
    printf("set chn markcount [chn] [value] -- set channel call filed count then set mark\n");
    printf("set chn lockflag [chn] [ENABLE|DISABLE] -- set channel lock swtich\n");
    printf("set chn lockcount [chn] [value] -- set channel call filed count then set locked\n");
    printf("set chn smsflag [chn] [ENABLE|DISABLE] -- set channel send sms test swtich\n");
    printf("set chn smsreport [chn] [ENABLE|DISABLE] -- set channel check sms report swtich\n");
    printf("set chn smscount [chn] [value] -- set channel send sms filed count then set locked\n");
    printf("set chn smssender [chn] [value] -- set channel send sms test addr\n");
    printf("set chn smsmsg [chn] [value] -- set channel send sms test msg\n");
	printf("set chn smsswitch [chn] [value] -- set channel sms swtich\n");
	printf("set chn daysms [chn] [value] -- set channel day sms limit\n");
	printf("set chn monsms [chn] [value] -- set channel month sms limit\n");
	printf("set chn successflag [chn] [value] -- set channel sms success cnt flag\n");
	printf("set chn monsmscnt [chn] [value] -- set channel month sms cnt\n");
	printf("set chn daysmscnt [chn] [value] -- set channel day sms cnt\n");
	printf("set chn calltimecnt [chn] [value] -- set channel call time  cnt\n");
	printf("set chn calltimetotal [chn] [value] -- set channel call time total\n");
	printf("set sim limitsta [chn] [sim_idx] [calllimit] [calllock] [callmark] [smslimit] -- set channel limit status\n");
	printf("set sim unlimit [chn] [sim_idx] -- unlimit channel simcard\n");
	printf("set sim unlock [chn] [sim_idx] -- unlock channel simcard\n");
	printf("set sim unmark [chn] [sim_idx] -- unmark channel simcard\n");
	printf("set sim calltimecnt [chn] [sim_idx] [value] -- set channel simcard  call time cnt\n");
	printf("set sim daysmscnt [chn] [sim_idx] [value] -- set channel simcard day sms cnt\n");
	printf("set sim monsmscnt [chn] [sim_idx] [value] -- set channel simcard mon sms cnt\n");
	printf("set sim callfailedcnt [chn] [sim_idx] [value] -- set channel call failed cnt\n");
	printf("set sim calllimitcnt [chn] [sim_idx] [daycalls] [dayanswers] [hourcalls] -- set channel calls \n");
	printf("set sim pincode [chn] [sim_idx] [pincode] -- set channel simcard pincode\n");
	printf("get sim pincode [chn] [sim_idx] -- get channel simcard pincode\n");
	printf("get chan pincode [chn] -- get current channel pincode \n");	
	printf("set allsim unlimit -- unlimit all simcard\n");
	printf("set allsim unlock -- unlock all simcard\n");
	printf("set allsim unmark -- unmark all simcard\n");
	printf("set allsim unsmslimit -- unsmslimit all simcard\n");
	printf("switch chan simcard [chn] [sim_idx] -- switch chan simcard\n");	
    return 0;
}


int cli_get_channel_settings(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	calllimit_setting_t setting;
	
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_get_channel_settings(channel_id-1, &setting);
	if(0 == res) {
		printf("       call_time_switch: %s\n", setting.call_time_sw?"ENABLE":"DISABLE");
		printf("call_time_single_switch: %s\n", setting.call_time_single_sw?"ENABLE":"DISABLE");
		printf(" call_time_total_switch: %s\n", setting.call_time_total_sw?"ENABLE":"DISABLE");
		printf("              call_time: %d\n", setting.call_time_settings);
		printf("        call_time_total: %d\n", setting.call_time_total);
		printf("         call_time_free: %d\n", setting.call_time_free);
		printf("         call_time_step: %d\n", setting.call_time_step);
		printf(" call_time_clean_switch: %s\n", setting.call_time_clean_sw?"ENABLE":"DISABLE");
		printf("   call_time_clean_type: %d\n", setting.call_time_settings);
		printf("             day_answer: %d\n", setting.day_answer_setting);
		printf("              day_calls: %d\n", setting.day_calls_settings);
		printf("             hour_calls: %d\n", setting.hour_calls_settings);
		printf("       calllimit_switch: %s\n", setting.calllimit_switch?"ENABLE":"DISABLE");
        printf("        detect sim lock: %s\n", setting.call_detect_flag?"ENABLE":"DISABLE");
        printf("            mark switch: %s\n", setting.call_fail_mark_flag?"ENABLE":"DISABLE");
        printf("        fail mark count: %d\n", setting.call_fail_mark_count);
	    printf("            lock switch: %s\n", setting.call_fail_lock_flag?"ENABLE":"DISABLE");
	    printf("        fail lock count: %d\n", setting.call_fail_lock_count);
	    printf("        sms test switch: %s\n", setting.call_fail_lock_sms_flag?"ENABLE":"DISABLE");
	    printf("       sms report check: %s\n", setting.call_fail_lock_sms_report_flag?"ENABLE":"DISABLE");
	    printf("       sms failed count: %d\n", setting.call_fail_lock_sms_count);
	    printf("               sms addr: %s\n", setting.call_fail_lock_sms_callee);
	    printf("                sms msg: %s\n", setting.call_fail_lock_sms_msg);
		printf("        smslimit_swtich: %s\n",setting.smslimit_switch?"ENABLE":"DISABLE"); 
		printf("                day_sms: %d\n",setting.day_sms_settings);  
		printf("                mon_sms: %d\n",setting.mon_sms_settings);  
		printf("         sms_clean_date: %d\n",setting.sms_clean_date); 
		printf("     sms_warning_switch: %s\n", setting.sms_warning_switch?"ENABLE":"DISABLE");
		printf("   smslimit_mon_warning_num: %d\n", setting.smslimit_mon_warning_num);
		printf("smslimit_mon_warning_callee: %s\n", setting.smslimit_mon_warning_callee);
		printf("   smslimit_mon_warning_msg: %s\n", setting.smslimit_mon_warning_msg);
		printf("      call_time_warning_num: %d\n", setting.call_time_warning_num);
		printf("   call_time_warning_callee: %s\n", setting.call_time_warning_callee);
		printf("      call_time_warning_msg: %s\n", setting.call_time_warning_msg);
		printf("      sim_policy: %s\n", setting.sim_policy?"Desc":"Asc");
		printf("   sim_switch_sw: %s\n", setting.sim_switch_sw?"ENABLE":"DISABLE");
		printf("      sim_reg_timeout: %d\n", setting.sim_reg_timeout);		
		printf("      total_callout_count: %d\n", setting.total_callout_count);	
		printf("      total_callout_time: %d\n", setting.total_callout_time);	
		printf("      total_sms_count: %d\n", setting.total_sms_count);	
		printf("      total_using_time: %d\n", setting.total_using_time);	
	}
    return 0;
}


int cli_get_channel_status(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	calllimit_statu_t status;
	
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_get_channel_status(channel_id-1, &status);

	if(0 == res) {
		printf("              callsta: %s\n", call_status_to_str((call_status_t)status.call_sta));
		printf("call_time_limit_flag:%s\n",status.call_time_limit_flag?"YES":"NO" );
		printf(" call_time_limit_sta:%s\n",status.call_time_limit_sta?"LIMIT":"UNLIMIT" );
		printf("call_time_count:%d\n",status.call_time_count);
		printf("call_time_remain:%d\n",status.call_time_remain );
		printf("call_time_clean_date:%s\n",status.call_time_clean_date );
		printf("     call_answer_flag: %s\n", status.call_answer_flag?"YES":"NO");
		printf(" call_generation_flag: %s\n", status.call_generation_flag?"YES":"NO");
		printf("  day_call_limit_flag: %s\n", status.day_call_limit_flag?"YES":"NO");
		printf("day_answer_limit_flag: %s\n", status.day_answer_limit_flag?"YES":"NO");
		printf(" hour_call_limit_flag: %s\n", status.hour_call_limit_flag?"YES":"NO");
		printf("            limit_sta: %s\n", status.sys_limit_sta?"LIMIT":"UNLIMIT");
		printf("      day_total_calls: %d\n", status.day_cur_calls);
		printf("     hour_total_calls: %d\n", status.hour_cur_calls);
		printf("    day_total_answers: %d\n", status.day_cur_answers);	
		printf("     call_failed_count: %d\n",status.call_failed_count);  
		printf("call_fail_send_sms_times:%d\n",status.call_fail_send_sms_times);  
		printf("call_fail_mark_status:%d\n",status.call_fail_mark_status);  
		printf("call_fail_lock_status: %d\n",status.call_fail_lock_status);  
		printf("          day_cur_sms: %d\n",status.day_cur_sms);  
		printf("         mon_cur_sms: %d\n",status.mon_cur_sms);  
		printf("         sms_limit_sta: %d\n",status.sms_limit_sta);  
		printf("	curr_callout_count: %d\n",status.curr_callout_count);  
		printf("     curr_callout_time: %d\n",status.curr_callout_time);  
		printf("        curr_sms_count: %d\n",status.curr_sms_count);  
		printf("       curr_using_time: %d\n",status.curr_using_time);  
 	}
    return 0;
}


int cli_get_channel_sim_status(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	int sim_idx = -1;
	calllimit_statu_t status;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx = atoi(argv[4]);

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_get_channel_sim_status(channel_id-1, sim_idx-1, &status);

	if(0 == res) {
		printf("              callsta: %s\n", call_status_to_str((call_status_t)status.call_sta));
		printf("call_time_limit_flag:%s\n",status.call_time_limit_flag?"YES":"NO" );
		printf(" call_time_limit_sta:%s\n",status.call_time_limit_sta?"LIMIT":"UNLIMIT" );
		printf("call_time_count:%d\n",status.call_time_count);
		printf("call_time_remain:%d\n",status.call_time_remain );
		printf("call_time_clean_date:%s\n",status.call_time_clean_date );
		printf("     call_answer_flag: %s\n", status.call_answer_flag?"YES":"NO");
		printf(" call_generation_flag: %s\n", status.call_generation_flag?"YES":"NO");
		printf("  day_call_limit_flag: %s\n", status.day_call_limit_flag?"YES":"NO");
		printf("day_answer_limit_flag: %s\n", status.day_answer_limit_flag?"YES":"NO");
		printf(" hour_call_limit_flag: %s\n", status.hour_call_limit_flag?"YES":"NO");
		printf("            limit_sta: %s\n", status.sys_limit_sta?"LIMIT":"UNLIMIT");
		printf("      day_total_calls: %d\n", status.day_cur_calls);
		printf("     hour_total_calls: %d\n", status.hour_cur_calls);
		printf("    day_total_answers: %d\n", status.day_cur_answers);	
		printf("     call_failed_count: %d\n",status.call_failed_count);  
		printf("call_fail_send_sms_times:%d\n",status.call_fail_send_sms_times);  
		printf("call_fail_mark_status:%d\n",status.call_fail_mark_status);  
		printf("call_fail_lock_status: %d\n",status.call_fail_lock_status);  
		printf("          day_cur_sms: %d\n",status.day_cur_sms);  
		printf("         mon_cur_sms: %d\n",status.mon_cur_sms);  
		printf("         sms_limit_sta: %d\n",status.sms_limit_sta);  
		printf("               sim_idx: %s\n",sim_idx_to_str(status.sim_idx)); 
		printf("               sim_sta: %s\n", sim_status_to_str(status.sim_sta)); 
		
 	}
    return 0;
}



static int sim_status_to_jsion(calllimit_statu_t *in, char * outstr, int chans, int inlen)
{
	int i = 0;
	int j = 0;
	int outlen = 0;
	calllimit_statu_t *psta;
	char sim_context[20] = {0};

    struct json_object *root_obj = NULL;
	struct json_object *sim_info_object = NULL;

	root_obj = json_object_new_object();
	if (NULL == root_obj) {
		printf("new jsion root_obj failed.\n");
		return 1;
	}

	for(i=0; i<chans; i++) {
		for(j=0; j<SIM_NUM; j++){
			psta = &in[i*SIM_NUM+j];
			sim_info_object = json_object_new_object();
			if (NULL == sim_info_object) {
				printf("[%d][%d]new jsion sim_info_object failed.\n", i+1, j+1);
				return 1;
			}
			if(psta) {
				json_object_object_add(sim_info_object, "day_cur_sms", json_object_new_int((int)psta->day_cur_sms));
				json_object_object_add(sim_info_object, "mon_cur_sms", json_object_new_int((int)psta->mon_cur_sms));
				json_object_object_add(sim_info_object, "limit_sta", psta->sys_limit_sta?json_object_new_string("LIMIT"):json_object_new_string("UNLIMIT"));
				json_object_object_add(sim_info_object, "call_fail_lock_status", json_object_new_int((int)psta->call_fail_lock_status));
				json_object_object_add(sim_info_object, "call_fail_mark_status", json_object_new_int((int)psta->call_fail_mark_status));
				json_object_object_add(sim_info_object, "call_time_limit_sta", psta->call_time_limit_sta?json_object_new_string("LIMIT"):json_object_new_string("UNLIMIT"));
				json_object_object_add(sim_info_object, "sms_limit_sta", json_object_new_int((int)psta->sms_limit_sta));
				json_object_object_add(sim_info_object, "day_total_calls", json_object_new_int((int)psta->day_cur_calls));
				json_object_object_add(sim_info_object, "hour_total_calls", json_object_new_int((int)psta->hour_cur_calls));
				json_object_object_add(sim_info_object, "day_total_answers", json_object_new_int((int)psta->day_cur_answers));
				json_object_object_add(sim_info_object, "call_failed_count", json_object_new_int((int)psta->call_failed_count));
				json_object_object_add(sim_info_object, "call_time_count", json_object_new_int((int)psta->call_time_count));
				json_object_object_add(sim_info_object, "sim_sta", json_object_new_int((int)psta->sim_sta));
				json_object_object_add(sim_info_object, "curr_callout_count", json_object_new_int((int)psta->curr_callout_count));
				json_object_object_add(sim_info_object, "curr_callout_time", json_object_new_int((int)psta->curr_callout_time/60));
				json_object_object_add(sim_info_object, "curr_sms_count", json_object_new_int((int)psta->curr_sms_count));
				json_object_object_add(sim_info_object, "curr_using_time", json_object_new_int((int)psta->curr_using_time/60));		
			}
			sprintf(sim_context, "%d-%d", i+1, j+1);
			json_object_object_add(root_obj, sim_context, sim_info_object);
			
		}

	}
	outlen = strlen(json_object_to_json_string(root_obj));
	if(outlen > inlen) {
//		printf("output json len(%d) is out of range\n", outlen);
		outlen = inlen;
	}
	memcpy(outstr, json_object_to_json_string(root_obj), outlen);
	json_object_put(root_obj);//free
	return 0;
}


int cli_get_all_status(int argc, char **argv)
{
	int res = -1;
	calllimit_statu_t status[MAX_CHAN*SIM_NUM];
	char out_jsion[MAX_JSON_LEN] = {0};
	
    if ( argc != 2 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	res = calllimit_get_all_status(status);
	if(res > 0) {
		sim_status_to_jsion(&status, out_jsion, res, sizeof(out_jsion));
		printf("%s\n", out_jsion);
 	}
    return 0;
}



int cli_set_channel_calltime(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int call_time = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	call_time = atoi(argv[4]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_calltime(channel_id-1, call_time);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_set_channel_daycalls(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int day_calls = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	day_calls = atoi(argv[4]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_daycalls(channel_id-1, day_calls);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_dayanswers(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int day_answers = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	day_answers = atoi(argv[4]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_dayanswers(channel_id-1, day_answers);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_hourcalls(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int hour_calls = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	hour_calls = atoi(argv[4]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_hourcalls(channel_id-1, hour_calls);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_set_channel_call_switch(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int call_switch = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);

	if(!strcmp(argv[4], "ENABLE")) 
		call_switch = 1;
	else if(!strcmp(argv[4], "DISABLE"))
		call_switch = 0;
	else {
		printf("set call_switch fail, please choose ENABLE/DISABLE\n");
		return -1;
	}
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_call_switch(channel_id-1, call_switch);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_filewrite_switch(int argc, char **argv)
{
	int res = -1;
	unsigned int file_switch = 0;
	
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	if(!strcmp(argv[3], "ENABLE")) 
		file_switch = 1;
	else if(!strcmp(argv[3], "DISABLE"))
		file_switch = 0;
	else {
		printf("set file_switch fail, please choose ENABLE/DISABLE\n");
		return -1;
	}
		

	res = calllimit_set_filewrite_switch(file_switch);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_clean_channel_day_calls(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_clean_channel_day_calls(channel_id-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_clean_channel_day_answers(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_clean_channel_day_answers(channel_id-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_clean_channel_hour_calls(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_clean_channel_hour_calls(channel_id-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_clean_channel_limit(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_clean_channel_limit(channel_id-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_unlock(int argc, char **argv)
{
    int channel_id = -1;
    int result;

    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

    channel_id = atoi(argv[3]);

    if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
    }

    result = calllimit_set_channel_unlock(channel_id-1);
    if(result < 0)
        printf("result:[%d]\n", result);


    return 0;
}

int cli_set_channel_unmark(int argc, char **argv)
{
    int channel_id = -1;
    int result;

    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

    channel_id = atoi(argv[3]);

    if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
    }

    result = calllimit_set_channel_unmark(channel_id-1);
    if(result < 0)
        printf("result:[%d]\n", result);

    return 0;
}


int cli_set_channel_reload(int argc, char **argv)
{
 //   int channel_id = -1;
	int result;
	
    if ( argc != 3 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	
    result = calllimit_reload_cfg();
    if(result < 0)
        printf("result:[%d]\n", result);
    
    return 0;
}

int cli_set_channel_reflesh_status(int argc, char **argv)
{
 //   int channel_id = -1;
	int result;
	
    if ( argc != 3 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	
    result = calllimit_reflesh_status_cfg();
    if(result < 0)
        printf("result:[%d]\n", result);
    
    return 0;
}

int cli_set_channel_detect(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int detect = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);

	if(!strcmp(argv[4], "ENABLE")) 
		detect = 0;
	else if(!strcmp(argv[4], "DISABLE"))
		detect = 1;
	else {
		printf("set call detect fail, please choose ENABLE/DISABLE\n");
		return -1;
	}
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_detect(channel_id-1, detect);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_mark_flag(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int flag = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);

	if(!strcmp(argv[4], "ON")) 
		flag = 0;
	else if(!strcmp(argv[4], "OFF"))
		flag = 1;
	else {
		printf("set call detect fail, please choose ON/OFF\n");
		return -1;
	}
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_mark_flag(channel_id-1, flag);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_mark_count(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int count = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	count = atoi(argv[4]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_mark_count(channel_id-1, count);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_lock_flag(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int flag = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);

	if(!strcmp(argv[4], "ON")) 
		flag = 0;
	else if(!strcmp(argv[4], "OFF"))
		flag = 1;
	else {
		printf("set call detect fail, please choose ON/OFF\n");
		return -1;
	}
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_lock_flag(channel_id-1, flag);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_lock_count(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int count = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	count = atoi(argv[4]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_lock_count(channel_id-1, count);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_lock_sms_flag(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int flag = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);

	if(!strcmp(argv[4], "ON")) 
		flag = 0;
	else if(!strcmp(argv[4], "OFF"))
		flag = 1;
	else {
		printf("set sms fail, please choose ON/OFF\n");
		return -1;
	}
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_lock_sms_flag(channel_id-1, flag);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_lock_smsreport_flag(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int flag = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);

	if(!strcmp(argv[4], "ON")) 
		flag = 0;
	else if(!strcmp(argv[4], "OFF"))
		flag = 1;
	else {
		printf("set sms report fail, please choose ON/OFF\n");
		return -1;
	}
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_lock_sms_report_flag(channel_id-1, flag);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_lock_sms_count(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	unsigned int count = 0;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	count = atoi(argv[4]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_lock_sms_count(channel_id-1, count);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_lock_sms_sender(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_lock_sms_callee(channel_id-1, argv[4]);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_lock_sms_msg(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_channel_lock_sms_msg(channel_id-1, argv[4]);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_clean_channel_sms_limit(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_clena_channel_sms_limit(channel_id-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_mon_sms_settings(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_mon_sms_limit_settings(channel_id-1, atoi(argv[4]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_day_sms_settings(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_day_sms_limit_settings(channel_id-1, atoi(argv[4]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_channel_sms_limit_switch(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if(channel_id == -1)
		channel_id = 0;

	res = calllimit_set_sms_limit_switch(channel_id-1, atoi(argv[4]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_sms_limit_success_flag(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if(channel_id == -1)
		channel_id = 0;

	res = calllimit_set_sms_limit_success_flag(channel_id-1, atoi(argv[4]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_mon_sms_cnt(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
		
	if(channel_id == -1)
		channel_id = 0;

	res = calllimit_set_mon_sms_cnt(channel_id-1, atoi(argv[4]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_day_sms_cnt(int argc, char **argv)
{
	int channel_id = -1;
	int res = -1;
	
    if ( argc != 5 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	
	if(channel_id == -1)
		channel_id = 0;
	res = calllimit_set_day_sms_cnt(channel_id-1, atoi(argv[4]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

 int cli_set_call_time_cnt(int argc, char ** argv){
	int channel_id = -1;
	int res = -1;
	
	if ( argc != 5 )
	{
	    printf("param error: argc = %d\r\n", argc);
	    return 0;
	}	

	channel_id = atoi(argv[3]);
	
	if(channel_id == -1)
		channel_id = 0;
	res = calllimit_set_call_time_cnt(channel_id-1, atoi(argv[4]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
	return 0;
 }
 int cli_set_call_time_total(int argc, char ** argv){
	int channel_id = -1;
	int res = -1;
	
	if ( argc != 5 )
	{
	    printf("param error: argc = %d\r\n", argc);
	    return 0;
	}	

	channel_id = atoi(argv[3]);
	
	if(channel_id == -1)
		channel_id = 0;
	res = calllimit_set_call_time_total(channel_id-1, atoi(argv[4]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
	return 0;
 }


int cli_set_sim_limitsta(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	unsigned int calllimit_flag = 0;
	unsigned int calllock_flag = 0;
	unsigned int callmark_flag = 0;
	unsigned int smslimit_flag = 0;
	
    if ( argc != 9 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx =  atoi(argv[4]);
	calllimit_flag = atoi(argv[5]);
	calllock_flag = atoi(argv[6]);
	callmark_flag = atoi(argv[7]);
	smslimit_flag = atoi(argv[8]);
			
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}

	if(calllimit_flag > 1 || calllock_flag > 1 || callmark_flag > 1 || smslimit_flag > 1) {
        printf("set flag status(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_sim_limitsta(channel_id-1, sim_idx-1, calllimit_flag, calllock_flag, callmark_flag, smslimit_flag);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_set_sim_unlimit(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 5 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx =  atoi(argv[4]);
			
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_sim_unlimit(channel_id-1, sim_idx-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_sim_unlock(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 5 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx =  atoi(argv[4]);
			
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_sim_unlock(channel_id-1, sim_idx-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_sim_unmark(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 5 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx =  atoi(argv[4]);
			
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_set_sim_unmark(channel_id-1, sim_idx-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_all_simcard_unlimit(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 3 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	res = calllimit_all_simcard_unlimit();
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_all_simcard_unlock(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 3 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	res = calllimit_all_simcard_unlock();
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_all_simcard_unmark(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 3 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	res = calllimit_all_simcard_unmark();
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_all_simcard_unsmslimit(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 3 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	res = calllimit_all_simcard_unsmslimit();
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_switch_chan_simcard(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 5 ) {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx =  atoi(argv[4]);
			
	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}

	res = calllimit_switch_chan_simcard(channel_id-1, sim_idx-1);
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_set_sim_daysmscnt(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 6 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx = atoi(argv[4]);

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}	
	
	res = calllimit_set_sim_daysmscnt(channel_id-1, sim_idx-1, atoi(argv[5]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_sim_monsmscnt(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 6 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx = atoi(argv[4]);

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}	
	
	res = calllimit_set_sim_monsmscnt(channel_id-1, sim_idx-1, atoi(argv[5]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}

int cli_set_sim_callfailedcnt(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 6 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx = atoi(argv[4]);

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}	
	
	res = calllimit_set_sim_callfailedcnt(channel_id-1, sim_idx-1, atoi(argv[5]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_set_sim_calllimitcnt(int argc, char **argv)
{
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	
    if ( argc != 8 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }	

	channel_id = atoi(argv[3]);
	sim_idx = atoi(argv[4]);

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
        printf("channel_id(%d) out of range.\n", channel_id);
        return 0;		
	}
	
	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
        printf("sim_idx(%d) out of range.\n", channel_id);
        return 0;		
	}	
	
	res = calllimit_set_sim_calllimitcnt(channel_id-1, sim_idx-1, atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
	if(res < 0) {
		printf("FAIL\n");
	} else {
		printf("OK\n");
	}
    return 0;
}


int cli_set_sim_calltimecnt(int argc, char ** argv){
   int channel_id = -1;
   int sim_idx = -1;
   int res = -1;
   
   if ( argc != 6 )
   {
	   printf("param error: argc = %d\r\n", argc);
	   return 0;
   }   
   
   channel_id = atoi(argv[3]);
   sim_idx = atoi(argv[4]);
   
   if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
	   printf("channel_id(%d) out of range.\n", channel_id);
	   return 0;	   
   }
   
   if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
	   printf("sim_idx(%d) out of range.\n", channel_id);
	   return 0;	   
   }   

   res = calllimit_set_sim_calltimecnt(channel_id-1, sim_idx-1, atoi(argv[5]));
   if(res < 0) {
	   printf("FAIL\n");
   } else {
	   printf("OK\n");
   }
   return 0;
}

int cli_set_sim_pincode(int argc, char ** argv){
	int channel_id = -1;
	int sim_idx = -1;
	char pincode[128] = {0};
	int res = -1;

	if ( argc != 6 )
	{
	   printf("param error: argc = %d\r\n", argc);
	   return 0;
	}   

	channel_id = atoi(argv[3]);
	sim_idx = atoi(argv[4]);
	if(strcmp(argv[5], "0")){
		memcpy(pincode, argv[5], strlen(argv[5]));
	}

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
	   printf("channel_id(%d) out of range.\n", channel_id);
	   return 0;	   
	}

	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
	   printf("sim_idx(%d) out of range.\n", channel_id);
	   return 0;	   
	}   

	res = calllimit_set_sim_pincode(channel_id-1, sim_idx-1, pincode);
	if(res < 0) {
	   printf("FAIL\n");
	} else {
	   printf("OK\n");
	}
	return 0;
}

int cli_get_sim_pincode(int argc, char ** argv){
	int channel_id = -1;
	int sim_idx = -1;
	int res = -1;
	sim_pincode__t pincode;
		
	if ( argc != 5 )
	{
	   printf("param error: argc = %d\r\n", argc);
	   return 0;
	}   

	channel_id = atoi(argv[3]);
	sim_idx = atoi(argv[4]);

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
	   printf("channel_id(%d) out of range.\n", channel_id);
	   return 0;	   
	}

	if((sim_idx > SIM_NUM) || (sim_idx <= 0)) {
	   printf("sim_idx(%d) out of range.\n", channel_id);
	   return 0;	   
	}   

	res = calllimit_get_sim_pincode(channel_id-1, sim_idx-1, &pincode);
	if(res == 0) {
		printf("%s\n", pincode.pincode);
	}

	return 0;
}

int cli_get_chan_pincode(int argc, char ** argv){
	int channel_id = -1;
	int res = -1;
	sim_pincode__t pincode;

	if ( argc != 4 )
	{
	   printf("param error: argc = %d\r\n", argc);
	   return 0;
	}   

	channel_id = atoi(argv[3]);

	if((channel_id > MAX_CHAN) || (channel_id <= 0)) {
	   printf("channel_id(%d) out of range.\n", channel_id);
	   return 0;	   
	}

	res = calllimit_get_channel_pincode(channel_id-1, &pincode);
	if(res == 0) {
		printf("%s\n", pincode.pincode);
	}
	return 0;
}

int cli_get_channel_remain_info(int argc, char **argv){
	int channel_id = 0;
	int res = -1;
	int i = 0;
	struct calllimit_get_remain_info remain;
	if(argc != 4){
		printf("param error:argc = %d\r\n", argc);
		return 0;
	}
	channel_id = atoi(argv[3]);
	channel_id = channel_id & 0xFFFF;
	res = calllimit_get_remain_info(channel_id, &remain);
	if(res < 0){ 
		  printf("get remain info FAIL\n"); 
	   }else{ 
		  if(channel_id != 0xFFFF){ 
			 printf("[%d]\n", channel_id); 
			 if(remain.__ptr[0].day_remain_calls == 0xFFFF) 
				printf("day_remain_calls=\n"); 
			 else 
				printf("day_remain_calls=%d\n", remain.__ptr[0].day_remain_calls); 
		  }else{ 
			 for(i = 0; i < remain.__size;i++){ 
				printf("[%d]\n", i+1); 
				if(remain.__ptr[i].day_remain_calls == 0xFFFF) 
				   printf("day_remain_calls=\n"); 
				else 
				   printf("day_remain_calls=%d\n", remain.__ptr[i].day_remain_calls); 
			 } 
		  } 
	 }
	return 0;
}

void cli_reg_calllimit(void)
{
    cb_func_reg((char *)"show chn settings", cli_get_channel_settings);
    cb_func_reg((char *)"show chn status", cli_get_channel_status);
	cb_func_reg((char *)"show sim status", cli_get_channel_sim_status);
	cb_func_reg((char *)"show allstatus", cli_get_all_status);
    cb_func_reg((char *)"show chn dayremaincalls",cli_get_channel_remain_info);
    cb_func_reg((char *)"set chn calltime", cli_set_channel_calltime);
    cb_func_reg((char *)"set chn daycalls", cli_set_channel_daycalls);
    cb_func_reg((char *)"set chn dayanswers", cli_set_channel_dayanswers);
    cb_func_reg((char *)"set chn hourcalls", cli_set_channel_hourcalls);
    cb_func_reg((char *)"set chn switch", cli_set_channel_call_switch);
    cb_func_reg((char *)"clean chn dayanswers", cli_clean_channel_day_answers);
    cb_func_reg((char *)"clean chn daycalls", cli_clean_channel_day_calls);
    cb_func_reg((char *)"clean chn hourcalls", cli_clean_channel_hour_calls);
	cb_func_reg((char *)"clean chn limited", cli_clean_channel_limit);
	cb_func_reg((char *)"set filewrite switch", cli_set_filewrite_switch);	
    cb_func_reg((char *)"set chn unlock", cli_set_channel_unlock);
    cb_func_reg((char *)"set chn unmark",cli_set_channel_unmark);
    cb_func_reg((char *)"set chn reload", cli_set_channel_reload);
	cb_func_reg((char *)"set chn reflesh", cli_set_channel_reflesh_status);
    cb_func_reg((char *)"set chn detect", cli_set_channel_detect);
    cb_func_reg((char *)"set chn markflag", cli_set_channel_mark_flag);
    cb_func_reg((char *)"set chn markcount", cli_set_channel_mark_count);
    cb_func_reg((char *)"set chn lockflag", cli_set_channel_lock_flag);
    cb_func_reg((char *)"set chn lockcount", cli_set_channel_lock_count);
    cb_func_reg((char *)"set chn smsflag",cli_set_channel_lock_sms_flag);
    cb_func_reg((char *)"set chn smsreport",cli_set_channel_lock_smsreport_flag);
    cb_func_reg((char *)"set chn smscount",cli_set_channel_lock_sms_count);
    cb_func_reg((char *)"set chn smssender",cli_set_channel_lock_sms_sender);
    cb_func_reg((char *)"set chn smsmsg",cli_set_channel_lock_sms_msg);
	cb_func_reg((char *)"set chn daysms", cli_set_channel_day_sms_settings);
	cb_func_reg((char *)"set chn monsms", cli_set_channel_mon_sms_settings);
	cb_func_reg((char *)"set chn smsswtich",cli_set_channel_sms_limit_switch);
	cb_func_reg((char *)"clean chn smslimit", cli_clean_channel_sms_limit);
	cb_func_reg((char *)"set chn successflag", cli_set_sms_limit_success_flag);
	cb_func_reg((char *)"set chn monsmscnt",cli_set_mon_sms_cnt); 
	cb_func_reg((char *)"set chn daysmscnt", cli_set_day_sms_cnt); 
	cb_func_reg((char *)"set chn calltimecnt", cli_set_call_time_cnt);
	cb_func_reg((char *)"set chn calltimetotal", cli_set_call_time_total);
	
	cb_func_reg((char *)"set sim limitsta", cli_set_sim_limitsta);
	cb_func_reg((char *)"set sim unlimit", cli_set_sim_unlimit);
	cb_func_reg((char *)"set sim unlock", cli_set_sim_unlock);
	cb_func_reg((char *)"set sim unmark",cli_set_sim_unmark); 
	cb_func_reg((char *)"set sim calltimecnt", cli_set_sim_calltimecnt); 
	cb_func_reg((char *)"set sim daysmscnt", cli_set_sim_daysmscnt);
	cb_func_reg((char *)"set sim monsmscnt", cli_set_sim_monsmscnt);
	cb_func_reg((char *)"set sim callfailedcnt", cli_set_sim_callfailedcnt);
	cb_func_reg((char *)"set sim calllimitcnt", cli_set_sim_calllimitcnt);

	cb_func_reg((char *)"set sim pincode", cli_set_sim_pincode);
	cb_func_reg((char *)"get sim pincode", cli_get_sim_pincode);
	cb_func_reg((char *)"get chan pincode", cli_get_chan_pincode);

	cb_func_reg((char *)"set allsim unlimit",cli_all_simcard_unlimit); 
	cb_func_reg((char *)"set allsim unlock", cli_all_simcard_unlock); 
	cb_func_reg((char *)"set allsim unmark", cli_all_simcard_unmark);
	cb_func_reg((char *)"set allsim unsmslimit", cli_all_simcard_unsmslimit);	
	cb_func_reg((char *)"switch chan simcard", cli_switch_chan_simcard);	
}

int cli_test_one(int argc, char **argv){
	if(strcmp(argv[0], "show") == 0) {
		if(strcmp(argv[1], "chn") == 0){
			if(strcmp(argv[2], "settings") == 0){
				cli_get_channel_settings(argc, argv);
			} else if(strcmp(argv[2], "status") == 0) {
				cli_get_channel_status(argc, argv);
			} else if(strcmp(argv[2], "dayremaincalls") == 0){
				cli_get_channel_remain_info(argc, argv);
			}
		} else if(strcmp(argv[1], "allstatus") == 0) {
			cli_get_all_status(argc, argv);
		} else if(strcmp(argv[1], "sim") == 0) {
			if(strcmp(argv[2], "status") == 0) {
				cli_get_channel_sim_status(argc, argv);
			}
		} 
	}

	if(strcmp(argv[0], "set") == 0) {
		if(strcmp(argv[1], "chn") == 0){
			if(strcmp(argv[2], "calltime") == 0){
				cli_set_channel_calltime(argc, argv);
			} else if(strcmp(argv[2], "daycalls") == 0) {
				cli_set_channel_daycalls(argc, argv);
			} else if(strcmp(argv[2], "dayanswers") == 0) {
				cli_set_channel_dayanswers(argc, argv);
			} else if(strcmp(argv[2], "hourcalls") == 0) {
				cli_set_channel_hourcalls(argc, argv);
			} else if(strcmp(argv[2], "switch") == 0) {
				cli_set_channel_call_switch(argc, argv);
			}else if(strcmp(argv[2], "unlock")== 0){
                            cli_set_channel_unlock(argc, argv);
			}else if(strcmp(argv[2], "unmark")== 0){
                            cli_set_channel_unmark(argc, argv);
			}else if(strcmp(argv[2], "reload")== 0){
                            cli_set_channel_reload(argc, argv);
			}else if(strcmp(argv[2], "reflesh")== 0){
                            cli_set_channel_reflesh_status(argc, argv);
			}else if(strcmp(argv[2], "smsflag")== 0){
                            cli_set_channel_lock_flag(argc, argv);
			}else if(strcmp(argv[2], "smsreport")== 0){
                            cli_set_channel_lock_smsreport_flag(argc, argv);
			}else if(strcmp(argv[2], "smscount")== 0){
                            cli_set_channel_lock_sms_count(argc, argv);
			}else if(strcmp(argv[2], "smssender")== 0){
                            cli_set_channel_lock_sms_sender(argc, argv);
			}else if(strcmp(argv[2], "smsmsg")== 0){
                            cli_set_channel_lock_sms_msg(argc, argv);
			}else if(strcmp(argv[2], "daysms") == 0){
				cli_set_channel_day_sms_settings(argc, argv);
			}else if(strcmp(argv[2], "monsms") == 0){
				cli_set_channel_mon_sms_settings(argc, argv);
			}else if(strcmp(argv[2],"smsswitch") == 0){
				cli_set_channel_sms_limit_switch(argc, argv);
			}else if(strcmp(argv[2],"successflag") == 0){
				cli_set_sms_limit_success_flag(argc, argv);
			}else if(strcmp(argv[2],"monsmscnt") == 0){
				cli_set_mon_sms_cnt(argc, argv);
			}else if(strcmp(argv[2],"daysmscnt") == 0){
				cli_set_day_sms_cnt(argc, argv);
			}else if(strcmp(argv[2], "calltimecnt") == 0){
				cli_set_call_time_cnt(argc, argv);
			}else if(strcmp(argv[2], "calltimetotal") == 0){
				cli_set_call_time_total(argc, argv);
			}
		
		} else if(strcmp(argv[1], "filewrite") == 0) {
			if(strcmp(argv[2], "switch") == 0){
				cli_set_filewrite_switch(argc, argv);
			}
		} else if(strcmp(argv[1], "sim") == 0) {
			if(strcmp(argv[2], "unlimit") == 0){
				cli_set_sim_unlimit(argc, argv);
			}else if(strcmp(argv[2], "unlock") == 0){
				cli_set_sim_unlock(argc, argv);
			}else if(strcmp(argv[2], "unmark") == 0){
				cli_set_sim_unmark(argc, argv);
			}else if(strcmp(argv[2], "calltimecnt") == 0){
				cli_set_sim_calltimecnt(argc, argv);
			}else if(strcmp(argv[2], "daysmscnt") == 0){
				cli_set_sim_daysmscnt(argc, argv);
			}else if(strcmp(argv[2], "monsmscnt") == 0){
				cli_set_sim_monsmscnt(argc, argv);
			}else if(strcmp(argv[2], "callfailedcnt") == 0){
				cli_set_sim_callfailedcnt(argc, argv);
			}else if(strcmp(argv[2], "calllimitcnt") == 0){
				cli_set_sim_calllimitcnt(argc, argv);
			}else if(strcmp(argv[2], "limitsta") == 0){
				cli_set_sim_limitsta(argc, argv);
			}else if(strcmp(argv[2], "pincode") == 0){
				cli_set_sim_pincode(argc, argv);
			}
		} else if(strcmp(argv[1], "allsim") == 0) {
			if(strcmp(argv[2], "unlimit") == 0){
				cli_all_simcard_unlimit(argc, argv);
			}else if(strcmp(argv[2], "unlock") == 0){
				cli_all_simcard_unlock(argc, argv);
			}else if(strcmp(argv[2], "unmark") == 0){
				cli_all_simcard_unmark(argc, argv);
			}else if(strcmp(argv[2], "unsmslimit") == 0){
				cli_all_simcard_unsmslimit(argc, argv);
			}
		}
	} 

	if(strcmp(argv[0], "clean") == 0) {
		if(strcmp(argv[1], "chn") == 0){ 
			if(strcmp(argv[2], "daycalls") == 0){
				cli_clean_channel_day_calls(argc, argv);
			} else if(strcmp(argv[2], "dayanswers") == 0){
				cli_clean_channel_day_answers(argc, argv);
			} else if(strcmp(argv[2], "hourcalls") == 0){
				cli_clean_channel_hour_calls(argc, argv);
			} else if(strcmp(argv[2], "limited") == 0){
				cli_clean_channel_limit(argc, argv);
			}else if (strcmp(argv[2], "smslimit") == 0){
				cli_clean_channel_sms_limit(argc, argv);
			}
		}
	}


	if(strcmp(argv[0], "get") == 0) {
		if(strcmp(argv[1], "chan") == 0){
			if(strcmp(argv[2], "pincode") == 0){
				cli_get_chan_pincode(argc, argv);
			} 
		} else if(strcmp(argv[1], "sim") == 0) {
			if(strcmp(argv[2], "pincode") == 0) {
				cli_get_sim_pincode(argc, argv);
			}
		} 
	}

	if(strcmp(argv[0], "switch") == 0) {
		if(strcmp(argv[1], "chan") == 0){
			if(strcmp(argv[2], "simcard") == 0){
				cli_switch_chan_simcard(argc, argv);
			} 
		} 
	}		
	
	return 0;
}


int main(int argc, char **argv)
{
	if(argc == 1){
	    cli_init();
	    cb_func_reg((char *)"?", cli_usage_main);
	    cb_func_reg((char *)"help", cli_usage_main);
	    cli_reg_calllimit();
	    run_main((char *)"calllimit>");
	}else{
		cli_test_one(argc - 1, argv + 1);
	}

    return 0;
}




