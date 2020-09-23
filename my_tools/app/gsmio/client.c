/*
	client.c

	Example gsm32 mcu handle service client in C

	Compilation in C (see samples/soap_gsmio/soap_gsmio.h):
	$ soapcpp2 -c soap_gsmio.h
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

#include "gsmio.h"

#include "soap_gsmio.nsmap"
#include "soapH.h"


//---------------------------------------------



//---------------------------------------------



////////////////////////////////////////////////////////////////////////////////////////////
gsmio_handle_t * gsmio_open(char *host, unsigned short port)
{
	gsmio_handle_t *hdl = malloc(sizeof(gsmio_handle_t));
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

void gsmio_close(gsmio_handle_t *handle)
{
	assert(handle);
	assert(handle->soap);
	soap_destroy((struct soap *)handle->soap);
	soap_end((struct soap *)handle->soap);
	//soap_done(soap);
	soap_free((struct soap *)handle->soap);
	free(handle);
}

// gsoap ns service method: set simcard source
// 0:simserver£»1:local
int gsmio_set_simcard_src(gsmio_handle_t *handle, int chn, int src)
{
	int ret = 0;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));
	assert(src == 0 || src == 1);

	soap_call_ns__gsmio_set_simcard_src((struct soap *)handle->soap, handle->url, "", chn, src, &ret);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

//gsoap ns service method: get simcard source
// 0:simserver£»1:local
int gsmio_get_simcard_src(gsmio_handle_t *handle, int chn, char *result)
{
	ns__gsmio_rsp_t rsp;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsmio_get_simcard_src((struct soap *)handle->soap, handle->url, "", chn, &rsp);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (rsp.value != NULL && strlen(rsp.value) > 0)
			strcpy(result, rsp.value);
	}

	return rsp.result;
}


//gsoap ns service method: set gsm module power on
int gsmio_pwr_on(gsmio_handle_t *handle, int chn)
{
	int ret = 0;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsmio_pwr_on((struct soap *)handle->soap, handle->url, "", chn, &ret);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
		return HDL_ERR;
	}
	return ret;
}

//gsoap ns service method: set gsm module power of
int gsmio_pwr_off(gsmio_handle_t *handle, int chn)
{
	int ret = 0;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsmio_pwr_off((struct soap *)handle->soap, handle->url, "", chn, &ret);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
		return HDL_ERR;
	}
	return ret;
}

// gsoap ns service method: get gsm module power status
int gsmio_get_pwr_status(gsmio_handle_t *handle, int chn, char *result)
{
	ns__gsmio_rsp_t rsp;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsmio_get_pwr_status((struct soap *)handle->soap, handle->url, "", chn, &rsp);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
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

//gsoap ns service method: set gsm module pwrkey on
int gsmio_pwrkey_on(gsmio_handle_t *handle, int chn)
{
	int ret = 0;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsmio_pwrkey_on((struct soap *)handle->soap, handle->url, "", chn, &ret);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

//gsoap ns service method: set gsm module pwrkey off
int gsmio_pwrkey_off(gsmio_handle_t *handle, int chn)
{
	int ret = 0;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsmio_pwrkey_off((struct soap *)handle->soap, handle->url, "", chn, &ret);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}

//gsoap ns service method: set gsm module emerg off
int gsmio_emerg_off(gsmio_handle_t *handle, int chn)
{
	int ret = 0;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	soap_call_ns__gsmio_emerg_off((struct soap *)handle->soap, handle->url, "", chn, &ret);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
		return HDL_ERR;
	}
	else
	{
		if (ret == HDL_ERR)
			return HDL_ERR;
	}
	return HDL_OK;
}


//gsoap ns service method: get gsm module pwrkey status
int gsmio_get_pwrkey_status(gsmio_handle_t *handle, int chn, char *result)
{
	ns__gsmio_rsp_t rsp;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsmio_get_pwrkey_status((struct soap *)handle->soap, handle->url, "", chn, &rsp);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
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
int gsmio_get_simcard_insert_status(gsmio_handle_t *handle, int chn, char *result)
{
	ns__gsmio_rsp_t rsp;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsmio_get_simcard_insert_status((struct soap *)handle->soap, handle->url, "", chn, &rsp);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
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
int gsmio_get_gsmboard_insert_status(gsmio_handle_t *handle, int chn, char *result)
{
	ns__gsmio_rsp_t rsp;

	assert(handle);
	assert((chn >= -1) && (chn < MAX_CHN/2));

	memset((char *)&rsp, 0, sizeof(rsp));
	
	soap_call_ns__gsmio_get_gsmboard_insert_status((struct soap *)handle->soap, handle->url, "", chn, &rsp);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
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

//gsoap ns service method: get mcu version
int gsmio_get_mcu_version(gsmio_handle_t *handle, char *result)
{
	int ret = 0;
	char *buf = NULL;

	assert(handle);
	
	ret = soap_call_ns__gsmio_get_mcu_version((struct soap *)handle->soap, handle->url, "", &buf);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
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
int gsmio_get_mcu_help(gsmio_handle_t *handle, char *result)
{
	int ret = 0;
	char *buf = NULL;

	assert(handle);
	
	ret = soap_call_ns__gsmio_get_mcu_help((struct soap *)handle->soap, handle->url, "", &buf);
	if (((struct soap *)handle->soap)->error)
	{
		soap_print_fault((struct soap *)handle->soap, stderr);
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


#if 0
void test_simcard_src(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	ns__gsmio_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
	
	gsmio_handle_t *hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		zsys_error("gsmio_open return NULL");
		return -1;
	}


	// set local
	ret = gsmio_set_simcard_src(hdl, ALL_CHN, SIMCARD_LOCAL);
	if (ret == HDL_OK)
	{
		zsys_debug("test_simcard_src: gsmio_set_simcard_src(chn:%d) suc", ALL_CHN);
		ret = gsmio_get_simcard_src(hdl, i, buf);
		if (strchr("F", buf[i]) != NULL || strchr("f", buf[i]) != NULL)
			zsys_debug("test_module_power_onoff: gsmio_pwr_on(%d) suc(status:[%s])", i, buf);
	}
	else if (ret == HDL_MCU_ERR)
		zsys_debug("test_module_power_onoff: gsmio_pwr_on(%d) fail, chn not exist", i);
	else
		zsys_error("test_module_power_onoff: gsmio_pwr_on(%d) err", i);
	ret = gsmio_get_simcard_src(hdl, ALL_CHN, buf);
	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		zsys_debug("buf[%d] = [%c]", i, buf[i]);
		if (strchr("F", buf[i]) != NULL || strchr("f", buf[i]) != NULL)
			ret = HDL_OK;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_simcard_src: gsmio_set_simcard_src(%d, %d) err(status:[%s])", ALL_CHN, SIMCARD_LOCAL, buf);
	else
		zsys_debug("test_simcard_src: gsmio_set_simcard_src(%d, %d) suc(status:[%s])", ALL_CHN, SIMCARD_LOCAL, buf);

	
	// set remote
	gsmio_set_simcard_src(hdl, ALL_CHN, SIMCARD_REMOTE);
	ret = gsmio_get_simcard_src(hdl, ALL_CHN, buf);
	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (strchr("0", buf[i]) != NULL)
			ret = HDL_OK;
	}
	if (ret != HDL_OK)
		zsys_error("test_simcard_src: gsmio_set_simcard_src(%d, %d) err(status:[%s])", ALL_CHN, SIMCARD_REMOTE, buf);
	else
		zsys_debug("test_simcard_src: gsmio_set_simcard_src(%d, %d) suc(status:[%s])", ALL_CHN, SIMCARD_REMOTE, buf);

	
	// set local
	ret = gsmio_set_simcard_src(hdl, ALL_CHN, SIMCARD_LOCAL);
	ret = gsmio_get_simcard_src(hdl, ALL_CHN, buf);

	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (strchr("F", buf[i]) != NULL || strchr("f", buf[i]) != NULL)
			ret = HDL_OK;
	}
	if (ret != HDL_OK)
		zsys_error("test_simcard_src: gsmio_set_simcard_src(%d, %d) err(status:[%s])", ALL_CHN, SIMCARD_LOCAL, buf);
	else
		zsys_debug("test_simcard_src: gsmio_set_simcard_src(%d, %d) suc(status:[%s])", ALL_CHN, SIMCARD_LOCAL, buf);
	

	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsmio_set_simcard_src(hdl, i, SIMCARD_REMOTE);
		ret = gsmio_get_simcard_src(hdl, i, buf);
		if (strstr(buf, "0") == NULL)
			zsys_error("test_simcard_src: gsmio_set_simcard_src(%d, %d) err(status:[%s])", i, SIMCARD_REMOTE, buf);
		else
			zsys_debug("test_simcard_src: gsmio_set_simcard_src(%d, %d) suc(status:[%s])", i, SIMCARD_REMOTE, buf);
	}
	ret = gsmio_get_simcard_src(hdl, ALL_CHN, buf);
	zsys_debug("test_simcard_src: gsmio_get_simcard_src done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

	
	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsmio_set_simcard_src(hdl, i, SIMCARD_LOCAL);
		ret = gsmio_get_simcard_src(hdl, i, buf);
		if (strstr(buf, "1") == NULL)
			zsys_error("test_simcard_src: gsmio_set_simcard_src(%d, %d) err(status:[%s])", i, SIMCARD_LOCAL, buf);
		else
			zsys_debug("test_simcard_src: gsmio_set_simcard_src(%d, %d) suc(status:[%s])", i, SIMCARD_LOCAL, buf);
	}
	ret = gsmio_get_simcard_src(hdl, ALL_CHN, buf);
	zsys_debug("test_simcard_src: gsmio_get_simcard_src done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

	
	gsmio_close(hdl);
}

void test_module_power_onoff(void)
{
	int i = 0;
	int ret = 0;
	int res = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	ns__gsmio_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
	gsmio_handle_t *hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		zsys_error("gsmio_open return NULL");
		return -1;
	}
	
	
	zsys_debug("test_module_power_onoff: gsmio_pwr_off(%d), ret = %d)", ALL_CHN, gsmio_pwr_off(hdl, ALL_CHN));
	ret = gsmio_get_pwr_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power_onoff: gsmio_get_pwr_status(%d), ret = %d, buf = %s", ALL_CHN, ret, buf);
	if (ret = HDL_OK)
	{
		for (i = 0; i < strlen(buf); i++)
		{
			if (strchr("0", buf[i]) == NULL)
				ret = HDL_ERR;
		}
		if (ret != HDL_OK)
			zsys_error("test_module_power_onoff: gsmio_pwr_off(%d) err(status:[%s])", ALL_CHN, buf);
		else
			zsys_debug("test_module_power_onoff: gsmio_pwr_off(%d) suc(status:[%s])", ALL_CHN, buf);
	}
	
	zsys_debug("test_module_power_onoff: gsmio_pwr_on(%d), ret = %d)", ALL_CHN, gsmio_pwr_on(hdl, ALL_CHN));
	ret = gsmio_get_pwr_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power_onoff: gsmio_get_pwr_status(%d), ret = %d, buf = %s", ALL_CHN, ret, buf);
	if (ret = HDL_OK)
	{
		for (i = 0; i < strlen(buf); i++)
		{
			if (strchr("f", buf[i]) != NULL || strchr("F", buf[i]) != NULL)
				ret = HDL_ERR;
		}
		if (ret != HDL_OK)
			zsys_error("test_module_power_onoff: gsmio_pwr_off(%d) err(status:[%s])", ALL_CHN, buf);
		else
			zsys_debug("test_module_power_onoff: gsmio_pwr_off(%d) suc(status:[%s])", ALL_CHN, buf);
	}

	
	zsys_debug("test_module_power_onoff: gsmio_pwr_off(%d), ret = %d)", ALL_CHN, gsmio_pwr_off(hdl, ALL_CHN));
	ret = gsmio_get_pwr_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power_onoff: gsmio_get_pwr_status(%d), ret = %d, buf = %s", ALL_CHN, ret, buf);
	if (ret = HDL_OK)
	{
		for (i = 0; i < strlen(buf); i++)
		{
			if (strchr("0", buf[i]) == NULL)
				ret = HDL_ERR;
		}
		if (ret != HDL_OK)
			zsys_error("test_module_power_onoff: gsmio_pwr_off(%d) err(status:[%s])", ALL_CHN, buf);
		else
			zsys_debug("test_module_power_onoff: gsmio_pwr_off(%d) suc(status:[%s])", ALL_CHN, buf);
	}
	

	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsmio_pwr_on(hdl, i);
		if (ret == HDL_OK)
		{
			zsys_debug("test_module_power_onoff: gsmio_pwr_on(%d) suc", i);
			ret = gsmio_get_pwr_status(hdl, i, buf);
			zsys_debug("test_module_power_onoff: gsmio_get_pwr_status(%d), ret = %d, buf = %s", i, ret, buf);
			if (strstr(buf, "1") != NULL)
				zsys_debug("test_module_power_onoff: gsmio_pwr_on(%d) suc(status:[%s])", i, buf);
		}
		else if (ret == HDL_MCU_ERR)
			zsys_debug("test_module_power_onoff: gsmio_pwr_on(%d) fail, chn not exist", i);
		else
			zsys_error("test_module_power_onoff: gsmio_pwr_on(%d) err", i);
	}
	ret = gsmio_get_pwr_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power_onoff: gsmio_get_pwr_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsmio_pwr_off(hdl, i);
		if (ret == HDL_OK)
		{
			zsys_debug("test_module_power_onoff: gsmio_pwr_offf(%d) suc", i);
			ret = gsmio_get_pwr_status(hdl, i, buf);
			zsys_debug("test_module_power_onoff: gsmio_get_pwr_status(%d), ret = %d, buf = %s", i, ret, buf);
			if (strstr(buf, "0") != NULL)
				zsys_debug("test_module_power_onoff: gsmio_pwr_off(%d) suc(status:[%s])", i, buf);
		}
		else if (ret == HDL_MCU_ERR)
			zsys_debug("test_module_power_onoff: gsmio_pwr_of(%d) fail, chn not exist", i);
		else
			zsys_error("test_module_power_onoff: gsmio_pwr_of(%d) err", i);
	}
	ret = gsmio_get_pwr_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power_onoff: gsmio_get_pwr_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);


	gsmio_pwr_on(hdl, ALL_CHN);
	zsys_debug("test_module_power_onoff: gsmio_pwr_on done, ret = [%d], chn = [%d]", ret, ALL_CHN);
	ret = gsmio_get_pwr_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_power_onoff: gsmio_get_pwr_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);
	
	gsmio_close(hdl);
}
void test_module_onoff(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	ns__gsmio_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
	gsmio_handle_t *hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		zsys_error("gsmio_open return NULL");
		return -1;
	}


	zsys_debug("gsmio_pwrkey_off(%d), ret = %d", ALL_CHN, gsmio_pwrkey_off(hdl, ALL_CHN));
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("gsmio_get_pwrkey_status(%d), ret = %d, buf = %s", ALL_CHN, ret, buf);
	if (ret = HDL_OK)
	{
		for (i = 0; i < strlen(buf); i++)
		{
			if (strchr("0", buf[i]) == NULL)
				ret = HDL_ERR;
		}
		if (ret != HDL_OK)
			zsys_error("gsmio_pwrkey_off(%d) err(status:[%s])", ALL_CHN, buf);
		else
			zsys_debug("gsmio_pwrkey_off(%d) suc(status:[%s])", ALL_CHN, buf);
	}
	
	zsys_debug("gsmio_pwrkey_on(%d), ret = %d", ALL_CHN, gsmio_pwrkey_on(hdl, ALL_CHN));
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("gsmio_get_pwrkey_status(%d), ret = %d, buf = %s", ALL_CHN, ret, buf);
	if (ret = HDL_OK)
	{
		for (i = 0; i < strlen(buf); i++)
		{
			if (strchr("f", buf[i]) != NULL || strchr("F", buf[i]) != NULL)
				ret = HDL_ERR;
		}
		if (ret != HDL_OK)
			zsys_error("gsmio_pwrkey_on(%d) err(status:[%s])", ALL_CHN, buf);
		else
			zsys_debug("gsmio_pwrkey_on(%d) suc(status:[%s])", ALL_CHN, buf);
	}

	zsys_debug("gsmio_pwrkey_off(%d), ret = %d", ALL_CHN, gsmio_pwrkey_off(hdl, ALL_CHN));
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("gsmio_get_pwrkey_status(%d), ret = %d, buf = %s", ALL_CHN, ret, buf);
	if (ret = HDL_OK)
	{
		for (i = 0; i < strlen(buf); i++)
		{
			if (strchr("0", buf[i]) == NULL)
				ret = HDL_ERR;
		}
		if (ret != HDL_OK)
			zsys_error("gsmio_pwrkey_off(%d) err(status:[%s])", ALL_CHN, buf);
		else
			zsys_debug("gsmio_pwrkey_off(%d) suc(status:[%s])", ALL_CHN, buf);
	}

	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsmio_pwrkey_on(hdl, i);
		if (ret == HDL_OK)
		{
			zsys_debug("gsmio_pwrkey_on(%d) suc", i);
			ret = gsmio_get_pwrkey_status(hdl, i, buf);
			zsys_debug("gsmio_get_pwr_status(%d), ret = %d, buf = %s", i, ret, buf);
			if (strstr(buf, "1") != NULL)
				zsys_debug("gsmio_pwrkey_on(%d) suc(status:[%s])", i, buf);
		}
		else if (ret == HDL_MCU_ERR)
			zsys_debug("gsmio_pwrkey_on(%d) fail, chn not exist", i);
		else
			zsys_error("gsmio_pwrkey_on(%d) err", i);
	}
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("gsmio_get_pwrkey_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);
	
	ret = HDL_OK;
	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsmio_pwrkey_off(hdl, i);
		if (ret == HDL_OK)
		{
			zsys_debug("gsmio_pwrkey_off(%d) suc", i);
			ret = gsmio_get_pwrkey_status(hdl, i, buf);
			zsys_debug("gsmio_get_pwr_status(%d), ret = %d, buf = %s", i, ret, buf);
			if (strstr(buf, "0") != NULL)
				zsys_debug("gsmio_pwrkey_off(%d) suc(status:[%s])", i, buf);
		}
		else if (ret == HDL_MCU_ERR)
			zsys_debug("gsmio_pwrkey_off(%d) fail, chn not exist", i);
		else
			zsys_error("gsmio_pwrkey_off(%d) err", i);
	}
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("gsmio_get_pwrkey_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

	zsys_debug("gsmio_pwrkey_on(%d), ret = %d", ALL_CHN, gsmio_pwrkey_on(hdl, ALL_CHN));
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("gsmio_get_pwrkey_status(%d), ret = %d, buf = %s", ALL_CHN, ret, buf);
	if (ret = HDL_OK)
	{
		for (i = 0; i < strlen(buf); i++)
		{
			if (strchr("f", buf[i]) != NULL || strchr("F", buf[i]) != NULL)
				ret = HDL_ERR;
		}
		if (ret != HDL_OK)
			zsys_error("gsmio_pwrkey_on(%d) err(status:[%s])", ALL_CHN, buf);
		else
			zsys_debug("gsmio_pwrkey_on(%d) suc(status:[%s])", ALL_CHN, buf);
	}
	

	gsmio_close(hdl);
}

void test_module_emerg_off(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	ns__gsmio_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
	gsmio_handle_t *hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		zsys_error("test_module_emerg_off: gsmio_open return NULL");
		return -1;
	}
	
	ret = gsmio_pwrkey_on(hdl, ALL_CHN);
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_emerg_off: gsmio_get_pwrkey_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);


	ret = gsmio_emerg_off(hdl, ALL_CHN);
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	ret = HDL_OK;
	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] != '0')
			ret = HDL_ERR;
	}
	
	if (ret != HDL_OK)
		zsys_error("test_module_emerg_off: gsmio_pwrkey_on(%d) err(status:[%s])", ALL_CHN, buf);
	else
		zsys_debug("test_module_emerg_off: gsmio_pwrkey_on(%d) suc(status:[%s])", ALL_CHN, buf);

	
	ret = gsmio_pwrkey_on(hdl, ALL_CHN);
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_emerg_off: gsmio_get_pwrkey_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);


	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsmio_emerg_off(hdl, i);
		ret = gsmio_get_pwrkey_status(hdl, i, buf);
		if (buf[0] != '0')
			zsys_error("test_module_emerg_off: gsmio_emerg_off(%d) err(status:[%s])", i, buf);
		else
			zsys_debug("test_module_emerg_off: gsmio_emerg_off(%d) suc(status:[%s])", i, buf);
	}
	
	ret = gsmio_get_pwrkey_status(hdl, ALL_CHN, buf);
	zsys_debug("test_module_emerg_off: gsmio_get_pwrkey_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

	
	gsmio_close(hdl);
}

void test_simcard_insert_det(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	ns__gsmio_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
	gsmio_handle_t *hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		zsys_error("test_simcard_insert_det: gsmio_open return NULL");
		return -1;
	}

	ret = gsmio_get_simcard_insert_status(hdl, ALL_CHN, buf);
	zsys_debug("test_simcard_insert_det: gsmio_get_simcard_insert_status done, ret = [%d], chn = [%d], buf = [%s]", ret, ALL_CHN, buf);

	for (i = 0; i < MAX_CHN; i++)
	{
		ret = gsmio_get_simcard_insert_status(hdl, i, buf);
		zsys_debug("test_simcard_insert_det: gsmio_get_simcard_insert_status done, ret = [%d], chn = [%d], buf = [%s]", ret, i, buf);
	}
	
	gsmio_close(hdl);
}
void test_gsmboard_insert_det(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	ns__gsmio_rsp_t rsp;
	memset((char *)&rsp, 0, sizeof(rsp));
	
	gsmio_handle_t *hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		zsys_error("test_gsmboard_insert_det: gsmio_open return NULL");
		return -1;
	}

	ret = gsmio_get_gsmboard_insert_status(hdl, ALL_CHN, buf);
	zsys_debug("test_gsmboard_insert_det: gsmio_get_gsmboard_insert_status done, ret = [%d], board = [%d], buf = [%s]", ret, ALL_CHN, buf);

	for (i = 0; i < MAX_CHN/2; i++) // Ò»¿égsmboard°üº¬Á½¿émodule
	{
		ret = gsmio_get_gsmboard_insert_status(hdl, i, buf);
		zsys_debug("test_gsmboard_insert_det: gsmio_get_gsmboard_insert_status done, ret = [%d], board = [%d], buf = [%s]", ret, i, buf);
	}
	
	gsmio_close(hdl);
}

void test_get_mcu_version(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
	gsmio_handle_t *hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		zsys_error("test_get_mcu_version: gsmio_open return NULL");
		return;
	}

	ret = gsmio_get_mcu_version(hdl, &buf);
	zsys_debug("test_get_mcu_version: gsm_get_mcu_version done, ret = [%d], buf = [%s]", ret, buf);
	
	gsmio_close(hdl);
}

void test_get_mcu_help(void)
{
	int i = 0;
	int ret = 0;
	unsigned char *result = NULL;
	char buf[1024] = {0};
	
	gsmio_handle_t *hdl = gsmio_open(LOCALHOST, GSOAP_PORT);
	if (hdl == NULL)
	{
		zsys_error("test_get_mcu_help: gsmio_open return NULL");
		return;
	}

	ret = gsmio_get_mcu_help(hdl, &buf);
	zsys_debug("test_get_mcu_help: gsm_get_mcu_help done, ret = [%d], buf = [%s]", ret, buf);
	
	gsmio_close(hdl);
}


void test(void)
{
#if 0
zsys_debug("test_simcard_src: begin=====================================================");
	test_simcard_src();
#endif
#if 0
zsys_debug("test_module_power: begin=====================================================");
	test_module_power_onoff();
#endif
#if 1
zsys_debug("test_module_onoff: begin=====================================================");
	test_module_onoff();
#endif
#if 0
zsys_debug("test_module_emerg_off: begin=====================================================");
	test_module_emerg_off();
#endif
#if 0
zsys_debug("test_simcard_insert_det: begin=====================================================");
	test_simcard_insert_det();
#endif
#if 0
zsys_debug("test_gsmboard_insert_det: begin=====================================================");
	test_gsmboard_insert_det();
#endif
#if 0
zsys_debug("test_get_mcu_version: begin=====================================================");
	test_get_mcu_version();
#endif
#if 0
zsys_debug("test_get_mcu_help: begin=====================================================");
	test_get_mcu_help();
#endif
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
