#ifndef __FILE_H__
#define __FILE_H__

#include "platform_def.h"
#include "download.h"

extern byte *nprghex;
extern int nprghex_length;
extern byte *partition;
extern int partition_length;
extern byte *qcsblhd_cfgdata ;
extern int qcsblhd_cfgdata_length ;
extern byte *qcsbl ;
extern int qcsbl_length ;
extern byte *pbl ;
extern int pbl_length ;
extern byte *oemsblhd ;
extern int oemsblhd_length ;
extern byte *oemsbl ;
extern int oemsbl_length ;
extern byte *amsshd ;
extern int amsshd_length ;
extern byte *amss ;
extern int amss_length ;
extern byte *dbl ;
extern int dbl_length ;
extern byte *fsbl;
extern int fsbl_length ;
extern byte *osbl ;
extern int osbl_length;

extern int image_read(dload_cfg_type * pdload);
extern int image_size();
extern int image_close();
#endif /*__FILE_H__*/

