#ifndef QUECTEL_COMM_H
#define QUECTEL_COMM_H

#define LOCKFILE	"/var/run/QFlash.pid"


enum usb_speed{
	usb_highspeed,
	usb_fullspeed,
	usb_superspeed
};


int q_port_detect(char** pp_diag_port, int interface);
int checkCPU();
int probe_quectel_speed(enum usb_speed* speed);
void strToLower(char* src);

int detect_adb();
int detect_diag_port();
int detect_diag_port(char **diag_port);
int detect_modem_port(char **modem_port);

int wait_diag_port_disconnect(int timeout /*s*/);
int wait_adb(int timeout);

int is_emergency_diag_port();

int detect_diag_port_timeout(int timeout);
int open_port_once(int ioflush);
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

int open_port_once(const char* dev);

void upgrade_process(int writesize,int size,int clear);
int already_running(const char *filename);

int show_user_group_name();
double get_now();


class transfer_statistics
{
public:
	static transfer_statistics* getInstance();

	void set_total(long long all_files_size/*kb*/);
	void set_write_bytes(long long transfer_bytes);
	int get_percent();
	void process_bar(char *result=NULL);
	
private:
	transfer_statistics();
	~transfer_statistics();
	transfer_statistics(const transfer_statistics&);
	transfer_statistics& operator=(const transfer_statistics&);
	
	static transfer_statistics* instance;
	long long m_all_files_bytes;			//all bytes
	long long m_transfer_bytes;				//current transfer bytes
	FILE * m_logfd;//Ê¸·ï¶çÊÁ
	int m_lastsize;//ºÇ¹¡°ì¼¡¼ÌÆþÅªÂç¾®
};

unsigned long get_file_size(const char* filename);

#endif
