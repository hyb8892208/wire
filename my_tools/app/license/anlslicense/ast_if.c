#include "inc.h"

#define MAX_AST_CMD_LEN		2048
#define MAX_AST_RES_LEN		256
#define max_sms_len			210
#define max_sms_ascii_len	140
#define max_sms_utf8_len	200
#define AT_CMD_GET_TIME		"AT+QLTS=2"
#define AST_AT_CMD				"asterisk -rx \"gsm send sync at %d \\\"%s\\\" %d\""
#define AST_CHECK_AT_RES		"+QLTS:"
#define AST_STOP_CMD			"/etc/init.d/asterisk stop &"
#define AST_KILL_CMD			"killall asterisk"
#define AST_START_CMD			"/etc/init.d/asterisk start &"
#define AST_STARTG_CMD		"asterisk -g"
#define AST_PIDOF_CMD			"pidof asterisk"


static int my_exec(const char *cmd, char *result)
{
	int res = 0;
	FILE *stream = NULL;

	if (cmd == NULL)
		return -1;
	stream = popen(cmd, "r");
	if(stream == NULL) {
		printf("popen %s failed. \n", cmd);
		return -1;
	}
	if (result != NULL) {
		res = fread(result, 1, MAX_AST_RES_LEN-1, stream);
		if (res > 0)	// read success
			res = 0;
//		printf("goto fread, res=%d, str[%s]\n", res, result);
	}
	pclose(stream);

	return res;
}

/*
LTE:
AT+QLTS=2
+QLTS: "2019/02/26,17:16:24+32,0"
*/
int ast_send_at(int chan_id, char *nowstr)
{   
	char ast_cmd[MAX_AST_CMD_LEN];
	char res_buf[MAX_AST_RES_LEN];
	int ast_timeout = 1200;
	int count = 10;
	int ret = -1;
	int i;
	char *ptr;

	if (chan_id <= 0 || nowstr == NULL)
		return -1;
	snprintf(ast_cmd, sizeof(ast_cmd), AST_AT_CMD, chan_id, AT_CMD_GET_TIME, ast_timeout);
//	printf("[%d] send at [%s].\n", chan_id, ast_cmd);
	for (i = 0; i < count; i++) {
		res_buf[0] = '\0';
		ret = my_exec((const char *)ast_cmd, res_buf);
		if (ret == 0) {
//			printf("[%d] res=[%s].\n", chan_id, res_buf);
			if (strstr(res_buf, AST_CHECK_AT_RES) != NULL) {
				//finish send sync at.
				ptr = strstr(res_buf, AST_CHECK_AT_RES) + strlen(AST_CHECK_AT_RES);
				while (*ptr == ' ' || *ptr == '\"')		// delete space and "
					ptr++;
				strncpy(nowstr, ptr, 32);
				printf("[%d] send at to asterisk success \n", chan_id);
				return ret;
			}
		} else {
			break;
		}
		ast_timeout += 500;
		snprintf(ast_cmd, sizeof(ast_cmd), AST_AT_CMD, chan_id, AT_CMD_GET_TIME, ast_timeout);
		sleep(1);
//		printf("[%d] %d goto asterisk [%s].\n", chan_id, i, ast_cmd);
	}
	printf("[%d] send at to asterisk failed \n", chan_id);

	return -1;
}

int ast_is_stop()
{
	int res;
	char val[256];

	val[0] = '\0';
	res= my_exec(AST_PIDOF_CMD, val);
	if (res < 0)
		return 0;
	if (strlen(val) == 0) {	// asterisk is stop
		return 1;
	}

	return 0;
}

void ast_stop()
{
	int res;
	printf("stopping asterisk.\n");

	if (ast_is_stop())
		return;
	res = my_exec(AST_STOP_CMD, NULL);
	if (res)
		my_exec(AST_KILL_CMD, NULL);
	return;
}

void ast_start()
{
	int res;
	printf("starting asterisk.\n");
	res = my_exec(AST_START_CMD, NULL);
	if (res)
		my_exec(AST_STARTG_CMD, NULL);
	return;
}


