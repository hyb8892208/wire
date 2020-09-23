#ifndef _MENUDISPLAY_H
#define _MENUDISPLAY_H


//#define BMP_LEN               192000
//#define BMP_LEN               77440
#define BMP_PLEN              640
//#define BMP_COUNT             BMP_LEN/BMP_PLEN

//#define LCD_WIDTH            220
//#define LCD_HIGH             176


#define SYS_CLEAR_CMD           "state=4\n"
#define SYS_START_CMD           "state=2\n"
#define SYS_REBOOT_CMD          "state=1\n" 
#define SYS_DRAWBMP_CMD         "draw b %d %d %d %d\n"
#define SYS_DRAWSTR_CMD         "draw s %d %d %d %sh %s\n"
#define SYS_DRAWR_CMD           "draw r %d %d %d %d %sh\n"
#define SYS_DRAWL_CMD           "draw l %d %d %d %d %sh\n"
#define SYS_BACKLIGHT_CMD      "lcd=%d\n"



#define STR_SYSTEM_STA         "System Status"
#define STR_NETWORKINFO        "Network Info"   
#define STR_NETWORKSETTING     "Network Settings"   
#define STR_DEVINFO             "Device Info"  
#define STR_WEBACCESS           "Web Access"  
#define STR_SSHACCESS           "SSH Access"  
#define STR_SYSTEMCTRL          "System Control"  
#define STR_BACK           	  "Back" 
#define STR_WANMODE         	  "WAN MODE"  
#define STR_LANMODE          	  "LAN MODE"  
#define STR_ENABLE         	  "Enable"  
#define STR_DISABLE          	  "Disable"  
#define STR_REBOOT          	  "Reboot"  
#define STR_RESET          	  "Factory Reset" 
#define STR_CONFIRM          	  "Confirm"  
#define STR_CANCEL          	  "Cancel" 
#define STR_SUCCESS          	  "success"
#define STR_DHCP          	  	  "DHCP"  
#define STR_STATIC          	  "Static" 
#define STR_FACTORY          	  "Factory Default"
#define STR_FAIL                 "fail"


#define IMAGE_DATA_PATH        "/etc/led/images/"

#define BMP_BACKGROUND        "background.bin"
#define BMP_MAINMENU          "mainmenu.bin"

#define ICON_NETWORKINFO        "networkinfo.bin"
#define ICON_DEVICEINFO     	  "deviceinfo.bin"
#define ICON_SYSTEMSTA      	  "systemstate.bin"
#define ICON_WEBACCESS          "webaccess.bin"
#define ICON_SSHACCESS          "ios-shuffle-strong.bin"
#define ICON_NETWORKSETTING    "networksetting.bin"
#define ICON_SYSTEMCTRL         "systemctrl.bin"
#define ICON_BACK               "return.bin"
#define ICON_WAN        		 "wan.bin"
#define ICON_LAN                "lan.bin"
#define ICON_ENABLE        	 "right.bin"
#define ICON_DISABLE            "wrong.bin"
#define ICON_REBOOT             "reboot.bin"
#define ICON_RESET             "defaule_group.bin"
#define ICON_RIGHT             "right.bin"
#define ICON_WRONG             "wrong.bin"
#define ICON_OK            		"ok.bin"
#define ICON_DHCP          	"dcdn.bin"  
#define ICON_STATIC          	"static.bin" 
#define ICON_FACTORY          	"chose_default.bin"
#define ICON_CHINESE           "chinese.bin"
#define ICON_ENGLISH          	"english.bin"  
#define ICON_LANGUAGE          "language.bin" 
#define ICON_POWERSAVE         "powersave.bin"
#define ICON_RESETPASS         "resetpassword.bin" 
#define ICON_ACCESS            "access.bin"



#define    dis_print_error(format,...)  printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define    dis_print_debug(format,...)	printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)


#define DEEP_BLUE       "01CF"
#define LIGHT_BULE      "7D7C"


#define SYS_LCD_VERSION         "ver\n"    
#define HW_VER_LABLE             "HwVer : "
#define HW_VER_LEN                20


typedef enum {
	MOPT_STSTEMSTATUS,
	MOPT_NETWORKINFO,
	MOPT_DEVINFO,
	MOPT_WEBACCESS,
	MOPT_SSHACCESS,
	MOPT_NETWORKSETTINGS,
	MOPT_STSTEMCONTROL,
	MOPT_BACK,
	MOPT_ENABLE,
	MOPT_DISABLE,
	MOPT_WANMODE,
	MOPT_LANMODE,
	MOPT_DHCP,
	MOPT_STATIC,
	MOPT_FACTORY,
	MOPT_REBOOT,
	MOPT_RESET,
	MOPT_CONFIRM,
	MOPT_CANCEL,
	MOPT_SUCCESS,
}Menuoptions_t;

typedef enum {
	LCD_OFF = 0,
	LCD_ON = 1,
}lcd_ctrl_t;

extern int menudis_init(int confd);
extern int control_lcd_backlight(lcd_ctrl_t onoff);
extern int read_backgrand_data(void);
extern int menudisplay_setfd(int confd);
extern void display_loading_page(void);
extern void display_mainmenu(key_type_t key_event);
extern void display_mainmenu_1(key_type_t key_event);
extern void display_mainmenu_2(key_type_t key_event);
extern void display_mainmenu_3(key_type_t key_event);
extern void display_mainmenu_4(key_type_t key_event);
extern void display_mainmenu_5(key_type_t key_event);
extern void display_mainmenu_6(key_type_t key_event);
extern void display_mainmenu_7(key_type_t key_event);
extern void display_mainmenu_8(key_type_t key_event);
extern void display_mainmenu_9(key_type_t key_event);
extern void display_mainmenu_10(key_type_t key_event);
extern void display_web_1(key_type_t key_event);
extern void display_web_2(key_type_t key_event);
extern void display_web_3(key_type_t key_event);
extern void display_web_reset_password_1(key_type_t key_event);
extern void display_web_reset_password_2(key_type_t key_event);


extern void display_networksetting(key_type_t key_event);
extern void display_networksetting_1(key_type_t key_event);
extern void display_networksetting_2(key_type_t key_event);
extern void display_networksetting_3(key_type_t key_event);
extern void display_networksetting_wan_1(key_type_t key_event);
extern void display_networksetting_wan_2(key_type_t key_event);
extern void display_networksetting_wan_3(key_type_t key_event);
extern void display_networksetting_wan_4(key_type_t key_event);
extern void display_networksetting_wan_dhcp_1(key_type_t key_event);
extern void display_networksetting_wan_dhcp_2(key_type_t key_event);
extern void display_networksetting_wan_static_1(key_type_t key_event);
extern void display_networksetting_wan_static_2(key_type_t key_event);
extern void display_networksetting_wan_factory_1(key_type_t key_event);
extern void display_networksetting_wan_factory_2(key_type_t key_event);
extern void display_networksetting_lan_1(key_type_t key_event);
extern void display_networksetting_lan_2(key_type_t key_event);
extern void display_networksetting_lan_3(key_type_t key_event);
extern void display_networksetting_lan_4(key_type_t key_event);
extern void display_networksetting_lan_dhcp_1(key_type_t key_event);
extern void display_networksetting_lan_dhcp_2(key_type_t key_event);
extern void display_networksetting_lan_static_1(key_type_t key_event);
extern void display_networksetting_lan_static_2(key_type_t key_event);
extern void display_networksetting_lan_factory_1(key_type_t key_event);
extern void display_networksetting_lan_factory_2(key_type_t key_event);

extern void display_back_confirm(key_type_t key_event);


extern void display_sshaccess_1(key_type_t key_event);
extern void display_sshaccess_2(key_type_t key_event);
extern void display_sshaccess_3(key_type_t key_event);
extern void display_sshaccess_enable_1(key_type_t key_event);
extern void display_sshaccess_enable_2(key_type_t key_event);
extern void display_sshaccess_disable_1(key_type_t key_event);
extern void display_sshaccess_disable_2(key_type_t key_event);


extern void display_systemctrl_1(key_type_t key_event);
extern void display_systemctrl_2(key_type_t key_event);
extern void display_systemctrl_3(key_type_t key_event);
extern void display_systemctrl_reboot_1(key_type_t key_event);
extern void display_systemctrl_reboot_2(key_type_t key_event);
extern void display_systemctrl_reset_1(key_type_t key_event);
extern void display_systemctrl_reset_2(key_type_t key_event);

extern void display_powersave_1(key_type_t key_event);
extern void display_powersave_2(key_type_t key_event);
extern void display_powersave_3(key_type_t key_event);
extern void display_powersave_enable_1(key_type_t key_event);
extern void display_powersave_enable_2(key_type_t key_event);
extern void display_powersave_disable_1(key_type_t key_event);
extern void display_powersave_disable_2(key_type_t key_event);


extern void display_language_1(key_type_t key_event);
extern void display_language_2(key_type_t key_event);
extern void display_language_3(key_type_t key_event);
extern void display_language_chinese_1(key_type_t key_event);
extern void display_language_chinese_2(key_type_t key_event);
extern void display_language_english_1(key_type_t key_event);
extern void display_language_english_2(key_type_t key_event);



extern void display_systemstatus_back(key_type_t key_event);
extern void display_networkinfo_back(key_type_t key_event);
extern void display_deviceinfo_back(key_type_t key_event);
extern void display_webaccess_back(key_type_t key_event);


#endif

