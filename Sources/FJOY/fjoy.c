/*#include "fjoy.h"
#include "rti.h"
#include "atd.h"

void fjoy_MeasureButtons (void *data, rti_time period, rti_id id);

void fjoy_Init(void)
{
	rti_Init();
	atd_Init(ATD0);
	
	rti_Register(fjoy_MeasureButtons, NULL, RTI_MS_TO_TICKS(JOY_SAMPLE_PERIOD_MS), RTI_NOW);
	
	return;
}

void fjoy_MeasureButtons (void *data, rti_time period, rti_id id)
{
	u8 i;
	for (i = 0; i < FJOY_BUTTONS; i++)
		//read buttons, e.g. fjoy_data.button[i] = BOOL(PORT).bit i
	
	return;
}*/