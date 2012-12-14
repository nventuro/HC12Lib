#ifndef _RTI_H
#define _RTI_H

#include "common.h"

#define RTI_FREQ 781 // Hz. 
// In order to change this value, RTI_PRESCALER in rti.c must be changed and recompiled.
#define RTI_PER (1.0/RTI_FREQ) // seconds

typedef u16 rti_time; // An integer multiple of RTI_PER (in miliseconds)

#define RTI_MS2PERIOD(ms) (DIV_CEIL((((u32)ms)>0?ms:0)*RTI_FREQ,1000)) // Converts miliseconds to rti_time

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
#define RTI_ONCE 0 // delay for a function that will only be called once (in this case, period is irrelevant)
#define RTI_NOW 1 // delay for a function that will be called for the first time as soon as the RTI interrupts the CPU

void rti_SetPeriod(rti_id id, rti_time period); 
// Changes the period of a registered rti_id

void rti_Cancel(rti_id n); 
// Cancels a registered rti_id

extern void interrupt rti_Service(void); 
// Interrupt function. To be registered in the IVT under rti.

#endif
