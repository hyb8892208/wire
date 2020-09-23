#ifndef _MENUCONF_H
#define _MENUCONF_H


#define    mconf_print_error(format,...)  printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define    mconf_print_debug(format,...)	printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)


#define   CONTEXT_MENUCONF          "menu"
#define   OPTION_POWERSAVEFLAG      "powersaveflag"
#define   OPTION_POWERSAVETIME      "powersavetime"
#define   OPTION_NOKEYTIME     	   "nokeytime"

#define   MENUCONFPATH                "/etc/led/config/menu.conf"


typedef struct menuconf_s {
	int PowerSaveFlag;                  //省电模式开关标志
	int PowerSaveTime;                  //省电模式关屏时间(默认60s)
	int NokeyTime;                       //无操作恢复信号页面时间(默认20s)
}menuconf_t;


extern int menu_read_conf(menuconf_t *pconf);
extern int menu_reflesh_conf(menuconf_t *pconf);



#endif

