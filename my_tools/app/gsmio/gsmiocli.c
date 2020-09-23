/*
	gsmiocli.c
*/

#include "gsmio.h"

#include "soap_gsmio.nsmap"
#include "soapH.h"
#include <ctype.h>


//---------------------------------------------



//---------------------------------------------



////////////////////////////////////////////////////////////////////////////////////////////




#define CLI_MAX_LEN (256)
unsigned char g_input_buf[CLI_MAX_LEN];
unsigned int g_input_idx = 0;
void print_usage(void)
{
	printf("Usage:\n");
	printf("gsmiocli command chn value\n");
	printf("command:\n");
	printf("    1:  set simcard src\n");
	printf("    2:  get simcard src status\n");
	printf("    3:  set pwr on\n");
	printf("    4:  set pwr off\n");
	printf("    5:  get pwr status\n");
	printf("    6:  set pwrkey on\n");
	printf("    7:  set pwrkey off\n");
	printf("    8:  set emerg off\n");
	printf("    9:  get pwrkey status\n");
	printf("    10: get simcard insert status src\n");
	printf("    11: get module insert status\n");
	printf("    12: get mcu version\n");
	printf("chn:\n");
	printf("    -1: all channels\n");
	printf("    0~max channel: \n");
	printf("value: avalid when command is set\n");
	printf("\n");
}

int is_digit(char *str)
{
	int i = 0;
	for (i = 0; i < strlen(str); i++)
		if (!isdigit(str[i]))
			return 0;
	return 1;
}
int check_argv(int argc, char **argv)
{
	if (argc == 2 && is_digit(argv[1]) && atoi(argv[1]) == 12)
		return 0;
	if (argc == 3 && (atoi(argv[1]) > 1 && atoi(argv[1]) < 12) && (is_digit(argv[2]) || strcmp(argv[2], "-1") == 0))
		return 0;
	if (argc == 4 && (atoi(argv[1]) > 0 && atoi(argv[1]) < 3) && (is_digit(argv[2]) || strcmp(argv[2], "-1") == 0) && is_digit(argv[3]) && (atoi(argv[3]) == 0 || atoi(argv[3]) == 1))
		return 0;
	return -1;
}
int main(int argc, char **argv)
{
	int ret = 0;
	gsmio_handle_t *hdl = NULL;
	char buff[1024] = {0};
	if (check_argv(argc, argv) != 0)
	{
		print_usage();
		return -1;
	}
	hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		printf("gsmio_open return NULL\n");
		return -1;
	}
	
	switch (atoi(argv[1]))
	{
		case 1:
			ret = gsmio_set_simcard_src(hdl, atoi(argv[2]), atoi(argv[3]));
			if (ret == HDL_OK)
				printf("set simcard source(chn:%d, src:%d) succ\n", atoi(argv[2]), atoi(argv[3]));
			else
				printf("set simcard source(chn:%d, src:%d) err(return=%d)\n", atoi(argv[2]), atoi(argv[3]), ret);
			break;
		case 2:
			ret = gsmio_get_simcard_src(hdl, atoi(argv[2]), buff);
			if (ret == HDL_OK)
				printf("get simcard source(chn:%d) succ(status=%s)\n", atoi(argv[2]), buff);
			else
				printf("get simcard source(chn:%d) err(return=%d, status=%s)\n", atoi(argv[2]), ret, buff);
			break;
		case 3:
			ret = gsmio_pwr_on(hdl, atoi(argv[2]));
			if (ret == HDL_OK)
				printf("pwr on(chn:%d) succ\n", atoi(argv[2]));
			else
				printf("pwr on(chn:%d) err(return=%d)\n", atoi(argv[2]), ret);
			break;
		case 4:
			ret = gsmio_pwr_off(hdl, atoi(argv[2]));
			if (ret == HDL_OK)
				printf("pwr off(chn:%d) succ\n", atoi(argv[2]));
			else
				printf("pwr off(chn:%d) err(return=%d)\n", atoi(argv[2]), ret);
			break;
		case 5:
			ret = gsmio_get_pwr_status(hdl, atoi(argv[2]), buff);
			if (ret == HDL_OK)
				printf("get pwr status(chn:%d) succ(status=%s)\n", atoi(argv[2]), buff);
			else
				printf("get pwr status(chn:%d) err(return=%d, status=%s)\n", atoi(argv[2]), ret, buff);
			break;
		case 6:
			ret = gsmio_pwrkey_on(hdl, atoi(argv[2]));
			if (ret == HDL_OK)
				printf("pwrkey on(chn:%d) succ\n", atoi(argv[2]));
			else
				printf("pwrkey on(chn:%d) err(return=%d)\n", atoi(argv[2]), ret);
			break;
		case 7:
			ret = gsmio_pwrkey_off(hdl, atoi(argv[2]));
			if (ret == HDL_OK)
				printf("pwrkey off(chn:%d) succ\n", atoi(argv[2]));
			else
				printf("pwrkey off(chn:%d) err(return=%d)\n", atoi(argv[2]), ret);
			break;
		case 8:
			ret = gsmio_emerg_off(hdl, atoi(argv[2]));
			if (ret == HDL_OK)
				printf("emerg off(chn:%d) succ\n", atoi(argv[2]));
			else
				printf("emerg off(chn:%d) err(return=%d)\n", atoi(argv[2]), ret);
			break;
		case 9:
			ret = gsmio_get_pwrkey_status(hdl, atoi(argv[2]), buff);
			if (ret == HDL_OK)
				printf("get pwrkey status(chn:%d) succ(status=%s)\n", atoi(argv[2]), buff);
			else
				printf("get pwrkey status(chn:%d) err(return=%d, status=%s)\n", atoi(argv[2]), ret, buff);
			break;
		case 10:
			ret = gsmio_get_simcard_insert_status(hdl, atoi(argv[2]), buff);
			if (ret == HDL_OK)
				printf("get simcard insert status(chn:%d) succ(status=%s)\n", atoi(argv[2]), buff);
			else
				printf("get simcard insert status(chn:%d) err(return=%d, status=%s)\n", atoi(argv[2]), ret, buff);
			break;
		case 11:
			ret = gsmio_get_gsmboard_insert_status(hdl, atoi(argv[2]), buff);
			if (ret == HDL_OK)
				printf("get gsmboard insert status(chn:%d) succ(status=%s)\n", atoi(argv[2]), buff);
			else
				printf("get gsmboard insert status(chn:%d) err(return=%d, status=%s)\n", atoi(argv[2]), ret, buff);
			break;
		case 12:
			ret = gsmio_get_mcu_version(hdl, buff);
			if (ret == HDL_OK)
				printf("%s\n", buff);
			else
				printf("get mcu version err(rettun=%d)\n%s)\n", ret, buff);
			break;
		default:
			printf("command(%d) unsupport\n", atoi(argv[1]));
			break;
	}

	gsmio_close(hdl);
	return 0;
}

