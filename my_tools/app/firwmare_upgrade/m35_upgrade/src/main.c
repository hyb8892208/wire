#include "chn_upgrade.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <alloca.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/stat.h>   
#include <sys/file.h>
#include <unistd.h>
#include <time.h>
#include "debug.h"
#include "bsp_api.h"
#include "rri_api.h"

#define MODULE_AT_PIPE_PREF          "/tmp/module_pipe/at-" /* 升级通道pipe文件前缀 */
#define FILE_PIPE_WRITE          "-r.pipe"              /* 写pipe文件名后缀(与server相反) */
#define FILE_PIPE_READ           "-w.pipe"              /* 读pipe文件名后缀(与server相反) */

#define M35             "Quectel_M35"
#define M26             "Quectel_M26"
#define SIMCOM_SIM6320C "SIMCOM_SIM6320C"
#define EC20F           "EC20CE"

#define PATH_LENGTH_MAX 254
#define NAME_LEN                 (32)
#define HW_PORT_MAX              (64)    /* 最大物理端口号个数(实际上目前最多32个) */


typedef enum ERROR_CODE{
    SUCCESS=0,
    SYNC_ERR=-1,
    BEGIN_ERR=-2,
    DOWNLOAD_ERR=-4,
    END_ERR=-5,
    RUN_ERR=-6,
    INIT_ERR=-7,
    RESET_ERR=-8,
    PIPE_ERR=-9
}ERROR_CODE_T;

typedef struct MODULE_INFO{
    char oldVersion[64];
    char newVersion[64];
    char moduleVersion[128];
    char startDate[64];
    char endDate[64];
    ERROR_CODE_T state;
    int channel;
}MODULE_INFO_T;

FILE *log_handle;
MODULE_INFO_T module_info;

char *get_error(ERROR_CODE_T error_code){
    switch(error_code){
        case SUCCESS:
            return "success";
        case SYNC_ERR:
            return "sync error";
        case BEGIN_ERR:
            return "begin error";
        case DOWNLOAD_ERR:
            return "download error";
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
    }
    return NULL;
}

int get_date(char *date){
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
    return 0;
}
int record_version_info(void){
#define VERSION_FILE "/data/log/module_upgrade.log"
   FILE* handle = fopen(VERSION_FILE, "a");
   if(handle == NULL){
       printf("open %s error\n", VERSION_FILE);
       return -1;
   }
   fprintf(handle, "channel:%d\n\n"
            "oldVersion:%s\n"
            "newVerson:%s\n"
            "moduleInfo:\n%s\n"
            "startDate:%s\n\n"
            "endDate:%s\n\n"
            "state:%s\n"
            "------------------------------------\n",
            module_info.channel,
            module_info.oldVersion,
            module_info.newVersion,
            module_info.moduleVersion,
            module_info.startDate,
            module_info.endDate,
            get_error(module_info.state));
   fclose(handle); 
   return 0;
}

int pipe_open(char *file_name, int mode)
{
    int res;

    if ( access(file_name, F_OK) == -1 )
    {
        res = mkfifo(file_name, 0777);
        if ( res != 0 )
        {
            printf("create fifo %s fail\n", file_name);
            return -1;
        }
    }

    res = open(file_name, mode|O_NONBLOCK);
    if ( res < 0 )
    {  
        printf("open fifo %s fail, fd = %d\n", file_name, res);
        return -1;
    }

    return res;
}

int pipe_new(int fds[], char *chan_no)
{
    char file_name[48];

    /* 打开读pipe文件 */
    sprintf(file_name, "%s%s%s", MODULE_AT_PIPE_PREF, chan_no, FILE_PIPE_READ);
    if ( (fds[PIFD_AT_R] = pipe_open(file_name, O_RDONLY)) < 0 )
    {
        printf("open read pipe file fail! file_name = %s, fd = %d\n", file_name, fds[PIFD_AT_R]);
        return -1;
    }

    /* 先用读打开，放在fd pseudo，这样就能用写打开 */
    sprintf(file_name, "%s%s%s", MODULE_AT_PIPE_PREF, chan_no, FILE_PIPE_WRITE);
    if ( (fds[PIFD_AT_P] = pipe_open(file_name, O_RDONLY)) < 0 )
    {
        close(fds[PIFD_AT_R]);
        printf("open pseudo read pipe file fail! file_name = %s, fd = %d\n", file_name, fds[PIFD_AT_P]);
        return -1;
    }

    /* 打开写pipe文件 */
    if ( (fds[PIFD_AT_W] = pipe_open(file_name, O_WRONLY)) < 0 )
    {
        close(fds[PIFD_AT_P]);
        close(fds[PIFD_AT_R]);
        printf("open write pipe file fail! file_name = %s, fd = %d\n", file_name, fds[PIFD_AT_W]);
        return -1;
    }
    
    /*增加文件锁*/
    if(flock(fds[PIFD_AT_R], LOCK_EX|LOCK_NB) < 0){
        printf("lock read pipe failed!\n");
        return -1;
    }
    //给写管道加锁，避免其他进程写入
    if(flock(fds[PIFD_AT_W], LOCK_EX|LOCK_NB) < 0){
        printf("lock write pipe failed!\n");
        return -1;
    }
    return 0;
}

#if 0
int query_process_exist(const char *process){
	if(!process)
		return -1;
	char cmd[128] = {0};
	char result[32] = {0};
	sprintf(cmd, "pidof %s", process);
	FILE *handle = popen(cmd, "r");
	if(handle == NULL){
		printf("open pipe error\n");
		return -1;
	}
	fread(result, 1, 32, handle);
	fclose(handle);
	printf("result =%s\n", result);
	if(atoi(result)){
		printf("%s is not exist!", process);
		return 1;
	}
	return 0;
}

int close_astersik(){
	int count = 10;
	int res = query_process_exist("asterisk");
	while( res != 0 && count > 0){
		system("/etc/init.d/asterisk stop");
		sleep(1);
		res = query_process_exist("asterisk");
		--count;
	}
	if(count > 0 && res == 0){
		printf("close asterisk success!\n");
		return 0;
	}else{
		printf("close asterisk failed!\n");
		return -1;
	}
}
#endif

int upgrade_init(char *channel){
	
	unsigned char status[128] = {0};
	int hwport = atoi(channel);
    char log_file[128] = {0};
    int result;
    memset(&module_info, 0, sizeof(module_info));
#ifdef ASTERISK
	if(close_astersik() < 0){
		return -1;
	}
#endif
	if( bsp_api_init(NULL, 0) < 0){
		printf("bsp_api_init failed!\n");
		return -1;
	}
	if( rri_api_init(NULL, 0) < 0){
		printf("rri_api_init failed!\n");
		return -1;
	}
    //设置bsp_server对应通道为升级模式，避免升级过程被打断
    if(chan_upgrade_status_set(hwport, 1, "upgrade", &result) < 0){
        printf("set channel to upgrade module failed!\n");
        return -1;
    }
    //先拉高power_key管脚电平
	if(module_powerkey_hign_low(hwport, 1, "upgrade") < 0){
		printf("set powerkey to hign level failed!\n");
        return -1;
    }
	//获取上电状态
	int res = module_power_state_get(hwport, status);
	if(res < 0){
		printf("get module turn on status failed!");
		return -1;
	}
    //如果电源没有正常上电，则先对模块进行上电
	if(status[0] == 0){
		if(module_power_on(hwport) < 0){
			printf("turn on the channel failed!\n");
			return -1;
		}
	}
    //保存升级开始时间
    get_date(module_info.startDate);
    //保存升级通道
    module_info.channel = hwport;
#if 0	
    //设置rri_server对应通道为升级模式，上报更多信息
	res = SetChannelUpgrade(hwport, 1);
	if(res < 0){
		printf("set upgrade flag failed!\n");
		return -1;
	}
#endif	
    sprintf(log_file, "/www/chn_%s", channel);
    log_handle = fopen(log_file, "w+");
    if(log_handle == NULL){
        printf("open log failed!\n");
        return -1;
    }

	return 0;
}

int upgrade_deinit(char *channel, int *fds){
	// printf("channel = %s\n", channel);
	// if(module_power_on(atoi(channel)) < 0)
		// return -1;
	int hwport = atoi(channel);
    int result;
    //升级完成，将rri_server对应通道设置为非升级模式	
	SetChannelUpgrade(hwport, 0);
	

    //设置bsp_server对应通道为非升级模式
    if(chan_upgrade_status_set(hwport, 0, "upgrade", &result) < 0){
        printf("set channel to upgrade module failed!\n");
        return -1;
    }



    //升级完成，先关闭写管道的文件锁
    flock(fds[PIFD_AT_W], LOCK_UN|LOCK_NB);

	close(fds[PIFD_AT_R]);
    close(fds[PIFD_AT_W]);
    close(fds[PIFD_AT_P]);
	
	rri_api_deinit();
  //  record_version_info();
	return 0;
}



int get_module_brd_version(char *channel, char *result){
	if(result == NULL)
		return -1;
	char read_pipe_name[1024] = {0};
	char write_pipe_name[1024] = {0};
//	int cnt;
	sprintf(write_pipe_name, "/tmp/module_pipe/mcu-%s-r.pipe", channel);
	sprintf(read_pipe_name, "/tmp/module_pipe/mcu-%s-w.pipe", channel);
	char buf[1024] = {0};
	//打开读取管道
	int read_fd = open(read_pipe_name, O_RDONLY|O_NONBLOCK);
	if(read_fd < 0){
		printf("open pipe error!\n");
		return -1;
	}
	
	//读空管道里面已经存在的内容
	read(read_fd, buf, 1024);

	memset(buf, 0, 1024);
	//打开写管道
	int write_fd = open(write_pipe_name, O_WRONLY);
	if(write_fd < 0){
		printf("open pipe error!\n");
		return -1;
	}

	sprintf(buf, "ver\n");

	if(write(write_fd, buf, strlen(buf)) < 0){
		printf("write error\n");
		return -1;
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
	strcpy(result,buf);
	close(read_fd);
	return 0;
}

int get_module_version( int fds[], char *buf){
    char tmp[128] = {0};    
	int i = 10; 
   
    read(fds[PIFD_AT_R], buf, 128);
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "ATI\r\n");

    if(write(fds[PIFD_AT_W], tmp, strlen(tmp)) < 0){
        printf("write version failed!\n");
        return -1;
    }

    memset(tmp, 0, sizeof(tmp));
    
	while(i > 0){
		if(read(fds[PIFD_AT_R], tmp, sizeof(tmp)) > 0){
			break;
		}
		usleep(30000);
		--i;
	}
    
    if(i > 0){
        //只保留版本信息
        int len = strlen(tmp);
        int j = 0, k = 0;
        while(k < len){
            if( tmp[k] == 0x0D)
                k++;
            buf[j] = tmp[k]; 
            ++k;
            ++j;
        }
        char *pos_s = strrchr(buf, ':');
        char *pos_end = strstr(buf, "OK");
        memset(tmp, 0, sizeof(tmp));
        strncpy(tmp, pos_s + 1, pos_end - pos_s - 2);
        memset(buf, 0, k);
        strcpy(buf, tmp);
    }else{
        strncpy(buf, "Unkown", 6);
    }
    return 0;
}

int module_reset(int channel){
    if(module_power_on(channel) < 0){
        printf("power on module failed!\n");
        return RESET_ERR;
    }
    sleep(1);
    if(module_power_on(channel) < 0){
        printf("power on module failed!\n");
        return RESET_ERR;
    }

   // fseek(log_handle, 0 - last_size, SEEK_CUR);
    fprintf(log_handle, "\nupgrade success");
    return 0;
}

int main(int argc, char **argv){
    if(argc < 3) 
    {
        printf("Usage:\n"
               "\t%s <chan_no> <firmware_file> \n", argv[0]);
        return 0;
    }
    if(upgrade_init(argv[1]) < 0){
		printf("upgrade init failed!\n");
        module_info.state = INIT_ERR;
		return -1;
	}

    char fw_path[PATH_LENGTH_MAX];
    int ret;
    int ota_fds[PIFD_NUM];
	char version[128] = {0};
	int hwport = atoi(argv[1]);
	//打开at管道
    ret = pipe_new(ota_fds, argv[1]);
    if(ret < 0)
    {
        DebugPrintf("chan %s pipe new failed ret = %d", argv[1], ret);
        module_info.state = PIPE_ERR;
        return ret;
    }

    memset(fw_path, 0, sizeof(fw_path));
    strncpy(fw_path, argv[2], strlen(argv[2]));
	//获取模块板版本类型
	get_module_brd_version(argv[1], module_info.moduleVersion);
    //获取模块版本信息
    get_module_version(ota_fds, module_info.oldVersion);	
	if(strstr(module_info.moduleVersion, M35)){
		if(!strstr(fw_path, "M35")){
			ErrorPrintf("This is not M35 module firmware!\n");
			return -1;
		}
		ret = m35f_process_update(ota_fds,atoi(argv[1]), fw_path);
    }else if(strstr(module_info.moduleVersion, M26)){
		if(!strstr(fw_path, "M26")){
			ErrorPrintf("This is not M26 module firmware!\n");
			return -1;
		}
		ret = m35f_process_update(ota_fds,atoi(argv[1]), fw_path);
	}else if(strstr(module_info.moduleVersion, SIMCOM_SIM6320C)){	
		if(!strstr(fw_path, "6320C")){
			ErrorPrintf("This is not SIMCOM_SIM6320C module firmware!\n");
            return -1;
		}
		//待实现
	}else if(strstr(module_info.moduleVersion, EC20F)){
		if(!strstr(fw_path, "EC20F")){
			ErrorPrintf("This is not EC20F module firmware!\n");
            return -1;
		}
		//待实现
	}else{
		upgrade_deinit(argv[1], ota_fds);
		bsp_api_deinit();
		printf("unkown module:%s",version);
		return -1;
	}
	
	if(ret < 0)
    {
        ErrorPrintf("process_update err: %d",ret);
		upgrade_deinit(argv[1], ota_fds);
		bsp_api_deinit();
        /*close(ota_fds[TTY_FD]);*/
        return ret;
    } 
    else if(ret == 1)
    {
        ErrorPrintf("Run new firmware fail, please restart module manually.");
        return -1;
    }


    //记录升级后的版本信息 
    get_module_version(ota_fds, module_info.newVersion);	
    //记录升级完成时间
    get_date(module_info.endDate);


    //升级成功，先去初始化，重置标记位，然后复位模块
    upgrade_deinit(argv[1], ota_fds);

    //重启模块
    module_reset(hwport);

	bsp_api_deinit();

    printf("Update Success\n");
    //记录版本信息
    record_version_info();
    return 0;
}
