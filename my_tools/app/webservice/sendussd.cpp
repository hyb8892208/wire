#include "common.h"
#include "hiredis.h"
#include <unistd.h>

static char *usage_ussd = NULL;
#define USSD_USAGE_FILE_NAME "/etc/asterisk/gw/usage_sendussd.info"

static int http2ussd_authenticate(cgicc::Cgicc CGI){

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
	/* check username and password */
	if(!username || !password){
		/* Authenticate fail*/
		return -1;
	}else{
		if(!strcmp(global_service.ussd_cfg.username, username) && !strcmp(global_service.ussd_cfg.password, password)){
			/* Authenticate success*/
			return 0;
		}else{
			/* Authenticate fail*/
			return -1;
		}
	}
	return 0;
}

static int check_httpussd_port(int port,redisContext *c)
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

static int get_rand_httpussd_port(redisContext *c)
{
	int port = 0;
	int i = 0,j = 0,min_list = 0xFF,max_idx = 0;
	int chan[__GSM_SUM__] = {0};
	int index = 0,tmp = 0;
	int online = 0;
	redisReply *reply = NULL;
	struct timeval tv;
	int ret = 0;
		
	for(i = 0; i < __GSM_SUM__;i++){
		if(1 == global_service.ussd_cfg.port_valid[0][i]){
			chan[j++] = i + 1;
		}
	}
	if(j == 0){
		return -1;
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
		ret = check_httpussd_port(chan[i],c);
		
		if(STATUS_SIM_OK != ret){
			continue;
		}

		return chan[i];
	}
	
	return -1;
}
static void compile_output_buffer(int format,int index, int channel, const char *id, char *result,char *outbuf, int outlen){
	char time_str[64] = {0};
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
	switch (format){
		case USSD_REPORT_JSON:
			snprintf(outbuf+strlen(outbuf), outlen-3,"\n\t\t\t\"%d\":"
								"[{\n\t\t\t\t\"port\":\"%d\","
								"\n\t\t\t\t\"time\":\"%s\","
								"\n\t\t\t\t\"id\":\"%s\","
								"\n\t\t\t\t\"result\":\"%s\""
								"\n\t\t\t}],",
								index,
								channel,
								time_str,
								id,
								result
								);
			break;
		case USSD_REPORT_STRING:
			snprintf(outbuf+strlen(outbuf), outlen,"--record %d start--\n"\
								"port: %d\n"\
								"time: %s\n"\
								"id:%s\n"\
								"result: %s\n"\
								"--record %d end--\n"\
								"\n",\
								index,channel,time_str,id,result, index);
			break;
		default:
			strcat(outbuf,result);
			break;
	}
}

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


int set_ussd_redis(struct ussd_args *args, char *outbuf, int outlen,redisContext *c ){
	char result[32] = {0};
	char from_str[4] = {0};
	char to_str[24] = {0};
	char *tmp_to = (char *)args->port;
	char buf[1280] = {0};
	int ret = 0,send_num = 0;
	FILE *fp = NULL;
	int port = 0;
	int report_fmt = global_service.ussd_cfg.result_format;
	redisReply * reply = NULL;
	int timeout = 0, debug = 0;
	switch(report_fmt)
	{
		case USSD_REPORT_JSON:
			sprintf(outbuf,"\t{\n\t\t\"message\":\"%s\","\
				"\n\t\t\"report\":[{",args->message);
			break;
		case USSD_REPORT_STRING:
			sprintf(outbuf,"message:%s\n\n",args->message);
			break;
		default:
			break;
	}


	do
	{	
		if(tmp_to)
			tmp_to = get_next_token(tmp_to ,',',to_str);

		if(strlen(to_str) == 0){
			port = get_rand_httpussd_port(c);
		}else{
			port = atoi(to_str);
		}
		
		if(port < 0)
			ret = STATUS_SIM_UNWORK;
		else
			ret = check_httpussd_port(port, c);
		
		switch(ret)
		{
			case STATUS_SIM_OK:
				snprintf(buf,1280,"{\"type\":\"asyncussd\","\																															 
					"\"msg\":\"%s\","\
					"\"timeout\":\"%s\","\
					"\"id\":\"%s\"}",
					args->message,
					args->timeout?args->timeout:"",
					args->id);
				
				reply =(redisReply *)redisCommand(c, "rpush app.asterisk.async.ussdlist.%d %s", port, buf);
				dlog(DEBUG_LEVEL1,"sms buf is :  %s\n",buf);
				if (NULL == reply)
				{
					dlog(DEBUG_LEVEL1,"Fail to execute command rpush ");
					strcpy(result,USSD_REPORT_FAIL);
				}else{
					strcpy(result,USSD_REPORT_SENDING);
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
		compile_output_buffer(report_fmt, send_num, port, args->id,result, outbuf, outlen);
		
	}while(NULL != tmp_to);


	switch(report_fmt)
	{
		case USSD_REPORT_JSON:
			sprintf(outbuf+strlen(outbuf)-1,"\n\t\t}]\n\t}");
			break;
		case USSD_REPORT_STRING:
		default:
			break;
	}
}

static void filter_msg(char *msg)
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
int get_ussd_usage_info(char *usage_file)
{
	int lock;
	FILE *fp;
	unsigned long long file_len = 0;
	unsigned long long tmp_len = 0;
	unsigned long long read_len = 0;
	char *ptr_usage_file = usage_file;
	char *ptr_info = NULL;

	if (ptr_usage_file == NULL || strlen(ptr_usage_file) == 0)
		ptr_usage_file = USSD_USAGE_FILE_NAME;
	lock = lock_file(ptr_usage_file);
	fp = fopen(ptr_usage_file, "r");
	if(NULL == fp) {
		fprintf(stderr,"Can't open %s\n",ptr_usage_file);
		unlock_file(lock);
		return -1;
	}
	fseek(fp, 0L, SEEK_END);
	file_len = ftell(fp);
	usage_ussd = (char*)malloc(file_len+1);
	ptr_info = usage_ussd;
	tmp_len = file_len;
	fseek(fp,0,SEEK_SET);
	while( tmp_len > 0){
		read_len = fread(ptr_info, 1, tmp_len, fp);
		if(read_len < 0){
			fprintf(stderr,"fread %s failed. len=%d\n",ptr_usage_file, read_len);
			free(usage_ussd);
			usage_ussd = NULL;
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

int handle_action_send_ussd(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request, redisContext *c) 
{
	using namespace cgicc;
	struct ussd_args ussd_args;
	memset(&ussd_args,0,sizeof(ussd_args));
	char outbuf[MAX_LEN_BUFFER*16], tmp[MAX_LEN_LINE];
	int board = 0, span = 0;
	char msg[4096];
	char to[4096];
	int http2ussd_enable = global_service.ussd_cfg.enable;
	int cors_enable = global_service.ussd_cfg.enable_cors; // adding a controling of cross origin access 

	const_form_iterator cgi_message		= CGI.getElement("message");	
	const_form_iterator cgi_from		= CGI.getElement("port");
	const_form_iterator cgi_order		= CGI.getElement("order");
	const_form_iterator cgi_timeout		= CGI.getElement("timeout");
	const_form_iterator cgi_url         = CGI.getElement("url");
	const_form_iterator cgi_type		= CGI.getElement("type");
	const_form_iterator cgi_result		= CGI.getElement("result");
	const_form_iterator cgi_id                 = CGI.getElement("id");

	if(cors_enable == 1){
		*IO << "Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n";
		*IO << "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n";
		*IO << "Access-Control-Allow-Origin: "; 
		*IO << global_service.ussd_cfg.allow_access_origin_url; // specify the origin url
		*IO << "\r\n";
		*IO << "Access-Control-Max-Age: 86400\r\n";
	}
		
	*IO << "Content-Type:text/plain; charset=UTF-8\n\n";
	IO->flush();

	if(http2ussd_authenticate(CGI) != 0 ){
		/* Authenticate fail*/
		dlog(DEBUG_LEVEL1, "Authentication failed\n");
		*IO << "Authentication Failed: Need valid username and password\n";
		goto fail;
	}

	if(http2ussd_enable != 1)
	{
		dlog(DEBUG_LEVEL1,"switch disable, http2sms_enable: %d\n", http2ussd_enable);
		*IO << "Function is disable!\n";
		goto fail;
	}
	
	memset(&ussd_args, 0, sizeof(ussd_args));

	if ((cgi_id != (*CGI).end()) && (!cgi_id->isEmpty())) {
		ussd_args.id = cgi_id->getValue().c_str();
	}else{
		ussd_args.id = "null";
	}

	if((cgi_timeout!= (*CGI).end()) && (!cgi_timeout->isEmpty())){
		ussd_args.timeout = cgi_timeout->getValue().c_str();
	}else{
		ussd_args.timeout="";
	}
	
	/* add CORS controlor*/
	if ((cgi_url != (*CGI).end()) && (!cgi_url->isEmpty())) {
		ussd_args.url = cgi_url->getValue().c_str();	
		if(ussd_args.url != NULL){
			if(cors_enable != 1){
				dlog(DEBUG_LEVEL1, "switch disable, CORS enable: %d\n", cors_enable);
				*IO << "Enable CORS Switch is disbale; function is disable!\n";
				goto fail;
			} else {
				if(strcmp(global_service.ussd_cfg.allow_access_origin_url, "*")){
					if(!strstr(ussd_args.url, global_service.ussd_cfg.allow_access_origin_url)){
						dlog(DEBUG_LEVEL1, "Allow access origin url: %s\n", global_service.ussd_cfg.allow_access_origin_url);
						*IO << "The url is not allowed to access;function is disable!\n"; 
						goto fail;
					}
				}
			}
		}
	}

	/* get cgi value */
	if ((cgi_from != (*CGI).end()) && (!cgi_from->isEmpty())) {
		ussd_args.port = cgi_from->getValue().c_str();
	}
	
	if (cgi_message != (*CGI).end()) {
		strncpy(msg,cgi_message->getValue().c_str(),sizeof(msg));
		dlog(DEBUG_LEVEL1, "cgi_message(length:%d) %s\n",strlen(msg), msg);
		filter_msg(msg);
		ussd_args.message = msg;
	}else
		goto fail;
	
	if ((cgi_result != (*CGI).end()) && (!cgi_result->isEmpty())) {
		ussd_args.result = cgi_result->getValue().c_str();
	}

	if ((cgi_type != (*CGI).end()) && (!cgi_type->isEmpty())) {
		ussd_args.type = cgi_type->getValue().c_str();
	}
	
	memset(outbuf,0,sizeof(outbuf));
	set_ussd_redis(&ussd_args, outbuf, sizeof(outbuf)-1, c);
	(*IO) << outbuf;
	return 0;
	
fail:
	if (usage_ussd)
		(*IO) << usage_ussd;
	return -1;
}

