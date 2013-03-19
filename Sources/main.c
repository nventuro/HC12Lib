#include "mc9s12xdp512.h"
#include "common.h"
#include "rftx.h"
#include <stdio.h>

void Init (void);

void done (void);
void main (void)
{
	Init ();
	
	rftx_Send(2,NULL,0,done);

	while (1)
		;
}

void Init (void)
{
	// Modules that don't require interrupts to be enabled
	asm cli;
	
	// Modules that do require interrupts to be enabled
	rftx_Init(_TRUE);

	return;
}

void done (void)
{
	printf("Termine!\n");
}