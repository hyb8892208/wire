#ifndef _MENUFUNCTION_H
#define _MENUFUNCTION_H


#define    menu_print_error(format,...)  printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define    menu_print_debug(format,...)	printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)

#define    LCD_SH_PATH         "/my_tools/lcd_info.sh"
#define    MAX_PARAMS        10 
#define    READ_BUFF_LEN     128

typedef enum {
	SSH_ENABLE,
	SSH_DISABLE,
}ssh_onoff_t;

typedef enum {
	NET_WAN,
	NET_LAN,
	NET_TYPE_LEN,
}network_type_t;

typedef enum {
	NET_DHCP,
	NET_STATIC,
	NET_FACTORY,
}network_mode_t;


typedef struct mdata_system_status_s {
	char active_wireless_channels[5];
	char current_calls[5];	
	char memory_usage[20];
	char storage_usage[40];
}mdata_system_status_t;


typedef struct mdata_network_info_s {
	char mode[10];
	char MAC[20];
	char ipaddr[20];
	char mask[20];
	char DNS[20];
}mdata_network_info_t;

typedef struct mdata_device_info_s {
	char ProductName[20];
	char ModelDescription[30];
	char SoftwareVer[20];
	char HardwareVer[20];
	char SystemTime[30];
	char UpTime[30];
}mdata_device_info_t;


typedef struct mdata_web_access_s {
	char Protocol[20];
	char Port[10];
}mdata_web_access_t;

typedef struct menudata_s {
	mdata_system_status_t system_status;
	mdata_network_info_t network_info[NET_TYPE_LEN];
	mdata_device_info_t device_info;
	mdata_web_access_t web_access;
}menudata_t;


extern int menufunc_get_system_status(mdata_system_status_t *status);
extern int menufunc_get_network_info(network_type_t type, mdata_network_info_t *netinfo);
extern int menufunc_get_device_info(mdata_device_info_t *devinfo);
extern int menufunc_get_web_access(mdata_web_access_t *webaccess);
extern int menufunc_get_language_type(void);
extern int menufunc_set_language(int lang);
extern int menufunc_set_ssh_access(ssh_onoff_t onoff);
extern int menufunc_set_networksettings(network_type_t type, network_mode_t mode);
extern int menufunc_system_reboot(void);
extern int menufunc_factory_reset(void);
extern int menufunc_web_reset_password(void);



#endif
