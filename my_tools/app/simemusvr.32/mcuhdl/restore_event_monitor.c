#include "mcuhdl.h"
#include "czmq.h"
#include <regex.h>

#define GSOAP_CONN_HOST "127.0.0.1"
#define GSOAP_CONN_PORT 8808

int handle_gsm_get_restore_event(void)
{
    int ret = 0;
    gsm_mcu_hdl_t *hdl;

    hdl = gsm_mcu_hdl_init(GSOAP_CONN_HOST, GSOAP_CONN_PORT);

    if ( hdl == NULL ) 
    {
    	printf("gsm_get_restore_event: gsoap handle is null, init handle failed\n");
    	return -1;
    } 
    else 
    {
    	ret = gsm_get_restore_event(hdl);
    }

    gsm_mcu_hdl_uinit(hdl);

    return ret;
}

int main(int argc, char **argv)
{
    printf("restore event monitor start...\n");

    while ( 1 )
    {
    	sleep(2);

    	if ( handle_gsm_get_restore_event() == 1 )
    	{
			system("/my_tools/add_syslog \"Factory reset from mcuhdl.\" &");
    	    system("/my_tools/restore_cfg_file > /dev/null 2>&1 &");
    	    printf("restore cfg file\n");
    	    break;
    	}
    }

    printf("restore event monitor end\n");

    return 0;
}

