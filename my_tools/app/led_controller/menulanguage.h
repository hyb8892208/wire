#ifndef _MENULANGUAGE_H
#define _MENULANGUAGE_H


#define    mlang_print_error(format,...)  printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define    mlang_print_debug(format,...)	printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)

#define LINE_SIZE                   1024
#define NAME_SIZE                   128

#define CONTEXT_MAIN_MENU        "main_menu"
#define CONTEXT_SYS_STA          "system_status"
#define CONTEXT_NET_INFO         "network_info"
#define CONTEXT_NET_SET          "network_settings"
#define CONTEXT_SYS_CTRL         "system_control"
#define CONTEXT_NET_MODE         "network_mode"
#define CONTEXT_DEV_INFO         "device_info"
#define CONTEXT_WEB_ACCESS       "web_access"
#define CONTEXT_LANGUAGE         "language"
#define CONTEXT_CONFIRM           "confirm"
#define CONTEXT_SWITCH            "switch"
#define CONTEXT_CONTROL_RES      "control_result"
#define CONTEXT_WEB      			"web"



#define MENU_LANG_CN_PATH       "/etc/led/lang/chinese"
#define MENU_LANG_EN_PATH       "/etc/led/lang/english"



typedef enum menu_language_s{
	LANG_CN,              //ÖÐÎÄ
	LANG_EN,               //Ó¢ÎÄ
	LANG_INVALID,
}menu_language_t;


typedef struct mlangdata_main_menu_s {
	char Menu[10];
	char SystemStatus[20];
	char NetworkInfo[20];
	char DeviceInfo[20];
	char WebAccess[20];
	char SSHAccess[20];
	char NetworkSettings[20];
	char SystemControl[20];
	char PowerSave[20];
	char Language[10];
	char Back[10];
}mlangdata_main_menu_t;


typedef struct mlangdata_system_status_s {
	char ActiveWirelessChannels[40];
	char ConcurrentCalls[40];
	char StorageUsage[20];
	char MemoryUsage[20];
}mlangdata_system_status_t;


typedef struct mlangdata_web_s {
	char Access[20];
	char ResetPassword[40];
}mlangdata_web_t;


typedef struct mlangdata_network_info_s {
	char Wan[10];
	char Lan[10];
	char Mode[10];
	char Mac[10];
	char IP[10];
	char Mask[20];
	char DNS[10];
}mlangdata_network_info_t;


typedef struct mlangdata_network_settings_s {
	char WanMode[20];
	char LanMode[20];
}mlangdata_network_settings_t;


typedef struct mlangdata_system_control_s {
	char Reboot[10];
	char FactoryReset[20];
}mlangdata_system_control_t;


typedef struct mlangdata_network_mode_s {
	char DHCP[10];
	char Static[10];
	char Factory[20];
	char Disable[20];	
}mlangdata_network_mode_t;


typedef struct mlangdata_device_info_s {
	char ProductName[20];
	char ModelDescription[40];
	char SoftwareVersion[40];
	char HardwareVersion[40];
	char SystemTime[20];
	char UpTime[20];
}mlangdata_device_info_t;


typedef struct mlangdata_web_access_s {
	char Protocol[20];
	char Port[20];
}mlangdata_web_access_t;



typedef struct mlangdata_language_s {
	char english[10];
	char chinese[10];
}mlangdata_language_t;



typedef struct mlangdata_confirm_s {
	char Confirm[20];
	char Cancel[20];
}mlangdata_confirm_t;



typedef struct mlangdata_onoff_s {
	char Enable[10];
	char Disable[10];
}mlangdata_onoff_t;



typedef struct mlangdata_control_result_s {
	char Success[10];
	char Fail[10];	
}mlangdata_control_result_t;


typedef struct mlangdata_s {
	mlangdata_main_menu_t main_menu;
	mlangdata_system_status_t system_status;
	mlangdata_network_info_t network_info;
	mlangdata_network_settings_t network_settings;
	mlangdata_system_control_t system_control;
	mlangdata_network_mode_t network_mode;
	mlangdata_device_info_t device_info;
	mlangdata_web_access_t web_access;
	mlangdata_language_t language;
	mlangdata_confirm_t confirm;
	mlangdata_onoff_t onoff;
	mlangdata_control_result_t control_result;
	mlangdata_web_t web;
}mlangdata_t;


extern int menulang_read_data(menu_language_t lang, mlangdata_t *plang);


#endif



