#include "usonic.h"
#include "timers.h"
#include "rti.h"
 
#define USONIC_TRIGG GLUE(PTT_PTT,USONIC_TRIGG_TIMER)
#define USONIC_TRIGG_DDR GLUE(DDRT_DDRT,USONIC_TRIGG_TIMER)

#define USONIC_STARTUP_DELAY_MS 200

#define USONIC_PULSE_TIME_US 10
#define USONIC_SAMPLE_PERIOD_MS 30
#define USONIC_COOLDOWN_MS 3
#define USONIC_TIMEOUT_MS 300

#define USONIC_OUT_OF_RANGE_MS 50
#define USONIC_US_TO_CM(us) (us/58)
#define USONIC_CONVERSION(x) (x < TIM_US_TO_TICKS(USONIC_OUT_OF_RANGE_MS*1e3) ? USONIC_US_TO_CM(TIM_TICKS_TO_US(x)):USONIC_INVALID_MEAS)

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

bool usonic_isInit = _FALSE;

void usonic_TriggerCallback (void);
void usonic_EchoCallback (void);
void usonic_EchoOverflow (void);

void usonic_InitCallback (void *data, rti_time period, rti_id id);
void usonic_SolveTiming (void *data, rti_time period, rti_id id);
void usonic_Timeout (void *data, rti_time period, rti_id id);

void usonic_Init (void)
{
	if (usonic_isInit == _TRUE)
		return;	

	usonic_data.stage = IDLE;
	
	tim_Init();	
	
	tim_GetTimer (TIM_OC, usonic_TriggerCallback, NULL, USONIC_TRIGG_TIMER);
	tim_GetTimer (TIM_IC, usonic_EchoCallback, usonic_EchoOverflow, USONIC_ECHO_TIMER);

	tim_DisconnectOutput(USONIC_TRIGG_TIMER);
	USONIC_TRIGG_DDR = DDR_OUT;
	USONIC_TRIGG = 0;
	
	rti_Init();
	rti_Register(usonic_InitCallback, NULL, RTI_ONCE, RTI_MS_TO_TICKS(USONIC_STARTUP_DELAY_MS));
	
	while (usonic_isInit != _TRUE)
		;
	
	return;
}

void usonic_InitCallback (void *data, rti_time period, rti_id id)
{
	usonic_isInit = _TRUE;
}

bool usonic_Measure (usonic_ptr callback)
{
	if ((usonic_data.stage != IDLE) || (callback == NULL))
		return _FALSE;
	
	usonic_data.callback = callback;
	usonic_data.halfReady = _FALSE;
	
	USONIC_TRIGG = 1;

	tim_SetValue (USONIC_TRIGG_TIMER, tim_GetGlobalValue() + TIM_US_TO_TICKS(USONIC_PULSE_TIME_US));
	tim_ClearFlag (USONIC_TRIGG_TIMER);
	tim_EnableInterrupts (USONIC_TRIGG_TIMER);
	
	rti_Register (usonic_SolveTiming, NULL, RTI_ONCE, RTI_MS_TO_TICKS(USONIC_SAMPLE_PERIOD_MS));
	usonic_data.timeOut = rti_Register(usonic_Timeout, NULL, RTI_ONCE, RTI_MS_TO_TICKS(USONIC_TIMEOUT_MS));
	
	usonic_data.stage = TRIGGERING;
	
	return _TRUE;
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
			
			usonic_data.stage = WAITING_FOR_ECHO_TO_END;
			
			break;
		
		case WAITING_FOR_ECHO_TO_END:
			usonic_data.measuredValue = (usonic_data.overflowCount * (u32)TIM_OVERFLOW_TICKS + tim_GetValue (USONIC_ECHO_TIMER)) - usonic_data.measuredValue;		
			
			tim_DisableInterrupts (USONIC_ECHO_TIMER);
			tim_DisableOvfInterrupts (USONIC_ECHO_TIMER);

			rti_Register (usonic_SolveTiming, NULL, RTI_ONCE, RTI_MS_TO_TICKS(USONIC_COOLDOWN_MS));	
			
			break;
	}

}

void usonic_EchoOverflow (void)
{
	usonic_data.overflowCount++;
}


void usonic_SolveTiming (void *data, rti_time period, rti_id id)
{
	// SolveTiming has to be called twice for a measurement to end: once ~30ms after the trigger pulse, and once ~2ms after the echo ended.
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
	tim_DisableInterrupts (USONIC_TRIGG_TIMER);
	tim_DisableInterrupts (USONIC_ECHO_TIMER);
	tim_DisableOvfInterrupts (USONIC_ECHO_TIMER);

	usonic_data.stage = IDLE;
	usonic_data.callback (USONIC_INVALID_MEAS);
	
	return;
}