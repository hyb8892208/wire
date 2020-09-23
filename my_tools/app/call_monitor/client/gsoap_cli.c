#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "../gsoap/soapStub.h"
#include "gsoap_cli.h"
#include "../gsoap/ns.nsmap"
char *g_endpoint = (char *)"http://127.0.0.1:8811";

static int cli_soap_init(struct soap * psoap)
{
        if(NULL == psoap)
        {
                printf("cli_soap_init error\n");
                return -1;
        }

	soap_init1(psoap, SOAP_XML_INDENT);
	soap_set_namespaces(psoap, namespaces);

        return 0;
}

static int cli_soap_finit(struct soap * psoap)
{
        if(NULL == psoap)
        {
                printf("cli_soap_finit error\n");
                return -1;
        }

    soap_destroy(psoap);
    soap_end(psoap);
    soap_done(psoap);

        return 0;
}

int callmonitor_reload_config()
{
	struct soap soap;
	int result = 0;
	cli_soap_init(&soap);
	soap_call_ns__config_reload(&soap, g_endpoint, "", &result);
	if(soap.error)
		result = -1;
	cli_soap_finit(&soap);
	return result;
}

int callmonitor_flush_status()
{
	struct soap soap;
	int result = 0;
	cli_soap_init(&soap);
	soap_call_ns__flush_status(&soap, g_endpoint, "", &result);
	if(soap.error)
		result = -1;
	cli_soap_finit(&soap);
	return result;
}


int callmonitor_get_chan_config(int chan_id, struct callmonitor_chan_config *conf)
{
	struct soap soap;
	int result = 0;
	struct ns__chan_config config;
	cli_soap_init(&soap);
	soap_call_ns__get_chan_conf(&soap, g_endpoint, "", chan_id,&config);
	if(soap.error)
		result = -1;
	else
		memcpy(conf, (struct callmonitor_chan_config *)&config, sizeof(struct callmonitor_chan_config));
	cli_soap_finit(&soap);
	return result;
}

int callmonitor_get_chan_data(int chan_id, struct callmonitor_chan_data *data)
{
	struct soap soap;
	int result = 0;
	struct ns__chan_data buf;
	cli_soap_init(&soap);
	soap_call_ns__get_chan_data(&soap, g_endpoint, "", chan_id,&buf);
	if(soap.error)
		result = -1;
	else
		memcpy(data, (struct callmonitor_chan_data *)&buf, sizeof(struct callmonitor_chan_data));
	cli_soap_finit(&soap);
	return result;
}