
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
    ͨ����Ϣ
*/
struct rri_channel_info_t {
    char rf_module_model_name[RRI_NAME_LEN];
    char rf_module_hw_version[RRI_NAME_LEN];
    char rf_module_fw_version[RRI_NAME_LEN];
    char rf_module_manufacturer[RRI_NAME_LEN];
    unsigned char haveDebugPort;          // �Ƿ��ж����ĵ��Կ�
    unsigned char haveUpgradePort;        // �Ƿ��ж�����������
    char audioEndpointName_r[RRI_NAME_LEN];     // �����ڶ˵����� ֱ�����ַ�������ʽΪ [ͨ�ŷ�ʽ]://[��ַ]:[�˿�]    ���ӣ� pipe=pipe://pipename, udp = udp://172.16.0.3:0000, tcp=tcp://172.16.0.3:3000
    char audioEndpointName_w[RRI_NAME_LEN];
    char atEndpointName_r[RRI_NAME_LEN];        // at����ڶ˵�����
    char atEndpointName_w[RRI_NAME_LEN];
    char DebugEndpointName_r[RRI_NAME_LEN];     // Debug �ڵĶ˵���
    char DebugEndpointName_w[RRI_NAME_LEN];
    char UpgradeEndpointName_r[RRI_NAME_LEN];   // �����ڵĶ˵�����
    char UpgradeEndpointName_w[RRI_NAME_LEN];
};

/*
    ͨ����Ƶ����
*/
struct rri_voice_attri_t{
    int sampleRate;
    int samples_per_block;
};

/*
    atͨ������
*/
struct rri_at_attri_t{
	int result;
    int baudrate;
    unsigned char XON_XOFF;
};

/*
    ͨ������״̬
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
    ��ȡServer�İ汾��
*/
int GetServerVersion(struct rri_server_version_t *sv);

/*
    ��ȡͨ������
*/
int GetChannelCount(int* nChannels);

/*
    ��ȡͨ��������Ϣ
*/
int GetChannelInfo(int nCh, struct rri_channel_info_t *info);

/*
    ��ȡͨ����Ƶ����
*/
int GetChannelAudioFromat(int nCh, struct rri_voice_attri_t *va);

/*
    ��ȡatͨ������
    baudrateΪ�����ʣ����������server�Ĳ����ʣ����Է���0
    XON_XOFF��ʾ�Ƿ���Ҫ��XON_XOFF���أ�0��ʾ����Ҫ�� 1��ʾ��Ҫ
*/
int GetAtPortInfo(int nCh, struct rri_at_attri_t *attri);

/*
    ��ȡDebugͨ������
*/
int GetDebugPortInfo(int nCh, struct rri_at_attri_t *attri);

/*
    ��ȡUpgradeͨ������
*/
int GetUpgradePortInfo(int nCh, struct rri_at_attri_t *attri);

/*
    ��ȡͨ������״̬
    ����ֵ0��ʾͨ��Ϊfree, 1��ʾͨ���Ѿ������ӡ������Ҫ����״̬��������չ
*/
int GetChannelConnectionState(int nCh, struct rri_chn_conn_state_t* state);

/*
    ��ʼ/ֹͣ��������
    newState = 1 ����ʼ��newState = 0 ����ֹͣ
*/
int SetAudioTransmitState(int nCh, int newState);

/*
    ���� server debug ����
*/
int SetServerDebug(int nVal);

/*
    ���� ͨ�� debug ����
*/
int SetChannelDebug(int nCh, int nVal);

/*
    ���� ͨ�� ¼�� ����
*/
int SetChannelSndDebug(int nCh, int nVal);

/*
    ���´򿪴���
*/
int ReopenSerial(int nCh);
/*
 *  ����ͨ����������
*/
int SetChannelUpgrade(int nCh, int state);


int SetChannelTxSndSpeed(int nCh, int speed);

int GetChannelTxSndSpeed(int nCh, int *speed);

int SetChannelTxSndBufSize(int nCh, int bufsize);

int GetChannelTxSndBufSize(int nCh, int *bufsize);

int SetChannelTxSndDelay(int nCh, int delay);

int GetChannelTxSndDelay(int nCh, int *delay);
#endif

