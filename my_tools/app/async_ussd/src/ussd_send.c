#include "../include/ussd_inc.h"

#define max_ussd_cmd_len	256
#define max_ussd_res_len		2048
#define ussd_sendto_ast_cmd		"asterisk -rx \"gsm send ussd %d \\\"%s\\\" %d \\\"%s\\\"\""
//#define ussd_sendto_ast_cmd		"asterisk -rx \"gsm send ussd %d \\\"%s\\\" %d \""

static int my_exec(const char *cmd, char *res)
{
	FILE *stream = NULL;

	stream = popen(cmd, "r");
	if(stream == NULL) {
		printf("popen %s failed. \n", cmd);
		return -1;
	}
	fread(res, 1, max_ussd_res_len-1, stream);
	pclose(stream);

	return 0;
}

int send_ussd(int chan_id, char *msg, char *timeout, char *uuid, char *result)
{   
	char ussd_cmd[max_ussd_cmd_len];
	char res_buf[max_ussd_res_len];
	int ussd_timeout = 10000;	// 10s
	int count = 10;
	int ret = 0, i;

//	printf("enter send_ussd\n");
	if (msg == NULL)
		return 0;
	if (timeout != NULL && strlen(timeout) > 0) {
		ussd_timeout = atoi(timeout);
	}
	snprintf(ussd_cmd, sizeof(ussd_cmd), ussd_sendto_ast_cmd, chan_id, msg, ussd_timeout, (uuid?uuid:""));
//	snprintf(ussd_cmd, sizeof(ussd_cmd), ussd_sendto_ast_cmd, chan_id, msg, ussd_timeout);
//	printf("[%d] goto asterisk [%s].\n", chan_id, ussd_cmd);
	for (i = 0; i < count; i++) {
		memset(res_buf, 0, max_ussd_res_len);
		ret = my_exec((const char *)ussd_cmd, res_buf);
		if (ret == 0) {
//			printf("[%d] res=[%s].\n", chan_id, res_buf);
			if (strstr(res_buf, "responses") != NULL) {
				//finish send ussd.
				ret = 1;
				if (result != NULL)
					strcpy(result, res_buf);
				printf("[%d] send ussd to asterisk success, msg=[%s] \n", chan_id, msg);
				return ret;
			//} else if (strstr(res_buf, "failed") != NULL || strstr(res_buf, "timeout") != NULL) {
			} else if (strstr(res_buf, "Sending USSD now") != NULL) {
				continue;
			} else {
				break;
			}
		}
		ussd_timeout += 10;
		snprintf(ussd_cmd, sizeof(ussd_cmd), ussd_sendto_ast_cmd, chan_id, msg, ussd_timeout, (uuid?uuid:""));
//		snprintf(ussd_cmd, sizeof(ussd_cmd), ussd_sendto_ast_cmd, chan_id, msg, ussd_timeout);
		sleep(1);
		//printf("[%d] %d goto asterisk [%s].\n", chan_id, i, ussd_cmd);
	}
	printf("[%d] send ussd to asterisk failed, msg=[%s] \n", chan_id, msg);

	return 0;
}

