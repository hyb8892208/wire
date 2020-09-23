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
	int PowerSaveFlag;                  //ʡ��ģʽ���ر�־
	int PowerSaveTime;                  //ʡ��ģʽ����ʱ��(Ĭ��60s)
	int NokeyTime;                       //�޲����ָ��ź�ҳ��ʱ��(Ĭ��20s)
}menuconf_t;


extern int menu_read_conf(menuconf_t *pconf);
extern int menu_reflesh_conf(menuconf_t *pconf);



#endif

