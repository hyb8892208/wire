/**********************************************************************************
Description : this program use to probe usb sound card, and map to logic port
Author      : zhongwei.peng@openvox.cn
Time        : 2016.06.24
Note        : 
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

#define USB_NODE_MAX           (256)
#define MY_PATH_MAX            (512)
#define MAP_INFO_MAX           (65536) // 64K
#define NAME_LEN               (64)
#define PORT_STR_MAX           (256)
#define INVALID_VALUE          (0xFFFF)

/* usb node map to sound card */
typedef struct tagST_NODE_TO_SND_NUM
{
    char node_name[NAME_LEN];  /* usb node */
    unsigned int snd_num;      /* sound card num */
}ST_NODE_TO_SND_NUM;

#if 1
ST_NODE_TO_SND_NUM arr_node_dev_info[USB_NODE_MAX] = 
{
    {"", 0}, /* no use, because port start from 1 */
    {"1-1.2.4.1.4.3", INVALID_VALUE}, /* top board, first left slot */
    {"1-1.2.4.1.4.3", INVALID_VALUE},
    {"1-1.2.4.1.3.3", INVALID_VALUE}, 
    {"1-1.2.4.1.3.3", INVALID_VALUE}, 
    {"1-1.2.4.3.2", INVALID_VALUE}, 
    {"1-1.2.4.3.2", INVALID_VALUE}, 
    {"1-1.2.4.4.1", INVALID_VALUE}, 
    {"1-1.2.4.4.1", INVALID_VALUE}, 
    {"1-1.2.4.1.1.3", INVALID_VALUE}, 
    {"1-1.2.4.1.1.3", INVALID_VALUE}, 
    {"1-1.2.4.1.2.2", INVALID_VALUE}, 
    {"1-1.2.4.1.2.2",INVALID_VALUE}, 
    {"1-1.2.4.2.1.1", INVALID_VALUE}, 
    {"1-1.2.4.2.1.1", INVALID_VALUE}, 
    {"1-1.2.4.2.1.4", INVALID_VALUE}, 
    {"1-1.2.4.2.1.4", INVALID_VALUE}, 
    {"1-1.2.3.1.4.3", INVALID_VALUE}, /* bottom board, first left slot */
	{"1-1.2.3.1.4.3", INVALID_VALUE}, 
    {"1-1.2.3.1.3.3", INVALID_VALUE}, 
	{"1-1.2.3.1.3.3",INVALID_VALUE}, 
    {"1-1.2.3.3.2", INVALID_VALUE},
	{"1-1.2.3.3.2", INVALID_VALUE},
    {"1-1.2.3.4.1", INVALID_VALUE},
	{"1-1.2.3.4.1", INVALID_VALUE},
    {"1-1.2.3.1.1.3", INVALID_VALUE}, 
	{"1-1.2.3.1.1.3", INVALID_VALUE}, 
    {"1-1.2.3.1.2.2", INVALID_VALUE}, 
	{"1-1.2.3.1.2.2", INVALID_VALUE}, 
    {"1-1.2.3.2.1.1", INVALID_VALUE}, 
	{"1-1.2.3.2.1.1", INVALID_VALUE}, 
    {"1-1.2.3.2.1.4", INVALID_VALUE}, 
	{"1-1.2.3.2.1.4", INVALID_VALUE}, 
    {"", 0}, 
    {"", 0}
};
#else
/* map array */
ST_NODE_TO_SND_NUM arr_node_dev_info[USB_NODE_MAX] = 
{
    {"", 0}, /* no use, because port start from 1 */
    {"1-1.2.2.1", INVALID_VALUE}, /* top board, first left slot */
    {"1-1.2.2.1", INVALID_VALUE},
    {"1-1.2.2.2", INVALID_VALUE}, 
    {"1-1.2.2.2", INVALID_VALUE}, 
    {"1-1.2.2.3", INVALID_VALUE}, 
    {"1-1.2.2.3", INVALID_VALUE}, 
    {"1-1.2.2.4", INVALID_VALUE}, 
    {"1-1.2.2.4", INVALID_VALUE}, 
    {"1-1.2.2.5", INVALID_VALUE}, 
    {"1-1.2.2.5", INVALID_VALUE}, 
    {"1-1.2.2.6", INVALID_VALUE}, 
    {"1-1.2.2.6", INVALID_VALUE}, 
    {"1-1.2.2.7", INVALID_VALUE}, 
    {"1-1.2.2.7", INVALID_VALUE}, 
    {"1-1.2.2.8", INVALID_VALUE}, 
    {"1-1.2.2.8", INVALID_VALUE}, 
    {"1-1.2.1.1", INVALID_VALUE}, /* bottom board, first left slot */
	{"1-1.2.1.1", INVALID_VALUE}, 
    {"1-1.2.1.2", INVALID_VALUE}, 
	{"1-1.2.1.2", INVALID_VALUE}, 
    {"1-1.2.1.3", INVALID_VALUE},
	{"1-1.2.1.3", INVALID_VALUE},
    {"1-1.2.1.4", INVALID_VALUE},
	{"1-1.2.1.4", INVALID_VALUE},
    {"1-1.2.1.5", INVALID_VALUE}, 
	{"1-1.2.1.5", INVALID_VALUE}, 
    {"1-1.2.1.6", INVALID_VALUE}, 
	{"1-1.2.1.6", INVALID_VALUE}, 
    {"1-1.2.1.7", INVALID_VALUE}, 
	{"1-1.2.1.7", INVALID_VALUE},
	
	{"1-1.2.4.1", INVALID_VALUE},
	{"1-1.2.4.1", INVALID_VALUE},
	{"1-1.2.4.2", INVALID_VALUE},
	{"1-1.2.4.2", INVALID_VALUE},
	{"1-1.2.4.3", INVALID_VALUE},
	{"1-1.2.4.3", INVALID_VALUE},
	{"1-1.2.4.4", INVALID_VALUE},
	{"1-1.2.4.4", INVALID_VALUE},
	{"1-1.2.4.5", INVALID_VALUE},
	{"1-1.2.4.5", INVALID_VALUE},
	{"1-1.2.4.6", INVALID_VALUE},
	{"1-1.2.4.6", INVALID_VALUE},
	{"1-1.2.4.7", INVALID_VALUE},
	{"1-1.2.4.7", INVALID_VALUE},
	
	{"1-1.2.1", INVALID_VALUE},
	
	{"8-2.4", INVALID_VALUE},
	{"8-2.4", INVALID_VALUE},
	{"8-2.5", INVALID_VALUE},
	{"8-2.5", INVALID_VALUE},
	{"8-2.6", INVALID_VALUE},
	{"8-2.6", INVALID_VALUE},
	{"8-2.7", INVALID_VALUE},
	{"8-2.7", INVALID_VALUE},
	{"7-5.1", INVALID_VALUE},
	{"7-5.1", INVALID_VALUE},
	{"7-5.5", INVALID_VALUE},
	{"7-5.5", INVALID_VALUE},
	{"7-5.6", INVALID_VALUE},
	{"7-5.6", INVALID_VALUE},
	{"7-5.7", INVALID_VALUE},
	{"7-5.7", INVALID_VALUE},
	{"8-6.1", INVALID_VALUE},
	{"8-6.1", INVALID_VALUE},
	{"8-6.5", INVALID_VALUE},
	{"8-6.5", INVALID_VALUE},
	{"8-6.6", INVALID_VALUE},
	{"8-6.6", INVALID_VALUE},
	{"8-6.7", INVALID_VALUE},
	{"8-6.7", INVALID_VALUE},
	
	{"1-1.2.2.2.1", INVALID_VALUE},
	{"1-1.2.2.2.1", INVALID_VALUE},
	{"1-1.2.2.2.2", INVALID_VALUE},
	{"1-1.2.2.2.2", INVALID_VALUE},
	{"1-1.2.2.2.3", INVALID_VALUE},
	{"1-1.2.2.2.3", INVALID_VALUE},
	{"1-1.2.2.2.4", INVALID_VALUE},
	{"1-1.2.2.2.4", INVALID_VALUE},
	{"1-1.2.2.2.5", INVALID_VALUE},
	{"1-1.2.2.2.5", INVALID_VALUE},
	{"1-1.2.2.2.6", INVALID_VALUE},
	{"1-1.2.2.2.6", INVALID_VALUE},
	{"1-1.2.2.2.7", INVALID_VALUE},
	{"1-1.2.2.2.7", INVALID_VALUE},
	
	{"1-1.2.1.4.1", INVALID_VALUE},
	{"1-1.2.1.4.1", INVALID_VALUE},
	{"1-1.2.1.4.2", INVALID_VALUE},
	{"1-1.2.1.4.2", INVALID_VALUE},
	{"1-1.2.1.4.3", INVALID_VALUE},
	{"1-1.2.1.4.3", INVALID_VALUE},
	{"1-1.2.1.4.4", INVALID_VALUE},
	{"1-1.2.1.4.4", INVALID_VALUE},
	{"1-1.2.1.4.5", INVALID_VALUE},
	{"1-1.2.1.4.5", INVALID_VALUE},
	{"1-1.2.1.4.6", INVALID_VALUE},
	{"1-1.2.1.4.6", INVALID_VALUE},
	{"1-1.2.1.4.7", INVALID_VALUE},
	{"1-1.2.1.4.7", INVALID_VALUE},

    {"1-1.2.4.1.4.3", INVALID_VALUE}, /* top board, first left slot */
    {"1-1.2.4.1.4.3", INVALID_VALUE},
    {"1-1.2.4.1.3.3", INVALID_VALUE}, 
    {"1-1.2.4.1.3.3", INVALID_VALUE}, 
    {"1-1.2.4.3.2", INVALID_VALUE}, 
    {"1-1.2.4.3.2", INVALID_VALUE}, 
    {"1-1.2.4.4.1", INVALID_VALUE}, 
    {"1-1.2.4.4.1", INVALID_VALUE}, 
    {"1-1.2.4.1.1.3", INVALID_VALUE}, 
    {"1-1.2.4.1.1.3", INVALID_VALUE}, 
    {"1-1.2.4.1.2.2", INVALID_VALUE}, 
    {"1-1.2.4.1.2.2",INVALID_VALUE}, 
    {"1-1.2.4.2.1.1", INVALID_VALUE}, 
    {"1-1.2.4.2.1.1", INVALID_VALUE}, 
    {"1-1.2.4.2.1.4", INVALID_VALUE}, 
    {"1-1.2.4.2.1.4", INVALID_VALUE}, 
    {"1-1.2.3.1.4.3", INVALID_VALUE}, /* bottom board, first left slot */
	{"1-1.2.3.1.4.3", INVALID_VALUE}, 
    {"1-1.2.3.1.3.3", INVALID_VALUE}, 
	{"1-1.2.3.1.3.3",INVALID_VALUE}, 
    {"1-1.2.3.3.2", INVALID_VALUE},
	{"1-1.2.3.3.2", INVALID_VALUE},
    {"1-1.2.3.4.1", INVALID_VALUE},
	{"1-1.2.3.4.1", INVALID_VALUE},
    {"1-1.2.3.1.1.3", INVALID_VALUE}, 
	{"1-1.2.3.1.1.3", INVALID_VALUE}, 
    {"1-1.2.3.1.2.2", INVALID_VALUE}, 
	{"1-1.2.3.1.2.2", INVALID_VALUE}, 
    {"1-1.2.3.2.1.1", INVALID_VALUE}, 
	{"1-1.2.3.2.1.1", INVALID_VALUE}, 
    {"1-1.2.3.2.1.4", INVALID_VALUE}, 
	{"1-1.2.3.2.1.4", INVALID_VALUE}, 
	
    {"", INVALID_VALUE}, 
	{"", INVALID_VALUE}, 
    {"", 0}, 
    {"", 0}
};
#endif
/* loop this fold to find usb sound card */
static const char sys_bus_usb_devices[] = "/sys/bus/usb/devices";

/* record sound conf */
static const char snd_usb_conf_path[] = "/etc/asound.conf";

/* record map info */
static const char snd_usb_info_path[] = "/tmp/usb-sound";

/* map info str buf */
static char snd_usb_map_str[MAP_INFO_MAX];
static unsigned long snd_usb_map_len;

/* /etc/asound.conf str buf */
static char snd_usb_conf_str[MAP_INFO_MAX];
static unsigned long snd_usb_conf_len;

static char port_str[PORT_STR_MAX];
static unsigned long port_str_len;

/* magic num. ipc_key specfies the unique IPC key in integer */
static unsigned int ipc_key = 143210;

/**************************************************
Description : inspect dir find all the sound dirs
Input       : d_name -- dir name
Output      : 
Author      : zhongwei.peng
Time        : 2016.07.04
Note        : 
***************************************************/
static void inspect_bus_entry(const char *d_name)
{
    unsigned long i = 0;
    unsigned long l = 0;
    unsigned long card_id = 0;
    char dir_path[MY_PATH_MAX];
    char devname[MY_PATH_MAX];
    char nodename[MY_PATH_MAX + 1];
    DIR *sbud = NULL;
    
	if (d_name[0] == '.' && (!d_name[1] || (d_name[1] == '.' && !d_name[2])))
		return;

	if ( isdigit(d_name[0]) && strchr(d_name, ':') )
    {
        /* check if the sound dir exist, or return */
        snprintf(dir_path, MY_PATH_MAX, "%s/%s/sound", sys_bus_usb_devices, d_name);
        if ( sbud = opendir(dir_path) ) 
        {
            closedir(sbud);

            for ( i = 0; i < USB_NODE_MAX; i++ ) 
            {
                snprintf(dir_path, MY_PATH_MAX, "%s/%s/sound/card%d", sys_bus_usb_devices, d_name, i);
                if ( sbud = opendir(dir_path) ) 
                {
                    closedir(sbud);
                    card_id = i;
                    break;
                }
            }
            if ( i >= USB_NODE_MAX )
            {
                return;
            }
        }
        else
        {
            return;
        }
        
        /* if card%d exist, get usb node name */
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

        /* fill the card id from the nodename */
        for ( i = 1; i < USB_NODE_MAX; i++ )
        {
            if ( strlen(arr_node_dev_info[i].node_name) == 0 )
            {
                break;
            }

            if ( strcmp(nodename, arr_node_dev_info[i].node_name) == 0 )
            {
                arr_node_dev_info[i].snd_num   = card_id;
                arr_node_dev_info[i+1].snd_num = card_id; /* 2 ports use the same chip */
                break;
            }
        }
    }
}

/**************************************************
Description : generate channel configuration by chip(/etc/asound)
Input       : snd_num -- usb sound num in current system
Output      : 
Author      : zhongwei.peng
Time        : 2016.07.04
Note        : 
***************************************************/
void generate_asound_config(unsigned int snd_num)
{
    int ret = 0;

    /* plugin hw: card */
    ret = snprintf(&snd_usb_conf_str[snd_usb_conf_len], MAP_INFO_MAX - snd_usb_conf_len, 
            "pcm.usbcard%d{\n" 
            "type hw\n" 
            "card %d\n" 
            "}\n\n",
            snd_num, snd_num);
    snd_usb_conf_len += ret;

    /* dmix Left */
    ret = snprintf(&snd_usb_conf_str[snd_usb_conf_len], MAP_INFO_MAX - snd_usb_conf_len, 
            "pcm.dmix-%d-L{\n"
            "type dmix\n" 
            "ipc_key %d\n" 
            "ipc_key_add_uid true\n" 
            "ipc_perm 0666\n" 
            "slave{\n" 
            "pcm \"usbcard%d\"\n" 
            "format \"S16_LE\"\n" 
            "rate 8000\n" 
            "channels 2\n"
            "period_size 160\n" 
            "}\n" 
            "bindings{\n" 
            "1 0\n" 
            "}\n" 
            "}\n\n",
            snd_num, ipc_key, snd_num);
    snd_usb_conf_len += ret;

    /* dmix Right */
    ret = snprintf(&snd_usb_conf_str[snd_usb_conf_len], MAP_INFO_MAX - snd_usb_conf_len, 
            "pcm.dmix-%d-R{\n"
            "type dmix\n" 
            "ipc_key %d\n" 
            "ipc_key_add_uid true\n" 
            "ipc_perm 0666\n" 
            "slave{\n" 
            "pcm \"usbcard%d\"\n" 
            "format \"S16_LE\"\n" 
            "rate 8000\n" 
            "channels 2\n"
            "period_size 160\n" 
            "}\n" 
            "bindings{\n" 
            "1 1\n" 
            "}\n" 
            "}\n\n",
            snd_num, ipc_key, snd_num);
    snd_usb_conf_len += ret;
    ipc_key++; /* ipc key: left = right */

    /* dsnoop Left */
    ret = snprintf(&snd_usb_conf_str[snd_usb_conf_len], MAP_INFO_MAX - snd_usb_conf_len, 
            "pcm.dsnoop-%d-L{\n"
            "type dsnoop\n"
            "ipc_key %d\n"
            "ipc_key_add_uid true\n"
			"ipc_perm 0666\n" 
            "slave{\n"
            "pcm \"usbcard%d\"\n" 
            "format S16_LE\n"
            "rate 8000\n"
            "period_size 160\n"
            "channels 2\n"
            "}\n"
            "bindings {\n"
            "0 0\n"
            "}\n"
            "}\n\n", 
            snd_num, ipc_key, snd_num);
    snd_usb_conf_len += ret;

    /* dsnoop Right */
    ret = snprintf(&snd_usb_conf_str[snd_usb_conf_len], MAP_INFO_MAX - snd_usb_conf_len, 
            "pcm.dsnoop-%d-R{\n"
            "type dsnoop\n"
            "ipc_key %d\n"
            "ipc_key_add_uid true\n"
			"ipc_perm 0666\n" 
            "slave{\n"
            "pcm \"usbcard%d\"\n" 
            "format S16_LE\n"
            "rate 8000\n"
            "period_size 160\n"
            "channels 2\n"
            "}\n"
            "bindings {\n"
            "0 1\n"
            "}\n"
            "}\n\n", 
            snd_num, ipc_key, snd_num);
    snd_usb_conf_len += ret;
    ipc_key++;

    /* asym Left */
    ret = snprintf(&snd_usb_conf_str[snd_usb_conf_len], MAP_INFO_MAX - snd_usb_conf_len, 
            "pcm.asym-%d-L{\n"
            "type asym\n"
            "playback.pcm \"dmix-%d-L\"\n"
            "capture.pcm \"dsnoop-%d-L\"\n"
            "}\n\n", 
            snd_num, snd_num, snd_num);
    snd_usb_conf_len += ret;

    /* asym Right */
    ret = snprintf(&snd_usb_conf_str[snd_usb_conf_len], MAP_INFO_MAX - snd_usb_conf_len, 
            "pcm.asym-%d-R{\n"
            "type asym\n"
            "playback.pcm \"dmix-%d-R\"\n"
            "capture.pcm \"dsnoop-%d-R\"\n"
            "}\n\n", 
            snd_num, snd_num, snd_num);
    snd_usb_conf_len += ret;

    /* just warnning */
    if ( snd_usb_conf_len >= MAP_INFO_MAX )
    {
        printf("error: snd_usb_conf_len[%d] exceed max[%d]\n", 
            snd_usb_conf_len, MAP_INFO_MAX);
        return;
    }
}

/**************************************************
Description : rebuild "/etc/asound.conf", ALSA will use it when init.
              build "/tmp/usb-sound" at the same time.
Input       : 
Output      : 
Author      : zhongwei.peng
Time        : 2016.07.04
Note        : 
***************************************************/
void rebuild_asound_conf(void)
{
    unsigned long i = 0;
    unsigned long port_sum = 0;
    unsigned int snd_num = 0;
    char buf[PORT_STR_MAX] = {0};
    int ret = 0;
    FILE *fp;

    memset(snd_usb_map_str, 0, MAP_INFO_MAX);
    snd_usb_map_len = 0;

    memset(snd_usb_conf_str, 0, MAP_INFO_MAX);
    snd_usb_conf_len = 0;

    memset(port_str, 0, PORT_STR_MAX);
    port_str_len = 0;

    for ( i = 1; i < USB_NODE_MAX; i += 2 )
    {
        if ( strlen(arr_node_dev_info[i].node_name) == 0 )
        {
            break;
        }

        if ( arr_node_dev_info[i].snd_num != INVALID_VALUE )
        {
            //printf("arr_node_dev_info[%d].node_name: %s\n", i, arr_node_dev_info[i].node_name);
            //printf("arr_node_dev_info[%d].snd_num: %d \n", i, arr_node_dev_info[i].snd_num);
            
            /* 1. /etc/asound.conf */
            snd_num = arr_node_dev_info[i].snd_num;
            generate_asound_config(snd_num);

            /* 2. /tmp/usb-sound */
            if ( port_str_len == 0 )
            {
                ret = snprintf(port_str, PORT_STR_MAX, "port=%d,%d", i, i+1);
            }
            else
            {
                ret = snprintf(&port_str[port_str_len], PORT_STR_MAX - port_str_len, ",%d,%d", i, i+1);
            }

            if ( ret > 0 )
            {
                port_str_len += ret;

                if ( port_str_len >= PORT_STR_MAX )
                {
                    printf("error: port_str_len[%d] exceed max[%d]\n", 
                    port_str_len, PORT_STR_MAX);
                    return;
                }
            }
            else
            {
                printf("error: create port str fail! ret = %d\n", ret);
                return;
            }

            ret = snprintf(&snd_usb_map_str[snd_usb_map_len], MAP_INFO_MAX - snd_usb_map_len, 
                "dev-%d=asym-%d-L\ndev-%d=asym-%d-R\n", 
                port_sum+1, snd_num, port_sum+2, snd_num);

            port_sum += 2;

            if ( ret > 0 )
            {
                snd_usb_map_len += ret;
                if ( snd_usb_map_len >= MAP_INFO_MAX )
                {
                    printf("error: snd_usb_map_len[%d] exceed max[%d]\n", 
                        snd_usb_map_len, MAP_INFO_MAX);
                    return;
                }
            }
            else
            {
                printf("error: create map str fail! ret = %d\n", ret);
                printf("i = %d, snd_num = %d, node_name: %s\n", i, snd_num, arr_node_dev_info[i].node_name);
                return;
            }
        }
    }

    fp = fopen(snd_usb_conf_path, "w+");

    if ( fp != NULL ) 
    {
        fwrite(snd_usb_conf_str, 1, snd_usb_conf_len, fp); 
		fclose(fp);
	}

    printf("%s", snd_usb_conf_str);

    fp = fopen(snd_usb_info_path, "w+");
    if ( fp != NULL ) 
    {
        sprintf(buf, "[usb_sound]\nsum=%d\n", port_sum);
        fwrite(buf, 1, strlen(buf), fp);
        printf("%s", buf);

        port_str[port_str_len++] = '\n';
        port_str[port_str_len] = '\0';
        fwrite(port_str, 1, port_str_len, fp);
        printf("%s", port_str);

        fwrite(snd_usb_map_str, 1, snd_usb_map_len, fp);
        printf("%s", snd_usb_map_str);

		fclose(fp);
	}
}

int main(int argc,char *argv[])
{
    unsigned long i = 0;
    struct dirent *de;
    DIR *sbud = NULL;

    sbud = opendir(sys_bus_usb_devices);
	if (sbud) 
    {
    	while ( (de = readdir(sbud)) )
    		inspect_bus_entry(de->d_name);

        closedir(sbud);
    }

    rebuild_asound_conf();

    return 0;
}

