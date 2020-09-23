#ifndef	 __IAP_UPGRADE_H__
#define	 __IAP_UPGRADE_H__

#define IAP_INFO \
"########################################################################\n" \
"#                   OpenVox IAP STM32 Update Utility                  #\n" \
"#                                %s                                #\n"     \
"#                     OpenVox Communication Co.,Ltd                    #\n" \
"#        Copyright (c) 2009-2017 OpenVox. All Rights Reserved.         #\n" \
"########################################################################\n" \
"Note: Do not turn power OFF while the update process is continuing.\n"      \
"      When the update is completed, the system will reboot.\n\n"



typedef struct opts {
	char *mode;
	const char *input;
	const char *output;
	int speed;
}opts_t;


enum OP_RES{
	ERR_OPEN_FAILED = 1,
	ERR_INVALIDED_PARAM,
	ERR_FILE_NOT_EXIST,
	ERR_PERMISSION,

};


static void get_welcome_info(void) ;
const char *iap_get_version(void);
const char *iap_show_version(void);
void iap_interrupt(int sign);

#endif

