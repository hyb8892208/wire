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
    unsigned char haveDebugPort;          // �Ƿ��ж����ĵ��Կ�
    unsigned char haveUpgradePort;        // �Ƿ��ж�����������
    char* audioEndpointName_r;     // �����ڶ˵����� ֱ�����ַ�������ʽΪ [ͨ�ŷ�ʽ]://[��ַ]:[�˿�]    ���ӣ� pipe=pipe://pipename, udp = udp://172.16.0.3:0000, tcp=tcp://172.16.0.3:3000
    char* audioEndpointName_w;
    char* atEndpointName_r;        // at����ڶ˵�����
    char* atEndpointName_w;
    char* DebugEndpointName_r;     // Debug �ڵĶ˵���
    char* DebugEndpointName_w;
    char* UpgradeEndpointName_r;   // �����ڵĶ˵�����
    char* UpgradeEndpointName_w;
};

/*
    ͨ����Ƶ����
*/
struct voice_attri_t{
    int sampleRate;
    int samples_per_block;
};

/*
    atͨ������
*/
struct at_attri_t{
    int result;
    int baudrate;
    unsigned char XON_XOFF;
};

/*
    ͨ������״̬
*/
struct chn_conn_state_t{
     int result;
     int audioStatus;
     int atStatus;
     int debugStatus;
     int upgradeStatus;
};

/*
    ��ȡServer�İ汾��
*/
int ns__GetServerVersion(struct server_version_t *sv);

/*
    ��ȡͨ������
*/
int ns__GetChannelCount(int* nChannels);

/*
    ��ȡͨ��������Ϣ
*/
int ns__GetChannelInfo(int nCh, struct channel_info_t *info);

/*
    ��ȡͨ����Ƶ����
*/
int ns__GetChannelAudioFromat(int nCh, struct voice_attri_t *va);

/*
    ��ȡatͨ������
    baudrateΪ�����ʣ����������server�Ĳ����ʣ����Է���0
    XON_XOFF��ʾ�Ƿ���Ҫ��XON_XOFF���أ�0��ʾ����Ҫ�� 1��ʾ��Ҫ
*/
int ns__GetAtPortInfo(int nCh, struct at_attri_t *attri);

/*
    ��ȡDebugͨ������
*/
int ns__GetDebugPortInfo(int nCh, struct at_attri_t *attri);

/*
    ��ȡUpgradeͨ������
*/
int ns__GetUpgradePortInfo(int nCh, struct at_attri_t *attri);

/*
    ��ȡͨ������״̬
    ����ֵ0��ʾͨ��Ϊfree, 1��ʾͨ���Ѿ������ӡ������Ҫ����״̬��������չ
*/
int ns__GetChannelConnectionState(int nCh, struct chn_conn_state_t* state);

/*
    ��ʼ/ֹͣ��������
*/
int ns__SetAudioTransmitState(int nCh, int newState, struct ns__gsoap_api_rsp_t *rsp);

/*
    ���� server debug ����
*/
int ns__SetServerDebug(int nVal, struct ns__gsoap_api_rsp_t *rsp);

/*
    ���� ͨ�� at debug ����
*/
int ns__SetChannelDebug(int nCh, int nVal, struct ns__gsoap_api_rsp_t *rsp);

/*
    ���´򿪴���
*/
int ns__ReopenSerial(int nCh, struct ns__gsoap_api_rsp_t *rsp);
/*
   ����ͨ����������
*/
int ns__SetChannelUpgrade(int nCh, int state, struct ns__gsoap_api_rsp_t *rsp);

/*
    ���� ͨ�� snd debug ����
*/
int ns__SetChannelSndDebug(int nCh, int nVal, struct ns__gsoap_api_rsp_t *rsp);

int ns__SetChannelTxSndSpeed( int nCh, int speed, struct ns__gsoap_api_rsp_t *rsp);

int ns__GetChannelTxSndSpeed( int nCh, struct ns__gsoap_api_rsp_t *rsp);

int ns__SetChannelTxSndBufSize( int nCh, int bufsize, struct ns__gsoap_api_rsp_t *rsp);

int ns__GetChannelTxSndBufSize( int nCh,  struct ns__gsoap_api_rsp_t *rsp);

int ns__SetChannelTxSndDelay( int nCh, int delay, struct ns__gsoap_api_rsp_t *rsp);

int ns__GetChannelTxSndDelay( int nCh,  struct ns__gsoap_api_rsp_t *rsp);
