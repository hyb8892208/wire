#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "keyboard.h"
#include "ledhdl.h"
#include "menudisplay.h"
#include "menuctrl.h"


int g_fd_dis = -1;
unsigned char * gImage_background = NULL;
extern int sys_state_output(char *result);


int menudisplay_init(int confd)
{
	g_fd_dis = confd;
	pthread_mutex_init(&menu_ctrl.lock, NULL);
	control_lcd_backlight(LCD_ON);
	menu_ctrl.language = (menu_language_t)menufunc_get_language_type();
	menulang_read_data(menu_ctrl.language, &menu_ctrl.Mlangdata);
	get_lcd_type((unsigned int * )&menu_ctrl.LcdType);
	return 0;
}

int get_lcd_type(unsigned int * LcdType)
{
	char buff[1024] = {0};
	char HwVer[HW_VER_LEN] = {0};
	char *HwVerLable = HW_VER_LABLE;
	int res = 0;
	char *p = NULL;
	int i = 0;
	
	__send_control_cmd(SYS_LCD_VERSION);
	usleep(100000);
	res = com_read(g_fd_dis, buff, sizeof(buff));
	if(res < 0) {
		printf("get lcd info fail (%d)\n", res);
		return -1;
	}
	p = strstr(buff, HwVerLable);
	if(p) {
		p+=strlen(HwVerLable);
		while(*p!='\n' && (i<HW_VER_LEN))
			HwVer[i++] = *p++;
		HwVer[i] = '\0';
	}
	if(!strcmp(HwVer, "V2.1")) {
		*LcdType = 0;           //	LCD_JTKJ2_0,
	} else if (!strcmp(HwVer, "V2.0")) {
		*LcdType = 1;           //	LCD_TFT3P4 
	} 
	return 0;
}

int menudisplay_setfd(int confd)
{
	g_fd_dis = confd;
	return 0;
}

int __send_control_cmd(char *buf) {
	int ret = 0;

	if(!buf) {
		return -1;
	}

	pthread_mutex_lock(&menu_ctrl.lock);	
	ret = com_write(g_fd_dis, buf, strlen(buf));
	pthread_mutex_unlock(&menu_ctrl.lock);
	return 0;
}

int send_bmp_date(char *buf, int len) {
	int res = 0;
	
	if(!buf) {
		return -1;
	}

	pthread_mutex_lock(&menu_ctrl.lock);	
	res = com_write(g_fd_dis, buf, len);
	pthread_mutex_unlock(&menu_ctrl.lock);		
	if(res != len) {
		printf("bmp send fail (%d)\n", res);
	}
	usleep(2000);
	return 0;
}


FILE * open_bmp_file(char *filename)
{
	FILE * fp;
	
	if(!filename) {
		return NULL;
	}
		
	if( NULL == (fp=fopen(filename,"r")) ) {
		dis_print_error("open %s fail.\n", filename);
		return NULL;
	}

	return fp;
}

int close_bmp_file(FILE * fp)
{
	if(!fp) {
		return -1;
	}
		
	fclose(fp);
	return 0;
}

int read_bmp_frame(FILE * fp, char *outbuf, int inlen)
{
	char readbuf[BMP_PLEN] = {0};
	
	if(!fp || !outbuf) {
		return -1;
	}

	if(inlen > BMP_PLEN) {
		return -1;
	}

	if(!fread(readbuf, 1, inlen, fp)) {	
		dis_print_debug("read bmp buff end\n");
		return 1;
	}	
	memcpy(outbuf, readbuf, inlen);	
	return 0;	
}


int read_backgrand_data(void)
{
	FILE * fp = NULL;
	unsigned char * readbuf = NULL;
	char image_path[128] = {0};
	int i = 0;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int bmp_len = p->memu_length*p->memu_width*2;
	
	readbuf = (char *) malloc(bmp_len);
	if(!readbuf) {
		dis_print_error("malloc fail.\n");
		return -1;
	}
	gImage_background = readbuf;

	if(menu_ctrl.LcdType == LCD_JTKJ2_0) {
		sprintf(image_path, "%s%s%s", IMAGE_DATA_PATH, "JTKJ-2.0/", BMP_BACKGROUND);
	} if(menu_ctrl.LcdType == LCD_TFT3P4) {
		sprintf(image_path, "%s%s%s", IMAGE_DATA_PATH, "TFT3P4/", BMP_BACKGROUND);
	}
	
	fp = open_bmp_file(image_path);
	if(!fp) {
		dis_print_error("open %s fail.\n", image_path);
		return -1;
	}

	for(i=0; i<(bmp_len/BMP_PLEN); i++)
		read_bmp_frame(fp, readbuf+i*BMP_PLEN, BMP_PLEN);

	close_bmp_file(fp);
}

int draw_picture(int x, int y, int w, int h, char *filename)
{
	FILE * fp = NULL;
	char buf[128] = {0};
	char bmpbuf[BMP_PLEN] = {0};
	char image_path[128] = {0};
	int bufflen = 0;
	int total_frame = 0;
	int remnain_len = 0;
	int i = 0;
	int res = -1;

	if(!filename) {
		return -1;
	}

	if(menu_ctrl.LcdType == LCD_JTKJ2_0) {
		sprintf(image_path, "%s%s%s", IMAGE_DATA_PATH, "JTKJ-2.0/", filename);
	} if(menu_ctrl.LcdType == LCD_TFT3P4) {
		sprintf(image_path, "%s%s%s", IMAGE_DATA_PATH, "TFT3P4/", filename);
	}
//	sprintf(image_path, "%s%s%s", IMAGE_DATA_PATH, filename);

	bufflen = w*h*2;
	total_frame = bufflen/BMP_PLEN;
	remnain_len = bufflen%BMP_PLEN;

	sprintf(buf, SYS_DRAWBMP_CMD, x, y, w, h);
	__send_control_cmd(buf);
	usleep(5000);	
	
	fp = open_bmp_file(image_path);
	if(!fp) {
		dis_print_error("open %s fail.\n", image_path);
		return -1;
	}
	
	for(i=0; i< total_frame; i++) {
		read_bmp_frame(fp, bmpbuf, BMP_PLEN);
		send_bmp_date(bmpbuf, BMP_PLEN);
		memset(bmpbuf, 0, sizeof(bmpbuf));
	}
	if(remnain_len > 0) {
		read_bmp_frame(fp, bmpbuf, remnain_len);
		send_bmp_date(bmpbuf, remnain_len);		
	}
	close_bmp_file(fp);
	return 0;
}


int draw_picture_data(int x, int y, int w, int h, char *data)
{
	FILE * fp = NULL;
	char buf[128] = {0};
	char *bmpbuf = data;
	int bufflen = 0;
	int total_frame = 0;
	int remnain_len = 0;
	int i = 0;
	int res = -1;

	if(!bmpbuf) {
		return -1;
	}
	
	bufflen = w*h*2;
	total_frame = bufflen/BMP_PLEN;
	remnain_len = bufflen%BMP_PLEN;

	sprintf(buf, SYS_DRAWBMP_CMD, x, y, w, h);
	__send_control_cmd(buf);
	usleep(5000);	
	
	for(i=0; i< total_frame; i++) {
		send_bmp_date(bmpbuf+i*BMP_PLEN, BMP_PLEN);
	}
	if(remnain_len > 0) {
		send_bmp_date(bmpbuf+i*BMP_PLEN, remnain_len);		
	}
	return 0;
}


int draw_string(int x, int y, int font_size, char *color, char *draw_str)
{
	char buf[64] = {0};

	sprintf(buf, SYS_DRAWSTR_CMD, x, y, font_size, color, draw_str);
	__send_control_cmd(buf);
	usleep(10000);
	return 0;	

}

int draw_line(int x1, int y1, int x2, int y2, char * color)
{
	char buf[64] = {0};

	sprintf(buf, SYS_DRAWL_CMD, x1, y1, x2, y2, color);
	__send_control_cmd(buf);
	return 0;	
}

int draw_rectangular(int x, int y, int w, int h, char *color)
{
	char buf[64] = {0};

	draw_line(x, y, x+w, y, color);
	draw_line(x, y, x, y+h, color);
	draw_line(x, y+h, x+w, y+h, color);
	draw_line(x+w, y, x+w, y+h, color);
	
	return 0;	
}

int clear_screen(void)
{
	char buf[64] = {0};

	sprintf(buf, SYS_CLEAR_CMD);
	__send_control_cmd(buf);
	menu_ctrl.MenuState = 1;
	usleep(800000);
	return 0;
}

int control_lcd_backlight(lcd_ctrl_t onoff)
{
	char buf[64] = {0};

	sprintf(buf, SYS_BACKLIGHT_CMD, onoff);
	__send_control_cmd(buf);
	menu_ctrl.LcdState = onoff;
	return 0;
}


int clear_bmp_block(int x, int y, int w, int h)
{
	int i; 
	int wlen = w*2;
	int xlen = x*2;
	int hlen = h*2;
	int ylen = y*2;
	Hardwaremodel_t lcdtype = menu_ctrl.LcdType;
	menu_display_t *p_mdis = &menu_ctrl.Mdisplay;
	unsigned char *p = gImage_background;
	unsigned char *buff = (unsigned char *)malloc(w*h*2+1);
//	unsigned char *buff = (unsigned char *)malloc(p_mdis->memu_length*p_mdis->memu_width*2+1);


	if(!buff || !p) {
		dis_print_error("input fail.\n");
		return -1;
	}
	
	memset(buff, 0, w*h*2);
	
	if(lcdtype == LCD_TFT3P4) {
		p += y*p_mdis->memu_length*2 + xlen; 		
		for(i=0; i<h; i++) { 
	      p += p_mdis->memu_length*2; 
		  memcpy(buff+i*wlen, p, wlen);
	   	} 
	} else if(lcdtype == LCD_JTKJ2_0) {
		p += x*p_mdis->memu_width*2 + ylen;		
		for(i=0; i<w; i++) { 
		  p += p_mdis->memu_width*2; 
		  memcpy(buff+i*hlen, p, hlen);
		} 
	}
	
	draw_picture_data(x, y, w, h, buff);
	free(buff);
	buff = NULL;
	
	return 0;
}

#if 0
int clear_bmp_block_vertical(int x, int y, int w, int h)
{
	int i; 
	int hlen = h*2;
	int ylen = y*2;
	unsigned char *p = gImage_background;
	unsigned char *buff = (unsigned char *)malloc(w*h*2+1);

	if(!buff || !p) {
		dis_print_error("input fail.\n");
		return -1;
	}
	
	memset(buff, 0, w*h*2);

	p += x*LCD_HIGH*2 + ylen; 		
	for(i=0; i<w; i++) { 
      p += LCD_HIGH*2; 
	  memcpy(buff+i*hlen, p, hlen);
   	} 
	draw_picture_data(x, y, w, h, buff);
	free(buff);
	buff = NULL;
	
	return 0;
}
#endif

void display_loading_page(void)
{
	char buf[128] = {0};
	int ret = -1;

	menu_ctrl.MenuState = 0;

	ret = sys_state_output(buf);
	if ( ret < 0 ) {
		printf("get date error\n");
	}	
	__send_control_cmd(buf);

	return ;	
}

void display_systembooting_page(void)
{
	char buf[128] = {0};

	sprintf(buf, SYS_START_CMD);
	__send_control_cmd(buf);
}


void display_mainmenu(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;
		
	clear_screen(); 	
	draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
	draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SYSTEMSTA);
	draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKINFO);
	draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DEVICEINFO); 
	draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_WEBACCESS);	
	draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	draw_string(p->box_x+40, p->box_y[0]+p->box_diff,  p->fontsize, "0", plang_mainmenu->SystemStatus);
	draw_string(p->box_x+40, p->box_y[1]+p->box_diff,  p->fontsize, "0", plang_mainmenu->NetworkInfo);
	draw_string(p->box_x+40, p->box_y[2]+p->box_diff,  p->fontsize, "0", plang_mainmenu->DeviceInfo);
	draw_string(p->box_x+40, p->box_y[3]+p->box_diff,  p->fontsize, "0", plang_mainmenu->WebAccess);

	return ;
}


void display_systemstatus(void)
{
	mdata_system_status_t *syssta = &menu_ctrl.MenuData.system_status;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_system_status_t *plang_syssta = &menu_ctrl.Mlangdata.system_status;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int pos_y = p->box_y[0]+20;
	char buff[128] = {0};
	int buff_len = 0;

	menufunc_get_system_status(syssta);

	
//	draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//	draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//	draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->SystemStatus);
	sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->SystemStatus);
	draw_string(p->box_x, 4, p->fontsize, "0", buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_syssta->ActiveWirelessChannels, syssta->active_wireless_channels);
	draw_string(5, pos_y, p->fontsize, "0", buff);	
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_syssta->ConcurrentCalls, syssta->current_calls);
	draw_string(5, pos_y+p->fontsize+4, p->fontsize, "0", buff);

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_syssta->StorageUsage, syssta->storage_usage);
	buff_len = strlen(buff) * p->fontsize/2 + 5;
	if(buff_len > p->memu_length) {
		draw_string(5, pos_y+2*(p->fontsize+4), p->fontsize, "0", buff);
		draw_string(5, pos_y+3*(p->fontsize+4), p->fontsize, "0", buff+((p->memu_length-5)*2/p->fontsize));
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%s: %s", plang_syssta->MemoryUsage, syssta->memory_usage);
		draw_string(5, pos_y+4*(p->fontsize+4), p->fontsize, "0", buff);		
	} else {
		draw_string(5, pos_y+2*(p->fontsize+4), p->fontsize, "0", buff);
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%s: %s", plang_syssta->MemoryUsage, syssta->memory_usage);
		draw_string(5, pos_y+3*(p->fontsize+4), p->fontsize, "0", buff);
	}
	return;
}

void display_networkinfo(network_type_t type)
{
	mdata_network_info_t *p_netinfo =NULL;
	mlangdata_network_info_t *plang_netinfo = &menu_ctrl.Mlangdata.network_info;
	char buff[128] = {0};
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int pos_y = p->box_y[0]+13;

	if(type >= NET_TYPE_LEN) {
		return;
	}
	p_netinfo =  &menu_ctrl.MenuData.network_info[type];

	menufunc_get_network_info(type, p_netinfo);

	draw_string(5, pos_y, p->fontsize, "0", (type==NET_WAN)?plang_netinfo->Wan:plang_netinfo->Lan);
	sprintf(buff, "%s: %s", plang_netinfo->Mode, p_netinfo->mode);	
	draw_string(5, pos_y+p->fontsize+1, p->fontsize, "0", buff);	
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_netinfo->Mac,p_netinfo->MAC);	
	draw_string(5, pos_y+2*(p->fontsize+1), p->fontsize, "0", buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_netinfo->IP, p_netinfo->ipaddr);
	draw_string(5, pos_y+3*(p->fontsize+1), p->fontsize, "0", buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_netinfo->Mask, p_netinfo->mask);	
	draw_string(5, pos_y+4*(p->fontsize+1), p->fontsize, "0", buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_netinfo->DNS, p_netinfo->DNS);		
	draw_string(5, pos_y+5*(p->fontsize+1), p->fontsize, "0", buff);
	draw_string(p->memu_length-p->fontsize*3, pos_y+5*(p->fontsize+1), p->fontsize, "0", (type==NET_WAN)?"2/2":"1/2");

	return;
}

void display_deviceinfo(void)
{
	mdata_device_info_t *p_devinfo = &menu_ctrl.MenuData.device_info;
	mlangdata_device_info_t *plang_devinfo = &menu_ctrl.Mlangdata.device_info;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	char buff[128] = {0};
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int pos_y = p->box_y[0]+3;	

	menufunc_get_device_info(p_devinfo);

//	draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//	draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//	draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->DeviceInfo);
	sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->DeviceInfo);
	draw_string(p->box_x, 4, p->fontsize, "0", buff);	
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_devinfo->ProductName, p_devinfo->ProductName);		
	draw_string(5, pos_y, p->fontsize, "0", buff);	
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: ", plang_devinfo->ModelDescription);	
	draw_string(5, pos_y+p->fontsize+1, p->fontsize, "0", buff); 	
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s", p_devinfo->ModelDescription);	
	draw_string(5, pos_y+2*(p->fontsize+1), p->fontsize, "0", buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_devinfo->SoftwareVersion, p_devinfo->SoftwareVer);	
	draw_string(5, pos_y+3*(p->fontsize+1), p->fontsize, "0", buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_devinfo->HardwareVersion, p_devinfo->HardwareVer);		
	draw_string(5, pos_y+4*(p->fontsize+1), p->fontsize, "0", buff);
	memset(buff, 0, sizeof(buff));
	if(menu_ctrl.LcdType == LCD_TFT3P4) {
		sprintf(buff, "%s: %s", plang_devinfo->SystemTime, p_devinfo->SystemTime);		
		draw_string(5, pos_y+5*(p->fontsize+1), p->fontsize, "0", buff);	
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%s: %s", plang_devinfo->UpTime, p_devinfo->UpTime);		
		draw_string(5, pos_y+6*(p->fontsize+1), p->fontsize, "0", buff);	
	} else if(menu_ctrl.LcdType == LCD_JTKJ2_0) {
		sprintf(buff, "%s: ", plang_devinfo->SystemTime);		
		draw_string(5, pos_y+5*(p->fontsize+1), p->fontsize, "0", buff);	
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%s", p_devinfo->SystemTime);		
		draw_string(5, pos_y+6*(p->fontsize+1), p->fontsize, "0", buff);	
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%s: %s", plang_devinfo->UpTime, p_devinfo->UpTime);		
		draw_string(5, pos_y+7*(p->fontsize+1), p->fontsize, "0", buff);	
	}
	return;
}

void display_webaccess(void)
{
	mdata_web_access_t *p_webacc = &menu_ctrl.MenuData.web_access;
	mlangdata_web_access_t *plang_webacc = &menu_ctrl.Mlangdata.web_access;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;	
	char buff[128] = {0};
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int pos_y = p->box_y[0]+3;


	menufunc_get_web_access(p_webacc);
	
//	draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//	draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//	draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->WebAccess);	
	sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->WebAccess);
	draw_string(p->box_x, 4, p->fontsize, "0", buff);	
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s: %s", plang_webacc->Protocol, p_webacc->Protocol);		
	draw_string(p->memu_length/5, p->memu_width/2,  p->fontsize, "0", buff);
	memset(buff, 0, sizeof(buff));	
	sprintf(buff, "%s: %s", plang_webacc->Port, p_webacc->Port);	
	draw_string(p->memu_length/5, p->memu_width/2+30,  p->fontsize, "0", buff);	

	return;
}

void display_mainmenu_1(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SYSTEMSTA);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemStatus);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_LANGUAGE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->Language);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		display_systemstatus();
	}
	return ;
}

void display_mainmenu_2(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;	
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1],  p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKINFO);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkInfo);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKINFO);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkInfo);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->NetworkInfo);
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkInfo);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);		
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		display_networkinfo(NET_LAN);
	}
	return ;
}


void display_mainmenu_3(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu; 
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DEVICEINFO);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->DeviceInfo);
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DEVICEINFO);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->DeviceInfo);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		display_deviceinfo();
	}
	return ;
}

void display_mainmenu_4(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu; 
	mlangdata_onoff_t *ponoff = &menu_ctrl.Mlangdata.onoff;
	mlangdata_web_t *pweb = &menu_ctrl.Mlangdata.web;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SSHACCESS);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKSETTING);		
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_SYSTEMCTRL);			
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_POWERSAVE);		
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SSHAccess);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkSettings);		
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemControl);			
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->PowerSave);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[3], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_WEBACCESS);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->WebAccess);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ACCESS);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESETPASS);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", pweb->Access);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", pweb->ResetPassword);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->WebAccess);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);	
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->WebAccess);		
	}
	return ;
}


void display_mainmenu_5(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_onoff_t *ponoff = &menu_ctrl.Mlangdata.onoff;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SSHAccess);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SSHACCESS);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);

	} else if(key_event == KEY_UP) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SYSTEMSTA);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKINFO);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DEVICEINFO);	
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_WEBACCESS);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemStatus);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkInfo);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->DeviceInfo);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->WebAccess);
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0],  p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENABLE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", ponoff->Enable);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", ponoff->Disable);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->SSHAccess);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);	
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->SSHAccess);		
	}
	return ;
}

void display_mainmenu_6(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_network_settings_t *plang_netset = &menu_ctrl.Mlangdata.network_settings;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKSETTING);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkSettings);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKSETTING);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkSettings);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_WAN);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_LAN);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netset->WanMode);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netset->LanMode);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->NetworkSettings);	
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkSettings);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);	
		
	}

}


void display_mainmenu_7(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_system_control_t *plang_sysctrl = &menu_ctrl.Mlangdata.system_control;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_SYSTEMCTRL);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemControl);
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_SYSTEMCTRL);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemControl);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_REBOOT);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESET);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_sysctrl->Reboot);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_sysctrl->FactoryReset);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->SystemControl);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);		
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->SystemControl);				
	}
	return ;
}

void display_mainmenu_8(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_onoff_t *plang_onoff = &menu_ctrl.Mlangdata.onoff;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_LANGUAGE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->Language);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[3], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_POWERSAVE);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->PowerSave);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENABLE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK); 
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->PowerSave);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);	
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->PowerSave);			
	}
	return ;
}


void display_mainmenu_9(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_language_t *planguage = &menu_ctrl.Mlangdata.language;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_LANGUAGE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->Language);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SSHACCESS);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKSETTING);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_SYSTEMCTRL); 
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_POWERSAVE);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SSHAccess);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkSettings);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemControl);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->PowerSave);
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENGLISH);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_CHINESE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK); 
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", planguage->english);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", planguage->chinese);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->Language);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);		
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->Language);			
	}
	return ;
}

void display_mainmenu_10(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SYSTEMSTA);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKINFO);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DEVICEINFO); 
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_WEBACCESS);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemStatus);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkInfo);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->DeviceInfo);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->WebAccess);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);		
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		display_loading_page();
	}	
	return ;
}




void display_web_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_web_t *pweb = &menu_ctrl.Mlangdata.web;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ACCESS);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", pweb->Access);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ACCESS);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", pweb->Access);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		display_webaccess();	
	}
	return;

}

void display_web_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	mlangdata_web_t *pweb = &menu_ctrl.Mlangdata.web;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x,  p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x,  p->box_y[1], 30, 30, ICON_RESETPASS);
		draw_string(p->box_x+40,  p->box_y[1]+p->box_diff, p->fontsize, "0", pweb->ResetPassword);	
		draw_rectangular(p->box_x,  p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESETPASS);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", pweb->ResetPassword);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);		
	}
	return;

}

void display_web_3(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x,  p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x+strlen(plang_mainmenu->Menu)*p->fontsize/2, 4, p->memu_length-(p->box_x+60), 30);
		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);		
		clear_bmp_block(p->box_x,  p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x,  p->box_y[0], 30, 30, ICON_SYSTEMSTA);
		draw_picture(p->box_x,  p->box_y[1], 30, 30, ICON_NETWORKINFO);
		draw_picture(p->box_x,  p->box_y[2], 30, 30, ICON_DEVICEINFO); 
		draw_picture(p->box_x,  p->box_y[3], 30, 30, ICON_WEBACCESS);	
		draw_rectangular(p->box_x,  p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemStatus);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkInfo);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->DeviceInfo);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->WebAccess);		
	}

	return;
}

void display_web_reset_password_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res= &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;
		
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		menufunc_web_reset_password();
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
		draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
	}
	return;
}

void display_web_reset_password_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_web_t *pweb = &menu_ctrl.Mlangdata.web;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ACCESS);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESETPASS);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", pweb->Access);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", pweb->ResetPassword);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->WebAccess);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);	
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->WebAccess);			
	}
	return;
}



void display_networksetting(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_network_settings_t *plang_netset = &menu_ctrl.Mlangdata.network_settings;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};

	if(key_event == KEY_OK) {	
//		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
//		draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
//		draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->NetworkSettings);		
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkSettings);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_WAN);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_LAN);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netset->WanMode);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netset->LanMode);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
	} else if(key_event == KEY_UP) { 
	} else if (key_event == KEY_DOWN) {	
	}
	
	return;
}

void display_networksetting_1(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_network_settings_t *plang_netset = &menu_ctrl.Mlangdata.network_settings;
	mlangdata_network_info_t *plang_netinfo = &menu_ctrl.Mlangdata.network_info;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_WAN);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netset->WanMode);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_WAN);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netset->WanMode);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
//		if(menu_ctrl.LcdType == LCD_TFT3P4) 
			sprintf(buff, "%s>%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkSettings, plang_netinfo->Wan);
//		else
//			sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkSettings);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Disable);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
	}
	return;
}

void display_networksetting_2(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_network_settings_t *plang_netset = &menu_ctrl.Mlangdata.network_settings;
	mlangdata_network_info_t *plang_netinfo = &menu_ctrl.Mlangdata.network_info;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_LAN);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netset->LanMode);	
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_LAN);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netset->LanMode);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
//		if(menu_ctrl.LcdType == LCD_TFT3P4) 
			sprintf(buff, "%s>%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkSettings, plang_netinfo->Lan);
//		else
//			sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkSettings);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);		
		
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_FACTORY);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Factory);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
	}
	return;
}

void display_networksetting_3(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x+(strlen(plang_mainmenu->Menu)*p->fontsize/2), 4, p->memu_length-(p->box_x+60), 30);
		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SSHACCESS);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKSETTING);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_SYSTEMCTRL);	
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_POWERSAVE);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SSHAccess);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkSettings);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemControl);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->PowerSave);	
	}
	return;
}

void display_networksetting_wan_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);	
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);		
	}
	return;
}

void display_networksetting_wan_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static); 
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static); 
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}


void display_networksetting_wan_3(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DISABLE);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Disable); 
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DISABLE);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Disable); 
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;

}

void display_networksetting_wan_4(key_type_t key_event)
{
	mlangdata_network_settings_t *plang_netset = &menu_ctrl.Mlangdata.network_settings;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	char buff[128] = {0};
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[3], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back); 
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[3], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back); 
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {	
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkSettings);
		clear_bmp_block(p->box_x+(strlen(plang_mainmenu->Menu)*p->fontsize/2), 4, (strlen(buff)*p->fontsize/2), 30);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_WAN);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_LAN);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netset->WanMode);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netset->LanMode);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
	}
	return;

}

void display_networksetting_wan_dhcp_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res = &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int res = -1;
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		res = menufunc_set_networksettings(NET_WAN, NET_DHCP);
		if(res == 0) {
//			draw_picture(138, 120, 30, 30, ICON_OK);
//			draw_string(178, 120, p->fontsize, "0", plang_ctrl_res->Success);
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
		} else {
//			draw_picture(138, 120, 30, 30, ICON_WRONG);
//			draw_string(178, 120, p->fontsize, "0", plang_ctrl_res->Fail);
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_WRONG);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Fail);			
		}
	}
	return;
}
void display_networksetting_wan_dhcp_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Disable);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);		
	}
	return;
}
void display_networksetting_wan_static_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res = &menu_ctrl.Mlangdata.control_result;
	int res = -1;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		res = menufunc_set_networksettings(NET_WAN, NET_STATIC);
		if(res == 0) {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
		} else {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_WRONG);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Fail);			
		}	
	}
	return;	
}
void display_networksetting_wan_static_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Disable);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);		
	}
	return;
}

void display_networksetting_wan_factory_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res = &menu_ctrl.Mlangdata.control_result;
	int res = -1;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		res = menufunc_set_networksettings(NET_WAN, NET_FACTORY);
		if(res == 0) {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
		} else {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_WRONG);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Fail);			
		}	
	}
	return;	
}
void display_networksetting_wan_factory_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Disable);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);		
	}
	return;
}


void display_networksetting_lan_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);	
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}

void display_networksetting_lan_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	menu_display_t *p = &menu_ctrl.Mdisplay;


	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static); 
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static); 
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}

void display_networksetting_lan_3(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_FACTORY);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Factory); 
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_FACTORY);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Factory); 
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}

void display_networksetting_lan_4(key_type_t key_event)
{
	mlangdata_network_settings_t *plang_netset = &menu_ctrl.Mlangdata.network_settings;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	char buff[128] = {0};
	menu_display_t *p = &menu_ctrl.Mdisplay;


	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[3], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back); 
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[3], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back); 
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {	
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->NetworkSettings);
		clear_bmp_block(p->box_x+(strlen(plang_mainmenu)*p->fontsize/2), 4, (strlen(buff)*p->fontsize/2), 30);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_WAN);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_LAN);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netset->WanMode);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netset->LanMode);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
	}
	return;
}

void display_networksetting_lan_dhcp_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res= &menu_ctrl.Mlangdata.control_result;
	int res = -1;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		res = menufunc_set_networksettings(NET_LAN, NET_DHCP);
		if(res == 0) {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
		} else {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_WRONG);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Fail);			
		}	

	}
	return;	
}
void display_networksetting_lan_dhcp_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1],  p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1],  p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_FACTORY);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Factory);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);		
	}
	return;
}
void display_networksetting_lan_static_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res= &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int res = -1;
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		res = menufunc_set_networksettings(NET_LAN, NET_STATIC);
		if(res == 0) {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
		} else {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_WRONG);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Fail);			
		}			
	}
	return;	
}
void display_networksetting_lan_static_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x,  p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0],p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_FACTORY);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Factory);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);		
	}
	return;
}

void display_networksetting_lan_factory_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res= &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int res = -1;
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1],  p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		res = menufunc_set_networksettings(NET_LAN, NET_FACTORY);
		if(res == 0) {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
		} else {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_WRONG);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Fail);			
		}
	}
	return;	
}
void display_networksetting_lan_factory_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_network_mode_t *plang_netmode = &menu_ctrl.Mlangdata.network_mode;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_DHCP);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_STATIC);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_FACTORY);
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_BACK);	
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_netmode->DHCP);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_netmode->Static);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_netmode->Factory);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);		
	}
	return;
}


void display_sshaccess_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENABLE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENABLE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);		
	}
	return;
}

void display_sshaccess_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);	
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0],  p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff,  p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);		
	}
	return;
}


void display_sshaccess_3(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
//		clear_bmp_block(88, 12, 200, 30);
//		draw_string(p->box_x, 12, 24, "0", plang_mainmenu->Menu);
		clear_bmp_block(p->box_x+(strlen(plang_mainmenu->Menu)*p->fontsize/2), 4, p->memu_length-(p->box_x+60), 30);
		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);		
		clear_bmp_block(p->box_x,  p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x,  p->box_y[0], 30, 30, ICON_SSHACCESS);
		draw_picture(p->box_x,  p->box_y[1], 30, 30, ICON_NETWORKSETTING);
		draw_picture(p->box_x,  p->box_y[2], 30, 30, ICON_SYSTEMCTRL);	
		draw_picture(p->box_x,  p->box_y[3], 30, 30, ICON_POWERSAVE);	
		draw_rectangular(p->box_x,  p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SSHAccess);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkSettings);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemControl);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->PowerSave);	
	}

	return;
}


void display_sshaccess_enable_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res= &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int res = -1;
		
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		res = menufunc_set_ssh_access(SSH_ENABLE);
		if(res == 0) {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
		} else {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_WRONG);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Fail);			
		}	
	}
	return;
}

void display_sshaccess_enable_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENABLE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	}
	return;
}

void display_back_confirm(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	
	if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	}
}

void display_sshaccess_disable_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res = &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	int res = -1;
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		res = menufunc_set_ssh_access(SSH_DISABLE);
		if(res == 0) {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);
		} else {
			draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_WRONG);
			draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Fail);			
		}			
	}
	return;
}

void display_sshaccess_disable_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENABLE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	}
	return;

}


void display_systemctrl_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_system_control_t *plang_sysctrl =  &menu_ctrl.Mlangdata.system_control;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_REBOOT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_sysctrl->Reboot);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_REBOOT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_sysctrl->Reboot);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);				
	}
	return;
}

void display_systemctrl_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_system_control_t *plang_sysctrl =  &menu_ctrl.Mlangdata.system_control;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESET);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_sysctrl->FactoryReset);	
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESET);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_sysctrl->FactoryReset);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}


void display_systemctrl_3(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
//		clear_bmp_block(88, 12, 200, 30);
//		draw_string(40, 12, 24, "0", plang_mainmenu->Menu);	
		clear_bmp_block(p->box_x+(strlen(plang_mainmenu->Menu)*p->fontsize/2), 4, p->memu_length-(p->box_x+60), 30);
		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SSHACCESS);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKSETTING);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_SYSTEMCTRL);	
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_POWERSAVE);	
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SSHAccess);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkSettings);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemControl);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->PowerSave);	
	}

	return;
}


void display_systemctrl_reboot_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		display_systembooting_page();
		menufunc_system_reboot();
	}
	return;	
}

void display_systemctrl_reboot_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_system_control_t *plang_sysctrl =  &menu_ctrl.Mlangdata.system_control;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_REBOOT);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESET);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_sysctrl->Reboot);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_sysctrl->FactoryReset);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}

void display_systemctrl_reset_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		display_systembooting_page();
		menufunc_factory_reset();
	}
	return;	
}

void display_systemctrl_reset_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_system_control_t *plang_sysctrl =  &menu_ctrl.Mlangdata.system_control;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_REBOOT);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESET);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_sysctrl->Reboot);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_sysctrl->FactoryReset);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}


void display_powersave_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x,  p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x,  p->box_y[0], 30, 30, ICON_ENABLE);
		draw_string(p->box_x+40,  p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);	
		draw_rectangular(p->box_x,  p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x,  p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x,  p->box_y[0], 30, 30, ICON_ENABLE);
		draw_string(p->box_x+40,  p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);
		draw_rectangular(p->box_x,  p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x,  p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x,  p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40,  p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x,  p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40,  p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x,  p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);		
	}
	return;
}
void display_powersave_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);	
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);		
	}
	return;
}
void display_powersave_3(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x+(strlen(plang_mainmenu->Menu)*p->fontsize/2), 4, p->memu_length-(p->box_x+60), 30);
		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);	
//		clear_bmp_block(88, 12, 200, 30);
//		draw_string(40, 12, 24, "0", plang_mainmenu->Menu);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SSHACCESS);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKSETTING);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_SYSTEMCTRL);	
		draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_POWERSAVE);	
		draw_rectangular(p->box_x, p->box_y[3], p->box_lenth, p->box_width, DEEP_BLUE);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SSHAccess);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkSettings);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemControl);
		draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->PowerSave);	
	}
	return;
}

void display_powersave_enable_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res= &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;
		
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		menu_ctrl.MenuConf.PowerSaveFlag = 0;
		menu_reflesh_conf(&menu_ctrl.MenuConf);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
		draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);		
	}
	return;
}
void display_powersave_enable_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENABLE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	}
	return;
}
void display_powersave_disable_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res = &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	
	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		menu_ctrl.MenuConf.PowerSaveFlag = 1;
		menu_reflesh_conf(&menu_ctrl.MenuConf);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
		draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);				
	}
	return;

}
void display_powersave_disable_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_onoff_t *plang_onoff =  &menu_ctrl.Mlangdata.onoff;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENABLE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_DISABLE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_onoff->Enable);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_onoff->Disable);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	}
	return;
}


void display_language_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_language_t *planguage =  &menu_ctrl.Mlangdata.language;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENGLISH);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", planguage->english);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENGLISH);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", planguage->english);
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);				
	}
	return;
}

void display_language_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_language_t *planguage =  &menu_ctrl.Mlangdata.language;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_CHINESE);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", planguage->chinese);	
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_CHINESE);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", planguage->chinese);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);		
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}

void display_language_3(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[2], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);	
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x+(strlen(plang_mainmenu->Menu)*p->fontsize/2), 4, p->memu_length-(p->box_x+60), 30);
		draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);	
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_LANGUAGE);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_BACK);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->Language);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	}

	return;
}
void display_language_chinese_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res = &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		menufunc_set_language	(LANG_CN);
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
		draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);		
	}
	return;	

}
void display_language_chinese_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_language_t *planguage =  &menu_ctrl.Mlangdata.language;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENGLISH);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_CHINESE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", planguage->english);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", planguage->chinese);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;

}
void display_language_english_1(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_control_result_t *plang_ctrl_res = &menu_ctrl.Mlangdata.control_result;
	menu_display_t *p = &menu_ctrl.Mdisplay;


	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_RIGHT);
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_confirm->Confirm);
		draw_rectangular(p->box_x, p->box_y[1], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		menufunc_set_language	(LANG_EN);	
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->memu_length/2-60, p->memu_width/2, 30, 30, ICON_OK);
		draw_string(p->memu_length/2-20, p->memu_width/2, p->fontsize, "0", plang_ctrl_res->Success);		
	}
	return;	
}
void display_language_english_2(key_type_t key_event)
{
	mlangdata_confirm_t *plang_confirm = &menu_ctrl.Mlangdata.confirm;
	mlangdata_language_t *planguage =  &menu_ctrl.Mlangdata.language;
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	if(key_event == KEY_DOWN) {
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_UP) { 
		clear_bmp_block(p->box_x, p->box_y[1], p->box_lenth+2, p->box_width+2);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_WRONG);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_confirm->Cancel);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	} else if(key_event == KEY_OK) {
		clear_bmp_block(p->box_x, p->box_y[0], p->box_lenth+2, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ENGLISH);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_CHINESE);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", planguage->english);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", planguage->chinese);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);	
	}
	return;
}

void display_mainmenu_no_rectangular(void)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	menu_display_t *p = &menu_ctrl.Mdisplay;

	clear_bmp_block(5, p->box_y[0], p->memu_length-5, p->memu_width-p->box_y[0]-1);
	clear_bmp_block(p->box_x+(strlen(plang_mainmenu->Menu)*p->fontsize/2), 4, p->memu_length-(p->box_x+60), 30);
	
	draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
	draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_SYSTEMSTA);
	draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_NETWORKINFO);
	draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_DEVICEINFO); 
	draw_picture(p->box_x, p->box_y[3], 30, 30, ICON_WEBACCESS);	
	draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", plang_mainmenu->SystemStatus);
	draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", plang_mainmenu->NetworkInfo);
	draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->DeviceInfo);
	draw_string(p->box_x+40, p->box_y[3]+p->box_diff, p->fontsize, "0", plang_mainmenu->WebAccess);
}

void display_systemstatus_back(key_type_t key_event)
{
	menu_display_t *p = &menu_ctrl.Mdisplay;
	
	if(key_event == KEY_OK) {
		display_mainmenu_no_rectangular();
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_DOWN) {
		
	} else if(key_event == KEY_UP) {

	}
	
	return;
}

void display_networkinfo_back(key_type_t key_event)
{
	menu_display_t *p = &menu_ctrl.Mdisplay;
	
	if(key_event == KEY_OK) {
		display_mainmenu_no_rectangular();
		draw_rectangular(p->box_x, p->box_y[1],  p->box_lenth, p->box_width, DEEP_BLUE);
	} else if(key_event == KEY_DOWN) {	
		clear_bmp_block(5, p->box_y[0], p->memu_length-5, p->memu_width-p->box_y[0]-1);
		display_networkinfo(NET_WAN);	
	} else if(key_event == KEY_UP) {
		clear_bmp_block(5, p->box_y[0], p->memu_length-5, p->memu_width-p->box_y[0]-1);
		display_networkinfo(NET_LAN);	
	}
	return;
}

void display_deviceinfo_back(key_type_t key_event)
{
	menu_display_t *p = &menu_ctrl.Mdisplay;
	
	if(key_event == KEY_OK) {
		display_mainmenu_no_rectangular();
		draw_rectangular(p->box_x, p->box_y[2], p->box_lenth, p->box_width, DEEP_BLUE);		
	} else if(key_event == KEY_DOWN) {
		
	} else if(key_event == KEY_UP) {

	}
	return;
}

void display_webaccess_back(key_type_t key_event)
{
	mlangdata_main_menu_t *plang_mainmenu = &menu_ctrl.Mlangdata.main_menu;
	mlangdata_web_t *pweb = &menu_ctrl.Mlangdata.web;
	menu_display_t *p = &menu_ctrl.Mdisplay;
	char buff[128] = {0};

	if(key_event == KEY_OK) {
		clear_bmp_block(5, p->box_y[0], p->memu_length-5, p->memu_width-p->box_y[0]-1);
		draw_picture(p->box_x, p->box_y[0], 30, 30, ICON_ACCESS);
		draw_picture(p->box_x, p->box_y[1], 30, 30, ICON_RESETPASS);
		draw_picture(p->box_x, p->box_y[2], 30, 30, ICON_BACK);	
		draw_string(p->box_x+40, p->box_y[0]+p->box_diff, p->fontsize, "0", pweb->Access);
		draw_string(p->box_x+40, p->box_y[1]+p->box_diff, p->fontsize, "0", pweb->ResetPassword);
		draw_string(p->box_x+40, p->box_y[2]+p->box_diff, p->fontsize, "0", plang_mainmenu->Back);
		draw_rectangular(p->box_x, p->box_y[0], p->box_lenth, p->box_width, DEEP_BLUE);
	//	draw_string(p->box_x, 4, p->fontsize, "0", plang_mainmenu->Menu);
	//	draw_string(p->box_x+48, 4, p->fontsize, "0", ">");
	//	draw_string(p->box_x+60, 4, p->fontsize, "0", plang_mainmenu->WebAccess);		
		sprintf(buff, "%s>%s", plang_mainmenu->Menu, plang_mainmenu->WebAccess);
		draw_string(p->box_x, 4, p->fontsize, "0", buff);
	} else if(key_event == KEY_DOWN) {
		
	} else if(key_event == KEY_UP) {

	}
	return;
}




