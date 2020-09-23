
#ifndef __RRI_API_H_
#define __RRI_API_H_

#define RRI_NAME_LEN    (64)

/*
gsoap param out struct
*/
struct rri_gsoap_api_rsp_t
{
	int result;     // 0:succ -1:err
	int cnt;        // output param len(int byte)
    char *value;    // output param(note: string only)
};

/*
    server version
*/
struct rri_server_version_t{
    char nName[RRI_NAME_LEN];
    int majorVersion;
    int minorVersion;
    int bugfixNumber;
    int buildNumber;
};

/*
    通道信息
*/
struct rri_channel_info_t {
    char rf_module_model_name[RRI_NAME_LEN];
    char rf_module_hw_version[RRI_NAME_LEN];
    char rf_module_fw_version[RRI_NAME_LEN];
    char rf_module_manufacturer[RRI_NAME_LEN];
    unsigned char haveDebugPort;          // 是否有独立的调试口
    unsigned char haveUpgradePort;        // 是否有独立的升级口
    char audioEndpointName_r[RRI_NAME_LEN];     // 语音口端点名字 直接用字符串，格式为 [通信方式]://[地址]:[端口]    例子： pipe=pipe://pipename, udp = udp://172.16.0.3:0000, tcp=tcp://172.16.0.3:3000
    char audioEndpointName_w[RRI_NAME_LEN];
    char atEndpointName_r[RRI_NAME_LEN];        // at命令口端点名字
    char atEndpointName_w[RRI_NAME_LEN];
    char DebugEndpointName_r[RRI_NAME_LEN];     // Debug 口的端点名
    char DebugEndpointName_w[RRI_NAME_LEN];
    char UpgradeEndpointName_r[RRI_NAME_LEN];   // 升级口的端点名。
    char UpgradeEndpointName_w[RRI_NAME_LEN];
};

/*
    通道音频特性
*/
struct rri_voice_attri_t{
    int sampleRate;
    int samples_per_block;
};

/*
    at通道特性
*/
struct rri_at_attri_t{
	int result;
    int baudrate;
    unsigned char XON_XOFF;
};

/*
    通道连接状态
*/
struct rri_chn_conn_state_t{
	 int result;
     int audioStatus;
     int atStatus; 
     int debugStatus; 
     int upgradeStatus;
};

int rri_api_init(char *host, unsigned short port);
void rri_api_deinit(void);

/*
    获取Server的版本号
*/
int GetServerVersion(struct rri_server_version_t *sv);

/*
    获取通道总数
*/
int GetChannelCount(int* nChannels);

/*
    获取通道基本信息
*/
int GetChannelInfo(int nCh, struct rri_channel_info_t *info);

/*
    获取通道音频特性
*/
int GetChannelAudioFromat(int nCh, struct rri_voice_attri_t *va);

/*
    获取at通道特性
    baudrate为波特率，如果不关心server的波特率，可以返回0
    XON_XOFF表示是否需要用XON_XOFF流控，0表示不需要， 1表示需要
*/
int GetAtPortInfo(int nCh, struct rri_at_attri_t *attri);

/*
    获取Debug通道特性
*/
int GetDebugPortInfo(int nCh, struct rri_at_attri_t *attri);

/*
    获取Upgrade通道特性
*/
int GetUpgradePortInfo(int nCh, struct rri_at_attri_t *attri);

/*
    获取通道连接状态
    返回值0表示通道为free, 1表示通道已经被连接。如果需要其他状态，可以扩展
*/
int GetChannelConnectionState(int nCh, struct rri_chn_conn_state_t* state);

/*
    开始/停止语音传输
    newState = 1 代表开始，newState = 0 代表停止
*/
int SetAudioTransmitState(int nCh, int newState);

/*
    设置 server debug 开关
*/
int SetServerDebug(int nVal);

/*
    设置 通道 debug 开关
*/
int SetChannelDebug(int nCh, int nVal);

/*
    设置 通道 录音 开关
*/
int SetChannelSndDebug(int nCh, int nVal);

/*
    重新打开串口
*/
int ReopenSerial(int nCh);
/*
 *  设置通道升级开关
*/
int SetChannelUpgrade(int nCh, int state);


int SetChannelTxSndSpeed(int nCh, int speed);

int GetChannelTxSndSpeed(int nCh, int *speed);

int SetChannelTxSndBufSize(int nCh, int bufsize);

int GetChannelTxSndBufSize(int nCh, int *bufsize);

int SetChannelTxSndDelay(int nCh, int delay);

int GetChannelTxSndDelay(int nCh, int *delay);
#endif

