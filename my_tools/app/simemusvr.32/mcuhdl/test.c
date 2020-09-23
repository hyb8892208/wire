#include "mcuhdl.h"
#include "czmq.h"

#define LOCALHOST "127.0.0.1"


void test_simcard_enable(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_simcard_enable: begin=====================================================");
	
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(LOCALHOST, 8808);
	if (hdl == NULL)
	{
		zsys_error("gsm_mcu_hdl_init return NULL");
		return;
	}


	// disable all
zsys_debug("test_simcard_enable: test gsm_simcard_disable(-1) ----------------------------");
	ret = gsm_simcard_disable(hdl, ALL_CHN);
	ret = gsm_get_simcard_status(hdl, ALL_CHN, buf);

	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != '0')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_simcard_enable: gsm_simcard_disable(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_simcard_enable: gsm_simcard_disable(%d) suc(status:[%s])", ALL_CHN, buf);

	
	// enable all
zsys_debug("test_simcard_enable: test gsm_simcard_enable(-1) ----------------------------");
	gsm_simcard_enable(hdl, ALL_CHN);
	ret = gsm_get_simcard_status(hdl, ALL_CHN, buf);
	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != 'f')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_simcard_enable: gsm_simcard_enable(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_simcard_enable: gsm_simcard_enable(%d) suc(status:[%s])", ALL_CHN, buf);

	
	// disable all
zsys_debug("test_simcard_enable: test gsm_simcard_disable(-1) ----------------------------");
	ret = gsm_simcard_disable(hdl, ALL_CHN);
	ret = gsm_get_simcard_status(hdl, ALL_CHN, buf);

	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != '0')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_simcard_enable: gsm_simcard_disable(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_simcard_enable: gsm_simcard_disable(%d) suc(status:[%s])", ALL_CHN, buf);
	

	// enable chn
zsys_debug("test_simcard_enable: test gsm_simcard_enable(0~31) ----------------------------");
	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		gsm_simcard_enable(hdl, i);
		ret = gsm_get_simcard_status(hdl, i, buf);
		if (buf[0] != '1')
			zsys_error("test_simcard_enable: gsm_simcard_enable(%d) err(status:[%s])", i, buf);
		else
			zsys_debug("test_simcard_enable: gsm_simcard_enable(%d) suc(status:[%s])", i, buf);
	}
	ret = gsm_get_simcard_status(hdl, ALL_CHN, buf);
	zsys_debug("test_simcard_enable: gsm_get_simcard_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

	
zsys_debug("test_simcard_enable: test gsm_simcard_disable(0~31) ----------------------------");
	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		gsm_simcard_disable(hdl, i);
		ret = gsm_get_simcard_status(hdl, i, buf);
		if (buf[0] != '0')
			zsys_error("test_simcard_enable: gsm_simcard_disable(%d) err(status:[%s])", i, buf);
		else
			zsys_debug("test_simcard_enable: gsm_simcard_disable(%d) suc(status:[%s])", i, buf);
	}
	ret = gsm_get_simcard_status(hdl, ALL_CHN, buf);
	zsys_debug("test_simcard_enable: gsm_get_simcard_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);


	gsm_simcard_enable(hdl, ALL_CHN);
	zsys_debug("test_simcard_enable: gsm_simcard_enable done, ret = [%d], chn = [%d]", ret, ALL_CHN);
	ret = gsm_get_simcard_status(hdl, ALL_CHN, buf);
	zsys_debug("test_simcard_enable: gsm_get_simcard_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);
	
	gsm_mcu_hdl_uinit(hdl);
}
void test_module_power(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_module_power: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(LOCALHOST, 8808);
	if (hdl == NULL)
	{
		zsys_error("gsm_mcu_hdl_init return NULL");
		return;
	}
	
	
zsys_debug("test_module_power: test gsm_module_power_off(-1) ----------------------------");
	ret = gsm_module_power_off(hdl, ALL_CHN);
	ret = gsm_get_module_power_status(hdl, ALL_CHN, buf);

	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != '0')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_module_power: gsm_module_power_off(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_module_power: gsm_module_power_off(%d) suc(status:[%s])", ALL_CHN, buf);

	
zsys_debug("test_module_power: test gsm_module_power_on(-1) ----------------------------");
	gsm_module_power_on(hdl, ALL_CHN);
	ret = gsm_get_module_power_status(hdl, ALL_CHN, buf);
	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != 'f')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_module_power: gsm_module_power_on(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_module_power: gsm_module_power_on(%d) suc(status:[%s])", ALL_CHN, buf);
	
zsys_debug("test_module_power: test gsm_module_power_off(-1) ----------------------------");
	ret = gsm_module_power_off(hdl, ALL_CHN);
	ret = gsm_get_module_power_status(hdl, ALL_CHN, buf);

	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != '0')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_module_power: gsm_module_power_off(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_module_power: gsm_module_power_off(%d) suc(status:[%s])", ALL_CHN, buf);
	

zsys_debug("test_module_power: test gsm_module_power_on(0~31) ----------------------------");
	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		gsm_module_power_on(hdl, i);
		ret = gsm_get_module_power_status(hdl, i, buf);
		if (buf[0] != '1')
			zsys_error("test_module_power: gsm_module_power_on(%d) err(status:[%s])", i, buf);
		else
			zsys_debug("test_module_power: gsm_module_power_on(%d) suc(status:[%s])", i, buf);
	}
	ret = gsm_get_module_power_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power: gsm_get_module_power_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);



	
zsys_debug("test_module_power: test gsm_module_power_off(0~31) ----------------------------");
	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		gsm_module_power_off(hdl, i);
		ret = gsm_get_module_power_status(hdl, i, buf);
		if (buf[0] != '0')
			zsys_error("test_module_power: gsm_module_power_off(%d) err(status:[%s])", i, buf);
		else
			zsys_debug("test_module_power: gsm_module_power_off(%d) suc(status:[%s])", i, buf);
	}
	ret = gsm_get_module_power_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power: gsm_get_module_power_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);


	gsm_module_power_on(hdl, ALL_CHN);
	zsys_debug("test_module_power: gsm_module_power_on done, ret = [%d], chn = [%d]", ret, ALL_CHN);
	ret = gsm_get_module_power_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power: gsm_get_module_power_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);
	
	gsm_mcu_hdl_uinit(hdl);
}
void test_module_onoff(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_module_onoff: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(LOCALHOST, 8808);
	if (hdl == NULL)
	{
		zsys_error("test_module_onoff: gsm_mcu_hdl_init return NULL");
		return;
	}

	
	
zsys_debug("test_module_onoff: test gsm_module_off(-1) ----------------------------");
	ret = gsm_module_off(hdl, ALL_CHN);
	sleep(10);
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);

	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != '0')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_module_onoff: gsm_module_off(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_module_onoff: gsm_module_off(%d) suc(status:[%s])", ALL_CHN, buf);

	
zsys_debug("test_module_onoff: test gsm_module_on(-1) ----------------------------");
	gsm_module_on(hdl, ALL_CHN);
	sleep(10);
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != 'f')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_module_onoff: gsm_module_on(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_module_onoff: gsm_module_on(%d) suc(status:[%s])", ALL_CHN, buf);
	
zsys_debug("test_module_onoff: test gsm_module_off(-1) ----------------------------");
	ret = gsm_module_off(hdl, ALL_CHN);
	sleep(10);
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);

	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != '0')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_module_onoff: gsm_module_off(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_module_onoff: gsm_module_off(%d) suc(status:[%s])", ALL_CHN, buf);
	

zsys_debug("test_module_onoff: test gsm_module_on(0~31) ----------------------------");
	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		gsm_module_on(hdl, i);
	}
	sleep(5);
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsm_get_module_status(hdl, i, buf);
		if (buf[0] != '1')
			zsys_error("test_module_onoff: gsm_module_on(%d) err(status:[%s])", i, buf);
		else
			zsys_debug("test_module_onoff: gsm_module_on(%d) suc(status:[%s])", i, buf);
	}
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_onoff: gsm_get_module_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);


zsys_debug("test_module_onoff: test gsm_module_off(0~31) ----------------------------");
	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		gsm_module_off(hdl, i);
	}
	sleep(5);
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsm_get_module_status(hdl, i, buf);
		if (buf[0] != '0')
			zsys_error("test_module_onoff: gsm_module_off(%d) err(status:[%s])", i, buf);
		else
			zsys_debug("test_module_onoff: gsm_module_off(%d) suc(status:[%s])", i, buf);
	}
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_onoff: gsm_get_module_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);


	//gsm_module_on(hdl, ALL_CHN);
	//usleep(50*1000);
	//zsys_debug("test_module_onoff: gsm_module_on done, ret = [%d], chn = [%d]", ret, ALL_CHN);
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_onoff: gsm_get_module_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);
	

	gsm_mcu_hdl_uinit(hdl);
}

void test_module_emerg_off(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_module_power: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(LOCALHOST, 8808);
	if (hdl == NULL)
	{
		zsys_error("test_module_emerg_off: gsm_mcu_hdl_init return NULL");
		return;
	}
	
	ret = gsm_module_on(hdl, ALL_CHN);
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_emerg_off: gsm_get_module_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);



	
zsys_debug("test_module_emerg_off: test gsm_module_emerg_off(-1) ----------------------------");
	ret = gsm_module_emerg_off(hdl, ALL_CHN);
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != '0')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_module_emerg_off: gsm_module_emerg_off(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_module_emerg_off: gsm_module_emerg_off(%d) suc(status:[%s])", ALL_CHN, buf);

	
	ret = gsm_module_on(hdl, ALL_CHN);
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_emerg_off: gsm_get_module_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);


zsys_debug("test_module_emerg_off: test gsm_module_emerg_off(0~31) ----------------------------");
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsm_module_emerg_off(hdl, i);
		ret = gsm_get_module_status(hdl, i, buf);
		if (buf[0] != '0')
			zsys_error("test_module_emerg_off: gsm_module_emerg_off(%d) err(status:[%s])", i, buf);
		else
			zsys_debug("test_module_emerg_off: gsm_module_emerg_off(%d) suc(status:[%s])", i, buf);
	}
	
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_emerg_off: gsm_get_module_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

	
	gsm_mcu_hdl_uinit(hdl);
}

void test_simcard_insert_det(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_simcard_insert_det: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(LOCALHOST, 8808);
	if (hdl == NULL)
	{
		zsys_error("test_simcard_insert_det: gsm_mcu_hdl_init return NULL");
		return;
	}

zsys_debug("test_simcard_insert_det: test gsm_get_simcard_insert_status(-1) ----------------------------");
	ret = gsm_get_simcard_insert_status(hdl, ALL_CHN, buf);
	zsys_debug("test_simcard_insert_det: gsm_get_simcard_insert_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

zsys_debug("test_simcard_insert_det: test gsm_get_simcard_insert_status(0~31) ----------------------------");
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsm_get_simcard_insert_status(hdl, i, buf);
		zsys_debug("test_simcard_insert_det: gsm_get_simcard_insert_status done, ret = [%d], chn = [%d], buf = [%s]", ret, i, buf);
	}
	
	gsm_mcu_hdl_uinit(hdl);
}
void test_gsmboard_insert_det(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_gsmboard_insert_det: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(LOCALHOST, 8808);
	if (hdl == NULL)
	{
		zsys_error("test_gsmboard_insert_det: gsm_mcu_hdl_init return NULL");
		return;
	}

zsys_debug("test_gsmboard_insert_det: test gsm_get_gsmboard_insert_status(-1) ----------------------------");
	ret = gsm_get_gsmboard_insert_status(hdl, ALL_CHN, buf);
	zsys_debug("test_gsmboard_insert_det: gsm_get_gsmboard_insert_status done, ret = [%d], board = [%d], buf = [%s]", ret, ALL_CHN, buf);

zsys_debug("test_gsmboard_insert_det: test gsm_get_gsmboard_insert_status(0~31) ----------------------------");
	for (i = 0; i < MAX_CHN/2; i++) // һ¿émboard°üédule
	{
		ret = gsm_get_gsmboard_insert_status(hdl, i, buf);
		zsys_debug("test_gsmboard_insert_det: gsm_get_gsmboard_insert_status done, ret = [%d], board = [%d], buf = [%s]", ret, i, buf);
	}
	
	gsm_mcu_hdl_uinit(hdl);
}

void test_module_status(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_module_status: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(LOCALHOST, 8808);
	if (hdl == NULL)
	{
		zsys_error("test_module_status: gsm_mcu_hdl_init return NULL");
		return;
	}

zsys_debug("test_gsmboard_test_module_statusinsert_det: test gsm_get_module_status(-1) ----------------------------");
	ret = gsm_get_module_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_status: gsm_get_module_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

zsys_debug("test_module_status: test gsm_get_module_status(0~31) ----------------------------");
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsm_get_module_status(hdl, i, buf);
		zsys_debug("test_module_status: gsm_get_module_status done, ret = [%d], chn = [%d], buf = [%s]", ret, i, buf);
	}
	
	gsm_mcu_hdl_uinit(hdl);
}
void test_get_version(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init(LOCALHOST, 8808);
	if (hdl == NULL)
	{
		zsys_error("test_module_status: gsm_mcu_hdl_init return NULL");
		return;
	}

	ret = gsm_get_mcu_version(hdl, buf);
	zsys_debug("test_get_version: ret = [%d], buf = [%s]", ret, buf);
	
	gsm_mcu_hdl_uinit(hdl);
}


void test(void)
{
	test_simcard_enable();
	test_module_power();
	test_module_onoff();
	test_module_emerg_off();
	test_simcard_insert_det();
	test_gsmboard_insert_det();
	test_module_status();
	test_get_version();
}
int main(int argc, char **argv)
{
	//struct soap soap;
	//char result[128] = {0};
	unsigned int result = 0;
	char url[128] = {0};
	int ret = 0;
	pid_t pid = 0;
	zsys_set_logident(argv[0]);

	test();

	zsys_shutdown();
	return 0;
}

