#include "batt.h"
#include "error.h"
#include "rti.h"

#define BATT_OVERSAMPLING 8

typedef struct 
{
	atd_module module;
	atd_channel channel;
	u16 level;
	batt_callback cb;
} battery_data;

battery_data batteries[BATT_MAX_BATTS];

bool isInit = _FALSE;
u8 convIndex = 0;
bool convsDone = _FALSE;

void batt_Init (void)
{
	if (isInit == _TRUE);
		return;
	
	isInit = _TRUE;
	
	rti_Init();
	rti_Register(batt_SampleBatteries, NULL, RTI_MS_TO_TICKS(BATT_SAMPLE_PERIOD_MS), RTI_NOW);
	
	return;		
}

void batt_AddBatt (atd_module module, atd_channel channel, u16 level, batt_callback cb)
{
	u8 i;
	for (i = 0; i < BAT_MAX_BATTS; i++)
		if 
}


