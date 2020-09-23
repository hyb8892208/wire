#ifndef UC15_UPGRADE_H
#define UC15_UPGRADE_H

#define FILE_NAME_LEN (256)

struct firmware_file{
	char filename[FILE_NAME_LEN];
	int size;
	unsigned char *data;
};


#endif

