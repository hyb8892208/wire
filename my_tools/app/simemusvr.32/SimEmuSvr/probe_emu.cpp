/**********************************************************************************
�ļ�����: ����������̽�ⵥ�������е�USB���ڣ��ҳ��ڵ����豸��֮���ӳ���ϵ��
����    : ����ά
ʱ��    : 2016.06.24
��ע    : ������ο���lsusbԴ�롣
***********************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include "serial.h"

#define USB_NODE_MAX           (256)     /* USB���256���ڵ� */
#define MY_PATH_MAX            (256)
#define MAP_INFO_MAX           (8192)
#define NAME_LEN               (64)

/* ͨ���Ŷ��壬�������ʾͨ������usb�����豸֮���ӳ���ϵ */
typedef enum tagENUM_CHANNEL
{
    CHAN_FIRST  = 1, 
    CHAN_END    = 32,      /* ���32��ͨ�� */
    CHAN_STM8_1 = 33,      /* �뵥Ƭ��֮��Ĵ��� */
}ENUM_CHANNEL;

/* ӳ���ϵ�ṹ�� */
typedef struct tagST_CHAN_TO_DEV
{
    char node_name[NAME_LEN];  /* �ڵ����� */
    char dev_name[NAME_LEN];   /* ��Ӧ���豸���� */
}ST_CHAN_TO_DEV;

/* ӳ���ϵ��Ϣ */
ST_CHAN_TO_DEV arr_node_dev_info[USB_NODE_MAX] = 
{
    {"", ""}, /* ��ʷԭ��ͨ���Ǵ�1��ʼ */ 
    {"1-1.2.3.2.3", ""},
    {"1-1.2.3.4.2", ""}, /* bottom board, first left slot */
	{"1-1.2.4.2.3", ""}, 
    {"1-1.2.4.4.2", ""}, /* top board, first left slot */
    {"", ""}
};

/* �������Ŀ¼�������ļ��� */
static const char sys_bus_usb_devices[] = "/sys/bus/usb/devices";

/* gsmģ��usb������Ϣ��������Ŀ¼ */
static const char usb_serial_info[] = "/tmp/emu_info";

/* ����ӳ����Ϣ����(�ַ���) */
static char ttyUSB_map_info[MAP_INFO_MAX];
static unsigned long map_info_len;

static void inspect_bus_entry(const char *d_name)
{
    unsigned long i = 0;
    unsigned long l = 0;
    char dir_path[MY_PATH_MAX];
    char devname[MY_PATH_MAX];
    char nodename[MY_PATH_MAX + 1];
    DIR *sbud = NULL;
    
	if (d_name[0] == '.' && (!d_name[1] || (d_name[1] == '.' && !d_name[2])))
		return;

	if ( isdigit(d_name[0]) && strchr(d_name, ':') )
    {
        /* 1. ��ȡ��Ӧ���豸���� */
        /* ͨ���Ƿ���ڸ��ļ���������usb ����ӳ���ϵ */
        for ( i = 0; i < USB_NODE_MAX; i++ ) 
        {
            l = snprintf(dir_path, MY_PATH_MAX, "%s/%s/ttyUSB%d", sys_bus_usb_devices, d_name, i);
            if ( l > 0 && l < MY_PATH_MAX ) 
            {
            	if ( sbud = opendir(dir_path) ) 
                {
                    closedir(sbud);
                    snprintf(devname, MY_PATH_MAX, "/dev/ttyUSB%d", i);
                    break;
                }   
            }
        }

        if ( i >= USB_NODE_MAX )
        {
            return;
        }
        
        /* 2. ��ȡbus�ڵ㣬ÿ��USB�豸����Ψһ�� */
        memset(nodename, 0, MY_PATH_MAX + 1);

        for ( i = 0; i < MY_PATH_MAX; i++ )
        {
            if ( ':' == d_name[i] )
            {
                break;
            }
            nodename[i] = d_name[i];
        }

        if ( i >= MY_PATH_MAX )
        {
            return;
        }

        /* 3. ���ÿ���豸�Ľڵ�Ŷ��ǹ̶��� */
        for ( i = 1; i < USB_NODE_MAX; i++ )
        {
            if ( strlen(arr_node_dev_info[i].node_name) == 0 )
            {
                break;
            }

            if ( strcmp(nodename, arr_node_dev_info[i].node_name) == 0 )
            {
                strcpy(arr_node_dev_info[i].dev_name, devname);
                break;
            }
        }
    }
}


int get_config(char *cfg_file, char *domain, char *item, char *content)
{
	int ret = -1;
	FILE *fp = NULL;
	char cmd[1024] = {0};
	if (cfg_file == NULL || domain == NULL || item == NULL || content == NULL)
	{
		return -1;
	}

	sprintf(cmd, "/my_tools/set_config %s get option_value %s %s", cfg_file, domain, item);
	fp = popen(cmd, "r");
	if (fp == NULL)
	{
		return ret;
	}
	while (!feof(fp))
	{
		fgets(content, 1024, fp);
	}
	if (strlen(content) > 0)
	{
		ret = 0;
	}
	pclose(fp);
	return ret;
}

int check_emu_exist(char *dev, int baud)
{
	int i = 0;
	int fd = -1;
	int len = 0;
	char buff[320] = {0};

	fd = open_serial(dev, baud, 0, 8, 1, 0);
	if (fd < 0)
	{
		return -1;
	}
	memset(buff, 0, sizeof(buff));
	len = serial_wr(fd, (unsigned char *)"\x80\x83\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
	if (len > 0)
	{
		if (((unsigned char *)buff)[0] != 0x80 || ((unsigned char *)buff)[1] != 0x82)
		{
			return -1;
		}
		else
			printf("Emu[%s] exist,[version:%s]\n", dev, buff+4);
	}
	else
	{
		return -1;
	}
	close_serial(fd);
	return 0;
}

int main(int argc,char *argv[])
{
    unsigned long i = 0;
    struct dirent *de;
    DIR *sbud = NULL;
    FILE *fp;
	int baud = 0;
	char content[128] = {0};

	get_config("/etc/asterisk/simemusvr.conf", "SimEmuSvr", "baudrate", content);
	if (strlen(content) > 0)
		baud = atoi(content);
	else
		baud = 460800;

    /* ��������/dev/ttyUSBx */
    sbud = opendir(sys_bus_usb_devices);
	if (sbud) 
    {
        memset(ttyUSB_map_info, 0, MAP_INFO_MAX);
        map_info_len = 0;

    	while ( (de = readdir(sbud)) )
    		inspect_bus_entry(de->d_name);

        closedir(sbud);
    }

    /* ��ӳ����Ϣд���ļ�ȥ */
    for ( i = 1; i < USB_NODE_MAX; i++ )
    {
        if ( strlen(arr_node_dev_info[i].node_name) == 0 )
        {
            break;
        }

        if ( strlen(arr_node_dev_info[i].dev_name) > 0 )
        {
        	if (check_emu_exist(arr_node_dev_info[i].dev_name, baud) != 0)
				continue;
            if ( snprintf(&ttyUSB_map_info[map_info_len], MAP_INFO_MAX - map_info_len, 
            "[%d]\ndev=%s\n", i, arr_node_dev_info[i].dev_name) > 0 )
            {
                while ( ttyUSB_map_info[map_info_len] )
                {
                    if ( map_info_len >= MAP_INFO_MAX )
                    {
                        ttyUSB_map_info[map_info_len - 1] = '\0';
                        return -1;
                    }
                    map_info_len++;
                }
            }
        }
    }

    fp = fopen(usb_serial_info, "w+");

    if ( fp != NULL ) 
    {
		//fprintf(fp, "%s", ttyUSB_map_info);
        fwrite(ttyUSB_map_info, 1, map_info_len, fp);
		fclose(fp);
	}

    printf("%s", ttyUSB_map_info);

    return 0;
}

