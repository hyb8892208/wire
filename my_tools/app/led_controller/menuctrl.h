#ifndef _MENUCTRL_H
#define _MENUCTRL_H

#include <pthread.h>

#include "menufunction.h"
#include "menulanguage.h"
#include "menuconf.h"


#define    menu_print_error(format,...)  printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define    menu_print_debug(format,...)	printf("[%s: %d] "format, __FILE__, __LINE__, ##__VA_ARGS__)


#define   NO_KEY_TIME      20         //若20S内未操作任何按钮，则返回到主界面
#define   SCAN_CYCLE       100000    //扫描周期100ms
#define   NO_KEY_CNT       NO_KEY_TIME*1000000/SCAN_CYCLE

#define   DORM_TIME        60       //若60S内未操作任何按钮，关闭LCD
#define   DORM_CNT         DORM_TIME*1000000/SCAN_CYCLE

#define   CHECK_LANG_TIME  2
#define   LANG_CNT          CHECK_LANG_TIME*1000000/SCAN_CYCLE


#define   MAX_SIGLE_PAGE_MENU   4

typedef enum {
	LCD_JTKJ2_0,
	LCD_TFT3P4
}Hardwaremodel_t;



typedef struct _show_data_s
{
	char data[100];
}show_data_t;


typedef struct menu_node_s
{
	struct menu_node_s *parent;
	struct menu_node_s *next;
	struct menu_node_s *prev;
	struct menu_node_s *cur_son;
	unsigned char soncount;
	void (*opera)(key_type_t); 
	void *showdata;
}menu_node_t;


typedef struct menu_display_s
{
	int box_y[MAX_SIGLE_PAGE_MENU];
	int box_x;
	int box_lenth;
	int box_width;
	int box_diff;
	int memu_length;
	int memu_width;
	int fontsize;
}menu_display_t;


typedef struct menu_ctrl_s
{
	int PowerSaveCnt;
	int NokeyCnt;
	int LangTime;
	int LangCnt;
	int MenuState;
	int LcdState;
	menu_language_t language;
	Hardwaremodel_t LcdType;
	pthread_mutex_t lock;
	menuconf_t MenuConf;
	menu_node_t MenuHead;
	menudata_t MenuData;
	mlangdata_t Mlangdata;
	menu_display_t Mdisplay;
}menu_ctrl_t;


extern menu_ctrl_t menu_ctrl;
extern int menu_ctrl_pthread_create(void);


#endif











