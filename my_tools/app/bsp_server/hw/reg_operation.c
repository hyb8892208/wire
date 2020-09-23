/**********************************************************************************
文件描述  ：获取底板版本信息、
            包括底板硬件版本和底板控制GSM等模块上电、掉电的MCU软件版本。
            获取后通过printf输出到stdout, 其它程序如想得到版本信息只能
            从stdout上获取。
作者/时间 : junyu.yang@openvox.cn/2018.1.15
***********************************************************************************/
#include "../common/public.h"

#include "../common/bsp_tools.h"

#include "../bsp_cli/cli.h"

#if T_DESC("寄存器功能")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX (128)

int fd = 0;
int wt = 200*1000;
int cli_usage(int argc, char **argv){
	printf("ver\t\t:ver\n");
	printf("read  reg\t:read <regaddr>\n");
	printf("read  bit\t:read <regaddr.bit>\n");
	printf("read  regs\t:read <regaddr-num>\n");
	printf("write reg\t:write <regaddr=value>\n");
	printf("write bit\t:write <regaddr.bit=value>\n\n");
	printf("eg:  \t\tread 3\n");
	printf(" \t\tread 3.0\n");
	printf(" \t\tread 0-03h\n");
	printf("\t\twrite 0=01h\n");
	printf("\t\twrite 0.1=1\n");
	return 0;
}

int reg_operation_init(char *dev_name){
	
	char buf[MAX] = {0};
	fd = open_serial(dev_name, 9600, 0, 8, 1, 0);
	if(fd < 0) 
	{
		perror(dev_name);
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "stty -F %s 9600", dev_name);
	system(buf);
	return 0;
}
int reg_operation_deinit(void){
	
	close(fd);
	return 0;
}

int reg_operation_write(char *data, int len){
	char buf[MAX] = {0};
	while(read(fd, buf, sizeof(buf)) > 0);

	if(write(fd, data, len) != len){
		perror("write data");
		return -1;
	}
	return 0;
}

int reg_operation_read(char *data, int len, char *result){
	char buf[MAX] = {0};
	while(read(fd, buf, sizeof(buf)) > 0);

	if(write(fd, data, len) != len){
		perror("write data");
		return -1;
	}
	usleep(wt);
	if(read(fd, result, 128) < 0){
		perror("read data");
	}
	return 0;
}

int cli_reg_operation(int argc, char **argv){
	char cmd_buf[MAX] = {0};
	char result[MAX] = {0};
	if(argc == 1 && strcmp(argv[0], "ver") == 0){
		sprintf(cmd_buf, "%s\n", argv[0]);
		if(reg_operation_read(cmd_buf, strlen(cmd_buf), result) == 0){
			printf("%s\n", result);
		}
	}else if(argc == 2 && strcmp(argv[0], "read") == 0){
		sprintf(cmd_buf, "%s %s\n", argv[0], argv[1]);
		if(reg_operation_read(cmd_buf, strlen(cmd_buf), result) == 0){
			printf("%s\n", result);
		}
	}else if(argc == 2 && strcmp(argv[0], "write") == 0){
		sprintf(cmd_buf, "%s %s\n", argv[0], argv[1]);
		reg_operation_write(cmd_buf, strlen(cmd_buf));
	}else{
		cli_usage(0, NULL);
	}
	return 0;
}

int main(int argc, char **argv)
{
	char prompt[MAX] = {0};
	char *pos = NULL;
	if(argc < 2)
	{
		printf("usage: \n"
		       "    %s <device path> [wait time]\n"
		       "    Note: Absolute path, time in ms(default 100ms)\n", argv[0]);
		return 0;
	} 
	if(reg_operation_init(argv[1]) < 0){
		return -1;
	}
	if(argc == 2){
		pos = strstr(argv[1], "tty");
		if(pos)
			strcpy( prompt, pos+3);
		else
			strcpy(prompt, "USB");
		strcat(prompt, ">");
		cli_init();
		cb_func_reg("?", cli_usage);
		cb_func_reg("help", cli_usage);
		cb_func_reg("read", cli_reg_operation);
		cb_func_reg("write", cli_reg_operation);
		cb_func_reg("ver", cli_reg_operation);
		
		run_main(prompt);
		reg_operation_deinit();
	}else{
		cli_reg_operation(argc -2, argv+2);
	}
	return 0;

}
#endif