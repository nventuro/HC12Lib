#include "mc9s12xdp512.h"
#include "common.h"
#include "pll.h"
#include "rftx.h"
#include "rfrx.h"
#include <stdio.h>
#include "quick_serial.h"

void Init (void);

void tx (void);
void rx (void);

u8 mem[5];

void main (void)
{	
	Init ();

	rfrx_Register(5, rx, mem);
	
	rftx_Send(5, mem, 0, tx);
	rftx_Send(5, mem, 0, tx);
	rftx_Send(5, mem, 0, tx);

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
	rfrx_Init();

	return;
}

void tx (void)
{
	putchar('t');	
//	rftx_Send(5, mem, 0, tx);
}

void rx(void)
{
	putchar('r');
}