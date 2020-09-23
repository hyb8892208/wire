#include <stdio.h>

#include "bspsched.h"
#include "swg_device.h"
#include "bsp_tools.h"
static int maxsched = 0;

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

int mod_schedule_check(struct swg_mod_brd *mod)
{
	int res = 0;
	int x;

	for (x=0;x<CHAN_MOD_EVENT_NUM;x++) { /* [1, 127] */
		if (!mod->mod_sched[x].callback) {
			break;
		}
	}
	
	if (x == CHAN_MOD_EVENT_NUM) {
		printf( "No more room in scheduler\n");
		res = -1;
	}

	return res;
}

int mod_schedule_event(struct swg_mod_brd *mod, int ms, void (*function)(void *data), void *data)
{
	int x;
	struct timeval tv;
	int time_diff;
	int ms_s;
	int ms_usec;
	for (x=0;x<CHAN_MOD_EVENT_NUM;x++) { /* [1, 127] */
		if (!mod->mod_sched[x].callback) {
			break;
		}
	}
	
	if (x == CHAN_MOD_EVENT_NUM) {
		printf("No more room in scheduler\n");
		return -1;
	}

	if (x > maxsched) {
		maxsched = x; /* [1, 127] */
	}

	/* Get current time */
	gettimeofday(&tv, NULL);
	
	/* Get the schedule end time */
//	if(mod->event_flag == 1){
//		tv = mod->mod_sched[x].when;
//	}
	tv.tv_sec += ms / 1000;
	tv.tv_usec += (ms % 1000) * 1000;
	if (tv.tv_usec > 1000000) {
		tv.tv_usec -= 1000000;
		tv.tv_sec += 1;
	}

	/* Set schedule */
	mod->mod_sched[x].when = tv;			/* end time */
	mod->mod_sched[x].callback = function;	/* callback function */
	mod->mod_sched[x].data = data;			/* data */
	mod->mod_sched[x].delay = ms;
	/* return schedule id */
	return x;
}


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
struct timeval *mod_schedule_next(struct swg_mod_brd *mod)
{
	struct timeval *closest = NULL;
	int x;
	
	for (x=1;x<CHAN_MOD_EVENT_NUM;x++) {
		if ((mod->mod_sched[x].callback) && 
			(!closest || (closest->tv_sec > mod->mod_sched[x].when.tv_sec) || 
						 ((closest->tv_sec == mod->mod_sched[x].when.tv_sec) && 
							(closest->tv_usec > mod->mod_sched[x].when.tv_usec)))
		)
		closest = &mod->mod_sched[x].when;
	}
	
	return closest;
}


/******************************************************************************
 * run schedule
 * param:
 *		mod: struct swg_mod_brd
 *		tv: current time
 * return:
 *
 ******************************************************************************/
static int __mod_schedule_run(struct swg_mod_brd *mod, struct timeval *tv)
{
	int x;
	void (*callback)(void *);
	void *data;
	int delay = 0;
	for (x=0;x<CHAN_MOD_EVENT_NUM;x++) {
		delay = (mod->mod_sched[x].when.tv_sec - tv->tv_sec)*1000 + (mod->mod_sched[x].when.tv_usec - tv->tv_usec)/1000;
		if (mod->mod_sched[x].callback &&
			(((mod->mod_sched[x].when.tv_sec < tv->tv_sec) ||
			 ((mod->mod_sched[x].when.tv_sec == tv->tv_sec) &&
			  (mod->mod_sched[x].when.tv_usec <= tv->tv_usec)))||
			  (delay > (mod->mod_sched[x].delay >> 2)))){
			/* get callback and data */  
//			mod->schedev = 0;
			callback = mod->mod_sched[x].callback;
			data = mod->mod_sched[x].data;

			/* clear mod_sched info */
			mod->mod_sched[x].callback = NULL;
			mod->mod_sched[x].data = NULL;
			/* call schedule routin */
			callback(data);

		}
//		BSP_PRT(DBG, "call back event failid, when.tv_sec=%d, when.tv_usec = %d, tv.tv_sec=%d, tv.tv_usec = %d, delay=%d\n",
//												mod->mod_sched[x].when.tv_sec, mod->mod_sched[x].when.tv_usec,
//												tv->tv_sec, tv->tv_usec, delay);
	}
	return 0;
}


/******************************************************************************
 * run schedule
 * param:
 *		mod: struct swg_mod_brd
 * return:
 *		run state
 ******************************************************************************/
int mod_schedule_run(struct swg_mod_brd *mod)
{
	struct timeval tv;

	/* Get current time without tz */
	gettimeofday(&tv, NULL);

	/* run schedule */
	return __mod_schedule_run(mod, &tv);
}


/******************************************************************************
 * delete schedule
 * param:
 *		mod: struct swg_mod_brd
 * return:
 *		void
 * e.g.
 *		mod_schedule_del(mod, gsm->retranstimer);
 ******************************************************************************/
void mod_schedule_del(struct swg_mod_brd *mod,int id)
{
	if ((id >= CHAN_MOD_EVENT_NUM) || (id < 0)) { /* [0, 127] */
		printf( "Asked to delete sched id %d???\n", id);
	}
	
	mod->mod_sched[id].callback = NULL;
}



#ifdef mod_TASK
static void (*mod_task_lock_func)(int span);
static void (*mod_task_unlock_func)(int span);

void set_mod_task_lock_func(void (*lock_func)(int),void (*unlock_func)(int))
{
	mod_task_lock_func = lock_func;
	mod_task_unlock_func = unlock_func;
}

void mod_task_run(struct swg_mod_brd *mod)
{
	if(mod->state != mod_STATE_READY)
		return;

	mod_task_lock_func(mod->span);

	if(mod->mod_task_head) {
		if(mod->mod_task_head->callback) {
			if (mod->mod_task_head->data) {
				mod->mod_task_head->callback(gsm, gsm->gsm_task_head->data);
				free(mod->mod_task_head->data);
			}
		}

		struct mod_task *tmp;
		tmp = mod->mod_task_head->next;
		if (mod->mod_task_head == gsm->gsm_task_tail) {
			mod->mod_task_tail = NULL;
		}

		free(mod->mod_task_head);
		mod->mod_task_head = tmp;
	}

	mod_task_unlock_func(mod->span);
}

void mod_task_destroy(struct swg_mod_brd *mod)
{
	struct mod_task *task, *tmp;

	mod_task_lock_func(mod->span);
	task = mod->mod_task_head;

	while(task) {
		tmp = task;
		task = task->next;
		if (tmp->data) {
			free(tmp->data);
		}
		free(tmp);
	}

	mod->mod_task_head = NULL;
	mod->mod_task_tail = NULL;

	mod_task_unlock_func(mod->span);
}

static int has_same_task(struct swg_mod_brd *mod,void (*callback)(struct mod_modul *, void*))
{
	struct mod_task *task;

	mod_task_lock_func(mod->span);
	task = mod->mod_task_head;

	while(task) {
		if (task->callback == callback) {
			mod_task_unlock_func(mod->span);
			return 1;
		}
		task = task->next;
	}

	mod_task_unlock_func(mod->span);

	return 0;
}

void mod_task_add_task(struct swg_mod_brd *mod, void (*callback)(struct swg_channel *, void*), void *data)
{
	struct swg_mod_brd *new_task;

	if (has_same_task(mod,callback)) {
		//return;
	}

	new_task = (struct mod_task*)malloc(sizeof(struct mod_task));

	if (!new_task) {
		return;
	}

	mod_task_lock_func(mod->span);
	new_task->callback = callback;
	new_task->data = data;
	new_task->next = NULL;

	if(mod->mod_task_head && gsm->gsm_task_tail) {
		mod->mod_task_tail->next = new_task;
		mod->mod_task_tail = new_task;
	} else {
		mod->mod_task_head = gsm->gsm_task_tail = new_task;
	}
	mod_task_unlock_func(mod->span);
}

#endif
