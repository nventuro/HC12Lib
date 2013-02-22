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
void dataReady_srv(void);

u8 buf[20];


void main (void)
{
	Init ();
	
	rti_Register(GetMeasurementsMask, NULL, RTI_MS_TO_TICKS(200), RTI_MS_TO_TICKS(0));
//	rti_Register(ReadWhoAmIMask, NULL, RTI_MS_TO_TICKS(1000), RTI_MS_TO_TICKS(500));
	
//	tim_GetTimer(TIM_IC, tim_ptr callback, tim_ptr overflow, tim_id timNumber);

	
	while (1)
		;
}

void Init (void)
{
	u16 i,j;
	for(i=0; i < 50000; i++)
		for(j=0; j < 40; j++)
			;

	iic_FlushBuffer();
//	tim_Init();
	
	// Modules that don't require interrupts to be enabled
	
	asm cli;
	
	// Modules that do require interrupts to be enabled
	rti_Init();
	iic_Init();
	dmu_Init();
	
	while (dmu_data.init == _FALSE)
		;
		
//	printf("Init done\n");
	
	iic_FreeBusReservation();

	
	return;
}

void GetMeasurementsMask(void *data, rti_time period, rti_id id)
{
	putchar('g');
	if (dmu_GetMeasurements() == _FALSE)
		putchar('f');
	else
		putchar('t');
	return;
}

#define BUFFER2PRINT 1

void ReadWhoAmIMask(void *data, rti_time period, rti_id id)
{
	putchar('w');
	dmu_ReceiveFromRegister(ADD_WHO_AM_I, printI2CData, dmu_ImFucked, BUFFER2PRINT, buf);
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
	{
		printf("%d %x\n",i, buf[i]);
		buf[i] = '\0';
	}
}


void dataReady_srv(void)
{
}

