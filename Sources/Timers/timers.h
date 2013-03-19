#ifndef _TIMERS_H
#define _TIMERS_H

#include "common.h"
#include "timers_macros.h"

#define TIM_TICK_NS 1333
// The resolution of the global timer (TCNT) is 1.333 microseconds. The TCNT overflows every 87.4 miliseconds.
// In order to change this value, TIM_PRESCALER in timers.c must be changed and the file recompiled.

#define TIM_OVERFLOW_TICKS 65536

#define TIM_US_TO_TICKS(us) (DIV_CEIL(((u32)us)*1e3,TIM_TICK_NS)) // Converts microseconds to timer ticks
#define TIM_TICKS_TO_US(ticks) (DIV_CEIL(ticks*TIM_TICK_NS,1e3)) // Converts timer ticks to microseconds

typedef enum
{
	TIM_IC,	// Input Capture
	TIM_OC	// Output Compare
} tim_type;

typedef void (*tim_ptr) (void); // A function callback for registering in a timer

typedef s8 tim_id; // An id for a registered callback

void tim_Init(void);
// Initializes the timer module. This requires no other modules to work, and doesn't require interrupts to be enabled.

tim_id tim_GetFreeTimer(tim_type reqType, tim_ptr callback, tim_ptr overflow);
// Sets one of the module's free timers as reqType. 
// When the timer interrupts, callback is called. 
// When the global timer (TCNT) overflows, overflow is called.
// callback and overflow are called with interrupts inhibited and MUST NOT disinhibit them.
// callback must not be NULL, otherwise, the timer is not assigned.
// The received timer has normal an overflow interrupts (they have to be enabled using tim_EnableInterrupts and tim_EnableOvfInterrupts).
// Returns the tim_id of the assigned timer. 

tim_id tim_GetTimer(tim_type reqType, tim_ptr callback, tim_ptr overflow, tim_id timNumber);
// Same as tim_GetTimer, but instead of assigning a random timer, timNUmberisUsed. 
// timNumber is a number, from 0 to 7, which indicates which timer module will be used.
// If the requested timer is free, the return value will equal timNumber.

void tim_FreeTimer(tim_id id);
// Deletes the overflow and interrupt callbacks from a previously set timer, and releases it so it can be reassigned using tim_GetTimer.

#define TIM_INVALID_ID (-1) // Returned by tim_GetFreeTimer if all timers are being used, or by tim_GetTimer if timNumber is being used.


// All of the functions below will fail or behave unexpectedly if they receive an invalid timer id.
// There are two versions for some of the following functions: the first one is a macro (defined in timers_macros.h), and requires the 
// received id to be determined in preprocessing time (a constant, not a variable). The second one, which has a tim_d prefix, is 
// dynamic, and can receive a non-constant id. The macro implementation is usually about twice as fast as the dynamic implementation, 
// and is therefore recommended, especially since the timer id is usually known to the programmer.


bool tim_dAreInterruptsEnabled(tim_id id);
// Informs whether or not interrupts are enabled for a timer.

void tim_dEnableInterrupts(tim_id id);
// Enables interrupts for a timer.
void tim_dDisableInterrupts(tim_id timId);
// Disables interrupts for a timer.

void tim_dClearFlag(tim_id id);
// Clears the interrupt flag of a timer. This is done automatically everytime callback is called, so this function is
// therefore rarely called.

void tim_EnableOvfInterrupts(tim_id id);
// Enables TCNT overflow interrupts for a timer.
void tim_DisableOvfInterrupts(tim_id id);
// Disables TCNT overflow interrupts for a timer.

#define tim_GetGlobalValue() (TCNT)
// Returns the value of the global timer (TCNT).


// Input capture functions

u16 tim_dGetValue(tim_id id);
// Returns a timer's value. This is only useful for Input Capture timers, returning the value of the TCNT when they last interrupted.

void tim_dSetFallingEdge(tim_id id);
// Sets an Input Capture's trigger as a falling edge.
void tim_dSetRisingEdge(tim_id id);
// Sets an Input Capture's trigger as a rising edge.
void tim_dSetBothEdge(tim_id id);
// Sets an Input Capture's trigger as both falling and rising edges.


// Output compare functions

void tim_dSetValue(tim_id id, u16 value);
// Sets a timer's value. This is only useful for Output Compare timers, setting the value of the TCNT that triggers an interrupt.

void tim_dSetOutputHigh(tim_id id);
void tim_dSetOutputLow(tim_id id);
void tim_dSetOutputToggle(tim_id id);
void tim_dDisconnectOutput(tim_id id);


extern void interrupt tim0_Service(void);
// Interrupt function. To be registered in the IVT under timch0.
extern void interrupt tim1_Service(void);
// Interrupt function. To be registered in the IVT under timch1.
extern void interrupt tim2_Service(void);
// Interrupt function. To be registered in the IVT under timch2.
extern void interrupt tim3_Service(void);
// Interrupt function. To be registered in the IVT under timch3.
extern void interrupt tim4_Service(void);
// Interrupt function. To be registered in the IVT under timch4.
extern void interrupt tim5_Service(void);
// Interrupt function. To be registered in the IVT under timch5.
extern void interrupt tim6_Service(void);
// Interrupt function. To be registered in the IVT under timch6.
extern void interrupt tim7_Service(void);
// Interrupt function. To be registered in the IVT under timch7.
extern void interrupt timOvf_Service(void);
// Interrupt function. To be registered in the IVT under timovf.

#endif