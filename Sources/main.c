 #include "mc9s12xdp512.h"
 #include "common.h"
#include "dmu.h"
#include "dmu_macros.h"
#include "rti.h"
#include "timers.h"
#include <stdio.h>
 #include "pll.h"
#include "quick_serial.h"
#include <limits.h>
#include "usonic.h"

#include "quad_control.h"

#define DMU_TIMER 1

extern struct dmu_data_T dmu_data;
 
void Init (void);
void PrintMeas (s32 measurement);
void GetMeasurementsMask(void *data, rti_time period, rti_id id);
void GetSamplesMask(void *data, rti_time period, rti_id id);
void dataReady_Srv(void);
void dataReady_Ovf(void);
void fifoOvf_Srv(void);
void icFcn(void);

 
struct tim_channelData dmu_timerData = {0,0};

u16 overflowCnt = 0;
u16 lastEdge = 0;

extern quat QEst;
extern void att_process(void);
extern bool have_to_output;

extern bool readyToCalculate;
extern struct motorData motData;

void sample_ready(void)
{
	if (tim_GetEdge(0) == EDGE_RISING) {
		tim_SetFallingEdge(0);
		dmu_GetMeasurements(att_process);
	} else {
		tim_SetRisingEdge(0);
	}
}

/* Main para control */

/*
void main (void)
{
	quat setpoint = {1,0,0,0};
	Init ();	
	
	DDRA_DDRA0 = 1;
	DDRA_DDRA1 = 1;
	
	tim_GetTimer(TIM_IC, sample_ready, NULL, 0);
	tim_SetRisingEdge(0);
	tim_ClearFlag(0);
	tim_EnableInterrupts(0);

	mot_Init();


	while (1) {
	
		frac thrust = 0;
		vec3 torque = {0,0,0};
		
		if (readyToCalculate){
			asm sei;
		
			thrust = h_control(0, 0);
			torque = adv_att_control(setpoint, QEst);
	
			motData = control_mixer(thrust, torque);
			
			
			printf("%d\n", thrust);
			
			readyToCalculate = _FALSE;
			
			asm cli;
		}
	}
}
*/


#define OC_PERIOD ((u8)62500)
#define TIM4_DUTY 14000
#define TIM5_DUTY 5000
#define TIM6_DUTY 10000
#define TIM7_DUTY 9375


void breakPoint_fcn(void)
{
	putchar('a');
}

/*
int main(void)
{
	volatile frac f = FRAC_0_5;
	
	PLL_SPEED(BUS_CLOCK_MHZ);
	qs_init(0, MON12X_BR);
 
 	asm cli;
 	
 	mot_Init();
 	
	
	
	while(1);

}

*/
void measure (s32 measurement);


// MAIN de testeo para DMU. 
 void main (void)
 {
	int a;
	char vel;

	PLL_SPEED(BUS_CLOCK_MHZ);

	Init ();

//	DDRA = 0x01;


/*
	tim_GetTimer(TIM_IC, dataReady_Srv, NULL, DMU_TIMER);
	tim_EnableInterrupts(DMU_TIMER);
	tim_SetRisingEdge(DMU_TIMER); 
*/

	mot_Init();


//	tim_GetTimer(TIM_IC, fifoOvf_Srv, NULL, DMU_TIMER);

//	tim_EnableInterrupts(DMU_TIMER);
//	tim_SetRisingEdge(DMU_TIMER); 

//	rti_Register(GetSamplesMask, NULL, RTI_MS_TO_TICKS(500), RTI_MS_TO_TICKS(

//	rti_Register(GetMeasurementsMask, NULL, RTI_MS_TO_TICKS(500), RTI_MS_TO_TICKS(500));
//	usonic_Measure(measure);
 	while (1)
 	{
 		vel = qs_getchar(0);
	 	if(vel == 'u')
	 	{
	 		motData.speed[0] += 300;
	 		motData.speed[1] += 300;
	 		motData.speed[2] += 300;
	 		motData.speed[3] += 300;
	 	}
	 	else if(vel == 'd')
	 	{
	 		motData.speed[0] -= 300;
	 		motData.speed[1] -= 300;
	 		motData.speed[2] -= 300;
	 		motData.speed[3] -= 300;
	 	}

 	}
 		
}

void measure (s32 measurement)
{
	if (measurement != USONIC_INVALID_MEAS)
		printf("Distance: %ld cm.\n",measurement);
	else
		printf("Invalid measurement.\n");
		
	 usonic_Measure(measure);
}

void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);

 	// Modules that don't require interrupts to be enabled
	tim_Init();
	rti_Init();	
	qs_init(0, MON12X_BR);
 
 	asm cli;
 
 	// Modules that do require interrupts to be enabled
	iic_Init();
//	dmu_Init();
//	usonic_Init();

	printf("Init done\n");		


	return;
}

void GetMeasurementsMask(void *data, rti_time period, rti_id id)
{
	dmu_GetMeasurements(dmu_PrintFormattedMeasurements_WO);	
	return;
}

void GetSamplesMask(void *data, rti_time period, rti_id id)
{
	dmu_FifoAverage(NULL);
	return;
}


void PrintMeas (s32 measurement)
{
	printf("%ld\n", measurement);
}


void dataReady_Srv(void)
{
	static u16 count=0;

	if (tim_GetEdge(DMU_TIMER) == EDGE_RISING)
	{
		tim_SetFallingEdge(DMU_TIMER);
		if (++count == 500)
		{
			dmu_GetMeasurements(dmu_PrintFormattedMeasurements);
			count = 0;
		}
	}
	else 
		tim_SetRisingEdge(DMU_TIMER);
}

void dataReady_Ovf(void)
{
//	dmu_timerData.overflowCnt++;
	overflowCnt++;

	return;
}

void fifoOvf_Srv(void)
{
//	printf("fifo ovf!!!\n");	


	if (dmu_data.fifo.enable == _FALSE)
		return;

	dmu_data.fifo.enable = _FALSE;
	dmu_ReadFifo(NULL);

//	dmu_FifoReset(NULL);

	return;
}



void icFcn()
{
	u32 time;

	time = tim_GetTimeElapsed(overflowCnt, 0, lastEdge);

	lastEdge = tim_GetValue(0);
	overflowCnt = 0;
 
	printf("t: %ld\n", time*TIM_TICK_NS);
 
 	return;
 }
