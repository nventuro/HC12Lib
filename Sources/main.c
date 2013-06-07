#include "mc9s12xdp512.h"
#include "common.h"
#include "pll.h"
#include "quick_serial.h"
#include "lcd.h"
#include "rftx.h"

void Init (void);

#define C1_ID 0
#define C2_ID 1
#define SHIFT_ID 2
#define BATT_ID 3

u8 up = 0xFF;
u8 down = 0x00;

void main (void)
{
	Init ();	
		
	while (1)
	{
		char c = qs_getchar(0);
		switch(c)
		{
			case 'w':
				rftx_Send(C1_ID,&down,7,NULL);
				break;
			case 'e':
				rftx_Send(C1_ID,&up,7,NULL);
				break;
			case 's':
				rftx_Send(C2_ID,&down,7,NULL);
				break;
			case 'd':
				rftx_Send(C2_ID,&up,7,NULL);
				break;
			case 'x':
				rftx_Send(SHIFT_ID,&down,7,NULL);
				break;
			case 'c':
				rftx_Send(SHIFT_ID,&up,7,NULL);
				break;
			case ' ':
				rftx_Send(BATT_ID,&down,7,NULL);
				break;
		}
	}
		
}

void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);

	// Modules that don't require interrupts to be enabled
	qs_init(0, MON12X_BR);
	
	asm cli;

	// Modules that do require interrupts to be enabled
	rftx_Init(_TRUE);

	return;
}
