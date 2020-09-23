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


#define CMD_GET_VER     "ver\n"

int main(int argc, char *argv[])
{
    int ret, wt = 200, fd = -1;
	int baud = 9600;
    char buf[128] = {0};

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
try_again:
    fd = open_serial(argv[1], baud, 0, 8, 1, 0);
    if(fd < 0) 
    {
        perror(argv[1]);
        return 0;
    }

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "stty -F %s %d", argv[1], baud);
    system(buf);

    /* 先读清 */
    while (read(fd, buf, sizeof(buf)) > 0)
    {
        printf("[%s %d]\n", __FUNCTION__, __LINE__);
    }

    ret = write(fd, CMD_GET_VER, strlen(CMD_GET_VER));
    if (ret <= 0)
    {
        perror(argv[1]);
        return 0;
    }

    usleep(wt);
    ret = read(fd, buf, sizeof(buf));
    if(ret < 0)
    {
		if(baud == 9600){
			close(fd);
			baud = 115200;
			goto try_again;
		}
        perror(argv[1]);
        return 0;
    }

    /* 直接使用单片机定义的Version格式, 如下，不再重新定义 */
    /* SWG1032_BASED
     * HwVer : V1.0
     * SwVer : V1.0
     * Jan 01 2018
     * 16:12:21 
     */
    printf("%s\n", buf);

    return 0;
}
#endif



