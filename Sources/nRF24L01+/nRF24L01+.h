#ifndef _NRF_H
#define _NRF_H

#include "common.h"

typedef enum 
{
	PTX = 0,
	PRX
} nrf_Type;


void nrf_Init (nrf_Type type);
// Enables the nrf module in either PTX or PRX mode. This requires the RTI module to work, and interrupts to be enabled.
// Additionally, the IRQ pin from the nrf must be connected to the IRQ pin on the HCS12, and nrf_irq_serv set on the 
// correspoding position in the ivt.

void interrupt nrf_irq_Service (void);


#endif