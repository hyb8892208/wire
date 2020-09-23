
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "../../gsoap/xml_client/soapH.h"
#include "../../gsoap/xml_client/calllimit.nsmap"
#include "../include/calllimit_api.h"


char *g_endpoint = (char *)"http://127.0.0.1:9801";

/**********************************************************
函数描述 : soap 初始化
输入参数 : 无
输出参数 : psoap结构指针
返回值   : 0 成功 -1 失败
作者/时间: mingyang.chen / 2017.8.3
************************************************************/
static int cli_soap_init(struct soap * psoap)
{
	if(NULL == psoap)
	{
		printf("cli_soap_init error\n");
		return -1;
	}

    soap_init1(psoap, SOAP_XML_INDENT);
	soap_set_namespaces(psoap, namespaces);

	return 0;
}

/**********************************************************
函数描述 : soap 注销
输入参数 : psoap结构指针
输出参数 : 无
返回值   : 0 成功 -1 失败
作者/时间: mingyang.chen / 2017.8.3
************************************************************/
static int cli_soap_finit(struct soap * psoap)
{
	if(NULL == psoap)
	{
		printf("cli_soap_finit error\n");
		return -1;
	}

    soap_destroy(psoap);
    soap_end(psoap);
    soap_done(psoap);

	return 0;
}


int calllimit_get_channel_settings(int channel_id, calllimit_setting_t *settings)
{
	struct soap soap;
	int result = 0;
	calllimit_get_settings_t setting;

	if(!settings) {
		printf("input illage.\n");		
		return -1;
	}
	
	cli_soap_init(&soap);

	soap_call_calllimit__get_channel_settings(&soap, g_endpoint, "", channel_id, &setting);
	
	if(soap.error)
	{
//		printf("soap error: %d, %s, %s\n", soap.error, *soap_faultcode(&soap), *soap_faultstring(&soap));
		result = -1;
		goto _out;
	}

	memcpy((char *)settings, (char *)&setting.settings, sizeof(calllimit_settings_t));
_out:
	cli_soap_finit(&soap);	
	return result;
}




int calllimit_get_channel_status(int channel_id, calllimit_statu_t *status)
{
	struct soap soap;
	int result = 0;
	calllimit_get_status_t getstatus;

	if(!status) {
		printf("input illage.\n");		
		return -1;
	}
	
	cli_soap_init(&soap);

	soap_call_calllimit__get_channel_status(&soap, g_endpoint, "", channel_id, &getstatus);
	
	if(soap.error)
	{
//		printf("soap error: %d, %s, %s\n", soap.error, *soap_faultcode(&soap), *soap_faultstring(&soap));
		result = -1;
		goto _out;
	}

	memcpy((char *)status, (char *)&getstatus.status, sizeof(calllimit_status_t));
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_get_channel_sim_status(int channel_id, int sim_idx, calllimit_statu_t *status)
{
	struct soap soap;
	int result = 0;
	calllimit_get_status_t getstatus;

	if(!status) {
		printf("input illage.\n");		
		return -1;
	}
	
	cli_soap_init(&soap);

	soap_call_calllimit__get_channel_sim_status(&soap, g_endpoint, "", channel_id, sim_idx, &getstatus);
	
	if(soap.error)
	{
		result = -1;
		goto _out;
	}

	memcpy((char *)status, (char *)&getstatus.status, sizeof(calllimit_status_t));
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_get_all_status(calllimit_statu_t *status)
{
	struct soap soap;
	int result = 0;
	int i = 0;
	calllimit_getall_statu_t get_sta;

	if(!status) {
		printf("input illage.\n");		
		return -1;
	}
	
	cli_soap_init(&soap);

	soap_call_calllimit__get_all_status(&soap, g_endpoint, "", &get_sta);
	
	if(soap.error)
	{
		result = -1;
		goto _out;
	}

	for(i=0; i<get_sta.result*4; i++)
		memcpy((char *)&status[i], (char *)&get_sta.status.__ptr[i], sizeof(calllimit_statu_t));

	result = get_sta.result;
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_channel_calltime(int channel_id, unsigned int calltime)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_calltime(&soap, g_endpoint, "", channel_id, calltime, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_channel_daycalls(int channel_id, unsigned int daycalls)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_daycalls(&soap, g_endpoint, "", channel_id, daycalls, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_channel_dayanswers(int channel_id, unsigned int dayanswers)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_dayanswers(&soap, g_endpoint, "", channel_id, dayanswers, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}
int calllimit_set_channel_hourcalls(int channel_id, unsigned int hourcalls)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_hourcalls(&soap, g_endpoint, "", channel_id, hourcalls, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_channel_call_switch(int channel_id, unsigned int call_switch)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_call_swiitch(&soap, g_endpoint, "", channel_id, call_switch, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_filewrite_switch(unsigned int file_switch)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_filewrite_switch(&soap, g_endpoint, "", file_switch, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}



int calllimit_clean_channel_day_calls(int channel_id)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__clean_channel_day_calls(&soap, g_endpoint, "", channel_id, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_clean_channel_hour_calls(int channel_id)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__clean_channel_hour_calls(&soap, g_endpoint, "", channel_id, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_clean_channel_day_answers(int channel_id)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__clean_channel_day_answers(&soap, g_endpoint, "", channel_id, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_clean_channel_limit(int channel_id)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__clean_channel_limit(&soap, g_endpoint, "", channel_id, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_channel_unlock(int channel_id)
{
	struct soap soap;
	int result = 0;
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_unlock(&soap, g_endpoint, "", channel_id,&result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_channel_unmark(int channel_id)
{
	struct soap soap;
	int result = 0;
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_unmark(&soap, g_endpoint, "", channel_id,&result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_reload_cfg(void)
{
    struct soap soap;
	int result = 0;
	cli_soap_init(&soap);

	soap_call_calllimit__reload_cfg(&soap, g_endpoint, "",&result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_reflesh_status_cfg(void)
{
    struct soap soap;
	int result = 0;
	cli_soap_init(&soap);

	soap_call_calllimit__reflesh_status_cfg(&soap, g_endpoint, "",&result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_channel_detect(int channel_id, unsigned int flag)
{
    struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_lock_detect(&soap, g_endpoint, "", channel_id, flag, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_channel_mark_flag(int channel_id, unsigned int flag)
{
       struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_mark_flag(&soap, g_endpoint, "", channel_id, flag, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_channel_mark_count(int channel_id, unsigned int count)
{
       struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_mark_count(&soap, g_endpoint, "", channel_id, count, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_channel_lock_flag(int channel_id, unsigned int flag)
{
       struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_lock_flag(&soap, g_endpoint, "", channel_id, flag, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_channel_lock_count(int channel_id, unsigned int count)
{
       struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_lock_count(&soap, g_endpoint, "", channel_id, count, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_channel_lock_sms_flag(int channel_id, unsigned int flag)
{
       struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_channel_sms_flag(&soap, g_endpoint, "", channel_id, flag, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;

}

int calllimit_set_channel_lock_sms_report_flag(int channel_id, unsigned int flag)
{
    struct soap soap;
    int result = 0;

    cli_soap_init(&soap);

    soap_call_calllimit__set_channel_sms_report_flag(&soap, g_endpoint, "", channel_id, flag, &result);

    if(soap.error) {
    	result = -1;
    	goto _out;
    }
_out:
    cli_soap_finit(&soap);	
    return result;
}

int calllimit_set_channel_lock_sms_count(int channel_id, unsigned int count)
{
    struct soap soap;
    int result = 0;

    cli_soap_init(&soap);

    soap_call_calllimit__set_channel_sms_count(&soap, g_endpoint, "", channel_id, count, &result);

    if(soap.error) {
    	result = -1;
    	goto _out;
    }
_out:
    cli_soap_finit(&soap);	
    return result;
}

int calllimit_set_channel_lock_sms_callee(int channel_id, char *addr)
{
    struct soap soap;
    int result = 0;

    cli_soap_init(&soap);

    soap_call_calllimit__set_channel_sms_sender(&soap, g_endpoint, "", channel_id, addr, &result);

    if(soap.error) {
    	result = -1;
    	goto _out;
    }
_out:
    cli_soap_finit(&soap);	
    return result;
}

int calllimit_set_channel_lock_sms_msg(int channel_id, char *msg)
{
    struct soap soap;
    int result = 0;

    cli_soap_init(&soap);

    soap_call_calllimit__set_channel_sms_msg(&soap, g_endpoint, "", channel_id, msg, &result);

    if(soap.error) {
    	result = -1;
    	goto _out;
    }
_out:
    cli_soap_finit(&soap);	
    return result;
}

int calllimit_clena_channel_sms_limit(int channel_id){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__clena_channel_sms_limit(&soap, g_endpoint, "", channel_id, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_day_sms_limit_settings(int channel_id, unsigned int sms_settings){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_day_sms_limit_settings(&soap, g_endpoint, "", channel_id, sms_settings, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_mon_sms_limit_settings(int channel_id, unsigned int sms_settings){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_mon_sms_limit_settings(&soap, g_endpoint, "", channel_id, sms_settings, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_sms_limit_switch(int channel_id, unsigned int sms_switch){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_sms_limit_switch(&soap, g_endpoint, "", channel_id, sms_switch, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_sms_limit_success_flag(int channel_id, unsigned int flag){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_sms_limit_success_flag(&soap, g_endpoint, "", channel_id, flag, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_mon_sms_cnt(int channel_id, unsigned int cur_sms){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_mon_sms_cnt(&soap, g_endpoint, "", channel_id, cur_sms, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_day_sms_cnt(int channel_id, unsigned int cur_sms){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_day_sms_cnt(&soap, g_endpoint, "", channel_id, cur_sms, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_call_time_cnt(int channel_id, unsigned int call_time_cnt){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_call_time_cnt(&soap, g_endpoint, "", channel_id, call_time_cnt, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_call_time_total(int channel_id, unsigned int total){
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__set_call_time_total(&soap, g_endpoint, "", channel_id, total, &result);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_sim_limitsta(int channel_id, int sim_idx, unsigned int calllimit_flag, unsigned int calllock_flag, unsigned int callmark_flag, unsigned int smslimit_flag)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_limitsta(&soap, g_endpoint, "", channel_id, sim_idx, calllimit_flag, calllock_flag, callmark_flag, smslimit_flag, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_sim_unlimit(int channel_id, int sim_idx)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_unlimit(&soap, g_endpoint, "", channel_id, sim_idx, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_sim_unlock(int channel_id, int sim_idx)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_unlock(&soap, g_endpoint, "", channel_id, sim_idx, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_sim_unmark(int channel_id, int sim_idx)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_unmark(&soap, g_endpoint, "", channel_id, sim_idx, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_all_simcard_unlimit(void)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__all_simcard_unlimit(&soap, g_endpoint, "", &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_all_simcard_unlock(void)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__all_simcard_unlock(&soap, g_endpoint, "", &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_all_simcard_unmark(void)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__all_simcard_unmark(&soap, g_endpoint, "", &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_all_simcard_unsmslimit(void)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__all_simcard_unsmslimit(&soap, g_endpoint, "", &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_switch_chan_simcard(int channel_id, int sim_idx)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__switch_chan_simcard(&soap, g_endpoint, "", channel_id, sim_idx, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_sim_daysmscnt(int channel_id, int sim_idx, unsigned int daysmscnt)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_daysmscnt(&soap, g_endpoint, "", channel_id, sim_idx, daysmscnt, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_sim_monsmscnt(int channel_id, int sim_idx, unsigned int monsmscnt)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_monsmscnt(&soap, g_endpoint, "", channel_id, sim_idx, monsmscnt, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_set_sim_calltimecnt(int channel_id, int sim_idx, unsigned int calltimecnt)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_calltimecnt(&soap, g_endpoint, "", channel_id, sim_idx, calltimecnt, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_sim_callfailedcnt(int channel_id, int sim_idx, unsigned int callfailedcnt)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_callfailedcnt(&soap, g_endpoint, "", channel_id, sim_idx, callfailedcnt, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_sim_calllimitcnt(int channel_id, int sim_idx, unsigned int daycalls, unsigned int dayanswers, unsigned int hourcalls)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_calllimitcnt(&soap, g_endpoint, "", channel_id, sim_idx, daycalls, dayanswers, hourcalls, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}


int calllimit_set_sim_pincode(int channel_id, int sim_idx, char *pincode)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__set_sim_pincode(&soap, g_endpoint, "", channel_id, sim_idx, pincode, &result);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_get_sim_pincode(int channel_id, int sim_idx, sim_pincode__t *pincode)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__get_sim_pincode(&soap, g_endpoint, "", channel_id, sim_idx, pincode);
	if(soap.error) {
		result = -1;
		goto _out;
	}		
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_get_channel_pincode(int channel_id, sim_pincode__t *pincode)
{
	struct soap soap;
	int result = 0;

	cli_soap_init(&soap);

	soap_call_calllimit__get_channel_pincode(&soap, g_endpoint, "", channel_id, pincode);
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

int calllimit_get_remain_info(int channel_id, struct calllimit_get_remain_info  *remain)
{
	struct soap soap;
	int result = 0;
	
	cli_soap_init(&soap);

	soap_call_calllimit__get_remain_info(&soap, g_endpoint, "", channel_id, (struct calllimit_get_remain_info_s *)remain);
	
	if(soap.error) {
		result = -1;
		goto _out;
	}
_out:
	cli_soap_finit(&soap);	
	return result;
}

