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

u8 mem3tx[4];
u8 mem5tx[4];
u8 mem3rx[4];
u8 mem5rx[4];
int i = 0;
void main (void)
{	
	Init ();
	mem3tx[0] = 'a';
	mem3tx[1] = 'b';
	mem3tx[2] = 'c';
	mem3tx[3] = '\0';
	
	mem5tx[0] = 'x';
	mem5tx[1] = 'y';
	mem5tx[2] = 'z';
	mem5tx[3] = '\0';
	
	rfrx_Register(3, rx, mem3rx);
	rfrx_Register(5, rx, mem5rx);
	
	rftx_Send(3, mem3tx, 31, NULL);
	rftx_Send(5, mem5tx, 31, NULL);

	while (1)
	{
		if (i == 2)
		{
			printf("llego %s y %s.\n",mem3rx,mem5rx);
		}
	}
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

void rx(void)
{
	i++;
}