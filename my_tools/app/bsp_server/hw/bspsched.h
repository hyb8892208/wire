#ifndef BSP_SCHED_H
#define BSP_SCHED_H

#include "bspsched.h"
#include "swg_device.h"


/* Scheduler routines */

/******************************************************************************
 * set schedule
 * param:
 *		mod: struct swg_mod_brd
 *		ms: delay (ms)
 *		function: set callback function 
 *		data: user data
 * return:
 *		int: schedule id (0 < id < MAX_SCHED)
 * e.g.
 *		mod->restart_timer = mod_schedule_event(gsm, 10000, em200_error_hard, gsm);
 ******************************************************************************/

int mod_schedule_check(struct swg_mod_brd *mod);

int mod_schedule_event(struct swg_mod_brd *mod, int ms, void (*function)(void *data), void *data);


/******************************************************************************
 * get next schedule time
 * param:
 *		mod: struct swg_mod_brd
 *		ms: delay (ms)
 *		function: set callback function 
 *		data: user data
 * return:
 *		int: schedule id (0 < id < MAX_SCHED)
 *		NULL: no schedule found
 * e.g.
 *		mod->restart_timer = mod_schedule_event(gsm, 10000, em200_error_hard, gsm);
 ******************************************************************************/
struct timeval *mod_schedule_next(struct swg_mod_brd *mod);


/******************************************************************************
 * run schedule
 * param:
 *		mod: struct swg_mod_brd
 * return:
 *		union mod_event
 ******************************************************************************/
int mod_schedule_run(struct swg_mod_brd *mod);


/******************************************************************************
 * delete schedule
 * param:
 *		mod: struct swg_mod_brd
 * return:
 *		void
 * e.g.
 *		mod_schedule_del(mod, gsm->retranstimer);
 ******************************************************************************/
void mod_schedule_del(struct swg_mod_brd *mod,int id);

#endif
