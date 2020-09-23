#include "../../gsoap/xml_server/soapH.h"
#include "../../gsoap/xml_server/calllimit.nsmap"

#include "../include/header.h"
#include "../include/gsoap_server.h"



typedef struct _gsoap_info_s
{
	int head;
	int tail;
	SOAP_SOCKET queue[GSOAP_MAX_QUEUE]; 
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_cond;
}gsoap_info_t;


static gsoap_sap_t gsoap_sap;
static gsoap_info_t gsoap_info;
static struct soap server_soap;

static void *gsoap_process_queue(void*);
static int gsoap_enqueue(SOAP_SOCKET);
static SOAP_SOCKET gsoap_dequeue();


void gsoap_init()
{
	memset(&gsoap_sap, 0, sizeof(gsoap_sap_t));
	memset(&gsoap_info, 0, sizeof(gsoap_info_t));
}

int gsoap_bind(gsoap_sap_t *sap)
{
	if (sap == NULL)
		return -1;
	memcpy(&gsoap_sap, sap, sizeof(gsoap_sap_t));

	return 0;
}

// ¶àÏß³Ì
void *gsoap_main_thread_cb_handler(void *arg)
{
	int i;
	unsigned short port = 0;
//	unsigned short count = 0;
	gsoap_params_t *ptr_params = NULL;
	SOAP_SOCKET master_sk, slave_sk;
	struct soap *ptr_soap = &server_soap;
	gsoap_info_t *ptr_info = &gsoap_info;
	struct soap *soap_thr[GSOAP_MAX_THR]; // each thread needs a runtime context 
	pthread_t gsoap_tid[GSOAP_MAX_THR];

	if (arg != NULL)
	{
		ptr_params = (gsoap_params_t*)arg;
		port = ptr_params->gsoap_port;
	//	count = ptr_params->count;
	}

	if(port == 0)
		port = GSOAP_SERVER_PORT;
	soap_init(ptr_soap);
	ptr_soap->bind_flags |= SO_REUSEADDR;	
	soap_set_namespaces(ptr_soap, namespaces1);
	master_sk = soap_bind(ptr_soap, GSOAP_SERVER_IP, port, GSOAP_BACK_LOG);
	if(!soap_valid_socket(master_sk))
	{
		soap_print_fault(ptr_soap, stderr);
		exit(1);
	}
	pthread_mutex_init(&ptr_info->queue_lock, NULL);
	pthread_cond_init(&ptr_info->queue_cond, NULL);
	for (i = 0; i < GSOAP_MAX_THR; i++)
	{
		soap_thr[i] = soap_copy(ptr_soap);
//		fprintf(stderr, "Starting thread %d\n", i);
		calllimit_pthread_create(&gsoap_tid[i], NULL, (void*(*)(void*))gsoap_process_queue, (void*)soap_thr[i]);
	}

	for (;;)
	{
		slave_sk = soap_accept(ptr_soap);
		if (!soap_valid_socket(slave_sk))
		{
			if (ptr_soap->errnum)
			{
				soap_print_fault(ptr_soap, stderr);
				continue; // retry 
			}
			else
			{
				fprintf(stderr, "Server timed out\n");
				break;
			}
		}
		while (gsoap_enqueue(slave_sk) == SOAP_EOM)
			sleep(1);
	}
	for (i = 0; i < GSOAP_MAX_THR; i++)
	{
		while (gsoap_enqueue(SOAP_INVALID_SOCKET) == SOAP_EOM)
			sleep(1);
	}
	for (i = 0; i < GSOAP_MAX_THR; i++)
	{
		fprintf(stderr, "Waiting for thread %d to terminate... ", i);
		pthread_join(gsoap_tid[i], NULL);
		fprintf(stderr, "terminated\n");
		soap_done(soap_thr[i]);
		free(soap_thr[i]);
	} 
	pthread_mutex_destroy(&ptr_info->queue_lock); 
	pthread_cond_destroy(&ptr_info->queue_cond);
	soap_done(ptr_soap);

	return NULL;
} 

void *gsoap_process_queue(void *soap)
{
	struct soap *ptr_soap = (struct soap*)soap;
	for (;;)
	{
		ptr_soap->socket = gsoap_dequeue();
		if (!soap_valid_socket(ptr_soap->socket))
		{
			gsoap_print_warn("%s call gsoap_dequeue failed!, socket=%d. \n", __FUNCTION__, ptr_soap->socket);
			break;
		}
		soap_serve(ptr_soap);
		soap_destroy(ptr_soap);
		soap_end(ptr_soap);
	}

	return NULL;
}

int gsoap_enqueue(SOAP_SOCKET sock)
{
	int status = SOAP_OK;
	int next;
	gsoap_info_t *ptr_info = &gsoap_info;

	pthread_mutex_lock(&ptr_info->queue_lock);
	next = ptr_info->tail + 1;
	if (next >= GSOAP_MAX_QUEUE)
		next = 0;
	if (next == ptr_info->head)
		status = SOAP_EOM;
	else
	{
		ptr_info->queue[ptr_info->tail] = sock;
		ptr_info->tail = next;
		pthread_cond_signal(&ptr_info->queue_cond);
	}
	pthread_mutex_unlock(&ptr_info->queue_lock);

	return status;
}

SOAP_SOCKET gsoap_dequeue()
{
	SOAP_SOCKET sock;
	gsoap_info_t *ptr_info = &gsoap_info;

	pthread_mutex_lock(&ptr_info->queue_lock);
	while (ptr_info->head == ptr_info->tail)
		pthread_cond_wait(&ptr_info->queue_cond, &ptr_info->queue_lock);
	sock = ptr_info->queue[ptr_info->head++];
	if (ptr_info->head >= GSOAP_MAX_QUEUE)
		ptr_info->head = 0;
	pthread_mutex_unlock(&ptr_info->queue_lock);

	return sock;
}


int gsoap_thread_create(void)
{
	pthread_t gsoap_tid;

	calllimit_pthread_create(&gsoap_tid, NULL, gsoap_main_thread_cb_handler, NULL);

	return 0;	
}



int calllimit__get_channel_settings(struct soap *soap, int channel_id, calllimit_get_settings_t *settings)
{	
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_get_channel_settings)
		settings->result = gsoap_sap.gsoap_cb_get_channel_settings(channel_id, &settings->settings);

	return SOAP_OK;
}


int calllimit__get_channel_status(struct soap *soap, int channel_id, calllimit_get_status_t *status)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_get_channel_status)
		status->result = gsoap_sap.gsoap_cb_get_channel_status(channel_id, &status->status);

	return SOAP_OK;
}

int calllimit__get_channel_sim_status(struct soap *soap, int channel_id, int sim_idx, calllimit_get_status_t *status)
{
	if(gsoap_sap.gsoap_cb_get_channel_sim_status)
		status->result = gsoap_sap.gsoap_cb_get_channel_sim_status(channel_id, sim_idx, &status->status);

	return SOAP_OK;
}

int calllimit__get_all_status(struct soap *soap, calllimit_getall_status_t *status)
{
//	struct soap *ptr_soap = &server_soap;

	status->status.__size = MAX_CHAN*SIM_NUM;
    status->status.__ptr = soap_malloc(soap, MAX_CHAN*SIM_NUM*sizeof(calllimit_status_t));

	if(gsoap_sap.gsoap_cb_get_all_status)
		status->result = gsoap_sap.gsoap_cb_get_all_status(status);

	return SOAP_OK;
}


int calllimit__set_channel_calltime(struct soap *soap, int channel_id, unsigned int calltime, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_set_channel_calltime)
		*result = gsoap_sap.gsoap_cb_set_channel_calltime(channel_id, calltime);

	return SOAP_OK;
}

int calllimit__set_channel_daycalls(struct soap *soap, int channel_id, unsigned int daycalls, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_set_channel_daycalls)
		*result = gsoap_sap.gsoap_cb_set_channel_daycalls(channel_id, daycalls);

	return SOAP_OK;
}

int calllimit__set_channel_dayanswers(struct soap *soap, int channel_id, unsigned int dayanswers, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_set_channel_dayanswers)
		*result = gsoap_sap.gsoap_cb_set_channel_dayanswers(channel_id, dayanswers);

	return SOAP_OK;
}

int calllimit__set_channel_hourcalls(struct soap *soap, int channel_id, unsigned int hourcalls, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_set_channel_hourcalls)
		*result = gsoap_sap.gsoap_cb_set_channel_hourcalls(channel_id, hourcalls);

	return SOAP_OK;
}


int calllimit__set_channel_call_swiitch(struct soap *soap, int channel_id, unsigned int call_swiitch, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_set_channel_switch)
		*result = gsoap_sap.gsoap_cb_set_channel_switch(channel_id, call_swiitch);

	return SOAP_OK;
}

int calllimit__set_filewrite_switch(struct soap *soap, unsigned int file_switch, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_set_filewrite_switch)
		*result = gsoap_sap.gsoap_cb_set_filewrite_switch(file_switch);

	return SOAP_OK;
}


int calllimit__clean_channel_day_calls(struct soap *soap, int channel_id, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_clean_channel_day_calls)
		*result = gsoap_sap.gsoap_cb_clean_channel_day_calls(channel_id);

	return SOAP_OK;
}


int calllimit__clean_channel_day_answers(struct soap *soap, int channel_id, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_clean_channel_day_answers)
		*result = gsoap_sap.gsoap_cb_clean_channel_day_answers(channel_id);

	return SOAP_OK;
}


int calllimit__clean_channel_hour_calls(struct soap *soap, int channel_id, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_clean_channel_hour_calls)
		*result = gsoap_sap.gsoap_cb_clean_channel_hour_calls(channel_id);

	return SOAP_OK;
}
int calllimit__clean_channel_limit(struct soap *soap, int channel_id, int *result)
{
//	struct soap *ptr_soap = &server_soap;
	
	if(gsoap_sap.gsoap_cb_clean_channel_limit)
		*result = gsoap_sap.gsoap_cb_clean_channel_limit(channel_id);

	return SOAP_OK;
}

int calllimit__set_channel_unlock(struct soap *soap, int channel_id, int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_unlock)
		*result = gsoap_sap.gsoap_cb_set_channel_unlock(channel_id);
    
    return SOAP_OK;
}

int calllimit__set_channel_unmark(struct soap *soap, int channel_id, int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_unmark)
		*result = gsoap_sap.gsoap_cb_set_channel_unmark(channel_id);
    
    return SOAP_OK;
}

int calllimit__reload_cfg(struct soap *soap,int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_reload_cfg)
		*result = gsoap_sap.gsoap_cb_reload_cfg( );
    
    return SOAP_OK;
}

int calllimit__reflesh_status_cfg(struct soap *soap,int *result)
{    
    if(gsoap_sap.gsoap_cb_reflesh_status_cfg)
		*result = gsoap_sap.gsoap_cb_reflesh_status_cfg( );
    
    return SOAP_OK;
}

int calllimit__set_channel_lock_detect(struct soap *soap, int channel_id, unsigned int flag,int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_check_flag)
		*result = gsoap_sap.gsoap_cb_set_channel_check_flag(channel_id,flag);
    
    return SOAP_OK;
}

int calllimit__set_channel_mark_flag(struct soap *soap, int channel_id, unsigned int flag,int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_mark_flag)
		*result = gsoap_sap.gsoap_cb_set_channel_mark_flag(channel_id,flag);
    
    return SOAP_OK;
}

int calllimit__set_channel_mark_count(struct soap *soap, int channel_id, unsigned int count,int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_mark_count)
		*result = gsoap_sap.gsoap_cb_set_channel_mark_count(channel_id,count);
    
    return SOAP_OK;
}

int calllimit__set_channel_lock_flag(struct soap *soap, int channel_id, unsigned int flag,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_lock_flag)
		*result = gsoap_sap.gsoap_cb_set_channel_lock_flag(channel_id,flag);
    
    return SOAP_OK;
}

int calllimit__set_channel_lock_count(struct soap *soap, int channel_id, unsigned int count,int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_lock_count)
		*result = gsoap_sap.gsoap_cb_set_channel_lock_count(channel_id,count);
    
    return SOAP_OK;
}

int calllimit__set_channel_sms_flag(struct soap *soap,int channel_id, unsigned int flag, int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_sms_flag)
		*result = gsoap_sap.gsoap_cb_set_channel_sms_flag(channel_id,flag);
    
    return SOAP_OK;
}

int calllimit__set_channel_sms_report_flag(struct soap *soap,int channel_id, unsigned int flag, int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_sms_report_flag)
		*result = gsoap_sap.gsoap_cb_set_channel_sms_report_flag(channel_id,flag);
    
    return SOAP_OK;
}

int calllimit__set_channel_sms_count(struct soap *soap,int channel_id, unsigned int count, int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_sms_failed_count)
		*result = gsoap_sap.gsoap_cb_set_channel_sms_failed_count(channel_id,count);
    
    return SOAP_OK;
}

int calllimit__set_channel_sms_sender(struct soap *soap,int channel_id, char *addr,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_sms_sender)
		*result = gsoap_sap.gsoap_cb_set_channel_sms_sender(channel_id,addr);
    
    return SOAP_OK;
}

int calllimit__set_channel_sms_msg(struct soap *soap,int channel_id, char *msg,int *result)
{
 //   struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_channel_sms_sender)
		*result = gsoap_sap.gsoap_cb_set_channel_sms_msg(channel_id,msg);
    
    return SOAP_OK;
}

//-----------------------------------------------------------------------

int calllimit__clena_channel_sms_limit(struct soap *soap,int channel_id,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_channel_sms_limit)
		*result = gsoap_sap.gsoap_cb_channel_sms_limit(channel_id);
    
    return SOAP_OK;
}
int calllimit__set_day_sms_limit_settings(struct soap *soap,int channel_id, unsigned int sms_settings, int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_day_sms_limit_settings)
		*result = gsoap_sap.gsoap_cb_set_day_sms_limit_settings(channel_id,sms_settings);
    
    return SOAP_OK;
}

int calllimit__set_mon_sms_limit_settings(struct soap *soap,int channel_id, unsigned int sms_settings, int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_mon_sms_limit_settings)
		*result = gsoap_sap.gsoap_cb_set_mon_sms_limit_settings(channel_id,sms_settings);
    
    return SOAP_OK;
}

int calllimit__set_sms_limit_switch(struct soap *soap, int channel_id, unsigned int sms_switch,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_sms_limit_switch)
		*result = gsoap_sap.gsoap_cb_set_sms_limit_switch(channel_id,sms_switch);
    
    return SOAP_OK;
}


int calllimit__set_sms_limit_success_flag(struct soap *soap, int channel_id, unsigned int flag,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_sms_limit_success_flag)
		*result = gsoap_sap.gsoap_cb_set_sms_limit_success_flag(channel_id,flag);
    
    return SOAP_OK;
}

int calllimit__set_day_sms_cnt(struct soap *soap, int channel_id, unsigned int cur_sms,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_day_sms_cnt)
		*result = gsoap_sap.gsoap_cb_set_day_sms_cnt(channel_id,cur_sms);
    
    return SOAP_OK;
}

int calllimit__set_mon_sms_cnt(struct soap *soap, int channel_id, unsigned int cur_sms,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_mon_sms_cnt)
		*result = gsoap_sap.gsoap_cb_set_mon_sms_cnt(channel_id,cur_sms);
    
    return SOAP_OK;
}
int calllimit__set_call_time_cnt(struct soap *soap, int channel_id, unsigned int call_time_cnt,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_call_time_cnt)
		*result = gsoap_sap.gsoap_cb_set_call_time_cnt(channel_id,call_time_cnt);
    
    return SOAP_OK;
}
int calllimit__set_call_time_total(struct soap *soap, int channel_id, unsigned int total,int *result)
{
//    struct soap *ptr_soap = &server_soap;
    
    if(gsoap_sap.gsoap_cb_set_call_time_total)
		*result = gsoap_sap.gsoap_cb_set_call_time_total(channel_id,total);
    
    return SOAP_OK;
}


int calllimit__set_sim_limitsta(struct soap *soap, int channel_id, int sim_idx, unsigned int calllimit_flag, unsigned int calllock_flag, unsigned int callmark_flag, unsigned int smslimit_flag, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_limitsta)
		*result = gsoap_sap.gsoap_cb_set_sim_limitsta(channel_id, sim_idx, calllimit_flag, calllock_flag, callmark_flag, smslimit_flag);
    
    return SOAP_OK;
}

int calllimit__set_sim_unlimit(struct soap *soap, int channel_id, int sim_idx, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_unlimit)
		*result = gsoap_sap.gsoap_cb_set_sim_unlimit(channel_id, sim_idx);
    
    return SOAP_OK;
}

int calllimit__set_sim_unlock(struct soap *soap, int channel_id, int sim_idx, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_unlock)
		*result = gsoap_sap.gsoap_cb_set_sim_unlock(channel_id, sim_idx);
    
    return SOAP_OK;
}

int calllimit__set_sim_unmark(struct soap *soap, int channel_id, int sim_idx, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_unmark)
		*result = gsoap_sap.gsoap_cb_set_sim_unmark(channel_id, sim_idx);
    
    return SOAP_OK;
}


int calllimit__all_simcard_unlimit(struct soap *soap, int *result)
{
    if(gsoap_sap.gsoap_cb_all_simcard_unlimit)
		*result = gsoap_sap.gsoap_cb_all_simcard_unlimit();
    
    return SOAP_OK;
}

int calllimit__all_simcard_unlock(struct soap *soap, int *result)
{
    if(gsoap_sap.gsoap_cb_all_simcard_unlock)
		*result = gsoap_sap.gsoap_cb_all_simcard_unlock();
    
    return SOAP_OK;
}

int calllimit__all_simcard_unmark(struct soap *soap, int *result)
{
    if(gsoap_sap.gsoap_cb_all_simcard_unmark)
		*result = gsoap_sap.gsoap_cb_all_simcard_unmark();
    
    return SOAP_OK;
}

int calllimit__all_simcard_unsmslimit(struct soap *soap, int *result)
{
    if(gsoap_sap.gsoap_cb_all_simcard_unsmslimit)
		*result = gsoap_sap.gsoap_cb_all_simcard_unsmslimit();
    
    return SOAP_OK;
}

int calllimit__switch_chan_simcard(struct soap *soap, int channel_id, int sim_idx, int *result)
{
    if(gsoap_sap.gsoap_cb_switch_chan_simcard)
		*result = gsoap_sap.gsoap_cb_switch_chan_simcard(channel_id, sim_idx);
    
    return SOAP_OK;
}


int calllimit__set_sim_daysmscnt(struct soap *soap, int channel_id, int sim_idx, unsigned int day_sms, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_daysmscnt)
		*result = gsoap_sap.gsoap_cb_set_sim_daysmscnt(channel_id, sim_idx, day_sms);
    
    return SOAP_OK;
}

int calllimit__set_sim_monsmscnt(struct soap *soap, int channel_id, int sim_idx, unsigned int mon_sms, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_monsmscnt)
		*result = gsoap_sap.gsoap_cb_set_sim_monsmscnt(channel_id, sim_idx, mon_sms);
    
    return SOAP_OK;
}

int calllimit__set_sim_calltimecnt(struct soap *soap, int channel_id, int sim_idx, unsigned int calltimecnt, int *result)
{
	if(gsoap_sap.gsoap_cb_set_sim_calltimecnt)
		*result = gsoap_sap.gsoap_cb_set_sim_calltimecnt(channel_id, sim_idx, calltimecnt);

	return SOAP_OK;
}


int calllimit__set_sim_callfailedcnt(struct soap *soap, int channel_id, int sim_idx, unsigned int callfailedcnt, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_callfailedcnt)
		*result = gsoap_sap.gsoap_cb_set_sim_callfailedcnt(channel_id, sim_idx, callfailedcnt);
    
    return SOAP_OK;
}

int calllimit__set_sim_calllimitcnt(struct soap *soap, int channel_id, int sim_idx, unsigned int daycalls, unsigned int dayanswers, unsigned int hourcalls, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_calllimitcnt)
		*result = gsoap_sap.gsoap_cb_set_sim_calllimitcnt(channel_id, sim_idx, daycalls, dayanswers, hourcalls);
}


int calllimit__set_sim_pincode(struct soap *soap, int channel_id, int sim_idx, char *pincode, int *result)
{
    if(gsoap_sap.gsoap_cb_set_sim_pincode)
		*result = gsoap_sap.gsoap_cb_set_sim_pincode(channel_id, sim_idx, pincode);
}


int calllimit__get_sim_pincode(struct soap *soap, int channel_id, int sim_idx, sim_pincode_t *result)
{
    if(gsoap_sap.gsoap_cb_get_sim_pincode)
		result->result = gsoap_sap.gsoap_cb_get_sim_pincode(channel_id, sim_idx, result->pincode);
}


int calllimit__get_channel_pincode(struct soap *soap, int channel_id, sim_pincode_t *result)
{
    if(gsoap_sap.gsoap_cb_get_channel_pincode)
		result->result = gsoap_sap.gsoap_cb_get_channel_pincode(channel_id, result->pincode);
}


int calllimit__get_remain_info(struct soap *soap, int channel_id, struct  calllimit_get_remain_info_s *remain){
	if(gsoap_sap.gsoap_cb_get_remain_info)
		gsoap_sap.gsoap_cb_get_remain_info(channel_id, remain);
	return SOAP_OK;
	
}

