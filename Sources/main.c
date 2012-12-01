#include "common.h"
 
void init(void);


void main(void)
{		
	init();
	
	for(;;)
	{

	}	
}


void init (void)
{
	// Modules that don't require interruptions to initialize

	_asm cli;
	
	// Modules that do require interruptions to initialize
}