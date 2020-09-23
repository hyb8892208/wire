#ifndef __CHN_UPGRADE_H
#define __CHN_UPGRADE_H

typedef enum tagPIPE_FD_EN
{       
    PIFD_AT_R = 0, /* read */
    PIFD_AT_W,     /* write */
    PIFD_AT_P,     /* pseudo */
    /*TTY_FD,     [> mcu device fd <]*/
    PIFD_NUM
}PIPE_FD_EN;

int m35f_process_update(int *fds, int channel, char *fw_path);

int sim6320c_process_update(int *fds, int channel, char *fw_path);

int ec20f_process_update(int *fds, int channel, char *fw_path);
#endif //__CHN_UPGRADE_H
