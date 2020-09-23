/**********************************************************************************
文件描述  ：获取底板版本信息、
            包括底板硬件版本和底板控制GSM等模块上电、掉电的MCU软件版本。
            获取后通过printf输出到stdout, 其它程序如想得到版本信息只能
            从stdout上获取。
作者/时间 : junyu.yang@openvox.cn/2018.1.15
***********************************************************************************/
#include "public.h"

#if T_DESC("寄存器功能")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bsp_tools.h"


#define CMD_GET_SLOT     "read 58\n"
#define CMD_GET_VERSION  "ver\n"
#define DEV_NAME_LEN		32
#define MCU_DEV_MAX_2U	12
#define MCU_DEV_MAX_1U	6
#define DEV_CMD_GET_NODE	"udevadm info -q path -n %s"
#define DEV_CMD_LEN		64
#define DEV_CMD_RES_LEN	160
#define DEV_SLOT_VS_1008_TYPE	0x50

typedef enum _dev_type_e
{
	DEV_TYPE_UNDEF=0,
	DEV_TYPE_1U = 1,
	DEV_TYPE_2U,
	DEV_TYPE_1008,
	DEV_TYPE_VS2_8X,
}dev_type_e;

typedef enum _dev_name_e
{
	DEV_NAME_UNKOWN=0,
	DEV_NAME_SWG_BASE,
	DEV_NAME_VS_USB,
	DEV_NAME_VS2_X8,
}dev_name_e;
	
typedef struct _mcu_dev_s
{
	char node_name[DEV_NAME_LEN];
	int slot_num;
}mcu_dev_t;

mcu_dev_t mcu_dev_2u[MCU_DEV_MAX_2U]  = 
{
	{"1-1.2.5.1", 1},
	{"1-1.2.6.6.1", 2},
	{"1-1.2.6.5.1", 3},
	{"1-1.2.1.1", 4},
	{"1-1.2.2.1", 5},
	{"1-1.2.6.1.1", 6},
	{"1-1.2.6.2.1", 7},
	{"1-1.2.3.1", 8},
	{"1-1.2.4.1", 9},
	{"1-1.2.6.4.1", 10},
	{"1-1.2.6.3.1", 11},
	{"", 0}
};

mcu_dev_t mcu_dev_1u[MCU_DEV_MAX_2U]  = 
{
	{"1-1.2.5.1", 1},
	{"1-1.2.6.6.1", 0},
	{"1-1.2.6.5.1", 0},
	{"1-1.2.1.1", 2},
	{"1-1.2.2.1", 3},
	{"1-1.2.6.1.1", 0},
	{"1-1.2.6.2.1", 0},
	{"1-1.2.3.1", 4},
	{"1-1.2.4.1", 5},
	{"1-1.2.6.4.1", 0},
	{"1-1.2.6.3.1", 0},
	{"", 0}
};

mcu_dev_t mcu_dev_vs1008[MCU_DEV_MAX_2U]  = 
{
	{"1-1.2.5.1", 0},
	{"1-1.2.6.6.1", 0},
	{"1-1.2.6.5.1", 0},
	{"1-1.2.1.1", 1},
	{"1-1.2.2.1", 2},
	{"1-1.2.6.1.1", 0},
	{"1-1.2.6.2.1", 0},
	{"1-1.2.3.1", 0},
	{"1-1.2.4.1", 0},
	{"1-1.2.6.4.1", 0},
	{"1-1.2.6.3.1", 0},
	{"", 0}
};
/*
mcu_dev_t mcu_dev_1u[MCU_DEV_MAX_1U]  = 
{
	{"1-1.2.5.1", 1},
	{"1-1.2.1.1", 2},
	{"1-1.2.2.1", 3},
	{"1-1.2.3.1", 4},
	{"1-1.2.4.1", 5},
	{"", 0}
};
*/

static int get_slot_num(char *dev_name, int wt, dev_name_e *name)
{
	int fd, ret;
	int baud=9600;
	char buf[DEV_CMD_LEN];
    char *name_swg_1032_base = "SWG1032_BASED";
	char *name_vs2_8x_down = "VS2_8XEC20_DOWN";
    char *name_VS_USB_base = "VS_USB_BASED";

try_again:
	fd = open_serial(dev_name, baud, 0, 8, 1, 0);
	if(fd < 0) 
	{
		perror(dev_name);
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "stty -F %s %d", dev_name, baud);
	system(buf);

	/* 先读清 */
	while (read(fd, buf, sizeof(buf)) > 0)
	{
		printf("[%s %d]\n", __FUNCTION__, __LINE__);
	}
/*******************get version begin********************/
	ret = write(fd, CMD_GET_VERSION, strlen(CMD_GET_VERSION));
	if (ret <= 0)
	{
		perror(dev_name);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	usleep(wt);
	ret = read(fd, buf, sizeof(buf));
	if(ret < 0)
	{
		if(baud == 9600){
			close(fd);
			baud = 115200;
			goto try_again;
		}
		perror(dev_name);
		return -1;
	}
	if(strstr(buf,name_swg_1032_base )){
		*name = DEV_NAME_SWG_BASE;
	}else if(strstr(buf,name_VS_USB_base )){
		*name = DEV_NAME_VS_USB;
	}else if(strstr(buf, name_vs2_8x_down)){
		*name = DEV_NAME_VS2_X8;
	}
	
/*******************get version end**********************/
	ret = write(fd, CMD_GET_SLOT, strlen(CMD_GET_SLOT));
	if (ret <= 0)
	{
		perror(dev_name);
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	usleep(wt);
	ret = read(fd, buf, sizeof(buf));
	if(ret < 0)
	{
		close(fd);
		perror(dev_name);
		return -1;
	}
//	ret = atoi(buf);
	ret = strtol(buf, NULL, 16);
	close_serial(fd);
	if (*name == DEV_NAME_VS_USB) {
		if(ret >= 0 && ret <= 2)
		{
			ret++;
			printf("slot_id : %d \n", ret);

		}
		else if(ret == 3)
		{
			printf("read slot_id error.\n");
			if(*name != DEV_TYPE_VS2_8X)
				ret = -1;
		}
		else if(ret < MCU_DEV_MAX_2U)
		{
			printf("slot_id : %d \n", ret);
		}
		else if (ret > DEV_SLOT_VS_1008_TYPE)
		{
			printf("slot_id : %d \n", ret & 0x3F);
	//		printf("maybe it is vs_usb_1008. \n", ret);
		}
	} else if (*name == DEV_NAME_VS2_X8) {
		printf("slot_id : 0x%x \n", ret);
	}
//	printf("slot_id : %d \n", ret);

	return ret;
}

static int my_exec(char *cmd, char *res)
{
	FILE *fp = NULL;

//	if(cmd == NULL || res == NULL)
//		return -1;
	fp = popen(cmd, "r");
	if(fp == NULL)
	{
		printf("popen %s failed. \n", cmd);
		return -1;
	}
	fread(res, 160, 1, fp);
	pclose(fp);
//	printf("fread: %s \n", res);

	return 0;
}

static int get_dev_type(char *dev_name, int wt)
{
	int ret, i, slot_num, high_opt;
	char cmd_buf[DEV_CMD_LEN] = "\0";
	char cmd_res[DEV_CMD_RES_LEN] = "\0";
	dev_name_e name;
	if(dev_name == NULL)
		return 0;

	slot_num = get_slot_num(dev_name, wt, &name);
	if(slot_num <= 0)
	{
		printf("slot_num[%d] is invalid. dev_name=%s \n", slot_num, dev_name);
		return -1;
	}
	else if(slot_num == 1)
	{
//		return DEV_TYPE_2U;
	}

	high_opt = slot_num & DEV_SLOT_VS_1008_TYPE;
	slot_num = slot_num & 0x0F;

	snprintf(cmd_buf, sizeof(cmd_buf), DEV_CMD_GET_NODE, dev_name);
	memset(cmd_res, 0, DEV_CMD_RES_LEN);
	ret = my_exec(cmd_buf, cmd_res);
	if(ret < 0)
	{
		printf("call my_exec failed. cmd=[%s].\n", cmd_buf);
		return -1;
	}

	ret = DEV_TYPE_UNDEF;
	/**  **/
	if(slot_num > 0) {
		for(i = 1; i < MCU_DEV_MAX_2U; i++)
		{
			if(name == DEV_NAME_VS2_X8){
				ret = DEV_TYPE_VS2_8X;
				break;
			}
			if (strstr(cmd_res, mcu_dev_2u[i].node_name) != NULL)
			{
				if (mcu_dev_2u[i].slot_num == slot_num)
				{
					ret = DEV_TYPE_2U;
					break;
				}
				else if(mcu_dev_1u[i].slot_num == slot_num && high_opt == 0)
				{
					ret = DEV_TYPE_1U;
					break;
				}
				else if(mcu_dev_vs1008[i].slot_num == slot_num && (high_opt > 0))
				{
					ret = DEV_TYPE_1008;
					break;
				}
			}
		}
	}
	if(DEV_TYPE_UNDEF == ret) {
		if (strstr(cmd_res, mcu_dev_1u[0].node_name) != NULL) {
			if (mcu_dev_1u[0].slot_num == slot_num) {
				ret = DEV_TYPE_1U;
			}
		} else if (strstr(cmd_res, mcu_dev_vs1008[3].node_name) != NULL || strstr(cmd_res, mcu_dev_vs1008[4].node_name) != NULL) {
			ret = DEV_TYPE_1008;
		}
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int wt = 200, dev_type;

	if(argc < 2)
	{
		printf("usage: \n"
			"    %s <device path> [wait time]\n"
			"    Note: Absolute path, time in ms(default 100ms)\n", argv[0]);
		return 0;
	} 
	else if( argc >= 3)
	{
		wt = atoi(argv[2]);
	}
	wt *= 1000;

	dev_type = get_dev_type(argv[1], wt);
	if (DEV_TYPE_1U == dev_type || DEV_TYPE_2U == dev_type )
		printf("dev_type : %dU \n", dev_type);
	else if (DEV_TYPE_1008 == dev_type )
		printf("dev_type : VS1008 \n");
	else if(DEV_TYPE_VS2_8X == dev_type){
		printf("dev_type : VS2_8X \n");
	}
	else
		printf("dev_type : %dU \n", dev_type);

	return 0;
}
#endif



