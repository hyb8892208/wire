#ifndef _CALLLIMIT_CFG_H_
#define _CALLLIMIT_CFG_H_

#define     MAX_LINE                     4096 
#define     LINE_SIZE                    128
#define     NAME_SIZE                    256
#define     PATH_SIZE                    256

#define     HW_INFO_FILE                "/tmp/hw_info.cfg"

#define     CFG_FILE_PATH              "/etc/asterisk/gw/call_limit"
#define     SETTINGS_FILE               "/etc/asterisk/gw/call_limit/calllimit_settings.conf"
#define     STATUS_FILE                 "/etc/asterisk/gw/call_limit/calllimit_statues.conf"
#define     SIM_INFO_FILE              "/etc/asterisk/gw/call_limit/sim_info.conf"

#define     CFG_FILE_PATH_ORG           "/etc/cfg/gw/call_limit"
#define     SETTINGS_FILE_ORG               "/etc/cfg/gw/call_limit/calllimit_settings.conf"
#define     STATUS_FILE_ORG                 "/etc/cfg/gw/call_limit/calllimit_statues.conf"       


#define     STATUS_STR                  "[%d]\nday_call_limit_flag=%d\nhour_call_limit_flag=%d\nday_answer_limit_flag=%d\nday_total_calls=%d\nhour_total_calls=%d\nday_total_answers=%d\n"             

extern int get_sys_info_cfg(sys_info_cfg_t *sys_info_cfg);
extern int read_call_limit_cfg(calllimit_t *call_limit);
extern int reload_call_limit_cfg(calllimit_t *call_limit);
extern int refresh_call_limit_status_cfg(calllimit_t *call_limit, int channel_id);


#endif
