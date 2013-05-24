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

#include "arith.h"
#include "motors.h"

#define DMU_TIMER 0

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

void sample_ready(void)
{
//	printf ("che estoy en sample ready\n");

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
	volatile s32 a = S32_MIN>>1;
	Init ();	
	
	DDRA_DDRA0 = 1;
	DDRA_DDRA1 = 1;
	
	tim_GetTimer(TIM_IC, sample_ready, NULL, 0);
	tim_SetRisingEdge(0);
	tim_ClearFlag(0);
	tim_EnableInterrupts(0);
//	printf("%ld\n", DFRAC_1);
	while (1) {
		if (have_to_output) {
			have_to_output = 0;
			printf("%d %d %d %d,", QEst.r, QEst.v.x, 
					QEst.v.y, QEst.v.z);
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


struct motorData{

	u16 dutyTime[4];
};



int main(void)
{
	volatile frac f = FRAC_0_5;
	
	PLL_SPEED(BUS_CLOCK_MHZ);
	qs_init(0, MON12X_BR);
 
 	asm cli;
 	
 	mot_Init();
 	
	
	
	while(1);

}




/*
// MAIN de testeo para DMU. 
 void main (void)
 {
	int a;

	PLL_SPEED(BUS_CLOCK_MHZ);

	Init ();

//	DDRA = 0x01;



//	tim_GetTimer(TIM_IC, dataReady_Srv, NULL, DMU_TIMER);
//	tim_EnableInterrupts(DMU_TIMER);
//	tim_SetRisingEdge(DMU_TIMER); 



//	tim_GetTimer(TIM_IC, fifoOvf_Srv, NULL, DMU_TIMER);

//	tim_EnableInterrupts(DMU_TIMER);
//	tim_SetRisingEdge(DMU_TIMER); 

//	rti_Register(GetSamplesMask, NULL, RTI_MS_TO_TICKS(500), RTI_MS_TO_TICKS(

	rti_Register(GetMeasurementsMask, NULL, RTI_MS_TO_TICKS(500), RTI_MS_TO_TICKS(500));
 
 	while (1)
 		;
}
*/

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
	dmu_Init();

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
	static u8 count=0;

	if (tim_GetEdge(DMU_TIMER) == EDGE_RISING)
	{
		tim_SetFallingEdge(DMU_TIMER);	
		dmu_GetMeasurements(dmu_PrintFormattedMeasurements_WO);
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
