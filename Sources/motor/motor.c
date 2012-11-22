/*
 * motor.c
 * 
 * Lab de micros - Grupo 2 - 2012
 */

#include <stdlib.h>
#include "common.h"
#include "derivative.h"
#include "rti2.h"
#include "rti_srvs.h"
#include "asyn.h"
#include "motor.h"
#include "spi.h"

#define PMOD 8
#define SMOT_OFF 0xFF

#define SMOT_PORT PTH
#define SMOT_DDR DDRH
#define SMOT_PMASK 0x0F
#define SMOT_LL_STEP 1
#define SM_ACTION_RATE 2

//#define MOT_LL_WRITE(x) MASKED_WRITE(SMOT_PORT, SMOT_PMASK, (x))
#define MOT_LL_WRITE(x) spi_fast_rxtx(x)

struct smot_sett {
	u8 p;
	int mode;
	int setpoint;
	int position;
	int auto_off;
	int direc;
};

const struct smot_sett smdefaults = {0, SMOT_FULL, 0, 0, 1, MOT_STOP};

static struct smot_sett smot1;

void smot_passive(void)
{
	MOT_LL_WRITE(SMOT_OFF);
}

void smot_stop(void)
{
	smot1.direc = MOT_STOP;
}

void smot_init(int auto_off)
{
	smot1 = smdefaults;
	smot1.auto_off = auto_off;
	
//	spi_init();
	
//	MASKED_WRITE(SMOT_DDR, SMOT_PMASK, SMOT_PMASK);
	smot_passive();
	
	rti_enable(SRV_SMOT, 1);
}

#define M_STEP(n, v)  (((v) > 0)? (n) : -(n))

u8 smot_step(int dir)
{
	u8 mv;
	int pp;
	
	if (smot1.mode == SMOT_HALF) {
		smot1.p += M_STEP(1, dir);
		pp = smot1.p;
	} else {
		smot1.p += M_STEP(2, dir);
		pp = smot1.p/2;
	}
	
	switch (smot1.mode) {
		case SMOT_WAVE:
			mv = 1<<(pp%4);
			smot1.position = (smot1.position/2)*2 + M_STEP(2, dir);
			break;
		case SMOT_FULL:
			mv = (1<<((pp)%4)) | (1<<(((pp+1))%4));
			smot1.position = (smot1.position/2)*2 + M_STEP(2, dir);
			break;
		case SMOT_HALF:
			mv = (1<<((pp/2)%4)) | (1<<(((pp+1)/2)%4));
			smot1.position += M_STEP(1, dir);
			break;
	}
	
	return ~mv;
}

void _smot_speed(int ppm, int setdir)
{
	int rs;
	
	if (ppm)
		rs = ((long long)((60/SM_ACTION_RATE) * RTI_FREQ)) / (abs(ppm));
	
	CRIT_ENTER();
	if (setdir)
		smot1.direc = (ppm == 0)? MOT_STOP : ((ppm > 0)? MOT_FORWARD : MOT_BACKWARD);
	if (ppm)
		rti_set_period(SRV_SMOT, rs);
	CRIT_LEAVE();
}

void smot_speed(int ppm)
{
	_smot_speed(ppm, 1);
}

void smot_steprate(int ppm)
{
	_smot_speed(ppm, 0);
}

void smot_reset(void)
{
	smot1.position = 0;
	smot1.setpoint = 0;
}

void smot_setpoint(int inc)
{ /* NO hacer esto mientras el motor está andando !!!!! */
/*	int *d = smot1.setpoint, n;
	
	ATOMIC_SETW(n, d)
	n += inc;
	ATOMIC_SETW(d, n)
*/
	smot1.setpoint += inc;
}

void smot_setmode(int mode)
{
	smot1.mode = mode;
}

int smot_srv(void *data, int t)
{
	int sp;
	
	if (!t) {
		smot1.setpoint += smot1.direc;
	}
	
	if (t && smot1.auto_off) {
		smot_stop();
		t = 0;
	} else if (!t) {
		int err = smot1.setpoint - smot1.position;
		int tol = (smot1.direc == SMOT_HALF)? 0 : 1;
		if (abs(err) > tol) {
			u8 cmd = smot_step(err);
			MOT_LL_WRITE(cmd);
		}
		t = SMOT_LL_STEP;
	}
	
//	if (smot1.position == smot1.setpoint) {
//		smot_reset();
//	}
	
	return t;
}

int smot_getdir(void)
{
	return smot1.direc;
}

int smot_getpos(void)
{
	return smot1.position;
}

void smot_spreset()
{
	CRIT_ENTER();
	smot1.setpoint = smot1.position;
	CRIT_LEAVE();
}