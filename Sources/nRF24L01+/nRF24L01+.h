#ifndef _NRF_H
#define _NRF_H

#include "common.h"
#include "mc9s12xdp512.h"

#define NRF_CE PTS_PTS2 // The Chip Enable pin
#define NRF_CE_DDR DDRS_DDRS2 // The DDR register for NRF_CE

typedef void (*nrf_ptr) (bool success, u8 *ackPayload, u8 length);

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

bool nrf_IsBusy(void);
// Informs wheter the module is busy. nrf_Transmit can only be called if IsBusy is false. After eot is called in
// nrf_Transmit, IsBusy will be false.
// IsBusy can only be called on PTX mode.

void nrf_Transmit (u8 *data, u8 length, nrf_ptr eot, u8 *payloadData);
// Initializes a transmission. data points to a segment of length (which must be non-zero, and must not be greater than 32)
// bytes, which will be transmitted. After the transmission ends, eot is called. success is true if an ACK packet was received 
// from the PRX, and false if the maximum number of retransmissions have occured. If payloadData == NULL, any payload
// received from the PRX is ignored. If it's not NULL, and the PRX answers with payload, it is stored in ackPayload, and
// length contains it's length.
// nrf_Transmit throws an error if the module is not initialized as PTX, or if a transmission is taking place.
// When eot is called, a new transmission can be initiated.

void interrupt nrf_irq_Service (void);


#endif

//	 RX: callback para un UNICO pipe. me da un chorizo de memoria, y cuando lo llamo ahí está lo que mandaron
// , y le digo el largo. Funcion para guardar el ack payload.	
	