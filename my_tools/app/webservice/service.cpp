/*
 * $Id: service.cpp,v 1.5 2007/07/02 18:48:19 sebdiaz Exp $ 
 *
 *  Copyright (C) 1996 - 2004 Stephen F. Booth
 *                       2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
 *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
 *
 */


#include <exception>
#include <iostream>
#include <string>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>

#include <signal.h> 

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>
#include <sys/mman.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "common.h"
#include "hiredis.h"

using namespace std;
using namespace cgicc;

#define EXEC_TIMEOUT 60 //execut cmmoand timeout.

int debug = 0x1; // level 0, 1, 2
char *global_debug_logfile = "/var/log/lighttpd/fastcgi.log";
FCgiIO *global_debug_io = NULL;
struct service_info global_service;

int ast_running()
{
	if((access("/var/run/asterisk.ctl",F_OK)) ==-1 ) {
		return 0; 
	}

	return 1;
}

char* __cut_str(char* buf, char* start, char* end, char* save, int size)
{
	char *findstart, *findend;
	char *ret;
	int len;
	save[0] = '\0';

	if (buf == NULL) {
		return NULL;
	}

	findstart = strstr(buf,start);
	if (!findstart) {
		return NULL;
	}
	findstart += strlen(start);

	//trim left
	while ( (*findstart == '\r') || (*findstart == '\t') || (*findstart == ' ') ) {
		findstart++;
	}

	if (end == NULL) {
		findend = buf + strlen(buf);
		ret = NULL;
	} else {
		findend = strstr(findstart,end);
		if (!findend) {
			return NULL;
		}
		ret = findend;
	}

	//trim right
	while ( (findend > findstart) && (*(findend-1) == '\n') || (*(findend-1) == '\r') || (*(findend-1) == '\t') || (*(findend-1) == ' ') ) {
		findend--;
	}

	len = findend - findstart;
	if (len < 0 || len >= size ) {
		return ret; //finded ""
	}

	strncpy(save,findstart,len);
	save[len] = '\0';
	return ret;

//	char format[64];
//	snprintf(format,sizeof(format),"%s%%%ds%s",start,size,end);
//	sscanf(buf,format,save);
}

void get_cellsinfo(char* outbuf, int outbuflen)
{
	char *file_path = "/tmp/auto_lock_cell/cells_info";
	int len;
	int fd;
	char *p;
	
	
	if ((fd = open(file_path,O_RDONLY)) < 0) {
		return;
	}
	flock(fd,LOCK_EX);
	p = outbuf;
	while( (outbuflen > 0) && (len = read(fd,p,outbuflen) > 0) ) {
		//p[len] = '\0';
		p += len;
		outbuflen -= len;
	}

	flock(fd,LOCK_UN);
	close(fd);
}

struct gsm_info {
	int port;
	char dchannel[32];
	char status[64];
	char type[32];
	char manufacturer[32];
	char model_name[32];
	char model_imei[32];
	char revision[64];
	char network_name[64];
	char network_status[64];
	char signal[8];
	char ber[8];
	char sim_imsi[32];
	char sim_sms_center_number[32];
	char own_number[32];
	char pdd[32];
	char asr[32];
	char acd[32];
	char last_event[64];
	char state[128];
	char limit[128];
	char last_send_at[64];
	char show_status[64];
	char remain_time[16];
	char band[32];
	char day_remain_calls[16];
};

void gsm_output_json(struct gsm_info* info, char* outbuf, int outbuflen)
{
	snprintf( outbuf,outbuflen,
			"\"%d\":[{\"port\":%d,"
			"\"d-channel\":\"%s\","
			"\"status\":\"%s\","
			"\"type\":\"%s\","
			"\"manufacturer\":\"%s\","
			"\"model_name\":\"%s\","
			"\"model_imei\":\"%s\","
			"\"revision\":\"%s\","
			"\"operator\":\"%s\","
			"\"register\":\"%s\","
			"\"signal\":\"%s\","
			"\"ber\":\"%s\","
			"\"sim_imsi\":\"%s\","
			"\"sim_sms_center_number\":\"%s\","
			"\"own_number\":\"%s\","
			"\"remain_time\":\"%s\","
			"\"pdd\":\"%s\","
			"\"asr\":\"%s\","
			"\"acd\":\"%s\","
			"\"last_event\":\"%s\","
			"\"state\":\"%s\","
			"\"limit\":\"%s\","
			"\"last_send_at\":\"%s\","
			"\"show_status\":\"%s\","
			"\"band\":\"%s\","
			"\"day_remain_calls\":\"%s\"}],",
			info->port,info->port,
			info->dchannel,
			info->status,
			info->type,
			info->manufacturer,
			info->model_name,
			info->model_imei,
			info->revision,
			info->network_name,
			info->network_status,
			info->signal,
			info->ber,
			info->sim_imsi,
			info->sim_sms_center_number,
			info->own_number,
			info->remain_time,
			info->pdd,
			info->asr,
			info->acd,
			info->last_event,
			info->state,
			info->limit,
			info->last_send_at,
			info->show_status,
			info->band,
			info->day_remain_calls);
}

void gsminfo_network_name_to_operator(char *network_name, int bufsize){
	if(strcmp(network_name, "CHINA MOBILE") == 0 ||
	   strcmp(network_name, "CHINA UNICOM") == 0 ||
	   strcmp(network_name, "CHINA TELECOM")== 0){
		return;
	}
	else if(strcmp(network_name, "CHN-CT") == 0){
		memset(network_name, 0, bufsize);
		strncpy(network_name, "CHINA TELECOM", bufsize);
	}else if(strcmp(network_name, "CHN-UNICOM") == 0){
		memset(network_name, 0, bufsize);
		strncpy(network_name, "CHINA UNICOM", bufsize);
	}else if(strcmp(network_name, "CHN-MOBILE") == 0){
		memset(network_name, 0, bufsize);
		strncpy(network_name, "CHINA MOBILE", bufsize);
	}
}
void exec_syscmd(const char *cmd, char *result, int resultlen){
	result[0] = '\0';
	FILE *handle = NULL;
	handle = popen(cmd, "r");
	if(handle == NULL)
		return ;
	if(fread(result, 1, resultlen, handle) < 0)
		return ;
	pclose(handle);
}
void get_day_remain_calls(char *buf, unsigned int buflen){
	const char *cmd = "/my_tools/calllimit_cli show chn dayremaincalls -1";
	exec_syscmd(cmd,buf, buflen);
}
/***************
[1]
day_remain_calls=30
[2]
day_remain_calls=20
...
****************/
void gsm_info_get_day_remain_count(char *inbuf,char *key, int port,  char *outbuf, int outbuflen){
	char context[32] = {0};
	char *pos, *begin, *end;
	int act_len = 0;
	memset(outbuf, '\0', outbuflen);
	if(strlen(inbuf) ==0)
		return;
	sprintf(context, "[%d]", port);
	pos = strstr(inbuf, context);
	if(pos){
		begin = strstr(pos, key);
		if(begin)
			end = strchr(begin, '\n');
		if(begin && end){
			begin = begin + strlen(key);//skip the key
			act_len = end - begin > outbuflen ? outbuflen:(end-begin);
			strncpy(outbuf, begin, end-begin);
		}
	}
	if(strlen(outbuf) == 0 )
		strcpy(outbuf, "No Limit");
}

void get_gsminfo(char* outbuf, int outbuflen)
{
/* 
Format:  
---Start Span 1---
D-channel: 2
Status: Power on, Provisioned, Up, Active, Standard
Type: CPE
Manufacturer: SIMCOM_Ltd
Model Name: SIMCOM_SIM840W
Model IMEI: 353511028445509
Revision: Revision:1116B01SIM840W16_MXIC_OPENVOX
Network Name: CHINA MOBILE
Network Status: Registered (Home network)
Signal Quality (0,31): 15
BER value (0,7): 0
SIM IMSI: 460028893042082
SIM SMS Center Number: +8613800755500
Remain Time: 0
PDD: 0
ASR: 0
ACD: 0
Last event: D-Channel Up
State: READY
Limit: Call, Sms, Locked, Marked
Last send AT:
---End Span 1---
*/
	char remain_calls_buf[2048] = {0};
	get_day_remain_calls(remain_calls_buf, sizeof(remain_calls_buf));
	if(ast_running()) {
		char filepath[32];
		DIR *dir;
		struct dirent *ptr;
		if ((dir=opendir("/tmp/gsm")) == NULL)
		{
			perror("Open dir error...");
			return ;
		}
		while ((ptr=readdir(dir)) != NULL)
		{
			char buf[1024];
			char *p;
			int len;
			int fd;

			if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)	 ///current dir OR parrent dir
				continue;
			snprintf(filepath,sizeof(filepath),"/tmp/gsm/%s",ptr->d_name);
			fd = open(filepath,O_RDONLY);
			if (fd < 0) {
				continue;
			}
			flock(fd,LOCK_EX);
			len = read(fd,buf,sizeof(buf));
			flock(fd,LOCK_UN);
			close(fd);
			if (len > 0) {
				buf[len] = '\0';
				struct gsm_info info;
				info.port = atoi(ptr->d_name);

				p = buf;
				p = __cut_str(p,"D-channel:","\n",info.dchannel,sizeof(info.dchannel));
				p = __cut_str(p,"Status:","\n",info.status,sizeof(info.status));
				p = __cut_str(p,"Type:","\n",info.type,sizeof(info.type));
				p = __cut_str(p,"Manufacturer:","\n",info.manufacturer,sizeof(info.manufacturer));
				p = __cut_str(p,"Model Name:","\n",info.model_name,sizeof(info.model_name));
				p = __cut_str(p,"Model IMEI:","\n",info.model_imei,sizeof(info.model_imei));
				p = __cut_str(p,"Revision:","\n",info.revision,sizeof(info.revision));
				
				p = __cut_str(p,"Network Name:","\n",info.network_name,sizeof(info.network_name));
				gsminfo_network_name_to_operator(info.network_name, sizeof(info.network_name));

				p = __cut_str(p,"Network Band:","\n",info.band,sizeof(info.band));
				p = __cut_str(p,"Network Status:","\n",info.network_status,sizeof(info.network_status));
				p = __cut_str(p,"Signal Quality (0,31):","\n",info.signal,sizeof(info.signal));
				p = __cut_str(p,"BER value (0,7):","\n",info.ber,sizeof(info.ber));
				p = __cut_str(p,"SIM IMSI:","\n",info.sim_imsi,sizeof(info.sim_imsi));
				p = __cut_str(p,"SIM SMS Center Number:","\n",info.sim_sms_center_number,sizeof(info.sim_sms_center_number));
				p = __cut_str(p,"Own Number:","\n",info.own_number,sizeof(info.own_number));
				p = __cut_str(p,"Remain Time:","\n",info.remain_time,sizeof(info.remain_time));
				p = __cut_str(p,"PDD:","\n",info.pdd,sizeof(info.pdd));
				p = __cut_str(p,"ASR:","\n",info.asr,sizeof(info.asr));
				p = __cut_str(p,"ACD:","\n",info.acd,sizeof(info.acd));
				p = __cut_str(p,"Last event:","\n",info.last_event,sizeof(info.last_event));
				p = __cut_str(p,"State:","\n",info.state,sizeof(info.state));
				p = __cut_str(p,"Limit:","\n",info.limit,sizeof(info.limit));
				p = __cut_str(p,"Last send AT:",NULL,info.last_send_at,sizeof(info.last_send_at));
				
				gsm_info_get_day_remain_count(remain_calls_buf,"day_remain_calls=", info.port ,info.day_remain_calls, sizeof(info.day_remain_calls));
				
				if(strstr(info.status,"Power off")) {
					strncpy(info.show_status,"<img src=\\\"../../images/nosim.gif\\\"/>",sizeof(info.show_status));
				} else if(strstr(info.status,"Power on, Provisioned, Down")) {
					strncpy(info.show_status,"<img src=\\\"../../images/nosim.gif\\\"/>",sizeof(info.show_status));
				} else if(strstr(info.status,"Undetected SIM Card")) {
					info.network_name[0] = 0;
					strncpy(info.network_status,"Undetected SIM Card",sizeof(info.network_status));
					info.state[0] = 0;
					strncpy(info.show_status,"<img src=\\\"../../images/nosim.gif\\\"/>",sizeof(info.show_status));
				} else if(strstr(info.status,"Power on, Provisioned, Up") ||
						  strstr(info.status,"Power on, Provisioned, Block") ||
						  strstr(info.status,"Power on, Provisioned, In Alarm, Up")) {
					int signal = atoi(info.signal);
					if(signal <= 0) {
						strncpy(info.show_status,"<img src=\\\"../../images/wifi0.gif\\\"/>",sizeof(info.show_status));
					} else if (signal <= 5) {
						strncpy(info.show_status,"<img src=\\\"../../images/wifi1.gif\\\"/>",sizeof(info.show_status));
					} else if (signal <= 10) {
						strncpy(info.show_status,"<img src=\\\"../../images/wifi2.gif\\\"/>",sizeof(info.show_status));
					} else if (signal <= 15) {
						strncpy(info.show_status,"<img src=\\\"../../images/wifi3.gif\\\"/>",sizeof(info.show_status));
					} else if (signal <= 20) {
						strncpy(info.show_status,"<img src=\\\"../../images/wifi4.gif\\\"/>",sizeof(info.show_status));
					} else if (signal <= 31) {
						strncpy(info.show_status,"<img src=\\\"../../images/wifi5.gif\\\"/>",sizeof(info.show_status));
					} else {
						strncpy(info.show_status,"<img src=\\\"../../images/wifi0.gif\\\"/>",sizeof(info.show_status));
					}
				} else {
					info.show_status[0] = '\0';
				}

				if(strlen(info.remain_time) <= 0) {
					strncpy(info.remain_time, "No Limit", sizeof(info.remain_time));
				}

				gsm_output_json(&info,buf,sizeof(buf));
				strncat(outbuf,buf,outbuflen - strlen(outbuf));
			}
		}		
		closedir(dir);
	}
}


void process_log(FCgiIO* output, const char* log_type, const char* method, const char* size, const char* port)
{
	//Test
	//Update
	//http://172.16.99.1/service?action=process_log&log_type=at_log&method=update&port=1&size=100
	//Delete
	//http://172.16.99.1/service?action=process_log&log_type=at_log&method=delete&port=1
	//Get contents
	//http://172.16.99.1/service?action=process_log&log_type=at_log&method=getcontents&port=1

	char logfile[256];

	if (log_type == NULL) {
		return;
	}

	if(!strcasecmp(log_type, "at_log")) {
		if ( port == NULL ) {
			return;
		}

		int nport;
		nport = atoi(port);
		if (nport <= 0 || nport > __GSM_SUM__) {
			return;
		}
		snprintf(logfile, sizeof(logfile), "/var/log/asterisk/at/%s",port);
	} else if(!strcasecmp(log_type,"emu_log")){
		if ( port == NULL ) {
			return;
		}

		if(*port <= '0' || *port > __GSM_SUM__ + '0'){
			return;
		}
		snprintf(logfile, sizeof(logfile), "/tmp/log/SimEmuSvr/%s",port);
	}else if(!strcasecmp(log_type, "sip_log")) {
		strncpy(logfile,"/var/log/asterisk/sip-log",sizeof(logfile));
	} else if(!strcasecmp(log_type, "iax_log")) {
		strncpy(logfile,"/var/log/asterisk/iax2-log",sizeof(logfile));
	} else if(!strcasecmp(log_type, "ast_log")) {
		strncpy(logfile,"/tmp/log/asterisk/log4gw",sizeof(logfile));
	} else if(!strcasecmp(log_type, "sys_log")) {
		strncpy(logfile,"/data/log/sys-log",sizeof(logfile));
	} else if(!strcasecmp(log_type, "bts_log")) {
		strncpy(logfile,"/var/log/auto_lock_cell.log",sizeof(logfile));
	} else if(!strcasecmp(log_type, "auto_intercall_log")) {
		strncpy(logfile,"/var/log/auto_intercall.log",sizeof(logfile));
	} else if(!strcasecmp(log_type, "auto_intersms_log")) {
		strncpy(logfile,"/var/log/auto_intersms.log",sizeof(logfile));
    } else if(!strcasecmp(log_type, "bsp_log")){
        strncpy(logfile,"/tmp/log/bsp_server.log", sizeof(logfile));
    } else if(!strcasecmp(log_type, "rri_log")){
        strncpy(logfile,"/tmp/log/rri_server.log", sizeof(logfile));
    } else if(!strcasecmp(log_type, "rri_pipe_log")){
        if ( port == NULL){
            return ;
        }
        int hw_port;
        hw_port = atoi(port);
        if( hw_port <= 0 || hw_port > __GSM_SUM__){
            return;
        }
        snprintf(logfile, sizeof(logfile), "/tmp/module_pipe/%d-%d-at.log", (hw_port-1)/2, hw_port);
        if(access(logfile, F_OK) != 0){
            snprintf(logfile, sizeof(logfile), "/tmp/module_pipe/%s-%d-at.log", "opvx_chan_unkown", hw_port);
        }
	} else {
		return;
	}

	if(method == NULL) {
		return;
	}

	if(!strcasecmp(method, "clean")) {
		truncate(logfile, 0);
		*output << "0&";
		return;
	} else 	if(!strcasecmp(method, "getcontents")) {
		int fd;
		char *pfile;
		int flength;

		fd = open(logfile, O_RDONLY);
		if (fd < 0) {
			return;
		}

		flength = lseek(fd, 0, SEEK_END);

		if (flength <= 0) {
			close(fd);
			return;
		}

		if(lseek(fd, 0, SEEK_SET) < 0) {
			close(fd);
			return;
		}	

		if ((pfile = (char *)mmap(0, flength, PROT_READ, MAP_SHARED, fd, 0)) == ((void *)-1)) {
			close(fd);
			return;
		}

		*output << pfile;
		munmap((void *)pfile, flength);

		close(fd);
	} else 	if(!strcasecmp(method, "update")) {

		if (size == NULL) {
			return;
		}

		int nsize;
		nsize = atoi(size);

		if (nsize < 0 ) {
			return;
		}

		int fd;
		char *pfile;
		int flength;

		fd = open(logfile, O_RDONLY);
		if (fd < 0) {
			return;
		}

		if(flock(fd,LOCK_EX) < 0) {
			close(fd);
			return;
		}

		flength = lseek(fd, 0, SEEK_END);

		if (flength <= 0) {
			flock(fd,LOCK_UN);
			close(fd);
			return;
		}

		if (flength == nsize) {
			flock(fd,LOCK_UN);
			close(fd);
			*output << flength << "&";
			return;
		} else if(flength > nsize) {
			if(lseek(fd, nsize, SEEK_SET) < 0) {
				flock(fd,LOCK_UN);
				close(fd);
				return;
			}

			char readbuf[1024+1];
			int readlen;
			char tmp[32];

			*output << flength << "&";

			while ((readlen = read(fd, readbuf, sizeof(readbuf)-1)) > 0) {
				readbuf[readlen] = '\0';
				*output << readbuf;
			}

			flock(fd,LOCK_UN);
			close(fd);
			return;
		} else {
			if(lseek(fd, 0, SEEK_SET) < 0) {
				flock(fd,LOCK_UN);
				close(fd);
				return;
			}

			if ((pfile = (char *)mmap(0, flength, PROT_READ, MAP_SHARED, fd, 0)) == ((void *)-1)) {
				close(fd);
				return;
			}

			*output << flength << "&";
			*output << pfile;
			munmap((void *)pfile, flength);

			flock(fd,LOCK_UN);
			close(fd);
			return;

		}
	}
}

void exec_astcmd(FCgiIO* output,const char *cmd)
{
	char cmd_copy[1024];
	char astcmd[1024];
	FILE *stream;
	char readbuf[1024+1];
	int len;
	time_t stime;

	strncpy(cmd_copy,cmd,sizeof(cmd_copy));

	snprintf(astcmd,sizeof(astcmd),"asterisk -rx \"%s\" 2> /dev/null",cmd_copy);

	stime = time(NULL);

	if( (stream = popen(astcmd, "r")) == NULL ) {
		return;
	}

	while( (len = fread(readbuf,1,sizeof(readbuf)-1,stream)) > 0 ) {
		readbuf[len] = '\0';
		*output << readbuf;
		output->flush();

		if ((time(NULL) - stime) > EXEC_TIMEOUT) {
			break;
		}
	}
	pclose(stream);
}

void exec_atcmd(FCgiIO* output,const char *cmd, const char *span, const char* timeout, const char* reportkeyword)
{
	if (cmd == NULL || span == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	char astcmd[1024];
	char newcmd[1024];
	int i,j;

	for (i = 0, j = 0; cmd[i] != '\0' && j < sizeof(newcmd)-3; i++, j++) {
		if (cmd[i] == '"') {
			newcmd[j++] = '\\';
			newcmd[j++] = '\\';
			newcmd[j++] = '\\';
			newcmd[j] = '"';
		} else {
			newcmd[j] = cmd[i];
		}
	}
	newcmd[j] = '\0';

	if (reportkeyword) {
		snprintf(astcmd,sizeof(astcmd),"gsm send sync at %s %s %s %s", span, newcmd, timeout, reportkeyword);
	} else {
		snprintf(astcmd,sizeof(astcmd),"gsm send sync at %s %s %s", span, newcmd, timeout);
	}
	exec_astcmd(output,astcmd);
}


void get_cfinfo(FCgiIO* output, const char *span, const char* timeout)
{
	if (span == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	char astcmd[1024];
	FILE *stream;
	char readbuf[1024+1];
	char allbuf[1024+1];
	int len;
	time_t stime;

	allbuf[0] = '\0';

	snprintf(astcmd,sizeof(astcmd),"asterisk -rx \"gsm show cfinfo %s %s\" 2> /dev/null",span, timeout);

	stime = time(NULL);

	if( (stream = popen(astcmd, "r")) == NULL ) {
		return;
	}

	while( (len = fread(readbuf,1,sizeof(readbuf)-1,stream)) > 0 ) {
		readbuf[len] = '\0';
		strncat(allbuf, readbuf, sizeof(allbuf) - strlen(allbuf) - 1);

		if ((time(NULL) - stime) > EXEC_TIMEOUT) {
			break;
		}
	}
	pclose(stream);

	if (allbuf[0] != '\0') {

		if (!strncmp(allbuf,"NOT READY",sizeof("NOT READY")-1)) {
			*output << "{\"Unconditional\":\"" << "NOT READY" << "\"";
			*output << ",\"Busy\":\"" << "NOT READY" << "\"";
			*output << ",\"NoReply\":\"" << "NOT READY" << "\"";
			*output << ",\"NotReachable\":\"" << "NOT READY" << "\"}";
		} else {
			char unconditional[64],busy[64],noreply[64],notreach[64];
			if(sscanf(allbuf,"Unconditional:%128[^\n]\nBusy:%128[^\n]\nNo Reply:%128[^\n]\nNot Reachable:%128[^\n]\n",unconditional,busy,noreply,notreach) == 4) {
				*output << "{\"Unconditional\":\"" << unconditional << "\"";
				*output << ",\"Busy\":\"" << busy << "\"";
				*output << ",\"NoReply\":\"" << noreply << "\"";
				*output << ",\"NotReachable\":\"" << notreach << "\"}";
			}
		}
	}
}

void set_cf_uncond(FCgiIO* output, const char *span, const char* number, const char* timeout)
{
	if (span == NULL || number == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	char astcmd[1024];

	snprintf(astcmd,sizeof(astcmd),"gsm set cf uncond %s %s %s",span, number, timeout);
	exec_astcmd(output,astcmd);
}


void set_cf_cond(FCgiIO* output, const char *span, const char* busy_num, const char* noreply_num, const char* notreach_num, const char* timeout)
{
	if (span == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	if (busy_num == NULL) {
		busy_num = "";
	}

	if (noreply_num == NULL) {
		noreply_num = "";
	}

	if (notreach_num == NULL) {
		notreach_num = "";
	}

	char astcmd[1024];

	snprintf(astcmd,sizeof(astcmd),"gsm set cf cond %s \\\"%s\\\" \\\"%s\\\" \\\"%s\\\" %s",span, busy_num, noreply_num, notreach_num, timeout);
	exec_astcmd(output,astcmd);
}

void set_cf_disable(FCgiIO* output, const char *span, const char* timeout)
{
	if (span == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	char astcmd[1024];

	snprintf(astcmd,sizeof(astcmd),"gsm set cf disable %s %s",span, timeout);
	exec_astcmd(output,astcmd);
}

void get_cwinfo(FCgiIO* output, const char *span, const char* timeout)
{
	if (span == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	char astcmd[1024];
	FILE *stream;
	char readbuf[1024+1];
	char allbuf[1024+1];
	int len;
	time_t stime;

	allbuf[0] = '\0';

	snprintf(astcmd,sizeof(astcmd),"asterisk -rx \"gsm show cwinfo %s %s\" 2> /dev/null",span, timeout);

	stime = time(NULL);

	if( (stream = popen(astcmd, "r")) == NULL ) {
		return;
	}

	while( (len = fread(readbuf,1,sizeof(readbuf)-1,stream)) > 0 ) {
		readbuf[len] = '\0';
		strncat(allbuf, readbuf, sizeof(allbuf) - strlen(allbuf) - 1);

		if ((time(NULL) - stime) > EXEC_TIMEOUT) {
			break;
		}
	}
	pclose(stream);

	if (allbuf[0] != '\0') {
		if(strcmp(allbuf,"ON\n") == 0) {
			*output << "{\"callwaiting\":\"ON\"}";
		} else if(strcmp(allbuf,"OFF\n") == 0) {
			*output << "{\"callwaiting\":\"OFF\"}";
		} else if(strcmp(allbuf,"TIMEOUT\n") == 0) {
			*output << "{\"callwaiting\":\"TIMEOUT\"}";
		} else if(strcmp(allbuf,"NOT READY\n") == 0) {
			*output << "{\"callwaiting\":\"NOT READY\"}";
		} else {
			*output << "{\"callwaiting\":\"UNKNOWN\"}";
		}
	}
}

void set_cw(FCgiIO* output, const char *span, const char* timeout, int sw)
{
	if (span == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	char astcmd[1024];
	snprintf(astcmd,sizeof(astcmd),"gsm set cw %s %s %s", sw ? "on" : "off", span, timeout);
	exec_astcmd(output,astcmd);
}

void get_curcellinfo(FCgiIO* output, const char *span, const char *timeout, const char *reset)
{
	if (span == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	char astcmd[1024];
	FILE *stream;
	char readbuf[1024+256];
	char allbuf[1024+256];
	int len;
	time_t stime;

	allbuf[0] = '\0';

	snprintf(astcmd,sizeof(astcmd),"asterisk -rx \"gsm show curcells %s %s %s json\" 2> /dev/null",
			 span, timeout, reset ? "reset" : "normal");

	stime = time(NULL);

	if( (stream = popen(astcmd, "r")) == NULL ) {
		return;
	}

	while( (len = fread(readbuf,1,sizeof(readbuf)-1,stream)) > 0 ) {
		readbuf[len] = '\0';
		strncat(allbuf, readbuf, sizeof(allbuf) - strlen(allbuf) - 1);

		if ((time(NULL) - stime) > EXEC_TIMEOUT) {
			break;
		}
	}
	pclose(stream);

	if (allbuf[0] != '\0') {
		*output << allbuf;
	}
}

void lock_cell(FCgiIO* output, const char *span, const char *arfcn, const char *timeout)
{
	if (span == NULL) {
		return;
	}

	if (timeout == NULL) {
		timeout = "10000";
	}

	char astcmd[1024];
	FILE *stream;
	char readbuf[1024+256];
	char allbuf[1024+256];
	int len;
	time_t stime;

	allbuf[0] = '\0';

	snprintf(astcmd,sizeof(astcmd),"asterisk -rx \"gsm lock cell %s \\\"%s\\\" active %s\" 2> /dev/null",span, arfcn, timeout);

	stime = time(NULL);

	if( (stream = popen(astcmd, "r")) == NULL ) {
		return;
	}

	while( (len = fread(readbuf,1,sizeof(readbuf)-1,stream)) > 0 ) {
		readbuf[len] = '\0';
		strncat(allbuf, readbuf, sizeof(allbuf) - strlen(allbuf) - 1);

		if ((time(NULL) - stime) > EXEC_TIMEOUT) {
			break;
		}
	}
	pclose(stream);

	if (allbuf[0] != '\0') {
		*output << allbuf;
	}
}

void exec_syscmd(FCgiIO* output,const char *cmd)
{
	char cmd_copy[1024];
	FILE *stream;
	char readbuf[1024+1];
	int len;
	time_t stime;

	strncpy(cmd_copy,cmd,sizeof(cmd_copy));

	stime = time(NULL);
	if( (stream = popen(cmd_copy, "r")) == NULL ) {
		return;
	}

	while( (len = fread(readbuf,1,sizeof(readbuf)-1,stream)) > 0 ) {
		readbuf[len] = '\0';
		*output << readbuf;
		output->flush();

		if ((time(NULL) - stime) > EXEC_TIMEOUT) {
			break;
		}
	}
	pclose(stream);
}

int handle_action_get_gsminfo(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	char outbuf[1024*__GSM_SUM__];
	int len;

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	outbuf[0] = 0;
	get_gsminfo(outbuf,sizeof(outbuf));
	if (outbuf[0] != 0) {
		len = strlen(outbuf);
		if (outbuf[len-1] == ',') {
			outbuf[len-1] = '\0';
		}
		(*IO) << "{";
		(*IO) << outbuf;
		(*IO) << "}";
	}

	return 0;
}
int handle_action_process_log(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *log_type = NULL, *method = NULL, *size = NULL, *port = NULL;
	const_form_iterator cgi_log_type = CGI.getElement("log_type");
	const_form_iterator cgi_method = CGI.getElement("method");
	const_form_iterator cgi_size = CGI.getElement("size");
	const_form_iterator cgi_port = CGI.getElement("port");

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_log_type != (*CGI).end()) && (!cgi_log_type->isEmpty())) {
		log_type = cgi_log_type->getValue().c_str();
	}
	if ((cgi_method != (*CGI).end()) && (!cgi_method->isEmpty())) {
		method = cgi_method->getValue().c_str();
	}
	if ((cgi_size != (*CGI).end()) && (!cgi_size->isEmpty())) {
		size = cgi_size->getValue().c_str();
	}
	if ((cgi_port != (*CGI).end()) && (!cgi_port->isEmpty())) {
		port = cgi_port->getValue().c_str();
	}

	process_log(IO, log_type, method, size, port);

	return 0;
}

int handle_action_astcmd(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *cmd = NULL;
	const_form_iterator cgi_cmd = CGI.getElement("cmd");

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_cmd != (*CGI).end()) && (!cgi_cmd->isEmpty())) {
		cmd = cgi_cmd->getValue().c_str();
		exec_astcmd(IO, cmd);
	}

	return 0;
}

int handle_action_sendat(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *cmd = NULL, *timeout = NULL, *span = NULL, *reportkeyword = NULL;
	const_form_iterator cgi_cmd = CGI.getElement("cmd");
	const_form_iterator cgi_span = CGI.getElement("span");
	const_form_iterator cgi_timeout = CGI.getElement("timeout"); //ms
	const_form_iterator cgi_reportkeyword = CGI.getElement("reportkeyword");

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_cmd != (*CGI).end()) && (!cgi_cmd->isEmpty())) {
		cmd = cgi_cmd->getValue().c_str();
	}

	if ((cgi_span != (*CGI).end()) && (!cgi_span->isEmpty())) {
		span = cgi_span->getValue().c_str();
	}

	if ((cgi_timeout != (*CGI).end()) && (!cgi_timeout->isEmpty())) {
		timeout = cgi_timeout->getValue().c_str();
	}

	if ((cgi_reportkeyword != (*CGI).end()) && (!cgi_reportkeyword->isEmpty())) {
		reportkeyword = cgi_reportkeyword->getValue().c_str();
	}

	exec_atcmd(IO, cmd, span, timeout, reportkeyword);

	return 0;
}

int handle_action_get_cfinfo(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *timeout = NULL, *span = NULL;
	const_form_iterator cgi_span = CGI.getElement("span");
	const_form_iterator cgi_timeout = CGI.getElement("timeout"); //ms

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_span != (*CGI).end()) && (!cgi_span->isEmpty())) {
		span = cgi_span->getValue().c_str();
	}

	if ((cgi_timeout != (*CGI).end()) && (!cgi_timeout->isEmpty())) {
		timeout = cgi_timeout->getValue().c_str();
	}

	get_cfinfo(IO, span, timeout);
	
	return 0;
}

int handle_action_set_cf(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *type = NULL, *timeout = NULL, *span = NULL;
	const_form_iterator cgi_span = CGI.getElement("span");
	const_form_iterator cgi_type = CGI.getElement("type");
	const_form_iterator cgi_timeout = CGI.getElement("timeout"); //ms

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_span != (*CGI).end()) && (!cgi_span->isEmpty())) {
		span = cgi_span->getValue().c_str();
	}

	if ((cgi_type != (*CGI).end()) && (!cgi_type->isEmpty())) {
		type = cgi_type->getValue().c_str();
	}

	if ((cgi_timeout != (*CGI).end()) && (!cgi_timeout->isEmpty())) {
		timeout = cgi_timeout->getValue().c_str();
	}

	if (type) {
		if(!strcmp(type,"uncond")) {
			const char * num = NULL;
			const_form_iterator cgi_num = CGI.getElement("number");

			if ((cgi_num != (*CGI).end()) && (!cgi_num->isEmpty())) {
				num = cgi_num->getValue().c_str();
			}

			if (num) {
				set_cf_uncond(IO, span, num, timeout);
			}
		} else if(!strcmp(type,"cond")) {
			const char *busy_num = NULL, *noreply_num = NULL, *notreach_num = NULL;
			const_form_iterator cgi_busy_num = CGI.getElement("busy");
			const_form_iterator cgi_noreply_num = CGI.getElement("noreply");
			const_form_iterator cgi_notreach_num = CGI.getElement("notreach");

			if ((cgi_busy_num != (*CGI).end()) && (!cgi_busy_num->isEmpty())) {
				busy_num = cgi_busy_num->getValue().c_str();
			}

			if ((cgi_noreply_num != (*CGI).end()) && (!cgi_noreply_num->isEmpty())) {
				noreply_num = cgi_noreply_num->getValue().c_str();
			}

			if ((cgi_notreach_num != (*CGI).end()) && (!cgi_notreach_num->isEmpty())) {
				notreach_num = cgi_notreach_num->getValue().c_str();
			}

			if (busy_num || noreply_num || notreach_num ) {
				set_cf_cond(IO, span, busy_num, noreply_num, notreach_num, timeout);
			}
		} else if(!strcmp(type,"disable")) {
			set_cf_disable(IO, span,timeout);
		}
	}
	
	return 0;
}

int handle_action_get_cwinfo(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *timeout = NULL, *span = NULL;
	const_form_iterator cgi_span = CGI.getElement("span");
	const_form_iterator cgi_timeout = CGI.getElement("timeout"); //ms

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_span != (*CGI).end()) && (!cgi_span->isEmpty())) {
		span = cgi_span->getValue().c_str();
	}

	if ((cgi_timeout != (*CGI).end()) && (!cgi_timeout->isEmpty())) {
		timeout = cgi_timeout->getValue().c_str();
	}

	get_cwinfo(IO, span, timeout);
	
	return 0;
}

int handle_action_set_cw(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *sw = NULL, *timeout = NULL, *span = NULL;
	const_form_iterator cgi_span = CGI.getElement("span");
	const_form_iterator cgi_sw = CGI.getElement("sw");
	const_form_iterator cgi_timeout = CGI.getElement("timeout"); //ms

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_span != (*CGI).end()) && (!cgi_span->isEmpty())) {
		span = cgi_span->getValue().c_str();
	}

	if ((cgi_sw != (*CGI).end()) && (!cgi_sw->isEmpty())) {
		sw = cgi_sw->getValue().c_str();
	}

	if ((cgi_timeout != (*CGI).end()) && (!cgi_timeout->isEmpty())) {
		timeout = cgi_timeout->getValue().c_str();
	}

	if (sw) {
		if(!strcmp(sw,"on")) {
			set_cw(IO, span, timeout, 1);
		} else if(!strcmp(sw,"off")) {
			set_cw(IO, span, timeout, 0);
		}
	}
	
	return 0;
}

int handle_action_get_cellinfo(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	char outbuf[4096*__GSM_SUM__];

	*IO << "Content-Type:text/html\n\n";
	IO->flush();
	
	memset(outbuf,0,sizeof(outbuf));
	get_cellsinfo(outbuf,sizeof(outbuf)-1);
	*IO << outbuf;
	
	return 0;
}

int handle_action_get_curcellinfo(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *timeout = NULL, *span = NULL, *reset = NULL;
	const_form_iterator cgi_span = CGI.getElement("span");
	const_form_iterator cgi_timeout = CGI.getElement("timeout"); //ms
	const_form_iterator cgi_reset = CGI.getElement("reset");

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_span != (*CGI).end()) && (!cgi_span->isEmpty())) {
		span = cgi_span->getValue().c_str();
	}

	if ((cgi_timeout != (*CGI).end()) && (!cgi_timeout->isEmpty())) {
		timeout = cgi_timeout->getValue().c_str();
	}

	if ((cgi_reset != (*CGI).end()) && (!cgi_reset->isEmpty())) {
		reset = cgi_reset->getValue().c_str();
	}

	get_curcellinfo(IO, span, timeout, reset);
	
	return 0;
}

int handle_action_lock_cell(Cgicc CGI, FCgiIO *IO, FCGX_Request *request)
{
	const char *timeout = NULL, *span = NULL, *arfcn = NULL;
	const_form_iterator cgi_span = CGI.getElement("span");
	const_form_iterator cgi_arfcn = CGI.getElement("arfcn");
	const_form_iterator cgi_timeout = CGI.getElement("timeout"); //ms

	*IO << "Content-Type:text/html\n\n";
	IO->flush();

	if ((cgi_span != (*CGI).end()) && (!cgi_span->isEmpty())) {
		span = cgi_span->getValue().c_str();
	}

	if ((cgi_arfcn != (*CGI).end()) && (!cgi_arfcn->isEmpty())) {
		arfcn = cgi_arfcn->getValue().c_str();
	}

	if ((cgi_timeout != (*CGI).end()) && (!cgi_timeout->isEmpty())) {
		timeout = cgi_timeout->getValue().c_str();
	}


	lock_cell(IO, span, arfcn, timeout);
	
	return 0;
}

static int dlog_service_config()
{
	if(debug){
		int i = 0, j = 0;
		
		/* cluster config */
		dlog(DEBUG_LEVEL1, "cluster config:<br>\n");
		dlog(DEBUG_LEVEL1, "---------[general]<br>\n");
		dlog(DEBUG_LEVEL1, "---------cluster mode = [%d]<br>\n", global_service.cluster_cfg.mode);
		dlog(DEBUG_LEVEL1, "---------[master]<br>\n");
		dlog(DEBUG_LEVEL1, "---------password = [%s]<br>\n", global_service.cluster_cfg.password);
		dlog(DEBUG_LEVEL1, "---------[slavelist]<br>\n");
		for(i = 0; i < __BRD_SUM__-1; i++){
			dlog(DEBUG_LEVEL1, "---------Board-%d_ip = [%s]<br>\n", i+2, global_service.cluster_cfg.slave_ips[i]);
		}

		/* astmanproxy config */
		dlog(DEBUG_LEVEL1, "astmanproxy config:<br>\n");
		dlog(DEBUG_LEVEL1, "---------port = %d<br>\n", global_service.ami_cfg.port);

		/* sms config */
		dlog(DEBUG_LEVEL1, "sms config:<br>\n");
		dlog(DEBUG_LEVEL1, "---------debug = %d<br>\n", global_service.sms_cfg.debug);
		dlog(DEBUG_LEVEL1, "---------timeout_total = %d s <br>\n", global_service.sms_cfg.timeout_total);
		dlog(DEBUG_LEVEL1, "---------timeout_gsm_send = %d ms <br>\n", global_service.sms_cfg.timeout_gsm_send);
		dlog(DEBUG_LEVEL1, "---------timeout_wait = %d s <br>\n", global_service.sms_cfg.timeout_wait);
		dlog(DEBUG_LEVEL1, "---------use_default_user = %d<br>\n", global_service.sms_cfg.use_default_user);
		for(i = 0; i < __BRD_SUM__; i++){
			for(j = 0; j < __GSM_SUM__; j++){
				dlog(DEBUG_LEVEL1, "---------gsm-%d.%d valid = %d <br>\n", i+1, j+1, global_service.sms_cfg.port_valid[i][j]);
			}
		}
	}

	return 0;
}

static int init_service_shm(struct service_info *service)
{
	char *file_path = "/webservice/service";
	struct service_shm *shmp = NULL; 
	int sem_id = -1, shm_id = -1, i = 0;
	key_t key;  

	/* init sem */
	if ((key = ftok(file_path, 'a')) < 0){  
		dlog(DEBUG_LEVEL1, "ftok error\n");  
		return -1;  
	} 

	if((sem_id = sem_init(key)) < 0){
		dlog(DEBUG_LEVEL1, "sem_init error\n");
		return -1;
	}

	if(sem_setval(sem_id, 1) < 0){
		dlog(DEBUG_LEVEL1, "sem_setval error\n");
		return -1;
	}

	/* init shm */
	if ((key = ftok(file_path, 'b')) < 0){  
		dlog(DEBUG_LEVEL1, "ftok error\n");  
		return -1;  
	} 
	
	if((shmp = (struct service_shm*)shm_init(key, &shm_id, sizeof(struct service_shm))) == NULL){
		dlog(DEBUG_LEVEL1, "shm_init error\n");
		return -1;
	}

	memset(shmp, 0, sizeof(*shmp));
	shmp->cur_sms_port = 0;
	for(i = 0; i < service->port_sum; i++)
		shmp->sms_port_state[i] = PORT_STATE_NOTREADY;
	service->semid = sem_id;
	service->shmid = shm_id;
	service->shm = shmp;

	dlog(DEBUG_LEVEL1, "SHM: semid[%d] shmid[%d] shmp[%p]\n", service->semid, service->shmid, service->shm);

	return 0;
}

static int init_service_config()
{
	int ret = 0;
	
	memset(&global_service, 0, sizeof(global_service));
	global_service.port_sum = __PORT_SUM__;
	ret = init_clt_config(&global_service.cluster_cfg);
	ret = init_ami_config(&global_service.ami_cfg);
	ret = init_sms_config(&global_service.sms_cfg);
	ret = init_ussd_config(&global_service.ussd_cfg);
	ret = init_get_smsstatus_config(&global_service.get_smsstatus_cfg);
	ret = init_service_shm(&global_service);
	dlog(DEBUG_LEVEL1, "ret %d\n", ret);
	get_usage_info(global_service.sms_cfg.usage_file);
	get_ussd_usage_info(global_service.ussd_cfg.usage_file);
	get_chanstate_usage_info(NULL);
	get_simstatus_usage_info(NULL);
	if(global_service.sms_cfg.debug > 0){
		debug = global_service.sms_cfg.debug;
	}
	
	/* check astmanproxy connection state or login */
	global_service.amp_timeout = 1;
	if(amp_login(&global_service) != 0){
		amp_logoff(&global_service);
		dlog(DEBUG_LEVEL0, "Astmanproxy connect fail\n");
	}

	dlog_service_config();

	return 0;
}

int main(int argc, const char **argv, char **envp)
{
	/*signal(SIGTERM, sigroutine);
	signal(SIGKILL, sigroutine);*/
	char *redis_ip = "127.0.0.1" ;
	FCGX_Request request;
	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);

	init_service_config();
	
	redisContext *c = NULL;
	c = redisConnect(redis_ip,6379);
	if (c->err) 
	{  
		printf("cerror\n");
		redisFree(c);  
		dlog(DEBUG_LEVEL1, "Connect to redis failed\n");
	}  
	
	while(FCGX_Accept_r(&request) == 0) {
		FCgiIO IO(request);
		Cgicc CGI(&IO);
		const_form_iterator action = CGI.getElement("action");

		if(debug){
			global_debug_io = &IO;
		}else{
			global_debug_io = NULL;
		}

		if( (action != (*CGI).end()) && (!action->isEmpty()) ) {
			if (action->getValue() == "get_gsminfo") {
				handle_action_get_gsminfo(CGI, &IO, &request);
			} else if(action->getValue() == "process_log") {
				handle_action_process_log(CGI, &IO, &request);
			} else if(action->getValue() == "astcmd") {
				handle_action_astcmd(CGI, &IO, &request);
			} else if(action->getValue() == "sendat") {
				handle_action_sendat(CGI, &IO, &request);
			} else if(action->getValue() == "get_cfinfo") {
				handle_action_get_cfinfo(CGI, &IO, &request);
			} else if(action->getValue() == "set_cf") {
				handle_action_set_cf(CGI, &IO, &request);
			} else if(action->getValue() == "get_cwinfo") {
				handle_action_get_cwinfo(CGI, &IO, &request);
			} else if(action->getValue() == "set_cw") {
				handle_action_set_cw(CGI, &IO, &request);
			} else if (action->getValue() == "get_cellsinfo") {
				handle_action_get_cellinfo(CGI, &IO, &request);
			} else if (action->getValue() == "get_curcellsinfo") {
				handle_action_get_curcellinfo(CGI, &IO, &request);
			} else if (action->getValue() == "lock_cell") {
				handle_action_lock_cell(CGI, &IO, &request);
			} else if (action->getValue() == "sendsms") {
				handle_action_sendsms(CGI, &IO, &request,c);
			} else if (action->getValue() == "chan_state" ) {
				handle_action_chan_state(CGI, &IO, &request, c);
			} else if (action->getValue() == "sendussd") {
				handle_action_send_ussd(CGI, &IO, &request, c);
			} else if (action->getValue() == "smsstatus"){
				handle_action_get_smsreports(CGI, &IO, &request);
			} else if(action->getValue() == "simstatus"){
				handle_action_get_simstatus(CGI, &IO, &request);
			} else if (action->getValue() == "smsremain"){
				handle_action_get_smsremain(CGI, &IO, &request, c);
			}
		}

		FCGX_Finish_r(&request);
	}

	return 0;
}
