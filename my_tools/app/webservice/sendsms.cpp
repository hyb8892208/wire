/*
  *  Filename: sendsms.cpp  Version: 0.1 
  *  
  *  Copyright (C) OPVX QMD
  *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
  *  Reference Stephen F. Booth
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
  */


#include "common.h"
#include "hiredis.h"
#include <unistd.h>


#define	USAGE_FILE_NAME	"/etc/asterisk/gw/usage_sendsms.info"
static char *usage_sendsms = NULL;

extern struct service_info global_service;

// 检测端口是否能够发送短信
static int check_httpsms_port(int port,redisContext *c)
{
	redisReply *reply = NULL;
	int ret = STATUS_SIM_UNWORK;
	reply = (redisReply *)redisCommand(c,"hget app.asterisk.simstatus.channel %d",port);
	
	if(reply->type == REDIS_REPLY_STRING && reply->str != NULL){
		ret =  atoi(reply->str);
	}
	freeReplyObject(reply);
	return ret;
}


// 获取可以发送短信的端口
static int get_min_httpsms_port(const char *type,char *from,redisContext *c)
{
	int port = 0;
	int i = 0,j = 0,min_list = 0xFF,max_idx = 0;
	int chan[__GSM_SUM__] = {0};
	int index = 0,tmp = 0;
	int online = 0;
	redisReply *reply = NULL;
	struct timeval tv;
	int ret = 1;
		
	for(i = 0; i < __GSM_SUM__;i++){
		if(1 == global_service.sms_cfg.port_valid[0][i]){
			chan[j++] = i + 1;
		}
	}
	if(j == 0){
		return STATUS_SIM_UNSELECTED;
	}

	gettimeofday(&tv,NULL);
	srand((unsigned int)tv.tv_usec);
	for(i = 0;i < j/2;i++)
	{			
		index=rand()%(j-i)+i;
		tmp=chan[i];
		chan[i]=chan[index];
		chan[index]=tmp;		
	}

	for(i = 0;i < j;i++)
	{
		ret = check_httpsms_port(chan[i],c);
		
		if(STATUS_SIM_OK != ret){
			continue;
		}
		
		if(NULL == type){
			reply = (redisReply *)redisCommand(c,"llen app.asterisk.async.smslist.%d",chan[i]);
		}else{
			reply = (redisReply *)redisCommand(c,"llen app.asterisk.php.gsm1.%d",chan[i]);
		}
		if(reply->type != REDIS_REPLY_INTEGER ){
			freeReplyObject(reply);
			continue;
		}
		online++;
		
		if(reply->integer < min_list){
			min_list = reply->integer;
			max_idx = i;
		}
		freeReplyObject(reply);
	}
	if(online > 0)
	{
		sprintf(from,"%d",chan[max_idx]);
		return STATUS_SIM_OK;
	}else{
		sprintf(from,"nil");
	}
	
	return STATUS_SIM_UNWORK;
}

//Return token's right string,copy token's left string to param 'left'.
static char* get_next_token(char *start, int token, char *left)
{
    char * find;
    find = strchr(start, token);
    if(find) {
        strncpy(left,start,find - start);
		left[find - start] = '\0';
        if(*(find+1) != '\0') {
            return find + 1;
        }
    }
    else{
		//This is last at command
        strcpy(left,start);
        return NULL;
    }
	return NULL;
}

/*
static char *usage_sendsms = "\n-------------------------\n\
Usage: http://ip:port/sendsms?username=xxx&password=xxx&phonenumber=xxx&message=xxx&port=xxx&&report=xxx&timeout=xxx\n\
	\n\
1. Parameter Description\n\
	\n\
1.1 username\n\
	Description:	The login username. Set in SMS settings page\n\
	Default Value:	None\n\
	Necessity:	Required\n\
	\n\
1.2 password\n\
	Description:	The login password. Set in SMS settings page\n\
	Default Value:	None\n\
	Necessity:	Required\n\
	\n\
1.3 phonenumber\n\
	Description:	Destination phonenumber to which the message is to be sent.\n\
	Default Value:	None\n\
	Necessity:	Required\n\
	Example:	10086,10087,10088\n\
	\n\
1.4 message\n\
	Description:	Message to be sent.\n\
	Default Value:	None\n\
	Necessity:	Required\n\
	\n\
1.5 port\n\
	Description:	Gsm port from which the message will be sent. \n\
	Default Value:	By default, the message will be send from any ready gsm port of the gateway.\n\
	Necessity:	Optional\n\
	Example:	gsm-x.x. eg: gsm-1.1 or gsm-1.1,gsm-2.2\n\
	\n\
1.6 report\n\
	Description:	Result of sending. You can choose JSON or String or NO. It can be Set in SMS settings page\n\
	Default Value:	JSON\n\
	Necessity:	Optional\n\
	\n\
1.7 timeout\n\
	Description:	The report return timeout. Unit: second.\n\
	Default Value:	0\n\
	Necessity:	Optional\n\
	\n\
2. Report Format (charset: UTF-8)\n\
	\n\
2.1 JSON\n\
	{\n\
		\"message\":\"xxx\",\n\
		\"report\":[{\n\
			\"0\":[{\n\
				\"port\":\"gsm-1.1\",\n\
				\"phonenumber\":\"10086\",\n\
				\"time\":\"2014-04-29 11:11:11\",\n\
				\"result\":\"success\"\n\
			}],\n\
			\"1\":[{\n\
				\"port\":\"gsm-1.2\",\n\
				\"phonenumber\":\"10087\",\n\
				\"time\":\"2014-04-29 11:11:12\",\n\
				\"result\":\"fail\"\n\
			}],\n\
			\"2\":[{\n\
				\"port\":\"gsm-2.2\",\n\
				\"phonenumber\":\"10088\",\n\
				\"time\":\"2014-04-29 11:11:13\",\n\
				\"result\":\"sending\"\n\
			}]\n\
		}]\n\
	}\n\
	\n\
2.2 STRING\n\
	\n\
	message:xxx\n\
	\n\
	--record 1 start--\n\
	port: gsm-1.1\n\
	phonenumber: 10086\n\
	time: 2014-04-29 11:11:11\n\
	result: success\n\
	--record 1 end--\n\
	\n\
	--record 2 start--\n\
	port: gsm-1.2\n\
	phonenumber: 10087\n\
	time: 2014-04-29 11:11:12\n\
	result: fail\n\
	--record 2 end--\n\
	\n\
	--record 3 start--\n\
	port: gsm-2.2\n\
	phonenumber: 10088\n\
	time: 2014-04-29 11:11:13\n\
	result: sending\n\
	--record 3 end--\n\
2.3 No\n\
	No report. \n\
	Request return immediately and task handled in the background.\n\
	\n-------------------------\n";
*/

static int send_sms_var_redis(struct sms_args *args,char *outbuf,int outlen,redisContext *c)
{
	char result[32] = {0};
	char from_str[4] = {0};
	char to_str[24] = {0};
	char *tmp_from = (char *)args->from;
	char *tmp_to = (char *)args->to;
	char buf[1280] = {0};
	int ret = 0,send_num = 0;
	FILE *fp = NULL;
	
	struct timeval tv;
	enum sms_report_format report_fmt = global_service.sms_cfg.report_format;
	redisReply * reply = NULL;

	gettimeofday(&tv, NULL);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
	strcpy(result,SMS_REPORT_FAIL);

	if(NULL != args->report){
		if(0 == strcasecmp(args->report,"JSON")){
			report_fmt = SMS_REPORT_JSON;
		}else if(0 == strcasecmp(args->report, "STRING")){
			report_fmt = SMS_REPORT_STRING;
		}
	}
	
	outbuf[0] = '\0';
	if(strlen(args->message) > 1000){
		strcpy(result,"message too long");
		switch(report_fmt)
		{
			case SMS_REPORT_JSON:
				sprintf(outbuf,"\t{\n\t\t\"message\":\"%s\","\
					"\n\t\t\"report\":[{\n\t\t\t\"1\":"\
					"[{\n\t\t\t\t\"port\":\"%s\","\
					"\n\t\t\t\t\"phonenumber\":\"%s\","\
					"\n\t\t\t\t\"time\":\"%s\","\
					"\n\t\t\t\t\"id\":\"%s\","
					"\n\t\t\t\t\"result\":"\
					"\"%s\"\n\t\t\t}]\n\t\t}]\n\t}",\
					args->message,args->from,args->to,buf,args->id,result);
				break;
			case SMS_REPORT_STRING:
				sprintf(outbuf,"--record 1 start--\n"\
					"port: %s\n"\
					"phonenumber: %s\n"\
					"time: %s\n"\
					"id: %s\n"\
					"result: %s\n"\
					"--record 1 end--\n"\
					"\n",\
					args->from,args->to,buf,args->id,result);
				break;
			default:
				break;
		}
		return 0;
	}

	switch(report_fmt)
	{
		case SMS_REPORT_JSON:
			sprintf(outbuf,"\t{\n\t\t\"message\":\"%s\","\
				"\n\t\t\"report\":[{",args->message);
			break;
		case SMS_REPORT_STRING:
			sprintf(outbuf,"message:%s\n\n",args->message);
			break;
		default:
			break;
	}
	
	do
	{
		tmp_to = get_next_token(tmp_to ,',',to_str);
		
		if(NULL == args->from){
			ret = get_min_httpsms_port(args->type,from_str,c);
		}else{
			tmp_from = get_next_token(tmp_from,',',from_str);
			if(NULL == tmp_from){
				tmp_from = (char *)args->from;
			}
			ret = check_httpsms_port(atoi(from_str),c);
		}
		switch(ret)
		{
			case STATUS_SIM_OK:
				if(NULL == args->type)
				{
					snprintf(buf, 1280, "{\"type\":\"asyncsms\",\"num\":\"%s\","\
						"\"msg\":\"%s\","\
						"\"flash\":\"0\",\"id\":\"%s\"}", \
						to_str,args->message, args->id);
					reply =(redisReply *)redisCommand(c, "rpush app.asterisk.async.smslist.%s %s",from_str,buf);
				}else{
					snprintf(buf,1280,"{\"from\":\"%s\","\
							"\"to\":\"%s\","\																																				 
							"\"message\":\"%s\","\
							"\"order\":\"%s\","\
							"\"report\":\"%s\","\
							"\"timeout\":\"%s\","\
							"\"pidnum\":\"%d\","\
							"\"id\":\"%s\","\
							"\"debug\":\"%s\"}",
							from_str,
							to_str,
							args->message,
							args->order,
							args->report,
							args->timeout,
							getpid(),
							args->id,
							args->debug);
					reply =(redisReply *)redisCommand(c, "rpush app.asterisk.httpsms.list %s",buf);
				}
				dlog(DEBUG_LEVEL1,"sms buf is :  %s\n",buf);
				if (NULL == reply)
				{
					dlog(DEBUG_LEVEL1,"Fail to execute command rpush ");
					strcpy(result,SMS_REPORT_FAIL);
				}else{
					strcpy(result,SMS_REPORT_SENDING);
					freeReplyObject(reply);
				}
				break;
			case STATUS_SIM_UNWORK:
				strcpy(result,SIM_STATUS_NOT_READY);
				break;
			case STATUS_SIM_UNSELECTED:
				strcpy(result,SIM_STATUS_NOT_SELECT);
				break;
			case STATUS_OTHER:
			default:
				break;
		}
		send_num++;
		
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
		switch(report_fmt)
		{
			case SMS_REPORT_JSON:
				sprintf(outbuf+strlen(outbuf),"\n\t\t\t\"%d\":"\
		            "[{\n\t\t\t\t\"port\":\"%s\","\
		            "\n\t\t\t\t\"phonenumber\":\"%s\","\
		            "\n\t\t\t\t\"time\":\"%s\","\
		            "\n\t\t\t\t\"id\":\"%s\","
		            "\n\t\t\t\t\"result\":"\
		            "\"%s\"\n\t\t\t}],",send_num,from_str,to_str,buf,args->id,result);
				break;
			case SMS_REPORT_STRING:
				sprintf(outbuf+strlen(outbuf),"--record %d start--\n"\
					"port: %s\n"\
					"phonenumber: %s\n"\
					"time: %s\n"\
					"id: %s\n"\
					"result: %s\n"\
					"--record %d end--\n"\
					"\n",\
					send_num,from_str,to_str,buf,args->id,result,send_num);
				break;
			default:
				strcat(outbuf,result);
				break;
		}
	}while(NULL != tmp_to);
	
	switch(report_fmt)
	{
		case SMS_REPORT_JSON:
			sprintf(outbuf+strlen(outbuf)-1,"\n\t\t}]\n\t}");
			break;
		case SMS_REPORT_STRING:
		default:
			break;
	}
	return 0;

}

static struct http2sms_task global_sms_task;
/***************************************
  *
  *          functions
  *
  *
  **************************************/

static void filter_sms(char *msg)
{
	int pos = 0;
	char * p;
	char cpy_msg[2048] = {0}; // 拷贝msg，作为源字符串
	strncpy(cpy_msg,msg,sizeof(cpy_msg));
	for (p = cpy_msg,pos = 0; *p != '\0'; p++, pos++) {
		*(msg+pos) = *p;
		switch (*p) {
		case '\r':
			*p = 17;
			break;
		case '\n':
			*p = 18;
			break;
		case '\\': //检测到'\'字符时，添加转义字符
			pos++;
			*(msg+pos) = '\\';
			break;
		case '\"': //检测到'"'字符时，添加转义字符
			*(msg+pos) = '\\';
			*(msg+pos+1) = '\"';
			pos++;
			break;
		default:
			break;
		}
	}
	*(msg+pos) = '\0';
}

static void filter_to(char *to)
{
	char * p;
	for (p = to; *p != '\0'; p++) {
		switch (*p) {
		case ' ':
			*p = '+';
			break;
		default:
			break;
		}
	}
}

static void reverse_sms(char *msg)
{
	char * p;
	for (p = msg; *p != '\0'; p++) {
		switch (*p) {
		case 17:
			*p = '\r';
			break;
		case 18:
			*p = '\n';
			break;
		default:
			break;
		}
	}
}

static bool send_over(void)
{
	struct sms_port_node *port = NULL;
	int i = 0, max = global_service.port_sum;
	
	if(global_sms_task.sms_to_cur != NULL)
		return false;

	if(global_sms_task.sms_msg.count > 1){
		for(i = 0; i < max; i++){
			port = &global_sms_task.sms_port[i];
			if(port->is_valid && port->is_selected){
				if(port->sms_to_cur && port->longsms_count < global_sms_task.sms_msg.count)
					return false;
			}
		}
	}

	return true;
}

static int dlog_port_info(int all)
{
	int i = 0;
	int max = global_service.port_sum;
	
	if(debug){
		struct sms_port_node *port = NULL;
		dlog(DEBUG_LEVEL1, "---------------\n");
		for(i = 0; i < max; i++){
			port = &global_sms_task.sms_port[i];
			if(!all){
				if(!port->is_valid || !port->is_selected){
					continue;
				}
			}
			dlog(DEBUG_LEVEL1, "gsm-%d.%d valid[%d] selected[%d] state[%d] longsms[%d/%d][%d][flag%d] timestamp[%ld.%6ld/%ld.%6ld] server[%s]\n", 
				port->board, 
				port->span,
				port->is_valid,
				port->is_selected,
				global_service.shm->sms_port_state[i],
				port->longsms_count,
				global_sms_task.sms_msg.count,
				port->longsms_succ,
				port->longsms_flag,
				port->timestamp.tv_sec, 
				port->timestamp.tv_usec,
				global_service.shm->sms_port_state_timestamp[i].tv_sec,
				global_service.shm->sms_port_state_timestamp[i].tv_usec,
				port->server);
		}
		dlog(DEBUG_LEVEL1, "---------------\n");
	}
	
	return 0;
}

static int report_string(char *outbuf, int outlen)
{
/*
Format:
--------------
message:xxx

--record 1 start--
from: gsm-1.1
to: 10086
time: 2014-04-29 11:11:11
result: success
--record 1 end--

--record 2 start--
from: gsm-1.2
to: 10087
time: 2014-04-29 11:11:12
result: fail
--record 2 end--

--record 3 start--
from: gsm-2.2
to: 10088
time: 2014-04-29 11:11:13
result: sending
--record 3 end--



*/
	struct sms_to_node *tmp = NULL;
	char buffer[MAX_LEN_BUFFER];
	char ori[MAX_LEN_BUFFER];
	int i = 0;

	strncpy(ori,global_sms_task.sms_msg.original,sizeof(ori));
	reverse_sms(ori);
	
//	snprintf(outbuf, outlen, "message: %s\n\n", global_sms_task.sms_msg.original);
	snprintf(outbuf, outlen, "message: %s\n\n", ori);
	strncat(outbuf, "record start\n\n", outlen-strlen(outbuf)-1);
	for(i = 1, tmp = global_sms_task.sms_to_head; tmp != NULL; tmp = tmp->next, i++){
		memset(buffer, 0, sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "--record %d start--\n"\
			"port: %s\n"\
			"phonenumber: %s\n"\
			"time: %s\n"\
			"result: %s\n"\
			"--record %d end--\n"\
			"\n",
			i,
			tmp->port,
			tmp->phonenumber,
			tmp->time,
			tmp->port[0]=='\0'?"":(tmp->result[0]=='\0'?SMS_REPORT_SENDING:tmp->result),
			i);
		strncat(outbuf, buffer, outlen-strlen(outbuf)-1);
	}
	strncat(outbuf, "record end\n", outlen-strlen(outbuf)-1);
	
	return 0;
}

static int report_json(char *outbuf, int outlen)
{
/*
Format:
{
	"message":"xxx",
	"report":[{
		"0":[{
			"from":"gsm-1.1",
			"to":"10086",
			"time":"2014-04-29 11:11:11",
			"result":"success"
		}],
		"1":[{
			"from":"gsm-1.2",
			"to":"10087",
			"time":"2014-04-29 11:11:12",
			"result":"fail"
		}],
		"2":[{
			"from":"gsm-2.2",
			"to":"10088",
			"time":"2014-04-29 11:11:13",
			"result":"sending"
		}]
	}]
}
*/
	struct sms_to_node *tmp = NULL;
	char buffer[MAX_LEN_BUFFER];
	char ori[MAX_LEN_BUFFER];
	int i = 0;

	strncpy(ori,global_sms_task.sms_msg.original,sizeof(ori));
	reverse_sms(ori);
		
//	snprintf(outbuf, outlen, "{\"message\":\"%s\",\"report\":[{", global_sms_task.sms_msg.original);
	snprintf(outbuf, outlen, "{\"message\":\"%s\",\"report\":[{", ori);

	for(i = 0, tmp = global_sms_task.sms_to_head; tmp != NULL; tmp = tmp->next, i++){
		memset(buffer, 0, sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "\"%d\":[{"\
			"\"port\":\"%s\","\
			"\"phonenumber\":\"%s\","\
			"\"time\":\"%s\","\
			"\"result\":\"%s\"}]",
			i,
			tmp->port,
			tmp->phonenumber,
			tmp->time,
			tmp->port[0]=='\0'?"":(tmp->result[0]=='\0'?SMS_REPORT_SENDING:tmp->result));
		strncat(outbuf, buffer, outlen-strlen(outbuf)-1);
		if(tmp->next != NULL){
			/* This is not the last one , so add "," */
			strncat(outbuf, ",", outlen-strlen(outbuf)-1);
		}
	}
	strncat(outbuf, "}]}", outlen-strlen(outbuf)-1);
	
	return 0;
}

static int destroy_sms_to(struct http2sms_task *task)
{
	struct sms_to_node *tmp = task->sms_to_head;

	while(task->sms_to_head != NULL){
		tmp = task->sms_to_head;
		task->sms_to_head = task->sms_to_head->next;
		free(tmp);
	}
	task->sms_to_cur = NULL;
	task->sms_to_rest = 0;
	
	return 0;
}

static int set_sms_cfg(struct sms_args *args, struct http2sms_task *task)
{
	int val = 0;
	
	dlog(DEBUG_LEVEL1, "\n");
	
	if(args->order != NULL){
		if(!strcasecmp(args->order, "round")){
			task->sms_cfg.port_order = PORT_ORDER_ROUND;
		}else if(!strcasecmp(args->order, "random")){
			task->sms_cfg.port_order = PORT_ORDER_RANDOM;
		}else if(!strcasecmp(args->order, "ascend")){
			task->sms_cfg.port_order = PORT_ORDER_ASCEND;
		}else{
			task->sms_cfg.port_order = PORT_ORDER_ROUND;
		}
	} else {
		task->sms_cfg.port_order = PORT_ORDER_ROUND;
	}	
	if(args->report != NULL){
		if(!strcasecmp(args->report, "JSON")){
			task->sms_cfg.report_format = SMS_REPORT_JSON;
		}else if(!strcasecmp(args->report, "STRING")){
			task->sms_cfg.report_format = SMS_REPORT_STRING;
		}else if(!strcasecmp(args->report, "NO")){
			task->sms_cfg.report_format = SMS_REPORT_NO;
		}else{
			task->sms_cfg.report_format = SMS_REPORT_JSON;
		}
	}

	if(args->timeout != NULL){
		if((val = atoi(args->timeout))> 0){
			task->sms_cfg.timeout_total = val;
		}else{
			task->sms_cfg.timeout_total = 0;
		}
	}

	if(args->debug != NULL){
		if((val = atoi(args->debug))> 0){
			task->sms_cfg.debug = val;
		}else{
			task->sms_cfg.debug = 0;
		}		
	}
	
	return 0;
}

static int set_sms_to(const char *sms_to_str, struct http2sms_task *task)
{
	struct sms_to_node *new_sms_to = NULL, *tail = NULL;
	char *sms_to = NULL, *start = NULL, *end = NULL;
	int len = strlen(sms_to_str), ret = -1, i = 0;
	
	dlog(DEBUG_LEVEL1, "\n");

	dlog(DEBUG_LEVEL1, "sms_to_str:%s\n",sms_to_str);
	if(!sms_to_str){
		return -1;
	}

	/* parse sms_to_str */
	task->sms_to_rest = 0;
	start = (char*)sms_to_str;
	while((*start) != '\0'){
		end = strchr(start, SMS_TO_DELIM);
		if(end != NULL && (end - start) == 0){
			start = end + 1;
			continue;
		}
		
		new_sms_to = (struct sms_to_node*)malloc(sizeof(struct sms_to_node));
		if(!new_sms_to){
			destroy_sms_to(task);
			free(sms_to);
			return -1;
		}
		memset(new_sms_to, 0, sizeof(*new_sms_to));
		new_sms_to->next = NULL;
		
		/* insert to sms_to  list */
		if(tail == NULL){
			task->sms_to_head = new_sms_to;
			tail = task->sms_to_head;
		}else{
			tail->next = new_sms_to;
			tail = new_sms_to;
		}
		task->sms_to_rest++;
		
		if(end == NULL){
			strncpy(new_sms_to->phonenumber, start, sizeof(new_sms_to->phonenumber));
			break;
		}else{
			strncpy(new_sms_to->phonenumber, start, end - start);
			new_sms_to->phonenumber[end - start] = '\0';
			start = end + 1;
		}
	}

	task->sms_to_cur = task->sms_to_head;

	if(debug){
		struct sms_to_node *tmp = NULL;
		for(tmp = global_sms_task.sms_to_head; tmp != NULL; tmp = tmp->next){
			dlog(DEBUG_LEVEL1, "get phonenumber: [%s]\n", tmp->phonenumber);
		}
	}

	return 0;
}

static int set_sms_msg(const char *sms_message_str, struct http2sms_task *task)
{
	int i = 0, len = 0, size = 0, src_pos = 0, des_pos = 0, utf8_pos = 0, max = 160 - 8, count = 0;
	int count_max = sizeof(task->sms_msg.contents) / sizeof(task->sms_msg.contents[0]);
	char c = 0;
	
	dlog(DEBUG_LEVEL1, "\n");

	if(!sms_message_str){
		return -1;
	}

	len = strlen(sms_message_str);
	
	dlog(DEBUG_LEVEL1, "Length of message [%d]\n", len);
	
	//modified by Ronggang, reset the max-length for a single sms;
	if (len<=160) {
		max=160;
	}
	
	for(i = 0; i < len; i++){
		if(*(sms_message_str+i) > 127)
		{
			max = 70 - 3; // utf-8 non-ascii
			dlog(DEBUG_LEVEL1, "Message type: utf-8\n");
		}
	}

	memset(&task->sms_msg, 0, sizeof(task->sms_msg));
	strncpy(task->sms_msg.original, sms_message_str, sizeof(task->sms_msg.original));

	count =  des_pos = src_pos = 0;
	while(*(task->sms_msg.original + src_pos) != '\0'){
		if((*(task->sms_msg.original + src_pos) == '"' || *(task->sms_msg.original + src_pos) == '\\' )
			&& des_pos < sizeof(task->sms_msg.contents[count])
		){
			*(task->sms_msg.contents[count] + des_pos) = '\\';
			des_pos++;
			utf8_pos++;
			if(utf8_pos >= max){
				des_pos = 0;
				utf8_pos = 0;
				count++;
				if(count >= count_max)
					break;
			}
		}

		c = *(task->sms_msg.original + src_pos);
		if((c & 0xFC) == 0xFC)
			size = 6;
		else if((c & 0xF8) == 0xF8)
			size = 5;
		else if((c & 0xF0) == 0xF0)
			size = 4;
		else if((c & 0xE0) == 0xE0)
			size = 3;	
		else if((c & 0xC0) == 0xC0)
			size = 2;
		else
			size = 1;
		
		dlog(DEBUG_LEVEL1, "[%02X]size[%d]\n", c, size);
		
		for(i = 0; i < size && des_pos + i < sizeof(task->sms_msg.contents[count]) && src_pos + i < sizeof(task->sms_msg.original); i++)
			*(task->sms_msg.contents[count] + des_pos + i) = *(task->sms_msg.original + src_pos + i);
		src_pos += size;
		des_pos += size;
		utf8_pos++;
		if(utf8_pos >= max){
			des_pos = 0;
			utf8_pos = 0;
			count++;
			if(count >= count_max)
				break;
		}
	}

	task->sms_msg.count = count + 1;
	
	if (len<=160) {
		task->sms_msg.count = 1;
	}
	

	dlog(DEBUG_LEVEL1, "message count %d [%s] utf8 len [%d]\n", task->sms_msg.count, task->sms_msg.contents[0], utf8_strlen(task->sms_msg.original));
	
	return 0;
}

static int set_sms_port(const char *sms_port_str, struct http2sms_task *task)
{
	int board = 0, span = 0, i = 0;
	char *start = NULL, *end = NULL;
	int port_sum = global_service.port_sum;
	
	dlog(DEBUG_LEVEL1, "\n");
	/* set valid port */
	for(i = 0; i < port_sum; i++){
		board = (i/__GSM_SUM__)+1;
		span = (i%__GSM_SUM__)+1;
		if(board == 1){
			strncpy(task->sms_port[i].server, "127.0.0.1", sizeof(task->sms_port[i].server));
			if(!sms_port_str)
				/* user don't select , up to config file */
				task->sms_port[i].is_valid = task->sms_cfg.port_valid[board-1][span-1];
			else
				/* user selected, up to user select */
				task->sms_port[i].is_valid = 1;
			task->sms_port[i].index = i;
			task->sms_port[i].board = board;
			task->sms_port[i].span = span;
			task->sms_port[i].sms_to_cur = NULL;
		}else if(board > 1){
			if(global_service.cluster_cfg.mode == CLUSTER_MASTER){
				if(global_service.cluster_cfg.slave_ips[board-2][0] != '\0'){
					strncpy(task->sms_port[i].server, global_service.cluster_cfg.slave_ips[board-2], sizeof(task->sms_port[i].server));
					if(!sms_port_str)
						/* user don't select , up to config file */
						task->sms_port[i].is_valid = task->sms_cfg.port_valid[board-1][span-1];
					else
						/* user selected, up to user select */
						task->sms_port[i].is_valid = 1;
					task->sms_port[i].index = i;
					task->sms_port[i].board = board;
					task->sms_port[i].span = span;
					task->sms_port[i].sms_to_cur = NULL;
				}
			}
		}
	}
	
	dlog(DEBUG_LEVEL1, "\n");

	/* set user selected port */
	if(sms_port_str){
		for(i = 0; i < port_sum; i++){
			task->sms_port[i].is_selected = 0;
		}
		/* parse sms_port_str */
		dlog(DEBUG_LEVEL1, "sms_port_str [%s]\n", sms_port_str);
		start = (char *)sms_port_str;
		while((*start) != '\0'){
			if(sscanf(start, __GSM_HEAD__"%d.%d", &board, &span) == 2 
				&& board > 0 && board <= __BRD_SUM__
				&& span > 0 && span <= __GSM_SUM__
			){
				i = (board-1)*__GSM_SUM__+span-1;
				task->sms_port[i].is_selected = 1;
			}
			
			end = strchr(start, SMS_TO_DELIM);
			if(end == NULL){
				break;
			}else{
				start = end + 1;
			}
		}
	}else{
		for(i = 0; i < port_sum; i++){
			task->sms_port[i].is_selected = 1;
		}
	}

	if(service_shm_lock(1) < 0){
		dlog(DEBUG_LEVEL1, "service_shm_lock\n");
		return -1;
	}
	/* check if shm port state out of date */
	if(global_service.shm){
		struct timeval tv;
		gettimeofday(&tv, NULL);
		for(i = 0; i < global_service.port_sum; i++){
			if(global_service.shm->sms_port_state[i] == PORT_STATE_SENDING 
				&& tv.tv_sec - global_service.shm->sms_port_state_timestamp[i].tv_sec > PORT_STATE_SENDING_TIMEOUT
			){
				global_service.shm->sms_port_state[i] = PORT_STATE_NOTREADY; /* state out of date */
			}else if(tv.tv_sec - global_service.shm->sms_port_state_timestamp[i].tv_sec > PORT_STATE_CHECKING_TIMEOUT){
				global_service.shm->sms_port_state[i] = PORT_STATE_NOTREADY; /* state out of date */
			}
		}
	}
	
	if(service_shm_unlock(1) < 0){
		dlog(DEBUG_LEVEL1, "service_shm_lock\n");
		return -1;
	}
	dlog_port_info(1);

	return 0;
}

static int set_port_state(struct sms_port_node *port, enum sms_port_state state)
{
	struct timeval tv;	
	gettimeofday(&tv, NULL);

	if(service_shm_lock(1) < 0) // nowait
		return -1;

	if(global_service.shm){
		global_service.shm->sms_port_state_timestamp[port->index] = tv;
		global_service.shm->sms_port_state[port->index] = state;
	}

	if(service_shm_unlock(1) < 0) //nowait
		return -1;
	
	return 0;
}

static int get_send_command(struct sms_port_node *port, char *outbuf, int outlen)
{
	struct sms_message *message = NULL;
	struct timeval tv;
	char value[MAX_LEN_NAME];
	int timeout = 0;
	
	if(send_over()){
		return -1;
	}

	message = &global_sms_task.sms_msg;
	timeout = global_sms_task.sms_cfg.timeout_gsm_send;

	if(message->count == 1){
		/* normal sms */
		if(port->sms_to_cur){
			return -1; // sending, wait for result
		}

		if(global_sms_task.sms_to_cur){
			port->sms_to_cur = global_sms_task.sms_to_cur;
			global_sms_task.sms_to_cur = global_sms_task.sms_to_cur->next;
		}else
			return -1; // send over
			
		gettimeofday(&tv, NULL);
		snprintf(port->actionid, sizeof(port->actionid), __GSM_HEAD__"%d.%d[%d][%s]%ld.%ld", 
			port->board, port->span, SMS_FLAG_SEND_SMS, port->sms_to_cur->phonenumber, tv.tv_sec, tv.tv_usec);

		//command: gsm send sync sms <span> <destination> <message> <timeout> [flash sms] [id] 
		snprintf(outbuf, outlen, "Action: Command\r\n"\
			"Command: gsm send sync sms %d %s \"%s\" %d\r\n"\
			"Server: %s\r\n"\
			"ActionID: %s\r\n"\
			"\r\n",
			port->span, 
			port->sms_to_cur->phonenumber,
			message->contents[0], 
			timeout,
			port->server, 
			port->actionid);

	}else if(message->count > 1){
		/* long sms */
		if(port->sms_to_cur){
			if(port->longsms_count >= message->count || !port->longsms_succ)
				return -1; // sending, wait for result
		}else{
			if(global_sms_task.sms_to_cur){
				port->sms_to_cur = global_sms_task.sms_to_cur;
				global_sms_task.sms_to_cur = global_sms_task.sms_to_cur->next;
			}else
				return -1; // send over
		}

		if(port->longsms_count == 0)
			port->longsms_flag =random_int(0, 255);
		
		gettimeofday(&tv, NULL);
		snprintf(port->actionid, sizeof(port->actionid), __GSM_HEAD__"%d.%d[%d][%s]%ld.%ld", 
			port->board, port->span, SMS_FLAG_SEND_SMS, port->sms_to_cur->phonenumber, tv.tv_sec, tv.tv_usec);
		
		//command: gsm send sync csms <span> <destination> <message> <flag> <smscount> <smssequence> <timeout> [flash sms] [id]
		snprintf(outbuf, outlen, "Action: Command\r\n"\
			"Command: gsm send sync csms %d %s \"%s\" %d %d %d %d\r\n"\
			"Server: %s\r\n"\
			"ActionID: %s\r\n"\
			"\r\n",
			port->span, 
			port->sms_to_cur->phonenumber,
			message->contents[port->longsms_count],
			port->longsms_flag,
			message->count,
			port->longsms_count + 1,
			timeout,
			port->server, 
			port->actionid); 
	}
	
	return 0;
}

/*
  *      port state change: 
  *                 PORT_STATE_READY      ---> PORT_STATE_SENDING         [ sms send ] 
  *                 PORT_STATE_SENDING  ---> PORT_STATE_SENDING         [ long sms send ]
  *
  */
static int send_port_sms(struct sms_port_node *port)
{
	struct timeval tv;
	char buffer[MAX_LEN_BUFFER];
	int ret = -1;
	//time_t now;

	dlog(DEBUG_LEVEL3, "gsm-%d.%d will send sms\n", port->board, port->span);

	memset(buffer, 0, sizeof(buffer));
	if(get_send_command(port, buffer, sizeof(buffer)) != 0){
		/* no sms to send */
		return -1;
	}

	//now = time(NULL);
	gettimeofday(&tv, NULL);
	strftime(port->sms_to_cur->time, sizeof(port->sms_to_cur->time), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec)); 
	snprintf(port->sms_to_cur->port, sizeof(port->sms_to_cur->port), __GSM_HEAD__"%d.%d", port->board, port->span);

	dlog(DEBUG_LEVEL1, "gsm-%d.%d: write [%s]\n",  port->board, port->span, buffer);
	ret = send(global_service.amp_fd, buffer, strlen(buffer), 0);
	if (ret <= 0) {        
		/* socket close */
		close(global_service.amp_fd);
		global_service.amp_fd = 0;
		strncpy(port->sms_to_cur->result, "fail", sizeof(port->sms_to_cur->result));
		return -1;
	}

	port->longsms_count++;
	port->longsms_succ = 0;
	port->timestamp = tv;
	if(global_service.shm){
		global_service.shm->sms_port_state[port->index] = PORT_STATE_SENDING;
		global_service.shm->sms_port_state_timestamp[port->index] = tv;
	}
	
	return 0;
}

/*
  *      port state change: 
  *                 PORT_STATE_NOTREADY ---> PORT_STATE_CHECKING              [ port ckeck ] 
  *
  */
static int send_port_check(struct sms_port_node *port)
{
	struct timeval tv;
	time_t begin, now;
	char command[MAX_LEN_COMMAND];
	char *p = NULL;
	int ret = -1, len = 0;

	dlog(DEBUG_LEVEL3, "gsm-%d.%d will be checked\n", port->board, port->span);

	gettimeofday(&tv, NULL);
	snprintf(port->actionid, sizeof(port->actionid), "gsm-%d.%d[%d]-%ld.%ld", 
		port->board, port->span, SMS_FLAG_PORT_STATE, tv.tv_sec, tv.tv_usec);
	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "Action: Command\r\n"\
		 "Command: gsm show span %d\r\n"\
		 "Server: %s\r\n"\
		 "ActionID: %s\r\n"\
		 "\r\n",
		 port->span,
		 port->server,
		 port->actionid);
	dlog(DEBUG_LEVEL3, "command: [%s]\n", command);

	ret = send(global_service.amp_fd, command, strlen(command), 0);
	if (ret <= 0) {        // client close
		close(global_service.amp_fd);
		global_service.amp_fd = 0;
		return -1;
	}

	port->timestamp = tv;
	if(global_service.shm){
		global_service.shm->sms_port_state[port->index] = PORT_STATE_CHECKING;
		global_service.shm->sms_port_state_timestamp[port->index] = tv;
	}

	return 0;
}

/*
  *      port state change: 
  *                 PORT_STATE_CHECKING ---> PORT_STATE_READY              [ port ready ] 
  *                 PORT_STATE_CHECKING ---> PORT_STATE_NOTREADY        [ port not ready ]
  *
  */
static int parse_port_check(struct sms_port_node *port, char *package)
{
	char value[MAX_LEN_NAME];

	dlog(DEBUG_LEVEL3, "\n");
	
	if(package == NULL || *package == '\0')
		return -1;

	if(global_service.shm->sms_port_state[port->index] != PORT_STATE_CHECKING)
		return -1;

	/* check power */
	memset(value, 0, sizeof(value));
	if(get_gsm_header(package, "Status", value, sizeof(value)) != 0){
		//dlog(DEBUG_LEVEL3, "gsm-%d.%d error1\n", port->board, port->span);
		goto invalid;
	}
	
	dlog(DEBUG_LEVEL3, "gsm-%d.%d: Status[%s]\n", port->board, port->span, value);
	if(strncasecmp(value, "Power on, Provisioned, Up", sizeof("Power on, Provisioned, Up")-1) != 0){
		//dlog(DEBUG_LEVEL3, "gsm-%d.%d error2\n", port->board, port->span);
		goto invalid;
	}

	/* check state */
	memset(value, 0, sizeof(value));
	if(get_gsm_header(package, "State", value, sizeof(value)) != 0){
		//dlog(DEBUG_LEVEL3, "gsm-%d.%d error3\n", port->board, port->span);
		goto invalid;
	}
	
	dlog(DEBUG_LEVEL1, "gsm-%d.%d: State[%s]\n", port->board, port->span, value);

	if(!strcasecmp(value, "SIM READY REQ")){
		goto invalid;
	}
	
	if(!strcasecmp(value, "READY")){
		return set_port_state(port, PORT_STATE_READY);
	}else{
		return set_port_state(port, PORT_STATE_NOTREADY);
	}
	
	return 0;
invalid:
	set_port_state(port, PORT_STATE_NOTREADY);
	port->is_valid = 0;
	return -1;
}


/*
  *      port state change: 
  *                 PORT_STATE_SENDING ---> PORT_STATE_NOTREADY      [ send over ] 
  *                 PORT_STATE_SENDING ---> PORT_STATE_SENDING        [ long sms sending ]
  *
  */
static int parse_port_sms(struct sms_port_node *port, char *package)
{
	dlog(DEBUG_LEVEL3, "\n");

	if(package == NULL || *package == '\0')
		return -1;

	if(global_service.shm->sms_port_state[port->index] != PORT_STATE_SENDING)
		return -1;
	
	if(strstr(package, SMS_RESULT_CHECK_NORMAL) || strstr(package, SMS_RESULT_CHECK_CONCATENATED)){
		dlog(DEBUG_LEVEL1, "gsm-%d.%d parse send result \n", port->board, port->span);
		if(strstr(package, SMS_RESULT_SUCCESS_KEYWORD)){
			dlog(DEBUG_LEVEL1, "gsm-%d.%d: send success [%d/%d]\n", port->board, port->span, port->longsms_count, global_sms_task.sms_msg.count);
			if(port->longsms_count < global_sms_task.sms_msg.count){
				port->longsms_succ = 1;
				dlog(DEBUG_LEVEL1, "gsm-%d.%d sending\n", port->board, port->span);
				return 0;
			}
			strncpy(port->sms_to_cur->result, SMS_REPORT_SUCCESS, sizeof(port->sms_to_cur->result));
		}else if(strstr(package, SMS_RESULT_FAIL_KEYWORD)){
			dlog(DEBUG_LEVEL1, "gsm-%d.%d: send fail \n", port->board, port->span);
			strncpy(port->sms_to_cur->result, SMS_REPORT_FAIL, sizeof(port->sms_to_cur->result));
		}else{
			dlog(DEBUG_LEVEL1, "gsm-%d.%d: sending \n", port->board, port->span);
			strncpy(port->sms_to_cur->result, SMS_REPORT_SENDING, sizeof(port->sms_to_cur->result));
		}
		port->longsms_count = 0;
		port->longsms_succ = 0;
		port->longsms_flag = 0;
		port->sms_to_cur = NULL; // release phonenumber
		global_sms_task.sms_to_rest--;
		return set_port_state(port, PORT_STATE_NOTREADY);
	}

	return 0;
}

static int amp_parse(char *pack)
{
	struct sms_port_node *port = NULL;
	char actionid[MAX_LEN_NAME];
	char server[MAX_LEN_NAME];
	int board = 0, span = 0, i = 0, flag;

	dlog(DEBUG_LEVEL1, "pack [%s]\n", pack);
	
	memset(actionid, 0, sizeof(actionid));
	memset(server, 0, sizeof(server));
	if(amp_get_header(pack, ASTMANPROXY_HEADER_ACTIONID, actionid, sizeof(actionid)) != 0
		|| amp_get_header(pack, ASTMANPROXY_HEADER_SERVER, server, sizeof(server)) != 0
	){
		return -1;
	}

	if(sscanf(actionid, __GSM_HEAD__"%d.%d[%d]", &board, &span, &flag) == 3){
		dlog(DEBUG_LEVEL3, "board[%d] span[%d] flag[%d]\n", board, span, flag);
		if(board > 0 && board <= __BRD_SUM__ && span > 0 && span <= __GSM_SUM__){
			i = (board - 1) * __GSM_SUM__ + span - 1;
			port = &global_sms_task.sms_port[i];
			if(!strcasecmp(port->actionid, actionid) && !strcasecmp(port->server, server)){
				/* have found the port who send this command */
				switch(flag){
				case SMS_FLAG_SEND_SMS:
					parse_port_sms(port, pack);
					break;
				case SMS_FLAG_PORT_STATE:
					parse_port_check(port, pack);
					break;
				default:
					break;
				}
			}
			
		}
	}

	return 0;
}

static int amp_write(void)
{
	struct sms_port_node *port = NULL;
	int ret = -1, i = 0, tmp = 0, send_sum = 0, sms_port_cur = 0;
	int port_sum = global_service.port_sum;
	
	if(send_over()){
		goto fail; // there is no phonenumber to send to
	}

	/*
	  *    count = 1 : first write port check
	  *    count = 2~? : wait for all port check over
	  *
	  */
	global_sms_task.amp_write_count ++;
	if(global_sms_task.amp_write_count >= 2 && global_sms_task.amp_write_count <= 100){
		usleep(1000);  // wait 100ms for all port check over
		return 0;
	}

	dlog_port_info(0);

	if(service_shm_lock(1) < 0)
		goto fail;
	
	sms_port_cur = global_service.shm->cur_sms_port; /* The current gsm port number is shm variable. */
		
	dlog(DEBUG_LEVEL1, "write port from %d by order %d\n", sms_port_cur, global_sms_task.sms_cfg.port_order);
	for(i = 0; i < port_sum; i++){
		tmp = (sms_port_cur+i)%port_sum;
		port = &global_sms_task.sms_port[tmp];
		if(port->is_selected && port->is_valid){ // port have no phonenumber
			dlog(DEBUG_LEVEL1, "2. gsm-%d.%d state[%d] timestamp[%ld.%6ld/%ld.%6ld]\n", 
				port->board,
				port->span,
				global_service.shm->sms_port_state[port->index],
				port->timestamp.tv_sec,
				port->timestamp.tv_usec,
				global_service.shm->sms_port_state_timestamp[port->index].tv_sec,
				global_service.shm->sms_port_state_timestamp[port->index].tv_usec);
			if(global_service.shm->sms_port_state[port->index] == PORT_STATE_READY
				|| (global_service.shm->sms_port_state[port->index] == PORT_STATE_SENDING 
					&& global_service.shm->sms_port_state_timestamp[port->index].tv_sec == port->timestamp.tv_sec
					&& global_service.shm->sms_port_state_timestamp[port->index].tv_usec == port->timestamp.tv_usec)
			){
				if(send_port_sms(port) == 0){
					sms_port_cur = tmp;
					send_sum ++;
				}
			}else if(global_service.shm->sms_port_state[port->index] == PORT_STATE_NOTREADY){
				send_port_check(port);
			}
		}
		
		if(send_over()){
			break;
		}
	}

	/* update cur port number */
	if(send_sum > 0){
		tmp = sms_port_cur%port_sum;
		switch(global_sms_task.sms_cfg.port_order){
		case PORT_ORDER_ROUND:
			tmp ++;
			break;
		case PORT_ORDER_RANDOM:
			tmp = random_int(0, port_sum);// [0,__PORT_SUM__-1)
			break;
		case PORT_ORDER_ASCEND:
			tmp = 0;
			break;
		default:
			tmp ++;
			break;
		}
		sms_port_cur = tmp%port_sum;
	}

	/* update shm */
	global_service.shm->cur_sms_port = sms_port_cur;
	if(service_shm_unlock(1) < 0)
		goto fail;

	if(send_sum <= 0)
		goto fail;

	return 0;
fail:
	usleep(100000); // 0.1s
	return -1;
}

static int amp_read(void)
{
	char buffer[MAX_LEN_BUFFER];
	char *p = NULL, *start = NULL, *end = NULL;
	int rest = 0, ret = 0, len = 0;

	p = global_service.recv_buf + global_service.recv_len;
	rest = sizeof(global_service.recv_buf) - global_service.recv_len;
	if(rest <= 0){
		memset(global_service.recv_buf, 0, sizeof(global_service.recv_buf));
		p = global_service.recv_buf;
		rest = sizeof(global_service.recv_buf);
	}
	
	while(rest > 0 && (ret = recv(global_service.amp_fd, p, rest, 0)) > 0){
		//dlog(DEBUG_LEVEL3, "socket read [%s]\n", global_service.recv_buf);
		p += ret;
		rest -= ret;
		start = global_service.recv_buf;
		while((end = strstr(start, "\r\n\r\n")) != NULL){
			/* find one socket package */
			*(end + sizeof("\r\n") - 1) = '\0';
			amp_parse(start);
			start = end + sizeof("\r\n\r\n") - 1;
		}
		if(start != global_service.recv_buf){
			len = strlen(start);
			memmove(global_service.recv_buf, start, len);
			memset(global_service.recv_buf + len, 0, sizeof(global_service.recv_buf) - len);
			p = global_service.recv_buf + len;
		}
	}
	
	global_service.recv_len = strlen(global_service.recv_buf);
	
	if(ret == 0){
		dlog(DEBUG_LEVEL1, "socket closed [%s], close\n", global_service.recv_buf);
		close(global_service.amp_fd);
		global_service.amp_fd = 0;
		return -1;
	}else if(ret < 0){
		/* client close  or error , then close socket */
		if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN){
			dlog(DEBUG_LEVEL1, "socket read next time [%s][%d]\n", global_service.recv_buf, global_service.recv_len);
		}else{
			dlog(DEBUG_LEVEL1, "socket error [%s], close\n", global_service.recv_buf);
			close(global_service.amp_fd);
			global_service.amp_fd = 0;
			return -1;
		}
	}
	
	dlog(DEBUG_LEVEL1, "socket read [%s]\n", global_service.recv_buf);

	return 0;
}

static int send_sms(struct sms_args *args, char *outbuf, int outlen)
{
	struct timeval select_tv = {20, 0};
	struct sms_port_node *port = NULL;
	char buffer[MAX_LEN_BUFFER];
	fd_set rset, wset;
	int ret = 0, i = 0, wait = 0, write_fail_count = 0;
	time_t begin = 0, now = 0, wait_begin = 0;

	dlog(DEBUG_LEVEL1, "\n");

	/*
	  *  init global_sms_task.
	  *  destory sms to list and close port socket for the last request.
	  *  
	  */
	srand((int)time(0));
	destroy_sms_to(&global_sms_task);
	memset(&global_sms_task, 0, sizeof(global_sms_task));
	global_sms_task.sms_cfg = global_service.sms_cfg;
	ret |= set_sms_cfg(args, &global_sms_task);
	ret |= set_sms_port(args->from, &global_sms_task);
	ret |= set_sms_to(args->to, &global_sms_task);
	ret |= set_sms_msg(args->message, &global_sms_task);
	if(ret != 0){
		dlog(DEBUG_LEVEL1, "ret [%d]\n", ret);
		destroy_sms_to(&global_sms_task);
		return -1;
	}
	
	select_tv.tv_sec = global_sms_task.sms_cfg.timeout_wait;

	dlog(DEBUG_LEVEL1, "select timeout[%d]\n", select_tv.tv_sec);
	time(&begin);
	while(1){
		/* init select read fd set and write fd set. */		
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(global_service.amp_fd, &rset);
		if(!send_over())
			FD_SET(global_service.amp_fd, &wset);
		
		if ((ret = select(global_service.amp_fd+1, &rset, &wset, NULL, &select_tv)) <= 0) {
			dlog(DEBUG_LEVEL1, "select error(<0) or timeout(==0) ret[%d] timeout[%d]\n", ret, select_tv.tv_sec);
			break;
		}

		/* readable */
		if (FD_ISSET(global_service.amp_fd, &rset)){
			dlog(DEBUG_LEVEL1, "readable\n");
			amp_read();
		}

		/* writeable */
		if (FD_ISSET(global_service.amp_fd, &wset)){
			dlog(DEBUG_LEVEL1, "writeable\n");
			if(amp_write() != 0){
				write_fail_count++;
			}else{
				write_fail_count = 0;
			}
		}

		/* something wrong occured. */
		if(global_service.amp_fd <= 0 || write_fail_count > 200){  // 20s divided by no port ready sleep duration
			/* socket close or long time no port ready, then break */
			dlog(DEBUG_LEVEL1, "socket close or 20 consecutive failures. sock[%d] fail count[%d]\n", global_service.amp_fd, write_fail_count);
			break;
		}

		/* send is over , wait result. */
		if(send_over()){
			/* send over and get report over, break */
			if(global_sms_task.sms_to_rest <= 0){
				dlog(DEBUG_LEVEL1, "send over and get report over\n");
				break;
			}
			
			/* wait timeout */
			if(wait == 0){
				time(&wait_begin);
				wait = 1;
			}else{
				time(&now);
				if((now - wait_begin) > global_sms_task.sms_cfg.timeout_wait){
					dlog(DEBUG_LEVEL1, "wait timeout [%d]\n", global_sms_task.sms_cfg.timeout_wait);
					break;
				}
			}
		}

		/* total timeout */
		if(global_sms_task.sms_cfg.timeout_total > 0){
			time(&now);
			if((now - begin) > global_sms_task.sms_cfg.timeout_total){
				dlog(DEBUG_LEVEL1, "total timeout [%d]\n", global_sms_task.sms_cfg.timeout_total);
				break;
			}	
		}
	}

	dlog(DEBUG_LEVEL1, "\n");
	if(outbuf != NULL && outlen > 0){
		if(global_sms_task.sms_cfg.report_format == SMS_REPORT_STRING){
			report_string(outbuf, outlen);
		}else{
			report_json(outbuf, outlen);
		}
	}

	dlog(DEBUG_LEVEL1, "\n");
	destroy_sms_to(&global_sms_task);
	
	return 0;
}

static int authenticate(cgicc::Cgicc CGI)
{
	const char *username 	= NULL;
	const char *password 	= NULL;
	cgicc::const_form_iterator cgi_username	= CGI.getElement("username");
	cgicc::const_form_iterator cgi_password	= CGI.getElement("password");

	/* get cgi username and password */
	if ((cgi_username != (*CGI).end()) && (!cgi_username->isEmpty())) {
		username = cgi_username->getValue().c_str();
	}
	if ((cgi_password != (*CGI).end()) && (!cgi_password->isEmpty())) {
		password = cgi_password->getValue().c_str();
	}
	//dlog(DEBUG_LEVEL1, "cgi_usr [%s] cgi_pwd [%s] valid_usr [%s] valid_pwd [%s]\n", username, password, global_service.sms_cfg.username, global_service.sms_cfg.password);

	/* check username and password */
	if(!username || !password){
		/* Authenticate fail*/
		return -1;
	}else{
		if(!strcmp(global_service.sms_cfg.username, username) && !strcmp(global_service.sms_cfg.password, password)){
			/* Authenticate success*/
			return 0;
		}else{
			/* Authenticate fail*/
			return -1;
		}
	}
}

int get_usage_info(char *usage_file)
{
	int lock;
	FILE *fp;
	unsigned long long file_len = 0;
	unsigned long long tmp_len = 0;
	unsigned long long read_len = 0;
	char *ptr_usage_file = usage_file;
	char *ptr_info = NULL;

	if (ptr_usage_file == NULL || strlen(ptr_usage_file) == 0)
		ptr_usage_file = USAGE_FILE_NAME;
	lock = lock_file(ptr_usage_file);
	fp = fopen(ptr_usage_file, "r");
	if(NULL == fp) {
		fprintf(stderr,"Can't open %s\n",ptr_usage_file);
		unlock_file(lock);
		return -1;
	}
	fseek(fp, 0L, SEEK_END);
	file_len = ftell(fp);
	usage_sendsms = (char*)malloc(file_len+1);
	ptr_info = usage_sendsms;
	tmp_len = file_len;
	fseek(fp,0,SEEK_SET);
	while( tmp_len > 0){
		read_len = fread(ptr_info, 1, tmp_len, fp);
		if(read_len < 0){
			fprintf(stderr,"fread %s failed. len=%d\n",ptr_usage_file, read_len);
			free(usage_sendsms);
			usage_sendsms = NULL;
			pclose(fp);
			unlock_file(lock);
			return -1;
		}else if(read_len == 0){
			break;
		}else{
			ptr_info += read_len;
			tmp_len -= read_len;
		}
	}
	fclose(fp);
	unlock_file(lock);

	return file_len;
}

int handle_action_sendsms(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request, redisContext *c) 
{
	using namespace cgicc;
	struct sms_args sms_args;
	memset(&sms_args,0,sizeof(sms_args));
	char outbuf[MAX_LEN_BUFFER*16], tmp[MAX_LEN_LINE];
	int board = 0, span = 0;
	char msg[4096];
	char to[4096];
	int http2sms_enable = global_service.sms_cfg.enable;
	int cors_enable = global_service.sms_cfg.enable_cors; // adding a controling of cross origin access 

	const_form_iterator cgi_to			= CGI.getElement("phonenumber");
	const_form_iterator cgi_message		= CGI.getElement("message");	
	const_form_iterator cgi_from		= CGI.getElement("port");
	const_form_iterator cgi_order		= CGI.getElement("order");
	const_form_iterator cgi_report		= CGI.getElement("report");	
	const_form_iterator cgi_timeout		= CGI.getElement("timeout");
	const_form_iterator cgi_debug		= CGI.getElement("debug");
	const_form_iterator cgi_id			= CGI.getElement("id");
	const_form_iterator cgi_url         = CGI.getElement("url");
	const_form_iterator cgi_type		= CGI.getElement("type");
	
	if(cors_enable == 1){
		*IO << "Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n";
		*IO << "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n";
		*IO << "Access-Control-Allow-Origin: "; 
		*IO << global_service.sms_cfg.allow_access_origin_url; // specify the origin url
		*IO << "\r\n";
		*IO << "Access-Control-Max-Age: 86400\r\n";
	}
		
	*IO << "Content-Type:text/plain; charset=UTF-8\n\n";
	IO->flush();

	if(debug){
		global_debug_io = IO;
		//dlog(DEBUG_LEVEL1, "Debug Level: %d\n", debug);
	}

	if(authenticate(CGI) != 0 ){
		/* Authenticate fail*/
		dlog(DEBUG_LEVEL1, "Authentication failed\n");
		*IO << "Authentication Failed: Need valid username and password\n";
		goto fail;
	}

	if(http2sms_enable != 1)
	{
		dlog(DEBUG_LEVEL1,"switch disable, http2sms_enable: %d\n", http2sms_enable);
		*IO << "Function is disable!\n";
		goto fail;
	}
	
	/*fanchao
	if(amp_ping(&global_service) != 0){
		dlog(DEBUG_LEVEL1, "Ping failed amp_fd[%d]\n", global_service.amp_fd);
		amp_logoff(&global_service);
		if(amp_login(&global_service) != 0){
			amp_logoff(&global_service);
			*IO  << "Internal connect failed\n";
			goto fail;
		}
	}
	*/
	memset(&sms_args, 0, sizeof(sms_args));

	/* add CORS controlor*/
	if ((cgi_url != (*CGI).end()) && (!cgi_url->isEmpty())) {
		sms_args.url = cgi_url->getValue().c_str();	
		if(sms_args.url != NULL){
			if(cors_enable != 1){
				dlog(DEBUG_LEVEL1, "switch disable, CORS enable: %d\n", cors_enable);
				*IO << "Enable CORS Switch is disbale; function is disable!\n";
				goto fail;
			} else {
				if(strcmp(global_service.sms_cfg.allow_access_origin_url, "*")){
					if(!strstr(sms_args.url, global_service.sms_cfg.allow_access_origin_url)){
						dlog(DEBUG_LEVEL1, "Allow access origin url: %s\n", global_service.sms_cfg.allow_access_origin_url);
						*IO << "The url is not allowed to access;function is disable!\n"; 
						goto fail;
					}
				}
			}
		}
	}

	/* get cgi value */
	if ((cgi_from != (*CGI).end()) && (!cgi_from->isEmpty())) {
		sms_args.from = cgi_from->getValue().c_str();
	}
	
	if ((cgi_to != (*CGI).end()) && (!cgi_to->isEmpty())) {
		//sms_args.to = cgi_to->getValue().c_str();
		strncpy(to,cgi_to->getValue().c_str(),sizeof(to));
		filter_to(to);
		sms_args.to = to;
		//dlog(DEBUG_LEVEL1, "sms_args.to %s\n", sms_args.to);
	}	
			
	
	//modified by Ronggang. Mar 31, 2015.  to support blank SMS;
	//if ((cgi_message != (*CGI).end()) && (!cgi_message->isEmpty())) {
	if (cgi_message != (*CGI).end()) {
		//sms_args.message = cgi_message->getValue().c_str();
		strncpy(msg,cgi_message->getValue().c_str(),sizeof(msg));
		dlog(DEBUG_LEVEL1, "cgi_message(length:%d) %s\n",strlen(msg), msg);
		filter_sms(msg);
		sms_args.message = msg;
	}
	if ((cgi_order != (*CGI).end()) && (!cgi_order->isEmpty())) {
		sms_args.order = cgi_order->getValue().c_str();
	}
	if ((cgi_report != (*CGI).end()) && (!cgi_report->isEmpty())) {
		sms_args.report = cgi_report->getValue().c_str();
	}
	if ((cgi_timeout != (*CGI).end()) && (!cgi_timeout->isEmpty())) {
		sms_args.timeout = cgi_timeout->getValue().c_str();
	}
	if ((cgi_debug != (*CGI).end()) && (!cgi_debug->isEmpty())) {
		sms_args.debug = cgi_debug->getValue().c_str();
	}

	if ((cgi_id != (*CGI).end()) && (!cgi_id->isEmpty())) {
		sms_args.id = cgi_id->getValue().c_str();
	}else{
		sms_args.id = "null";
	}

	if ((cgi_type != (*CGI).end()) && (!cgi_type->isEmpty())) {
		sms_args.type = cgi_type->getValue().c_str();
	}

	if(!sms_args.to){
		*IO << "Send SMS Fail: Need valid sms receiver number [phonenumer?]\n";
		goto fail;
	}
	
	//modified by Ronggang. Mar 31, 2015.  to support blank SMS;
	//
	//if(!sms_args.message){
	//	*IO << "Send SMS Fail: Need valid sms message [message?]\n";
	//	goto fail;
	//}

	if(sms_args.debug != NULL){
		int val = 0;
		if((val = atoi(sms_args.debug))> 0){
			debug = val;
		}		
	}
	
	memset(outbuf,0,sizeof(outbuf));
//	send_sms(&sms_args, outbuf, sizeof(outbuf)-1);
	send_sms_var_redis(&sms_args,outbuf,sizeof(outbuf)-1,c); //modify by fanchao
	(*IO) << outbuf;
	return 0;
	
fail:
	if (usage_sendsms)
		(*IO) << usage_sendsms;
	return -1;
}

