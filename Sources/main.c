#include "mc9s12xdp512.h"
#include "common.h"
#include "pll.h"
#include "rftx.h"
#include "rfrx.h"
#include <stdio.h>
#include "quick_serial.h"

void Init (void);

void tx3 (void);
void tx5 (void);
void rx (void);

u8 mem[5];

void main (void)
{	
	Init ();

	rfrx_Register(5, rx, mem);
	
	rftx_Send(5, mem, 0, tx3);

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
	rfrx_Init();
	rftx_Init(_TRUE);
	
	return;
}

void tx3 (void)
{
	putchar('q');	
	rftx_Send(5, mem, 0, tx5);
}

void tx5 (void)
{
	putchar('w');	
	rftx_Send(3, mem, 0, tx3);
}

void rx(void)
{
	putchar('5');
}