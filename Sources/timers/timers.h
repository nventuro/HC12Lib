#ifndef _TIMERS_H
#define _TIMERS_H

#include "common.h"

#define INVALID_TIMER (-1)

typedef enum
{
	TIM_IC,
	TIM_OC
} tim_type;

typedef void (*tim_ptr) (void);

void tim_init (void);

s8 tim_getTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf);
s8 tim_safeGetTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf);
s8 tim_getSpecificTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf, u8 timNumber);

void tim_freeTimer(s8 timId);

void tim_setFallingEdge(s8 timId);
void tim_setRisingEdge(s8 timId);
void tim_setBothEdge(s8 timId);

void tim_setOutputHigh(s8 timId);
void tim_setOutputLow(s8 timId);
void tim_setOutputToggle(s8 timId);
void tim_disconnectOutput(s8 timId);

bool tim_areInterruptsEnabled (s8 timId);
void tim_enableInterrupts(s8 timId);
void tim_disableInterrupts(s8 timId);

void tim_enableOvfInterrupts(s8 timId);
void tim_disableOvfInterrupts(s8 timId);

void tim_clearFlag(s8 timId);

u16 tim_getValue(s8 timId);
void tim_setValue(s8 timId, u16 value);

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