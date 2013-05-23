#include "timers.h"
#include "error.h"


enum {PIN_LOW=0, PIN_HIGH};

#define MOTOR_SLAVE1_OC 4
#define MOTOR_SLAVE2_OC 5
#define MOTOR_SLAVE3_OC 6
#define MOTOR_MASTER_OC 7
#define MOT_PERIOD 9375		// 20 ms with TIMER_PRESCALER = 4

#define MOT_LINK_MASK ((1 << MOTOR_SLAVE1_OC) | (1 << MOTOR_SLAVE2_OC) | \
			(1 << MOTOR_SLAVE3_OC) | (1 << MOTOR_MASTER_OC))


struct motorData{

	u16 duty[4];
};

void mot_SlaveErr(void);
void mot_MasterSrv(void);

extern struct motorData control(void);

// Nota: los linkeos se pueden hacer más rápido si se utiliza MOT_LINK_MASK directamente en vez de llamar
// a la función.

void mot_Init(void) 
{
	tim_id timerId[4];
	
	timerId[0] = tim_GetTimer(TIM_OC, mot_SlaveErr, NULL, MOTOR_SLAVE1_OC);	
	timerId[1] = tim_GetTimer(TIM_OC, mot_SlaveErr, NULL, MOTOR_SLAVE2_OC);
	timerId[2] = tim_GetTimer(TIM_OC, mot_SlaveErr, NULL, MOTOR_SLAVE3_OC);
	timerId[3] = tim_GetTimer(TIM_OC, mot_MasterSrv, NULL, MOTOR_MASTER_OC);

	if ((timerId[0] < 0) || (timerId[1] < 0) || (timerId[2] < 0) || (timerId[3] < 0))
		err_Throw("Timers for motors already in use.");
	
	tim_SetOutputLow(MOTOR_SLAVE1_OC);
	tim_SetOutputLow(MOTOR_SLAVE2_OC);
	tim_SetOutputLow(MOTOR_SLAVE3_OC);

	tim7_dLinkTimer(MOT_LINK_MASK, MOT_LINK_MASK);
	tim_SetOutputToggle(MOTOR_MASTER_OC);
	tim_EnableInterrupts(MOTOR_MASTER_OC);

	return;
}


void mot_MasterSrv(void)
{
	static u16 latchedTime;
	static struct motorData motData = { {0,0,0,0} };

	
	if ( PTT_PTT7 == PIN_HIGH)
	{		 
		latchedTime = tim_GetValue(MOTOR_MASTER_OC);
		
		tim_SetValue(MOTOR_MASTER_OC, latchedTime + motData.duty[0]);
		tim_SetValue(MOTOR_SLAVE1_OC, latchedTime + motData.duty[1]);
		tim_SetValue(MOTOR_SLAVE2_OC, latchedTime + motData.duty[2]);
		tim_SetValue(MOTOR_SLAVE3_OC, latchedTime + motData.duty[3]);
		
		tim7_dUnlinkTimer(MOT_LINK_MASK);
		
		motData = control();
	}
	else
	{
		tim_SetValue(MOTOR_MASTER_OC, latchedTime + MOT_PERIOD);
		tim7_dLinkTimer(MOT_LINK_MASK, MOT_LINK_MASK);
	}
	
	return;
}

void mot_SlaveErr(void) 
{
	err_Throw("A slave motor has interrupted.");
	return;
}
