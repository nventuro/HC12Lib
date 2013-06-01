#include "fjoy.h"
#include "rti.h"
#include "atd.h"
#include "error.h"

void fjoy_PeriodicCall (void *data, rti_time period, rti_id id);

struct {
	fjoy_callback callback[FJOY_MAX_CALLBACKS];
} fjoy_data;

bool fjoy_isInit = _FALSE;


void fjoy_Init(void)
{
	u8 i;
	
	if (fjoy_isInit == _TRUE)
		return;
	
	fjoy_isInit = _TRUE;
	
	
	for (i = 0; i < FJOY_MAX_CALLBACKS; i++)
		fjoy_data.callback[i] = NULL;
		
	rti_Init();	
	rti_Register(fjoy_PeriodicCall, NULL, RTI_MS_TO_TICKS(FJOY_SAMPLE_PERIOD_MS), RTI_NOW);
	
	atd_Init(ATD0);
	
	return;
}

void fjoy_PeriodicCall (void *data, rti_time period, rti_id id)
{
	// do stuff (average samples of buttons and analog inputs, scale inputs)
	
	// call registered callbacks
	
	return;
}


void fjoy_CallOnUpdate(fjoy_callback cb)
{
	u8 i;
	
	if (cb == NULL)
		err_Throw("fjoy: attempt to register a NULL callback.\n");
	
	for (i = 0; i < FJOY_MAX_CALLBACKS; i++)
		if 	(fjoy_data.callback[i] == NULL)
			break;
	
	if (i == FJOY_MAX_CALLBACKS)
		err_Throw("fjoy: attempt to register more callbacks than there's memory for.\n");
	else
		fjoy_data.callback[i] = cb;
	
	return;
}