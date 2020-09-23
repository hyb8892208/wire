
#include <stdio.h>
#include <string.h>
#include "menulanguage.h"


static int get_option_value(char * file_path, char * context_name, char * option_name, char * out_value)
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

	if( NULL == (fp=fopen(file_path,"r")) ) {
		mlang_print_debug("%s Can't open %s\n",file_path, __FUNCTION__);
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
						memcpy(name,buf+i+2,len-i-4);
						name[len-i-4] = '\0';
						fclose(fp);
						memcpy(out_value,name,len-i-3);
						return 0;
					}
				}
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	return -1;
}


static int menulang_get_mainmenu(mlangdata_main_menu_t *pmain_menu, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pmain_menu || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "Menu", tmp))) {
		strcpy(pmain_menu->Menu, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "SystemStatus", tmp))) {
		strcpy(pmain_menu->SystemStatus, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "NetworkInfo", tmp))) {
		strcpy(pmain_menu->NetworkInfo, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "DeviceInfo", tmp))) {
		strcpy(pmain_menu->DeviceInfo, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "WebAccess", tmp))) {
		strcpy(pmain_menu->WebAccess, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "SSHAccess", tmp))) {
		strcpy(pmain_menu->SSHAccess, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "NetworkSettings", tmp))) {
		strcpy(pmain_menu->NetworkSettings, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "SystemControl", tmp))) {
		strcpy(pmain_menu->SystemControl, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "PowerSave", tmp))) {
		strcpy(pmain_menu->PowerSave, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "Language", tmp))) {
		strcpy(pmain_menu->Language, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_MAIN_MENU, "Back", tmp))) {
		strcpy(pmain_menu->Back, tmp);	
	}
	else {
		goto _error;
	}	
	
	return 0;
_error:
	return -1;	
}


static int menulang_get_system_status(mlangdata_system_status_t *psys_sta, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!psys_sta || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_SYS_STA, "ActiveWirelessChannels", tmp))) {
		strcpy(psys_sta->ActiveWirelessChannels, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_SYS_STA, "ConcurrentCalls", tmp))) {
		strcpy(psys_sta->ConcurrentCalls, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_SYS_STA, "StorageUsage", tmp))) {
		strcpy(psys_sta->StorageUsage, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_SYS_STA, "MemoryUsage", tmp))) {
		strcpy(psys_sta->MemoryUsage, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_network_info(mlangdata_network_info_t*pnet_info, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pnet_info || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_NET_INFO, "Wan", tmp))) {
		strcpy(pnet_info->Wan, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_INFO, "Lan", tmp))) {
		strcpy(pnet_info->Lan, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_INFO, "Mode", tmp))) {
		strcpy(pnet_info->Mode, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_INFO, "Mac", tmp))) {
		strcpy(pnet_info->Mac, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_INFO, "IP", tmp))) {
		strcpy(pnet_info->IP, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_INFO, "Mask", tmp))) {
		strcpy(pnet_info->Mask, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_INFO, "DNS", tmp))) {
		strcpy(pnet_info->DNS, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_network_settings(mlangdata_network_settings_t *pnet_set, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pnet_set || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_NET_SET, "WanMode", tmp))) {
		strcpy(pnet_set->WanMode, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_SET, "LanMode", tmp))) {
		strcpy(pnet_set->LanMode, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_system_control(mlangdata_system_control_t *psys_ctrl, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!psys_ctrl || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_SYS_CTRL, "Reboot", tmp))) {
		strcpy(psys_ctrl->Reboot, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_SYS_CTRL, "FactoryReset", tmp))) {
		strcpy(psys_ctrl->FactoryReset, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_network_mode(mlangdata_network_mode_t *pnet_mode, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pnet_mode || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_NET_MODE, "DHCP", tmp))) {
		strcpy(pnet_mode->DHCP, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_MODE, "Static", tmp))) {
		strcpy(pnet_mode->Static, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_MODE, "Factory", tmp))) {
		strcpy(pnet_mode->Factory, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_NET_MODE, "Disable", tmp))) {
		strcpy(pnet_mode->Disable, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_device_info(mlangdata_device_info_t *pdev_info, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pdev_info || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_DEV_INFO, "ProductName", tmp))) {
		strcpy(pdev_info->ProductName, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_DEV_INFO, "ModelDescription", tmp))) {
		strcpy(pdev_info->ModelDescription, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_DEV_INFO, "SoftwareVersion", tmp))) {
		strcpy(pdev_info->SoftwareVersion, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_DEV_INFO, "HardwareVersion", tmp))) {
		strcpy(pdev_info->HardwareVersion, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_DEV_INFO, "SystemTime", tmp))) {
		strcpy(pdev_info->SystemTime, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_DEV_INFO, "UpTime", tmp))) {
		strcpy(pdev_info->UpTime, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_web_access(mlangdata_web_access_t *pweb_acces, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pweb_acces || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_WEB_ACCESS, "Protocol", tmp))) {
		strcpy(pweb_acces->Protocol, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_WEB_ACCESS, "Port", tmp))) {
		strcpy(pweb_acces->Port, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_language(mlangdata_language_t *plang, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!plang || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_LANGUAGE, "english", tmp))) {
		strcpy(plang->english, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_LANGUAGE, "chinese", tmp))) {
		strcpy(plang->chinese, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_confirm(mlangdata_confirm_t *pconfirm, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pconfirm || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_CONFIRM, "Confirm", tmp))) {
		strcpy(pconfirm->Confirm, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_CONFIRM, "Cancel", tmp))) {
		strcpy(pconfirm->Cancel, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_switch(mlangdata_onoff_t *pswitch, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pswitch || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_SWITCH, "Enable", tmp))) {
		strcpy(pswitch->Enable, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_SWITCH, "Disable", tmp))) {
		strcpy(pswitch->Disable, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_get_control_result(mlangdata_control_result_t *pctrl_res, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pctrl_res || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_CONTROL_RES, "Success", tmp))) {
		strcpy(pctrl_res->Success, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_CONTROL_RES, "Fail", tmp))) {
		strcpy(pctrl_res->Fail, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}

static int menulang_get_web(mlangdata_web_t *pweb, char *langpath)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pweb || !langpath) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(langpath, CONTEXT_WEB, "Access", tmp))) {
		strcpy(pweb->Access, tmp);	
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(langpath, CONTEXT_WEB, "ResetPassword", tmp))) {
		strcpy(pweb->ResetPassword, tmp);	
	}
	else {
		goto _error;
	}

	return 0;
_error:
	return -1;	
}


static int menulang_set_default_mainmenu(mlangdata_main_menu_t * pmain_menu)
{
	if(!pmain_menu) {
		return -1;
	}

	memset(pmain_menu, 0, sizeof(mlangdata_main_menu_t));
	strcpy(pmain_menu->Menu, "Menu");
	strcpy(pmain_menu->SystemStatus, "System Status");
	strcpy(pmain_menu->NetworkInfo, "Network Info");
	strcpy(pmain_menu->DeviceInfo, "Device Info");
	strcpy(pmain_menu->WebAccess, "Web Access");
	strcpy(pmain_menu->SSHAccess, "SSH Access");
	strcpy(pmain_menu->NetworkSettings, "Network Settings");
	strcpy(pmain_menu->SystemControl, "System Control");
	strcpy(pmain_menu->PowerSave, "Power Save");
	strcpy(pmain_menu->Language, "Language");
	strcpy(pmain_menu->Back, "Back");

	return 0;
}


static int menulang_set_default_system_status(mlangdata_system_status_t * psys_sta)
{
	if(!psys_sta) {
		return -1;
	}

	memset(psys_sta, 0, sizeof(mlangdata_system_status_t));
	strcpy(psys_sta->ActiveWirelessChannels, "Active Wireless Channels");
	strcpy(psys_sta->ConcurrentCalls, "Concurrent Calls");
	strcpy(psys_sta->StorageUsage, "Storage Usage");
	strcpy(psys_sta->MemoryUsage, "Memory Usage");

	return 0;
}


static int menulang_set_default_network_info(mlangdata_network_info_t *pnet_info)
{
	if(!pnet_info) {
		return -1;
	}

	memset(pnet_info, 0, sizeof(mlangdata_network_info_t));
	strcpy(pnet_info->Wan, "WAN");
	strcpy(pnet_info->Lan, "LAN");
	strcpy(pnet_info->Mode, "Mode");
	strcpy(pnet_info->Mac, "MAC");
	strcpy(pnet_info->IP, "IP");
	strcpy(pnet_info->Mask, "Mask");
	strcpy(pnet_info->DNS, "DNS");	

	return 0;
}


static int menulang_set_default_network_settings(mlangdata_network_settings_t *pnet_set)
{
	if(!pnet_set) {
		return -1;
	}

	memset(pnet_set, 0, sizeof(mlangdata_network_settings_t));
	strcpy(pnet_set->WanMode, "WAN MODE");
	strcpy(pnet_set->LanMode, "LAN MODE");	

	return 0;
}


static int menulang_set_default_system_control(mlangdata_system_control_t *psys_ctrl)
{
	if(!psys_ctrl) {
		return -1;
	}

	memset(psys_ctrl, 0, sizeof(mlangdata_system_control_t));
	strcpy(psys_ctrl->Reboot, "Reboot");
	strcpy(psys_ctrl->FactoryReset, "Factory Reset");	

	return 0;
}


static int menulang_set_default_network_mode(mlangdata_network_mode_t *pnet_mode)
{
	if(!pnet_mode) {
		return -1;
	}

	memset(pnet_mode, 0, sizeof(mlangdata_network_mode_t));
	strcpy(pnet_mode->DHCP, "DHCP");
	strcpy(pnet_mode->Static, "Static");	
	strcpy(pnet_mode->Factory, "Factory");	
	strcpy(pnet_mode->Disable, "Disable");	

	return 0;
}


static int menulang_set_default_device_info(mlangdata_device_info_t *pdev_info)
{
	if(!pdev_info) {
		return -1;
	}

	memset(pdev_info, 0, sizeof(mlangdata_device_info_t));
	strcpy(pdev_info->ProductName, "Product Name");
	strcpy(pdev_info->ModelDescription, "Model Description");	
	strcpy(pdev_info->SoftwareVersion, "Software Version");	
	strcpy(pdev_info->HardwareVersion, "Hardware Version");
	strcpy(pdev_info->SystemTime, "SystemTime");	
	strcpy(pdev_info->UpTime, "Up Time");	

	return 0;
}


static int menulang_set_default_web_access(mlangdata_web_access_t *pweb_access)
{
	if(!pweb_access) {
		return -1;
	}

	memset(pweb_access, 0, sizeof(mlangdata_web_access_t));
	strcpy(pweb_access->Protocol, "Protocol");
	strcpy(pweb_access->Port, "Port");		

	return 0;
}


static int menulang_set_default_language(mlangdata_language_t *plang)
{
	if(!plang) {
		return -1;
	}

	memset(plang, 0, sizeof(mlangdata_language_t));
	strcpy(plang->english, "English");
	strcpy(plang->chinese, "Chinese");		

	return 0;
}


static int menulang_set_default_confirm(mlangdata_confirm_t *pconfim)
{
	if(!pconfim) {
		return -1;
	}

	memset(pconfim, 0, sizeof(mlangdata_confirm_t));
	strcpy(pconfim->Confirm, "Confirm");
	strcpy(pconfim->Cancel, "Cancel");		

	return 0;
}



static int menulang_set_default_switch(mlangdata_onoff_t *ponoff)
{
	if(!ponoff) {
		return -1;
	}

	memset(ponoff, 0, sizeof(mlangdata_onoff_t));
	strcpy(ponoff->Enable, "Enable");
	strcpy(ponoff->Disable, "Disable");		

	return 0;
}


static int menulang_set_default_control_result(mlangdata_control_result_t *pctrl_res)
{
	if(!pctrl_res) {
		return -1;
	}

	memset(pctrl_res, 0, sizeof(mlangdata_control_result_t));
	strcpy(pctrl_res->Success, "success");
	strcpy(pctrl_res->Fail, "fail"); 	

	return 0;
}

static int menulang_set_default_web(mlangdata_web_t *pweb)
{
	if(!pweb) {
		return -1;
	}

	memset(pweb, 0, sizeof(mlangdata_web_t));
	strcpy(pweb->Access, "Access");
	strcpy(pweb->ResetPassword, "Reset Password"); 	

	return 0;
}


static int menulang_set_default_data(mlangdata_t *plang)
{
	if(!plang) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}

	menulang_set_default_mainmenu(&plang->main_menu);
	menulang_set_default_system_status(&plang->system_status);
	menulang_set_default_network_info(&plang->network_info);
	menulang_set_default_network_settings(&plang->network_settings);
	menulang_set_default_system_control(&plang->system_control);
	menulang_set_default_network_mode(&plang->network_mode);
	menulang_set_default_device_info(&plang->device_info);
	menulang_set_default_web_access(&plang->web_access);
	menulang_set_default_language(&plang->language);
	menulang_set_default_confirm(&plang->confirm);
	menulang_set_default_switch(&plang->onoff);
	menulang_set_default_control_result(&plang->control_result);
	menulang_set_default_web(&plang->web);

	return 0;
}

int menulang_read_data(menu_language_t lang, mlangdata_t *plang)
{
	int res = -1;
	char langpath[128] = {0};

	if(!plang) {
		mlang_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}

	if(lang == LANG_CN) 
		strcpy(langpath, MENU_LANG_CN_PATH);
	else 
		strcpy(langpath, MENU_LANG_EN_PATH);	
			
	res = menulang_get_mainmenu(&plang->main_menu, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_mainmenu fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_system_status(&plang->system_status, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_system_status fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_network_info(&plang->network_info, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_network_info fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_network_settings(&plang->network_settings, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_network_settings fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_system_control(&plang->system_control, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_system_control fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_network_mode(&plang->network_mode, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_network_mode fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_device_info(&plang->device_info, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_device_info fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_web_access(&plang->web_access, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_web_access fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_language(&plang->language, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_language fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_confirm(&plang->confirm, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_confirm fail.\n", __FUNCTION__);
		goto _out;
	}	
	
	res = menulang_get_switch(&plang->onoff, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_switch fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_control_result(&plang->control_result, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_control_result fail.\n", __FUNCTION__);
		goto _out;
	}

	res = menulang_get_web(&plang->web, langpath);	
	if(res < 0) {
		mlang_print_error("%s menulang_get_web fail.\n", __FUNCTION__);
		goto _out;
	}
	
	return 0;
_out:
	menulang_set_default_data(plang);
	return -1;
}




