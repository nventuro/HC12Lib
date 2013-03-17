#include "mc9s12xdp512.h"
#include "common.h"
#include "dmu.h"
#include "rti.h"
#include "timers.h"
#include <stdio.h>
#include "pll.h"
#include "quick_serial.h"

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
 
void main (void)
{
	int a;
	PLL_SPEED(BUS_CLOCK_MHZ);
	qs_init(0, MON12X_BR);

	DDRA = 0x01;
	
		
//	Init ();

//	tim_GetTimer(TIM_IC, dataReady_Srv, dataReady_Ovf, DMU_TIMER);

/*
	tim_GetTimer(TIM_IC, fifoOvf_Srv, NULL, DMU_TIMER);

	tim_EnableInterrupts(DMU_TIMER);
	tim_SetRisingEdge(DMU_TIMER); 
*/
/*
	asm cli;
	tim_Init();
	tim_GetTimer(TIM_OC, NULL, dataReady_Ovf, 0);
	tim_dSetOutputToggle(0);
	tim_dEnableInterrupts(0);
*/
//	rti_Register(GetSamplesMask, NULL, RTI_MS_TO_TICKS(500), RTI_MS_TO_TICKS(500));


	rti_Init();
	tim_Init();
	asm cli;

	tim_GetTimer(TIM_IC, icFcn, dataReady_Ovf, 0);
	tim_EnableOvfInterrupts(0);
	tim_EnableInterrupts(0);
	tim_SetRisingEdge(0);
	
	rti_Register(GetMeasurementsMask, NULL, 500, RTI_MS_TO_TICKS(500));


	printf("tomy la concha de tu madre\n");

	
	while (1)
		;
}

void Init (void)
{
	// Modules that don't require interrupts to be enabled
	iic_FlushBuffer();
	tim_Init();
	rti_Init();	
	qs_init(0, MON12X_BR);
	
	asm cli;
	
	// Modules that do require interrupts to be enabled
	iic_Init();
	dmu_Init();

	while (dmu_data.init == _FALSE)
		;
	printf("Init done\n");		
	
	
	return;
}

void GetMeasurementsMask(void *data, rti_time period, rti_id id)
{
	if (PORTA_PA0 == 1)
	{
		PORTA_PA0 = 0;
		tim_SetRisingEdge(0);
	}
	else
	{
		PORTA_PA0 = 1;
		tim_SetFallingEdge(0);
	}
		
//	dmu_GetMeasurements();
	
	return;
}

void GetSamplesMask(void *data, rti_time period, rti_id id)
{
	dmu_GetSamples();
	return;
}


void PrintMeas (s32 measurement)
{
	printf("%ld\n", measurement);
}


void dataReady_Srv(void)
{
	if (tim_GetEdge(DMU_TIMER) == EDGE_RISING)
	{
		tim_SetFallingEdge(DMU_TIMER);	
		dmu_GetMeasurements();
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
	dmu_ReadFifo(dmu_PrintFifoMem);
	
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
