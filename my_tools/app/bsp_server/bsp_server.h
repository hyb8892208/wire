/*
    bsp server м╥нд╪Ч
*/
#ifndef __BSP_SERVER_H_
#define __BSP_SERVER_H_

#define BACKLOG				(100)
#define MAX_THR				(10)
#define MAX_QUEUE			(1000)

int soap_get_port(void);
int soap_server_start(void);
void soap_server_stop(void);

#endif


