#include "mc9s12xdp512.h"
#include "common.h"
#include "usonic.h"
#include <stdio.h>
#include "rti.h"

void Init (void);
void PrintMeas(s32 meas);

void main (void)
{
	Init();
	
	while (1)
		usonic_Measure(PrintMeas);
}

void Init (void)
{
	// Modules that don't require interrupts to be enabled
	usonic_Init();
	
	asm cli;
	
	// Modules that do require interrupts to be enabled
	
	return;
}

void PrintMeas(s32 meas)
{
	if (meas == USONIC_INVALID_MEAS)
		printf("invalid\n");
	else
		printf("imprimo %ld\n",meas);
}
