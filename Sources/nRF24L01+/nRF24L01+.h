#ifndef _NRF_H
#define _NRF_H

#include "common.h"
#include "mc9s12xdp512.h"

#define NRF_CE PTS_PTS2 // The Chip Enable pin
#define NRF_CE_DDR DDRS_DDRS2 // The DDR register for NRF_CE

typedef void (*nrf_PTXptr) (bool success, u8 *ackPayload, u8 length);
typedef void (*nrf_PRXptr) (u8 *data, u8 length);

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


// PTX FUNCTIONS

void nrf_Transmit (u8 *data, u8 length, nrf_PTXptr eot);
// Initializes a transmission to Pipe 0. data points to a segment of length (which must be non-zero, and must not be greater than 32)
// bytes, which will be transmitted. After the transmission ends, eot is called. success is true if an ACK packet was received 
// from the PRX, and false if the maximum number of retransmissions have occured. If the PRX answers with payload, it is stored in ackPayload, and
// length contains it's length.
// ackPayload must be read before any call to any other nrf function is done.
// nrf_Transmit throws an error if the module is not initialized as PTX, or if a transmission is taking place.
// When eot is called, a new transmission can be initiated.

bool nrf_IsBusy(void);
// Informs wheter the module is busy. nrf_.Transmit can only be called if IsBusy is false. After eot is called in
// nrf_Transmit, IsBusy will be false.
// IsBusy can only be called on PTX mode.

// PRX FUNCTIONS

void nrf_Receive (nrf_PRXptr eot);
// Registers a callback for reception on Pipe 0. When data is received, it is stored in data, and eot is called. length
// (eot's argument) contains the number of bytes received, which are stored in data.
// data must be read before any call to any other nrf function is done.
// nrf_Receive can only be called in PRX mode.

void nrf_StoreAckPayload (u8 *data, u8 length);
// Uploads to the device the payload that will be sent with the next ACK, stored in data, of length bytes.

void interrupt nrf_irq_Service (void);


#endif
	