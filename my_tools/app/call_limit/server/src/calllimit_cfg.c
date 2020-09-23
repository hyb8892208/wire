#include "../include/header.h"
#include "../include/calllimit.h"
#include "../include/calllimit_cfg.h"
#include "../include/calllimit_log.h"



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
		print_error("%s Can't open %s\n",__FUNCTION__, lock_path);
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


static int is_file_exist(const char *file_path)
{
	if(NULL == file_path) {
		return -1;
	} 
	if(0 == access(file_path, F_OK)) {
		return 0;
	} 
	return -1;
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

#if 0
static int get_hw_info_effective_channels(hw_info_cfg_t *hw_info_cfg)
{
	int i = 0;
	int res = -1;
	int offset = 0;
	char chan_type[NAME_SIZE] = {0};
	char getvalue[NAME_SIZE] = {0};

	if(0 == hw_info_cfg->effective_channum) {
		print_error("%s mode no effective channels.\n", __FUNCTION__);
		return -1;
	}

	if(hw_info_cfg->total_channels > MAX_CHAN) {
		print_error("%s total chan(%d) out of range.\n", __FUNCTION__, hw_info_cfg->total_channels);
		return -1;		
	}

	for(i=0; i<(int)hw_info_cfg->total_channels; i++){
		sprintf(chan_type, "chan_%d_type", i+1);
		if(!(res = get_option_value(HW_INFO_FILE, "channel", chan_type, getvalue))) {
			if(strcmp(getvalue, "unkown")) {
				hw_info_cfg->effective_channum[offset++] = i;
	//			print_debug("effective_chan_%d\n", hw_info_cfg->effective_channum[offset-1]);	
			}
		} else {
			continue;
		}		
		memset(chan_type, 0, NAME_SIZE);
		memset(getvalue, 0, NAME_SIZE);
	}	

	return 0;
}
#endif

int get_sys_info_cfg(sys_info_cfg_t *sys_info_cfg)
{
	int res = -1;
	char tmp[NAME_SIZE] = {0};

	if(!sys_info_cfg) {
		log_printf(LOG_ERROR, "hw_info_cfg is null\n");
		return -1;
	}
	
	if(!(res = get_option_value(HW_INFO_FILE, "sys", "total_chan_count", tmp))) {
		sys_info_cfg->total_chans = atoi(tmp);
	} else {
		log_printf(LOG_ERROR, "get sys total_channels fail\n");
		goto err_out;
	}	

	if(!(res = get_option_value(HW_INFO_FILE, "sys", "sys_type", tmp))) {
		sys_info_cfg->sys_type = atoi(tmp);
		log_printf(LOG_DEBUG, "sys_type=%d\n", sys_info_cfg->sys_type);
	} else {
		log_printf(LOG_ERROR, "get sys sys_type fail.\n");
		goto err_out;
	}	

	return 0;
	
err_out:
	sys_info_cfg->sys_type = CHAN_1SIM;
	sys_info_cfg->total_chans = MAX_CHAN;
	return -1;
}


static int create_filepath(const char * filepath)
{
	struct stat file_stat;
	int ret = -1;

	if(!filepath) {
		return -1;
	}

	ret = stat(filepath, &file_stat);
	if(ret < 0) {
		if(errno == ENOENT) {		 
			ret = mkdir(filepath, 0775);
			print_debug("creat dir '/%s'/\n", filepath);
			if(ret < 0) {
				print_error("Could not create directory \'%s\' \n",
					filepath);
				return -1;
			}
		} else {
			print_error("bad file path\n");
			return -1;
		}
	}
	return 0;
}

static int create_call_limit_settings_cfg(calllimit_t *call_limit)
{
	int i = 0;
	FILE *fp;
	calllimit_chan_t *p_chan = NULL;

	p_chan = call_limit->chans;
	create_filepath(CFG_FILE_PATH);
		
	if( NULL == (fp = fopen(SETTINGS_FILE, "w+")) ) {
		log_printf(LOG_ERROR, "Can't create %s\n", SETTINGS_FILE);
		return -1;
	}

       fprintf(fp, "[general]\n");
       fprintf(fp, "IsRefleshFile=%d\n",call_limit->sys_info.IsRefleshFile);
	for(i=0; i<MAX_CHAN; i++) {
		fprintf(fp, "[%d]\n", i+1);
		fprintf(fp, "call_time_switch=%d\n", p_chan->cfg.calltime_cfg.call_time_sw);
		fprintf(fp, "call_time_single_switch=%d\n", p_chan->cfg.calltime_cfg.call_time_single_sw);
		fprintf(fp, "call_time_settings=%d\n", p_chan->cfg.calltime_cfg.call_time_settings);
		fprintf(fp, "call_time_total_switch=%d\n", p_chan->cfg.calltime_cfg.call_time_total_sw);
		fprintf(fp, "call_time_total=%d\n", p_chan->cfg.calltime_cfg.call_time_total);
		fprintf(fp, "call_time_free=%d\n", p_chan->cfg.calltime_cfg.call_time_free);
		fprintf(fp, "call_time_clean_switch=%d\n", p_chan->cfg.calltime_cfg.call_time_clean_sw);
		fprintf(fp, "call_time_clean_type=%d\n", p_chan->cfg.calltime_cfg.call_time_clean_type);
		fprintf(fp, "call_time_step=%d\n", p_chan->cfg.calltime_cfg.call_time_step);
		fprintf(fp, "day_calls_settings=%d\n", p_chan->cfg.calllimit_cfg.day_calls_settings);
		fprintf(fp, "day_answer_setting=%d\n", p_chan->cfg.calllimit_cfg.day_answer_setting);
		fprintf(fp, "hour_calls_settings=%d\n", p_chan->cfg.calllimit_cfg.hour_calls_settings);
		fprintf(fp, "call_limit_switch=%d\n", p_chan->cfg.calllimit_cfg.calllimit_switch);
		fprintf(fp, "call_detect_flag=%d\n", p_chan->cfg.calllock_cfg.call_detect_flag);
		fprintf(fp, "call_fail_mark_flag=%d\n", p_chan->cfg.calllock_cfg.call_fail_mark_flag);
		fprintf(fp, "call_fail_mark_count=%d\n", p_chan->cfg.calllock_cfg.call_fail_mark_count);
		fprintf(fp, "call_fail_lock_flag=%d\n", p_chan->cfg.calllock_cfg.call_fail_lock_flag);
		fprintf(fp, "call_fail_lock_count=%d\n", p_chan->cfg.calllock_cfg.call_fail_lock_count);
		fprintf(fp, "call_fail_lock_sms_flag=%d\n", p_chan->cfg.calllock_cfg.call_fail_lock_sms_flag);
		fprintf(fp, "call_fail_lock_sms_count=%d\n", p_chan->cfg.calllock_cfg.call_fail_lock_sms_count);
		fprintf(fp, "call_fail_lock_sms_report_flag=%d\n", p_chan->cfg.calllock_cfg.call_fail_lock_sms_report_flag);
		fprintf(fp, "call_fail_lock_sms_callee=%s\n", p_chan->cfg.calllock_cfg.call_fail_lock_sms_callee);
		fprintf(fp, "call_fail_lock_sms_msg=%s\n", p_chan->cfg.calllock_cfg.call_fail_lock_sms_msg);
		fprintf(fp, "sms_limit_switch=%d\n", p_chan->cfg.smslimit_cfg.smslimit_switch);
        fprintf(fp, "sms_limit_success_flag=%d\n", p_chan->cfg.smslimit_cfg.smslimit_success_flag);
		fprintf(fp, "day_sms_settings=%d\n", p_chan->cfg.smslimit_cfg.day_sms_settings);
		fprintf(fp, "mon_sms_settings=%d\n", p_chan->cfg.smslimit_cfg.mon_sms_settings);
		fprintf(fp, "sms_clean_date=%d\n", p_chan->cfg.smslimit_cfg.sms_clean_date);
		fprintf(fp, "sms_warning_switch=%d\n", p_chan->cfg.smslimit_cfg.sms_warning_switch);
		fprintf(fp, "sms_limit_warning_num=%d\n", p_chan->cfg.smslimit_cfg.smslimit_mon_warning_num);
		fprintf(fp, "sms_limit_warning_callee=%s\n", p_chan->cfg.smslimit_cfg.smslimit_mon_warning_callee);
		fprintf(fp, "sms_limit_warning_msg=%s\n", p_chan->cfg.smslimit_cfg.smslimit_mon_warning_msg);
		fprintf(fp, "call_time_warning_num=%d\n", p_chan->cfg.calltime_cfg.call_time_warning_num);
		fprintf(fp, "call_time_warning_callee=%s\n", p_chan->cfg.calltime_cfg.call_time_warning_callee);
		fprintf(fp, "call_time_warning_msg=%s\n", p_chan->cfg.calltime_cfg.call_time_warning_msg);
		fprintf(fp, "sim_policy=%d\n", p_chan->cfg.simswitch_cfg.sim_policy);
		fprintf(fp, "sim_switch_sw=%d\n", p_chan->cfg.simswitch_cfg.sim_switch_sw);
		fprintf(fp, "sim_reg_timeout=%d\n", p_chan->cfg.simswitch_cfg.sim_reg_timeout);
		fprintf(fp, "total_sms_count=%d\n", p_chan->cfg.simswitch_cfg.total_sms_count);
		fprintf(fp, "total_callout_count=%d\n", p_chan->cfg.simswitch_cfg.total_callout_count);
		fprintf(fp, "total_callout_time=%d\n", p_chan->cfg.simswitch_cfg.total_callout_time);
		fprintf(fp, "total_using_time=%d\n", p_chan->cfg.simswitch_cfg.total_using_time);
		p_chan++;
	}
	fclose(fp);
	return 0;
}


static int create_call_limit_status_cfg(calllimit_t *call_limit)
{
	int i = 0;
	int j = 0;
	int sim_num = 1;
	FILE *fp;
	calllimit_chan_t *p_chan = call_limit->chans;
	sys_info_cfg_t *sys_info = &call_limit->sys_info;

	if( NULL == (fp = fopen(STATUS_FILE, "w+")) ) {
		log_printf(LOG_ERROR, "Can't create %s\n", STATUS_FILE);
		return -1;
	}

	if(CHAN_4SIM == sys_info->sys_type)
		sim_num = SIM_NUM;
	
	for(i=0; i<MAX_CHAN; i++) {
		for(j=0; j<sim_num; j++) {
			
			if(CHAN_4SIM == sys_info->sys_type) 
				fprintf(fp, "[%d-%d]\n", i+1, j+1);
			else
				fprintf(fp, "[%d]\n", i+1);
			
			fprintf(fp, "call_time_count=%d\n", p_chan->sim[j].calltime_info.call_time_count);
			fprintf(fp, "call_time_remain=%d\n", p_chan->sim[j].calltime_info.call_time_remain);
			fprintf(fp, "call_time_limit_flag=%d\n", p_chan->sim[j].calltime_info.call_time_limit_flag);
			if(p_chan->cfg.calltime_cfg.call_time_clean_type !=  CALL_TIME_CLEAN_TYPE_UNKOWN){
				char tmp_buf[64] = {0};
				strftime(tmp_buf, sizeof(tmp_buf), "%Y-%m-%d %H:%M:%S", &p_chan->sim[j].calltime_info.call_time_clean_date);
				fprintf(fp, "call_time_clean_date=%s\n", tmp_buf);
			}
			fprintf(fp, "day_call_limit_flag=%d\n", p_chan->sim[j].calllimit_info.day_call_limit_flag);
			fprintf(fp, "hour_call_limit_flag=%d\n", p_chan->sim[j].calllimit_info.hour_call_limit_flag);
			fprintf(fp, "day_answer_limit_flag=%d\n", p_chan->sim[j].calllimit_info.day_answer_limit_flag);
			fprintf(fp, "day_total_calls=%d\n", p_chan->sim[j].calllimit_info.day_cur_calls);
			fprintf(fp, "hour_total_calls=%d\n", p_chan->sim[j].calllimit_info.hour_cur_calls);
			fprintf(fp, "day_total_answers=%d\n", p_chan->sim[j].calllimit_info.day_cur_answers);
	        fprintf(fp, "call_failed_count=%d\n", p_chan->sim[j].calllock_info.call_failed_count);
			fprintf(fp, "call_fail_mark_status=%d\n", p_chan->sim[j].calllock_info.call_fail_mark_status);
			fprintf(fp, "call_fail_lock_status=%d\n", p_chan->sim[j].calllock_info.call_fail_lock_status);
			fprintf(fp, "day_sms_limit_flag=%d\n", p_chan->sim[j].smslimit_info.day_sms_limit_flag);
			fprintf(fp, "mon_sms_limit_flag=%d\n", p_chan->sim[j].smslimit_info.mon_sms_limit_flag);
			fprintf(fp, "day_total_sms=%d\n", p_chan->sim[j].smslimit_info.day_cur_sms);
			fprintf(fp, "mon_total_sms=%d\n", p_chan->sim[j].smslimit_info.mon_cur_sms);
			fprintf(fp, "sms_limit_warning_flag=%d\n", p_chan->sim[j].smslimit_info.sms_limit_warning_flag);
			fprintf(fp, "call_time_warning_flag=%d\n", p_chan->sim[j].calltime_info.call_time_warning_flag);
			fprintf(fp, "sim_idx=%d\n", p_chan->sim_idx);
//			fprintf(fp, "sim_using_time=%d\n", p_chan->sim[j].sim_using_time);
//			fprintf(fp, "sim_callout_time=%d\n", p_chan->sim[j].sim_callout_time);
//			fprintf(fp, "sim_callout_count=%d\n", p_chan->sim[j].sim_callout_count);
//			fprintf(fp, "sim_sms_count=%d\n", p_chan->sim[j].sim_sms_count);
		}
		p_chan++;
	}
	fclose(fp);
	return 0;
}


static int create_sim_info_cfg(calllimit_t *call_limit)
{
	int i = 0;
	int j = 0;
	FILE *fp;
	calllimit_chan_t *p_chan = call_limit->chans;
	sys_info_cfg_t *sys_info = &call_limit->sys_info;

	if(sys_info->sys_type != CHAN_4SIM)
		return 0;
	
	if( NULL == (fp = fopen(SIM_INFO_FILE, "w+")) ) {
		log_printf(LOG_ERROR, "Can't create %s\n", SIM_INFO_FILE);
		return -1;
	}
	
	for(i=0; i<MAX_CHAN; i++) {
		for(j=0; j<SIM_NUM; j++) {
			fprintf(fp, "[%d-%d]\n", i+1, j+1);
			fprintf(fp, "pincode=%s\n", p_chan->sim[j].sim_ping_code);
		}
		p_chan++;
	}
	fclose(fp);
	return 0;
}



static int read_call_limit_settings_cfg(calllimit_t *call_limit)
{
	int res = -1;
	int i = 0, j = 0;
	char tmp[NAME_SIZE] = {0};
	char context_name[NAME_SIZE] = {0};
	calllimit_chan_t *p_chan = call_limit->chans;  
	sys_info_cfg_t *sys_info_cfg = &call_limit->sys_info;
	
	if(!(res = get_option_value(SETTINGS_FILE, "general", "IsRefleshFile", tmp))) {
	    sys_info_cfg->IsRefleshFile= atoi(tmp);
	} else {
		log_printf(LOG_INFO, "get general IsRefleshFile fail.\n");
	}

	for(i=0; i<(int)sys_info_cfg->total_chans; i++) {
		
		calllock_conf_t *pcalllock = &p_chan->cfg.calllock_cfg;
		calllimit_conf_t *pcalllimit = &p_chan->cfg.calllimit_cfg;  
		calltime_conf_t *pcalltime = &p_chan->cfg.calltime_cfg;   
		smslimit_conf_t *psmslimit = &p_chan->cfg.smslimit_cfg;
		
		sprintf(context_name, "%d", i+1);
		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_switch", tmp))) {
			pcalltime->call_time_sw= atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_switch fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_single_switch", tmp))) {
			pcalltime->call_time_single_sw= atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_single_switch fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_settings", tmp))) {
			pcalltime->call_time_settings = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_settings fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_total_switch", tmp))) {
			pcalltime->call_time_total_sw= atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_total_switch fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_total", tmp))) {
			pcalltime->call_time_total = atoi(tmp);
			for(j=0; j<SIM_NUM; j++) {
				p_chan->sim[j].calltime_info.call_time_remain = pcalltime->call_time_total -  p_chan->sim[j].calltime_info.call_time_count;
				if(p_chan->sim[j].calltime_info.call_time_remain < 0)
					p_chan->sim[j].calltime_info.call_time_remain = 0;
				p_chan->sim[j].calltime_info.call_time_redis_flag = FLAG_NO;
			}
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_total fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_free", tmp))) {
			pcalltime->call_time_free = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_free_time fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_clean_switch", tmp))) {
			pcalltime->call_time_clean_sw= atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_clean_switch fail.\n", i+1);
		}
		
		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_clean_type", tmp))) {
			pcalltime->call_time_clean_type= atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_clean_type fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_clean_date", tmp))) {
			for(j=0; j<SIM_NUM; j++)
				strptime(tmp, "%Y-%m-%d %H:%M:%S", &p_chan->sim[j].calltime_info.call_time_clean_date);
		} else {
	//		log_printf(LOG_INFO, "get chan_%d call_time_clean_date fail.", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_step", tmp))) {
			pcalltime->call_time_step = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_step fail.\n", i+1);
		}
		if(!(res = get_option_value(SETTINGS_FILE, context_name, "day_calls_settings", tmp))) {
			pcalllimit->day_calls_settings = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d day_calls_settings fail.\n", i+1);
		}	
		
		if(!(res = get_option_value(SETTINGS_FILE, context_name, "day_answer_setting", tmp))) {
			pcalllimit->day_answer_setting = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d day_answer_setting fail.\n", i+1);
		}	

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "hour_calls_settings", tmp))) {
			pcalllimit->hour_calls_settings = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d hour_calls_settings fail.\n", i+1);
		}			

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_limit_switch", tmp))) {
			pcalllimit->calllimit_switch = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_limit_switch fail.\n", i+1);
		}
              
		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_detect_flag", tmp))) {
			pcalllock->call_detect_flag = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_check_option fail.\n", i+1);
		}

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_mark_flag", tmp))) {
			pcalllock->call_fail_mark_flag= atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_fail_mark_flag fail.\n", i+1);
		}

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_mark_count", tmp))) {
			pcalllock->call_fail_mark_count = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_fail_mark_count fail.\n", i+1);
		}

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_lock_flag", tmp))) {
			pcalllock->call_fail_lock_flag = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_fail_lock_flag fail.\n", i+1);
		}

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_lock_count", tmp))) {
			pcalllock->call_fail_lock_count= atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_fail_lock_count fail.\n", i+1);
		}

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_lock_sms_callee", tmp))) {
			memcpy(pcalllock->call_fail_lock_sms_callee,tmp,strlen(tmp));
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_fail_lock_sms_callee fail.\n", i+1);
		}

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_lock_sms_report_flag", tmp))) {
            pcalllock->call_fail_lock_sms_report_flag = atoi(tmp);
        } else {
			log_printf(LOG_DEBUG, "get chan_%d call_fail_lock_sms_report_flag fail.\n", i+1);
        }
            
        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_lock_sms_count", tmp))) {
            pcalllock->call_fail_lock_sms_count = atoi(tmp);
        } else {
        	log_printf(LOG_DEBUG, "get chan_%d call_fail_lock_sms_count fail.\n", i+1);
        }
                
        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_lock_sms_msg", tmp))) {
			memcpy(pcalllock->call_fail_lock_sms_msg,tmp,strlen(tmp));
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_fail_lock_sms_msg fail.\n", i+1);
		}

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_fail_lock_sms_flag", tmp))) {
            pcalllock->call_fail_lock_sms_flag = atoi(tmp);
        } else {
        	log_printf(LOG_DEBUG, "get chan_%d call_fail_lock_sms_flag fail.\n", i+1);
        }

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "sms_limit_switch", tmp))) {
            psmslimit->smslimit_switch = atoi(tmp);
        } else {
        	log_printf(LOG_DEBUG, "get chan_%d sms_limit_switch fail.\n", i+1);
        }	
	
        if(!(res = get_option_value(SETTINGS_FILE, context_name, "day_sms_settings", tmp))) {
            psmslimit->day_sms_settings = atoi(tmp);
        } else {
			log_printf(LOG_DEBUG, "get chan_%d day_sms_settings fail.\n", i+1);
        }

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "mon_sms_settings", tmp))) {
            psmslimit->mon_sms_settings = atoi(tmp);
        } else {
        	log_printf(LOG_DEBUG, "get chan_%d mon_sms_settings fail.\n", i+1);
        }

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "sms_clean_date", tmp))) {
            psmslimit->sms_clean_date= atoi(tmp);
        } else {
        	log_printf(LOG_DEBUG, "get chan_%d sms_clean_date fail.\n", i+1);
        }

        if(!(res = get_option_value(SETTINGS_FILE, context_name, "sms_limit_success_flag", tmp))) {
            psmslimit->smslimit_success_flag= atoi(tmp);
        } else {
        	log_printf(LOG_DEBUG, "get chan_%d sms_limit_success_flag fail.\n", i+1);
        }
		
		if(!(res = get_option_value(SETTINGS_FILE, context_name, "sms_warning_switch", tmp))) {
			psmslimit->sms_warning_switch = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d sms_warning_switch fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "sms_limit_warning_num", tmp))) {
			psmslimit->smslimit_mon_warning_num = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d sms_limit_warning_num fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "sms_limit_warning_callee", tmp))) {
			strncpy(psmslimit->smslimit_mon_warning_callee, tmp, sizeof(psmslimit->smslimit_mon_warning_callee));
		} else {
			log_printf(LOG_DEBUG, "get chan_%d sms_limit_warning_callee fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "sms_limit_warning_msg", tmp))) {
			strncpy(psmslimit->smslimit_mon_warning_msg, tmp, sizeof(psmslimit->smslimit_mon_warning_msg));
		} else {
			log_printf(LOG_DEBUG, "get chan_%d sms_limit_warning_msg fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_warning_num", tmp))) {
			pcalltime->call_time_warning_num = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_warning_num fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_warning_callee", tmp))) {
			strncpy(pcalltime->call_time_warning_callee, tmp, sizeof(pcalltime->call_time_warning_callee));
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_warning_callee fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "call_time_warning_msg", tmp))) {
			strncpy(pcalltime->call_time_warning_msg, tmp, sizeof(pcalltime->call_time_warning_msg));
		} else {
			log_printf(LOG_DEBUG, "get chan_%d call_time_warning_msg fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "sim_policy", tmp))) {
			p_chan->cfg.simswitch_cfg.sim_policy = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d sim_policy fail.\n", i+1);
		}

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "sim_switch_sw", tmp))) {
			p_chan->cfg.simswitch_cfg.sim_switch_sw = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d sim_switch_sw fail.\n", i+1);
		}
		
		if(!(res = get_option_value(SETTINGS_FILE, context_name, "sim_reg_timeout", tmp))) {
			p_chan->cfg.simswitch_cfg.sim_reg_timeout = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d sim_reg_timeout fail.\n", i+1);
		}	

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "total_sms_count", tmp))) {
			p_chan->cfg.simswitch_cfg.total_sms_count = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d total_sms_count fail.\n", i+1);
		}	

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "total_callout_count", tmp))) {
			p_chan->cfg.simswitch_cfg.total_callout_count = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d total_callout_count fail.\n", i+1);
		}	

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "total_callout_time", tmp))) {
			p_chan->cfg.simswitch_cfg.total_callout_time = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d total_callout_time fail.\n", i+1);
		}	

		if(!(res = get_option_value(SETTINGS_FILE, context_name, "total_using_time", tmp))) {
			p_chan->cfg.simswitch_cfg.total_using_time = atoi(tmp);
		} else {
			log_printf(LOG_DEBUG, "get chan_%d total_using_time fail.\n", i+1);
		}	
		
		p_chan++;
		memset(tmp, 0, NAME_SIZE);
		memset(context_name, 0, NAME_SIZE);
	}

	return 0;
}


static int read_call_limit_status_cfg(calllimit_t *call_limit)
{
	int res = -1;
	int i = 0;
	int j = 0;
	int sim_num = 1;
	char tmp[NAME_SIZE] = {0};
	char context_name[NAME_SIZE] = {0};
	calllimit_chan_t *p_chan = call_limit->chans;
	sys_info_cfg_t *sys_info = &call_limit->sys_info;

	if(CHAN_4SIM == sys_info->sys_type)
		sim_num = SIM_NUM;

	for(i=0; i<(int)sys_info->total_chans; i++) {
		for(j=0; j<sim_num; j++) {
			if(CHAN_4SIM == sys_info->sys_type)
				sprintf(context_name, "%d-%d", i+1, j+1);
			else
				sprintf(context_name, "%d", i+1);

			if(!(res = get_option_value(STATUS_FILE, context_name, "call_time_count", tmp))) {//已用时长
				p_chan->sim[j].calltime_info.call_time_count = atoi(tmp);
				if(p_chan->cfg.calltime_cfg.call_time_total > 0 && p_chan->cfg.calltime_cfg.call_time_total > p_chan->sim[j].calltime_info.call_time_count){
					p_chan->sim[j].calltime_info.call_time_remain = p_chan->cfg.calltime_cfg.call_time_total - p_chan->sim[j].calltime_info.call_time_count;
				}else
					p_chan->sim[j].calltime_info.call_time_remain = 0;
			} else {
				p_chan->sim[j].calltime_info.call_time_remain = p_chan->cfg.calltime_cfg.call_time_total ;
				log_printf(LOG_DEBUG, "[%d][%d]get call_time_count fail.", i+1, j+1);
			}

			if(!(res = get_option_value(STATUS_FILE, context_name, "call_time_clean_date", tmp))) {//next auto reset date
				time_t now;
				time_t t;
				time(&now);
				t = mktime(&p_chan->sim[j].calltime_info.call_time_clean_date);
				if(now >= (t+3600 * 24))//date if less than 1 days, will use
					strptime(tmp, "%Y-%m-%d %H:%M:%S",&p_chan->sim[j].calltime_info.call_time_clean_date);
			} else {
		//		log_printf(LOG_INFO, "[%d][%d]get call_time_clean_date fail.", i+1, j+1);
			}

			if(!(res = get_option_value(STATUS_FILE, context_name, "day_call_limit_flag", tmp))) {
				p_chan->sim[j].calllimit_info.day_call_limit_flag = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get day_call_limit_flag fail.\n", i+1, j+1);
			}	

			if(!(res = get_option_value(STATUS_FILE, context_name, "hour_call_limit_flag", tmp))) {
				p_chan->sim[j].calllimit_info.hour_call_limit_flag = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get hour_call_limit_flag fail.\n", i+1, j+1);
			}	

			if(!(res = get_option_value(STATUS_FILE, context_name, "day_answer_limit_flag", tmp))) {
				p_chan->sim[j].calllimit_info.day_answer_limit_flag = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get day_answer_limit_flag fail.\n", i+1, j+1);
			}	
			
			if(!(res = get_option_value(STATUS_FILE, context_name, "day_total_calls", tmp))) {
				p_chan->sim[j].calllimit_info.day_cur_calls = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get day_total_calls fail.\n", i+1, j+1);
			}

			if(!(res = get_option_value(STATUS_FILE, context_name, "hour_total_calls", tmp))) {
				p_chan->sim[j].calllimit_info.hour_cur_calls = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get hour_total_calls fail.\n", i+1, j+1);
			}

			if(!(res = get_option_value(STATUS_FILE, context_name, "day_total_answers", tmp))) {
				p_chan->sim[j].calllimit_info.day_cur_answers = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get day_total_answers fail.\n", i+1, j+1);
			}

	        if(!(res = get_option_value(STATUS_FILE, context_name, "call_failed_count", tmp))) {
				p_chan->sim[j].calllock_info.call_failed_count = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get call_failed_count fail.\n", i+1, j+1);
			}

	        if(!(res = get_option_value(STATUS_FILE, context_name, "call_fail_mark_status", tmp))) {
				p_chan->sim[j].calllock_info.call_fail_mark_status = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get call_fail_mark_status fail.\n", i+1, j+1);
			}

	        if(!(res = get_option_value(STATUS_FILE, context_name, "call_fail_lock_status", tmp))) {
				p_chan->sim[j].calllock_info.call_fail_lock_status = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get call_fail_lock_status fail.\n", i+1, j+1);
			}

			if(!(res = get_option_value(STATUS_FILE, context_name, "day_sms_limit_flag", tmp))) {
				p_chan->sim[j].smslimit_info.day_sms_limit_flag = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get day_sms_limit_flag fail.\n", i+1, j+1);
			}
			if(!(res = get_option_value(STATUS_FILE, context_name, "mon_sms_limit_flag", tmp))) {
				p_chan->sim[j].smslimit_info.mon_sms_limit_flag= atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get mon_sms_limit_flag fail.\n", i+1, j+1);
			}
			if(!(res = get_option_value(STATUS_FILE, context_name, "day_total_sms", tmp))) {
				p_chan->sim[j].smslimit_info.day_cur_sms = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get day_total_sms fail.\n", i+1, j+1);
			}
			if(!(res = get_option_value(STATUS_FILE, context_name, "mon_total_sms", tmp))) {
				p_chan->sim[j].smslimit_info.mon_cur_sms = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get mon_total_sms fail.\n", i+1, j+1);
			}

			if(!(res = get_option_value(STATUS_FILE, context_name, "sms_limit_warning_flag", tmp))) {
				p_chan->sim[j].smslimit_info.sms_limit_warning_flag = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get sms_limit_warning_flag fail.\n", i+1, j+1);
			}

			if(!(res = get_option_value(STATUS_FILE, context_name, "call_time_warning_flag", tmp))) {
				p_chan->sim[j].calltime_info.call_time_warning_flag = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get call_time_warning_flag fail.\n", i+1, j+1);
			}
			
			if(!(res = get_option_value(STATUS_FILE, context_name, "sim_idx", tmp))) {
				p_chan->sim_idx = atoi(tmp);
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get sim_idx fail.\n", i+1, j+1);
			}
		
		}
		p_chan++;
		memset(tmp, 0, NAME_SIZE);
		memset(context_name, 0, NAME_SIZE);
	}

	return 0;
}


static int read_sim_info_cfg(calllimit_t *call_limit)
{
	int res = -1;
	int i = 0;
	int j = 0;
	char tmp[NAME_SIZE] = {0};
	char context_name[NAME_SIZE] = {0};
	calllimit_chan_t *p_chan = call_limit->chans;
	sys_info_cfg_t *sys_info = &call_limit->sys_info;

	if(CHAN_4SIM != sys_info->sys_type)
		return 0;

	for(i=0; i<(int)sys_info->total_chans; i++) {
		for(j=0; j<SIM_NUM; j++) {
			sprintf(context_name, "%d-%d", i+1, j+1);
			if(!(res = get_option_value(SIM_INFO_FILE, context_name, "pincode", tmp))) {
				memset(p_chan->sim[j].sim_ping_code, 0, sizeof(p_chan->sim[j].sim_ping_code));
				memcpy(p_chan->sim[j].sim_ping_code,tmp,strlen(tmp));
			} else {
				log_printf(LOG_DEBUG, "[%d][%d]get sim_ping_code fail.\n", i+1, j+1);
			}
		
		}
		p_chan++;
		memset(tmp, 0, NAME_SIZE);
		memset(context_name, 0, NAME_SIZE);
	}

	return 0;
}



int read_call_limit_cfg(calllimit_t *call_limit)
{
	if(!call_limit) {
		log_printf(LOG_ERROR, "call_limit is null.\n");
		return -1;		
	}

	if(is_file_exist(SETTINGS_FILE)) {
		create_call_limit_settings_cfg(call_limit);	
	} else {
            read_call_limit_settings_cfg(call_limit);
       }

	if(is_file_exist(STATUS_FILE)) {
		create_call_limit_status_cfg(call_limit);		
	} else {
		read_call_limit_status_cfg(call_limit);
	}

	if(is_file_exist(SIM_INFO_FILE)) {
		create_sim_info_cfg(call_limit);
	} else {
		read_sim_info_cfg(call_limit);
	}

    if(call_limit->sys_info.IsRefleshFile == ENABLE)
    {
        start_reflesh_callstatus_conf();
    }
	
	return 0;
}


int reload_call_limit_cfg(calllimit_t *call_limit)
{
	int res = -1;
	res = read_call_limit_settings_cfg(call_limit);	
	res|= read_sim_info_cfg(call_limit);
	read_log_file();
	return res;
}


static int delete_one_from_status_cfg(unsigned char sys_type, int channel_id)
{
	int s_flag = 0;
	int lineid = 0;   
	int maxline = 0;    
	char buff[LINE_SIZE] = {0};
	char head_content[10] = {0};
	char tail_content[10] = {0};
	char tmp[MAX_LINE][LINE_SIZE] = {{0}};	 
	FILE *fp;
	int lock;

	lock = lock_file(STATUS_FILE);

	if( NULL == (fp = fopen(STATUS_FILE, "r+")) ) {
		log_printf(LOG_ERROR, "Can't open %s\n", STATUS_FILE);
		unlock_file(lock);
		return -1;
	}	

	if(CHAN_4SIM == sys_type) {
		sprintf(head_content, "[%d-1]\n", channel_id);
		sprintf(tail_content, "[%d-1]\n", channel_id+1);
	} else {
		sprintf(head_content, "[%d]\n", channel_id);
		sprintf(tail_content, "[%d]\n", channel_id+1);
	}

	while( fgets(buff, LINE_SIZE, fp) ) {
		lineid++;
		if(lineid >= MAX_LINE) {
			log_printf(LOG_ERROR, "lineid is out of range.\n");
			fclose(fp);
			unlock_file(lock);
			return -1;
		}
		if(!strcmp(buff, head_content)) {
			s_flag = 1;
		} else if(!strcmp(buff, tail_content)) {
			s_flag = 2;
		}
		if(1 == s_flag) {
			strcpy(tmp[lineid--], buff);		
		} else {
			strcpy(tmp[lineid], buff); 
		}
		memset(buff, 0, sizeof(buff));
	}

    maxline = lineid;   
    rewind(fp);   
    fclose(fp);     
    remove(STATUS_FILE);   //   删除原文件  

    if((fp = fopen(STATUS_FILE, "w")) == NULL) {   
		log_printf(LOG_ERROR, "Can't open %s\n", STATUS_FILE);
		unlock_file(lock);
        return -1;   
    }
    for(lineid = 1; lineid <= maxline; lineid++) {
    	fputs(tmp[lineid], fp);  
    }
    fclose(fp);
	unlock_file(lock);
    return 0;
}


static int parse_calllimit_status(calllimit_chan_t *p_chan, unsigned char sys_type, char (*output)[LINE_SIZE], int index)
{
	int offset = index;
	int sim_num = 1;
	int i = 0;
	
	if(!output || !p_chan) {
		return 0;
	}

	if(CHAN_4SIM == sys_type)
		sim_num = SIM_NUM;
	else
		sim_num = 1;

	for(i=0; i<sim_num; i++) {
		if(CHAN_4SIM == sys_type)	
			sprintf(output[offset++], "[%d-%d]\n", p_chan->id, i+1);
		else
			sprintf(output[offset++], "[%d]\n", p_chan->id);
		
		calllock_info_t *pcalllock_info = &p_chan->sim[i].calllock_info;   
		calllimit_info_t *pcalllimit_info = &p_chan->sim[i].calllimit_info;   
		calltime_info_t *pcalltime_info = &p_chan->sim[i].calltime_info;   
		smslimit_info_t *psmslimit_info = &p_chan->sim[i].smslimit_info;   	
				
		sprintf(output[offset++], "call_time_count=%d\n", pcalltime_info->call_time_count);
		sprintf(output[offset++], "call_time_remain=%d\n", pcalltime_info->call_time_remain);
		sprintf(output[offset++], "call_time_limit_flag=%d\n", pcalltime_info->call_time_limit_flag);
		if(p_chan->cfg.calltime_cfg.call_time_clean_type !=	CALL_TIME_CLEAN_TYPE_UNKOWN){
			char tmp_buf[64] = {0};
			strftime(tmp_buf, sizeof(tmp_buf), "%Y-%m-%d %H:%M:%S", &pcalltime_info->call_time_clean_date);
			sprintf(output[offset++],"call_time_clean_date=%s\n", tmp_buf);
		}
		sprintf(output[offset++], "day_call_limit_flag=%d\n", pcalllimit_info->day_call_limit_flag);
		sprintf(output[offset++], "hour_call_limit_flag=%d\n", pcalllimit_info->hour_call_limit_flag);
		sprintf(output[offset++], "day_answer_limit_flag=%d\n", pcalllimit_info->day_answer_limit_flag);
		sprintf(output[offset++], "day_total_calls=%d\n", pcalllimit_info->day_cur_calls);
		sprintf(output[offset++], "hour_total_calls=%d\n", pcalllimit_info->hour_cur_calls);
		sprintf(output[offset++], "day_total_answers=%d\n", pcalllimit_info->day_cur_answers);
		sprintf(output[offset++], "call_failed_count=%d\n", pcalllock_info->call_failed_count);
		sprintf(output[offset++], "call_fail_mark_status=%d\n",pcalllock_info->call_fail_mark_status);
		sprintf(output[offset++], "call_fail_lock_status=%d\n",pcalllock_info->call_fail_lock_status);
		sprintf(output[offset++], "day_sms_limit_flag=%d\n",psmslimit_info->day_sms_limit_flag);
		sprintf(output[offset++], "mon_sms_limit_flag=%d\n",psmslimit_info->mon_sms_limit_flag);
	//	sprintf(output[offset++], "sms_limit_success_flag=%d\n",p_call_limit->smslimit_success_flag);
		sprintf(output[offset++], "day_total_sms=%d\n",psmslimit_info->day_cur_sms);
		sprintf(output[offset++], "mon_total_sms=%d\n",psmslimit_info->mon_cur_sms);
		sprintf(output[offset++], "sms_limit_warning_flag=%d\n", psmslimit_info->sms_limit_warning_flag);
		sprintf(output[offset++], "call_time_warning_flag=%d\n", pcalltime_info->call_time_warning_flag);
		sprintf(output[offset++], "sim_idx=%d\n", p_chan->sim_idx);
//		sprintf(output[offset++], "sim_using_time=%d\n", p_chan->sim[i].sim_using_time);
//		sprintf(output[offset++], "sim_callout_time=%d\n", p_chan->sim[i].sim_callout_time);
//		sprintf(output[offset++], "sim_callout_count=%d\n", p_chan->sim[i].sim_callout_count);
//		sprintf(output[offset++], "sim_sms_count=%d\n", p_chan->sim[i].sim_sms_count);
	}

	return offset;
}

static int add_one_from_status_cfg(calllimit_t *call_limit, int channel_id)
{
	FILE *fp;
//	int i = 0;	
//	int idx = 0;	
	int s_flag = 0;
	int m_flag = 0;
	int lineid = 0;   
	int maxline = 0;  	
	int lock;
	char content[10] = {0};		
	char buff[LINE_SIZE] = {0};
	char tmp[MAX_LINE][LINE_SIZE]={{0}};	 
	calllimit_chan_t *p_chan = &call_limit->chans[channel_id-1];	

	lock = lock_file(STATUS_FILE);

	if( NULL == (fp = fopen(STATUS_FILE, "r+")) ) {
		log_printf(LOG_ERROR, "Can't open %s\n", STATUS_FILE);
		unlock_file(lock);
		return -1;
	}	

	if(CHAN_4SIM == call_limit->sys_info.sys_type)
		sprintf(content, "[%d-1]\n", channel_id+1);
	else
		sprintf(content, "[%d]\n", channel_id+1);

	while( fgets(buff, LINE_SIZE, fp) ) {
		lineid++;
		if(lineid >= MAX_LINE) {
			log_printf(LOG_ERROR, "lineid is out of range.\n");
			fclose(fp);
			unlock_file(lock);
			return -1;
		}		
		if(!strcmp(buff, content)) {
			s_flag = 1;
			m_flag = 1;
			lineid = parse_calllimit_status(p_chan, call_limit->sys_info.sys_type, tmp, lineid);
			strcpy(tmp[lineid++], buff);
		} 
		if(s_flag) {
			s_flag = 0;
		} else {
			strcpy(tmp[lineid], buff);
		}
		memset(buff, 0, sizeof(buff));
	}

	if(!m_flag) {
		lineid++;
		lineid = parse_calllimit_status(p_chan, call_limit->sys_info.sys_type, tmp, lineid);
	}

    maxline = lineid;   
    rewind(fp);   
    fclose(fp);    
    remove(STATUS_FILE);   //   删除原文件  

    if((fp = fopen(STATUS_FILE, "w")) == NULL) {   
        log_printf(LOG_ERROR, "Can't open %s\n", STATUS_FILE);
		unlock_file(lock);
        return -1;   
    }
    for(lineid = 1; lineid <= maxline; lineid++) {
    	fputs(tmp[lineid], fp);  
    }
    fclose(fp);
	unlock_file(lock);

	return 0;	
}


int refresh_call_limit_status_cfg(calllimit_t *call_limit, int channel_id)
{
	int res = -1;

//	if(ENABLE != call_limit->IsRefleshFile) {
//		return 0;
//	}
	pthread_mutex_lock(&call_limit->lock);
	delete_one_from_status_cfg(call_limit->sys_info.sys_type, channel_id);
	res = add_one_from_status_cfg(call_limit, channel_id);
	pthread_mutex_unlock(&call_limit->lock);
	return res;
}





