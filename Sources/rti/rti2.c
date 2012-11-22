/*
 * rti2.c
 * 
 * Control de la RTI
 * Lab de micros - 2012 - Grupo X
 *
 */

#include "rti2.h"
#include "derivative.h"
#include "common.h"
#include "asyn.h"

#define RTI_DIV (0x29)

#ifdef DEBUG

#define RTI_DEBUG_DD DDRE_DDRE3
#define RTI_DEBUG_PIN PORTE_PE3

#define RTI_DBG_INIT() (RTI_DEBUG_DD = 1)
#define RTI_DBG_ENTER() (RTI_DEBUG_PIN = 1)
#define RTI_DBG_LEAVE() (RTI_DEBUG_PIN = 0)

#else /* not DEBUG*/

#define RTI_DBG_INIT()
#define RTI_DBG_ENTER()
#define RTI_DBG_LEAVE()

#endif /*DEBUG*/

#ifdef RTI_COMPAT

#define RTI_MAX_FCNS 20

struct rti_cb {
	rti_time period;
	rti_time cnt;
	rti_time (*f)(void *, rti_time);
	int protect;
	void *data;
};

static struct rti_cb rti_tbl[RTI_MAX_FCNS];

#endif /*RTI_COMPAT */

extern struct rti_srv rti_srv_tbl[];

/* ****** ** IMPORTANTE * ***** */
void rti_reenable(void)
{
	CRGFLG_RTIF = 1;
}

/* ****** ** ---------- * ****** */

int rti_ll_timer_run_nonstop(struct rti_timer *ti)
{
	int g;

	if (ti->cnt) {
		ti->cnt--;
		g = 0;
	} else {
		ti->cnt = ti->period - 1;
		g = 1;
	}
	
	return g;
}

void rti_timer_reload(struct rti_timer *ti)
{
	ti->cnt = ti->period;
}

static void process_srv(struct rti_srv *srv)
{
	if (rti_ll_timer_run_nonstop(&srv->osc) && srv->enabled) {
		if (srv->ramp_cnt == 0 || srv->ramp_cnt == srv->ramp_ths)
			srv->ramp_ths = srv->f(srv->data, srv->ramp_cnt);
		
		if ((++(srv->ramp_cnt)) >= srv->ramp_steps)
			srv->ramp_cnt = 0;
	}
}

void rti_init()
{
	int i;
	
	RTI_DBG_INIT();

#ifdef RTI_COMPAT
	for (i = 0; i < RTI_MAX_FCNS; i++)
		rti_tbl[i].f = NULL;
#endif /*RTI_COMPAT */
	
	RTICTL = RTI_DIV;
	CRGINT_RTIE = 1;
	rti_reenable();
}

void rti_enable(rti_srvid s, int v)
{
	
	rti_srv_tbl[s].enabled = v;
	
	//int *ds = &(rti_srv_tbl[s].enabled);
	
	//ATOMIC_SETPW(ds, v);
}

void rti_runctl(rti_srvid s, int v)
{
	if (!v) {
		rti_srv_tbl[s].stop_req = 1;
	} else {
		rti_srv_tbl[s].stop_req = 0;
		rti_enable(s, 1);
	}
}

void rti_set_period(rti_srvid s, int pw)
{
	
	//asm sei;
	rti_srv_tbl[s].ramp_steps = pw;
	//asm cli;
	
	//int *ds = &(rti_srv_tbl[s].ramp_steps)
	
	//ATOMIC_SETW(ds, pw);
}

#ifdef RTI_COMPAT
timer_id rti_register2(rti_time (*f)(void *, rti_time), void *data, 
				rti_time period, rti_time delay, int protect)
{
	int i;
	
	for (i = 0; i < RTI_MAX_FCNS; i++) {
		if (rti_tbl[i].f == NULL) {
			rti_tbl[i].f = f;
			rti_tbl[i].data = data;
			rti_tbl[i].period = period;
			rti_tbl[i].protect = protect;
			rti_tbl[i].cnt = delay;
			break;
		}
	}
		
	if (i == RTI_MAX_FCNS)
		i = -E_NOMEM;
	
	return i;
}

timer_id rti_register(rti_time (*f)(void *, rti_time), void *data, 
						rti_time period, rti_time delay)
{
	return rti_register2(f, data, period, delay, RTI_DEFAULT_PROTECT);
}
/*
rti_time rti_set_period(int n, rti_time period)
{
	rti_time old_p = rti_tbl[n].period;
	
	rti_tbl[n].period = period;
	
	return old_p;
}
*/
void rti_cancel(timer_id n)
{
	rti_tbl[n].protect = RTI_NORMAL;
	rti_tbl[n].cnt = RTI_CANCEL;
}

#endif /*RTI_COMPAT */

void interrupt rti_srv(void)
{
	int i;

	RTI_DBG_ENTER();

#ifdef RTI_COMPAT
	for (i = 0; i < RTI_MAX_FCNS; i++) {
		if (rti_tbl[i].f != NULL) {
			if (rti_tbl[i].cnt == RTI_AUTOCANCEL) {
				if (!rti_tbl[i].protect == RTI_PROTECT)
					rti_tbl[i].f = NULL;
			} else {
				if(!(--rti_tbl[i].cnt)) {
					rti_tbl[i].period = rti_tbl[i].f(rti_tbl[i].data, rti_tbl[i].period);
					rti_tbl[i].cnt = rti_tbl[i].period;
				}
			}
		}
	}
#endif /*RTI_COMPAT */
	for (i = 0; i < RTI_N_SRVS; i++) {
		process_srv(rti_srv_tbl + i);
	}
	
	RTI_DBG_LEAVE();
	rti_reenable();
}

/* funciones convenientes */
#ifdef RTI_COMPAT
rti_time flagger(void *flag, rti_time pw)
{
	*((u8*)flag) = 1;
	
	return pw;
}
#endif /*RTI_COMPAT */
