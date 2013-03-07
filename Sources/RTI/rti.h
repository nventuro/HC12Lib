#ifndef _RTI_H
#define _RTI_H

#include "common.h"

enum {_XTAL=0, _PLL};
#define RTI_CLOCK_SOURCE _PLL

#define RTI_CLOCK_MHZ ((RTI_CLOCK_SOURCE == _XTAL) ? 16 : BUS_CLOCK_MHZ)

enum {BASE_2=0, BASE_10};
#define RTI_DIVIDER_BASE BASE_10

#define RTI_PRESCALER (0b00011011 | (RTI_DIVIDER_BASE << 7)) // 24MHz / (24*10^3) = 976 Hz.

extern u32 rti_divTable[];

#define FREQ_HALF_DIVIDER ((u32)(1+(RTI_PRESCALER & 0x0F)))
#define FREQ_DIV_2 (FREQ_HALF_DIVIDER*(((u32)1 << (9+((RTI_PRESCALER & 0x70)>>4) ))))
#define FREQ_DIV_10 (FREQ_HALF_DIVIDER * rti_divTable[(RTI_PRESCALER & 0x70)>>4])

#if ((RTI_DIVIDER_BASE == BASE_2) && ((RTI_PRESCALER & 0x70) == 0))
#warning "Invalid divider value; check RTI divider table."
#endif

 
#define RTI_FREQ ((u16)((RTI_CLOCK_MHZ*((u32)1000000))/((RTI_DIVIDER_BASE == BASE_10)? (FREQ_DIV_10) : (FREQ_DIV_2)))) // Hz. 
// In order to change this value, RTI_PRESCALER above must be changed and the file recompiled.
// Note that divider base also affects rti divider (check tables 2-7 and 2-8 of MC9S12(...).pdf)
#define RTI_PER (1.0/RTI_FREQ) // seconds

typedef u16 rti_time; // A type for expressing time, each rti_time equals one RTI_PER

#define RTI_MS_TO_TICKS(ms) (DIV_CEIL(((u32)ms)*RTI_FREQ,1000)) // Converts miliseconds to rti_time

typedef s8 rti_id; // An id for a registered callback

typedef void (*rti_ptr) (void *data, rti_time period, rti_id id); // A function callback for registering in the RTI


void rti_Init(void);
// Initializes the RTI module. This requires no other modules to work, and doesn't require interrupts to be enabled.

rti_id rti_Register(rti_ptr callback, void *data, rti_time period, rti_time delay);
// Registers a callback function to be called periodically every period*RTI_PER seconds, after an initial delay of delay*RTI_PER seconds.
// period and delay can be set using RTI_MS2DIV(timeInMiliseconds).
// When callback is called, it receives data, period and its rti_id. 
// callback is called with interrupts inhibited and MUST NOT disinhibit them.
// Returns the rti_id of the registed callback. 

#define RTI_INVALID_ID (-1) // Returned by rti_Register if the RTI memory is full and callback registration can't be done.
	 
#define RTI_ALWAYS 1 // period for a function that will always be called (its frequency is RTI_FREQ)
#define RTI_ONCE 0 // period for a function that will only be called once after a certain delay
#define RTI_NOW 1 // delay for a function that will be called for the first time as soon as the RTI interrupts the CPU

void rti_SetPeriod(rti_id id, rti_time period); 
// Changes the period of a registered rti_id

void rti_Cancel(rti_id n); 
// Cancels a registered rti_id

extern void interrupt rti_Service(void); 
// Interrupt function. To be registered in the IVT under rti.

#endif
