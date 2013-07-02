#ifndef _NRF_H
#define _NRF_H

#include "common.h"
#include "mc9s12xdp512.h"

#define NRF_CE PTM_PTM1 // The Chip Enable pin
#define NRF_CE_DDR DDRM_DDRM1 // The DDR register for NRF_CE

typedef enum 
{
	PTX = 0,
	PRX
} nrf_Type;

void nrf_Init (nrf_Type type);
// Enables the nrf module in either PTX or PRX mode. This requires the RTI module to work, and interrupts to be enabled.
// Additionally, the IRQ pin from the nrf must be connected to the IRQ pin on the HCS12, and nrf_irq_serv set on the 
// correspoding position in the ivt.
// Only Pipe 0 is used.

void interrupt nrf_irq_Service (void);


#endif