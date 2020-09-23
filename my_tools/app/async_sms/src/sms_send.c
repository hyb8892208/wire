#include "../include/sms_inc.h"
#include "../include/utf8_simple.h"
#include "log_debug.h"
#define	max_sms_cmd_len	2048
#define	max_sms_res_len		256
#define max_sms_len			210
#define max_short_sms_ascii_len   160
#define max_long_sms_ascii_len	  140
#define max_sms_utf8_len	200
#define	sms_sendto_ast_cmd		"asterisk -rx \"gsm send sync sms %d \\\"%s\\\" \\\"%s\\\" %d %d \\\"%s\\\"\""
#define	csms_sendto_ast_cmd		"asterisk -rx \"gsm send sync csms %d \\\"%s\\\" \\\"%s\\\" %d %d %d %d %d \\\"%s\\\"\""
#define RESEND_COUNT 10

static int my_exec(const char *cmd, char *res)
{
	FILE *stream = NULL;

	stream = popen(cmd, "r");
	if(stream == NULL) {
		printf("popen %s failed. \n", cmd);
		return -1;
	}
	fread(res, 1, max_sms_res_len-1, stream);
	pclose(stream);

	return 0;
}

int send_sms(int chan_id, char *msg, char *num, int flashsms, char *id)
{   
    char sms_cmd[max_sms_cmd_len];
    char res_buf[max_sms_res_len];
    int sms_timeout = 90;
    int count = RESEND_COUNT;
    int ret = 0, i;
    
    if (msg == NULL || num == NULL || id == NULL)
        return 0;
    snprintf(sms_cmd, sizeof(sms_cmd), sms_sendto_ast_cmd, chan_id, num, msg, sms_timeout, flashsms, id);
//    printf("[%d] goto asterisk [%s].\n", chan_id, sms_cmd);
    for (i = 0; i < count; i++) {
		memset(res_buf, 0, max_sms_res_len);
        ret = my_exec((const char *)sms_cmd, res_buf);
        if (ret == 0) {
		log_print(INFO,"[%d] res=[%s].\n", chan_id, res_buf);
            if (strstr(res_buf, "SUCCESS") != NULL) {
                //finish send sync sms.
//              if(strstr(res_buf, "SUCCESS") != NULL)
                ret = 1;
//                printf("[%d] send sms to asterisk %s, phone=[%s], msg=[%s] \n", chan_id, ret?"succ":"fail", num, msg);
                return ret;
            } else if (strstr(res_buf, "WAS USING") != NULL) {  //  in the was using , we should need send msg again.
				continue;
		}else if(strstr(res_buf, "WAS FAILED") != NULL) {
			break;
		}else {
			return -1;
			//break;
		}
        }else{
		log_print(DEBUG, "[%d]ret = %d\n",chan_id,ret);
	}
        sms_timeout += 10;
        snprintf(sms_cmd, sizeof(sms_cmd), sms_sendto_ast_cmd, chan_id, num, msg, sms_timeout, flashsms, id);
		sleep(1);
//      printf("[%d] %d goto asterisk [%s].\n", chan_id, i, sms_cmd);
    }
//    printf("[%d] send sms to asterisk failed, phone=[%s], msg=[%s] \n", chan_id, num, msg);
//  sleep(1);
    
    return 0;
}


//³¤¶ÌÐÅ·¢ËÍ
int send_csms(int chan_id, char *msg, char *num, int flashsms, char *id,
	unsigned int flag,int smscount,int smssequence)
{	
	char sms_cmd[max_sms_cmd_len];
	char res_buf[max_sms_res_len];
	int sms_timeout = 90;
	int count = RESEND_COUNT;
	int ret = 0, i;
	
	if (msg == NULL || num == NULL || id == NULL|| smscount <1)
		return 0;

	snprintf(sms_cmd, sizeof(sms_cmd), csms_sendto_ast_cmd, chan_id, num, msg,flag,smscount,smssequence, sms_timeout, flashsms, id); 	
//	printf("[%d] goto asterisk [%s].\n", chan_id, sms_cmd);
	for (i = 0; i < count; i++) {
		res_buf[0] = '\0';
		ret = my_exec((const char *)sms_cmd, res_buf);
		if (ret == 0) {
			printf("[%d] res=[%s].\n", chan_id, res_buf);
			if (strstr(res_buf, "SUCCESS") != NULL) {
				//finish send sync sms.
//				if(strstr(res_buf, "SUCCESS") != NULL)
				ret = 1;
//				printf("[%d] send sms to asterisk %s, phone=[%s], msg=[%s] \n", chan_id, ret?"succ":"fail", num, msg);
				return ret;
			} else if (strstr(res_buf, "WAS USING") != NULL) {	//  in the was using , we should need send msg again.
				continue;
			} else if(strstr(res_buf, "WAS FAILED") != NULL){
				break;
			}else {
				return -smscount;
//				break;
			}
		}
		sms_timeout += 10;
 		snprintf(sms_cmd, sizeof(sms_cmd), csms_sendto_ast_cmd, chan_id, num, msg,flag,smscount,smssequence, sms_timeout, flashsms, id);
		sleep(1);
 //		printf("[%d] %d goto asterisk [%s].\n", chan_id, i, sms_cmd);
	}
//	printf("[%d] send sms to asterisk failed, phone=[%s], msg=[%s] \n", chan_id, num, msg);
//	sleep(1);
	
	return 0;
}

static int is_china_cdma(int chan_id ){
	char cmd[256] ;
	char result[256]= {0};
	snprintf(cmd, sizeof(cmd),"cat /tmp/gsm/%d|grep \"Network Name\" | awk -F: '{print $2}'", chan_id);
	if(my_exec(cmd, result) == 0){
		if(strstr(result, "CHINA TELECOM") || strstr(result, "CHN-CT"))
			return 0;
	}
	return 1;
}

int splite_sms(int chan_id, char *msg, char **sms)
{
	int ret = 0,i = 0;
	int msg_idx = 0,sms_idx = 0,max_sms_text_len;
	int sms_len = 0,sms_add_step = 0;
	if (msg == NULL)
		return 0;
	if(is_china_cdma(chan_id) == 0){
		max_sms_text_len = max_long_sms_ascii_len/2;
		if(get_utf8_length((unsigned char *)msg,strlen(msg)) >max_sms_text_len){
			max_sms_text_len -= 3;
		}
		sms_add_step = I;
	}else{
		if(strlen(msg) ==  get_utf8_length((unsigned char *)msg,strlen(msg))){
			sms_add_step = I;
			if(strlen(msg) <= max_short_sms_ascii_len)
				max_sms_text_len = max_short_sms_ascii_len + sms_add_step;
			else
				max_sms_text_len = max_long_sms_ascii_len;
		}else{
			sms_add_step = III;
			max_sms_text_len = max_sms_utf8_len;
		}
	}
	
	do
	{
		ret = get_char_utf_len((unsigned char)msg[msg_idx]);
		msg_idx += ret;
		sms_idx += ret;
		sms_len += sms_add_step;
		if(sms_len >= max_sms_text_len || '\0' == msg[msg_idx])
		{
			strncpy(sms[i],msg+msg_idx-sms_idx,sms_idx);
			sms[i][sms_idx] = '\0';
			sms_idx = 0;
			i++;
			sms_len = 0;
		}
	}while('\0' != msg[msg_idx]);

	return i;
}

