#include "mcuhdl.h"
#include "czmq.h"
#include <regex.h>

#define GSOAP_CONN_HOST "127.0.0.1"
#define GSOAP_CONN_PORT 8808

#define ACTION_POWER_ON (1)
#define ACTION_POWER_OFF (0)

void init_module_power_action(int action)
{
	int ret = 0;
	
	zsys_debug("init_module_power_action:");
	
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(GSOAP_CONN_HOST, GSOAP_CONN_PORT);
	if (hdl == NULL)
	{
		zsys_error("gsm_mcu_hdl_init return NULL");
		return;
	}

	if ( action == ACTION_POWER_ON ) {
		ret = gsm_module_power_on(hdl, ALL_CHN);
	} else if ( action == ACTION_POWER_OFF ) {
		ret = gsm_module_power_off(hdl, ALL_CHN);
	} else {
		zsys_error("ERROR: init_module_power_action paramater:action error.");
	}
	
	if (ret != HDL_OK)
		zsys_error("ERROR: init_module_power_action: port(%d) failed", ALL_CHN);
	else
		zsys_debug("DEBUG: init_module_power_action: port(%d) successful", ALL_CHN);

	gsm_mcu_hdl_uinit(hdl);

}

void init_simcard_enable(void)
{
	int ret = 0;
	
	zsys_debug("init_simcard_enable:");
	
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(GSOAP_CONN_HOST, GSOAP_CONN_PORT);
	if (hdl == NULL)
	{
		zsys_error("gsm_mcu_hdl_init return null");
		return;
	}

	// enable all
	ret = gsm_simcard_enable(hdl, ALL_CHN);
	
	if (ret != HDL_OK)
		zsys_error("error: init_simcard_enable: port(%d) failed", ALL_CHN);
	else
		zsys_debug("debug: init_simcard_enable: port(%d) successful", ALL_CHN);

	gsm_mcu_hdl_uinit(hdl);
}

void init_gsm_module_on(void)
{
	int ret = 0;
	
	zsys_debug("init_gsm_module_on:");
	
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(GSOAP_CONN_HOST, GSOAP_CONN_PORT);
	if (hdl == NULL)
	{
		zsys_error("gsm_mcu_hdl_init return NULL");
		return;
	}

	// enable all
	ret = gsm_module_on(hdl, ALL_CHN);
	if (ret != HDL_OK)
		zsys_error("ERROR: init_gsm_module_on: port(%d) failed", ALL_CHN);
	else
		zsys_debug("DEBUG: init_gsm_module_on: port(%d) successful", ALL_CHN);

	gsm_mcu_hdl_uinit(hdl);
}

int substring(const char *src, char *dest, const char *pattern)
{
	char errbuf[1024];
    char match[100];
    int err = 0;
    regex_t reg;
    regmatch_t pmatch[64];
    
    if ( regcomp(&reg, pattern, REG_EXTENDED) < 0) {
        regerror(err, &reg, errbuf, sizeof(errbuf));
        zsys_error("ERROR: substring  regcomp error: %s\n", errbuf);
		return -1;
    }
	
	err = regexec(&reg, src, 2, pmatch, 0);
    if ( err == REG_NOMATCH) {
		zsys_error("ERROR: substring not match pattern");
		return -1;
    } else {
        memset(match,'\0',sizeof(match));
        int len = pmatch[1].rm_eo - pmatch[1].rm_so;
        memcpy(match, src+pmatch[1].rm_so,len);
		memcpy(dest, match, strlen(match));
    }	
	return 0;
}

void init_hw_version(void)
{
	int ret = 0;
	char buf[1024] = {0};
	char hw_ver[1024] = {0};
	const char *pattern = "HwVer : V(.*)\n";
	char *filename = "/version/hardware_version";
	FILE *fd;
	zsys_debug("get hardware verion:");

	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(GSOAP_CONN_HOST, GSOAP_CONN_PORT);
	if (hdl == NULL)
	{
		zsys_error("error: init_hw_version: init hdl return null");
		return;
	}

	// get hardware version
	ret = gsm_get_mcu_version(hdl, buf);
	gsm_mcu_hdl_uinit(hdl);
	if (ret != HDL_OK){
		zsys_error("error: init_hw_version: get hardware version failed");
		return;
	} else {
		zsys_debug("debug: init_hw_version: get hardware version(%s) successful", buf);
	}

	ret = substring(buf, hw_ver, pattern);
	if (ret) {
		zsys_error("error: get hardware version error");
		strcpy(hw_ver, "hardware error");
	}
	// save buf to /version/hardware_version
	if ( NULL == (fd=fopen(filename,"w")) ) {
		zsys_error("error: init_hw_version: open file %s error", filename);
		return;
	}
	fwrite(hw_ver, strlen(hw_ver), 1, fd);
	fclose(fd);
}

void init_gsm(void)
{
	init_module_power_action(ACTION_POWER_OFF);
	init_module_power_action(ACTION_POWER_ON);
	init_simcard_enable();
	init_gsm_module_on();
}

int main(int argc, char **argv)
{
	zsys_set_logident(argv[0]);

	init_gsm();
	init_hw_version();
	zsys_shutdown();
	return 0;
}

