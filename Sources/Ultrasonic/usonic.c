#include "usonic.h"
#include "timers.h"
#include "mc9s12xdp512.h"
#include <stdio.h>

#define USONIC_TRIGG GLUE(PTT_PTT,USONIC_TRIGG_TIMER)
#define USONIC_TRIGG_DDR GLUE(DDRT_DDRT,USONIC_TRIGG_TIMER)

#define USONIC_PULSE_TIME 7
#define USONIC_TIMEOUT_OVF 3

#define USONIC_CONVERSION(x) ((x)*TIM_TICK_NS/58000)

typedef enum
{
	IDLE,
	TRIGGERING,
	WAITING_FOR_ECHO,
	WAITING_FOR_ECHO_TO_END,
} USONIC_STAGE;

struct
{
	usonic_ptr callback;
	
	USONIC_STAGE stage;
	u32 measuredValue;
	u8 overflowCount;
} usonic_data;

bool usonic_IsInit = _FALSE;

void usonic_TriggerCallback (void);
void usonic_EchoCallback (void);
void usonic_EchoOverflow (void);

void usonic_Init (void)
{
	u32 i;
	if (usonic_IsInit == _TRUE)
		return;
	
	usonic_IsInit = _TRUE;	

	usonic_data.stage = IDLE;
	if (tim_dAreInterruptsEnabled(USONIC_TRIGG_TIMER))
		;

	tim_Init();	
	
	tim_GetTimer (TIM_OC, usonic_TriggerCallback, NULL, USONIC_TRIGG_TIMER);
	tim_GetTimer (TIM_IC, usonic_EchoCallback, usonic_EchoOverflow, USONIC_ECHO_TIMER);

	tim_DisconnectOutput(USONIC_TRIGG_TIMER);
	USONIC_TRIGG_DDR = DDR_OUT;
	USONIC_TRIGG = 0;
	
	for (i = 0; i < 10000; i++)
		asm nop;

	return;
}

void usonic_Measure (usonic_ptr callback)
{
	if ((usonic_data.stage != IDLE) || (callback == NULL))
		return;
	
	usonic_data.callback = callback;	
	
	USONIC_TRIGG = 1;
	
	tim_SetValue (USONIC_TRIGG_TIMER, tim_GetGlobalValue() + USONIC_PULSE_TIME);
	tim_ClearFlag (USONIC_TRIGG_TIMER);
	tim_EnableInterrupts (USONIC_TRIGG_TIMER);
	
	usonic_data.stage = TRIGGERING;
	
	return;
}

void usonic_TriggerCallback (void)
{
	switch (usonic_data.stage)
	{
		case TRIGGERING:
			USONIC_TRIGG = 0;
			
			tim_DisableInterrupts (USONIC_TRIGG_TIMER);
			
			tim_ClearFlag (USONIC_ECHO_TIMER);
			tim_SetRisingEdge (USONIC_ECHO_TIMER);
			tim_EnableInterrupts (USONIC_ECHO_TIMER);
			
			usonic_data.stage = WAITING_FOR_ECHO;
			
			break;
			
		case WAITING_FOR_ECHO_TO_END:
			if (usonic_data.overflowCount >= USONIC_TIMEOUT_OVF)
			{
				tim_DisableOvfInterrupts (USONIC_ECHO_TIMER);
				tim_DisableInterrupts (USONIC_ECHO_TIMER);
				tim_DisableInterrupts (USONIC_TRIGG_TIMER);
				usonic_data.stage = IDLE;
				printf("timeout\n");
			}
			
			break;
	}
}

void usonic_EchoCallback (void)
{
	tim_EnableInterrupts (USONIC_TRIGG_TIMER);
	
	switch (usonic_data.stage)
	{
		case WAITING_FOR_ECHO:
			usonic_data.measuredValue = tim_GetValue (USONIC_ECHO_TIMER);
			usonic_data.overflowCount = 0;
			tim_SetFallingEdge (USONIC_ECHO_TIMER);
			tim_EnableOvfInterrupts (USONIC_ECHO_TIMER);
			
			usonic_data.stage = WAITING_FOR_ECHO_TO_END;
			
			break;
		
		case WAITING_FOR_ECHO_TO_END:
			usonic_data.measuredValue = (usonic_data.overflowCount * (u32)TIM_OVERFLOW_TICKS + tim_GetValue (USONIC_ECHO_TIMER)) - usonic_data.measuredValue;		
			
			tim_DisableInterrupts (USONIC_ECHO_TIMER);
			tim_DisableOvfInterrupts (USONIC_ECHO_TIMER);
			tim_DisableInterrupts (USONIC_TRIGG_TIMER);
			usonic_data.stage = IDLE;
			
			(*usonic_data.callback) (USONIC_CONVERSION (usonic_data.measuredValue));
						
			break;
	}

}

void usonic_EchoOverflow (void)
{
	usonic_data.overflowCount++;
}