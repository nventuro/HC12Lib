#include "mc9s12xdp512.h"
#include "common.h"

void Init (void);

void main (void)
{
	Init();
	while (1)
		;
}

void Init (void)
{
	// Modules that don't require interrupts to be enabled
	
	asm cli;
	
	// Modules that do require interrupts to be enabled
	
	return;
}