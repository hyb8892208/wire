#ifndef __CALLMONITOR_CLI_H
#define __CALLMONITOR_CLI_H

struct callmonitor_chan_config{
	unsigned int total_call_dur;
	unsigned int total_call_times;
	unsigned int total_call_answers;
	time_t online_time;
	unsigned char handle_type;
};

struct callmonitor_chan_data{
	unsigned int cur_call_dur;
	unsigned int cur_call_times;
	unsigned int cur_call_answers;
	time_t last_online_time;
};

int callmonitor_reload_config();

int callmonitor_flush_status();

int callmonitor_get_chan_config(int chan_id, struct callmonitor_chan_config *config);

int callmonitor_get_chan_data(int chan_id, struct callmonitor_chan_data *data);


#endif


