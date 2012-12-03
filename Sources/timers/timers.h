#ifndef _TIMERS_H
#define _TIMERS_H

#include "common.h"

#define TIM_STEP 100 // The resolution of the global timer (TCNT) in nanoseconds.
// In order to change this value, TIM_PRESCALER in tim.c must be changed and recompiled.

typedef enum
{
	TIM_IC,	// Input Capture
	TIM_OC	// Output Compare
} tim_type;

typedef void (*tim_ptr) (void); // A function callback for registering in a timer

typedef s8 tim_id; // An id for a registered callback

void tim_Init (void);
// Initializes the timer module. This requires no other modules to work, and doesn't require interrupts to be enabled.

tim_id tim_GetTimer(tim_type reqType, tim_ptr callback, tim_ptr overflow);
// Sets one of the module's free timers as reqType. 
// When the timer interrupts, callback is called. 
// When the global timer (TCNT) overflows, overflow is called.
// callback and overflow are called with interrupts inhibited and MUST NOT disinhibit them.
// The received timer has normal an overflow interrupts (they have to be enabled using tim_EnableInterrupts and tim_EnableOvfInterrupts).
// Returns the tim_id of the assigned timer. 

tim_id tim_GetSpecificTimer(tim_type reqType, tim_ptr callback, tim_ptr overflow, tim_id timNumber);
// Same as tim_GetTimer, but instead of assigning a random timer, timNUmberisUsed.

#define TIM_INVALID_ID (-1) // Returned by tim_GetTimer if all timers are being used, or by tim_GetSpecificTimer if timNumber is being used.

void tim_FreeTimer(tim_id id);
// Deletes the overflow and interrupt callbacks from a previously set timer, and releases it so it can be reassigned using tim_GetTimer.

void tim_SetFallingEdge(tim_id id);
// Sets an Input Capture's trigger as a falling edge.
void tim_SetRisingEdge(tim_id id);
// Sets an Input Capture's trigger as a rising edge.
void tim_SetBothEdge(tim_id id);
// Sets an Input Capture's trigger as both falling and rising edges.

void tim_SetOutputHigh(tim_id id);
void tim_SetOutputLow(tim_id id);
void tim_SetOutputToggle(tim_id id);
void tim_DisconnectOutput(tim_id id);

bool tim_AreInterruptsEnabled (tim_id id);
// Informs whether or not interrupts are enabled for a timer.
void tim_EnableInterrupts(tim_id id);
// Enables interrupts for a timer.
void tim_DisableInterrupts(tim_id id);
// Disables interrupts for a timer.

void tim_EnableOvfInterrupts(tim_id id);
// Enables TCNT overflow interrupts for a timer.
void tim_DisableOvfInterrupts(tim_id id);
// Disables TCNT overflow interrupts for a timer.

void tim_ClearFlag(tim_id id);
// Clears the interrupt flag of a timer. This is done automatically everytime callback and overflow are called.

void tim_SetValue(tim_id id, u16 value);
// Sets a timer's value. This is only useful for Output Compare timers, setting the value of the TCNT that triggers an interrupt.
u16 tim_GetValue(tim_id id);
// Returns a timer's value. This is only useful for Input Capture timers, returning the value of the TCNT when they last interrupted.

u16 tim_GetGlobalValue(void);
// Returns the value of the global timer (TCNT).

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