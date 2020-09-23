#include <stdio.h>
#include <sys/types.h>                                                                                                                                                       
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include<sys/file.h>
#include <sys/epoll.h>

#include "serial.h"

#define SLOT_NBR 	8
#define SERIAL_WR_TIMEOUT 1

#define CMD_IS_SETATR  0x81 //需要resp
#define CMD_IS_GETVER  0x83 //需要resp
#define CMD_IS_REQRST  0x85 //需要resp reset itself
//  -> PC.
#define CMD_IS_DATA    0    //不需要resp
#define IS_REQ_RST_ICC    0x10 //不需要resp
#define IS_VER            0x82 //resp to pc
#define IS_RESP_RST       0x84 //resp to pc at reset
#define SLOT_IS_EMU_CTRL  0x80 //STM32 self;

#define M35_APDU_BACK	"\x2f\xe2"
#define EC20_APDU_BACK	"\x3f\x0"

struct module_apdu_back_s
{
	unsigned char apdu[2];
};

int g_sleep_time=4;

int DataToHex(unsigned char *buff, unsigned short buff_len)
{	
	unsigned short i = 0;
	char buff_msg[1024] = {0};
	
	int len = 0;

	while (i < buff_len)
	{
		sprintf(buff_msg+len, "%02x ", buff[i]);
		len += 3;
		i++;
	}
	printf("[DataToHex]%s \n",buff_msg);
	return len;
}

int ast_running()
{
    if((access("/var/run/asterisk.ctl",F_OK)) ==-1 ) {                                                                                                                       
        return 0;
    }

    return 1;
}

int Emu_test_Lock()
{
	int fd = open("/tmp/lock/Emu_test.lock",O_RDONLY|O_CREAT);
	if(-1 != fd){
		if(-1 ==  flock(fd,LOCK_EX|LOCK_NB)){
			printf("Emu_test is running...\n");
			close(fd);
			return 1;
		}
	}
	return 0;
}

int simemu_running()
{
	int fd = open("/tmp/lock/SimEmuSvr.lock",O_RDONLY|O_CREAT);
	if(-1 != fd){
		if(-1 ==  flock(fd,LOCK_EX|LOCK_NB)){
			printf("SimEmuSvr is running...\n");
			close(fd);
			return 1;
		}
	}
	return 0;
}

int get_config(char* file_path, char* context_name, char* option_name,char *content)
{
	if (file_path == NULL || context_name == NULL || option_name == NULL || content == NULL)
	{
		return -1;
	}

	char buf[1024] = {0};
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	char name[256];
	FILE* fp;

	if( NULL == (fp=fopen(file_path,"r")) ) {
		printf("[ERROR]Can't open %s",file_path);
		return -1;
	}

	while(fgets(buf,1024,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0== strcmp(name,context_name) ) {
						finded=1;
					} else {
						if(finded) 
							finish=1;
					}
				}
				break;
			case '=':
				if(finded && !finish) {
					memcpy(name,buf,i);
					name[i] = '\0';
					if(0==strcmp(name,option_name)) {
						memcpy(name,buf+i+1,len-i-1-1);
						name[len-i-1-1] = '\0';
						sprintf(content,"%s",name);
						fclose(fp);
						return 1;
					}
				}
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	return 0;
}

int check_module(unsigned short board,struct module_apdu_back_s *module_apdu)
{
    int i = 0;
    char context[12];
    char option_name[16];
    for(i = board * SLOT_NBR - SLOT_NBR + 1;i <= board *SLOT_NBR;i++)
    {
        sprintf(option_name,"chan_%d_type",i);
        get_config("/tmp/hw_info.cfg", "channel", option_name, context);
		printf("%s=%s\n",option_name,context);
	
		// 暂时只支持 ec20,ec25,m35 三种模块的测试，m35测试速度最快
        if(0 == strncmp("EC20",context,4) || 0 == strncmp("EC25",context,4)){
			memcpy(module_apdu[(i-1)%SLOT_NBR].apdu,EC20_APDU_BACK,2);
			g_sleep_time = 4;
        }else if(0 == strcmp("Quectel_M35",context) || 0 == strcmp("Quectel_M26",context)){
			memcpy(module_apdu[(i-1)%SLOT_NBR].apdu,M35_APDU_BACK,2);
		}else{
			printf("%s unsupport module:%s\n",option_name,context);
		}
    }

    return 0;

}

int resetSTM32(int handle, unsigned short board)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int ret = -1;
	int trys = 2;
	int i = 0;
	
	buff_req[0] = 0x80;
	buff_req[1] = CMD_IS_REQRST;
	buff_req[2] = 0;
	buff_req[3] = 0;
	for (i = 0; i < trys; i++)
	{
		ret = serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == 0x80 && buff_rsp[1] == IS_RESP_RST && buff_rsp[2] == 0 && buff_rsp[3] == 0)
		{
			printf("resetSTM32[00-%02d]: reset STM32 succ\n", board);
			ret = 0;
			sleep(1);
			break;
		}
		else
		{
			printf("resetSTM32[00-%02d]: reset STM32 error(%d:%s)\n", board, errno, strerror(errno));
			sleep(1);
		}
	}

	return ret;
}

int getMini52Version(int handle, unsigned short slot)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int len = 0;
	int trys = 1;
	int ret = -1;
	int i = 0;
	
	buff_req[0] = (unsigned char)slot;
	buff_req[1] = CMD_IS_GETVER;
	buff_req[2] = 0;
	buff_req[3] = 0;

	for (i = 0; i < trys; i++)
	{
		len = serial_wr_atr(handle, buff_req, 4, buff_rsp, sizeof(buff_rsp), SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == (unsigned char)slot && buff_rsp[1] == IS_VER && len == (buff_rsp[3]+4))
		{
			printf("Mini52[slot:%d] version[%s]\n",  slot, buff_rsp+4);
			ret = 0;
			//Sleep(1000);
			break;
		}
		printf("getMini52Version: get Mini52[slot:%d] version error(%d:%s)\n",  slot, errno, strerror(errno));
		//Sleep(1000);
	}
	
	return ret;
}

int getAllMini52Version(int handle)
{
	unsigned short i = 0;
	int ret = 0;
	int res = 0;
	for (i = 0; i < 8; i++)
	{
		ret = getMini52Version(handle, i);
		if (ret < 0)
		{
			res = 1;
		}
	}
	return res;
}


int getSTM32Version(int fd)
{
	int len = 0;                                                                                                                                                             
    char buff[320] = {0};
    int ret = 0;
	len = serial_wr(fd, (unsigned char *)"\x80\x83\x00\x00", 4, (unsigned char *)buff, sizeof(buff), SERIAL_WR_TIMEOUT);
	if (len > 0)
	{
		if (((unsigned char *)buff)[0] != 0x80 || ((unsigned char *)buff)[1] != 0x82)
		{
			return 1;
		}else{
			printf("Emu version[%s]\n", buff+4);
		}                                                                                                                                                                    
	}
	else
	{
		printf("Read version error:%s\n",strerror(errno));
		return 1;
	}
	return ret;
}
int resetMini52WithoutSleep(int handle,unsigned short slot)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};
	int ret = -1;
	int trys = 1;
	int len = 0;
	int i = 0;
	
	buff_req[0] = (unsigned char)slot;
	buff_req[1] = CMD_IS_REQRST;
	buff_req[2] = 0;
	buff_req[3] = 0;
	for (i = 0; i < trys; i++)
	{
		//ret =
		//socket_pty_wr(csock,
		//buff_req,
		//4,
		//buff_rsp,
		//&len,
		//SERIAL_WR_TIMEOUT);
		ret = serial_wr_atr((unsigned int)handle, buff_req, 4, buff_rsp, len, SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == (unsigned char)slot && buff_rsp[1] == IS_RESP_RST && buff_rsp[2] == 0 && buff_rsp[3] == 0)
		{
			printf("[INFO]resetMini52[%02d]: reset Mini52 succ\n",slot);
			ret = 0;
			break;
		}
		else
		{
			printf("[ERRO]resetMini52[%02d]: reset Mini52 error(%d:%s)\n",slot, errno, strerror(errno));
			return -1;
		}
	}

	return ret;
}

int resetAllMini52(int handle)
{
	unsigned short i = 0;
	int ret = 0;
	int res = 0;
	for (i = 0; i < SLOT_NBR; i++)
	{
		ret = resetMini52WithoutSleep(handle, i);
		if (ret < 0)
		{
			res = -1;
		}
	}
	sleep(1);
	return res;
}

void usage()
{
	printf("emu_test [1/2/3/4] <board no.>\n");
}

static int send_at_command(int chan, int timeout, char *at_cmd, char *result){
	char write_pipe_name[1024] = {0};
	char read_pipe_name[1024] = {0};
	int res = -1;
	int cnt = 2;
	char buf[1024] = {0};
	int write_fd, read_fd;
	int epollfd = -1;
	struct epoll_event events[1],ev;
	int epollnum = 0, fds;
	char *pos = buf;
	fds = epoll_create(1);


	memset(buf, 0, 1024);
	sprintf(write_pipe_name, "/tmp/module_pipe/at-%d-r.pipe", chan);
	sprintf(read_pipe_name, "/tmp/module_pipe/at-%d-w.pipe", chan);

	write_fd = open(write_pipe_name, O_WRONLY);
	if(write_fd < 0){
		printf("open pipe error!\n");
		return -1;
	}


	read_fd = open(read_pipe_name, O_RDONLY|O_NONBLOCK);
	if(read_fd < 0){
		printf("open pipe error!\n");
		return -1;
	}

	while(read(read_fd, buf, 1024) > 0);//读空管道
	
	ev.data.fd = read_fd;
	ev.events = EPOLLIN;
	epoll_ctl(fds, EPOLL_CTL_ADD, read_fd, &ev);

	sprintf(buf, "%s\r\n", at_cmd);

	if(write(write_fd, buf, strlen(buf)) < 0){
		printf("write error\n");
		return -1;
	}

	while(cnt > 0){
		epollnum = epoll_wait(fds, events, 1, timeout*1000);
		if(epollnum > 0){
			res = read(read_fd, pos, 1024);	
			if(strstr(buf, "OK")){
				if(result)
					strcpy(result, buf);
				break;
			}
			pos+=res;
		}else{
			break;	
		}
		cnt--;
	}

	epoll_ctl(fds, EPOLL_CTL_DEL, read_fd, &ev);

	close(write_fd);
	close(read_fd);
	return res;
}

static void rri_cfun_mini(int chan)
{
	char cmd[128] = {0};
	char result[1024] = {0};
	int try_cnt = 2;
#if 0
	sprintf(cmd, "/my_tools/rri_cli at %d AT+CFUN=0 >/dev/null ", chan);
	printf("%s\n", cmd);
	system(cmd);

#else
	while(try_cnt > 0){
		send_at_command(chan, 15, "AT+CFUN=0", NULL);
		send_at_command(chan, 3, "AT+CFUN?", result);
		if(strstr(result, "+CFUN: 0"))
			break;
		try_cnt--;
	}
#endif
}
static void rri_cfun_full(int chan )
{
	char cmd[128] = {0};
#if 0
	printf("%s\n", cmd);
	sprintf(cmd, "/my_tools/rri_cli at %d AT+CFUN=1 >/dev/null ", chan);
	system(cmd);

#else
	send_at_command(chan, 0,"AT+CFUN=1", NULL);
#endif
}

void module_turn_off(int board)
{
	int i = 0;
	char rst_cmd[128];
	for(i = board * SLOT_NBR - SLOT_NBR + 1;i <= board *SLOT_NBR ;i++){
		rri_cfun_mini(i);
	}
}
void module_turn_on(int board)
{
	int i = 0;
	char rst_cmd[128];
	for(i = board * SLOT_NBR - SLOT_NBR + 1;i <= board *SLOT_NBR ;i++){
		rri_cfun_full(i);
	}
}
void sim_disable()
{	
	system("/my_tools/bsp_cli sim disable -1");
}

int set_sim_atr(int handle,unsigned char *sim_atr, unsigned short atr_len)
{
	unsigned char buff_req[320] = {0};
	unsigned char buff_rsp[320] = {0};

	int len = 0;
	int ret = 0;
	int res = 0;
	int i = 0;
	
	if (handle < 0 || sim_atr == NULL)
	{
		return -1;
	}

	buff_req[1] = CMD_IS_SETATR;
	buff_req[2] = 0;
	buff_req[3] = (unsigned char)(atr_len);
	memcpy((char *)(buff_req+4), sim_atr, atr_len);
	len = 4 + atr_len;

	for(;i < SLOT_NBR;i++)
	{
		buff_req[0] = (unsigned char)i;
		memset(buff_rsp, 0, sizeof(buff_rsp));
		ret = serial_wr_atr(handle, buff_req, len, buff_rsp, ret, SERIAL_WR_TIMEOUT);
		if (buff_rsp[0] == (unsigned char)i && buff_rsp[1] == 0x80 && ret == (buff_rsp[3]+4))
		{
 			printf("[INFO]setSimAtr[%02d]: set sim atr succ\n",i);
			res |= 1 << i;
		}
		else
		{
 			printf("[ERRO]setSimAtr[%02d]: set sim atr error(%d:%s)\n",i, errno, strerror(errno));
		}
	}
	return res;
}

int checkEmuData(char *buff, int len)
{
	unsigned short len_body;
	if (buff == NULL || len <= 0)
	{
		return -1;
	}
	len_body = ((unsigned char)buff[2]<<8) + (unsigned char)buff[3];
	if ((len - 4) == len_body)
	{
		return 0;
	}
	return -1;
}

unsigned char emu_test(int emu_no,struct module_apdu_back_s *module_apdu)
{
    int fd = -1;
    int len = 0;
    unsigned char buff[320] = {0};
	char cnt_apdu[SLOT_NBR] = {0};
    unsigned char ret = 0;

	int pos = 0;
	int burst = 0;
	unsigned char *pbuff = NULL;
	int plen = 0;
	unsigned char slot_nbr = 0;
	time_t time_start;

	sprintf((char *)buff,"/dev/opvx/emu/%d",emu_no - 1);
	fd = open_serial((char *)buff, 460800, 0, 8, 1, 0);
    if (fd < 0)
    {
        printf("%s:%s\n",(char *)buff,strerror(errno));
        return 0;
    }
	
	sim_disable();
	module_turn_off(emu_no);

	memset(buff, 0, sizeof(buff));
	
	if(0 != resetSTM32(fd, 0)){
		printf("reset STM32 error\n");
		return 0;
	}
	// get STM32F103 version
	if(0 != getSTM32Version(fd)){
		printf("get STM32 version error\n");
		return 0;
	}
	// reset Mini52
	if(0 != resetAllMini52(fd)){
		printf("reset  mini52 error\n");
		return 0;
	}
	if(0 != getAllMini52Version(fd)){
		printf("get  mini52 version error\n");
		return 0;
	}

	if(0xff != set_sim_atr(fd,(unsigned char *)"\x3b\x9e\x95\x80\x1f\x47\x80\x31\xe0\x73\xee\x21\x1b\x66\x86\x88\x42\x08\x4b\x10\x5c\x00",22)){
		printf("set sim atr error\n");
		return 0;
	}
	sleep(g_sleep_time);
	module_turn_on(emu_no);

	time(&time_start);
	while(ret != 0xff && time(NULL) - time_start < 40)
	{
		len = serial_wr(fd, (unsigned char *)"\x80\x97\x00\x00", 4, (unsigned char *)buff, (unsigned int)sizeof(buff), SERIAL_WR_TIMEOUT);
		if (len > 0)
		{
			if (checkEmuData((char *)buff, len) == 0)
			{
				if ((buff)[0] == 0x80 && (buff)[1] == 0x94)
				{
					pos = 4;
					burst = 1;
				}
				else
				{
					pos = 0;
					burst = 0;
				}
				while (pos < len)
				{
					pbuff = buff + pos;
					if (burst == 1)
					{
						//pbuff = buff + pos;
						plen = (pbuff[2]<<8) + pbuff[3] + 4;
						if (plen > 320)
						{
							printf("[ERRO]CommHdlTask: Data from Emu invalid!!!\n");
							break;
						}
						pos += plen;
					}
					else
					{
						//pbuff = buff;
						plen = len;
						pos += len;
					}
					if ((pbuff)[1] == CMD_IS_DATA || (pbuff)[1] == IS_REQ_RST_ICC)
					{
						slot_nbr = (pbuff)[0];
			
						if (slot_nbr > 7 && slot_nbr != SLOT_IS_EMU_CTRL){
							printf("[ERRO]CommHdlTask slot_nbr(%d) invalid!!!\n", slot_nbr);
						}
						else
						{
							if (slot_nbr != SLOT_IS_EMU_CTRL && pbuff[1] != 0x10) 
							{ // return data
								if(buff[4] == module_apdu[slot_nbr].apdu[0] && buff[5] == module_apdu[slot_nbr].apdu[1]){
									DataToHex(buff, (unsigned short)len);
									if(cnt_apdu[slot_nbr] > 0){
										ret |= 1 << slot_nbr;
									}
								}else if((buff[4] == 0x0 || buff[4] == 0xa0) && buff[5] == 0xa4){
									DataToHex(buff, (unsigned short)len);
									if( 0 == (ret & (1 << slot_nbr))){
										cnt_apdu[slot_nbr]++;
										buff[3] = 0x1;
										buff[4] = buff[5];
										DataToHex(buff,5);
										len = serial_write_n(fd, buff, 5, SERIAL_WR_TIMEOUT);
									}
								}else{
									printf("Not supposed apdu:\n");
									DataToHex(buff,(unsigned short)len);
								}
							}
						}
					}
					else
					{
						if ((pbuff)[0] == 0x80 && (pbuff)[1] == 0x96 && (pbuff)[2] == 0 && (pbuff)[3] == 0 && plen == 4)
						{
							//DataToHex((unsigned char *)buff, (unsigned short)len);
						}
					}
				}
			}
		}
	}
	return ret;
}


int main(int argc,char **argv)
{
	if(argc != 2){
		usage();
		return 0;
	}

	
	struct module_apdu_back_s module_apdu[SLOT_NBR];

	if(0 != Emu_test_Lock()){
		return 0;
	}
	
    if(0 != check_module(atoi(argv[1]),module_apdu)){
        return 0;
    }

	if(ast_running()){
		printf("kill process asterisk\n");
		system("sh /etc/init.d/asterisk stop");
	}

	if(simemu_running()){
		printf("kill process SimEmuSvr\n");
		system("sh /etc/init.d/simemu.sh stop");
	}
	
	unsigned char ret = emu_test(atoi(argv[1]),module_apdu);
	if(0xff == ret){
		printf("\033[32memu[%s] test success\033[0m\n",argv[1]);
	}
	else
	{
		int i = 0 ;
		for(;i < SLOT_NBR;i++){
			if(0 == (ret & (1 << i))){
				printf("\033[31memu[%d-%d] test fail\n\033[0m",atoi(argv[1]),i+1);
			}
		}
	}
	return ret;
}



