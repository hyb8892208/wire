#include "openvox_process_bar.h"
#include "openvox_version_record.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

UpgradeCount* UpgradeCount::instance = NULL;

UpgradeCount *UpgradeCount::getInstance(){
	if(instance == NULL){
		instance = new UpgradeCount();
	}
	return instance;
}

UpgradeCount::UpgradeCount(){
	char filename[64] = {0};
	sprintf(filename, "/www/chn_%d", module_info::getInstance()->get_channel());
	m_handle = fopen(filename, "w+");
	if(m_handle == NULL){
		std::cout << "open log file failed!" <<std::endl;
	}
	m_last_size = 0;
	m_write_size = 0;
	m_total_size = 0;
}

UpgradeCount::~UpgradeCount(){
	if(m_handle)
		fclose(m_handle);
}

void UpgradeCount::set_total_size(long long total_size){
	m_total_size = total_size;
}

void UpgradeCount::set_write_size(long long write_size){
	m_write_size = m_write_size + write_size;
}

void UpgradeCount::process_bar(char *result){
	fseek(m_handle, 0-m_last_size, SEEK_CUR);
	
	if(result){
		m_last_size = fprintf(m_handle, "%s", result);
		fflush(m_handle);
	}
	else{
		m_last_size = fprintf(m_handle, "%d", (m_write_size * 100) /m_total_size);
	}
}
