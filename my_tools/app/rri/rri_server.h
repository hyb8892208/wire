
#ifndef __RRI_SERVER_H_
#define __RRI_SERVER_H_


#define BACKLOG				(100)
#define MAX_THR				(10)
#define MAX_QUEUE			(1000)

int soap_get_port(void);
int soap_server_start(void);
void soap_server_stop(void);

int svr_chan_voice_start(int hwport);
int svr_chan_voice_stop(int hwport);
unsigned int svr_get_hwport_num(void);
void svr_global_debug_set(int val);
int svr_chan_debug_set(int hwport, int val);
int svr_chan_debug_snd_set(int hwport, int val);
int module_group_com_reopen(int hwport);
int svr_get_pipe_names
(
    int hwport, 
    char *audio_r, 
    char *audio_w, 
    char *at_r, 
    char *at_w, 
    char *cmd_r, 
    char *cmd_w
);
int svr_get_at_port_info(
    int hwport,
    int *bound,
    unsigned char *x_on_off
);

int svr_get_debug_port_info(
    int hwport,
    int *bound,
    unsigned char *x_on_off
);

int svr_get_upgrade_port_info(
    int hwport,
    int *bound,
    unsigned char *x_on_off
);

int svr_get_channel_connect_state(
    int hwport,
    int *audioStatus,
    int *atStatus,
    int *debugStatus,
    int *upgradStatus
);

int svr_chan_upgrade_set(int hwport, int flag);

int svr_get_channel_txsnd_buf_size(int hwport, int *bufsize);

int svr_set_channel_txsnd_buf_size(int hwport, int bufsize);

int svr_get_channel_txsnd_speed(int hwport, int *speed);

int svr_set_channel_txsnd_speed(int hwport, int speed);

int svr_get_channel_txsnd_speed(int hwport, int *delay);

int svr_set_channel_txsnd_speed(int hwport, int delay);
#endif


