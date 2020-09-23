#include "../include/mms_inc.h"
#include <sys/file.h>
#include "../include/mms.h"
#include "../include/mms_get_devport.h"
#include "../include/mms_download.h"
#include "../include/mms_queue.h"
#include "../include/mms_file.h"
#include "../include/mms_redis_if.h"
#include "../include/mms_debug.h"
#include <signal.h>
#define	__LOCK_FILE__		"/var/lock/handle_mms.lock"

mms_t mms;
static int check_lock_process()
{
	int fd, rc;
	char *filename = __LOCK_FILE__;

	fd = open(filename, O_CREAT|O_RDWR, 0666);
	if (fd < 0)
		return -1;
	rc = flock(fd, LOCK_EX|LOCK_NB);
	if (rc < 0) {
		printf("%s is locked.\n", filename);
		close(fd);
		return -1;
	}

	return 0;
}

void sched_task()
{
	while(mms.run_flag == 1) {
		sleep(1);
	}

	return ;
}

//normal exit 
void signal_exit(int signum){
	mms.run_flag = 0;
}

void signal_reload_config(int signum){
	void *data;
	load_mms_conf(mms.mms_config);
	pthread_cancel(mms.mms_dl_pthread.dl_thread_id);//
	pthread_join(mms.mms_dl_pthread.dl_thread_id, &data);
	mms_create_dl_thread(&mms);
	
}

void mms_init(mms_t *p_mms){
	
	p_mms->run_flag = 1;
	mms_dl_init(p_mms);
	mms_result_init(p_mms);
	mms_config_init(p_mms);
	mms_hw_info_init();
	
	load_mms_queue_conf(p_mms->mms_dl_pthread.wait_queue);
	load_mms_conf(p_mms->mms_config);

}

void mms_create_thread(mms_t *p_mms){
	
	//create pthread
	mms_create_result_thread(p_mms);
	mms_create_dl_thread(p_mms);
	mms_redis_init(p_mms);
	mms_flush_file_init(p_mms);
	
}

void mms_deinit(mms_t *p_mms){
	mms_redis_deinit(p_mms);
	mms_flush_file_deinit(p_mms);
	mms_dl_deinit(p_mms);
	mms_result_deinit(p_mms);
	mms_config_deinit(p_mms->mms_config);
	mms_hw_info_deinit();
}

int main(int argc, char **argv)
{
	int log_level = 0;
	//SIGUSR1 reload config
	signal(SIGUSR1, signal_reload_config);
	//SIGQUIT, SIGHUP, SIGTREM, SIGINT normal exit process
	signal(SIGQUIT , signal_exit);
	signal(SIGHUP , signal_exit);
	signal(SIGTERM , signal_exit);
	signal(SIGINT , signal_exit);
	
	if (check_lock_process())
		return -1;
	if(argc >= 2)
		log_level = atoi(argv[1]);
	
	mms_log_init(log_level);
	
	mms_init(&mms);

	mms_create_thread(&mms);
	
	sched_task();
	
	mms_deinit(&mms);
	
	mms_log_deinit();
	return 0;
}

