#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include "openvox_version_record.h"

#ifdef POWER_RESET
extern "C"{
	#include "bsp_api.h"
}
#endif

module_info* module_info::module_info_t  = NULL; 

int upgrade_channel = 0;

void set_upgrade_channel(int channel){
	upgrade_channel = channel;
}
void get_date(char * date){                                                                                                                                                              
	struct tm *ptm; 
	long   ts; 
	    
	ts = time(NULL); 
	ptm = localtime(&ts); 
	    
	sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
	            ptm-> tm_year+1900,
	            ptm-> tm_mon+1,
	            ptm-> tm_mday,
	            ptm-> tm_hour,
	            ptm-> tm_min,
	            ptm-> tm_sec);
}

/*
 * 输入参数：
 *    channel:通道号
 *    flag   :升级前升级后标记位
 * 输出参数:
 *    version:获取到的版本号
 * */
void get_mod_version(int channel, int flag, char *version){
	char tmp[128] = {0};    
	int try_count = 10;
	char read_pipe_name[256] = {0};
	char write_pipe_name[256] = {0};
	sprintf(write_pipe_name, "/tmp/module_pipe/at-%d-r.pipe", channel);
	sprintf(read_pipe_name, "/tmp/module_pipe/at-%d-w.pipe", channel);
	char buf[1024]  = {0};
	int read_fd = open(read_pipe_name, O_RDONLY|O_NONBLOCK);
	if(read_fd < 0){
		printf("open pipe error!\n");
		return ;
	}
	
	read(read_fd, buf, 1024);

	
	memset(buf, 0, 1024);
again:
	int i = 10; 
	int write_fd = open(write_pipe_name, O_WRONLY);
	if(write_fd < 0){
		printf("open pipe error!\n");
		return;
	}
	read(read_fd, buf, 1024);
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "AT+CGMR\r\n");

	if(write(write_fd, tmp, strlen(tmp)) < 0){
	    printf("write version failed!\n");
	    return;
	}

	memset(tmp, 0, sizeof(tmp));

	while(i > 0){
	    if(read(read_fd, tmp, sizeof(tmp)) > 0){
	        break;
	    }
	    usleep(30000);
	    --i;
	}  
	--try_count;
	if(i == 0 && try_count > 0){
		//如果第一次读取失败，意味着模块已经升级完成，但没有完全起来。睡眠一段时间，等待模块起来
		if(flag == 1)
			sleep(12);
		goto again;
	}
	if(i > 0){
	    //只保留版本信息
	    int len = strlen(tmp);
	    int j = 0, k = 0;
	    while(k < len){
	        if( tmp[k] == 0x0D)
	            k++;
	        version[j] = tmp[k]; 
	        ++k;
	        ++j;
	    }
	    char *pos_s = strrchr(version, ':');
	    char *pos_end = strstr(version, "OK");
	    memset(tmp, 0, sizeof(tmp));
	   if(pos_s != NULL && pos_end != NULL){
		    strncpy(tmp, pos_s + 1, pos_end - pos_s - 2);
		    memset(version, 0, k);
		    strcpy(version, tmp);
	   }
	}else{
	    strncpy(version, "Unkown", 6);
	}  
	return ;
}


void get_mod_brd_version(int channel,  char *version){
	if(version == NULL)
		return;
	char read_pipe_name[1024] = {0};
	char write_pipe_name[1024] = {0};
	//  int cnt;
	sprintf(write_pipe_name, "/tmp/module_pipe/mcu-%d-r.pipe", channel);
	sprintf(read_pipe_name, "/tmp/module_pipe/mcu-%d-w.pipe", channel);
	char buf[1024] = {0};
	//打开读取管道
	int read_fd = open(read_pipe_name, O_RDONLY|O_NONBLOCK);
	if(read_fd < 0){
	    printf("open pipe error!\n");
	    return ;
	}  
	   
	//读空管道里面已经存在的内容
	read(read_fd, buf, 1024);
	   
	memset(buf, 0, 1024);
	//打开写管道
	int write_fd = open(write_pipe_name, O_WRONLY);
	if(write_fd < 0){
	    printf("open pipe error!\n");
	    return ;
	}  
	   
	sprintf(buf, "ver\n");
	   
	if(write(write_fd, buf, strlen(buf)) < 0){
	    printf("write error\n");
	    return ;
	}  
	close(write_fd);
	   
	memset(buf, 0, 1024);
	   
	//尝试读取管道里面的内容10次，读到立即返回，每次间隔30ms
	int i = 10; 
	while(i > 0){
	    if(read(read_fd, buf, sizeof(buf)) > 0){
	        break;
	    }
	    usleep(30000);
	    --i;
	}
	strcpy(version,buf);
	close(read_fd);
	return;
}

const char *get_state_str(int state){
	switch (state){
		case 0:
		    return "success";
		case SYNC_ERR:
	            return "sync error";
	        case BEGIN_ERR:
	            return "begin error";
	        case LOAD_ERR:
	            return "load file error";
	        case DOWNLOAD_ERR:
	            return "download file error";
	        case END_ERR:
	            return "end error";
	        case RUN_ERR:
	            return "run firmerror";
	        case INIT_ERR:
	            return "init error";
	        case RESET_ERR:
	            return "reset module error";
	        case PIPE_ERR:
	            return "open pipe error";
	        case COM_ERR:
	            return "no find port error";
		 case MODULE_ERR:
		 	return "module is unkown state, error";
		 case FLASH_ERR:
		 	return "unkonw flash type";
	}
	return "";
}
module_info::module_info(){
	strcpy(m_oldVersion, "Unkown");
	strcpy(m_newVersion, "Unkown");
	m_state = 0;
}
module_info *module_info::getInstance(){
	if(module_info_t == NULL){
		module_info_t = new module_info();
		module_info_t->m_channel = upgrade_channel;
	}
	return module_info_t;
}
void module_info::set_old_version(char *oldVersion){
	if(oldVersion)
		strncpy(m_oldVersion, oldVersion, 64);
	else
		get_mod_version(m_channel, 0, m_oldVersion);
}

void module_info::set_new_version(char *newVersion){
	if(newVersion)
		strncpy(m_newVersion, newVersion, 64);
	else
		get_mod_version(m_channel, 1, m_newVersion);
}

void module_info::set_mod_version(char *modVersion){
	if(modVersion)
		strncpy(m_modVersion, modVersion, 64);
	else
		get_mod_brd_version(m_channel, m_modVersion);
}

void module_info::set_start_time(char *startTime){
	if(startTime)
		strncpy(m_startTime, startTime, 64);
	else
		get_date(m_startTime);
}

void module_info::set_end_time(char *endTime){
	if(endTime)
		strncpy(m_endTime, endTime, 64);
	else
		get_date(m_endTime);
}

void module_info::set_upgrade_state(int state){
	m_state = state;
}

int module_info::get_channel(){
	return m_channel;
}

int module_info::record_info_to_file(){
	char filename[1024];
	sprintf(filename, "/data/log/module_upgrade.log");
	FILE *version_handle = fopen(filename, "a+");
	if(version_handle == NULL){
		printf("open %s failed!\n", filename);
		return -1;
	}
	fprintf(version_handle,
		  "channel:%d\n\n"
		   "oldVersion:%s\n"
		   "newVersion:%s\n"
		   "moduleVersion:\n%s\n"
		   "startTime:%s\n"
		   "endTime:%s\n"
		   "state:%s\n"
		   "-----------------------------------\n",
		   m_channel,
		   m_oldVersion,
		   m_newVersion,
		   m_modVersion,
		   m_startTime,
		   m_endTime,
		   get_state_str(m_state));
	fclose(version_handle);
	return 0;
}
#ifdef POWER_RESET
void module_info::module_reset(void){
	unsigned char state = 2;
	int try_count = 30; 
	bsp_api_init(NULL, 0);
	int res;
	if(module_turn_on_state_get(m_channel, &state) < 0){
		printf("get module state failed!\n");
		return;
	}
	printf("get module_state ok, state is %s\n", state == 0? "OFF":"ON");
	if(state == 0){
		res = module_turn_off(m_channel);
		if(res < 0){
			printf("turn off module failed, m_channel = %d\n", m_channel);
			return;
		}
		//尝试10次，确定模块已经正常关机
		while(try_count > 0){
			if(module_turn_on_state_get(m_channel, &state) < 0){
				printf("get module state failed!\n");
				return;
			}else{
				printf("state = %d", state);
				if(state == 0){
					break;
				}
			}
			--try_count;
			sleep(1);
		}
		if(try_count <= 0){
			printf("turn off module failed!\n");
			return;
		}
	}
	res = module_turn_on(m_channel);
	if(res < 0){
		printf("turn on module failed!\n");
		return;
	}

	bsp_api_deinit();
}
#endif
