#include "timers.h"
#include "error.h"

#define	TIMER_PRESCALER 6 	// 50MHz / 2^6 = 781.25 kHz. The TCNT resolution is 1.28us 
#define TIM_AMOUNT 8

#define SET_TIOS_OC(i) (TIOS |= (1 << i))
#define SET_TIOS_IC(i) (TIOS &= ~(1 << i))

#define CLEAR_OVF_FLAG() (TFLG2_TOF = 1)

struct {
	bool isTimerUsed[TIM_AMOUNT];
	tim_ptr cbArray[TIM_AMOUNT];
	tim_ptr ovfArray[TIM_AMOUNT];
	bool ovfIntEnable[TIM_AMOUNT];
} tim_data;

bool tim_isInit = _FALSE;

void tim_AssignTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf, tim_id i);

void tim_Init(void) 
{
	tim_id i;
	
	if (tim_isInit == _TRUE)
		return;
	
	tim_isInit = _TRUE;
	

	for (i = 0; i < TIM_AMOUNT; i++)
	{
		tim_data.isTimerUsed[i] = _FALSE;
		tim_data.cbArray[i] = NULL;		
		tim_data.ovfArray[i] = NULL;
		tim_data.ovfIntEnable[i] = _FALSE;
		tim_dDisableInterrupts(i);			
	}
	
	TIOS = 0x00; // Input Capture by default
	TSCR2_PR = TIMER_PRESCALER;
	TSCR2_TOI = 1;
	TSCR1_TEN = 1;		
	
	return;
}

void tim_GetTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf, tim_id timNumber)
{
	if ((tim_data.isTimerUsed[timNumber] == _TRUE) || (cb == NULL))
		err_Throw("timer: attempted to reserve a timer that's already in use.\n");
	
	if ((timNumber < 0) || (timNumber > 7))
		err_Throw("timer: attempted to reserve an invalid timer number.\n");
	
	tim_AssignTimer(reqType, cb, ovf, timNumber);
	
	return;	
}


tim_id tim_GetFreeTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf)
{
	tim_id i;

	if (cb == NULL)
		err_Throw("timer: null callback received.\n");

	for (i = 0; i < TIM_AMOUNT; i++)
		if (tim_data.isTimerUsed[i] == _FALSE)
		{
			tim_AssignTimer(reqType, cb, ovf, i);
			break;
		}
		
	if (i == TIM_AMOUNT)
		err_Throw("timer: no more free timers.\n");
	
	return i;	
}

void tim_AssignTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf, tim_id i)
{
	tim_dDisableInterrupts(i);
	tim_ClearFlag(i);
	
	tim_data.isTimerUsed[i] = _TRUE;
	tim_data.cbArray[i] = cb;
	tim_data.ovfArray[i] = ovf;
	tim_data.ovfIntEnable[i] = _FALSE;
	
	if (reqType == TIM_OC)
		SET_TIOS_OC(i);
	else
		SET_TIOS_IC(i);
}

void tim_FreeTimer(tim_id timId)
{
	tim_dDisableInterrupts(timId);
	tim_ClearFlag(timId);
	
	tim_data.isTimerUsed[timId] = _FALSE;
	tim_data.ovfIntEnable[timId] = _FALSE;
	tim_data.cbArray[timId] = NULL;
	tim_data.ovfArray[timId] = NULL;
	
	return;
}

void tim_dEnableInterrupts(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TIE_C0I = 1;
			break;
		case 1:
			TIE_C1I = 1;
			break;
		case 2:
			TIE_C2I = 1;
			break;
		case 3:
			TIE_C3I = 1;
			break;
		case 4:
			TIE_C4I = 1;
			break;
		case 5:
			TIE_C5I = 1;
			break;
		case 6:
			TIE_C6I = 1;
			break;
		case 7:
			TIE_C7I = 1;
			break;
	}
			
	return;
}

void tim_dDisableInterrupts(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TIE_C0I = 0;
			break;
		case 1:
			TIE_C1I = 0;
			break;
		case 2:
			TIE_C2I = 0;
			break;
		case 3:
			TIE_C3I = 0;
			break;
		case 4:
			TIE_C4I = 0;
			break;
		case 5:
			TIE_C5I = 0;
			break;
		case 6:
			TIE_C6I = 0;
			break;
		case 7:
			TIE_C7I = 0;
			break;
	}
	
	return;
}

bool tim_dAreInterruptsEnabled (tim_id timId)
{
	switch (timId)
	{
		case 0:
			if (TIE_C0I == 0)
				return _FALSE;
			else
				return _TRUE;
		case 1:
			if (TIE_C1I == 0)
				return _FALSE;
			else
				return _TRUE;
		case 2:
			if (TIE_C2I == 0)
				return _FALSE;
			else
				return _TRUE;
		case 3:
			if (TIE_C3I == 0)
				return _FALSE;
			else
				return _TRUE;
		case 4:
			if (TIE_C4I == 0)
				return _FALSE;
			else
				return _TRUE;
		case 5:
			if (TIE_C5I == 0)
				return _FALSE;
			else
				return _TRUE;
		case 6:
			if (TIE_C6I == 0)
				return _FALSE;
			else
				return _TRUE;
		case 7:
			if (TIE_C7I == 0)
				return _FALSE;
			else
				return _TRUE;
	}
			
	return _FALSE;
}

void tim_EnableOvfInterrupts(tim_id timId)
{
	tim_data.ovfIntEnable[timId] = _TRUE;

	return;
}


void tim_DisableOvfInterrupts(tim_id timId)
{
	tim_data.ovfIntEnable[timId] = _FALSE;

	return;
}


void tim_dClearFlag(tim_id timId)
{
	TFLG1 = 1<<timId;
	
	return;	
}

void tim_dSetFallingEdge(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TCTL4_EDG0A = 0;
			TCTL4_EDG0B = 1;
			break;
		case 1:
			TCTL4_EDG1A = 0;
			TCTL4_EDG1B = 1;
			break;
		case 2:
			TCTL4_EDG2A = 0;
			TCTL4_EDG2B = 1;
			break;
		case 3:
			TCTL4_EDG3A = 0;
			TCTL4_EDG3B = 1;
			break;
		case 4:
			TCTL3_EDG4A = 0;
			TCTL3_EDG4B = 1;
			break;
		case 5:
			TCTL3_EDG5A = 0;
			TCTL3_EDG5B = 1;
			break;
		case 6:
			TCTL3_EDG6A = 0;
			TCTL3_EDG6B = 1;
			break;
		case 7:
			TCTL3_EDG7A = 0;
			TCTL3_EDG7B = 1;
			break;
	}
	
	return;
}


void tim_dSetRisingEdge(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TCTL4_EDG0A = 1;
			TCTL4_EDG0B = 0;
			break;
		case 1:
			TCTL4_EDG1A = 1;
			TCTL4_EDG1B = 0;
			break;
		case 2:
			TCTL4_EDG2A = 1;
			TCTL4_EDG2B = 0;
			break;
		case 3:
			TCTL4_EDG3A = 1;
			TCTL4_EDG3B = 0;
			break;
		case 4:
			TCTL3_EDG4A = 1;
			TCTL3_EDG4B = 0;
			break;
		case 5:
			TCTL3_EDG5A = 1;
			TCTL3_EDG5B = 0;
			break;
		case 6:
			TCTL3_EDG6A = 1;
			TCTL3_EDG6B = 0;
			break;
		case 7:
			TCTL3_EDG7A = 1;
			TCTL3_EDG7B = 0;
			break;
	}
	
	return;
}


void tim_dSetBothEdge(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TCTL4_EDG0A = 1;
			TCTL4_EDG0B = 1;
			break;
		case 1:
			TCTL4_EDG1A = 1;
			TCTL4_EDG1B = 1;
			break;
		case 2:
			TCTL4_EDG2A = 1;
			TCTL4_EDG2B = 1;
			break;
		case 3:
			TCTL4_EDG3A = 1;
			TCTL4_EDG3B = 1;
			break;
		case 4:
			TCTL3_EDG4A = 1;
			TCTL3_EDG4B = 1;
			break;
		case 5:
			TCTL3_EDG5A = 1;
			TCTL3_EDG5B = 1;
			break;
		case 6:
			TCTL3_EDG6A = 1;
			TCTL3_EDG6B = 1;
			break;
		case 7:
			TCTL3_EDG7A = 1;
			TCTL3_EDG7B = 1;
			break;
	}
	
	return;
}


void tim_dSetOutputHigh(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TCTL2_OL0 = 1;
			TCTL2_OM0 = 1;
			break;
		case 1:
			TCTL2_OL1 = 1;
			TCTL2_OM1 = 1;
			break;
		case 2:
			TCTL2_OL2 = 1;
			TCTL2_OM2 = 1;
			break;
		case 3:
			TCTL2_OL3 = 1;
			TCTL2_OM3 = 1;
			break;
		case 4:
			TCTL1_OL4 = 1;
			TCTL1_OM4 = 1;
			break;
		case 5:
			TCTL1_OL5 = 1;
			TCTL1_OM5 = 1;
			break;
		case 6:
			TCTL1_OL6 = 1;
			TCTL1_OM6 = 1;
			break;
		case 7:
			TCTL1_OL7 = 1;
			TCTL1_OM7 = 1;
			break;
	}
	
	return;
}

void tim_dSetOutputLow(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TCTL2_OL0 = 0;
			TCTL2_OM0 = 1;
			break;
		case 1:
			TCTL2_OL1 = 0;
			TCTL2_OM1 = 1;
			break;
		case 2:
			TCTL2_OL2 = 0;
			TCTL2_OM2 = 1;
			break;
		case 3:
			TCTL2_OL3 = 0;
			TCTL2_OM3 = 1;
			break;
		case 4:
			TCTL1_OL4 = 0;
			TCTL1_OM4 = 1;
			break;
		case 5:
			TCTL1_OL5 = 0;
			TCTL1_OM5 = 1;
			break;
		case 6:
			TCTL1_OL6 = 0;
			TCTL1_OM6 = 1;
			break;
		case 7:
			TCTL1_OL7 = 0;
			TCTL1_OM7 = 1;
			break;
	}
	
	return;
}

void tim_dSetOutputToggle(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TCTL2_OL0 = 1;
			TCTL2_OM0 = 0;
			break;
		case 1:
			TCTL2_OL1 = 1;
			TCTL2_OM1 = 0;
			break;
		case 2:
			TCTL2_OL2 = 1;
			TCTL2_OM2 = 0;
			break;
		case 3:
			TCTL2_OL3 = 1;
			TCTL2_OM3 = 0;
			break;
		case 4:
			TCTL1_OL4 = 1;
			TCTL1_OM4 = 0;
			break;
		case 5:
			TCTL1_OL5 = 1;
			TCTL1_OM5 = 0;
			break;
		case 6:
			TCTL1_OL6 = 1;
			TCTL1_OM6 = 0;
			break;
		case 7:
			TCTL1_OL7 = 1;
			TCTL1_OM7 = 0;
			break;
	}
	
	return;
}


void tim_dDisconnectOutput(tim_id timId)
{
	switch (timId)
	{
		case 0:
			TCTL2_OL0 = 0;
			TCTL2_OM0 = 0;
			break;
		case 1:
			TCTL2_OL1 = 0;
			TCTL2_OM1 = 0;
			break;
		case 2:
			TCTL2_OL2 = 0;
			TCTL2_OM2 = 0;
			break;
		case 3:
			TCTL2_OL3 = 0;
			TCTL2_OM3 = 0;
			break;
		case 4:
			TCTL1_OL4 = 0;
			TCTL1_OM4 = 0;
			break;
		case 5:
			TCTL1_OL5 = 0;
			TCTL1_OM5 = 0;
			break;
		case 6:
			TCTL1_OL6 = 0;
			TCTL1_OM6 = 0;
			break;
		case 7:
			TCTL1_OL7 = 0;
			TCTL1_OM7 = 0;
			break;
	}
	
	return;
}

u16 tim_dGetValue(tim_id timId)
{
	switch (timId)
	{
		case 0:
			return TC0;
		case 1:
			return TC1;
		case 2:
			return TC2;
		case 3:
			return TC3;
		case 4:
			return TC4;
		case 5:
			return TC5;
		case 6:
			return TC6;
		case 7:
			return TC7;
	}
}


void tim_dSetValue(tim_id timId, u16 value)
{
	switch (timId)
	{
		case 0:
			TC0 = value;
			break;
		case 1:
			TC1 = value;
			break;
		case 2:
			TC2 = value;
			break;
		case 3:
			TC3 = value;
			break;
		case 4:
			TC4 = value;
			break;
		case 5:
			TC5 = value;
			break;
		case 6:
			TC6 = value;
			break;
		case 7:
			TC7 = value;
			break;
	}
			
	return;
}

u32 tim_dGetTimeElapsed(u16 overflowCnt, tim_id timId, u16 lastEdge)
{
	return ( (overflowCnt * TIM_CNT_MAX + tim_dGetValue(timId)) - lastEdge);
}

void interrupt tim0_Service(void)
{
	tim_ClearFlag(0);

	tim_data.cbArray[0]();
	
	return;
}


void interrupt tim1_Service(void)
{
	tim_ClearFlag(1);
	
	tim_data.cbArray[1]();
	
	return;
}


void interrupt tim2_Service(void)
{
	tim_ClearFlag(2);

	tim_data.cbArray[2]();
	
	return;
}


void interrupt tim3_Service(void)
{
	tim_ClearFlag(3);

	tim_data.cbArray[3]();
	
	return;
}


void interrupt tim4_Service(void)
{
	tim_ClearFlag(4);

	tim_data.cbArray[4]();
	
	return;
}


void interrupt tim5_Service(void)
{
	tim_ClearFlag(5);

	tim_data.cbArray[5]();
	
	return;
}


void interrupt tim6_Service(void)
{
	tim_ClearFlag(6);
	
	tim_data.cbArray[6]();
	
	return;
}


void interrupt tim7_Service(void)
{
	tim_ClearFlag(7);

	tim_data.cbArray[7]();
	
	return;
}


void interrupt timOvf_Service(void)
{
	tim_id i;
	CLEAR_OVF_FLAG();
		
	for (i = 0; i < TIM_AMOUNT; i++)
		if ((tim_data.ovfArray[i] != NULL) && (tim_data.ovfIntEnable[i] == _TRUE))
			(*tim_data.ovfArray[i])();
}