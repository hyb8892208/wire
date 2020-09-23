#include "common.h"
#include <unistd.h>
#include "sqlite3.h"
#define SIMSTATUS_USAGE_FILE_NAME "/etc/asterisk/gw/usage_simstatus.info"
#define SMS_OUTBOX_DB "/data/log/smsoutbox.db"
typedef enum sms_outbox_row_e{
	SMS_OUTBOX_ID = 0,
	SMS_OUTBOX_PORT,
	SMS_OUTBOX_PHONENUMBER,
	SMS_OUTBOX_TIME,
	SMS_OUTBOX_MR,
	SMS_OUTBOX_MESSAGE,
	SMS_OUTBOX_STATUS,
	SMS_OUTBOX_UUID,
	SMS_OUTBOX_IMSI,
	SMS_OUTBOX_CAUSE,
	SMS_OUTBOX_MAX,
}sms_outbox_row_t;

#define SMSSTATUS_FAIL      "FAILED"
#define SMSSTATUS_SEND     "SEND"
#define SMSSTATUS_DRIVED  "DELIVERD"
#define CHANNEL_STATUS_PATH "/tmp/gsm"

static char *usage_simstatus = NULL;
int callback_flag = 0;//There's no way to tell the difference when sqlite is no data available. so, this used to issue data is available
const char *get_smsstatus(char *status)
{
	int int_status = 0;
	if(status)
		int_status = atoi(status);
	switch(int_status){
		case 0:
			return SMSSTATUS_FAIL;
			break;
		case 1:
			return SMSSTATUS_SEND;
			break;
		case 2:
			return SMSSTATUS_DRIVED;
			break;
	}
}

static int smsreporst_callback( void*data,int argc, char **argv, char **azColName)
{
	cgicc::FCgiIO *IO = (cgicc::FCgiIO *)data;
	char outbuf[4096] = {0};
	char imsi[64] ={0};
#if 0
	const char *port_tag = "port";
	const char *time_tag = "time";
	const char *uuid_tag = "uuid";
	const char *message_tag = "message";
	const char *status_tag = "status";
#endif
	const char *port_tag = global_service.get_smsstatus_cfg.url_port;
	const char *time_tag = global_service.get_smsstatus_cfg.url_time;
	const char *uuid_tag = global_service.get_smsstatus_cfg.url_id;
	const char *imsi_tag = global_service.get_smsstatus_cfg.url_imsi;
//	const char *uuid_tag = "id";
//	const char *imsi_tag = "imsi";
	const char *cause_tag = "cause";
	const char *message_tag = global_service.get_smsstatus_cfg.url_message;
	const char *status_tag =  global_service.get_smsstatus_cfg.url_status;
	const char *num_tag = global_service.get_smsstatus_cfg.url_num;
	
	callback_flag = 1;
	if(argc > 0){
		snprintf(outbuf+strlen(outbuf), sizeof(outbuf)-3,
									"{\n\t\t\t\t\"%s\":\"%s\","
									"\n\t\t\t\t\"%s\":\"%s\","
									"\n\t\t\t\t\"%s\":\"%s\","
									"\n\t\t\t\t\"%s\":\"%s\","
									"\n\t\t\t\t\"%s\":\"%s\","
									"\n\t\t\t\t\"%s\":\"%s\","
									"\n\t\t\t\t\"%s\":\"%s\""
									"\n\t\t\t}",
									uuid_tag, argv[SMS_OUTBOX_UUID],
									num_tag, argv[SMS_OUTBOX_PHONENUMBER],
									imsi_tag, argv[SMS_OUTBOX_IMSI],
									port_tag, argv[SMS_OUTBOX_PORT],
									time_tag, argv[SMS_OUTBOX_TIME],
									status_tag, get_smsstatus(argv[SMS_OUTBOX_STATUS]),
									cause_tag, argv[SMS_OUTBOX_CAUSE]
									);

	}else{
		*IO << "This SMS is not exist\n";
	}
	
	*IO << outbuf;
	*IO << "\n" ;
	return 0;
}

int handle_action_get_smsreports(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request)
{
	int err;
	sqlite3 *sqlite_hander = NULL;
	using namespace cgicc;
	char outbuf[MAX_LEN_BUFFER*16], tmp[MAX_LEN_LINE];
	char sqlite_cmd[128];
	const char *num;
	const char *uuid;
	char *szErrMsg = NULL;
	int get_smsstatus_enabe = global_service.get_smsstatus_cfg.get_smsstatus_enabe;
	callback_flag = 0;
	const_form_iterator cgi_num		= CGI.getElement("phonenumber");	//phone num
	const_form_iterator cgi_uuid		= CGI.getElement("id");   //uuid
	
	*IO << "Content-Type:text/html\n\n";
	IO->flush();
	if((cgi_num!= (*CGI).end()) && (!cgi_num->isEmpty())){
		num = cgi_num->getValue().c_str();
	}else{
		num = "";
	}
	if((cgi_uuid!= (*CGI).end()) && (!cgi_uuid->isEmpty())){
		uuid = cgi_uuid->getValue().c_str();
	}else{
		uuid = "";
	}

//	if(get_smsstatus_enabe == 0){
//		*IO << "function is disable\n";
//	}
	
	if(strlen(num) == 0){
		*IO << "phonenum is not valild\n";
		return -1;
	}else if(strlen(uuid) > 0){//strlen(num) >0 && strlen(uuid) > 0
		snprintf(sqlite_cmd, sizeof(sqlite_cmd),"SELECT * FROM sms_out WHERE phonenumber=\"%s\" AND uuid=\"%s\" order by id desc Limit 1", num, uuid);
	}else{//strlen(num) > 0 && strlen(uuid) == 0
		snprintf(sqlite_cmd, sizeof(sqlite_cmd),"SELECT * FROM sms_out WHERE phonenumber=\"%s\" order by id desc Limit 1", num);
	}
	
	err =  sqlite3_open(SMS_OUTBOX_DB, &sqlite_hander);
	if(err < 0){
		*IO << "failed";
	}
	 err = sqlite3_exec(sqlite_hander, sqlite_cmd, smsreporst_callback, IO, &szErrMsg);
	 if(szErrMsg){
		sqlite3_free(szErrMsg);
	 }
	 
	 sqlite3_close(sqlite_hander);
	 
	 if(err||callback_flag == 0){
		*IO << "This SMS is not exist\n";
	 }
	 
	return 0;
}

int get_simstatus_usage_info(char *usage_file)
{
	int lock;
	FILE *fp;
	unsigned long long file_len = 0;
	unsigned long long tmp_len = 0;
	unsigned long long read_len = 0;
	char *ptr_usage_file = usage_file;
	char *ptr_info = NULL;

	if (ptr_usage_file == NULL || strlen(ptr_usage_file) == 0)
		ptr_usage_file = SIMSTATUS_USAGE_FILE_NAME;
	lock = lock_file(ptr_usage_file);
	fp = fopen(ptr_usage_file, "r");
	if(NULL == fp) {
		fprintf(stderr,"Can't open %s\n",ptr_usage_file);
		unlock_file(lock);
		return -1;
	}
	fseek(fp, 0L, SEEK_END);
	file_len = ftell(fp);
	usage_simstatus = (char*)malloc(file_len+1);
	ptr_info = usage_simstatus;
	tmp_len = file_len;
	fseek(fp,0,SEEK_SET);
	while( tmp_len > 0){
		read_len = fread(ptr_info, 1, tmp_len, fp);
		if(read_len < 0){
			fprintf(stderr,"fread %s failed. len=%d\n",ptr_usage_file, read_len);
			free(usage_simstatus);
			usage_simstatus = NULL;
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

static char* __cut_line_str(char* buf, const char* start, const char* end, char* save, int size)
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
}

static int get_one_port_simstatus(int port,char *outbuf, int outbuf_len){
	char filepath[32];
	char buf[1024];
	char network_status[32];
	char status[64];
	char imsi[64];
	const char *imsi_tag = "imsi";
	const char *status_tag = "status";
	char *p = NULL;
	sprintf(filepath, "%s/%d", CHANNEL_STATUS_PATH, port);
	if(file_get_contents(filepath, buf, sizeof(buf)) < 0){
		return -1;
	}
	p = __cut_line_str(buf,"Network Status:","\n",network_status,sizeof(network_status));
	p = __cut_line_str(p, "SIM IMSI:","\n",imsi,sizeof(imsi));
	p = __cut_line_str(p,"State:","\n",status,sizeof(status));
	if(strstr(status,"Undetected SIM Card")){
		strncpy(network_status,"Undetected SIM Card",sizeof(network_status));
	}
	snprintf(buf, sizeof(buf), "\n\t\t\t\"%d\":"
						   "[{\n\t\t\t\t\"%s\":\"%s\","
						   "\n\t\t\t\t\"%s\":\"%s\""
						   "\n\t\t\t}],",
						   port,imsi_tag,  imsi,
						   status_tag,network_status
		                               );
	strncat(outbuf, buf,outbuf_len);
}
int handle_action_get_simstatus(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request)
{
	using namespace cgicc;
	char outbuf[8192] = {0};
	
	int i = 0, start_port, end_port;
	
	const char *port;
	
	const_form_iterator cgi_port = CGI.getElement("port");
	
	*IO << "Content-Type:text/plain; charset=UTF-8\n\n";
	IO->flush();

	if((cgi_port!= (*CGI).end()) && (!cgi_port->isEmpty())){
		port = cgi_port->getValue().c_str();
	}else{
		port = "";
	}
	
	if(strlen(port) == 0){
		*IO << "Invalid parameter port\r\n";
		goto fail;
	}
	
	if(port[0] == '0' && atoi(port) == 0){
		start_port = 1;
		end_port = global_service.ussd_cfg.total_channel+1;
	}else{
		start_port = atoi(port);
		end_port = start_port + 1;
		if(start_port <= 0){
			*IO << "Invalid parameter port, out of range\r\n";
			goto fail;	
		}else if(start_port > global_service.ussd_cfg.total_channel){
			*IO << "Invalid parameter port, out of range\r\n";
			goto fail;	
		}
	}
	for(i = start_port; i < end_port ; i++)
	{
		get_one_port_simstatus(i, outbuf, sizeof(outbuf));
	}
	outbuf[strlen(outbuf) - 1] = '\0';
	*IO << "\t\t{";
	*IO << outbuf;
	*IO << "\n\t\t}";
	*IO << "\n";
	return 0;
fail:
	if(usage_simstatus)
		*IO << usage_simstatus;
	return -1;
}

int handle_action_get_smsremain(cgicc::Cgicc CGI, cgicc::FCgiIO *IO, FCGX_Request *request, redisContext *c){
	using namespace cgicc;
	char outbuf[512] = {0};
	char tmpbuf[32] = {0};
	int total_chan = global_service.ussd_cfg.total_channel;
	char *p = NULL;
	redisReply *reply = NULL;
	int i = 0;
	sprintf(outbuf, "{\"Total_channel\":\"%d\", \"Smsremain\":[{", total_chan);
	for(i = 0; i < total_chan ;i++){
		reply = (redisReply *)redisCommand(c,"llen app.asterisk.async.smslist.%d",i+1);
		if(reply->type != REDIS_REPLY_INTEGER ){
			sprintf(tmpbuf, "\"%d\":\"%d\",", i+1, 0);
		}else{
			sprintf(tmpbuf, "\"%d\":\"%d\",", i+1, reply->integer);
		}
		strcat(outbuf, tmpbuf);
	}
	if(total_chan > 0)
		outbuf[strlen(outbuf)-1]='\0';
	strcat(outbuf, "}]}");

	*IO << "Content-Type:text/html\n\n";
	 IO->flush();
	*IO << outbuf;
	*IO << "\n";
}