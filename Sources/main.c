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
void dataReady_Srv(void);
void dataReady_Ovf(void);
void fifoOvf_Srv(void);

 
struct tim_channelData dmu_timerData = {0,0};

 
void main (void)
{	
	PLL_SPEED(BUS_CLOCK_MHZ);
		
	Init ();
//	tim_GetTimer(TIM_IC, dataReady_Srv, dataReady_Ovf, DMU_TIMER);
//	tim_GetTimer(TIM_IC, fifoOvf_Srv, NULL, DMU_TIMER);

//	tim_EnableInterrupts(DMU_TIMER);
//	tim_SetRisingEdge(DMU_TIMER); 

	
	rti_Register(GetMeasurementsMask, NULL, RTI_MS_TO_TICKS(1000), RTI_MS_TO_TICKS(500));

	while (1)
		;
}

void Init (void)
{
	u16 i,j;
	for(i=0; i < 50000; i++)
		for(j=0; j < 4; j++)
			;

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
	dmu_GetMeasurements();
	
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
	dmu_timerData.overflowCnt++;
	return;
}

void fifoOvf_Srv(void)
{	
	if (dmu_data.fifo.enable == _FALSE)
		return;
	
	dmu_data.fifo.enable = _FALSE;
	dmu_ReadFifo(dmu_PrintFifoMem);
	
	return;
}




