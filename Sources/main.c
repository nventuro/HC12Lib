#include "mc9s12xdp512.h"
#include "common.h"
#include "usonic.h"
#include "lcd.h"
#include <stdio.h>

extern struct dmu_data;


void Init (void);
void PrintMeas (s32 measurement);

void main (void)
{
	Init ();
	
	while (1)
		usonic_Measure (PrintMeas);
}

void Init (void)
{
	// Modules that don't require interrupts to be enabled
	
	asm cli;
	
	// Modules that do require interrupts to be enabled
	iic_Init();
	dmu_Init();
	
	while (dmu_data.init == _FALSE)
		;
	printf("Init done\n");
	
	return;
}

void PrintMeas (s32 measurement)
{
	printf("%ld\n", measurement);
}
