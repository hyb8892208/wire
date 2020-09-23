#ifndef __GSMIO_H__
#define __GSMIO_H__

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
#define HDL_MCU_ERR			(-2)

#define ALL_CHN			(-1)

#define SIMCARD_LOCAL	(1)
#define SIMCARD_REMOTE	(0)


#define MAX_CHN_PER_EMU		(8)
//---------------------------------------------
#define LOCALHOST		"127.0.0.1"
#define GSOAP_PORT		(8808)

//---------------------------------------------
/* = "http://localhost:8080"; to test against samples/webserver */



//---------------------------------------------
typedef struct gsmio_handle_s
{
	//struct soap *soap;
	void *soap;
	char host[16];
	unsigned short port;
	char url[128];
}gsmio_handle_t;


//---------------------------------------------
//struct soap * gsmio_open(char *host, unsigned short port);
gsmio_handle_t * gsmio_open(char *host, unsigned short port);

//void gsmio_close(char *soap);
void gsmio_close(gsmio_handle_t *handle);


//deprecated
//gsoap ns service method: set gsm simcard source
// chn: -1,0~7
// src:0:simserver; 1:local simcard
int gsmio_set_simcard_src(gsmio_handle_t *handle, int chn, int src);

//deprecated
//gsoap ns service method: get gsm simcard status
// chn: -1,0~7
// result: 0:simserver; 1:local simcard
int gsmio_get_simcard_src(gsmio_handle_t *handle, int chn, char *result);

//gsoap ns service method: set gsm module power on
int gsmio_pwr_on(gsmio_handle_t *handle, int chn);

//gsoap ns service method: set gsm module power off
int gsmio_pwr_off(gsmio_handle_t *handle, int chn);

// gsoap ns service method: get gsm module power status
int gsmio_get_pwr_status(gsmio_handle_t *handle, int chn, char *result);

//gsoap ns service method: set gsm module pwrkey on
int gsmio_pwrkey_on(gsmio_handle_t *handle, int chn);

//gsoap ns service method: set gsm module pwrkey off
int gsmio_pwrkey_off(gsmio_handle_t *handle, int chn);

//gsoap ns service method: set gsm module emerg off off
int gsmio_emerg_off(gsmio_handle_t *handle, int chn);

//gsoap ns service method: get gsm module status
int gsmio_get_pwrkey_status(gsmio_handle_t *handle, int chn, char *result);

//gsoap ns service method: get gsm simcard insert status
int gsmio_get_simcard_insert_status(gsmio_handle_t *handle, int chn, char *result);

// gsoap ns service method: get gsm gsmboard insert status
int gsmio_get_gsmboard_insert_status(gsmio_handle_t *handle, int chn, char *result);

//gsoap ns service method: get mcu version
int gsmio_get_mcu_version(gsmio_handle_t *handle, char *result);

//gsoap ns service method: get mcu help
//int gsmio_get_mcu_help(gsmio_handle_t *handle, char *result);



#endif
