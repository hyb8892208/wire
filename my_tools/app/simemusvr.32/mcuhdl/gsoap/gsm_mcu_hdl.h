/*
	gsm_mcu_hdl.h

	This is a gSOAP header file with a calculator data binding and Web
	service interface to implement clients and services

	The service operations and type definitions use familiar C/C++ syntax.

	The following methods are defined by this gSOAP service definition:

	gsm_simcard_enable(chn, result)
	gsm_simcard_disable(chn, result)
	gsm_get_simcard_status(chn, unsigned result)
	gsm_module_power_on(chn, result)
	gsm_module_power_off(chn, result)
	gsm_get_module_power_status(chn, result)
	gsm_module_emerg_off(chn, result)
	gsm_get_module_emerg_off_status(chn, result)
	gsm_module_on(chn, result)
	gsm_module_off(chn, result)
	gsm_get_module_status(chn, unsigned result)
	gsm_get_simcard_insert_status(chn, result)
	gsm_get_gsmboard_insert_status(chn, result)
	gsm_get_mcu_version(result)
	gsm_get_mcu_help(result)

	Build steps for C (see samples/gsm_mcu_hdl):
	$ soapcpp2 -c -r gsm_mcu_hdl.h
	$ cc -o client client.c stdsoap2.c soapC.c soapClient.c
	$ cc -o server server.c stdsoap2.c soapC.c soapServer.c

	Build steps for C++ (see samples/gsm_mcu_hdl++):
	$ soapcpp2 -j -r gsm_mcu_hdl.h
	$ c++ -o calcclient++ calcclient.cpp stdsoap2.cpp soapC.cpp soapcalcProxy.cpp
	$ c++ -o calcserver++ calcserver.cpp stdsoap2.cpp soapC.cpp soapcalcService.cpp

	Option -r generates a soapReadme.md report.

	Note that soapcpp2 option -j generates proxy and service classes, which
	encapsulate the method operations in the class instead of defining them
	as global functions as in C. 

	The //gsoap directives are used to bind XML namespaces and to define
	Web service properties:

	//gsoap <ns> service name: <WSDLserviceName> <documentationText>
	//gsoap <ns> service style: [rpc|document]
	//gsoap <ns> service encoding: [literal|encoded]
	//gsoap <ns> service namespace: <WSDLnamespaceURI>
	//gsoap <ns> service location: <WSDLserviceAddressLocationURI>

	Web service operation properties:

	//gsoap <ns> service method-style: <methodName> [rpc|document]
	//gsoap <ns> service method-encoding: <methodName> [literal|encoded]
	//gsoap <ns> service method-action: <methodName> <actionString>
	//gsoap <ns> service method-documentation: <methodName> <documentation>

	and type schema properties:

	//gsoap <ns> schema namespace: <schemaNamespaceURI>
	//gsoap <ns> schema elementForm: [qualified|unqualified]
	//gsoap <ns> schema attributeForm: [qualified|unqualified]
	//gsoap <ns> schema documentation: <documentationText>
	//gsoap <ns> schema type-documentation: <typeName> <documentationText>

	where <ns> is an XML namespace prefix, which is used in C/C++ operation
	names, e.g. ns__add(), and type names, e.g. xsd__int.

	See the documentation for the full list of //gsoap directives.

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

//gsoap ns service name:	gsm_mcu_hdl Simple calculator service described at https://www.genivia.com/dev.html
//gsoap ns service protocol:	SOAP
//gsoap ns service style:	rpc
//gsoap ns service encoding:	encoded
//gsoap ns service namespace:	http://websrv.cs.fsu.edu/~engelen/gsm_mcu_hdl.wsdl
//gsoap ns service location:	http://websrv.cs.fsu.edu/~engelen/calcserver.cgi

//gsoap ns schema namespace:	urn:gsm_mcu_hdl

struct ns__gsm_mcu_rsp_t
{
	int result; // 0:succ -1:err
	int cnt;	// bit valid
	//unsigned long long value;
	//unsigned int *value;
	char *value;
	//unsigned int value[8];
	//unsigned char status[32];
};

//gsoap ns service method: gsm simcard enable
int ns__gsm_simcard_enable(int chn, int *result);

//gsoap ns service method: gsm simcard disable
int ns__gsm_simcard_disable(int chn, int *result);

//gsoap ns service method: get gsm simcard status
int ns__gsm_get_simcard_status(int chn, struct ns__gsm_mcu_rsp_t *result);

//gsoap ns service method: gsm module power on
int ns__gsm_module_power_on(int chn, int *result);

//gsoap ns service method: gsm module power off
int ns__gsm_module_power_off(int chn, int *result);

// gsoap ns service method: get gsm module power status
int ns__gsm_get_module_power_status(int chn, struct ns__gsm_mcu_rsp_t *result);

//gsoap ns service method: gsm module emerg off
int ns__gsm_module_emerg_off(int chn, int *result);

//gsoap ns service method: get gsm module emerg off status
//int ns__gsm_get_module_emerg_off_status(int chn, unsigned int *result);

//gsoap ns service method: gsm module on
int ns__gsm_module_on(int chn, int *result);

//gsoap ns service method: gsm module off
int ns__gsm_module_off(int chn, int *result);

//gsoap ns service method: get gsm module status
int ns__gsm_get_module_status(int chn, struct ns__gsm_mcu_rsp_t *result);

//gsoap ns service method: get gsm simcard insert status
int ns__gsm_get_simcard_insert_status(int chn, struct ns__gsm_mcu_rsp_t *result);

// gsoap ns service method: get gsm gsmboard insert status
int ns__gsm_get_gsmboard_insert_status(int chn, struct ns__gsm_mcu_rsp_t *result);

//gsoap ns service method: set led control
int ns__gsm_set_led_control(char *buf, char **result);

//gsoap ns service method: get mcu version
//int ns__gsm_get_mcu_version(struct ns__gsm_mcu_rsp_t *result);
int ns__gsm_get_mcu_version(char **result);

//gsoap ns service method: get mcu help
//int ns__gsm_get_mcu_help(struct ns__gsm_mcu_rsp_t *result);
int ns__gsm_get_mcu_help(char **result);

int ns__gsm_get_restore_event(char **result);

int ns__gsm_mcu_reg_read(int brd, int reg, int num, char **result);

int ns__gsm_mcu_reg_write(int brd, int reg, unsigned char val, char **result);

