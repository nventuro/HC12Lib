#include "mc9s12xdp512.h"
#include "common.h"
#include "pll.h"
#include "quick_serial.h"
#include "nRF24L01+.h"
#include <stdio.h>

void Init (void);
void nrf_Callback (u8 *data, u8 length);

void main (void)
{
	Init ();
		
	nrf_Receive(nrf_Callback);
	
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
	nrf_Init(PRX);

	return;
}

void nrf_Callback (u8 *data, u8 length)
{
	u8 index;
	for (index = 0; index < length; index++)
		printf("%d\t",((s16)(data[index])));
	putchar('\n');
}
	