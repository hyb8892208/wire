#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>	
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define AT_WRTIE_PIPE "/tmp/module_pipe/at-%d-r.pipe"
#define AT_READ_PIPE  "/tmp/module_pipe/at-%d-w.pipe"
#define AT_LEN 32
#define MAX_CHAN 32
#define MAX_SLOT 4
#define MAX_VALUE_LEN 128
#define SLOT_TEST_LOG  "/tmp/slot_test.log"

typedef enum atcommand_t{
	AT_TEST=0,
	AT_ECHO,
	AT_CMEE,
	AT_CFUN_MINI ,
	AT_CFUN_FULL,
	AT_SIM_STATE,
	AT_COMMOND_NUM,
}ATCOMMAND_T;

typedef struct at_info_t{
	char at[AT_LEN];
}AT_INFO_T;

//AT_INFO at_info[AT_COMMOND_NUM] = {
AT_INFO_T at_info[] = {
	"AT\r\n",
	"ATE0\r\n",
	"AT+CMEE=2\r\n",
	"AT+CFUN=0\r\n",
	"AT+CFUN=1\r\n",
	"AT+CPIN?\r\n",
};

typedef struct at_result_t{
	char result[MAX_SLOT][MAX_VALUE_LEN];
}AT_RESULT_T;

AT_RESULT_T at_results[MAX_CHAN];

int count = 0;
int sim_slots = 0;
pthread_mutex_t at_lock;

static char * replace_enter(char *src){
	char *p = src;
	while(*p != '\0'){
		if(*p == '\n' || *p == '\r')
			*p = ' ';
		p++;
	}
	return src;
}

static int write_at_cmd(int write_fd, int read_fd, char *data, int len, char *out, int out_len){
	char tmp[256];
	int try_count = 500;
	while(read(read_fd, tmp, sizeof(tmp)) > 0);

	if(write(write_fd, data, len) < 0){
		printf("write data error\n");
		return -1;
	}
	memset(tmp, 0, sizeof(tmp));
	while(try_count > 0){
		usleep(20000);
		try_count--;
		if(read(read_fd, tmp, sizeof(tmp))<= 0)
			continue;
		if( strstr(tmp, "OK") || strstr(tmp, "+CME ERROR:"))
			break;
	}
	if(try_count > 0){
		strncpy(out,tmp, out_len);
		return 0;
	}
	return -1;
}

static int set_sim_slot(int channel_id, int slot_id){
	char cmd[128];
	sprintf(cmd, "/my_tools/bsp_cli sim sel %d %d > /dev/null", channel_id, slot_id);
	return system(cmd);
}

void *at_thread_handler(void *data)
{
	AT_RESULT_T *result = (AT_RESULT_T *)data;
	char tmp[MAX_VALUE_LEN] = {0};
	char send_dev[256];
	char recv_dev[256];
	int write_fd = -1;
	int read_fd = -1;
	int try_count = 0;
	int i,j;
	int channel_id = result - at_results + 1;

	sprintf(send_dev, AT_WRTIE_PIPE, channel_id);
	sprintf(recv_dev, AT_READ_PIPE, channel_id);

	write_fd = open(send_dev, O_WRONLY);
	read_fd = open(recv_dev, O_RDONLY|O_NONBLOCK);
	if(write_fd < 0){
		printf("open %s fail.\n", send_dev);
		return (void *)NULL;
	}
	if(read_fd < 0){
		printf("open %s fail.\n", recv_dev);
		return (void *)NULL;
	}
	for(i = 0; i < sim_slots; i++){
		write_at_cmd(write_fd, read_fd, at_info[AT_CFUN_MINI].at, strlen(at_info[AT_CFUN_MINI].at), tmp, sizeof(tmp));
		set_sim_slot(channel_id, i+1);
		for(j = 0; j < AT_COMMOND_NUM; j++){
			try_count = 0;
try_again:
			memset(tmp, 0, sizeof(tmp));
			if(write_at_cmd(write_fd, read_fd, at_info[j].at, strlen(at_info[j].at), tmp, sizeof(tmp)) < 0){
				printf("[%d][%d]write data fail\n",channel_id, i+1);
				if(j == AT_TEST){
					sprintf(result->result[i], "send AT fail");
					break;
				}
				continue;
			}
			if(j == AT_CFUN_FULL)
				sleep(2);
			if(j == AT_SIM_STATE){
				printf("[%d][%d]%s\n", channel_id, i+1,  replace_enter(tmp));
				if(strstr(tmp, "SIM busy") && try_count < 10){
					sleep(1);
					try_count++;
					goto try_again;
				}
				memcpy(result->result[i], tmp, MAX_VALUE_LEN);
			}
		}
		//锁
		pthread_mutex_lock(&at_lock);
		count++;
		pthread_mutex_unlock(&at_lock);
	}
	return (void *)NULL;
}

void led_create_redis_thread(void *data)
{
	int ret;
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);


	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		printf("pthread_attr_setstacksize failed.\n");
		return;
	} 

	ret = pthread_create(&tid, &attr, at_thread_handler, data);
	if(ret < 0){
		printf("create led redis thread fail\n");
	}

}

int record_test_log(int total_chan, int total_slot){
	int i = 0,j =0;
	FILE *handle = fopen(SLOT_TEST_LOG,"w+");
	if(handle == NULL){
		printf("record log fail.\n");
		return -1;
	}
	for(i = 0; i < total_chan;i++){
		for(j = 0; j < total_slot; j++){
			fprintf(handle, "%d,%d,%s\n", i+1, j+1, at_results[i].result[j]);
		}
	}
	fclose(handle);
	return 0;
}

int main(int argc, char **argv)
{
	int i;
	int total_chan = 0;
	int total_slot = 0;
	if(argc < 2){
		printf("usage:%s $total_chan\n", argv[0]);
		return -1;
	}
	if(argc == 3){
		sim_slots = atoi(argv[2]);
	}
	if(sim_slots <= 0)
		sim_slots = MAX_SLOT;
	else if(sim_slots > MAX_SLOT)
		sim_slots = MAX_SLOT;

	total_chan = atoi(argv[1]);
	if(total_chan > MAX_CHAN)
		total_chan = 32;
	else if(total_chan <= 0)
		total_chan = 16;

	total_slot = total_chan * sim_slots;

	pthread_mutex_init(&at_lock, NULL);
	for(i = 0; i < total_chan;i++){
		led_create_redis_thread(&at_results[i]);
	}

	while(count < total_slot){
		sleep(1);
	}

	record_test_log(total_chan, MAX_SLOT);
	pthread_mutex_destroy(&at_lock);
	return 0;
}


