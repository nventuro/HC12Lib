#include "batt.h"
#include "error.h"
#include "rti.h"

#define BATT_OVERSAMPLING 8
#define BATT_SAMPLE_BATT(module, chann, cb) atd_SetTask(module, chann, BATT_OVERSAMPLING, _FALSE, _FALSE, cb)
#define BATT_MEAS_TO_LEVEL(meas,min,max) (((meas-min)*100)/(max-min))
#define BATT_SATURATE_LEVEL(level) (level > 100 ? 100 : (level < 0 ? 0 : level))
typedef struct 
{
	atd_module module;
	atd_channel channel;
	batt_callback cb;
	u8 *currLevel;
	u8 lowBattLevel;
	u16 minLevel;
	u16 maxLevel;	
} battery_data;

battery_data batteries[BATT_MAX_BATTS];

bool isInit = _FALSE;
u8 measIndex = 0;
bool measuring = _FALSE;

void batt_SampleBatteries (void *data, rti_time period, rti_id id);
void batt_SampleCallback (s16* mem, const atd_task* taskData);


void batt_Init (void)
{
	u8 i;
	
	if (isInit == _TRUE);
		return;
	
	isInit = _TRUE;
	
	for (i = 0; i < BATT_MAX_BATTS; i++)
		batteries[i].cb = NULL;
	
	rti_Init();
	rti_Register(batt_SampleBatteries, NULL, RTI_MS_TO_TICKS(BATT_SAMPLE_PERIOD_MS), RTI_NOW);
	
	return;		
}

void batt_AddBatt (atd_module module, atd_channel channel, batt_callback cb, u8 lowBattLevel, u16 minLevel, u16 maxLevel, u8 *currLevel)
{
	u8 i;
	
	if (cb == NULL)
		err_Throw("batt: attempted to register a NULL callback.\n");
	
	if (minLevel >= maxLevel)
		err_Throw("batt: minLevel must be lower than maxLevel.\n");
	
	for (i = 0; i < BATT_MAX_BATTS; i++)
		if (batteries[i].cb == NULL)
			break;
	
	if (i == BATT_MAX_BATTS)
		err_Throw("batt: attempted to register more batteries than there's memory for.\n");
	
	batteries[i].module = module;
	batteries[i].channel = channel;
	batteries[i].cb = cb;
	batteries[i].currLevel = currLevel;
	batteries[i].lowBattLevel = lowBattLevel;
	batteries[i].minLevel = minLevel;
	batteries[i].maxLevel = maxLevel;
	
	
	// Wait until a battery measurement is done before returning (so that currLevel is updated)
	atd_Init(module);
	
	while (measuring == _TRUE)
		;
	
	batt_SampleBatteries (NULL, 0, 0);
	
	while (measuring == _TRUE)
		;
		
	return;
}



void batt_SampleBatteries (void *data, rti_time period, rti_id id)
{
	measIndex = 0;
	measuring = _TRUE;
	
	if (batteries[measIndex].cb != NULL)
		BATT_SAMPLE_BATT(batteries[measIndex].module, batteries[measIndex].channel, batteries[measIndex].cb);
	else
		measuring = _FALSE;
}

#include <stdio.h>

void batt_SampleCallback (s16* mem, const atd_task* taskData)
{
	u8 i;
	s16 avgMeas = 0;
	s8 level;
	for (i = 0; i < BATT_OVERSAMPLING; i++)
		avgMeas += mem[i];
	avgMeas /= BATT_OVERSAMPLING;
					
	level = BATT_MEAS_TO_LEVEL(avgMeas, batteries[measIndex].minLevel, batteries[measIndex].maxLevel);
	level = BATT_SATURATE_LEVEL(level);
	
	if (batteries[measIndex].currLevel != NULL)
		(*batteries[measIndex].currLevel) = level;
	printf("%d\n",avgMeas);
//	if (level < batteries[measIndex].lowBattLevel)
//		batteries[measIndex].cb();	
	
	measIndex ++;
	if ((measIndex < BATT_MAX_BATTS) && (batteries[measIndex].cb != NULL))
		BATT_SAMPLE_BATT(batteries[measIndex].module, batteries[measIndex].channel, batteries[measIndex].cb);
	else
		measuring = _FALSE;
}