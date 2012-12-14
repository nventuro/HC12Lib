#ifndef _TIMERS_MACROS_H
#define _TIMERS_MACROS_H

#include "mc9s12xdp512.h"

extern int tim_macroVar;

#define TCTL3_EDG0x tim_macroVar
#define TCTL3_EDG1x tim_macroVar
#define TCTL3_EDG2x tim_macroVar
#define TCTL3_EDG3x tim_macroVar
#define TCTL4_EDG4x tim_macroVar
#define TCTL4_EDG5x tim_macroVar
#define TCTL4_EDG6x tim_macroVar
#define TCTL4_EDG7x tim_macroVar

#define TCTL1_OM0 tim_macroVar
#define TCTL1_OM1 tim_macroVar
#define TCTL1_OM2 tim_macroVar
#define TCTL1_OM3 tim_macroVar
#define TCTL2_OM4 tim_macroVar
#define TCTL2_OM5 tim_macroVar
#define TCTL2_OM6 tim_macroVar
#define TCTL2_OM7 tim_macroVar

#define TCTL1_OL0 tim_macroVar
#define TCTL1_OL1 tim_macroVar
#define TCTL1_OL2 tim_macroVar
#define TCTL1_OL3 tim_macroVar
#define TCTL2_OL4 tim_macroVar
#define TCTL2_OL5 tim_macroVar
#define TCTL2_OL6 tim_macroVar
#define TCTL2_OL7 tim_macroVar

#define tim_AreInterruptsEnabled(timId) BOOL(GLUE2(TIE_C,timId,I))
#define tim_EnableInterrupts(timId) (GLUE2(TIE_C,timId,I)=1)
#define tim_DisableInterrupts(timId) (GLUE2(TIE_C,timId,I)=0)

#define tim_ClearFlag(timId) (TFLG1=1<<timId)

#define tim_GetValue(timId) (GLUE(TC,timId))
#define tim_SetFallingEdge(timId) do{if(timId<4){GLUE2(TCTL4_EDG,timId,x)=0x2;}else{GLUE2(TCTL3_EDG,timId,x)=0x2;}}while(0)
#define tim_SetRisingEdge(timId) do{if(timId<4){GLUE2(TCTL4_EDG,timId,x)=0x1;}else{GLUE2(TCTL3_EDG,timId,x)=0x1;}}while(0)
#define tim_SetBothEdge(timId) do{if(timId<4){GLUE2(TCTL4_EDG,timId,x)=0x3;}else{GLUE2(TCTL3_EDG,timId,x)=0x3;}}while(0)

#define tim_SetValue(timId,value) (GLUE(TC,timId)=value)
#define tim_SetOutputHigh(timId) do{if(timId<4){GLUE(TCTL2_OL,timId)=1;GLUE(TCTL2_OM,timId)=1;}else{GLUE(TCTL1_OL,timId)=1;GLUE(TCTL1_OM,timId)=1;}}while(0)
#define tim_SetOutputLow(timId) do{if(timId<4){GLUE(TCTL2_OL,timId)=0;GLUE(TCTL2_OM,timId)=1;}else{GLUE(TCTL1_OL,timId)=0;GLUE(TCTL1_OM,timId)=1;}}while(0)
#define tim_SetOutputToggle(timId) do{if(timId<4){GLUE(TCTL2_OL,timId)=1;GLUE(TCTL2_OM,timId)=0;}else{GLUE(TCTL1_OL,timId)=1;GLUE(TCTL1_OM,timId)=0;}}while(0)
#define tim_DisconnectOutput(timId) do{if(timId<4){GLUE(TCTL2_OL,timId)=0;GLUE(TCTL2_OM,timId)=0;}else{GLUE(TCTL1_OL,timId)=0;GLUE(TCTL1_OM,timId)=0;}}while(0)

#endif