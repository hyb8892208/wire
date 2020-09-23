#ifndef __REDIS_IF_H__
#define __REDIS_IF_H__
#include "inc.h"

#define MAX_CHANNEL_COUNT		80
#define LICENSE_STATUS_UNLIMITED	"unlimited"
#define LICENSE_STATUS_VALID		"valid"
#define LICENSE_STATUS_NEARLY		"nearly"
#define LICENSE_STATUS_EXPIRED		"expired"


typedef struct _redis_sap_
{
	void (*redis_check_license)();
	
} redis_sap_t;

extern void redis_init(redis_sap_t *psap);
extern int get_redis_gsmreg_status(unsigned char status[]);
extern int redis_set_license_endtime(char *endtime);
extern int redis_set_license_status(char *status);

#endif

