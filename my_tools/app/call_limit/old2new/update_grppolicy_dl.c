#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#define GW_GSM_SETTINS "/etc/asterisk/gw_gsm.conf"
#define EXTRA_CHANNELS "/etc/asterisk/extra-channels.conf"

#define CFG_GW_GSM_SETTINS "/etc/cfg/gw_gsm.conf"
#define CFG_EXTRA_CHANNELS "/etc/cfg/extra-channels.conf"

#define CALL_LIMIT_SETTINGS "/etc/asterisk/gw/call_limit/calllimit_settings.conf"
#define CALL_LIMIT_STATUS "/etc/asterisk/gw/call_limit/calllimit_statues.conf"

#define CFG_CALL_LIMIT_SETTINGS "/etc/cfg/gw/call_limit/calllimit_settings.conf"
#define CFG_CALL_LIMIT_STATUS "/etc/cfg/gw/call_limit/calllimit_statues.conf"

#define AUTO_RESET_TIME_DIR "/data/diallimit/gsm/%d"
#define HW_INFO_FILE "/tmp/hw_info.cfg"
#define CALL_LIMIT_PATH "/etc/asterisk/gw/call_limit/"


#define GW_ROUTING_CONF "/etc/asterisk/gw_routing.conf"
#define GW_GROUP_CONF    "/etc/asterisk/gw_group.conf"
#define EXTENSIONS_ROUTING_CONF "/etc/asterisk/extensions_routing.conf"
#define CFG_GW_ROUTING_CONF "/etc/cfg/gw_rouing.conf"
#define CFG_GW_GROUP_CONF     "/etc/cfg/gw_group.conf"
#define CFG_EXTENSIONS_ROUTING_CONF  "/etc/cfg/extensions_routing.conf"
#define GW_LOGGER_CONF "/etc/asterisk/gw.conf"

#define GRP_HEADER "rtg-"
#define SIP_HEADER "sip-"

#define MAX_CHANNEL (44)
#define PATH_SIZE 256
#define LINE_SIZE 128
#define NAME_SIZE 256

struct dail_limit{
	unsigned int dl_step; 
	unsigned int dl_single_limit; 
	unsigned char dl_total_sw;
	unsigned char dl_single_sw; 
	unsigned int dl_total_limit; 
	unsigned int dl_remain_time;
	unsigned int dl_free_time;
	unsigned int dl_warning_time; 
	char dl_warning_num[32]; 
	char  dl_warning_describe[128]; 
	unsigned int  dl_auto_reset_sw;
	unsigned int  dl_auto_reset_type; 
	struct tm dl_auto_reset_date;
};

struct file_content{
	char filename[256];
	char *content;
	int filesize;
};
struct dail_limit dl[MAX_CHANNEL];

const char *str_replace(char *path)
{
	int i;
	int len = strlen(path);
	for(i=0; i<len; i++) {
		if(path[i] == '/') {
			path[i] = '_';
		}
	}
	return path;
}

static int lock_file(const char* path)
{
	char temp_path[PATH_SIZE];
	char lock_path[PATH_SIZE];
	int fd;

	snprintf(temp_path, PATH_SIZE, "%s", path);
	snprintf(lock_path, PATH_SIZE, "/tmp/lock/%s.lock", str_replace(temp_path));

	fd = open(lock_path, O_WRONLY|O_CREAT);
	if(fd <= 0) {
		printf("%s Can't open %s\n",__FUNCTION__, lock_path);
		return -1;
	}

	//Lock
	flock(fd, LOCK_EX);

	return fd;
}


static int unlock_file(int fd)
{
	if(fd <= 0) {
		return -1;
	} else {
		//UnLock
		flock(fd,LOCK_UN);
		close(fd);
		return 0;
	}
}


static int get_option_value(const char * file_path, const char * context_name, const char * option_name, char * out_value)
{
	char buf[LINE_SIZE];
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	FILE* fp;
	char name[NAME_SIZE];
	int lock;

	lock=lock_file(file_path);

	if( NULL == (fp=fopen(file_path,"r")) ) {
		
		unlock_file(lock);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
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
						unlock_file(lock);
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
	unlock_file(lock);
	return -1;
}


int get_hw_info_cfg(void)
{
	int res = -1;
	char tmp[NAME_SIZE] = {0};
	int total_channel;
	if(!(res = get_option_value(HW_INFO_FILE, "sys", "total_chan_count", tmp))) {
		total_channel = atoi(tmp);
	} else {
		printf("%s get sys total_channels fail.\n", __FUNCTION__);
		return -1;
	}
	
	return total_channel;

}

static int get_diallimit_clean_date(int channel, int *remain, struct tm *date){
	char path[128];
	char buf1[128] = {0};
	char buf2[128] = {0};
	int handle;
	int res;
	sprintf(path, AUTO_RESET_TIME_DIR, channel);
	handle = open(path, O_RDONLY);
	if(handle < 0){
		memset(date, 0, sizeof(struct tm));
		return 0;
	}
	read(handle, buf1, sizeof(buf1));
	sscanf(buf1,"%d\n%128s",remain,buf2);
	strptime(buf2, "%Y-%m-%d-%H:%M:%S", date);
	close(handle);
	return 0;
}

int get_gsm_settings(int channels, struct dail_limit* gsm_dl){
	int dl_flag = 0;
	int loop_flag = 0;
	char context[16];
	char tmp[256];
	struct dail_limit *dl;
	int i = 0;
	for(i = 0; i < channels; i++){
		sprintf(context, "%d", i+1);
		dl = &gsm_dl[i];
		if(get_option_value(GW_GSM_SETTINS, context, "dl_single_sw", tmp) == 0){
			if(strcmp(tmp, "on") == 0){
				dl->dl_single_sw= 1;
				loop_flag = 1;
				dl_flag = 1;
			}else{
				dl->dl_single_sw = 0;
			}
		}
		
		if(get_option_value(GW_GSM_SETTINS, context, "dl_total_sw", tmp) == 0 ){
			if(strcmp(tmp, "on") == 0){
				dl->dl_total_sw= 1;
				loop_flag = 1;
				dl_flag = 1;
			}else{
				dl->dl_total_sw = 0;
			}
		}
		if(loop_flag == 0){
			dl->dl_step = 0;
			dl->dl_single_limit= 0;
			dl->dl_total_limit= 0;
			dl->dl_free_time= 0;
			dl->dl_warning_time= 0;
			dl->dl_auto_reset_type = 0;
			memset(dl->dl_warning_num, 0, sizeof(dl->dl_warning_num));
			memset(dl->dl_warning_describe, 0, sizeof(dl->dl_warning_describe));
			continue;
		}
		if(get_option_value(GW_GSM_SETTINS, context, "dl_step", tmp) == 0 ){	
			dl->dl_step= atoi(tmp);
		}
		if(get_option_value(GW_GSM_SETTINS, context, "dl_single_limit", tmp) == 0 ){
			dl->dl_single_limit= atoi(tmp);
		}

		if(get_option_value(GW_GSM_SETTINS, context, "dl_total_limit", tmp) == 0 ){
			dl->dl_total_limit= atoi(tmp);
		}

		if(get_option_value(GW_GSM_SETTINS, context, "dl_free_time", tmp) == 0 ){
			dl->dl_free_time= atoi(tmp);
		}

		if(get_option_value(GW_GSM_SETTINS, context, "dl_warning_time", tmp) == 0 ){
			dl->dl_warning_time= atoi(tmp);
		}

		if(get_option_value(GW_GSM_SETTINS, context, "dl_warning_num", tmp) == 0 ){
//			dl->dl_warning_num= atoi(tmp);
			strncpy(dl->dl_warning_num, tmp, sizeof(dl->dl_warning_num));
		}else{
			memset(dl->dl_warning_num, 0, sizeof(dl->dl_warning_num));
		}

		if(get_option_value(GW_GSM_SETTINS, context, "dl_warning_describe", tmp) == 0 ){
			strncpy(dl->dl_warning_describe, tmp, sizeof(dl->dl_warning_describe));
		}else{
			memset(dl->dl_warning_describe, 0, sizeof(dl->dl_warning_describe));
		}

		if(get_option_value(GW_GSM_SETTINS, context, "dl_auto_reset_sw", tmp) == 0){
			if(strcmp(tmp, "on") == 0){
				dl->dl_auto_reset_sw = 1;
			}else{
				dl->dl_auto_reset_sw = 0;
			}
		}
		if(get_option_value(GW_GSM_SETTINS, context, "dl_auto_reset_type", tmp) == 0 ){
			if(strcmp(tmp, "day") == 0)
				dl->dl_auto_reset_type = 1;
			else if(strcmp(tmp, "week") == 0)
				dl->dl_auto_reset_type = 2;
			else if(strcmp(tmp, "mon") == 0)
				dl->dl_auto_reset_type = 3;
		}
		memset(tmp, 0, sizeof(tmp));
		get_diallimit_clean_date(i+1, &dl->dl_remain_time ,&dl->dl_auto_reset_date);
		//date
		memset(tmp, 0, sizeof(tmp));
		
	}
	return dl_flag;
}
static int read_file_content( struct file_content *content){
	struct stat st;
	int res;
	if(content == NULL)
		return -1;
	if(access(content->filename, F_OK) != 0)
		return -1;

	int handle = open(content->filename, O_RDONLY);
	if(handle < 0){
		return -1;
	}

	fstat(handle, &st);

	content->filesize = st.st_size;

	content->content = (char *)malloc(content->filesize+1);

	if(content->content == NULL){
		close(handle);
		return -1;
	}
	memset(content->content, 0,content->filesize+1);
	res = read(handle, content->content, content->filesize);
	if(res != content->filesize )
		return -1;
	close(handle);
	return 0;
}

static int update_gsm_settings(int channels){
	struct file_content gsm_settings = {
		.filename=GW_GSM_SETTINS,
	};
	char tmp_file[1024];
	strcpy(tmp_file, "/tmp/calllimit_tmp.conf");

	if(read_file_content(&gsm_settings)<0){
		return 0;
	}
	
	FILE* handle = fopen(tmp_file, "w+");
	char *value = NULL;
	value = strtok(gsm_settings.content, "\n");
	while(value != NULL){
		while(*value == ' ')value++;
		//delete dail limit param
		if(strncmp(value, "dl_step", strlen("dl_step")) == 0 ||
			strncmp(value, "dl_single_sw", strlen("dl_single_sw")) == 0||
			strncmp(value, "dl_single_limit", strlen("dl_single_limit")) == 0||
			strncmp(value, "dl_total_sw", strlen("dl_total_sw")) == 0||
			strncmp(value, "dl_total_limit", strlen("dl_total_limit")) == 0||
			strncmp(value, "dl_free_time", strlen("dl_free_time")) == 0||
			strncmp(value, "dl_warning_time", strlen("dl_warning_time")) == 0||
			strncmp(value, "dl_warning_num", strlen("dl_warning_num")) == 0||
			strncmp(value, "dl_warning_describe", strlen("dl_warning_describe")) == 0||
			strncmp(value, "dl_auto_reset_sw", strlen("dl_auto_reset_sw")) == 0||
			strncmp(value, "dl_auto_reset_type", strlen("dl_auto_reset_type")) == 0||
			strncmp(value, "dl_auto_reset_date", strlen("dl_auto_reset_date")) == 0
			){
			value = strtok(NULL, "\n");
			continue;
		}else{
			fprintf(handle, "%s\n", value);
		}
		value = strtok(NULL, "\n");
	}

	fclose(handle);

	unlink(GW_GSM_SETTINS);
	
	rename(tmp_file, GW_GSM_SETTINS);

	free(gsm_settings.content);
	return 0;
}


static int update_extra_channels(int channels){
	struct file_content extra_channels = {
		.filename=EXTRA_CHANNELS,
	};
	char tmp_file[1024];
	strcpy(tmp_file, "/tmp/calllimit_tmp.conf");

	if(read_file_content(&extra_channels)<0){
		return 0;
	}
	
	FILE* handle = fopen(tmp_file, "w+");
	char *value = NULL;
	value = strtok(extra_channels.content, "\n");
	while(value != NULL){
		while(*value == ' ')value++;
		//delete dail limit param
		if(strncmp(value, "dl_step", strlen("dl_step")) == 0 ||
			strncmp(value, "dl_single_sw", strlen("dl_single_sw")) == 0||
			strncmp(value, "dl_single_limit", strlen("dl_single_limit")) == 0||
			strncmp(value, "dl_total_sw", strlen("dl_total_sw")) == 0||
			strncmp(value, "dl_total_limit", strlen("dl_total_limit")) == 0||
			strncmp(value, "dl_free_time", strlen("dl_free_time")) == 0||
			strncmp(value, "dl_warning_time", strlen("dl_warning_time")) == 0||
			strncmp(value, "dl_warning_num", strlen("dl_warning_num")) == 0||
			strncmp(value, "dl_warning_describe", strlen("dl_warning_describe")) == 0||
			strncmp(value, "dl_auto_reset_sw", strlen("dl_auto_reset_sw")) == 0||
			strncmp(value, "dl_auto_reset_type", strlen("dl_auto_reset_type")) == 0||
			strncmp(value, "dl_auto_reset_date", strlen("dl_auto_reset_date")) == 0
			){
			value = strtok(NULL, "\n");
			continue;
		}else if(strncmp(value, "channel=>", strlen("channel=>")) == 0 ||strncmp(value, "channel =>", strlen("channel =>")) == 0){
			fprintf(handle, "%s\n\n", value);
		}else{
			fprintf(handle, "%s\n", value);
		}
		value = strtok(NULL, "\n");
	}

	fclose(handle);

	unlink(EXTRA_CHANNELS);
	
	rename(tmp_file, EXTRA_CHANNELS);

	free(extra_channels.content);
	
	return 0;
}


#define append_calltime_limit_settings(handle, dl) add_new_calllimit_section(handle, dl)

static void add_new_calllimit_section(FILE *handle, struct dail_limit  *dl){

	if( dl->dl_single_sw == 1 ||dl->dl_total_sw == 1 )
		fprintf(handle,"call_time_switch=1\n");
	else
		fprintf(handle,"call_time_switch=0\n");
	fprintf(handle, "call_time_step=%d\n", dl->dl_step);
	fprintf(handle, "call_time_single_switch=%d\n", dl->dl_single_sw);
	fprintf(handle, "call_time_settings=%d\n", dl->dl_single_limit);
	fprintf(handle, "call_time_total_switch=%d\n", dl->dl_total_sw);
	fprintf(handle, "call_time_total=%d\n", dl->dl_total_limit);
	fprintf(handle, "call_time_free=%d\n", dl->dl_free_time);
	fprintf(handle, "call_time_clean_switch=%d\n", dl->dl_auto_reset_sw);
	fprintf(handle, "call_time_clean_type=%d\n", dl->dl_auto_reset_type);
	if(dl->dl_auto_reset_sw > 0){
		char tmp[64] = {0};
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &dl->dl_auto_reset_date);
		fprintf(handle, "call_time_clean_date=%s\n", tmp);	
	}
	fprintf(handle, "call_time_warning_num=%d\n", dl->dl_warning_time);
	fprintf(handle, "call_time_warning_callee=%s\n", dl->dl_warning_num);
	fprintf(handle, "call_time_warning_msg=%s\n", dl->dl_warning_describe);
}

static void create_new_settings(int channels, struct dail_limit  *dl){
	int i = 0;
	FILE *handle;
	if(access(CALL_LIMIT_PATH, F_OK) != 0){
		mkdir(CALL_LIMIT_PATH, 0755);
	}
	handle = fopen(CALL_LIMIT_SETTINGS, "w+");
	for(i = 0; i < channels; i++){
		fprintf(handle, "[%d]\n", i+1);
		add_new_calllimit_section(handle, &dl[i]);
	}
	fclose(handle);
}

static int update_calllimit_settings(int channels, struct dail_limit  *dl){
	
	char tmp_file[1024];
	char *value = NULL;
	FILE* handle = NULL;
	int i = 0;
	int flag = 0;
	int chn;
	struct file_content calllmit_settings = {
		.filename=CALL_LIMIT_SETTINGS,
	};

	strcpy(tmp_file, "/tmp/calllimit_tmp.conf");

	if(read_file_content(&calllmit_settings) < 0){
		create_new_settings(channels, dl);
		return 0;
	}

	 handle = fopen(tmp_file, "w+");
	 value = strtok(calllmit_settings.content, "\n");
	while(value != NULL){
		if(flag == 0 && value[0] == '['){
			if(strncmp(&value[1], "general", strlen("general")) != 0){
				chn = atoi(value+1);
				flag = 1;
			}
		}else if(flag == 1 && value[0] == '['){
			flag = 2;
		}
		
		if(flag == 2){
			if(chn <= channels){
				chn--;
				append_calltime_limit_settings(handle, &dl[chn]);
			}
			flag = 1;
			chn = atoi(value+1);
		}
		if(strncmp(value, "call_time_switch", strlen("call_time_switch")) == 0 ||
			strncmp(value, "call_time_step", strlen("call_time_step")) == 0||
			strncmp(value, "call_time_single_switch", strlen("call_time_single_switch")) == 0||
			strncmp(value, "call_time_settings", strlen("call_time_settings")) == 0||
			strncmp(value, "call_time_total_switch", strlen("call_time_total_switch")) == 0||
			strncmp(value, "call_time_total", strlen("call_time_total")) == 0||
			strncmp(value, "call_time_free", strlen("call_time_free")) == 0||
			strncmp(value, "call_time_clean_switch", strlen("call_time_clean_switch")) == 0||
			strncmp(value, "call_time_clean_type", strlen("call_time_clean_type")) == 0||
			strncmp(value, "call_time_clean_date", strlen("call_time_clean_date")) == 0||
			strncmp(value, "call_time_warning_num", strlen("call_time_warning_num")) == 0||
			strncmp(value, "call_time_warning_callee", strlen("call_time_warning_callee")) == 0||
			strncmp(value, "call_time_warning_msg", strlen("call_time_warning_msg")) == 0
			){
			//noting to do
		}else
			fprintf(handle, "%s\n", value);
		
		value = strtok(NULL, "\n");
	}

	if(channels == 44){
		chn = channels - 1;
		append_calltime_limit_settings(handle, &dl[chn]);
	}
	unlink(CALL_LIMIT_SETTINGS);
	rename(tmp_file,CALL_LIMIT_SETTINGS );
	free(calllmit_settings.content);
}



static void create_new_status(int channels, struct dail_limit  *dl){
	int i = 0;
	FILE *handle;
	if(access(CALL_LIMIT_PATH, F_OK) != 0){
		mkdir(CALL_LIMIT_PATH, 0755);
	}
	handle = fopen(CALL_LIMIT_STATUS, "w+");
	for(i = 0; i < channels; i++){
		fprintf(handle, "[%d]\n", i+1);
		fprintf(handle, "call_time_count=%d\n", dl[i].dl_total_limit - dl[i].dl_remain_time);
		fprintf(handle, "call_time_remain=%d\n", dl[i].dl_remain_time );
	}
}

static int update_calllimit_status(int channels, struct dail_limit  *dl){	
	char tmp_file[1024];
	char *value = NULL;
	FILE* handle = NULL;
	int i = 0;
	int flag = 0;
	int chn;
	struct file_content calllmit_status = {
		.filename=CALL_LIMIT_STATUS,
	};

	strcpy(tmp_file, "/tmp/calllimit_tmp.conf");

	if(read_file_content(&calllmit_status) < 0){
		create_new_status(channels, dl);
		return 0;
	}

	 handle  = fopen(tmp_file, "w+");
	 value = strtok(calllmit_status.content, "\n");
	while(value != NULL){
		if(flag == 0 && value[0] == '['){
			chn = atoi(value+1);
			flag = 1;
		}else if(flag == 1 && value[0] == '['){
			flag = 2;
		}
		
		if(flag == 2){
			if(chn <= channels){
				chn--;
				fprintf(handle, "call_time_count=%d\n", dl[chn].dl_total_limit - dl[chn].dl_remain_time);
				fprintf(handle, "call_time_remain=%d\n", dl[chn].dl_remain_time );
			}
			flag = 1;
			chn = atoi(value+1);
		}
		if(strncmp(value, "call_time_count", strlen("call_time_count")) == 0 ||
			strncmp(value, "call_time_remain", strlen("call_time_remain")) == 0){
			//noting to do
		}else
			fprintf(handle, "%s\n", value);
		
		value = strtok(NULL, "\n");
	}

	if(channels == MAX_CHANNEL){
		chn = channels - 1;
		fprintf(handle, "call_time_count=%d\n", dl[chn].dl_total_limit - dl[chn].dl_remain_time);
		fprintf(handle, "call_time_remain=%d\n", dl[chn].dl_remain_time );
	}
	unlink(CALL_LIMIT_STATUS);
	rename(tmp_file,CALL_LIMIT_STATUS );
	free(calllmit_status.content);
	return 0;
}


void save_file_to_cfg(void){
	char cmd[512] = {0};
	if(access(GW_GSM_SETTINS, F_OK) == 0){
		sprintf(cmd, "cp -rf %s %s", GW_GSM_SETTINS, CFG_GW_GSM_SETTINS);
		system(cmd);
	}
	if(access(EXTRA_CHANNELS, F_OK) == 0){
		sprintf(cmd, "cp -rf %s %s", EXTRA_CHANNELS, CFG_EXTRA_CHANNELS);
		system(cmd);
	}
	if(access(CALL_LIMIT_SETTINGS, F_OK) == 0){
		sprintf(cmd, "cp -rf %s %s", CALL_LIMIT_SETTINGS, CFG_CALL_LIMIT_SETTINGS);
		system(cmd);
	}
	if(access(CALL_LIMIT_STATUS, F_OK) == 0){
		sprintf(cmd, "cp -rf %s %s", CALL_LIMIT_STATUS, CFG_CALL_LIMIT_STATUS);
		system(cmd);
	}
}

static void stop_calllimit(void){
	system("/my_tools/calllimit_cli set chn reflesh");
	system("/etc/init.d/call_limit.sh stop");
}


static int  calllimit_cfg_old2new(void){
	int flag = 0;
	int channels = get_hw_info_cfg();
	if(channels <= 0){
		printf("No vaild channel.\n");
		return 0;
	}
	
	flag = get_gsm_settings(channels, dl);
	if(flag > 0){
		stop_calllimit();
	}
	printf("get gsm_settings succcess!\n");
	
	if(update_gsm_settings(channels) < 0){
		printf("update gw_gsm.conf settings failed!\n");
		return -1;
	}
	printf("update gw_gsm.conf succcess!\n");
	
	if(update_extra_channels(channels)<0){
		printf("update gsm channesl settings failed!\n");
		return -1;
	}
	printf("update  extra_chanels.conf succcess!\n");
	
	if(flag > 0){
		if(update_calllimit_settings(channels,  dl) < 0){
			printf("update call_limit_settings.conf failed!\n");
			return -1;
		}
		printf("update  call_limit_settings.conf succcess!\n");
		if(update_calllimit_status(channels,  dl) < 0){
			printf("update call_limit_status.conf failed!\n");
			return -1;
		}
	}
	
	printf("update  call_limit_status.conf succcess!\n");
	save_file_to_cfg();

}

static int get_grp_order(char *grp_name){
	char tmp[256];
	get_option_value(GW_GROUP_CONF, grp_name, "order", tmp);
	return atoi(tmp);
}

static char* get_grp_type(char *grp_name){
	char c[8]={0};
	char tmp[256];
	if(get_option_value(GW_GROUP_CONF, grp_name, "policy", tmp) == 0){
		if(strcmp(tmp, "ascending") == 0){
			strcpy(c, "g");
		}else if(strcmp(tmp, "descending") == 0){
			strcpy(c, "G");
		}else if(strcmp(tmp, "roundrobin") == 0){
			strcpy(c, "r");
		}else if(strcmp(tmp, "reverseroundrobin") == 0){
			strcpy(c, "R");
		}
	}
	return c;
}


static int is_GrpPolicy_line(char *value, char *rule_name){
	const char *GrpPolicy = "GrpPolicy(";
	char *begin_pos = strstr(value, GrpPolicy);
	if(begin_pos){
		char *pos = begin_pos+strlen(GrpPolicy);
		while(*pos != ')')*rule_name++= *pos++;
		return 1;
	}
	return 0;
}

static int is_Wait_line(char *value){
	const char *Wait = "Wait";
	if(strstr(value, Wait))
		return 1;
	return 0;
}

static int is_Policy_line(char *value){
	const char *Policy="POLICY";
	const char *dial_failover = "dial-failover";
	if(strstr(value, Policy) && !strstr(value, dial_failover))
		return 1;
	return 0;
}

static int is_DialFailover_line(char *value, char *forward_num, char *exten, char *each){
	const char *Policy="POLICY";
	const char *dial_failover = "dial-failover";
	if(strstr(value, Policy) && strstr(value, dial_failover)){
		char *delimit = "=>";
		char *dial_failover = "dial-failover,";
		char *exten_str = "${";
		char *each_pos = strstr(value, delimit) +strlen(delimit) ;
		char *dail_pos = strstr(value, dial_failover) + strlen(dial_failover);
		char *exten_pos = strstr(value,exten_str) + strlen(exten_str);
		if(!dail_pos||!each_pos||!dail_pos)
			return 0;
		while(*each_pos==' ')each_pos++;
		while(*each_pos != ',')*each++ = *each_pos++;//get each string

		while(*dail_pos !=',' )*forward_num++=*dail_pos++;//get forward_num string
		
		while(*exten_pos != '}')*exten++=*exten_pos++;//get exten string
		return 1;
	}
	return 0;
}

static int is_last_hanup_line(char *value){
	const char *hanup= "Hangup()";
	if(strstr(value, hanup))
		return 1;
	return 0;
}

static int is_gotoif_line(char *value){
	const char *gotoif= "Gotoif";
	if(strstr(value, gotoif))
		return 1;
	return 0;
}

static int is_tmpdst_line(char *value){
	const char *tmpdst = "tmpdst";
	if(strstr(value, tmpdst))
		return 1;
	return 0;
}

static int is_include_line(char *value, char *rule_name){
	const char *include_str = "include";
	if(strstr(value, include_str)){
		char *pos = NULL;
		pos = strstr(value, GRP_HEADER) + strlen(GRP_HEADER);
		char *end_pos = strrchr(value, '-');
		if(pos && end_pos){
			while(pos < end_pos)
				*rule_name++ = *pos++;
			return 1;
		}
	}
	return 0;
}
static inline content_compare(char *value, const char *header){
	*value++;
	if(strncmp(value, header, strlen(header)) == 0)
		return 1;
	return 0;
}

static int is_grp_routing_content(char *value){
	return content_compare(value, GRP_HEADER);
}

static int is_sip_routing_content(char *value){
	return content_compare(value, SIP_HEADER);
}

static char *get_grp_name(char *value, char *grp_name){
	char *begin_pos=strstr(value, "rtg-")+strlen("rtg-");
	char *end_pos = strrchr(value, '-');
	char *pos = grp_name;
	while(begin_pos != end_pos)*pos++=*begin_pos++;
	return grp_name;
}

static void update_grppolicy(){
	char tmp_file[256];
	FILE *handle;
	char *value = NULL;
	char rule_name[256];
	char each[32] ={0};
	char policy_name[64];
	char foward_num[64];
	char grp_name[64];
	char exten[64]={0};
	char tmp_buf[256];
	int num;
	int order;
	char c;
	char cdr_sw[16] = {0};
	get_option_value(GW_LOGGER_CONF, "cdr", "switch", cdr_sw); 
	
	struct file_content extensions_routing = {
		.filename = EXTENSIONS_ROUTING_CONF,
	};
//	read_file_content(&extensions_routing);
	strcpy(tmp_file, "/tmp/calllimit_tmp.conf");

	if(read_file_content(&extensions_routing) < 0){
		return;
	}

	 handle  = fopen(tmp_file, "w+");
	 value = strtok(extensions_routing.content, "\n");

	while(value){
		while(*value == ' ')value++;//clear space
		if(value[0]=='['){
			fprintf(handle, "%s\n", value);
			char rule_name[64] = {0};
			char to_channel[64] = {0};
			if(is_sip_routing_content(value)){//sip-routing
				while((value = strtok(NULL, "\n"))!= NULL){
					while(*value == ' ')value++;//clear space
					if(value[0] == '['){
						fprintf(handle, "\n\n");
						break;
					}
					if(is_include_line(value, rule_name)){
						fprintf(handle, "%s\n", value);
						get_option_value(GW_ROUTING_CONF, rule_name, "to_channel", to_channel); 
					}else if( is_gotoif_line( value)){
						fprintf(handle, "%s\n", value);
						if(strcmp(cdr_sw, "on") == 0 && strstr(to_channel, "grp-")){
							fprintf(handle, "exten => h,n,Set(tmpdst1=${CDR(dstchannel):6})\n");
							fprintf(handle, "exten => h,n,Set(tmpdst2=${CUT(tmpdst1,-,-1)})\n");
							fprintf(handle, "exten => h,n,Set(CDR_TOCHAN=gsm-$[$[${tmpdst2} + 1]/2])\n");
						}
					}else if(is_tmpdst_line(value)){
						//noting to do
					}else{
						fprintf(handle, "%s\n", value);
					}
				}	
			}else if(is_grp_routing_content(value)){//grp-routing
				memset(grp_name, 0, sizeof(grp_name));
				memset(foward_num, 0, sizeof(foward_num));
				memset(exten, 0, sizeof(exten));
				memset(each, 0, sizeof(each));
				while((value = strtok(NULL, "\n"))!= NULL){
					while(*value == ' ')value++;//clear space
					if(value[0] == '['){
						fprintf(handle, "\n\n");
						break;
					}
					if(is_GrpPolicy_line(value, grp_name)){
						//noting to do
					}else if(strlen(grp_name) > 0 &&is_Wait_line(value)){
						//noting to do
					}else if(is_Policy_line(value)){
						//noting to do
					}else if(is_DialFailover_line(value,foward_num, exten, each)){
						fprintf(handle, "exten => %s,n,Macro(dial-failover,%s,${%s},RET,extra/%s%d,0,0)\n", each, foward_num, exten,get_grp_type(grp_name), get_grp_order(grp_name));
					}else{
						fprintf(handle,"%s\n",value);
					}
				}
			}else{
				while((value = strtok(NULL, "\n"))!= NULL){
					while(*value == ' ')value++;//clear space
					if(value[0] == '['){
						fprintf(handle, "\n\n");
						break;
					}
					fprintf(handle, "%s\n", value);
				}
			}
		}else{
			fprintf(handle,"%s\n", value);
			value = strtok(NULL, "\n");
		}
	}
	fclose(handle);
	unlink(EXTENSIONS_ROUTING_CONF);
	rename(tmp_file, EXTENSIONS_ROUTING_CONF);
	free(extensions_routing.content);
}

static int save_routing_file_to_cfg(){
	char cmd[256] ={0};
	if(access(EXTENSIONS_ROUTING_CONF, F_OK) == 0)
		sprintf(cmd, "cp -rf %s %s", EXTENSIONS_ROUTING_CONF, CFG_EXTENSIONS_ROUTING_CONF);
	system(cmd);
	return 0;
}
static int grppolicy_old2new(){
	
	update_grppolicy();
	save_routing_file_to_cfg();
	return 0;
}
int main(int argc, char **argv){

	calllimit_cfg_old2new();
	grppolicy_old2new();
	
	return 0;
}
