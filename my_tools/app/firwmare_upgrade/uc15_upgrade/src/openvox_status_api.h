#ifndef OPENVOX_STATUS_API_H
#define OPENVOX_STATUS_API_H

#ifdef __cplusplus
extern "C" {
#endif

//void *create_process_bar();

int set_total_size(int size);

int set_write_size(int size);

int process_par(char *str);

int set_channel(int channel);

int set_old_version();

int set_new_version();

int set_module_version();

int set_start_time();

int set_end_time();

int set_upgrade_state(int state);

int record_info_to_file();

#ifdef __cplusplus
}
#endif

#endif
