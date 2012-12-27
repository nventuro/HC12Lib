#include "mc9s12xdp512.h"
#include "common.h"
#include "usonic.h"
#include "lcd.h"

void Init (void);
void PrintMeas (s32 measurement);

void main (void)
{
	Init ();
	
	while (1)
		usonic_Measure (PrintMeas);
}

void Init (void)
{
	// Modules that don't require interrupts to be enabled
	
	asm cli;
	
	// Modules that do require interrupts to be enabled
	usonic_Init ();
	lcd_Init (LCD_2004);
	
	return;
}

void PrintMeas (s32 measurement)
{
	if (measurement == USONIC_INVALID_MEAS)
		lcd_Print ("Medicion invalida.");
	else
		lcd_Print ("Distancia: algo cm");
}
