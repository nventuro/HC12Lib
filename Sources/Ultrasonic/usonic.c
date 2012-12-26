#include "usonic.h"
#include "timers.h"
#include "rti.h"
#include "mc9s12xdp512.h"
 
#define USONIC_TRIGG GLUE(PTT_PTT,USONIC_TRIGG_TIMER)
#define USONIC_TRIGG_DDR GLUE(DDRT_DDRT,USONIC_TRIGG_TIMER)

#define USONIC_PULSE_TIME DIV_CEIL(10000,TIM_TICK_NS)
#define USONIC_SAMPLE_PERIOD_MS 30
#define USONIC_COOLDOWN_MS 3
#define USONIC_TIMEOUT_MS 300

#define USONIC_OUT_OF_RANGE (((u32)50)*1000*1000/TIM_TICK_NS)
#define USONIC_CONVERSION(x) (x<USONIC_OUT_OF_RANGE?((x)*TIM_TICK_NS/58000):USONIC_INVALID_MEAS)

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
	s32 measuredValue;
	u8 overflowCount;
	
	bool halfReady;
	rti_id timeOut;
} usonic_data;

bool usonic_IsInit = _FALSE;

void usonic_TriggerCallback (void);
void usonic_EchoCallback (void);
void usonic_EchoOverflow (void);

void usonic_SolveTiming (void *data, rti_time period, rti_id id);
void usonic_Timeout (void *data, rti_time period, rti_id id);

void usonic_Init (void)
{
	u32 i;
	if (usonic_IsInit == _TRUE)
		return;
	
	usonic_IsInit = _TRUE;	

	usonic_data.stage = IDLE;

	rti_Init();
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
	usonic_data.halfReady = _FALSE;
	
	USONIC_TRIGG = 1;

	tim_SetValue (USONIC_TRIGG_TIMER, tim_GetGlobalValue() + USONIC_PULSE_TIME);
	tim_ClearFlag (USONIC_TRIGG_TIMER);
	tim_EnableInterrupts (USONIC_TRIGG_TIMER);
	
	rti_Register (usonic_SolveTiming, NULL, RTI_ONCE, RTI_MS2PERIOD(USONIC_SAMPLE_PERIOD_MS));
	
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
	}
}

void usonic_EchoCallback (void)
{
	switch (usonic_data.stage)
	{
		case WAITING_FOR_ECHO:
			usonic_data.measuredValue = tim_GetValue (USONIC_ECHO_TIMER);
			usonic_data.overflowCount = 0;
			tim_SetFallingEdge (USONIC_ECHO_TIMER);
			tim_EnableOvfInterrupts (USONIC_ECHO_TIMER);

			usonic_data.timeOut = rti_Register(usonic_Timeout, NULL, RTI_ONCE, RTI_MS2PERIOD(USONIC_TIMEOUT_MS));
			
			usonic_data.stage = WAITING_FOR_ECHO_TO_END;
			
			break;
		
		case WAITING_FOR_ECHO_TO_END:
			usonic_data.measuredValue = (usonic_data.overflowCount * (u32)TIM_OVERFLOW_TICKS + tim_GetValue (USONIC_ECHO_TIMER)) - usonic_data.measuredValue;		
			
			tim_DisableInterrupts (USONIC_ECHO_TIMER);
			tim_DisableOvfInterrupts (USONIC_ECHO_TIMER);

			rti_Register (usonic_SolveTiming, NULL, RTI_ONCE, RTI_MS2PERIOD(USONIC_COOLDOWN_MS));	
			
			break;
	}

}

void usonic_EchoOverflow (void)
{
	usonic_data.overflowCount++;
}

// Callback that counts 30ms or 1 ms
void usonic_SolveTiming (void *data, rti_time period, rti_id id)
{
	if (usonic_data.halfReady == _FALSE)
		usonic_data.halfReady = _TRUE;
	else
	{
		rti_Cancel(usonic_data.timeOut);
		usonic_data.stage = IDLE;
		usonic_data.callback (USONIC_CONVERSION (usonic_data.measuredValue));
	}
	return;
}

void usonic_Timeout (void *data, rti_time period, rti_id id)
{
	tim_DisableInterrupts (USONIC_ECHO_TIMER);
	tim_DisableOvfInterrupts (USONIC_ECHO_TIMER);

	usonic_data.stage = IDLE;
	usonic_data.callback (USONIC_INVALID_MEAS);
	
	return;
}