#include "mc9s12xdp512.h"
#include "common.h"
#include "rftx.h"
#include <stdio.h>

void Init (void);
u8 i = 0;
void done (void);
void main (void)
{
	u32 i;
	Init ();
	
	for (i = 0; i < 2000; i++)
		;
	
	rftx_Send(7,NULL,0,done);

	while (1)
		;
}

void Init (void)
{
	// Modules that don't require interrupts to be enabled
	rftx_Init(_TRUE);
	asm cli;
	
	// Modules that do require interrupts to be enabled


	return;
}

void done (void)
{
	putchar('d');
	rftx_Send(i,NULL,0,done);
	i++;
	if (i == 7)
		i = 0;
}
