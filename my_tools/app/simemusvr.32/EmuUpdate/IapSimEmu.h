
#ifndef _IAP_SIMEMU_H
#define _IAP_SIMEMU_H
#include "IapBase.h"

#define  ACCEPTED_IAP_TOTAL                        2
#define  SET_MCU_RUNNING_MODE_MAX_TIMES            2


void *IapEmuThread(void *arg);

#endif