#ifndef _RFRX_H
#define _RFRX_H

#include "common.h"

#define RFRX_DATA_TIMER 3 // The timer module used for receiving data, connected to the reciver's data pin.
// If the timer is changed, rfrx.c must be recompiled.

typedef void (*rfrx_ptr) (u8 length); // A function callback used for notifying data reception, of length bits

void rfrx_Init(void);
// Initializes the RF RX module. This requires the Timers module to work, and doesn't require interrupts to be enabled.

void rfrx_Register(u8 id, rfrx_ptr eot, u8 *data);
// Assigns a callback and memory for communications directed to id id. Whenever a successful communication ends, 
// eot is called, and the received data is stored in data, of length length (eot's argument). data is filled from its
// MSB (bit 7) to its LSB (bit 0).

//rfrx_Delete(u8 id)

#endif