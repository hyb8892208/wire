/**********************************************************************************
�ļ�����  ����ȡ�װ�汾��Ϣ��
            �����װ�Ӳ���汾�͵װ����GSM��ģ���ϵ硢�����MCU����汾��
            ��ȡ��ͨ��printf�����stdout, ������������õ��汾��Ϣֻ��
            ��stdout�ϻ�ȡ��
����/ʱ�� : junyu.yang@openvox.cn/2018.1.15
***********************************************************************************/
#include "public.h"

#if T_DESC("�Ĵ�������")

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

    /* �ȶ��� */
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

    /* ֱ��ʹ�õ�Ƭ�������Version��ʽ, ���£��������¶��� */
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



