#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "keyboard.h"
#include "menudisplay.h"
#include "menuctrl.h"


menu_ctrl_t menu_ctrl;

typedef void (*Menu_Opera_t)(key_type_t);


Menu_Opera_t g_mainmemu_opera[10] = {display_mainmenu_1, display_mainmenu_2, display_mainmenu_3, display_mainmenu_4, display_mainmenu_5, 
										display_mainmenu_6, display_mainmenu_7, display_mainmenu_8, display_mainmenu_9, display_mainmenu_10};


Menu_Opera_t g_web_opera[3] = {display_web_1, display_web_2, display_web_3};
Menu_Opera_t g_web_reset_password_opera[2] = {display_web_reset_password_1, display_web_reset_password_2};

Menu_Opera_t g_sshaccess_opera[3] = {display_sshaccess_1, display_sshaccess_2, display_sshaccess_3};
Menu_Opera_t g_sshaccess_enable_opera[2] = {display_sshaccess_enable_1, display_sshaccess_enable_2};
Menu_Opera_t g_sshaccess_disable_opera[2] = {display_sshaccess_disable_1, display_sshaccess_disable_2};

Menu_Opera_t g_netset_opera[3] = {display_networksetting_1, display_networksetting_2, display_networksetting_3};
Menu_Opera_t g_networksetting_wan_opera[4] = {display_networksetting_wan_1, display_networksetting_wan_2, display_networksetting_wan_3, display_networksetting_wan_4};
Menu_Opera_t g_networksetting_lan_opera[4] = {display_networksetting_lan_1, display_networksetting_lan_2, display_networksetting_lan_3, display_networksetting_lan_4};

Menu_Opera_t g_networksetting_wan_dhcp_opera[2] = {display_networksetting_wan_dhcp_1, display_networksetting_wan_dhcp_2};
Menu_Opera_t g_networksetting_wan_static_opera[2] = {display_networksetting_wan_static_1, display_networksetting_wan_static_2};
Menu_Opera_t g_networksetting_wan_factory_opera[2] = {display_networksetting_wan_factory_1, display_networksetting_wan_factory_2};
Menu_Opera_t g_networksetting_lan_dhcp_opera[2] = {display_networksetting_lan_dhcp_1, display_networksetting_lan_dhcp_2};
Menu_Opera_t g_networksetting_lan_static_opera[2] = {display_networksetting_lan_static_1, display_networksetting_lan_static_2};
Menu_Opera_t g_networksetting_lan_factory_opera[2] = {display_networksetting_lan_factory_1, display_networksetting_lan_factory_2};

Menu_Opera_t g_sysctrl_opera[3] = {display_systemctrl_1, display_systemctrl_2, display_systemctrl_3};
Menu_Opera_t g_sysctrl_reboot_opera[2] = {display_systemctrl_reboot_1, display_systemctrl_reboot_2};
Menu_Opera_t g_sysctrl_reset_opera[2] = {display_systemctrl_reset_1, display_systemctrl_reset_2};

Menu_Opera_t g_powersave_opera[3] = {display_powersave_1, display_powersave_2, display_powersave_3};
Menu_Opera_t g_powersave_enable_opera[2] = {display_powersave_enable_1, display_powersave_enable_2};
Menu_Opera_t g_powersave_disable_opera[2] = {display_powersave_disable_1, display_powersave_disable_2};

Menu_Opera_t g_language_opera[3] = {display_language_1, display_language_2, display_language_3};
Menu_Opera_t g_language_chinese_opera[2] = {display_language_chinese_1, display_language_chinese_2};
Menu_Opera_t g_language_english_opera[2] = {display_language_english_1, display_language_english_2};


menu_node_t * menu_list_insert(menu_node_t *menunode, void (*opera)(key_type_t), void *showdata)
{
	menu_node_t *pnew = NULL;
	
	if(!menunode) {
		menu_print_error("menunode is NULL.\n");
		return NULL;
	}
	
	pnew = (menu_node_t *)malloc(sizeof(menu_node_t));
	if(!pnew) {
		menu_print_error("note malloc fail.\n");
		return NULL;
	}

	pnew->parent = menunode;
	pnew->next = pnew;
	pnew->prev = pnew;
	pnew->cur_son = NULL;
	pnew->opera = opera;
	pnew->showdata = showdata;
	menunode->cur_son = pnew;
	return pnew;
}

menu_node_t * menu_search_child_list(menu_node_t *menunode, int index)
{
	int i = 0;
	menu_node_t * plist = menunode->cur_son;

	if(!menunode) {
		menu_print_error("menunode is NULL.\n");
		return NULL;
	}

	for(i=0; i<index; i++) {
		plist = plist->next;
		if(!plist) {
			menu_print_error("no child found from 0x%x ,index(%d).\n", menunode, index);
			return NULL;		
		}
	}	

	return plist;
}

int menu_child_list_insert(menu_node_t *menunode, int listlen, void (*opera_arr[])(key_type_t), void **data)
{
	menu_node_t *pnew = NULL;
	menu_node_t *phead = NULL;
	menu_node_t *plist = NULL;
	int i = 0;
	
	if(!menunode) {
		menu_print_error("menunode is NULL.\n");
		return -1;
	}

	phead = (menu_node_t *)malloc(sizeof(menu_node_t));
	if(!phead) {
		menu_print_error("note malloc fail.\n");
		return -1;
	}

	phead->cur_son = NULL;
	phead->opera = opera_arr[0];
	if(data)
		phead->showdata = data[0];
	phead->parent = menunode;
	phead->next = phead;
	phead->prev = phead;
	menunode->cur_son = phead;
	
	plist = phead;	
	for(i=1; i<listlen; i++) {
		pnew = (menu_node_t *)malloc(sizeof(menu_node_t));
		if(!pnew) {
			menu_print_error("note malloc fail.\n");
			return -1;
		}
		pnew->opera = opera_arr[i];
		if(data)
			pnew->showdata = data[i];
		pnew->next = phead;
		pnew->prev = plist;
		phead->prev = pnew;
		plist->next = pnew;
		plist = plist->next;
	}
	
	return 0;
}


int menu_create_systemstatus_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;

	ptmp = menu_search_child_list(pmenu, 0);
	menu_list_insert(ptmp, display_systemstatus_back, NULL);
	return 0;
}

int menu_create_networkinfo_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;

	ptmp = menu_search_child_list(pmenu, 1);
	menu_list_insert(ptmp, display_networkinfo_back, NULL);
	return 0;
}

int menu_create_deviceinfo_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;

	ptmp = menu_search_child_list(pmenu, 2);
	menu_list_insert(ptmp, display_deviceinfo_back, NULL);
	return 0;
}


int menu_create_web_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;
	menu_node_t *pweb;
	menu_node_t *psshaccess_confirm;

	ptmp = menu_search_child_list(pmenu, 3);
	menu_child_list_insert(ptmp, 3, g_web_opera, NULL);

	pweb = menu_search_child_list(ptmp, 0);
	menu_list_insert(pweb, display_webaccess_back, NULL);

	pweb = menu_search_child_list(ptmp, 1);
	menu_child_list_insert(pweb, 2, g_web_reset_password_opera, NULL); 	
	psshaccess_confirm = menu_search_child_list(pweb, 0);
	menu_list_insert(psshaccess_confirm, display_back_confirm, NULL);	

	return 0;
}


int menu_create_sshaccess_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;
	menu_node_t *psshaccess;
	menu_node_t *psshaccess_confirm;

	ptmp = menu_search_child_list(pmenu, 4);
	menu_child_list_insert(ptmp, 3, g_sshaccess_opera, NULL);

	psshaccess = menu_search_child_list(ptmp, 0);
	menu_child_list_insert(psshaccess, 2, g_sshaccess_enable_opera, NULL);	
	psshaccess_confirm = menu_search_child_list(psshaccess, 0);
	menu_list_insert(psshaccess_confirm, display_back_confirm, NULL);

	psshaccess = menu_search_child_list(ptmp, 1);
	menu_child_list_insert(psshaccess, 2, g_sshaccess_disable_opera, NULL); 	
	psshaccess_confirm = menu_search_child_list(psshaccess, 0);
	menu_list_insert(psshaccess_confirm, display_back_confirm, NULL);	

	return 0;
}

int menu_create_networksetting_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;
	menu_node_t *pnetworksetting;
	menu_node_t *pnetworksetting_confirm;	
	menu_node_t *pback_confirm;	

	ptmp = menu_search_child_list(pmenu, 5);
	menu_child_list_insert(ptmp, 3, g_netset_opera, NULL);
	
	pnetworksetting = menu_search_child_list(ptmp, 0);
	menu_child_list_insert(pnetworksetting, 4, g_networksetting_wan_opera, NULL);	
	pnetworksetting_confirm = menu_search_child_list(pnetworksetting, 0);
	menu_child_list_insert(pnetworksetting_confirm, 2, g_networksetting_wan_dhcp_opera, NULL);	
	pback_confirm = menu_search_child_list(pnetworksetting_confirm, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);
	pnetworksetting_confirm = menu_search_child_list(pnetworksetting, 1);
	menu_child_list_insert(pnetworksetting_confirm, 2, g_networksetting_wan_static_opera, NULL);	
	pback_confirm = menu_search_child_list(pnetworksetting_confirm, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);
	pnetworksetting_confirm = menu_search_child_list(pnetworksetting, 2);
	menu_child_list_insert(pnetworksetting_confirm, 2, g_networksetting_wan_factory_opera, NULL);		
	pback_confirm = menu_search_child_list(pnetworksetting_confirm, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);
	
	pnetworksetting = menu_search_child_list(ptmp, 1);
	menu_child_list_insert(pnetworksetting, 4, g_networksetting_lan_opera, NULL);	
	pnetworksetting_confirm = menu_search_child_list(pnetworksetting, 0);
	menu_child_list_insert(pnetworksetting_confirm, 2, g_networksetting_lan_dhcp_opera, NULL);	
	pback_confirm = menu_search_child_list(pnetworksetting_confirm, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);
	pnetworksetting_confirm = menu_search_child_list(pnetworksetting, 1);
	menu_child_list_insert(pnetworksetting_confirm, 2, g_networksetting_lan_static_opera, NULL);	
	pback_confirm = menu_search_child_list(pnetworksetting_confirm, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);	
	pnetworksetting_confirm = menu_search_child_list(pnetworksetting, 2);
	menu_child_list_insert(pnetworksetting_confirm, 2, g_networksetting_lan_factory_opera, NULL);		
	pback_confirm = menu_search_child_list(pnetworksetting_confirm, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);	

	return 0;
}


int menu_create_systemcontrol_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;
	menu_node_t *psysctrl;
	menu_node_t *pback_confirm;

	ptmp = menu_search_child_list(pmenu, 6);
	menu_child_list_insert(ptmp, 3, g_sysctrl_opera, NULL);
	psysctrl = menu_search_child_list(ptmp, 0);
	menu_child_list_insert(psysctrl, 2, g_sysctrl_reboot_opera, NULL);
	pback_confirm =  menu_search_child_list(psysctrl, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);		
	psysctrl = menu_search_child_list(ptmp, 1);
	menu_child_list_insert(psysctrl, 2, g_sysctrl_reset_opera, NULL);	
	pback_confirm =  menu_search_child_list(psysctrl, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);	

	return 0;
}


int menu_create_powersave_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;
	menu_node_t *powersave;
	menu_node_t *pback_confirm;

	ptmp = menu_search_child_list(pmenu, 7);
	menu_child_list_insert(ptmp, 3, g_powersave_opera, NULL);
	powersave = menu_search_child_list(ptmp, 0);
	menu_child_list_insert(powersave, 2, g_powersave_enable_opera, NULL);
	pback_confirm =  menu_search_child_list(powersave, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);		
	powersave = menu_search_child_list(ptmp, 1);
	menu_child_list_insert(powersave, 2, g_powersave_disable_opera, NULL);	
	pback_confirm =  menu_search_child_list(powersave, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);	
	
	return 0;
}


int menu_create_language_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;
	menu_node_t *planguage;
	menu_node_t *pback_confirm;

	ptmp = menu_search_child_list(pmenu, 8);
	menu_child_list_insert(ptmp, 3, g_language_opera, NULL);
	planguage = menu_search_child_list(ptmp, 0);
	menu_child_list_insert(planguage, 2, g_language_english_opera, NULL);
	pback_confirm =  menu_search_child_list(planguage, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);		
	planguage = menu_search_child_list(ptmp, 1);
	menu_child_list_insert(planguage, 2, g_language_chinese_opera, NULL);	
	pback_confirm =  menu_search_child_list(planguage, 0);
	menu_list_insert(pback_confirm, display_back_confirm, NULL);	
	
	return 0;
}

menu_node_t * menu_create_mainmenu_list(menu_node_t *pmenu)
{
	menu_node_t *ptmp;	
	
	ptmp = menu_list_insert(pmenu, display_mainmenu, NULL);
	menu_child_list_insert(ptmp, 10, g_mainmemu_opera, NULL);

	return ptmp;
}

int menu_create_lcdmenu_list(menu_node_t *phead)
{
	menu_node_t *pmenu;
	menu_node_t *ptmp = phead;	

	if(!phead) {
		return -1;
	}

	pmenu = menu_create_mainmenu_list(ptmp);
	menu_create_systemstatus_list(pmenu);
	menu_create_networkinfo_list(pmenu);
	menu_create_deviceinfo_list(pmenu);
	menu_create_web_list(pmenu);
	menu_create_sshaccess_list(pmenu);
	menu_create_networksetting_list(pmenu);
	menu_create_systemcontrol_list(pmenu);
	menu_create_powersave_list(pmenu);
	menu_create_language_list(pmenu);
	
	return 0;
}

int menu_headnode_init(menu_node_t *phead)
{
	menu_node_t *pmenu = phead;

	if(pmenu) {	
		pmenu->parent = NULL;
		pmenu->cur_son = NULL;
		pmenu->next = pmenu;
		pmenu->prev = pmenu;
		pmenu->opera = display_loading_page;
		pmenu->soncount = 0;
	}	
	return 0;
}

int menu_list_init(menu_node_t *phead)
{
	if(!phead)
		return -1;
	
	menu_headnode_init(phead);
	menu_create_lcdmenu_list(phead);                 
	return 0;
}

int menu_display_param_init(menu_display_t *pMdisplay, Hardwaremodel_t LcdType)
{
	int i = 0;
	
	if(!pMdisplay) {
		return -1;
	}

	if(LcdType == LCD_JTKJ2_0)
	{
		pMdisplay->fontsize = 16;      //
		pMdisplay->memu_length = 220;  //
		pMdisplay->memu_width = 176;   
		pMdisplay->box_x = 20;          //
		pMdisplay->box_lenth = 170;     //
		pMdisplay->box_width = 30;
		pMdisplay->box_diff = (pMdisplay->box_width - pMdisplay->fontsize)/2;
		for(i=0; i<MAX_SIGLE_PAGE_MENU; i++) {
			pMdisplay->box_y[i] = i*38+28;
		}
	} else {
		pMdisplay->fontsize = 24;
		pMdisplay->memu_length = 400;
		pMdisplay->memu_width = 240;
		pMdisplay->box_x = 40;
		pMdisplay->box_lenth = 240;
		pMdisplay->box_width = 30;
		pMdisplay->box_diff = (pMdisplay->box_width - pMdisplay->fontsize)/2;
		for(i=0; i<MAX_SIGLE_PAGE_MENU; i++) {
			pMdisplay->box_y[i] = i*48+57;
		}		
	}
	return 0;
}

int menu_init(void) 
{
	menu_ctrl_t *pctrl = &menu_ctrl;
	
	pctrl->NokeyCnt = 0;
	pctrl->MenuState = 0;
	pctrl->PowerSaveCnt = 0;
	pctrl->LangCnt = 0;
	pctrl->LangTime = CHECK_LANG_TIME*1000000/SCAN_CYCLE;
	
	menu_display_param_init(&pctrl->Mdisplay, pctrl->LcdType);
	
	menu_read_conf(&pctrl->MenuConf);
	read_backgrand_data();

	menu_list_init(&pctrl->MenuHead);
}

void *menu_ctrl_prog(void *data)
{
	key_type_t key_event;
	menu_language_t lang = LANG_INVALID;
	menu_language_t new_lang = LANG_INVALID;
	menu_ctrl_t *pmenuctrl = (menu_ctrl_t *)data;
	menu_node_t *pnodectrl = &pmenuctrl->MenuHead;
	menu_node_t *ptmp = &pmenuctrl->MenuHead;

	pnodectrl = pnodectrl->cur_son;
	while(1) {
		pmenuctrl->NokeyCnt++;
		pmenuctrl->PowerSaveCnt++;
		pmenuctrl->LangCnt++;

		if(pmenuctrl->LangCnt > pmenuctrl->LangTime) {
			pmenuctrl->LangCnt = 0;
			new_lang = (menu_language_t)menufunc_get_language_type();
			if((lang != new_lang) && (lang != LANG_INVALID)) {
				menulang_read_data(new_lang, &pmenuctrl->Mlangdata);
				pmenuctrl->language = new_lang;
				pnodectrl = ptmp->cur_son;
				if(pnodectrl && (pmenuctrl->MenuState == 1)) {
					if(pnodectrl->opera)
						pnodectrl->opera(KEY_OK);
					if(pnodectrl->cur_son) {
						pnodectrl = pnodectrl->cur_son;
					}
				}					
			}
			lang = new_lang;
		}
		
		key_event = get_keyboard_event();
		if(key_event == KEY_OK) {
			pmenuctrl->NokeyCnt = 0;
			pmenuctrl->PowerSaveCnt = 0;
			if(pmenuctrl->LcdState == LCD_OFF) {
				control_lcd_backlight(LCD_ON);
			}
			if(pnodectrl) {
				if(pnodectrl->opera == display_back_confirm) {
					pnodectrl->opera(KEY_OK);
					if(pnodectrl->parent)
						pnodectrl = pnodectrl->parent->next;
				}		
				if(pnodectrl->opera)
					pnodectrl->opera(key_event);
				if(pnodectrl->cur_son) {
					pnodectrl = pnodectrl->cur_son;
				} else {
					if(pnodectrl->next)
						pnodectrl = pnodectrl->next->parent;
				}
			}		
		} else if(key_event == KEY_UP) {
			pmenuctrl->NokeyCnt = 0;
			pmenuctrl->PowerSaveCnt = 0;
			if(pmenuctrl->LcdState == LCD_OFF) {
				control_lcd_backlight(LCD_ON);
			}
			if(pnodectrl) {
				if(pnodectrl->opera)
					pnodectrl->opera(key_event);
				
				if(pnodectrl->prev) {
					pnodectrl = pnodectrl->prev;
				} 				
				if((pnodectrl == ptmp->cur_son) || (pnodectrl == ptmp)) {
					pnodectrl = pnodectrl->cur_son;
				}
			}		
		} else if(key_event == KEY_DOWN) {
			pmenuctrl->NokeyCnt = 0;
			pmenuctrl->PowerSaveCnt = 0;
			if(pmenuctrl->LcdState == LCD_OFF) {
				control_lcd_backlight(LCD_ON);
			}
			if(pnodectrl) {
				if(pnodectrl->opera)
					pnodectrl->opera(key_event);
				if(pnodectrl->next) 
					pnodectrl = pnodectrl->next;
				if((pnodectrl == ptmp->cur_son) || (pnodectrl == ptmp)) {
					pnodectrl = pnodectrl->cur_son;
				}
			}				
		}	

		if((pmenuctrl->NokeyCnt > pmenuctrl->MenuConf.NokeyTime) && (pmenuctrl->LcdState == LCD_ON)) {
			pmenuctrl->NokeyCnt = 0;
			if(pmenuctrl->MenuState) {
				pmenuctrl->MenuState = 0;
				pnodectrl = ptmp->cur_son;
			}
		}	
		
		if((pmenuctrl->PowerSaveCnt > pmenuctrl->MenuConf.PowerSaveTime) && (pmenuctrl->LcdState == LCD_ON)) {
			pmenuctrl->PowerSaveCnt = 0;
			if(!pmenuctrl->MenuConf.PowerSaveFlag) {			
				control_lcd_backlight(LCD_OFF);
				pnodectrl = ptmp;
			}
		}
		
		usleep(SCAN_CYCLE);
	}

	return NULL;
}


int menu_ctrl_pthread_create(void)
{
	pthread_t menu_tid;

	menu_init();

	led_pthread_create(&menu_tid, NULL, menu_ctrl_prog, &menu_ctrl);
	return 0;	
}







