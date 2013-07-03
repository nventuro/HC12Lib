#include "mc9s12xdp512.h"
#include "common.h"
#include "pll.h"
#include "quick_serial.h"
#include "nRF24L01+.h"
#include <stdio.h>

void Init (void);
void nrf_Callback (bool success, u8 *ackPayload, u8 length);

u8 data[10];

void main (void)
{
	u8 i;
	Init ();	
	for (i = 0; i < 10; i++)
		data[i] = 0xAA;
	nrf_Transmit(data, 10, nrf_Callback);
	while (1)
		;	
}

void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);

	// Modules that don't require interrupts to be enabled
	qs_init(0, MON12X_BR);
	
	asm cli;

	// Modules that do require interrupts to be enabled
	nrf_Init(PTX);

	return;
}

void nrf_Callback (bool success, u8 *ackPayload, u8 length)
{
	if (success == _TRUE)
		printf("Salio todo bien.\n");
	else
		printf("Nadie tiro un ACK.\n");
	
	nrf_Transmit(data, 10, nrf_Callback);
}
	