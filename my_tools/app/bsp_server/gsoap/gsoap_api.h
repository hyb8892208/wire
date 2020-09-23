/*
	gsoap_api.h

	This is a gSOAP header file with a calculator data binding and Web
	service interface to implement clients and services

	The service operations and type definitions use familiar C/C++ syntax.

	Build steps for C (see samples/gsoap_api):
	$ soapcpp2 -c -r gsoap_api.h
	$ cc -o client client.c stdsoap2.c soapC.c soapClient.c
	$ cc -o server server.c stdsoap2.c soapC.c soapServer.c

	Build steps for C++ (see samples/gsoap_api++):
	$ soapcpp2 -j -r gsoap_api.h
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

#ifndef T_DESC
#define T_DESC(x) 1
#endif

/*
gsoap param out struct
*/
struct ns__gsoap_api_rsp_t
{
	int result;     // 0:succ -1:err
	int cnt;        // output param len(int byte)
    char *value;    // output param(note: string only)
};

#if T_DESC("simcard")
/*
desc      : enable simcard by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_enable(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : disable simcard by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_disable(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get simcard state(disable/enable) by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_enable_state_get(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get simcard state(insert/remove) by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_insert_state_get(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get simcard event(insert/remove) by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__simcard_insert_event_get(int chn, struct ns__gsoap_api_rsp_t *rsp);
#endif

#if T_DESC("module")
/*
desc      : turn on specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_turn_on(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : turn off specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_turn_off(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : reset specified module by channel
param in  : chn -- channel,  it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.10.26
*/
int ns__module_reset(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : power off specified module by channel ON TIMER
param in  : chn -- channel
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.11.07
*/
int ns__module_turn_on_timer(int chn, int timer, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get module state(turn on/turn off)
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_turn_on_state_get(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : power on specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_power_on(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : power off specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_power_off(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get module state(power on/power off)
param in  : chn -- channel, if (chn == -1), it means all channels
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__module_power_state_get(int chn, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get module number
param in  :
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.3.25
*/
int ns__module_num_get(struct ns__gsoap_api_rsp_t *rsp);


/*
desc      : get module uid
param in  : idx -- module index
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.3.25
*/
int ns__module_uid_get(int idx,  struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get module reset key status
param in  : index -- module index
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.3.25
*/
int ns__module_reset_status_get(int index, struct ns__gsoap_api_rsp_t *rsp );

/*
desc      : get module reset key status
param in  : index -- module index
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.4.17
*/
int ns__module_powerkey_hign_low( int chn, int level, char *id, struct ns__gsoap_api_rsp_t *rsp );
#endif

#if T_DESC("board mcu")
/*
desc      : get board mcu version info
param in  : 
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__brdmcu_version(struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get board mcu version info (internal api)
param in  : idx -- mcu id
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__brdmcu_int_version(unsigned int idx, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : read board mcu reg
param in  : brd -- board num(0,1)
            reg -- reg addr
            num -- reg num to read
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__brdmcu_reg_read(int brd, int reg, int num, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : read board mcu reg
param in  : brd -- board num(0,1)
            reg -- reg addr
            value -- value to write
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int ns__brdmcu_reg_write(int brd, int reg, unsigned char value, struct ns__gsoap_api_rsp_t *rsp);

#endif

#if T_DESC("board info")

/*
desc      : get board channel number
param in  : 
param out : rsp
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.12.20
*/
int ns__brdinfo_chn_num_get(struct ns__gsoap_api_rsp_t *rsp);


/*
desc      : get board name and version
param in  : 
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.3.25
*/
int ns__brdinfo_version(struct ns__gsoap_api_rsp_t *rsp);
#endif

#if T_DESC("upgrade")
/*
desc      : select upgrade channel
param in  : chn -- chanenels, if (chn == 0xFFFF), it means all channels unselect
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.3.25
*/
int ns__upgrade_chan_select(int chn, struct ns__gsoap_api_rsp_t *rsp);
/*
desc      : get upgrade channel status
param in  : chn -- chanenels
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.3.25
*/
int ns__upgrade_chan_status(int chn, struct ns__gsoap_api_rsp_t *rsp);

int ns__chan_upgrade_status_set(int chn, int flag, char *id, struct ns__gsoap_api_rsp_t *rsp);
#endif

#if T_DESC("debug")
int ns__bsp_server_debug_level(int value, struct ns__gsoap_api_rsp_t *rsp );
#endif

/*
desc      : set mod_brd debug uart switch
param in  : index -- module brd index
			enable -- 0 close debug uart
			enable -- 0 open  debug uart
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.3.25
*/
int ns__brd_set_debug_uart(int index, int enable, struct ns__gsoap_api_rsp_t *rsp );
/*
desc      : set channel work led status
param in  : chan -- channel, -1 means all channels
			SigOrWork -- signel led or work led
			status -- led status
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.08.03
*/

int ns__chan_led_set_sig( int chan, int status, struct ns__gsoap_api_rsp_t *rsp );

/*
desc      : set channel work led status
param in  : chan -- channel, -1 means all channels
            status -- led status
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.08.03
*/

int ns__chan_led_set_work( int chan, int status, struct ns__gsoap_api_rsp_t *rsp );

/*
desc      : set all channels led on or off
param in  : status -- 0 means off
                      1 means on
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.08.03
*/

int ns__chan_led_set_all(int status, struct ns__gsoap_api_rsp_t *rsp );

/*
desc      : set mod_brd system led on or off
param in  : status -- 0 means off
                      1 means on
            index  -- mod brd index
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.09.22
*/

int ns__mod_brd_led_set_sys(int index, int status, struct ns__gsoap_api_rsp_t *rsp);

/*
desc      : get systype
param out : rsp
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.08.03
*/

int ns__bsp_get_sys_type(struct ns__gsoap_api_rsp_t *rsp );

int ns__set_sim_card_slot(int chan, int slot, struct ns__gsoap_api_rsp_t *rsp );

int ns__get_sim_state_all(int chan, struct ns__gsoap_api_rsp_t *rsp );

int ns__get_sim_state_one(int chan, int card,struct ns__gsoap_api_rsp_t *rsp );

int ns__get_sim_version(int index, struct ns__gsoap_api_rsp_t *rsp);
