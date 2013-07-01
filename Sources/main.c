#include "mc9s12xdp512.h"
#include "common.h"
#include "pll.h"
#include "quick_serial.h"
#include "lcd.h"
#include "fjoy_disp.h"
#include "spi.h"
#include "batt.h"
#include <stdio.h>

void Init (void);
void test(void); 
u8 in[5];
u8 out[5];
void main (void)
{
	Init ();	
	batt_AddBatt (ATD0, 0, test, 50, 30, 80, NULL);
	printf("Puto.");
	//fjoy_PrintAxes();
	/*in[0] = 0x0F;
	in[1] = 0xF0;
	in[2] = 0x00;
	in[3] = 0xFF;
	in[4] = 0xAA;
	
	spi_Transfer (&in, &out, 5, test);*/
	
	while (1)
		;	
}

void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);

	// Modules that don't require interrupts to be enabled
	qs_init(0, MON12X_BR);
	//spi_Init(_FALSE, _FALSE);
	batt_Init();
	
	asm cli;

	// Modules that do require interrupts to be enabled
	//lcd_Init(LCD_2004);

	return;
}

void test(void)
{
	printf("Sent.\n");
	printf("%d\n", (s16) out[0]);
	printf("%d\n", (s16) out[1]);
	printf("%d\n", (s16) out[2]);
	printf("%d\n", (s16) out[3]);
	printf("%d\n", (s16) out[4]);
	
}