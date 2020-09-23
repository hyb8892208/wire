#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "menufunction.h"
#include "menulanguage.h"



static int menufunc_get_buff_from_sh(char *outbuff, char *params)
{
	FILE *fp;
	char buffer[READ_BUFF_LEN] = {0};
	char cmdline[READ_BUFF_LEN] = {0};
	
	if(!outbuff || !params) {
		menu_print_error("input fail\n");
		return -1;
	}

	sprintf(cmdline, "%s %s", LCD_SH_PATH, params);
	fp = popen(cmdline,"r");
	fgets(buffer, sizeof(buffer), fp);
	memcpy(outbuff, buffer, strlen(buffer));
	pclose(fp);
	return 0;
}


int menufunc_pased_buff_from_sh(char *inbuff, char outbuff[][READ_BUFF_LEN])
{
    char *ptr;
	char *retptr;
	char *savestr;
	int i = 0;

	if(!inbuff || !(*outbuff)) {
		menu_print_error("input fail\n");
		return -1;
	}

    ptr = inbuff;
    while ((retptr=strtok_r(ptr, ",", &savestr)) != NULL) {
		if(i < MAX_PARAMS) {
			memcpy(outbuff[i++], retptr, strlen(retptr));
//			menu_print_debug("outbuff[%d]: %s\n", i-1, outbuff[i-1]);	
		}
        ptr = NULL;
    }	
	
}

int menufunc_get_system_status(mdata_system_status_t *status)
{
	char buffer[READ_BUFF_LEN] = {0};
	char readbuff[MAX_PARAMS][READ_BUFF_LEN] = {0};
	int i = 0;

	if(!status) {
		menu_print_error("input fail\n");
		return -1;	
	}
	
	memset(status, 0, sizeof(mdata_system_status_t));
	menufunc_get_buff_from_sh(buffer, "system_status");	
	menufunc_pased_buff_from_sh(buffer, readbuff);
	
	memcpy(status->active_wireless_channels, readbuff[0], strlen(readbuff[0]));	
	memcpy(status->current_calls, readbuff[1], strlen(readbuff[1]));
	memcpy(status->storage_usage, readbuff[2], strlen(readbuff[2]));
	memcpy(status->memory_usage, readbuff[3], strlen(readbuff[3]));

	return 0;
}

int menufunc_get_network_info(network_type_t type, mdata_network_info_t *netinfo)
{
	char buffer[READ_BUFF_LEN] = {0};
	char readbuff[MAX_PARAMS][READ_BUFF_LEN] = {0};

	if(!netinfo) {
		menu_print_error("input fail\n");
		return -1;	
	}
	
	memset(netinfo, 0, sizeof(mdata_network_info_t));
	menufunc_get_buff_from_sh(buffer, (type==NET_LAN)?"network_lan_info":"network_wan_info");
	menufunc_pased_buff_from_sh(buffer, readbuff);

	memcpy(netinfo->mode, readbuff[0], strlen(readbuff[0]));
	memcpy(netinfo->MAC, readbuff[1], strlen(readbuff[1]));
	memcpy(netinfo->ipaddr, readbuff[2], strlen(readbuff[2]));
	memcpy(netinfo->mask, readbuff[3], strlen(readbuff[3]));	
	memcpy(netinfo->DNS, readbuff[4], strlen(readbuff[4]));	

	return 0;
}

int menufunc_get_device_info(mdata_device_info_t *devinfo)
{
	char buffer[READ_BUFF_LEN] = {0};
	char readbuff[MAX_PARAMS][READ_BUFF_LEN] = {0};

	if(!devinfo) {
		menu_print_error("input fail\n");
		return -1;	
	}

	memset(devinfo, 0, sizeof(mdata_device_info_t));
	menufunc_get_buff_from_sh(buffer, "device_info");	
	menufunc_pased_buff_from_sh(buffer, readbuff);

	memcpy(devinfo->ProductName, readbuff[0], strlen(readbuff[0]));
	memcpy(devinfo->ModelDescription, readbuff[1], strlen(readbuff[1]));
	memcpy(devinfo->SoftwareVer, readbuff[2], strlen(readbuff[2]));
	memcpy(devinfo->HardwareVer, readbuff[3], strlen(readbuff[3]));	
	memcpy(devinfo->SystemTime, readbuff[4], strlen(readbuff[4]));	
	memcpy(devinfo->UpTime, readbuff[5], strlen(readbuff[5]));	

	return 0;
}

int menufunc_get_web_access(mdata_web_access_t *webaccess)
{
	char buffer[READ_BUFF_LEN] = {0};
	char readbuff[MAX_PARAMS][READ_BUFF_LEN] = {0};

	if(!webaccess) {
		menu_print_error("input fail\n");
		return -1;		
	}

	memset(webaccess, 0, sizeof(mdata_web_access_t));
	menufunc_get_buff_from_sh(buffer, "web_access");
	menufunc_pased_buff_from_sh(buffer, readbuff);
	
	memcpy(webaccess->Protocol, readbuff[0], strlen(readbuff[0]));
	memcpy(webaccess->Port, readbuff[1], strlen(readbuff[1]));

	return 0;
}

int menufunc_get_language_type(void)
{
	char buffer[READ_BUFF_LEN] = {0};
	
	menufunc_get_buff_from_sh(buffer, "language_type");
	buffer[2] = '\0';

	if(!strcmp(buffer, "CH")) {
		return LANG_CN;
	}else if(!strcmp(buffer, "EN")) {
		return LANG_EN;
	}
	return LANG_EN;
}

int menufunc_set_language(int lang)
{
	char buffer[READ_BUFF_LEN] = {0};

	menufunc_get_buff_from_sh(buffer, (lang==LANG_EN)?"language_setting EN":"language_setting CH");
	return 0;
}


int menufunc_set_ssh_access(ssh_onoff_t onoff)
{
	char buffer[READ_BUFF_LEN] = {0};
	
	menufunc_get_buff_from_sh(buffer, (onoff==SSH_ENABLE)?"ssh_access start":"ssh_access stop");
	
	return 0;
}

int menufunc_set_networksettings(network_type_t type, network_mode_t mode)
{
	char buffer[READ_BUFF_LEN] = {0};
	
	if(NET_DHCP == mode) {		
		menufunc_get_buff_from_sh(buffer, (type==NET_LAN)?"network_lan_setting dhcp":"network_wan_setting dhcp");
	} else if(NET_STATIC == mode) {
		menufunc_get_buff_from_sh(buffer, (type==NET_LAN)?"network_lan_setting static":"network_wan_setting static");
	} else if(NET_FACTORY == mode) {
		menufunc_get_buff_from_sh(buffer, (type==NET_LAN)?"network_lan_setting factory":"network_wan_setting factory");
	}

	return 0;
}

int menufunc_system_reboot(void)
{
	char buffer[READ_BUFF_LEN] = {0};
	
	menufunc_get_buff_from_sh(buffer, "reboot_setting");
	return 0;
}

int menufunc_factory_reset(void)
{
	char buffer[READ_BUFF_LEN] = {0};
	
	menufunc_get_buff_from_sh(buffer, "factory_reset");
	return 0;
}


int menufunc_web_reset_password(void)
{
	char buffer[READ_BUFF_LEN] = {0};
	
	menufunc_get_buff_from_sh(buffer, "reset_lighttpd_login");
	return 0;
}




