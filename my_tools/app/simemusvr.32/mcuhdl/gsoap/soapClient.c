/* soapClient.c
   Generated by gSOAP 2.8.17 from /home/peng/wg32_dev/my_tools/app/simemusvr.32/mcuhdl/gsoap/gsm_mcu_hdl.h

Copyright(C) 2000-2013, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"
#ifdef __cplusplus
extern "C" {
#endif

SOAP_SOURCE_STAMP("@(#) soapClient.c ver 2.8.17 2017-09-25 07:39:46 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_simcard_enable(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, int *result)
{	struct ns__gsm_simcard_enable soap_tmp_ns__gsm_simcard_enable;
	struct ns__gsm_simcard_enableResponse *soap_tmp_ns__gsm_simcard_enableResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_simcard_enable.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_simcard_enable(soap, &soap_tmp_ns__gsm_simcard_enable);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_simcard_enable(soap, &soap_tmp_ns__gsm_simcard_enable, "ns:gsm-simcard-enable", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_simcard_enable(soap, &soap_tmp_ns__gsm_simcard_enable, "ns:gsm-simcard-enable", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_simcard_enableResponse = soap_get_ns__gsm_simcard_enableResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_simcard_enableResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_simcard_enableResponse->result)
		*result = *soap_tmp_ns__gsm_simcard_enableResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_simcard_disable(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, int *result)
{	struct ns__gsm_simcard_disable soap_tmp_ns__gsm_simcard_disable;
	struct ns__gsm_simcard_disableResponse *soap_tmp_ns__gsm_simcard_disableResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_simcard_disable.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_simcard_disable(soap, &soap_tmp_ns__gsm_simcard_disable);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_simcard_disable(soap, &soap_tmp_ns__gsm_simcard_disable, "ns:gsm-simcard-disable", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_simcard_disable(soap, &soap_tmp_ns__gsm_simcard_disable, "ns:gsm-simcard-disable", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_simcard_disableResponse = soap_get_ns__gsm_simcard_disableResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_simcard_disableResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_simcard_disableResponse->result)
		*result = *soap_tmp_ns__gsm_simcard_disableResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_get_simcard_status(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, struct ns__gsm_mcu_rsp_t *result)
{	struct ns__gsm_get_simcard_status soap_tmp_ns__gsm_get_simcard_status;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_get_simcard_status.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_get_simcard_status(soap, &soap_tmp_ns__gsm_get_simcard_status);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_get_simcard_status(soap, &soap_tmp_ns__gsm_get_simcard_status, "ns:gsm-get-simcard-status", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_get_simcard_status(soap, &soap_tmp_ns__gsm_get_simcard_status, "ns:gsm-get-simcard-status", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_ns__gsm_mcu_rsp_t(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_ns__gsm_mcu_rsp_t(soap, result, "", "");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_module_power_on(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, int *result)
{	struct ns__gsm_module_power_on soap_tmp_ns__gsm_module_power_on;
	struct ns__gsm_module_power_onResponse *soap_tmp_ns__gsm_module_power_onResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_module_power_on.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_module_power_on(soap, &soap_tmp_ns__gsm_module_power_on);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_module_power_on(soap, &soap_tmp_ns__gsm_module_power_on, "ns:gsm-module-power-on", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_module_power_on(soap, &soap_tmp_ns__gsm_module_power_on, "ns:gsm-module-power-on", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_module_power_onResponse = soap_get_ns__gsm_module_power_onResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_module_power_onResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_module_power_onResponse->result)
		*result = *soap_tmp_ns__gsm_module_power_onResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_module_power_off(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, int *result)
{	struct ns__gsm_module_power_off soap_tmp_ns__gsm_module_power_off;
	struct ns__gsm_module_power_offResponse *soap_tmp_ns__gsm_module_power_offResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_module_power_off.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_module_power_off(soap, &soap_tmp_ns__gsm_module_power_off);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_module_power_off(soap, &soap_tmp_ns__gsm_module_power_off, "ns:gsm-module-power-off", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_module_power_off(soap, &soap_tmp_ns__gsm_module_power_off, "ns:gsm-module-power-off", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_module_power_offResponse = soap_get_ns__gsm_module_power_offResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_module_power_offResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_module_power_offResponse->result)
		*result = *soap_tmp_ns__gsm_module_power_offResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_get_module_power_status(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, struct ns__gsm_mcu_rsp_t *result)
{	struct ns__gsm_get_module_power_status soap_tmp_ns__gsm_get_module_power_status;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_get_module_power_status.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_get_module_power_status(soap, &soap_tmp_ns__gsm_get_module_power_status);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_get_module_power_status(soap, &soap_tmp_ns__gsm_get_module_power_status, "ns:gsm-get-module-power-status", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_get_module_power_status(soap, &soap_tmp_ns__gsm_get_module_power_status, "ns:gsm-get-module-power-status", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_ns__gsm_mcu_rsp_t(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_ns__gsm_mcu_rsp_t(soap, result, "", "");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_module_emerg_off(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, int *result)
{	struct ns__gsm_module_emerg_off soap_tmp_ns__gsm_module_emerg_off;
	struct ns__gsm_module_emerg_offResponse *soap_tmp_ns__gsm_module_emerg_offResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_module_emerg_off.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_module_emerg_off(soap, &soap_tmp_ns__gsm_module_emerg_off);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_module_emerg_off(soap, &soap_tmp_ns__gsm_module_emerg_off, "ns:gsm-module-emerg-off", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_module_emerg_off(soap, &soap_tmp_ns__gsm_module_emerg_off, "ns:gsm-module-emerg-off", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_module_emerg_offResponse = soap_get_ns__gsm_module_emerg_offResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_module_emerg_offResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_module_emerg_offResponse->result)
		*result = *soap_tmp_ns__gsm_module_emerg_offResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_module_on(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, int *result)
{	struct ns__gsm_module_on soap_tmp_ns__gsm_module_on;
	struct ns__gsm_module_onResponse *soap_tmp_ns__gsm_module_onResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_module_on.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_module_on(soap, &soap_tmp_ns__gsm_module_on);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_module_on(soap, &soap_tmp_ns__gsm_module_on, "ns:gsm-module-on", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_module_on(soap, &soap_tmp_ns__gsm_module_on, "ns:gsm-module-on", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_module_onResponse = soap_get_ns__gsm_module_onResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_module_onResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_module_onResponse->result)
		*result = *soap_tmp_ns__gsm_module_onResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_module_off(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, int *result)
{	struct ns__gsm_module_off soap_tmp_ns__gsm_module_off;
	struct ns__gsm_module_offResponse *soap_tmp_ns__gsm_module_offResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_module_off.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_module_off(soap, &soap_tmp_ns__gsm_module_off);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_module_off(soap, &soap_tmp_ns__gsm_module_off, "ns:gsm-module-off", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_module_off(soap, &soap_tmp_ns__gsm_module_off, "ns:gsm-module-off", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_int(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_module_offResponse = soap_get_ns__gsm_module_offResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_module_offResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_module_offResponse->result)
		*result = *soap_tmp_ns__gsm_module_offResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_get_module_status(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, struct ns__gsm_mcu_rsp_t *result)
{	struct ns__gsm_get_module_status soap_tmp_ns__gsm_get_module_status;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_get_module_status.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_get_module_status(soap, &soap_tmp_ns__gsm_get_module_status);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_get_module_status(soap, &soap_tmp_ns__gsm_get_module_status, "ns:gsm-get-module-status", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_get_module_status(soap, &soap_tmp_ns__gsm_get_module_status, "ns:gsm-get-module-status", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_ns__gsm_mcu_rsp_t(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_ns__gsm_mcu_rsp_t(soap, result, "", "");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_get_simcard_insert_status(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, struct ns__gsm_mcu_rsp_t *result)
{	struct ns__gsm_get_simcard_insert_status soap_tmp_ns__gsm_get_simcard_insert_status;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_get_simcard_insert_status.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_get_simcard_insert_status(soap, &soap_tmp_ns__gsm_get_simcard_insert_status);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_get_simcard_insert_status(soap, &soap_tmp_ns__gsm_get_simcard_insert_status, "ns:gsm-get-simcard-insert-status", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_get_simcard_insert_status(soap, &soap_tmp_ns__gsm_get_simcard_insert_status, "ns:gsm-get-simcard-insert-status", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_ns__gsm_mcu_rsp_t(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_ns__gsm_mcu_rsp_t(soap, result, "", "");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_get_gsmboard_insert_status(struct soap *soap, const char *soap_endpoint, const char *soap_action, int chn, struct ns__gsm_mcu_rsp_t *result)
{	struct ns__gsm_get_gsmboard_insert_status soap_tmp_ns__gsm_get_gsmboard_insert_status;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_get_gsmboard_insert_status.chn = chn;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_get_gsmboard_insert_status(soap, &soap_tmp_ns__gsm_get_gsmboard_insert_status);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_get_gsmboard_insert_status(soap, &soap_tmp_ns__gsm_get_gsmboard_insert_status, "ns:gsm-get-gsmboard-insert-status", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_get_gsmboard_insert_status(soap, &soap_tmp_ns__gsm_get_gsmboard_insert_status, "ns:gsm-get-gsmboard-insert-status", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	soap_default_ns__gsm_mcu_rsp_t(soap, result);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_get_ns__gsm_mcu_rsp_t(soap, result, "", "");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_set_led_control(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *buf, char **result)
{	struct ns__gsm_set_led_control soap_tmp_ns__gsm_set_led_control;
	struct ns__gsm_set_led_controlResponse *soap_tmp_ns__gsm_set_led_controlResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_set_led_control.buf = buf;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_set_led_control(soap, &soap_tmp_ns__gsm_set_led_control);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_set_led_control(soap, &soap_tmp_ns__gsm_set_led_control, "ns:gsm-set-led-control", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_set_led_control(soap, &soap_tmp_ns__gsm_set_led_control, "ns:gsm-set-led-control", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	*result = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_set_led_controlResponse = soap_get_ns__gsm_set_led_controlResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_set_led_controlResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_set_led_controlResponse->result)
		*result = *soap_tmp_ns__gsm_set_led_controlResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_get_mcu_version(struct soap *soap, const char *soap_endpoint, const char *soap_action, char **result)
{	struct ns__gsm_get_mcu_version soap_tmp_ns__gsm_get_mcu_version;
	struct ns__gsm_get_mcu_versionResponse *soap_tmp_ns__gsm_get_mcu_versionResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_get_mcu_version(soap, &soap_tmp_ns__gsm_get_mcu_version);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_get_mcu_version(soap, &soap_tmp_ns__gsm_get_mcu_version, "ns:gsm-get-mcu-version", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_get_mcu_version(soap, &soap_tmp_ns__gsm_get_mcu_version, "ns:gsm-get-mcu-version", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	*result = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_get_mcu_versionResponse = soap_get_ns__gsm_get_mcu_versionResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_get_mcu_versionResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_get_mcu_versionResponse->result)
		*result = *soap_tmp_ns__gsm_get_mcu_versionResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_get_mcu_help(struct soap *soap, const char *soap_endpoint, const char *soap_action, char **result)
{	struct ns__gsm_get_mcu_help soap_tmp_ns__gsm_get_mcu_help;
	struct ns__gsm_get_mcu_helpResponse *soap_tmp_ns__gsm_get_mcu_helpResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_get_mcu_help(soap, &soap_tmp_ns__gsm_get_mcu_help);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_get_mcu_help(soap, &soap_tmp_ns__gsm_get_mcu_help, "ns:gsm-get-mcu-help", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_get_mcu_help(soap, &soap_tmp_ns__gsm_get_mcu_help, "ns:gsm-get-mcu-help", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	*result = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_get_mcu_helpResponse = soap_get_ns__gsm_get_mcu_helpResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_get_mcu_helpResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_get_mcu_helpResponse->result)
		*result = *soap_tmp_ns__gsm_get_mcu_helpResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_get_restore_event(struct soap *soap, const char *soap_endpoint, const char *soap_action, char **result)
{	struct ns__gsm_get_restore_event soap_tmp_ns__gsm_get_restore_event;
	struct ns__gsm_get_restore_eventResponse *soap_tmp_ns__gsm_get_restore_eventResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_get_restore_event(soap, &soap_tmp_ns__gsm_get_restore_event);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_get_restore_event(soap, &soap_tmp_ns__gsm_get_restore_event, "ns:gsm-get-restore-event", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_get_restore_event(soap, &soap_tmp_ns__gsm_get_restore_event, "ns:gsm-get-restore-event", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	*result = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_get_restore_eventResponse = soap_get_ns__gsm_get_restore_eventResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_get_restore_eventResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_get_restore_eventResponse->result)
		*result = *soap_tmp_ns__gsm_get_restore_eventResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_mcu_reg_read(struct soap *soap, const char *soap_endpoint, const char *soap_action, int brd, int reg, int num, char **result)
{	struct ns__gsm_mcu_reg_read soap_tmp_ns__gsm_mcu_reg_read;
	struct ns__gsm_mcu_reg_readResponse *soap_tmp_ns__gsm_mcu_reg_readResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_mcu_reg_read.brd = brd;
	soap_tmp_ns__gsm_mcu_reg_read.reg = reg;
	soap_tmp_ns__gsm_mcu_reg_read.num = num;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_mcu_reg_read(soap, &soap_tmp_ns__gsm_mcu_reg_read);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_mcu_reg_read(soap, &soap_tmp_ns__gsm_mcu_reg_read, "ns:gsm-mcu-reg-read", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_mcu_reg_read(soap, &soap_tmp_ns__gsm_mcu_reg_read, "ns:gsm-mcu-reg-read", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	*result = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_mcu_reg_readResponse = soap_get_ns__gsm_mcu_reg_readResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_mcu_reg_readResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_mcu_reg_readResponse->result)
		*result = *soap_tmp_ns__gsm_mcu_reg_readResponse->result;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__gsm_mcu_reg_write(struct soap *soap, const char *soap_endpoint, const char *soap_action, int brd, int reg, unsigned char val, char **result)
{	struct ns__gsm_mcu_reg_write soap_tmp_ns__gsm_mcu_reg_write;
	struct ns__gsm_mcu_reg_writeResponse *soap_tmp_ns__gsm_mcu_reg_writeResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://websrv.cs.fsu.edu/~engelen/calcserver.cgi";
	soap_begin(soap);
	soap->encodingStyle = "";
	soap_tmp_ns__gsm_mcu_reg_write.brd = brd;
	soap_tmp_ns__gsm_mcu_reg_write.reg = reg;
	soap_tmp_ns__gsm_mcu_reg_write.val = val;
	soap_serializeheader(soap);
	soap_serialize_ns__gsm_mcu_reg_write(soap, &soap_tmp_ns__gsm_mcu_reg_write);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns__gsm_mcu_reg_write(soap, &soap_tmp_ns__gsm_mcu_reg_write, "ns:gsm-mcu-reg-write", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns__gsm_mcu_reg_write(soap, &soap_tmp_ns__gsm_mcu_reg_write, "ns:gsm-mcu-reg-write", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!result)
		return soap_closesock(soap);
	*result = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns__gsm_mcu_reg_writeResponse = soap_get_ns__gsm_mcu_reg_writeResponse(soap, NULL, "", "");
	if (!soap_tmp_ns__gsm_mcu_reg_writeResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (result && soap_tmp_ns__gsm_mcu_reg_writeResponse->result)
		*result = *soap_tmp_ns__gsm_mcu_reg_writeResponse->result;
	return soap_closesock(soap);
}

#ifdef __cplusplus
}
#endif

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapClient.c */