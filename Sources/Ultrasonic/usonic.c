#include "usonic.h"
#include "timers.h"

#define USONIC_TRIGG GLUE(PTT_PTT,USONIC_TRIGG_TIMER)
#define USONIC_TRIGG_DDR GLUE(DDRT_DDRT,USONIC_TRIGG_TIMER)

#define USONIC_PULSE_TIME 6
#define USONIC_EDGE_TIME 4

#define USONIC_CONVERSION(x) ((x)/58)

struct
{
	bool busy;
	tim_id trigger;
	tim_id echo;
	bool echoInhibited;
	usonic_ptr callback;
	
	bool measuringWidth;
	u32 measuredValue;
	u8 overflowCount;
} usonic_data;

bool usonic_IsInit = _FALSE;

void usonic_TriggerCallback (void);
void usonic_EchoCallback (void);
void usonic_EchoOverflow (void);

void usonic_Init (void)
{
	if (usonic_IsInit == _TRUE)
		return;
		
	usonic_IsInit = _TRUE;	
	
	usonic_data.busy = _FALSE;
	
	
	tim_Init();	
	usonic_data.trigger = tim_GetTimer (TIM_OC, usonic_TriggerCallback, NULL, USONIC_TRIGG_TIMER);
	usonic_data.echo = tim_GetTimer (TIM_IC, usonic_EchoCallback, usonic_EchoOverflow, USONIC_ECHO_TIMER);
	usonic_data.echoInhibited = _FALSE;
	
	tim_DisconnectOutput(usonic_data.trigger);
	USONIC_TRIGG_DDR = DDR_OUT;
	USONIC_TRIGG = 0;

	return;
}

void usonic_Measure (usonic_ptr callback)
{
	if (usonic_data.busy == _TRUE)
		return;
	
	usonic_data.busy = _TRUE;
	
	usonic_data.callback = callback;	
	
	USONIC_TRIGG = 1;
	
	tim_SetValue (usonic_data.trigger, tim_GetGlobalValue() + USONIC_PULSE_TIME);
	tim_ClearFlag (usonic_data.trigger);
	tim_EnableInterrupts (usonic_data.trigger);
	
	return;
}

void usonic_TriggerCallback (void)
{
    if (usonic_data.echoInhibited == _TRUE)
	{
		tim_DisableInterrupts(usonic_data.trigger);
		
		usonic_data.echoInhibited = _FALSE;
		
		if (usonic_data.measuringWidth == _TRUE)
		{
			tim_ClearFlag (usonic_data.echo);
			tim_SetFallingEdge (usonic_data.echo);
			tim_EnableInterrupts (usonic_data.echo);
		}
		else
		{
			usonic_data.busy = _FALSE;
			(*usonic_data.callback) (USONIC_CONVERSION (usonic_data.measuredValue));
		}			
    }
    else
    {
    	USONIC_TRIGG = 0;
    	tim_DisableInterrupts (usonic_data.trigger);
    	
    	tim_ClearFlag (usonic_data.echo);
    	tim_SetRisingEdge (usonic_data.echo);
    	tim_EnableInterrupts (usonic_data.echo);
    }
}

void usonic_EchoCallback (void)
{
	usonic_data.echoInhibited = _TRUE;
	tim_DisableInterrupts (usonic_data.echo);
	
	tim_SetValue (usonic_data.trigger, tim_GetGlobalValue() + USONIC_EDGE_TIME);
	tim_ClearFlag (usonic_data.trigger);
	tim_EnableInterrupts (usonic_data.trigger);
	
	if (usonic_data.measuringWidth == _FALSE)
	{
		usonic_data.measuringWidth = _TRUE;
		usonic_data.measuredValue = tim_GetValue (usonic_data.echo);
		usonic_data.overflowCount = 0;
	}
	else
	{
		usonic_data.measuringWidth = _FALSE;
		usonic_data.measuredValue = (usonic_data.overflowCount * TIM_OVERFLOW_TICKS + tim_GetValue (usonic_data.echo)) - usonic_data.measuredValue;
	}

}

void usonic_EchoOverflow (void)
{
	usonic_data.overflowCount++;
}