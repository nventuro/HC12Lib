#include "mc9s12xdp512.h"
#include "common.h"
#include "dmu.h"
#include "rti.h"
#include "timers.h"
#include <stdio.h>


#define DMU_TIMER 0

extern struct dmu_data_T dmu_data;

void Init (void);
void PrintMeas (s32 measurement);
void GetMeasurementsMask(void *data, rti_time period, rti_id id);
void printI2CData(void);
void ReadWhoAmIMask(void *data, rti_time period, rti_id id);
void dataReady_Srv(void);
void dataReady_Ovf(void);


struct tim_channelData dmu_timerData = {0,0};

 
void main (void)
{
	Init ();
//	tim_GetTimer(TIM_IC, dataReady_Srv, dataReady_Ovf, DMU_TIMER);
//	tim_EnableInterrupts(DMU_TIMER);
//	tim_SetRisingEdge(DMU_TIMER); 

	
	rti_Register(GetMeasurementsMask, NULL, RTI_MS_TO_TICKS(200), RTI_MS_TO_TICKS(0));
	
//	tim_GetTimer(TIM_IC, tim_ptr callback, tim_ptr overflow, tim_id timNumber);

	
	while (1)
		;
}

void Init (void)
{
	u16 i,j;
	for(i=0; i < 50000; i++)
		for(j=0; j < 4; j++)
			;

	iic_FlushBuffer();
	tim_Init();
	
	
	// Modules that don't require interrupts to be enabled
	
	asm cli;
	
	// Modules that do require interrupts to be enabled
	rti_Init();
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

#define BUFFER2PRINT 1

void ReadWhoAmIMask(void *data, rti_time period, rti_id id)
{
	putchar('w');
	dmu_ReceiveFromRegister(ADD_WHO_AM_I, printI2CData, dmu_ImFucked, BUFFER2PRINT, NULL);
	return;
}


void PrintMeas (s32 measurement)
{
	printf("%ld\n", measurement);
}

void printI2CData(void)
{
	int i;
	for (i = 0; i < BUFFER2PRINT; i++)
		printf("%d %x\n",i, iic_commData.dataPtr[i]);
}


void dataReady_Srv(void)
{
	putchar('r');
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
	putchar('o');
	dmu_timerData.overflowCnt++;
	return;
}