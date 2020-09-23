/*
	soap_gsmio.h

	This is a gSOAP header file with a calculator data binding and Web
	service interface to implement clients and services

	The service operations and type definitions use familiar C/C++ syntax.

	The following methods are defined by this gSOAP service definition:

	gsmio_set_simcard_src(chn, src, result);
	gsmio_get_simcard_src(chn, result);
	gsmio_pwr_on(chn, result);
	gsmio_pwr_off(chn, result);
	gsmio_get_pwr_status(chn, result);
	gsmio_pwrkey_on(chn, result);
	gsmio_pwrkey_off(chn, result);
	gsmio_emerg_off(chn, result);
	gsmio_get_pwrkey_status(chn, result);
	gsmio_get_simcard_insert_status(chn, result);
	gsmio_get_gsmboard_insert_status(chn, result);
	gsmio_get_mcu_version(result);
	gsmio_get_mcu_help(result);

	Build steps for C (see samples/soap_gsmio):
	$ soapcpp2 -c -r soap_gsmio.h
	$ cc -o client client.c stdsoap2.c soapC.c soapClient.c
	$ cc -o server server.c stdsoap2.c soapC.c soapServer.c

	Build steps for C++ (see samples/soap_gsmio++):
	$ soapcpp2 -j -r soap_gsmio.h
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

//gsoap ns service name:	soap_gsmio Simple calculator service described at https://www.genivia.com/dev.html
//gsoap ns service protocol:	SOAP
//gsoap ns service style:	rpc
//gsoap ns service encoding:	encoded
//gsoap ns service namespace:	http://websrv.cs.fsu.edu/~engelen/soap_gsmio.wsdl
//gsoap ns service location:	http://websrv.cs.fsu.edu/~engelen/calcserver.cgi

//gsoap ns schema namespace:	urn:soap_gsmio

typedef struct ns__gsmio_rsp_s
{
	int result; // 0:succ -1:err
	int cnt;	// bit valid
	char *value;
}ns__gsmio_rsp_t;

//gsoap ns service method: set gsm simcard source
int ns__gsmio_set_simcard_src(int chn, int src, int *result);

//gsoap ns service method: get gsm simcard source
int ns__gsmio_get_simcard_src(int chn, ns__gsmio_rsp_t *result);

//gsoap ns service method: set gsm module power on
int ns__gsmio_pwr_on(int chn, int *result);

//gsoap ns service method: set gsm module power off
int ns__gsmio_pwr_off(int chn, int *result);

// gsoap ns service method: get gsm module power status
int ns__gsmio_get_pwr_status(int chn, ns__gsmio_rsp_t *result);

//gsoap ns service method: set gsm module pwrkey on
int ns__gsmio_pwrkey_on(int chn, int *result);

//gsoap ns service method: set gsm module pwrkey off
int ns__gsmio_pwrkey_off(int chn, int *result);

//gsoap ns service method: set gsm module emerg off
int ns__gsmio_emerg_off(int chn, int *result);

//gsoap ns service method: get gsm module status
int ns__gsmio_get_pwrkey_status(int chn, ns__gsmio_rsp_t *result);

//gsoap ns service method: get gsm simcard insert status
int ns__gsmio_get_simcard_insert_status(int chn, ns__gsmio_rsp_t *result);

// gsoap ns service method: get gsm gsmboard insert status
int ns__gsmio_get_gsmboard_insert_status(int chn, ns__gsmio_rsp_t *result);

//gsoap ns service method: get mcu version
//int ns__gsm_get_mcu_version(ns__gsmio_rsp_t *result);
int ns__gsmio_get_mcu_version(char **result);

//gsoap ns service method: get mcu help
//int ns__gsm_get_mcu_help(ns__gsmio_rsp_t *result);
int ns__gsmio_get_mcu_help(char **result);

