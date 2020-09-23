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

/*
gsoap param out struct
*/
struct ns__gsoap_api_rsp_t
{
	int result;     // 0:succ -1:err
	int cnt;        // output param len(int byte)
    char *value;    // output param(note: string only)
};

/*
    server version
*/
struct server_version_t{
    char *nName;
    int majorVersion;
    int minorVersion;
    int bugfixNumber;
    int buildNumber;
};

struct channel_info_t {
    char* rf_module_model_name;
    char* rf_module_hw_version;
    char* rf_module_fw_version;
    char* rf_module_manufacturer;
    unsigned char haveDebugPort;          // 是否有独立的调试口
    unsigned char haveUpgradePort;        // 是否有独立的升级口
    char* audioEndpointName_r;     // 语音口端点名字 直接用字符串，格式为 [通信方式]://[地址]:[端口]    例子： pipe=pipe://pipename, udp = udp://172.16.0.3:0000, tcp=tcp://172.16.0.3:3000
    char* audioEndpointName_w;
    char* atEndpointName_r;        // at命令口端点名字
    char* atEndpointName_w;
    char* DebugEndpointName_r;     // Debug 口的端点名
    char* DebugEndpointName_w;
    char* UpgradeEndpointName_r;   // 升级口的端点名。
    char* UpgradeEndpointName_w;
};

/*
    通道音频特性
*/
struct voice_attri_t{
    int sampleRate;
    int samples_per_block;
};

/*
    at通道特性
*/
struct at_attri_t{
    int result;
    int baudrate;
    unsigned char XON_XOFF;
};

/*
    通道连接状态
*/
struct chn_conn_state_t{
     int result;
     int audioStatus;
     int atStatus;
     int debugStatus;
     int upgradeStatus;
};

/*
    获取Server的版本号
*/
int ns__GetServerVersion(struct server_version_t *sv);

/*
    获取通道总数
*/
int ns__GetChannelCount(int* nChannels);

/*
    获取通道基本信息
*/
int ns__GetChannelInfo(int nCh, struct channel_info_t *info);

/*
    获取通道音频特性
*/
int ns__GetChannelAudioFromat(int nCh, struct voice_attri_t *va);

/*
    获取at通道特性
    baudrate为波特率，如果不关心server的波特率，可以返回0
    XON_XOFF表示是否需要用XON_XOFF流控，0表示不需要， 1表示需要
*/
int ns__GetAtPortInfo(int nCh, struct at_attri_t *attri);

/*
    获取Debug通道特性
*/
int ns__GetDebugPortInfo(int nCh, struct at_attri_t *attri);

/*
    获取Upgrade通道特性
*/
int ns__GetUpgradePortInfo(int nCh, struct at_attri_t *attri);

/*
    获取通道连接状态
    返回值0表示通道为free, 1表示通道已经被连接。如果需要其他状态，可以扩展
*/
int ns__GetChannelConnectionState(int nCh, struct chn_conn_state_t* state);

/*
    开始/停止语音传输
*/
int ns__SetAudioTransmitState(int nCh, int newState, struct ns__gsoap_api_rsp_t *rsp);

/*
    设置 server debug 开关
*/
int ns__SetServerDebug(int nVal, struct ns__gsoap_api_rsp_t *rsp);

/*
    设置 通道 at debug 开关
*/
int ns__SetChannelDebug(int nCh, int nVal, struct ns__gsoap_api_rsp_t *rsp);

/*
    重新打开串口
*/
int ns__ReopenSerial(int nCh, struct ns__gsoap_api_rsp_t *rsp);
/*
   设置通道升级开关
*/
int ns__SetChannelUpgrade(int nCh, int state, struct ns__gsoap_api_rsp_t *rsp);

/*
    设置 通道 snd debug 开关
*/
int ns__SetChannelSndDebug(int nCh, int nVal, struct ns__gsoap_api_rsp_t *rsp);

int ns__SetChannelTxSndSpeed( int nCh, int speed, struct ns__gsoap_api_rsp_t *rsp);

int ns__GetChannelTxSndSpeed( int nCh, struct ns__gsoap_api_rsp_t *rsp);

int ns__SetChannelTxSndBufSize( int nCh, int bufsize, struct ns__gsoap_api_rsp_t *rsp);

int ns__GetChannelTxSndBufSize( int nCh,  struct ns__gsoap_api_rsp_t *rsp);

int ns__SetChannelTxSndDelay( int nCh, int delay, struct ns__gsoap_api_rsp_t *rsp);

int ns__GetChannelTxSndDelay( int nCh,  struct ns__gsoap_api_rsp_t *rsp);
