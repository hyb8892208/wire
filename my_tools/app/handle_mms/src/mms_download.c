#include "mms_inc.h"
#include "mms.h"
#include "mms_decode.h"
#include "mms_queue.h"
#include "mms_get_devport.h"
#include "mms_debug.h"
#include <curl/curl.h>
#include <pthread.h>
#include <stdlib.h>


#define MMS_NET_DEV "ppp0"
#define DIALUP_CONNECT_COMMAND "/my_tools/module_connect_net.sh -c %d -a %s -u %s -p %s -v IPV4V6 -t ppp > /dev/null "
#define DIALUP_SELECT_PORT "/my_tools/bsp_cli upgrade sel %d > /dev/null"
#define DIALUP_STOP_COMMAND "/my_tools/module_disconnect_net.sh %d > /dev/null"
/*
static void mms_dialup_select_port(int channel){
	char cmd[128] = {0};
	sprintf(cmd, DIALUP_SELECT_PORT, channel);
	system(cmd);
	sleep(1);
}
*/
static void mms_dialup_disconnect(int channel){
	char cmd[256] = {0};
	sprintf(cmd, DIALUP_STOP_COMMAND, channel);
	system(DIALUP_STOP_COMMAND);
}

static int mms_dialup_connect(int channel, mms_dialup_info_t* info ){
	char cmd[256] = {0};
	//int devport;
	int res;
	//mms_dialup_select_port(channel);
	//devport = mms_get_devport(channel);
	sprintf(info->device, "ppp%d", channel);
	sprintf(cmd, DIALUP_CONNECT_COMMAND, channel, info->apn, info->username, info->password); 
	res = system(cmd);
	if(res < 0){ 
		return -1;
	}
	return 0;
}

/*
curl callback
*/
static size_t mms_curl_write_func(void *ptr, size_t size, size_t nmemb, void *data){
	int real_size = size*nmemb;
	mms_memory_t*mms_memory = (mms_memory_t *)data;
	if(mms_memory->total < real_size+mms_memory->size)//memeory is not enough, realloc again.
		mms_memory->buf = (char *)realloc(mms_memory->buf, mms_memory->size + real_size + 1);
	if( mms_memory->buf == NULL)
		return -1;
	memcpy(&(mms_memory->buf[mms_memory->size]), ptr, real_size);
	mms_memory->size += real_size;
	mms_memory->buf[mms_memory->size] = '\0';
	return real_size;
}
/*
0 means success
-1 means error
-2 means net interface is not valied, maybe ppp failed
*/
static int mms_curl_get_content(char *device, char *mmsurl, char *dns, mms_memory_t *mms_memory){
	CURL *curl;
	CURLcode res;;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl == NULL){
		MMS_LOG_PRINT(LOG_ERROR,"curl init failed\n");
		return -1;
	}
	res = curl_easy_setopt(curl, CURLOPT_URL, mmsurl);
	if(res != 0){
		curl_easy_cleanup(curl);
		MMS_LOG_PRINT(LOG_ERROR, "set url failed\n");
		return -1;
	}
	res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)mms_memory);
	if(res != 0){
		curl_easy_cleanup(curl);
		MMS_LOG_PRINT(LOG_ERROR,"set CURLOPT_WRITEDATA failed\n");
		return -1;
	}
	res = curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30 );
	if(res != 0){
		MMS_LOG_PRINT(LOG_ERROR, "set timout failed.\n");
	}
	res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mms_curl_write_func);
	if(res != 0){
		curl_easy_cleanup(curl);
		MMS_LOG_PRINT(LOG_ERROR,"set CURLOPT_WRITEFUNCTION failed\n");
		return -1;
	}
	res = curl_easy_setopt(curl, CURLOPT_INTERFACE, device );
	if(res != 0){
		curl_easy_cleanup(curl);
		MMS_LOG_PRINT(LOG_ERROR, "set CURLOPT_INTERFACE failed\n");
		return -1;
	}
#if 0
	if(strlen(dns) > 0){
		res = curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, dns );
		if(res != 0){
			curl_easy_cleanup(curl);
			MMS_LOG_PRINT(LOG_ERROR, "set CURLOPT_DNS_SERVERS failed\n");
			return -1;
		}
	}
#endif
	res = curl_easy_perform(curl);
	if(res != 0){
		curl_easy_cleanup(curl);
		MMS_LOG_PRINT(LOG_ERROR, "download mms from %s failed, res=%d, mms_net_dev=%s\n", mmsurl, res, device);
		if(res == CURLE_INTERFACE_FAILED)//this means net interface is not vaild
			return -2;
		return -1;
	}
	MMS_LOG_PRINT(LOG_INFO, "download mms from %s success.\n", mmsurl);
	curl_easy_cleanup(curl);
	return 0;
}

static void mms_get_local_time(struct tm *out_date){
	struct tm *ptr = NULL;
	time_t lt;
	lt =time(NULL);
	ptr=localtime(&lt);
	memcpy((char *)out_date, (char *)ptr, sizeof(struct tm));
}

static void get_next_mms_handler_waikup(struct tm *dl_time, struct timespec *abstime){
	struct tm tm_now;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	mms_get_local_time(&tm_now);
	int next_time = dl_time->tm_hour *3600 + dl_time->tm_min * 60 + dl_time->tm_sec ;
	int now_time = tm_now.tm_hour *3600 + tm_now.tm_min * 60 + tm_now.tm_sec;
	if(next_time <= now_time){
		abstime->tv_sec = 24 * 3600 + next_time - now_time;
	}else{
		abstime->tv_sec = next_time - now_time;
	}
	abstime->tv_sec += tv.tv_sec;
}

void *mms_curl_handler(void *data){
	mms_t *p_mms = (mms_t *)data;
	mms_dl_pthread_info_t *dl_thread_info = &p_mms->mms_dl_pthread;
	mms_queue_t *wait_queue = dl_thread_info->wait_queue;
	mms_queue_t *fail_queue = dl_thread_info->fail_queue;
	mms_config_t *config = p_mms->mms_config;
	mms_message *msg = dl_thread_info->mms_msg;
	mms_memory_t *mem = dl_thread_info->mms_memory; 
	mms_result_queue_t *result_queue = p_mms->mms_result_pthread.result_queue;
	mms_queue_item_t item;
	mms_result_queue_item_t result_item;
	int port, res, flag = 0;
	struct tm *tm;
	mms_dialup_info_t mms_dailup_info;
	struct timespec next_time;
	int old_state;
	int first_run_flag = 1;
	int i = 0;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
	//pthread_mutex_lock(&dl_thread_info->mms_lock);
	while(1){
		if(first_run_flag && config->dl_type == MMS_DL_URL_TIME){//ontime
			get_next_mms_handler_waikup(&config->dl_time, &next_time);
			pthread_cond_timedwait(&dl_thread_info->mms_cond, &dl_thread_info->mms_lock, &next_time);
			first_run_flag = 0;
		}
		memset(&item, 0, sizeof(mms_queue_item_t));
		//mms queue pop data
		while(queue_pop(wait_queue, &item) == -1){//
			flag = 0;
			memset(&mms_dailup_info, 0, sizeof(mms_dailup_info));//clear memory to 0, means next mms will call dialup again.
			if(config->dl_type == MMS_DL_URL_TIME){//ontime
				get_next_mms_handler_waikup(&config->dl_time, &next_time);
				pthread_cond_timedwait(&dl_thread_info->mms_cond, &dl_thread_info->mms_lock, &next_time);
				//printf("cond timeout\n");
			}else{
				pthread_cond_wait(&dl_thread_info->mms_cond, &dl_thread_info->mms_lock);
			}
		}
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
		port = atoi(item.tv[MMS_ID_PORT].value);
		mms_dialup_disconnect(port);
		mms_dialup_connect(port, &config->dialup_info[port-1]);
		flag = 1;
		memcpy(&mms_dailup_info, &config->dialup_info[port-1], sizeof(mms_dialup_info_t));
		sleep(6);//wait pppd success

		//get mms content
		res = mms_curl_get_content(config->dialup_info[port-1].device ,item.tv[MMS_ID_URL].value , config->dialup_info[port-1].dns, mem);
		if(res == -2){
			flag = 1;
			MMS_LOG_PRINT(LOG_INFO, "dialup failed, try again\n");
			mms_dialup_disconnect(port);
			mms_dialup_connect(port, &config->dialup_info[port-1]);
			sleep(5);
			res = mms_curl_get_content(config->dialup_info[port-1].device,item.tv[MMS_ID_URL].value, config->dialup_info[port-1].dns, mem);
			if(res < 0){
				if(item.try_count < 10){
					++item.try_count;
					MMS_LOG_PRINT(LOG_INFO, "port[%s] download mms %s failed, put to fail_queue\n", item.tv[MMS_ID_PORT].value, item.tv[MMS_ID_URL].value);
					queue_put(fail_queue, &item);
				}else{
					MMS_LOG_PRINT(LOG_ERROR, "recive %s %s failed\n", item.tv[MMS_ID_PORT].value, item.tv[MMS_ID_URL].value);
				}
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
				usleep(20000);//20ms, there can be cancel 
				mms_dialup_disconnect(port);
				continue;
			}
		}else if(res == -1){
			res = mms_curl_get_content(config->dialup_info[port-1].device, item.tv[MMS_ID_URL].value, config->dialup_info[port-1].dns,mem);
			if(res < 0){
				if(item.try_count < 10){
					++item.try_count;
					MMS_LOG_PRINT(LOG_INFO, "port[%s] download mms %s failed, put to fail_queue\n", item.tv[MMS_ID_PORT].value, item.tv[MMS_ID_URL].value);
					queue_put(fail_queue, &item);
				}else{
					MMS_LOG_PRINT(LOG_ERROR,"recive %s %s failed\n", item.tv[MMS_ID_PORT].value, item.tv[MMS_ID_URL].value);
				}
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
				usleep(20000);//20ms there can be cancel 
				mms_dialup_disconnect(port);
				continue;
			}
		}

		mms_dialup_disconnect(port);
		
		//mms decoder
		if( MMSDecode(msg, mem->buf, mem->size) == 0){
			result_item.result_buf.port = atoi(item.tv[MMS_ID_PORT].value);
			strncpy(result_item.result_buf.phonenumber, msg->from, sizeof(result_item.result_buf.phonenumber));
			tm = localtime(&msg->sec);
			sprintf(result_item.result_buf.time, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
			strncpy(result_item.result_buf.buf, msg->content, sizeof(result_item.result_buf.buf));
			mms_result_queue_put(result_queue, &result_item);
			pthread_cond_signal(&p_mms->mms_result_pthread.result_cond);
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
		MMSFree(msg);
		i++;
		if(mem->size > mem->total){
			mem->buf = (char *)realloc(mem->buf, mem->total);
		}
		mem->size = 0;
		usleep(20000);//20ms
	}
//	mms_dialup_disconnect();
}


void mms_dl_init(void *data){
	mms_t *p_mms = (mms_t *)data;
	mms_dl_pthread_info_t *ptr_info = &p_mms->mms_dl_pthread;
	ptr_info->wait_queue = NULL;
	ptr_info->fail_queue = NULL;
	ptr_info->wait_queue = queue_init(ptr_info->wait_queue);
	ptr_info->fail_queue = queue_init(ptr_info->fail_queue);
	pthread_mutex_init(&ptr_info->mms_lock, NULL);
	pthread_cond_init(&ptr_info->mms_cond, NULL);
	ptr_info->mms_msg = (mms_message*)malloc(sizeof(mms_message));
	
	//init mms_memory
	ptr_info->mms_memory = (mms_memory_t *)malloc(sizeof(mms_memory_t));
	ptr_info->mms_memory->total = MMS_MOREORY_DEAULT;
	ptr_info->mms_memory->size = 0;
	ptr_info->mms_memory->buf = (char *)malloc(ptr_info->mms_memory->total);
}

void *mms_create_dl_thread(void *data){
	int ret;
	mms_t *p_mms = (mms_t *)data;
	pthread_attr_t attr;
	mms_dl_pthread_info_t *ptr_info = &p_mms->mms_dl_pthread;
	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		MMS_LOG_PRINT(LOG_ERROR,"goto pthread_attr_setstacksize failed.\n");
		return (void*)NULL;
	}
	
	ret = pthread_create(&ptr_info->dl_thread_id, &attr, mms_curl_handler, data);
	if(ret != 0)
		MMS_LOG_PRINT(LOG_ERROR, "goto pthread_create failed.\n");
	else
		MMS_LOG_PRINT(LOG_DEBUG, "pthread_create succ. ret=%d\n", ret);
	return (void*)NULL;
}

void mms_dl_deinit(void *data){
	mms_t *p_mms = (mms_t *)data;
	void *status;
	mms_dl_pthread_info_t *ptr_info = &p_mms->mms_dl_pthread;
	pthread_cancel(ptr_info->dl_thread_id);
	pthread_join(ptr_info->dl_thread_id, &status);
	pthread_mutex_destroy(&ptr_info->mms_lock);
	pthread_cond_destroy(&ptr_info->mms_cond);
	queue_destroy(ptr_info->wait_queue);
	queue_destroy(ptr_info->fail_queue);
	if(ptr_info->mms_memory->buf)
		free(ptr_info->mms_memory->buf);
	if(ptr_info->mms_memory)
		free(ptr_info->mms_memory);
	if(ptr_info->mms_msg)
		free(ptr_info->mms_msg);
}
