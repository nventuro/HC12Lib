#ifndef _TIMERS_H
#define _TIMERS_H

#include "common.h"

typedef enum
{
	TIM_IC,	// Input Capture
	TIM_OC	// Output Compare
} tim_type;

typedef void (*tim_ptr) (void); // A function callback for registering in a timer

typedef s8 tim_id; // An id for a registered callback

void tim_init (void);
// Initializes the Timer module. This requires no other modules to work.

tim_id tim_getTimer(tim_type reqType, tim_ptr callback, tim_ptr overflow);
// Sets one of the module's free timers as reqType. 
// When the timer interrupts, callback is called. 
// When the global timer (TCNT) overflows, overflow is called.
// callback and overflow are called with interrupts inhibited and MUST NOT disinhibit them.
// Returns the tim_id of the assigned timer. 

tim_id tim_getSpecificTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf, u8 timNumber);
// Same as tim_GetTimer, but instead of assigning a random timer, timNUmberisUsed.

#define TIM_INVALID_ID (-1) // Returned by tim_GetTimer if all timers are being used, or by tim_GetSpecificTimer if timNumber is being used.

void tim_freeTimer(tim_id id);

void tim_setFallingEdge(tim_id id);
void tim_setRisingEdge(tim_id id);
void tim_setBothEdge(tim_id id);

void tim_setOutputHigh(tim_id id);
void tim_setOutputLow(tim_id id);
void tim_setOutputToggle(tim_id id);
void tim_disconnectOutput(tim_id id);

bool tim_areInterruptsEnabled (tim_id id);
void tim_enableInterrupts(tim_id id);
void tim_disableInterrupts(tim_id id);

void tim_enableOvfInterrupts(tim_id id);
void tim_disableOvfInterrupts(tim_id id);

void tim_clearFlag(tim_id id);

u16 tim_getValue(tim_id id);
void tim_setValue(tim_id id, u16 value);

u16 tim_getGlobalValue(void);

extern void interrupt tim0_srv(void);
extern void interrupt tim1_srv(void);
extern void interrupt tim2_srv(void);
extern void interrupt tim3_srv(void);
extern void interrupt tim4_srv(void);
extern void interrupt tim5_srv(void);
extern void interrupt tim6_srv(void);
extern void interrupt tim7_srv(void);
extern void interrupt timOvf_srv(void);

#endif