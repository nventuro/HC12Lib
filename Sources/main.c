#include "mc9s12xdp512.h"
#include "common.h"
#include "pll.h"
#include "rftx.h"
#include <stdio.h>
#include "quick_serial.h"

void Init (void);

void tx (void);
u8 data[4];

void main (void)
{	
	u32 a;

	Init ();

	data[0] = 0xFF;
	data[1] = 0x00;
	data[2] = 0xF0;
	data[3] = 0x0F;
	
	for (a = 0; a < 20000; a++)
		;
		
	rftx_Send(5, data, 4*8, tx);

	while (1)
		;
}

void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);

	// Modules that don't require interrupts to be enabled
	qs_init(0,MON12X_BR);
	asm cli;

	// Modules that do require interrupts to be enabled
	rftx_Init(_TRUE);

	return;
}

void tx (void)
{
	putchar('d');
	putchar('\n');
	
	rftx_Send(5, data, 4*8, tx);
}