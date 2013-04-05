#ifndef _RFTX_H
#define _RFTX_H

#include "common.h"

#define RFTX_DATA_TIMER 4 // The timer module used for transmitting data, connected to the transmitter's data pin.
// If the timer is changed, rftx.c must be recompiled.

// Timing in us
// f = 3khz
#define RFTX_1_HIGH_TIME_US 130
#define RFTX_1_LOW_TIME_US 200
#define RFTX_0_HIGH_TIME_US 200
#define RFTX_0_LOW_TIME_US 130

#define RFTX_DEAD_TIME_US 400
#define RFTX_START_TX_TIME_US 100
#define RFTX_TICK_TIMEOUT_US 5000
#define RFTX_TICK_DURATION_US 400

#define RFTX_BIT_TIME_US (RFTX_1_HIGH_TIME_US+RFTX_1_LOW_TIME_US)

// Timing in TCNT ticks
#define RFTX_1_HIGH_TIME_TICKS TIM_US_TO_TICKS(RFTX_1_HIGH_TIME_US)
#define RFTX_1_LOW_TIME_TICKS TIM_US_TO_TICKS(RFTX_1_LOW_TIME_US)
#define RFTX_0_HIGH_TIME_TICKS TIM_US_TO_TICKS(RFTX_0_HIGH_TIME_US)
#define RFTX_0_LOW_TIME_TICKS TIM_US_TO_TICKS(RFTX_0_LOW_TIME_US)

#define RFTX_DEAD_TIME_TICKS TIM_US_TO_TICKS(RFTX_DEAD_TIME_US)
#define RFTX_START_TX_TIME_TICKS TIM_US_TO_TICKS(RFTX_START_TX_TIME_US)
#define RFTX_TICK_TIMEOUT_TICKS TIM_US_TO_TICKS(RFTX_TICK_TIMEOUT_US)
#define RFTX_TICK_DURATION_TICKS TIM_US_TO_TICKS(RFTX_TICK_DURATION_US)

#define RFTX_BIT_TIME_TICKS TIM_US_TO_TICKS(RFTX_BIT_TIME_US)


typedef void (*rftx_ptr) (void); // A function callback used for notifying the end of a transmission

void rftx_Init(bool ecc);
// Initializes the RF TX module. This requires the Timers module to work, and doesn't require interrupts to be enabled.
// If ecc equals _TRUE, a 15-bit Hamming Error Correcting Code (11 data bits, 4 parity bits) will be used by the module for all communications.

void rftx_Send(u8 id, u8 *data, u8 length, rftx_ptr eot);
// If the transmitter is idle, this commences data transmission. Otherwise, the transmission request is stored in a FIFO queue and will be 
// completed at a later time. In both of these scenarios, rftx_Send returns _TRUE. However, if the FIFO queue is full, the request is not stored,
// and rftx_Send returns _FALSE. There are as many queue slots as there are id's.
// id indicates the id of the receiver (from 0 to 7). The content of the message (of length bits, up to 128, from MSB TO LSB) must be stored beforehand in data, and 
// must not be modified until eot is called, since the module will be actively using this portion of RAM.
// eot is called when the tranmission ends, and a new tranmission can be requested at this time.

#endif