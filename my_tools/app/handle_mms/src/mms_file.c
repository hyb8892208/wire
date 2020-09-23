#include "mms_inc.h"
#include <time.h>
#include "mms_file.h"
#include "mms.h"
#include "mms_redis_if.h"
#include "mms_debug.h"

#define MMS_QUEUE_FILE "/data/log/mms_queue.list"
#define MMS_CONF_FILE "/etc/asterisk/gw/mms.conf"
#define ONTIME_TYPE_STR  "ontime"
#define REALTIME_TYPE_STR  "imme"

int load_mms_queue_conf(  mms_queue_t *queue)
{
	char *linebuf;
	size_t size = 0;
	mms_queue_item_t item;
	item.next = NULL;
	if(access(MMS_QUEUE_FILE, F_OK) != 0){
		MMS_LOG_PRINT(LOG_WARNING, "file is not exist\n");
		return -1;
	}
	FILE *handle  = fopen(MMS_QUEUE_FILE, "r");
	if(handle == NULL){
		MMS_LOG_PRINT(LOG_ERROR, "open file filed\n");
		return -1;
	}
	while (getline(&linebuf, &size, handle) != -1){
		if(linebuf[0] == '#')
			continue;
		memset(&item, 0, sizeof(mms_queue_item_t));
		sscanf(linebuf, "%d %s %s %s", &item.try_count, item.tv[MMS_ID_TYPE].value, item.tv[MMS_ID_PORT].value, item.tv[MMS_ID_URL].value );
		queue_put(queue, &item);
		//queue_put_sort(queue,item);
	}
	free(linebuf);
	fclose(handle);
	//clear mms queue list
	truncate(MMS_QUEUE_FILE, 0);
	return 0;
}


int flush_mms_queue_conf(mms_t *p_mms)
{
	char buf[256] = {0};
	int flag = 0;
	mms_queue_t *wait_queue =  p_mms->mms_dl_pthread.wait_queue;
	mms_queue_t *fail_queue =  p_mms->mms_dl_pthread.fail_queue;
	mms_queue_item_t *wait_item;
	mms_queue_item_t fail_item;
	FILE *handle  = fopen(MMS_QUEUE_FILE, "w+");
	if(handle == NULL){
		MMS_LOG_PRINT(LOG_ERROR, "open file filed\n");
		return -1;
	}
	fprintf(handle, "# try_count type port url\n");
	wait_item = queue_get_next(wait_queue, NULL);
	for( ; wait_item; wait_item=queue_get_next(wait_queue, wait_item)){
		sprintf(buf, "%d %s %s %s\n", wait_item->try_count, wait_item->tv[MMS_ID_TYPE].value, wait_item->tv[MMS_ID_PORT].value, wait_item->tv[MMS_ID_URL].value);
		fwrite(buf, 1, strlen(buf), handle);
	}
	while(queue_pop(fail_queue, &fail_item) == 0){
		sprintf(buf, "%d %s %s %s\n", fail_item.try_count, fail_item.tv[MMS_ID_TYPE].value, fail_item.tv[MMS_ID_PORT].value, fail_item.tv[MMS_ID_URL].value);
		fwrite(buf, 1, strlen(buf), handle);
		queue_put(wait_queue, &fail_item);
		flag = 1;
	}
	fclose(handle);
	if(flag == 1)
		pthread_cond_signal(&p_mms->mms_dl_pthread.mms_cond);
	return 0;
}

void *mms_flush_file_hander(void *data){
	mms_t *p_mms = (mms_t *)data;
	int old_state;
	while(1){
		sleep(600);//10minitue
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);//set pthread state to cancel disable state
		flush_mms_queue_conf(p_mms);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);//set pthread state to canncel enable state
	}
}

void mms_flush_file_init(void *data){
	int ret;
	mms_t *ptr_mms = (mms_t *)data;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		MMS_LOG_PRINT(LOG_ERROR,"goto pthread_attr_setstacksize failed.\n");
		return ;
	}

	ret = pthread_create(&ptr_mms->sava_thread_id, &attr, mms_flush_file_hander, data);
	if(ret != 0)
		MMS_LOG_PRINT(LOG_ERROR, "goto pthread_create failed.\n");
	else
		MMS_LOG_PRINT(LOG_INFO, "pthread_create succ. ret=%d\n", ret);
}

void mms_flush_file_deinit(void *data){
	mms_t *ptr_mms = (mms_t *)data;
	void *status;
	pthread_cancel(ptr_mms->sava_thread_id);
	pthread_join(ptr_mms->sava_thread_id, &status);
	//save memory data to flash
	flush_mms_queue_conf(ptr_mms);
}
static int get_option_value(const char * file_path, const char * context_name, const char * option_name, char * out_value)
{
	char buf[MAX_CONTENT_LEN];
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	FILE* fp;
	char name[MAX_VALUE_LEN];
//	int lock;

	//lock=lock_file(file_path);

	if( NULL == (fp=fopen(file_path,"r")) ) {
		
	//	unlock_file(lock);
		return -1;
	}

	while(fgets(buf,MAX_CONTENT_LEN,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0== strcmp(name,context_name) ) {
						finded=1;
					} else {
						if(finded) 
							finish=1;
					}
				}
				break;
			case '=':
				if(finded && !finish) {
					memcpy(name,buf,i);
					name[i] = '\0';
					if(0==strcmp(name,option_name)) {
						memcpy(name,buf+i+1,len-i-1-1);
						name[len-i-1-1] = '\0';
						fclose(fp);
				//		unlock_file(lock);
						memcpy(out_value,name,len-i-1);
						return 0;
					}
				}
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	//unlock_file(lock);
	return -1;
}

int load_mms_conf(mms_config_t *config)
{
	char value[MAX_VALUE_LEN] = {0};
	int i = 0;
	char contex[MAX_VALUE_LEN] = {0};
//	struct tm tm;
	if(access(MMS_CONF_FILE, F_OK) != 0){
		MMS_LOG_PRINT(LOG_WARNING,"%s is not exist\n", MMS_CONF_FILE);
		return -1;
	}
	config->save_url_timeout = 10;
	config->dl_timeout = 20;
	if(get_option_value(MMS_CONF_FILE, "general", "download_type", value) == 0){
		if(strcmp(value, ONTIME_TYPE_STR) == 0)
			config->dl_type = MMS_DL_URL_TIME;
		else if(strcmp(value, REALTIME_TYPE_STR) == 0)
			config->dl_type = MMS_DL_URL_IMME;
		else
			config->dl_type = MMS_DL_URL_OFF;
	}
	if(config->dl_type == MMS_DL_URL_TIME && get_option_value(MMS_CONF_FILE, "general", "download_time", value) == 0){
		if(strlen(value) > 0){
			strptime(value, "%H:%M:%S", &config->dl_time);
		}
	}
	
	if(get_option_value(MMS_CONF_FILE, "general", "save_url_flag", value) == 0){
		if(strcmp(value, "yes") == 0)
			config->save_url_flag = 1;
		else
			config->save_url_flag = 0;
	}

	if(get_option_value(MMS_CONF_FILE, "general", "save_url_timeout", value) == 0){
		config->save_url_timeout = atoi(value);
	}

	if(get_option_value(MMS_CONF_FILE, "general", "dl_timeout", value) == 0){
		config->dl_timeout = atoi(value);
	}
	
	for(i = 0; i < _MAX_CHANNEL_COUNT_; i++){
		sprintf(contex, "%d", i+1);
		memset(value, 0 ,sizeof(value));
		if(get_option_value(MMS_CONF_FILE, contex, "apn", value) == 0){
			strncpy(config->dialup_info[i].apn, value, MAX_URL_LEN);
		}
		memset(value, 0 ,sizeof(value));
		if(get_option_value(MMS_CONF_FILE, contex, "username", value) == 0){
			strncpy(config->dialup_info[i].username, value, MAX_USERNAME_LEN);
		}
		memset(value, 0 ,sizeof(value));
		if(get_option_value(MMS_CONF_FILE, contex, "password", value) == 0){
			strncpy(config->dialup_info[i].username, value, MAX_PASSOWRD_LEN);
		}
		memset(value, 0 ,sizeof(value));
		if(get_option_value(MMS_CONF_FILE, contex, "dns", value) == 0){
			strncpy(config->dialup_info[i].dns, value, MAX_VALUE_LEN);
		}else
			config->dialup_info[i].dns[0] = '\0';
	}
	MMS_LOG_PRINT(LOG_DEBUG,"load conf success\n");
	return 0;
}

void mms_config_deinit(mms_config_t *config){
	if(config)
		free(config);
	config = NULL;
}

int mms_config_init(mms_t *p_mms){
	p_mms->mms_config = (mms_config_t*)malloc(sizeof(mms_config_t));
	memset(p_mms->mms_config, 0, sizeof(mms_config_t));
	return 0;	
}

