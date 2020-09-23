#include "openvox_status_api.h"
#include "openvox_process_bar.h"
#include "openvox_version_record.h"


extern "C" int set_total_size(int size){

	UpgradeCount::getInstance()->set_total_size(size);
	return 0;
}

extern "C" int set_write_size(int size){

	UpgradeCount::getInstance()->set_write_size(size);
	return 0;
}

extern "C" int process_par(char *str){

	UpgradeCount::getInstance()->process_bar(str);
	return 0;
}


extern "C" int set_channel(int channel){
	set_upgrade_channel(channel);
	return 0;
}

extern "C" int set_old_version(){
	module_info::getInstance()->set_old_version(NULL);
	return 0;
}

extern "C" int set_new_version(){
	module_info::getInstance()->set_new_version(NULL);
	return 0;

}

extern "C" int set_module_version(){
	module_info::getInstance()->set_mod_version(NULL);
	return 0;
}

extern "C" int set_start_time(){
	module_info::getInstance()->set_start_time(NULL);
	return 0;
}

extern "C" int set_end_time(){
	module_info::getInstance()->set_end_time(NULL);
	return 0;
}

extern "C" int set_upgrade_state(int state){
	module_info::getInstance()->set_upgrade_state(state);
	return 0;
}
extern "C" int record_info_to_file(){
	module_info::getInstance()->record_info_to_file();
	return 0;
}

