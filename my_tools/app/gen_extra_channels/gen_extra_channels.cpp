#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include "acfa_file.h"
#include "acfa_config.h"
using namespace std;
#define MAX (32)
#define NOFIND_ERR 1


static void extra_append_spare_line(acfa::ini_file *extra_channels){
	
	acfa::ini_basic_line *line = new acfa::ini_spare_line();
	extra_channels->append_line(line);
	// line = new acfa::ini_spare_line();
	// extra_channels->append_line(line);
}


static void rebuild_extra_bcch_line(acfa::ini_file *gw_gsm, char *section, acfa::ini_file *extra_channels){
	char *bcch[8] = {"bcch_type", "bcch_range", "bcch_fixed", "bcch_calls", "bcch_faileds", "bcch_asr", "bcch_incalls", "bcch_minrxl"};
	char *tmp_str = NULL;
	for(int i = 0; i < 8; i++){
		tmp_str = (char *)gw_gsm->get_value(section, (char *)bcch[i]);
		if(tmp_str == NULL){
			extra_channels->append_value((char *)"", bcch[i], (char *)"");
		}
		tmp_str = NULL;
	}
}
static void rebuild_extra_dail_line(acfa::ini_file *gw_gsm, char *section, acfa::ini_file *extra_channels){
	char *dial[12] = {"dl_step", "dl_single_sw", "dl_single_limit", "dl_total_sw", "dl_total_limit", "dl_free_time", "dl_warning_time", "dl_warning_num", "dl_warning_describe", "dl_auto_reset_sw", "dl_auto_reset_type", "dl_auto_reset_date"};
	char *tmp_str = NULL;
	for(int i = 0; i < 12; i++){
		tmp_str = (char *)gw_gsm->get_value(section, (char *)dial[i]);
		if(tmp_str == NULL){
			if(!strcmp(dial[i], "dl_step")){
				extra_channels->append_value((char *)"", dial[i], (char *)"60");
			}else if(!strcmp(dial[i], "dl_single_sw")){
				extra_channels->append_value((char *)"", dial[i], (char *)"off");
			}else if(!strcmp(dial[i], "dl_total_sw")){
				extra_channels->append_value((char *)"", dial[i], (char *)"off");
			}else if(!strcmp(dial[i], "dl_free_time")){
				extra_channels->append_value((char *)"", dial[i], (char *)"0");
			}else if(!strcmp(dial[i], "dl_warning_time")){
				extra_channels->append_value((char *)"", dial[i], (char *)"0");
			}else if(!strcmp(dial[i], "dl_auto_reset_sw")){
				extra_channels->append_value((char *)"", dial[i], (char *)"off");
			}else if(!strcmp(dial[i], "dl_auto_reset_type")){
				extra_channels->append_value((char *)"", dial[i], (char *)"day");
			}else{
				extra_channels->append_value((char *)"", dial[i], (char *)"");
			}
		}
		tmp_str = NULL;
	}
}
static void alg_get_vaild_channel_num(acfa::ini_file *gw_gsm, std::vector<int> &varry){
	acfa::ini_section *sec = gw_gsm->get_section_list()._first;
	while(sec){
		char *section = (char *)sec->_pos->get_key();
		acfa::ini_basic_line *line = gw_gsm->find_first(section, (char *)"name");
		if(!line){
			sec = sec->_next;
			continue;
		}
		varry.push_back(atoi(section));
		sec = sec->_next;
	}
	sort(varry.begin(), varry.end());
}

static void GSM_Settings_get_vaild_group(acfa::ini_file *gw_gsm, std::vector<std::string> &group){
    acfa::ini_section *sec = gw_gsm->get_section_list()._first;
    while(sec){
        char *section = (char *)sec->_pos->get_key();
        acfa::ini_basic_line *line = gw_gsm->find_first(section, (char *)"name");
        if(!line){
            sec = sec->_next;
            continue;
        }    
        group.push_back("");
        sec = sec->_next;
    }    
    sort(group.begin(), group.end());
}


static int GSM_Settings_get_group_info(acfa::ini_file *gw_group, std::vector<std::string> &arr_group)
{

	char tmp[2048] = {0};

	acfa::ini_section *sec = gw_group->get_section_list()._first;

	while(sec) {
		char * section = (char *)sec->_pos->get_key();
		if((section != NULL) && (strlen(section) != 0))
		{
			char * type = (char *)gw_group->get_value(section,(char *)"type");
			if((type != NULL) && (!strcasecmp(type,"gsm"))) 
			{
				char * order = (char *)gw_group->get_value(section,(char *)"order");

				char * members = (char *)gw_group->get_value(section,(char *)"members");
				memset(tmp,0,sizeof(tmp));
				memcpy(tmp,members,strlen(members));
				char *buf = tmp;
				char *token;
				unsigned int chanel_num;
				while((token = strsep(&buf, ",")) != NULL){
					chanel_num = atoi(token+4)-1;
					if(chanel_num < arr_group.size()) {
						arr_group[chanel_num].append(order);
						arr_group[chanel_num].append(",");
					} else {
						printf(" group info channel:%d out of range!\n", chanel_num);
					}
				}
			}
		}
		sec = sec->_next;
	}

	if(arr_group.size()){
		for(unsigned int i=0 ; i<arr_group.size() ; i++) {
			if(arr_group[i].size())
				arr_group[i].erase(arr_group[i].end()-1, arr_group[i].end());
		}
	}

	return 0;
}

int GSM_Setting_ExtraChannles(acfa::ini_file *gw_gsm, acfa::ini_file *extra_channels){
	//std::cout << "{" << __func__ << ":" << __LINE__ << "}"<< std::endl;
	char *key = NULL, *value = NULL, *section = NULL;
	acfa::ini_section *gw_gsm_section = NULL;
	acfa::ini_basic_line *line = NULL;
	acfa::ini_file gw_group;
	gw_group.load("/etc/asterisk/gw_group.conf");
	
	std::vector<int> varry;
	std::vector<std::string> group;
	alg_get_vaild_channel_num(gw_gsm, varry);
	GSM_Settings_get_vaild_group(gw_gsm, group);
	GSM_Settings_get_group_info(&gw_group, group);
	int channels = varry.size();
	std::cout << "{" << "channel SUM:" << channels << "}" << std::endl;

	for(int i = 0; i < channels; i++){
		char tmp[MAX] = {0};
		char tmp_str[MAX] = {0};
		sprintf(tmp, "%d", varry[i]);
		section = (char *)tmp;

		gw_gsm_section = gw_gsm->find_section(tmp);

		//add group , context, signalling, smscodec and switchtype;
		sprintf(tmp_str, "gsm-%d", varry[i]);
		extra_channels->append_value((char *)"", (char *)"group", (char *)group[i].c_str());
		extra_channels->append_value((char *)"", (char *)"context", tmp_str);
		extra_channels->append_value((char *)"", (char *)"signalling", (char *)"gsm");
		extra_channels->append_value((char *)"", (char *)"smscodec", (char *)"utf-8");
		extra_channels->append_value((char *)"", (char *)"switchtype", (char *)"SIMCOM_SIM840W");
		if(gw_gsm_section){
			line = gw_gsm_section->_pos->_next;
			while(line != gw_gsm_section->_last){
				char *key = (char *)line->get_key();
				char *value = (char *)line->get_value();
				if(key && value){
					if(!strcmp(key, "name") || !strcmp(key, "tosip") || !strcmp(key, "operator_fullname") || !strcmp(key, "codec_selected") || !strcmp(key, "needpin")){
						//std::cout << "Don't need to copy" << std::endl;
					} else {
						if(!strcmp(key, "pin")){
							char *needpin = NULL;
							needpin = (char *)gw_gsm->get_value(section, (char *)"needpin");
							if(needpin != NULL && !strcmp(needpin, "true")){
								extra_channels->append_value((char * )"", key, value);	
							} 
						} else {
							extra_channels->append_value((char * )"", key, value);
						}
					}
				}
				//std::cout << key << "="<< value << std::endl;
				line = line->_next;
			}
			if(line){
				key = (char *)line->get_key();
				value = (char *)line->get_value();	
				if(key && value)
					extra_channels->append_value((char * )"", key, value);
			}
			rebuild_extra_dail_line(gw_gsm, section, extra_channels);
			rebuild_extra_bcch_line(gw_gsm, section, extra_channels);
			sprintf(tmp_str, "%d", 2 * varry[i] - 1);
			extra_channels->append_value((char * )"", (char *)"channel", tmp_str, 0);
			extra_append_spare_line(extra_channels);
		}

	}
	return 0;
}

int gw_gsm2extra_channels(void){
	acfa::ini_file gw_gsm;
	acfa::ini_file extra_channels;
	gw_gsm.load("/etc/asterisk/gw_gsm.conf");
	GSM_Setting_ExtraChannles(&gw_gsm, &extra_channels);
	extra_channels.save_to("/etc/asterisk/extra-channels.conf");
	return 0;
}
int main(int argc, const char **argv)
{
	gw_gsm2extra_channels();
	return 0;
}
