#ifndef _GRFRX_H
#define _GRFRX_H

#include "common.h"

#define GRFRX_DATA_TIMER 3 // The timer module used for receiving data, connected to the reciver's data pin.
// If the timer is changed, grfrx.c must be recompiled.

typedef void (*grfrx_ptr) (u8 length); // A function callback used for notifying data reception, of length bits

void grfrx_Init(void);
// Initializes the RF RX module. This requires the Timers module to work, and doesn't require interrupts to be enabled.

void grfrx_Register(u8 id, grfrx_ptr eot, u8 *data);
// Assigns a callback and memory for communications directed to id id (from 0 to 7). Whenever a successful communication ends, 
// eot is called, and the received data is stored in data, of length length (eot's argument). data is filled from its
// MSB (bit 7) to its LSB (bit 0).

#endif