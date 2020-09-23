/*************************************************************************
    * File Name: probe_emu.c
    * Author: wengang.mu
    * Mail: wengang.mu@openvox.com 
    * Created Time: Fri 09 Aug 2019 05:45:46 PM CST
 ************************************************************************/
#include "typedef.h"
#include "ahead.h"
#include "serial.h"

#define EMU_MAX 11
#define EMU_DEV_PATH "/dev/opvx/emu/"

int write_cmd(int fd, char *cmd, int cmd_len, char *result, int len){
    char tmp[256];
    if(fd < 0)
        return -1; 
    while(read(fd, tmp, 256) >0);

    write(fd, cmd, cmd_len); 
    usleep(20000);
	if(read(fd, result, len) < 0){ 
		return -1; 
	}   

    return 0;
}

int main(int argc, char **argv){
	char emu_path[256] = {0};
	int i = 0, j = 0;
	int fd;
	char emu_cmd[4] = {0x00,0x83,0x00,0x00};
	char buff_rsp[320] = {0};
	for(i = 0; i < EMU_MAX; i++){
		sprintf(emu_path, "%s%d", EMU_DEV_PATH, i);
		if(access(emu_path, F_OK) != 0){
			continue;
		}

		fd = open_serial(emu_path, 460800, 0, 8, 1, 0);
		if(fd < 0){
			continue;
		}

		emu_cmd[0] = 0x80;
		if(write_cmd(fd, emu_cmd, 4, buff_rsp, sizeof(buff_rsp)) == 0){

			if((buff_rsp[0]&0xFF) == 0x80 && (buff_rsp[1]&0xFF) == 0x82){
				printf("emu[%d] DETECTED, version: %s\n",i+1, buff_rsp+4);
			}else{
				printf("emu[%d] UNDETECTED\n", i+1);
			}

			for(j = 0; j < 8; j++){
				emu_cmd[0] = j;
				if(write_cmd(fd, emu_cmd, 4, buff_rsp, sizeof(buff_rsp)) == 0){
					if((buff_rsp[0] & 0xFF) == j && (buff_rsp[1] & 0xFF) == 0x82){
						printf("mini52[%d][%d] DETECTED, version: %s\n", i + 1, j + 1,buff_rsp+4);
					}else{
						printf("mini52[%d][%d] UNDETECTED\n", i + 1, j + 1);
					}
				}
				else{
					printf("mini52[%d][%d] UNDETECTED\n", i + 1, j + 1);
				}

			}
		}else{
			printf("emu[%d] UNDETECTED\n", i+1);
		}
		printf("\n");
		close_serial(fd);
	}
	return 0;
}

