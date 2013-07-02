#include "mc9s12xdp512.h"
#include "common.h"
#include "pll.h"
#include "quick_serial.h"
#include "nRF24L01+.h"

void Init (void);

void main (void)
{
	Init ();	
	
	while (1)
		;	
}

void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);

	// Modules that don't require interrupts to be enabled
	qs_init(0, MON12X_BR);
	
	asm cli;

	// Modules that do require interrupts to be enabled
	nrf_Init(PTX);

	return;
}
