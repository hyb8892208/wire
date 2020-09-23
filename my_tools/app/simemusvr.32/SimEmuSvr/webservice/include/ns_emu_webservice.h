 
//gsoap ns service name: webproxy
//gsoap ns service protocol: SOAP
//gsoap ns service style: rpc
//gsoap ns service namespace: http://127.0.0.1:8868/webproxy.wsdl
//gsoap ns service location: http://127.0.0.1:8868
//gsoap ns service encoding: encoded
//gsoap ns schema namespace: urn:webproxy


int ns__EMULogSetting(int logclass,int comlogswitch,int *result);

int ns__EMUGetVersion(int EMU_No,char **version);


int ns__EMUResetSTM32(int EMU_No,int *result);


int ns__EMUGetMini52Version(int EMU_No,char **version);


int ns__EMUResetMini52(int EMU_No,int *result);

int ns__EMUGetDeviceStatus(int EMU_No,int *result);

int ns__EMUGetNetStatus(int EMU_No,int *result);

