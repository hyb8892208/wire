

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "gsoap_cli.h"

typedef enum handle_type_e{
	HANDLE_CALL_INTERNAL = 1 << 0,//内部呼叫
	HANDLE_CALL_EXTERNAL = 1 << 1,//外部呼叫
	HANDLE_SMS_INTERNAL = 1 << 2,//内部短信
	HANDLE_SMS_EXTENAL = 1 << 3,//外部短信
	HANDLE_INTERNET = 1 << 4,//拨号上网
}handle_type_t;


int get_handle_type_name(unsigned char handle_type){
	char buf[256] = {0};
	
	if(handle_type &HANDLE_CALL_INTERNAL ){
		strcat(buf,"call_internal," );
	}
	if(handle_type & HANDLE_CALL_EXTERNAL){
		strcat(buf, "call_external,");
	}
	if(handle_type & HANDLE_SMS_INTERNAL){
		strcat(buf, "sms_internal,");
	}
	if(handle_type & HANDLE_SMS_EXTENAL){
		strcat(buf, "sms_external,");
	}
	if(handle_type & HANDLE_INTERNET){
		strcat(buf, "internet,");
	}
	if(strlen(buf) > 0){
		buf[strlen(buf) - 1] = '\0';
	}else{
		strcpy(buf, "unkown");
	}
	printf("%s\n\n",buf);
	return 0;
}


int cli_usage_main(int argc, char **argv){
	printf("usage:\n");
	printf("\tconfig get [channle]\n");
	printf("\tcalldata get [channle]\n");
	printf("\tconfig reload\n");
	printf("\tflush status\n");
	return 0;
}

int cli_reload_config(int argc ,char **argv)
{
	if(argc < 2){
		cli_usage_main(argc, argv);
		return -1;
	}
	callmonitor_reload_config();
	printf("reload config success\n");
	return 0;
}

int cli_flush_status(int argc ,char **argv)
{
	if(argc < 2){
		cli_usage_main(argc, argv);
		return -1;
	}
	callmonitor_flush_status();
	printf("reload config success\n");
	return 0;
}

int cli_get_config(int argc, char **argv)
{
	if(argc  < 3){
		cli_usage_main(argc, argv);
		return -1;
	}
	int chan_id = atoi(argv[2]);
	struct callmonitor_chan_config config;
	if(callmonitor_get_chan_config(chan_id, &config) == 0){
		printf("\n");
		printf("total_answers=%d\n", config.total_call_answers);
		printf("total_times=%d\n", config.total_call_times);
		printf("total_dur=%d\n", config.total_call_dur);
		printf("online_time=%d\n", config.online_time/60);
		printf("handle_type=");
		get_handle_type_name(config.handle_type);
	}else 
		printf("\nget call data fail\n\n");
	return 0;
}

int cli_get_calldata(int argc, char **argv)
{
	if(argc < 3){
		cli_usage_main(argc, argv);
		return -1;
	}
	int chan_id = atoi(argv[2]);
	struct callmonitor_chan_data config;
	if(callmonitor_get_chan_data(chan_id, &config) == 0){
		printf("\n");
		printf("cur_answers=%d\n", config.cur_call_answers);
		printf("cur_times=%d\n", config.cur_call_times);
		printf("cur_dur=%d\n", config.cur_call_dur);
		printf("last_online_time=%d\n\n", config.last_online_time/60);
	}else
		printf("\nget call data fail\n\n");
}


int test_one(int argc, char **argv)
{
	int chan_id = 0;
	if(argc >= 2 && strcmp(argv[0], "config") == 0 ){
		if(strcmp(argv[1], "get" )== 0){
			cli_get_config(argc, argv);
		}else if(strcmp(argv[1], "reload") == 0){
			cli_reload_config(argc, argv);
		}
	}else if(argc >= 2 && strcmp(argv[0], "calldata") == 0){
		if(strcmp(argv[1], "get") == 0)
			cli_get_calldata(argc, argv);
	}else if(argc >= 2 && strcmp(argv[0], "flush") == 0){
		if(strcmp(argv[1], "status") == 0)
			cli_flush_status(argc, argv);
	}
	return 0;
}

void cli_reg(){
	cb_func_reg("config reload", cli_reload_config);
	cb_func_reg("config get", cli_get_config);
	cb_func_reg("calldata get",cli_get_calldata);
	cb_func_reg("flush status", cli_flush_status);
	
}

int main(int argc, char **argv){

	if(argc == 1){
		cli_init();
		cb_func_reg("?", cli_usage_main);
		cb_func_reg("help", cli_usage_main);
		cli_reg();
		run_main((char *)"callmonitor>");
	}else{
		test_one(argc - 1, argv + 1);
	}

	return 0;
}
