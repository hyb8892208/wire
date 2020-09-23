#ifndef __VERSION_RECORD_H
#define __VERSION_RECORD_H

 typedef enum ERROR_CODE{
	SUCCESS=0,
	SYNC_ERR=-1,
	BEGIN_ERR=-2,
	LOAD_ERR=-3,
	DOWNLOAD_ERR=-4,
        END_ERR=-5, 
        RUN_ERR=-6, 
        INIT_ERR=-7,
        RESET_ERR=-8,
        PIPE_ERR=-9,
        COM_ERR=-10,
        UNKOWN =-20,
}ERROR_CODE_T;

void set_upgrade_channel(int channel);
void get_date(char * date);
void get_mod_version(int channel, int flag, char *version);
void get_mod_brd_version(char *version);
class module_info{
private:
	char m_oldVersion[64];
	char m_newVersion[64];
	char m_modVersion[128];
	char m_startTime[64];
	char m_endTime[64];
	int m_state;
	int m_channel;
	static module_info *module_info_t;
	module_info();
//	set_channel(int channel);
public:
	static module_info *getInstance();
	void set_old_version(char *oldVersion);
	void set_new_version(char *newVersion);
	void set_mod_version(char *moduleVersion);
	void set_start_time(char *startDate);
	void set_end_time(char *endDate);
	void set_upgrade_state(int state);
	int get_channel();
//	void module_reset(void);
	int record_info_to_file();
};


#endif
