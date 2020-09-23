#ifndef __MCU_HDL_H__
#define __MCU_HDL_H__


//#include "soapH.h"
//#include "gsm_mcu_hdl.nsmap"

//#include "czmq.h"
//#include <mcheck.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
//---------------------------------------------

#ifndef MAX_CHN
#define MAX_CHN			(32)
#endif

#define HDL_OK			(0)
#define HDL_ERR			(-1)

#define ALL_CHN			(-1)


//---------------------------------------------
#define LOCALHOST		"127.0.0.1"

//---------------------------------------------
/* = "http://localhost:8080"; to test against samples/webserver */



//---------------------------------------------
typedef struct gsm_mcu_hdl_s
{
	//struct soap *soap;
	void *soap;
	char host[16];
	unsigned short port;
	char url[128];
}gsm_mcu_hdl_t;


//---------------------------------------------
//struct soap * gsm_mcu_hdl_init(char *host, unsigned short port);
gsm_mcu_hdl_t * gsm_mcu_hdl_init(char *host, unsigned short port);

//void gsm_mcu_hdl_uinit(char *soap);
void gsm_mcu_hdl_uinit(gsm_mcu_hdl_t *hdl);


//gsoap ns service method: gsm simcard enable
// chn: -1,0~7
int gsm_simcard_enable(gsm_mcu_hdl_t *hdl, int chn);

//gsoap ns service method: gsm simcard disable
// chn: -1,0~7
int gsm_simcard_disable(gsm_mcu_hdl_t *hdl, int chn);

//gsoap ns service method: get gsm simcard status
// chn: -1,0~7
int gsm_get_simcard_status(gsm_mcu_hdl_t *hdl, int chn, char *result);

//gsoap ns service method: gsm module power on
int gsm_module_power_on(gsm_mcu_hdl_t *hdl, int chn);

//gsoap ns service method: gsm module power off
int gsm_module_power_off(gsm_mcu_hdl_t *hdl, int chn);

// gsoap ns service method: get gsm module power status
int gsm_get_module_power_status(gsm_mcu_hdl_t *hdl, int chn, char *result);

//gsoap ns service method: gsm module emerg off
int gsm_module_emerg_off(gsm_mcu_hdl_t *hdl, int chn);

//gsoap ns service method: gsm module on
int gsm_module_on(gsm_mcu_hdl_t *hdl, int chn);

//gsoap ns service method: gsm module off
int gsm_module_off(gsm_mcu_hdl_t *hdl, int chn);

//gsoap ns service method: get gsm module status
int gsm_get_module_status(gsm_mcu_hdl_t *hdl, int chn, char *result);

//gsoap ns service method: get gsm simcard insert status
int gsm_get_simcard_insert_status(gsm_mcu_hdl_t *hdl, int chn, char *result);

// gsoap ns service method: get gsm gsmboard insert status
int gsm_get_gsmboard_insert_status(gsm_mcu_hdl_t *hdl, int chn, char *result);

//gsoap ns service method: get mcu version
int gsm_get_mcu_version(gsm_mcu_hdl_t *hdl, char *result);

//gsoap ns service method: set led control
int gsm_set_led_control(gsm_mcu_hdl_t *hdl, char *buf, char *result);

int gsm_get_restore_event(gsm_mcu_hdl_t *hdl);

int gsm_mcu_reg_read(gsm_mcu_hdl_t *hdl, int brd, int reg, int num, char *result);

int gsm_mcu_reg_write(gsm_mcu_hdl_t *hdl, int brd, int reg, unsigned char val, char *result);


#endif
