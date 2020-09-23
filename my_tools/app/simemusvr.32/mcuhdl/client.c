/*
	client.c

	Example gsm32 mcu handle service client in C

	Compilation in C (see samples/gsm_mcu_hdl/gsm_mcu_hdl.h):
	$ soapcpp2 -c gsm_mcu_hdl.h
	$ cc -o client client.c stdsoap2.c soapC.c soapClient.c
	where stdsoap2.c is in the 'gsoap' directory, or use libgsoap:
	$ cc -o client client.c soapC.c soapClient.c -lgsoap

--------------------------------------------------------------------------------
gSOAP XML Web services tools
Copyright (C) 2001-2008, Robert van Engelen, Genivia, Inc. All Rights Reserved.
This software is released under one of the following two licenses:
GPL or Genivia's license for commercial use.
--------------------------------------------------------------------------------
GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------
A commercial use license is available from Genivia, Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/
#include "soapH.h"
#include "gsm_mcu_hdl.nsmap"
#include "mcuhdl.h"

//---------------------------------------------



//---------------------------------------------



////////////////////////////////////////////////////////////////////////////////////////////
gsm_mcu_hdl_t * gsm_mcu_hdl_init(char *host, unsigned short port)
{
	gsm_mcu_hdl_t *hdl = malloc(sizeof(gsm_mcu_hdl_t));
	if (hdl == NULL)
		return NULL;
	
	if (host == NULL)
		strcpy(hdl->host, LOCALHOST);
	else
		strcpy(hdl->host, host);
	if (port == 0)
		hdl->port = 8808;
	else
		hdl->port = port;
	sprintf(hdl->url, "http://%s:%d", hdl->host, hdl->port);

	hdl->soap = NULL;
	hdl->soap = soap_malloc((struct soap *)hdl->soap, sizeof(struct soap));
	if (hdl->soap == NULL)
	{
		free(hdl);
		return NULL;
	}
	
	soap_init1((struct soap *)hdl->soap, SOAP_XML_INDENT);
	return hdl;
}

void gsm_mcu_hdl_uinit(gsm_mcu_hdl_t *hdl)
{
	assert(hdl);
	assert(hdl->soap);
	soap_destroy((struct soap *)hdl->soap);
	soap_end((struct soap *)hdl->soap);
	//soap_done(soap);
	soap_free((struct soap *)hdl->soap);
	free(hdl);
}

//gsoap ns service method: gsm simcard enable
// chn: -1,0~7
int gsm_simcard_enable(gsm_mcu_hdl_t *hdl, int chn)
{
	int ret = 0;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsm_simcard_enable((struct soap *)hdl->soap, hdl->url, "", chn, &ret);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}


//gsoap ns service method: gsm simcard disable
// chn: -1,0~7
int gsm_simcard_disable(gsm_mcu_hdl_t *hdl, int chn)
{
	int ret = 0;
	
	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsm_simcard_disable((struct soap *)hdl->soap, hdl->url, "", chn, &ret);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}


//gsoap ns service method: get gsm simcard status
// chn: -1,0~7
int gsm_get_simcard_status(gsm_mcu_hdl_t *hdl, int chn, char *result)
{
	struct ns__gsm_mcu_rsp_t rsp;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsm_get_simcard_status((struct soap *)hdl->soap, hdl->url, "", chn, &rsp);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (rsp.value != NULL && strlen(rsp.value) > 0)
		{
			strcpy(result, rsp.value);
		}
	}
	return rsp.result;
}


//gsoap ns service method: gsm module power on
int gsm_module_power_on(gsm_mcu_hdl_t *hdl, int chn)
{
	int ret = 0;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsm_module_power_on((struct soap *)hdl->soap, hdl->url, "", chn, &ret);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

//gsoap ns service method: gsm module power off
int gsm_module_power_off(gsm_mcu_hdl_t *hdl, int chn)
{
	int ret = 0;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsm_module_power_off((struct soap *)hdl->soap, hdl->url, "", chn, &ret);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

// gsoap ns service method: get gsm module power status
int gsm_get_module_power_status(gsm_mcu_hdl_t *hdl, int chn, char *result)
{
	struct ns__gsm_mcu_rsp_t rsp;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsm_get_module_power_status((struct soap *)hdl->soap, hdl->url, "", chn, &rsp);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (rsp.value != NULL && strlen(rsp.value) > 0)
		{
			strcpy(result, rsp.value);
		}
	}
	return rsp.result;
}

//gsoap ns service method: gsm module emerg off
int gsm_module_emerg_off(gsm_mcu_hdl_t *hdl, int chn)
{
	int ret = 0;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsm_module_emerg_off((struct soap *)hdl->soap, hdl->url, "", chn, &ret);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

//gsoap ns service method: get gsm module emerg off status
//int gsm_get_module_emerg_off_status(int chn, unsigned int *result);

//gsoap ns service method: gsm module on
int gsm_module_on(gsm_mcu_hdl_t *hdl, int chn)
{
	int ret = 0;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsm_module_on((struct soap *)hdl->soap, hdl->url, "", chn, &ret);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

//gsoap ns service method: gsm module off
int gsm_module_off(gsm_mcu_hdl_t *hdl, int chn)
{
	int ret = 0;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsm_module_off((struct soap *)hdl->soap, hdl->url, "", chn, &ret);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

//gsoap ns service method: get gsm module status
int gsm_get_module_status(gsm_mcu_hdl_t *hdl, int chn, char *result)
{
	struct ns__gsm_mcu_rsp_t rsp;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsm_get_module_status((struct soap *)hdl->soap, hdl->url, "", chn, &rsp);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (rsp.value != NULL && strlen(rsp.value) > 0)
		{
			strcpy(result, rsp.value);
		}
	}
	return rsp.result;
}

//gsoap ns service method: get gsm simcard insert status
int gsm_get_simcard_insert_status(gsm_mcu_hdl_t *hdl, int chn, char *result)
{
	struct ns__gsm_mcu_rsp_t rsp;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsm_get_simcard_insert_status((struct soap *)hdl->soap, hdl->url, "", chn, &rsp);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (rsp.value != NULL && strlen(rsp.value) > 0)
		{
			strcpy(result, rsp.value);
		}
	}
	return rsp.result;
}

// gsoap ns service method: get gsm gsmboard insert status
int gsm_get_gsmboard_insert_status(gsm_mcu_hdl_t *hdl, int chn, char *result)
{
	struct ns__gsm_mcu_rsp_t rsp;

	assert(hdl);
	assert((chn >= -1) && (chn < MAX_CHN/2));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsm_get_gsmboard_insert_status((struct soap *)hdl->soap, hdl->url, "", chn, &rsp);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (rsp.value != NULL && strlen(rsp.value) > 0)
		{
			strcpy(result, rsp.value);
		}
	}
	return rsp.result;
}

int gsm_set_led_control(gsm_mcu_hdl_t *hdl, char *buf_w, char *result)
{
	int ret = 0;
	assert(hdl);
	ret = soap_call_ns__gsm_set_led_control((struct soap *)hdl->soap, hdl->url, "", buf_w, &result);
	if (((struct soap *)hdl->soap)->error){
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	} else {
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

//gsoap ns service method: get mcu version
int gsm_get_mcu_version(gsm_mcu_hdl_t *hdl, char *result)
{
	int ret = 0;
	char *buf = NULL;

	assert(hdl);
	ret = soap_call_ns__gsm_get_mcu_version((struct soap *)hdl->soap, hdl->url, "", &buf);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	if (strlen(buf) <= 0)
	{
		return HDL_ERR;
	}
	strcpy(result, buf);
	return HDL_OK;
}


//gsoap ns service method: get mcu help
int gsm_get_mcu_help(gsm_mcu_hdl_t *hdl, char *result)
{
	int ret = 0;
	char *buf = NULL;

	assert(hdl);
	
	ret = soap_call_ns__gsm_get_mcu_help((struct soap *)hdl->soap, hdl->url, "", &buf);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	if (strlen(buf) <= 0)
	{
		return HDL_ERR;
	}
	strcpy(result, buf);
	return HDL_OK;
}

/* 有恢复默认设置事件就返回1否则返回其它 */
int gsm_get_restore_event(gsm_mcu_hdl_t *hdl)
{
    int ret = 0;
	char *buf = NULL;

	assert(hdl);
	
	ret = soap_call_ns__gsm_get_restore_event((struct soap *)hdl->soap, hdl->url, "", &buf);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
    
	if (strlen(buf) <= 0)
		return HDL_ERR;

	if ( strncmp("01", buf, 2) == 0 )
        return 1;
    
	return HDL_OK;
}

/* 
    读mcu上的寄存器
*/
int gsm_mcu_reg_read(gsm_mcu_hdl_t *hdl, int brd, int reg, int num, char *result)
{
    int ret = 0;
	char *buf = NULL;

	assert(hdl);

    ret = soap_call_ns__gsm_mcu_reg_read((struct soap *)hdl->soap, hdl->url, "", brd, reg, num, &buf);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}

    strcpy(result, buf);
	return HDL_OK;
}

/* 
    写mcu上的寄存器
*/
int gsm_mcu_reg_write(gsm_mcu_hdl_t *hdl, int brd, int reg, unsigned char val, char *result)
{
    int ret = 0;
	char *buf = NULL;

	assert(hdl);

    ret = soap_call_ns__gsm_mcu_reg_write((struct soap *)hdl->soap, hdl->url, "", brd, reg, val, &buf);
	if (((struct soap *)hdl->soap)->error)
	{
		soap_print_fault((struct soap *)hdl->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}

    strcpy(result, buf);
	return HDL_OK;
}

#if 0
void test_simcard_enable(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	struct ns__gsm_mcu_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
zsys_debug("test_simcard_enable: begin=====================================================");
	
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("gsm_mcu_hdl_init return NULL");
		return -1;
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
	struct ns__gsm_mcu_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
zsys_debug("test_module_power: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("gsm_mcu_hdl_init return NULL");
		return -1;
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
	struct ns__gsm_mcu_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
zsys_debug("test_module_onoff: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("test_module_onoff: gsm_mcu_hdl_init return NULL");
		return -1;
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
	struct ns__gsm_mcu_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
zsys_debug("test_module_power: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("test_module_emerg_off: gsm_mcu_hdl_init return NULL");
		return -1;
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
	struct ns__gsm_mcu_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
zsys_debug("test_simcard_insert_det: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("test_simcard_insert_det: gsm_mcu_hdl_init return NULL");
		return -1;
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
	struct ns__gsm_mcu_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
zsys_debug("test_gsmboard_insert_det: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("test_gsmboard_insert_det: gsm_mcu_hdl_init return NULL");
		return -1;
	}

zsys_debug("test_gsmboard_insert_det: test gsm_get_gsmboard_insert_status(-1) ----------------------------");
	ret = gsm_get_gsmboard_insert_status(hdl, ALL_CHN, buf);
	zsys_debug("test_gsmboard_insert_det: gsm_get_gsmboard_insert_status done, ret = [%d], board = [%d], buf = [%s]", ret, ALL_CHN, buf);

zsys_debug("test_gsmboard_insert_det: test gsm_get_gsmboard_insert_status(0~31) ----------------------------");
	for (i = 0; i < MAX_CHN/2; i++) // 一块gsmboard包含两块module
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
	struct ns__gsm_mcu_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
zsys_debug("test_module_status: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("test_module_status: gsm_mcu_hdl_init return NULL");
		return -1;
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

void test_get_mcu_version(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_get_mcu_version: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("test_get_mcu_version: gsm_mcu_hdl_init return NULL");
		return;
	}

	ret = gsm_get_mcu_version(hdl, &buf);
	zsys_debug("test_get_mcu_version: gsm_get_mcu_version done, ret = [%d], buf = [%s]", ret, buf);
	
	gsm_mcu_hdl_uinit(hdl);
}

void test_get_mcu_help(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
zsys_debug("test_get_mcu_help: begin=====================================================");
	gsm_mcu_hdl_t *hdl = gsm_mcu_hdl_init("172.16.1.14", 8808);
	if (hdl == NULL)
	{
		zsys_error("test_get_mcu_help: gsm_mcu_hdl_init return NULL");
		return;
	}

	ret = gsm_get_mcu_help(hdl, &buf);
	zsys_debug("test_get_mcu_help: gsm_get_mcu_help done, ret = [%d], buf = [%s]", ret, buf);
	
	gsm_mcu_hdl_uinit(hdl);
}


void test(void)
{
	//test_simcard_enable();
	//test_module_power();
	//test_module_onoff();
	//test_module_emerg_off();
	test_simcard_insert_det();
	//test_gsmboard_insert_det();
	//test_module_status();
	test_get_mcu_version();
	test_get_mcu_help();
}
#endif

#if 0
int main(int argc, char **argv)
{
	struct soap soap;
	//char result[128] = {0};
	unsigned int result = 0;
	char url[128] = {0};
	int ret = 0;
	pid_t pid = 0;
	zsys_set_logident (argv[0]);

	test();

	zsys_shutdown();
	return 0;
}
#endif
