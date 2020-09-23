#ifndef __GEN_IMAGE_H__
#define __GEN_IMAGE_H__
#include <stdint.h>

#ifdef	__GNUC__
#define	PACKED	__attribute__((packed))
#elif defined STM32F042x6
#define PACKED __packed
#else
#error "We do not know how your compiler packs structures"
#endif

#define INFO \
"########################################################################\n" \
"#                   OpenVox IAP STM32 Rebuild Utility                  #\n" \
"#                                %s                                #\n"     \
"#                     OpenVox Communication Co.,Ltd                    #\n" \
"#        Copyright (c) 2009-2017 OpenVox. All Rights Reserved.         #\n" \
"########################################################################\n\n" \

#if 0
struct apdu{ 
	char tag[8];         // Erase flag 
	char major;        // Major Version
	char minor;        // Minor Version 
	char subminor;    //Subminor Version
	uint16_t build;     //Build 
	char mcu[24];           //mcu model
	char hw_type[16];     //hardware type
	char bin[32];           // STM32 bin
	uint32_t size;         // the STM32 bin size 
	uint8_t crc16[2];        //The crc16 of he STM32 bin 
}PACKED;
#endif

/* Total size = (32 + 32) Byte */
struct apdu{ 
	char tag[8];		/* Unique flag */
	uint32_t mcu;		/* MCU model */
	char hw_type[8];	/* Hardware desc */
	uint32_t size;		 /* App size */
	uint8_t crc16[2];	/* APP CRC16*/ 
	uint8_t major;		/* App major number */
	uint8_t minor;		/* App minor number */
	uint8_t subminor;		/* App subminor number */
	uint8_t bugfix;		/* App bugfix number */
	uint16_t build;		/* App uild number */
	char bin[32];           /* Firmware filename */
}PACKED;

typedef struct apdu apdu_t;

enum{
	ERR_NONE = 0,
	ERR_NOT_EXIST = -1,
	ERR_INVALID_PARAM = -2,
	ERR_INIT = -3,
};

/********************************************************************
  *
  *  apdu init/show/destroy operations
  *
  ********************************************************************/
struct apdu  *acl_init(struct apdu *apdu, char *filename);
void apdu_destory(struct apdu *apdu);
void apdu_show(struct apdu *apdu);

#endif

