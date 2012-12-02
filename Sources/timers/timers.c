#include "timers.h"
#include "mc9s12xdp512.h"

#define TIM_AMOUNT 8

#define	TIMER_PRESCALER 2 // 40MHz / 2^2 = 10 MHz. The TCNT resolution is 100ns.

#define SET_TIOS_OC(i) (TIOS |= (1 << i))
#define SET_TIOS_IC(i) (TIOS &= ~(1 << i))

#define IS_VALID_ID(id) (((id >= 0) && (id < TIM_AMOUNT)) ? _TRUE : _FALSE)

struct {
	bool isTimerUsed[TIM_AMOUNT];
	tim_ptr cbArray[TIM_AMOUNT];
	tim_ptr ovfArray[TIM_AMOUNT];
	bool ovfIntEnable[TIM_AMOUNT];
} tim_data;

bool tim_isInit = _FALSE;

void assignTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf, tim_id i);
tim_type tim_getType(tim_id id);


void tim_init(void) 
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
		tim_disableInterrupts(i);			
	}
	
	TIOS = 0x00; // Input Capture by default
	TSCR2 |= TIMER_PRESCALER;
	TSCR2_TOI = 1;
	TSCR1_TEN = 1;		
	
	return;
}

tim_id tim_getSpecificTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf, tim_id timNumber)
{
	if (tim_data.isTimerUsed[timNumber] == _TRUE)
		return TIM_INVALID_ID;
	
	assignTimer(reqType, cb, ovf, timNumber);
	
	return timNumber;
}


tim_id tim_getTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf)
{
	tim_id i;
	for (i = 0; i < TIM_AMOUNT; i++)
		if (tim_data.isTimerUsed[i] == _FALSE)
		{
			assignTimer(reqType, cb, ovf, i);
			break;
		}
		
	if (i == TIM_AMOUNT)
		i = TIM_INVALID_ID;
	
	return i;	
}

void assignTimer(tim_type reqType, tim_ptr cb, tim_ptr ovf, tim_id i)
{
	tim_disableInterrupts(i);
	tim_clearFlag(i);
	
	tim_data.isTimerUsed[i] = _TRUE;
	tim_data.cbArray[i] = cb;
	tim_data.ovfArray[i] = ovf;
	tim_data.ovfIntEnable[i] = _FALSE;
	
	if (reqType == TIM_OC)
		SET_TIOS_OC(i);
	else
		SET_TIOS_IC(i);
}

void tim_freeTimer(tim_id timId)
{
	if (!IS_VALID_ID(timId))
		return;
	
	tim_disableInterrupts(timId);
	tim_clearFlag(timId);
	
	tim_data.isTimerUsed[timId] = _FALSE;
	tim_data.ovfIntEnable[timId] = _FALSE;
	tim_data.cbArray[timId] = NULL;
	tim_data.ovfArray[timId] = NULL;
	
	return;
}

tim_type tim_getType(tim_id id)
{
	return ((TIOS & (1 << id)) ? TIM_OC : TIM_IC);
}


void tim_setFallingEdge(tim_id timId)
{
	if(!IS_VALID_ID(timId) || tim_getType(timId) == TIM_OC)
		return;
	
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


void tim_setRisingEdge(tim_id timId)
{
	if(!IS_VALID_ID(timId) || tim_getType(timId) == TIM_OC)
		return;
	
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


void tim_setBothEdge(tim_id timId)
{
	if(!IS_VALID_ID(timId) || tim_getType(timId) == TIM_OC)
		return;
	
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


void tim_setOutputHigh(tim_id timId)
{
	if(!IS_VALID_ID(timId) || tim_getType(timId) == TIM_IC)
		return;
	
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

void tim_setOutputLow(tim_id timId)
{
	if(!IS_VALID_ID(timId) || tim_getType(timId) == TIM_IC)
		return;
	
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

void tim_setOutputToggle(tim_id timId)
{
	if(!IS_VALID_ID(timId) || tim_getType(timId) == TIM_IC)
		return;
	
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


void tim_disconnectOutput(tim_id timId)
{
	if(!IS_VALID_ID(timId) || tim_getType(timId) == TIM_IC)
		return;
	
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


void tim_enableInterrupts(tim_id timId)
{
	if(!IS_VALID_ID(timId))
		return;
	
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


void tim_disableInterrupts(tim_id timId)
{
	if(!IS_VALID_ID(timId))
		return;
	
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

bool tim_areInterruptsEnabled (tim_id timId)
{
	if(!IS_VALID_ID(timId))
		return _FALSE;
	
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

void tim_enableOvfInterrupts(tim_id timId)
{
	if (!IS_VALID_ID(timId))
		return;
	
	tim_data.ovfIntEnable[timId] = _TRUE;

	return;
}


void tim_disableOvfInterrupts(tim_id timId)
{
	if (!IS_VALID_ID(timId))
		return;
	
	tim_data.ovfIntEnable[timId] = _FALSE;

	return;
}


void tim_clearFlag(tim_id timId)
{
	if(!IS_VALID_ID(timId))
		return;
	
	TFLG1 = 1<<timId;
	
	return;	
}


u16 tim_getValue(tim_id timId)
{
	if(!IS_VALID_ID(timId))
		return 0;
	
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


void tim_setValue(tim_id timId, u16 value)
{
	if(!IS_VALID_ID(timId))
		return;
	
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


u16 tim_getGlobalValue(void)
{
	return TCNT;
}


void interrupt tim0_srv(void)
{
	tim_clearFlag(0);

	if (tim_data.cbArray[0] != NULL)
		(*tim_data.cbArray[0])();
	
	return;
}


void interrupt tim1_srv(void)
{
	tim_clearFlag(1);

	if (tim_data.cbArray[1] != NULL)
		(*tim_data.cbArray[1])();
	
	return;
}


void interrupt tim2_srv(void)
{
	tim_clearFlag(2);

	if (tim_data.cbArray[2] != NULL)
		(*tim_data.cbArray[2])();
	
	return;
}


void interrupt tim3_srv(void)
{
	tim_clearFlag(3);

	if (tim_data.cbArray[3] != NULL)
		(*tim_data.cbArray[3])();
	
	return;
}


void interrupt tim4_srv(void)
{
	tim_clearFlag(4);

	if (tim_data.cbArray[4] != NULL)
		(*tim_data.cbArray[4])();
	
	return;
}


void interrupt tim5_srv(void)
{
	tim_clearFlag(5);

	if (tim_data.cbArray[5] != NULL)
		(*tim_data.cbArray[5])();
	
	return;
}


void interrupt tim6_srv(void)
{
	tim_clearFlag(6);

	if (tim_data.cbArray[6] != NULL)
		(*tim_data.cbArray[6])();
	
	return;
}


void interrupt tim7_srv(void)
{
	tim_clearFlag(7);

	if (tim_data.cbArray[7] != NULL)
		(*tim_data.cbArray[7])();
	
	return;
}


void interrupt timOvf_srv(void)
{
	tim_id i;
	TFLG2_TOF = 1;
	
	for (i = 0; i < TIM_AMOUNT; i++)
		if ((tim_data.ovfArray[i] != NULL) && (tim_data.ovfIntEnable[i] == _TRUE))
			(*tim_data.ovfArray[i])();
}