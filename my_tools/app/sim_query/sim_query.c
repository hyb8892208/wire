
#include "sim_query.h"
#include "hiredis.h"
struct LOG_DEBUG_S debug_prefix[4] = 
{
	{DEBUG,"[DEBUG]",""},
	{WARN,"[WARN]",YELLOW},
	{ERROR,"[ERROR]",RED},
	{NOTICE,"[NOTICE]",CYAN},
};


int g_log_class = DEBUG;
FILE *g_log_fp = NULL;


void z_printf(int bits,int line,char *format,...)
{
	if(bits < g_log_class){
		return;
	}
	
	va_list va_args;
	char buf[1024] = {0};
	char* pbuf = buf;
	char str_time[24] = {0};
	time_t t = time(NULL);
	va_start(va_args, format);
	if(DEBUG == g_log_class)
	{
		strftime(str_time,24,"%Y-%m-%d %X",localtime(&t)); 
		pbuf += sprintf(pbuf, "[%s]", str_time);
	}
	
	pbuf += sprintf(pbuf, "%s{%d}:%s",debug_prefix[bits].color,line,debug_prefix[bits].prefix_bits);
	vsnprintf(pbuf, 1024, format, va_args);
	if(NULL == g_log_fp){
		printf("%s%s\n",buf,RESET);
	}else{
		fprintf(g_log_fp,"%s%s\n",buf,RESET);
		fflush(g_log_fp);
	}
	va_end(va_args);

}

void open_log_file(char *filename)
{
	g_log_fp = fopen(filename,"ab+");
	if(NULL == g_log_fp){
		debug(ERROR,"open %s error",filename);
	}
	debug(NOTICE,"................%s %s..................",__DATE__,__TIME__);
}

int checkProcessExist(char *proc_name)
{
    int fd = open(proc_name,O_RDONLY|O_CREAT);
    if(-1 != fd){
        if(-1 ==  flock(fd,LOCK_EX|LOCK_NB)){
            debug(ERROR,"%s is running...\n",proc_name);
            close(fd);
            return -1;
        }
    }
    else{
        debug(ERROR,"%s::No such file or directory !!!\n",proc_name);
        return -1;
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
		debug(ERROR ,"Can't open %s",file_path);
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
				break;
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	return 0;
}

/**************************************************************************** 
* 函数名称 : match_test
* 功能描述 : 短信匹配测试函数
* 参    数 : optarg
			o:匹配测试类型:余额匹配和号码匹配
			k:关键字
			m:短信内容
			p:小数点符号，默认'.'
			t:千分位符号，默认','
* 返 回 值 : 0 匹配失败 >0匹配成功
* 作    者 : liyezhen
* 设计日期 : 2017-9-5
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/

void match_test(int argc,char **argv)
{
	int result;
	char message[1024] = {0};
	char key_words[32] = {0};
	char query_type = UNKNOWN;
	char buff[32] = {0};
	char point_char = '.';
	char thouandth_char = ',';
	
	while( (result = getopt(argc, argv, "o::k::m::p::t::")) != -1 )
	{
		switch(result)
		{
			case 'o':
				query_type = atoi(optarg);
				break;
			case 'k':
				strcpy(key_words,optarg);
				break;
			case 'm':
				strncpy(message,optarg,sizeof(message));
				break;
			case 'p':
				point_char = optarg[0];
				break;
			case 't':
				thouandth_char = optarg[0];
				break;
			default:
				break;
		}
	}

	if(BALANCE_QUERY & query_type){
		SMSBanlanceStringMatch(message,key_words,buff,point_char,thouandth_char);
	}
	else if(PHONENUM_QUERY & query_type){
		SMSPhoneNumStringMatch(message,key_words,buff);
	}
	fprintf(stdout,"%s\n",buff);
}

int init_conf(QUERY_INFO_T *head)
{
	QUERY_INFO_T *p = NULL;
	char context[32] = {0};
	int i = 0;
	get_config(HW_INFO_FILE,"sys","total_chan_count",context);
	int port_num = 0;
	port_num = atoi(context);

	debug(DEBUG,"%d channels",port_num);
	for(i = 0; i < port_num;i++)
	{
		p = (QUERY_INFO_T *)malloc(sizeof(QUERY_INFO_T));
		if(NULL == p)
		{
			debug(ERROR,"malloc %d bytes error:%s",sizeof(QUERY_INFO_T),strerror(errno));
			exit(-1);
		}
		memset(p,0,sizeof(QUERY_INFO_T));
		p->bal_chan_stat = '1';
		p->num_chan_stat = '1';
		p->query_phonenum_counts= 0;
		sem_init(&p->sem,0,0);
		pthread_mutex_init(&p->flag_mtx,NULL);
		head->next = p;
		head = p;
	}
	return port_num;
}

int conf_reload(QUERY_INFO_T *param)
{
	char context[128] = {0};
	char channel[3];
	int i = 0;
	QUERY_INFO_T *p = param->next;

	for(i = 1;i < MAX_CHN && p;i++,p=p->next)
	{
		sprintf(channel,"%d",i);
		if(1 == get_config(CONF_FILE,channel,"bal_match_key",context)){
			strcpy(p->balance_param.match_key,context);
		}
		if(1 == get_config(CONF_FILE,channel,"bal_send_msg",context)){
			strcpy(p->balance_param.send_msg,context);
		}
		if(1 == get_config(CONF_FILE,channel,"bal_dst_num",context)){
			strcpy(p->balance_param.dst_num,context);
		}
		if(1 == get_config(CONF_FILE,channel,"bal_recv_num",context)){
			strcpy(p->balance_param.recv_num,context);
		}

		if(1 == get_config(CONF_FILE,channel,"phonenum_match_key",context)){
			strcpy(p->phone_num_param.match_key,context);
		}
		if(1 == get_config(CONF_FILE,channel,"phonenum_send_msg",context)){
			strcpy(p->phone_num_param.send_msg,context);
		}
		if(1 == get_config(CONF_FILE,channel,"phonenum_dst_num",context)){
			strcpy(p->phone_num_param.dst_num,context);
		}
		if(1 == get_config(CONF_FILE,channel,"phonenum_recv_num",context)){
			strcpy(p->phone_num_param.recv_num,context);
		}
		

		if(1 == get_config(CONF_FILE,channel,"query_type",context)){
			p->query_type = atoi(context);
		}
		if(1 == get_config(CONF_FILE,channel,"interval",context)){
			p->interval = atoi(context) * 60;
		}
		if(1 == get_config(CONF_FILE,channel,"registered_query",context)){
			p->registered_query = atoi(context);
		}
		if(1 == get_config(CONF_FILE,channel,"call_counts_query",context)){
			if(0 < atoi(context)){
				p->call_query_counts = (p->call_counts_query * p->call_query_counts) / atoi(context);
			}
			p->call_counts_query = atoi(context);
		}
		if(1 == get_config(CONF_FILE,channel,"point_char",context)){
			p->point_char = context[0];
		}else{
			p->point_char = '.';
		}
		if(1 == get_config(CONF_FILE,channel,"thousandth_char",context)){
			p->thousandth_char = context[0];
		}else{
			p->thousandth_char = ',';
		}
		
		p->channel = i;
		debug(DEBUG,"%d-Channel query_type=%d,interval=%d seconds,registered_query=%d,call_counts_query=%d.point_char=%c,thousandth_char=%c",\
			i,p->query_type,p->interval,p->registered_query,p->call_counts_query,p->point_char,p->thousandth_char);
		debug(DEBUG,"bal_match_key:%s,bal_send_msg:%s,bal_dst_num:%s,bal_recv_num:%s,phonenum_match_key:%s,phonenum_send_msg:%s,phonenum_dst_num:%s,phonenum_recv_num:%s",\
			p->balance_param.match_key,p->balance_param.send_msg,p->balance_param.dst_num,p->balance_param.recv_num,p->phone_num_param.match_key,p->phone_num_param.send_msg,\
			p->phone_num_param.dst_num,p->phone_num_param.recv_num);
	}	
	return 0;
}	

/**************************************************************************** 
* 函数名称 : SMSPhoneNumStringMatch
* 功能描述 : 短信匹配号码函数
* 参    数 : char *dst: 短信内容
			char *src:匹配关键字
			char *context:匹配内容
* 返 回 值 : 0 匹配失败 >0匹配成功
* 作    者 : liyezhen
* 设计日期 : 2017-9-5
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int SMSPhoneNumStringMatch(char *dst,char *key,char *context)
{
	if(!dst || !key){
		return 0;
	}
	char *p = strstr(dst,key);
	int i = 0,j = 0;
	int find_step = 0;
	if(p)
	{
		i = strlen(key);
		while(i < strlen(dst) && find_step < 2)
		{
			switch(find_step)
			{
				case 0:
					if( (p[i] >= '0' && p[i] <= '9') || (p[i]=='+' && p[i+1] >= '0' && p[i+1] <= '9'))
					{
						find_step = 1;
						break;
					}
					i++;
					break;
				case 1:
					if( (p[i] >= '0' && p[i] <= '9') || ((p[i]=='+'|| p[i]== '-') && p[i+1] >= '0' && p[i+1] <= '9'))
					{
						context[j] = p[i];
						i++;
						j++;
					}
					else{
						find_step = 2;
					}
					break;
				case 2:
				default:
					break;
			}
		}
	}
	return j;
}

/**************************************************************************** 
* 函数名称 : SMSPhoneNumStringMatch
* 功能描述 : 短信匹配余额函数
* 参    数 : char *dst: 短信内容
			char *key:匹配关键字
			char *context:匹配内容
			char *point_char 小数点符号
			char *thousandth_char	千分位符号
* 返 回 值 : 0 匹配失败 >0匹配成功
* 作    者 : liyezhen
* 设计日期 : 2018-7-12
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int SMSBanlanceStringMatch(char *dst,char *key,char *context,char point_char,char thousandth_char)
{
	if(!dst || !key){
		return 0;
	}
	char *p = strstr(dst,key);
	int buf_length = strlen(dst);
	int i = 0,j = 0;
	int point_index = 0;
	char point_flag = 0;
	char number_flag = 0;
	if(p)
	{
		for(i = strlen(key);i<buf_length;i++)
		{
			if(p[i] >= '0' && p[i] <='9'){
				context[j++] = p[i];
				number_flag++;
			}else if(thousandth_char == p[i]){
				if(number_flag)
				{
					if((p[i + 1] < '0' || p[i + 1] >'9') || (p[i + 3] < '0' || p[i + 3] > '9')){
						break;
					}
				}
			}else if(point_char == p[i]){
				if(number_flag)
				{
					if( 0 == point_flag){
						point_flag++;
						context[j++] = p[i];
						point_index = j;
					}else{
						break;
					}
				}
			}else{
				if(number_flag){
					break;
				}
			}
		};
	}
	context[j] = '\0';
	return j > point_index ? j:0;
}

/****************************************************************************************
* 函数名称 : getChannelCallCounts
* 功能描述 : 获取通道的挂断的呼叫次数函数
* 参    数 : char *outbound_buff: gsm show statistics outbound命令的返回内容
			int length:buff长度
			int channel:通道号
* 返 回 值 : 呼叫挂断次数
* 作    者 : liyezhen
* 设计日期 : 2018-7-13
* 修改日期		  修改人		   修改内容  

Span | Count | All Duration | Answerd | Cancel | Busy | No Answer | No Dialtone | No Carrier
1    | 3     | 4            | 0       | 2      | 0    | 0         | 0           | 0        
2    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        
3    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        

4    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        
5    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        

6    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        

7    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        
8    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        

9    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        
10   | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        

11   | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        
12   | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        

13   | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        

14   | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        
15   | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0        

16   | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0 
********************************************************************************************/
int getChannelCallCounts(char *outbound_buff,int length,int channel)
{
	int i = 0;
	int split_char = 0;
	int find_step = 0;
	for(;i < length;i++)
	{
		switch(outbound_buff[i])
		{
			case '\n':
				if(channel == atoi(outbound_buff + i + 1)){
					find_step = 1;
				}
				break;
			case '|':
				if(find_step){
					split_char++;
					/*当呼叫建立后，Count会立即增加，如果是通过呼叫查询，会查询失败*/
					if(1 == split_char){
						return atoi(outbound_buff + i + 1);
					}
				}
				break;
			default:
				break;
		}
	}
	return 0;
}

static int ExecRedisCommandAppendArgv(redisContext *context, const char *cmd, const char *key, const char *filed, const char *value){
	int argc = 0;
	int i = 0;
	int cnt = 0;
	int ret = 0;
	char **argv;
	size_t *argvlen;
	if(cmd)
		argc++;
	if(key)
		argc++;
	if(filed)
		argc++;
	if(value)
		argc++;

	argvlen = (size_t*)malloc(argc*sizeof(size_t));

	argv = (char **)malloc(argc * sizeof(char *));

	if(cmd){
		argvlen[cnt] = strlen(cmd);
		argv[cnt] = strdup(cmd);
		cnt++;
	}

	if(key){
		argvlen[cnt] = strlen(key);
		argv[cnt] = strdup(key);
		cnt++;
	}

	if(filed){
		argvlen[cnt] = strlen(filed);
		argv[cnt] = strdup(filed);
		cnt++;
	}

	if(value){
		argvlen[cnt] = strlen(value);
		argv[cnt] = strdup(value);
		cnt++;
	}

	redisReply *reply;
	redisAppendCommandArgv(context,argc, (const char **)argv, argvlen);
	redisGetReply(context,(void *)&reply);

	if (reply->type == REDIS_REPLY_ERROR)
	{
		printf("[ERRO]ExecRedisCommand:error(%s)", reply->str);
	}
	else
	{
		if (reply->str != NULL)
		{
			ret = 0;
		}else{
			ret = 1;
		}
	}

	freeReplyObject((void *)reply);

	for(i = 0; i < argc; i++){
		free(argv[i]);
	}

	free(argv);

	free(argvlen);

	return ret;
}

/**************************************************************************** 
* 函数名称 : ExecRedisCommand
* 功能描述 : 执行redis命令
* 参    数 : redisContext *context: redis连接结构体
			char *rds_cmd:redis运行命令
			char *buff:redis返回内容
* 返 回 值 : -1 redis命令执行失败  1 命令内容为空或者执行成功但是返回内容为空 0 执行成功后并且获取到返回内容
* 作    者 : liyezhen
* 设计日期 : 2018-7-10
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int ExecRedisCommand(redisContext *context,char *rds_cmd,char *buff)
{
	int ret = -1;

	if(0 == strlen(rds_cmd)){
		return 1;
	}
	
	redisReply *reply = (redisReply *)redisCommand(context, rds_cmd);
	if (reply->type == REDIS_REPLY_ERROR)
	{
		debug(ERROR,"[ERRO]ExecRedisCommand:%s error(%s)", rds_cmd, reply->str);
	}
	else
	{
		if (reply->str != NULL)
		{
			ret = 0;
			strcpy(buff,reply->str);
			//debug(DEBUG,"ExecRedisCommand:%s=%s",rds_cmd,reply->str);
		}else{
			ret = 1;
			//debug(ERROR,"ExecRedisCommand:%s return NULL", rds_cmd);
		}
	}
	freeReplyObject((void *)reply);
	return ret;
}

/**************************************************************************** 
* 函数名称 : InitRedisAllofChannelSMSInfo
* 功能描述 : 初始化redis短信请求字段
* 参    数 : int chan_no:通道号，从1开始
* 返 回 值 : void
* 作    者 : liyezhen
* 设计日期 : 2018-7-10
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void InitRedisAllofChannelSMSInfo(redisContext *context,char *rds_key,int chan_no)
{
	char rds_cmd[6][256];
	int i = 0;
	sprintf(rds_cmd[0],"hset %s %d-send-stat %d",rds_key,chan_no,SMS_IDLE);
	sprintf(rds_cmd[1],"hdel %s %d-recv-num",rds_key,chan_no);
	sprintf(rds_cmd[2],"hdel %s %d-recv-msg",rds_key,chan_no);
	sprintf(rds_cmd[3],"hdel %s %d-match-str",rds_key,chan_no);
	sprintf(rds_cmd[4],"hdel %s %d-send-num",rds_key,chan_no);
	sprintf(rds_cmd[5],"hdel %s %d-send-msg",rds_key,chan_no);
	
	for(i = 0; i < 6;i++){
		ExecRedisCommand(context,rds_cmd[i],rds_cmd[0]);
	}
}

int getChannelSMSReceiveFromRedis(redisContext *context,char *rds_key,int chan_no,char *msg)
{
	if(!msg){
		return -1;
	}
	int ret = -1;
	char rds_cmd[256] = {0};
	sprintf(rds_cmd,"hget %s %d-recv-msg",rds_key,chan_no );
	ret = ExecRedisCommand(context,rds_cmd,msg);
	return ret;
}

/**************************************************************************** 
* 函数名称 : setSMSRecvParamtoRedis
* 功能描述 : 设置Redis短信接收参数
* 参    数 : QUERY_INFO_T *param:通道号，从1开始
* 返 回 值 : void
* 作    者 : liyezhen
* 设计日期 : 2018-7-10
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int setSMSRecvParamtoRedis(redisContext *context,unsigned short chan_no,struct SMS_PARAM_S *p,char *rds_key)
{
	char field[3][32];
	char value[3][128];
	char rds_cmd[128];
	int i;
	sprintf(field[0],"%d-recv-num",chan_no);
	sprintf(field[1],"%d-send-stat",chan_no);
	sprintf(field[2],"%d-match-str",chan_no);
	strncpy(value[0],p->recv_num, 128);
	sprintf(value[1],"%d",SMS_SENT);
	strncpy(value[2],p->match_key,128);
/*	
	sprintf(rds_cmd[0],"hset %s %d-recv-num %s",rds_key,chan_no,p->recv_num);
	sprintf(rds_cmd[1],"hset %s %d-send-stat %d",rds_key,chan_no,SMS_SENT);
	sprintf(rds_cmd[2],"hset %s %d-match-str %s",rds_key,chan_no,p->match_key);
*/
	sprintf(rds_cmd,"hdel %s %d-recv-msg",rds_key,chan_no);

	for(i = 0; i < 3;i++)
	{
		if(-1 == ExecRedisCommandAppendArgv(context,"hset", rds_key, field[i], value[i]))//There will be return an error when match-str is null. 
		{
			InitRedisAllofChannelSMSInfo(context,rds_key,chan_no);
			return RET_ERROR;
		}
	}
	if(-1 == ExecRedisCommand(context,rds_cmd,rds_cmd))//There will be return an error when match-str is null. 
	{
		InitRedisAllofChannelSMSInfo(context,rds_key,chan_no);
		return RET_ERROR;
	}
	return RET_OK;
}

int setSMSQueryParamtoRedis(redisContext *context,unsigned short chan_no,struct SMS_PARAM_S *p,char *rds_key)
{
	char field[3][32];
	char value[3][128];
	int i;
	sprintf(field[0],"%d-send-num",chan_no);
	sprintf(field[1],"%d-send-stat",chan_no);
	sprintf(field[2],"%d-send-msg",chan_no);
	strncpy(value[0],p->dst_num, 128);
	sprintf(value[1],"%d",SMS_NEED_SEND);
	strncpy(value[2],p->send_msg,128);
/*	
	sprintf(rds_cmd[0],"hset %s %d-send-num %s",rds_key,chan_no,p->dst_num);
	sprintf(rds_cmd[1],"hset %s %d-send-stat %d",rds_key,chan_no,SMS_NEED_SEND);
	sprintf(rds_cmd[2],"hset %s %d-send-msg %s",rds_key,chan_no,p->send_msg);
*/
	for(i = 0; i < 3;i++)
	{
		if(-1 == ExecRedisCommandAppendArgv(context, "hset", rds_key, field[i], value[i]))
		{
			InitRedisAllofChannelSMSInfo(context,rds_key,chan_no);
			return RET_ERROR;
		}
	}
	return RET_OK;
}

void *getPhonenumfromRedis(void *pparam)
{
	QUERY_INFO_T *param = (QUERY_INFO_T *)pparam;
	char match_buff[32] = {0};
	char rds_cmd[128] = {0};
	char ast_cmd[128] = {0};
	char buff[1024];
	int timeout = 0;
	int ussd_flag = 0;
	
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err)
	{
		debug(ERROR,"SimQuery: redisConnect error(%s)", context->errstr);
		return NULL;
	}

	if(strlen(param->ussd_res) > 0){
		strcpy(buff, param->ussd_res);
		param->ussd_res[0] = '\0';
		ussd_flag = 1;
	}

	while(timeout < MAX_QUERY_TIME)
	{
		if(ussd_flag == 1 || 0 == getChannelSMSReceiveFromRedis(context,REDIS_KEY_SIMQUERY_PHONENUM_INFO,param->channel,buff))
		{
			if(0 < SMSPhoneNumStringMatch(buff,param->phone_num_param.match_key,match_buff))
			{
				sprintf(rds_cmd,"hset %s %d %s",REDIS_KEY_SIMPHONENUM,param->channel,match_buff);
				ExecRedisCommand(context,rds_cmd,buff);
				debug(DEBUG,"Channle-%d phonenum is %s",param->channel,match_buff);
				sprintf(ast_cmd,"asterisk -rx 'gsm set simnum %d \"%s\"'", param->channel, match_buff);
				system(ast_cmd);
				debug(DEBUG,"%s",ast_cmd);
			}else{
				debug(WARN,"Channle-%d phonenum query error:%s ",param->channel,buff);
				sprintf(rds_cmd,"hset %s %d error",REDIS_KEY_SIMPHONENUM,param->channel);
				ExecRedisCommand(context,rds_cmd,buff);
			}
			
			break;
		}
		timeout += 5;
		sleep(5);
	}

	InitRedisAllofChannelSMSInfo(context,REDIS_KEY_SIMQUERY_PHONENUM_INFO,param->channel);
	redisFree(context);
	
	if(timeout >= MAX_QUERY_TIME){//query failed. try again
		if(param->query_phonenum_counts < MAX_QUERY_NUM){
			param->num_chan_stat = '1';
		}
		debug(WARN,"Channle-%d query phonenum over %d seconds",param->channel,MAX_QUERY_TIME);
	}
	return NULL;
}


void *getBalancefromRedis(void *pparam)
{
	QUERY_INFO_T *param = (QUERY_INFO_T *)pparam;
	char match_buff[32] = {0};
	char rds_cmd[128] = {0};
	char buff[1024];
	int timeout = 0;
	int ussd_flag = 0;

	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err)
	{
		debug(ERROR,"SimQuery: redisConnect error(%s)", context->errstr);
		return NULL;
	}
	if(strlen(param->ussd_res) > 0){
		strcpy(buff, param->ussd_res);
		param->ussd_res[0] = '\0';
		ussd_flag = 1;
	}
	while(timeout < MAX_QUERY_TIME)
	{
		
		if(ussd_flag == 1 || 0 == getChannelSMSReceiveFromRedis(context,REDIS_KEY_SIMQUERY_BALANCE_INFO,param->channel,buff))
		{
			if(0 < SMSBanlanceStringMatch(buff,param->balance_param.match_key,match_buff,param->point_char,param->thousandth_char))
			{
				sprintf(rds_cmd,"hset %s %d %s",REDIS_KEY_SIMBALANCE,param->channel,match_buff);
				ExecRedisCommand(context,rds_cmd,buff);
				debug(DEBUG,"Channle-%d balance is %s",param->channel,match_buff);
			}else{
				sprintf(rds_cmd,"hset %s %d error",REDIS_KEY_SIMBALANCE,param->channel);
				ExecRedisCommand(context,rds_cmd,buff);
				debug(WARN,"Channle-%d phonenum query error:%s ",param->channel,buff);
			}
			break;
		}
		timeout += 5;
		sleep(5);
	}
	if(timeout >= MAX_QUERY_TIME){
		debug(WARN,"Channle-%d query balance over %d seconds",param->channel,MAX_QUERY_TIME);
	}
	
	InitRedisAllofChannelSMSInfo(context,REDIS_KEY_SIMQUERY_BALANCE_INFO,param->channel);
	redisFree(context);
	return NULL;
}

static int ExecSystemCmd(char *cmd, unsigned int len,char *result){
	int ret = 0;
	FILE *fhandle = NULL;
	if(cmd == NULL)
		return -1;
	 fhandle = popen(cmd, "r");
	if(fhandle == NULL)
		return -1;
	ret = fread(result, 1, len, fhandle);
	if(ret < 0){
		pclose(fhandle);
		return -1;
	}
	pclose(fhandle);
	return 0;
}

int SimQuery(QUERY_INFO_T *param,int query_type)
{
	char ast_cmd[128] = {0};
	struct SMS_PARAM_S *p = NULL;
	int query_way = UNKNOWN;
	char rds_key[128] = {0};
	char result[256] = {0};
	int match_flag = 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr,100 * 1024);
	pthread_t recv_sms_tid;

	param->ussd_res[0] = '\0';
	
	if(BALANCE_QUERY == query_type){
		p = &param->balance_param;
		query_way = param->query_type & 0x3;
		strcpy(rds_key,REDIS_KEY_SIMQUERY_BALANCE_INFO);
	}else if(PHONENUM_QUERY == query_type){
		p = &param->phone_num_param;
		query_way = (param->query_type >> 4) & 0x3;
		strcpy(rds_key,REDIS_KEY_SIMQUERY_PHONENUM_INFO);
	}else{
		debug(WARN,"SimQuery:Channle-%d Unknown query tyep:%d",param->channel,query_type);
		return RET_OK;
	}

	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err)
	{
		debug(ERROR,"SimQuery: redisConnect error(%s)", context->errstr);
		return RET_ERROR;
	}
	
	if(RET_OK == setSMSRecvParamtoRedis(context,param->channel,p,rds_key))
	{
		switch(query_way & 0x3)
		{
			case SMS:
				if(RET_ERROR == setSMSQueryParamtoRedis(context,param->channel,p,rds_key)){
					InitRedisAllofChannelSMSInfo(context,rds_key,param->channel);
					redisFree(context);
					return RET_ERROR;
				}
				//sprintf(ast_cmd,"asterisk -rx 'gsm send sync sms %d %s %s 2000 0'",param->channel,p->dst_num,p->send_msg);
				break;
		
			case CALL:
				sprintf(ast_cmd,"asterisk -rx 'gsm check phone stat %d %s 1 2000'",param->channel,p->dst_num);
				break;
		
			case USSD:
				sprintf(ast_cmd,"asterisk -rx 'gsm send ussd %d \"%s\" 2000'",param->channel,p->send_msg);
				if(ExecSystemCmd(ast_cmd, 256, result) == 0){
					if(strstr(result, USSD_TEXT_STRING)){
						result[strlen(result) - 1] = '\0';//delete the last '\n'
						strcpy(param->ussd_res, strstr(result, USSD_TEXT_STRING)+strlen(USSD_TEXT_STRING));
					}
					if(BALANCE_QUERY == query_type){
						if(strstr(param->ussd_res, param->balance_param.match_key) == NULL)
							param->ussd_res[0] = '\0';
					}else{
						if(strstr(param->ussd_res, param->phone_num_param.match_key) == NULL)
							param->ussd_res[0] = '\0';
					}
				}
				ast_cmd[0]= '\0';
				break;
			default:
				debug(ERROR,"SimQuery:Unknown query way:%d!!!",query_way);
				InitRedisAllofChannelSMSInfo(context,rds_key,param->channel);
				redisFree(context);
				return RET_ERROR;
				break;
		}
		if(strlen(ast_cmd) > 0)
		{	
			debug(DEBUG,"%s",ast_cmd);
			system(ast_cmd);
		}
		
		if(BALANCE_QUERY == query_type){
			pthread_create(&recv_sms_tid,&attr,getBalancefromRedis,(void *)param);
		}else{
			pthread_create(&recv_sms_tid,&attr,getPhonenumfromRedis,(void *)param);
		}
		pthread_detach(recv_sms_tid);
	}
	redisFree(context);
	return RET_OK;
}

void *SimQueryTask(void *param)
{
	QUERY_INFO_T *p = (QUERY_INFO_T *)param;

	while(1)
	{
		sem_wait(&p->sem);
 
		if('0' == p->bal_chan_stat && ON == p->query_balance_flag)
		{
			debug(DEBUG,"SimQueryTask:Channel-%d query balance",p->channel);
			SimQuery(p,p->query_type & BALANCE_QUERY);
			pthread_mutex_lock(&p->flag_mtx);
			p->query_balance_flag = OFF;
			pthread_mutex_unlock(&p->flag_mtx);			
		}else if('0' == p->num_chan_stat && ON == p->query_phonenum_flag)
		{
			debug(DEBUG,"SimQueryTask:Channel-%d query phonenum",p->channel);
			if(RET_OK == SimQuery(p,p->query_type & PHONENUM_QUERY))
			{
				/*号码查询只在注册成功后查询一次，重新运行进程才会再次查询*/
				pthread_mutex_lock(&p->flag_mtx);
				p->query_phonenum_flag = OFF;
				p->query_phonenum_counts++ ;
				pthread_mutex_unlock(&p->flag_mtx);
			}
		}
		debug(DEBUG,"Channel-%d query type is %d",p->channel,p->query_type);
	}
	return NULL;
}

void RegisteredsetQueryFlag(QUERY_INFO_T *param,char *flag)
{
	pthread_mutex_lock(&param->flag_mtx);
	*flag = ON;
	pthread_mutex_unlock(&param->flag_mtx);
	sem_post(&param->sem);
	debug(NOTICE,"Channle-%d %s",param->channel,__FUNCTION__);
}

void CallCountsetQueryFlag(QUERY_INFO_T *param,char *flag)
{
	pthread_mutex_lock(&param->flag_mtx);
	*flag = ON;
	pthread_mutex_unlock(&param->flag_mtx);
	sem_post(&param->sem);
	debug(NOTICE,"Channle-%d %s",param->channel,__FUNCTION__);
}

void  *IntervalsetQueryFlagTask(void *param)
{
	QUERY_INFO_T *p = (QUERY_INFO_T *)param;
	while(1)
	{
		if(p->interval > 0)
		{
			pthread_mutex_lock(&p->flag_mtx);
			p->query_balance_flag = ON;
			pthread_mutex_unlock(&p->flag_mtx);
			sem_post(&p->sem);
			debug(NOTICE,"Channle-%d %s query after %d seconds",p->channel,__FUNCTION__,p->interval);
			sleep(p->interval);
		}else{
			sleep(MAX_SLEEP_TIME);
		}
		
	}
}

void *StatScanTask(QUERY_INFO_T *head)
{
	QUERY_INFO_T *p = NULL;
	struct stat buf;
	stat(STAT_CONF_FILE, &buf);
	time_t time_file_modify = buf.st_mtime;
	char rds_cmd[128] = {0};
	sprintf(rds_cmd,"hget %s ",REDIS_KEY_SIMSTATUS);
	int rds_length = strlen(rds_cmd);
	char ast_cmd[128];
	sprintf(ast_cmd,"asterisk -rx 'gsm show statistics outbound'");
	FILE *stream = NULL;
	int len = 0;
	char buff[8192];
	char chan_stat[2] = {0}; 
	int call_counts = 0;
	
	redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
	if (context->err)
	{
		debug(ERROR,"[ERRO]StatScanTask:redisConnect error(%s)",context->errstr);
		return NULL;
	}

	while(1)
	{
		if (stat(STAT_CONF_FILE, &buf) == -1){
			debug(ERROR,"stat %s error,",STAT_CONF_FILE);
		}else{
			if(buf.st_mtime != time_file_modify)
			{
				debug(NOTICE,"%s has been changed,need to reload query param",STAT_CONF_FILE);
				conf_reload((QUERY_INFO_T *)head);
				time_file_modify = buf.st_mtime;
			}
		}
		if(0 == stat(LOG_FILE,&buf))
		{
			if(buf.st_size > 100 * 1024)		//100k
			{
				system("echo '' > /tmp/log/sim_query.log");
				debug(NOTICE,"clean log file %s",LOG_FILE);
			}
		}
			
		stream = popen(ast_cmd, "r");
		len = fread(buff,1,sizeof(buff)-1,stream);
		pclose(stream);
		p = head->next;
		while(p)
		{
			if(len > 0 && p->call_counts_query)
			{
				call_counts = getChannelCallCounts(buff,len,p->channel);
				if(call_counts / p->call_counts_query > p->call_query_counts)
				{
					CallCountsetQueryFlag(p,&p->query_balance_flag);
					p->call_query_counts = call_counts / p->call_counts_query;
				}
			}

			if(p->query_type > 0)
			{
				sprintf(rds_cmd + rds_length,"%d",p->channel);
				chan_stat[0] = '1';
				ExecRedisCommand(context,rds_cmd,chan_stat);

				if('0' == chan_stat[0])
				{
					if(p->query_type & BALANCE_QUERY && chan_stat[0] != p->bal_chan_stat)
					{
						p->bal_chan_stat = '0';						
						if(p->registered_query & ON){
							RegisteredsetQueryFlag(p,&p->query_balance_flag);
						}
					}else if(p->query_type & PHONENUM_QUERY && chan_stat[0] != p->num_chan_stat)
					{
						p->num_chan_stat = '0';
						RegisteredsetQueryFlag(p,&p->query_phonenum_flag);
					}
				}
				else
				{
					if(p->bal_chan_stat == '0')
					{
						p->bal_chan_stat = '1';
						debug(NOTICE,"Channel-%d del balance info",p->channel);
					}
					if(p->num_chan_stat == '0')
					{
						p->query_phonenum_counts = 0;
						p->num_chan_stat = '1';
						debug(NOTICE,"Channel-%d del phone num info",p->channel);
					}
				}
			}

			p = p->next;
		}

		sleep(2);
	}
	
	redisFree(context);
	return NULL;
}

int main(int argc,char **argv)
{
	if(argc > 2){
		match_test(argc,argv);
		return 0;
	}else{
		g_log_class = atoi(argv[1]);
	}
	
	if(checkProcessExist(LOCK_FILE)){
		return -1;
	}
	open_log_file(LOG_FILE);
	
	pthread_t sim_query_pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr,100 * 1024);
	QUERY_INFO_T param;
	memset(&param,0,sizeof(QUERY_INFO_T));

	init_conf(&param);
	conf_reload(&param);
	QUERY_INFO_T *p = param.next;
	while(p)
	{
		if(0 != pthread_create(&sim_query_pid,&attr,SimQueryTask,(void *)p)){
			debug(ERROR,"[%d]Create SimQueryTask thread fail:%s",p->channel,strerror(errno));
			exit(-1);
		}

		if(0 != pthread_create(&sim_query_pid,&attr,IntervalsetQueryFlagTask,(void *)p)){
			debug(ERROR,"[%d]Create IntervalsetQueryFlag thread fail:%s",p->channel,strerror(errno));
			exit(-1);
		}
		p = p->next;
	}
	
	StatScanTask(&param);
	return 0;
}


