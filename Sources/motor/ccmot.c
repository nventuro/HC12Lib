#include <stdlib.h>
#include "derivative.h"
#include "pwm.h"
#include "ccmotor.h"


#define PH_ENA	PWME_PWME7
#define PH_IN1	PTM_PTM1
#define PH_IN2	PTM_PTM2
#define MOT_CLR 1
#define MOT_SET 0
#define MOT_CHAN 7
#define MOT_DISABLE 0
#define SPEED_LVLS 126
#define MOT_PWM_PER SPEED_LVLS
#define MOT_ENABLE MOT_PWM_PER

#define MOT_SET_DTY(n) pwm_set_dty(MOT_CHAN,n)

void mot_free_run (void)
{
	MOT_SET_DTY(MOT_DISABLE);
}

void mot_fast_break (void)
{
	MOT_SET_DTY(MOT_ENABLE);
	PH_IN1 = MOT_SET;
	PH_IN2 = MOT_SET;
}


void  mot_speed (char speed)
{
	int dty;
	
	if (abs(speed)>SPEED_LVLS)
		dty=SPEED_LVLS;
	else
		dty = abs(speed);
	 
	
	if (speed == 0)
	{
		mot_fast_break();
	} 
	else 
	{
		mot_free_run();
		PH_IN1 = MOT_CLR;
		PH_IN2 = MOT_CLR;
		if (speed > 0)
		{
			PH_IN1 = MOT_SET;
		}
		else 
		{
			
			PH_IN2 = MOT_SET;
		}
	
		MOT_SET_DTY(dty);
	}
}

void mot_init (void) 
{
	DDRM = 0xFF;
	pwm_set_clkpre (4, 4);
	pwm_set_per (MOT_CHAN, MOT_PWM_PER);
	MOT_SET_DTY(MOT_DISABLE);
	pwm_set_chan(MOT_CHAN, 1, PWM_NO_CHANGE, PWM_NO_CHANGE, PWM_NO_CHANGE);
	pwm_ctrl (MOT_CHAN,PWM_ON);
}
