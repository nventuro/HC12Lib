#ifndef _GRFTX_H
#define _GRFTX_H

#include "common.h"

#define GRFTX_DATA_TIMER 4 // The timer module used for transmitting data, connected to the transmitter's data pin.
// If the timer is changed, grftx.c must be recompiled.

// Timing in us
// f = 3khz
#define GRFTX_1_HIGH_TIME_US 325
#define GRFTX_1_LOW_TIME_US 500
#define GRFTX_0_HIGH_TIME_US 500
#define GRFTX_0_LOW_TIME_US 325

#define GRFTX_DEAD_TIME_US 1000
#define GRFTX_START_TX_TIME_US 250
#define GRFTX_TICK_TIMEOUT_US 7500
#define GRFTX_TICK_DURATION_US 1500

#define GRFTX_BIT_TIME_US (GRFTX_1_HIGH_TIME_US+GRFTX_1_LOW_TIME_US)

// Timing in TCNT ticks
#define GRFTX_1_HIGH_TIME_TICKS TIM_US_TO_TICKS(GRFTX_1_HIGH_TIME_US)
#define GRFTX_1_LOW_TIME_TICKS TIM_US_TO_TICKS(GRFTX_1_LOW_TIME_US)
#define GRFTX_0_HIGH_TIME_TICKS TIM_US_TO_TICKS(GRFTX_0_HIGH_TIME_US)
#define GRFTX_0_LOW_TIME_TICKS TIM_US_TO_TICKS(GRFTX_0_LOW_TIME_US)

#define GRFTX_DEAD_TIME_TICKS TIM_US_TO_TICKS(GRFTX_DEAD_TIME_US)
#define GRFTX_START_TX_TIME_TICKS TIM_US_TO_TICKS(GRFTX_START_TX_TIME_US)
#define GRFTX_TICK_TIMEOUT_TICKS TIM_US_TO_TICKS(GRFTX_TICK_TIMEOUT_US)
#define GRFTX_TICK_DURATION_TICKS TIM_US_TO_TICKS(GRFTX_TICK_DURATION_US)

#define GRFTX_BIT_TIME_TICKS TIM_US_TO_TICKS(GRFTX_BIT_TIME_US)


typedef void (*grftx_ptr) (void); // A function callback used for notifying the end of a transmission

void grftx_Init(bool ecc);
// Initializes the RF TX module. This requires the Timers module to work, and doesn't require interrupts to be enabled.
// If ecc equals _TRUE, a 15-bit Hamming Error Correcting Code (11 data bits, 4 parity bits) will be used by the module for all communications.

void grftx_Send(u8 id, u8 *data, u8 length, grftx_ptr eot);
// If the transmitter is idle, this commences data transmission. Otherwise, the transmission request is stored in a FIFO queue and will be 
// completed at a later time. In both of these scenarios, grftx_Send returns _TRUE. However, if the FIFO queue is full, the request is not stored,
// and an error is thrown. There are as many queue slots as there are id's.
// id indicates the id of the receiver (from 0 to 7). The content of the message (of length+1 bits, up to 128, from MSB TO LSB) must be stored beforehand in data, and 
// must not be modified until eot is called, since the module will be actively using this portion of RAM.
// eot is called when the tranmission ends, and a new tranmission can be requested at this time.

#endif