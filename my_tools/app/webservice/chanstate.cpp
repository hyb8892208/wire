#include "common.h"
#include "hiredis.h"
#include <unistd.h>

#define	CHAN_STATE_USAGE_FILE	"/etc/asterisk/gw/usage_chanstate.info"
static char *usage_chanstate = NULL;
/*
static char *usage_chanstate = "\n----------------------------------------------------\n\
Usage: http://hostname:80/chan_state?username=xxx&password=xxx\n\
	\n\
1. Parameter Description\n\
	\n\
1.1 username\n\
	Description:    The login username. Set in SMS settings page\n\
	Default Value:  None\n\
	Necessity:  Required\n\
1.2 password\n\
	Description:    The login password. Set in SMS settings page\n\
	Default Value:  None\n\
	Necessity:  Required\n\
	\n\
2. Report Format (charset: UTF-8)\n\
	\n\
2.1 JSON\n\
	{\n\
		\"The total idle channels\":\"xxx\",\n\
		\"Detialed channel status\":[{\n\
			\"1\":\"0\",\n\
			\"2\":\"0\",\n\
			\"3\":\"0\",\n\
			\"4\":\"0\",\n\
			\"5\":\"0\",\n\
			\"6\":\"0\",\n\
			\"7\":\"0\",\n\
			\"8\":\"0\",\n\
			\"9\":\"0\",\n\
			\"10\":\"0\",\n\
			\"11\":\"0\",\n\
			\"12\":\"0\",\n\
			.......\n\
		}]\n\
	}\n\
\n---------------------------------------------------\n";
*/

#if 0
static int init_http_login_info(){
	const char *file_path = "/etc/asterisk/http_service.conf";
	int fd, len;
	char buf[64]; 
	char *colon_position = NULL;
	char *end_position = NULL;
	char *start_position = NULL;

	fd = open(file_path, O_RDONLY);
	if(fd < 0){
		return 0;
	}
	/*reading http_service.conf content*/
	flock(fd, LOCK_EX);
	len = read(fd, buf, sizeof(buf));
	flock(fd, LOCK_UN);
	close(fd);
	if(len > 0){
		start_position = buf;
		colon_position = strstr(buf, ":");
		end_position = strstr(buf, "\n");
		while((end_position > colon_position) && (*(end_position - 1) == '\n') || (*(end_position - 1) == '\t') || (*(end_position - 1) == '\r') || (*(end_position - 1) == ' ')){
			end_position--;
		}
		/*get username from http_service.conf*/
		strncpy(global_service.http_info.username, buf, colon_position - start_position);
		/*get password from http_service.conf*/
		strncpy(global_service.http_info.password, colon_position + 1, end_position - colon_position - 1);

		global_service.http_info.username[colon_position - start_position] = '\0';
		global_service.http_info.password[end_position - colon_position - 1] = '\0';
	}
	return 0;
} 
#endif

static int init_http_login_info()
{
	const char *file_path = "/etc/asterisk/gw/web_server.conf";
	char line[MAX_LEN_LINE];
	char key[MAX_LEN_LINE];
	char value[MAX_LEN_LINE];
	char section[MAX_LEN_LINE];
	char kv_format[64], s_format[64];
	int val = 0, i = 0, j = 0;
	struct http_service_info *ptr_info = &global_service.http_info;

	FILE* fp;
	int lock;

	lock = lock_file(file_path);
	if( NULL == (fp = fopen(file_path,"r")) ) {
		fprintf(stderr,"Can't open %s\n",file_path);
		strncpy(ptr_info->username,"admin",sizeof(ptr_info->username));
		strncpy(ptr_info->password,"admin",sizeof(ptr_info->password));
		unlock_file(lock);
		return -1;
	}

	/* set sscanf format */
	memset(s_format, 0, sizeof(s_format));
	memset(kv_format, 0, sizeof(kv_format));
	snprintf(s_format, sizeof(s_format), "[%%%d[^]]]", sizeof(section));
	snprintf(kv_format, sizeof(kv_format), "%%%d[^=]=%%%ds", sizeof(key), sizeof(value));

	/* read config file */
	memset(line, 0, sizeof(line));
	memset(section, 0, sizeof(line));
	while(!feof(fp) && fgets(line, sizeof(line), fp)) {
		memset(key,0,sizeof(key));
		memset(value,0,sizeof(value));
		
		if(sscanf(line, s_format, section) == 1){
			trim(section);
		}else if(section[0] != '\0' && sscanf(line, kv_format, key, value) == 2){
			trim(key);
			trim(value);
			if(!strcasecmp(section,"general")){
				if(!strcasecmp(key,"username")) {
					strncpy(ptr_info->username,value,sizeof(ptr_info->username));
				}else if(!strcasecmp(key,"password")) {
					strncpy(ptr_info->password,value,sizeof(ptr_info->password));
				}else if(!strcasecmp(key, "usage_file")){ /* Add paramter of usage_file */
					strncpy(ptr_info->usage_file, value, sizeof(ptr_info->usage_file));
				}
			}
		}
		memset(line,0,sizeof(line));
	}

	fclose(fp);
	unlock_file(lock);
}
static int authenticate(cgicc::Cgicc CGI){
	const char *username	= NULL;
	const char *password	= NULL;
	cgicc::const_form_iterator cgi_username = CGI.getElement("username");
	cgicc::const_form_iterator cgi_password = CGI.getElement("password");

	/* get cig username and password*/
	if ((cgi_username != (*CGI).end()) && (!cgi_username->isEmpty())) {
		username = cgi_username->getValue().c_str();
	}
	if ((cgi_password != (*CGI).end()) && (!cgi_password->isEmpty())) {
		password = cgi_password->getValue().c_str();
	}
	/*check username and pasword*/
	if(!username || !password ){
		/*Authenticate fail*/
		return -1;
	} else {
		if( !strcmp(global_service.http_info.username, username) && !strcmp(global_service.http_info.password, password)) {
			/*Authenticate success*/
			return 0;
		} else {
			/*Authenticate fail*/
			return -1;
		}
	}
}
static int process_command(char *outbuf, int outlen, redisContext *c){
	char buf[MAX_LEN_BUFFER*4], tmp[MAX_LEN_BUFFER*4], chan_status[10];
	int counts = 0, m_bits = 0, port_sum;
	int cluster_mode = global_service.cluster_cfg.mode;
	const char *chan_callstatus_key = "app.asterisk.gsmstatus.channel";

	/*get all content of hashtable app.asterisk.gsmstatus.channel*/
	redisReply * reply = (redisReply *)redisCommand(c, "hgetall %s", chan_callstatus_key);
	if ( NULL == reply ) {
		dlog(DEBUG_LEVEL1, "Fail to execute command hgetall %s", chan_callstatus_key);
		freeReplyObject(reply);
		return -1;
	}
	port_sum = reply->elements/2;

	memset(buf, 0, sizeof(buf));
	memset(tmp, 0, sizeof(tmp));
	for (int i = 0; i < port_sum; i++){
		if(strcmp(reply->element[2 * i + 1]->str,"0") == 0) { /*"0" represent of channel idel*/
			counts++;
		}
		/*converting the data into jason*/
		sprintf(chan_status, "\"%s\":\"%s\"", reply->element[2 * i]->str, reply->element[2 * i + 1]->str);
		sprintf(tmp +  m_bits, "%s,", chan_status);
		m_bits += (strlen(chan_status)+1 ) ;
	}
	strncpy(buf, tmp, strlen(tmp) - 1);
	if(outbuf != NULL) {
		snprintf(outbuf, outlen,
			"{\"The total idle channels\":\"%d\",\"Detailed channel state\":[{%s}]}", 
			counts, buf);
	}
	freeReplyObject(reply);
	
	return 0;
}

int get_chanstate_usage_info(const char *usage_file)
{
	int lock;
	FILE *fp;
	unsigned long long file_len = 0;
	unsigned long long tmp_len = 0;
	unsigned long long read_len = 0;
	const char *ptr_usage_file;
	char *ptr_info = NULL;

	if (usage_file == NULL || strlen(usage_file) == 0)
		ptr_usage_file = CHAN_STATE_USAGE_FILE;
	else 
		ptr_usage_file = usage_file;
	lock = lock_file(ptr_usage_file);
	fp = fopen(ptr_usage_file, "r");
	if(NULL == fp) {
		fprintf(stderr,"Can't open %s\n",ptr_usage_file);
		unlock_file(lock);
		return -1;
	}
	fseek(fp, 0L, SEEK_END);
	file_len = ftell(fp);
	usage_chanstate = (char*)malloc(file_len+1);
	ptr_info = usage_chanstate;
	tmp_len = file_len;
	fseek(fp,0,SEEK_SET);
	while( tmp_len > 0){
		read_len = fread(ptr_info, 1, tmp_len, fp);
		if(read_len < 0){
			fprintf(stderr,"fread %s failed. len=%d\n",ptr_usage_file, read_len);
			free(usage_chanstate);
			usage_chanstate = NULL;
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

int handle_action_chan_state(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request, redisContext *c){
	using namespace cgicc;
	char outbuf[MAX_LEN_BUFFER*4];
	init_http_login_info();
	
	*IO << "Content-Type:text/plain; charset=UTF-8\n\n";
	IO->flush();
	if(authenticate(CGI) != 0 ){
		/* Authenticate fail */
		dlog(DEBUG_LEVEL1, "Authentication failed\n");
		*IO << "Authentication Failed: Need valid username and password\n";
		goto fail;
	}
	memset(outbuf,0,sizeof(outbuf));

	process_command(outbuf, sizeof(outbuf)-1, c);
	/*output the data*/
	(*IO) << outbuf;
	IO->flush();
	return 0;

fail:
	(*IO) << usage_chanstate;
	return -1;
}
