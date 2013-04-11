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
void tx1 (void);
void tx7 (void);
//void rx (void);

u8 mem3tx[6];
u8 mem5tx[6];
u8 mem1tx[4];
u8 mem7tx[7];
void main (void)
{	
	Init ();
	mem1tx[0] = 's';
	mem1tx[1] = 'o';
	mem1tx[2] = 's';
	mem1tx[3] = ' ';
	
	mem3tx[0] = 'm';
	mem3tx[1] = 'e';
	mem3tx[2] = 'd';
	mem3tx[3] = 'i';
	mem3tx[4] = 'o';
	mem3tx[5] = ' ';
	
	mem5tx[0] = 't';
	mem5tx[1] = 'r';
	mem5tx[2] = 'o';
	mem5tx[3] = 'l';
	mem5tx[4] = 'o';
	mem5tx[5] = ' ';
	
	mem7tx[0] = 'j';
	mem7tx[1] = 'u';
	mem7tx[2] = 'a';
	mem7tx[3] = 'n';
	mem7tx[4] = 'i';
	mem7tx[5] = '.';
	mem7tx[6] = '\n';
	
	
//	rfrx_Register(3, rx, mem3rx);
//	rfrx_Register(5, rx, mem5rx);
	
	rftx_Send(1, mem1tx, 31, tx1);
	rftx_Send(3, mem3tx, 47, tx3);
	rftx_Send(5, mem5tx, 47, tx5);

	rftx_Send(7, mem7tx, 55, tx7);

	while (1)
	{
/*		if (i == 2)
		{
			printf("llego %s y %s.\n",mem3rx,mem5rx);
		}*/
	}
}

void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);

	// Modules that don't require interrupts to be enabled
	qs_init(0,MON12X_BR);
	asm cli;

	// Modules that do require interrupts to be enabled
//	rfrx_Init();
	rftx_Init(_TRUE);
	
	return;
}
/*
void rx(void)
{
	i++;
}*/

void tx3(void)
{
	rftx_Send(3,mem3tx,47,tx3);
}

void tx5(void)
{
	rftx_Send(5,mem5tx,47,tx5);
}
void tx1(void)
{
	rftx_Send(1,mem1tx,31,tx1);
}

void tx7(void)
{
	rftx_Send(7,mem7tx,55,tx7);
}