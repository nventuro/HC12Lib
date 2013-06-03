#include "fjoy.h"
#include "rti.h"
#include "error.h"
#include <stdio.h>

#define FJOY_READ_CHANNEL(chann, cb) atd_SetTask(FJOY_ATD_MODULE, chann, FJOY_ATD_OVERSAMPLING, _FALSE, _FALSE, cb)

void fjoy_UpdateStatus (void *data, rti_time period, rti_id id);
void fjoy_ATDCallback (s16* mem, const struct atd_task* taskData);

#define FJOY_ATD_FIRST_CHANN FJOY_YAW_CHANN

#define LINEAR_SCALE_U(x,min,max,bits) (((x-min)*(POW_2(bits)-1))/(max-min))
#define LINEAR_SCALE_S(x,min,max,repos,bits) ((x > repos) ? LINEAR_SCALE_S_UPP(x,repos,max,bits) : LINEAR_SCALE_S_LOW(x,repos,min,bits))
#define LINEAR_SCALE_S_UPP(x,repos,max,bits) ((((x-repos)*(POW_2(bits-1)-1)))/(max-repos))
#define LINEAR_SCALE_S_LOW(x,repos,min,bits) (((x-repos)*(-(POW_2(bits-1))))/(min-repos))

#define SATURATE_U(x,bits) ((x > (POW_2(bits)-1)) ? (POW_2(bits)-1) : ((x < 0) ? 0 : x))
#define SATURATE_S(x,bits) ((x > (POW_2(bits-1)-1)) ? (POW_2(bits-1)-1) : ((x < (-(POW_2(bits-1)))) ? (-(POW_2(bits-1))) : x))


// Calibration

#define YAW_MIN 78/4
#define YAW_MAX 117/4
#define YAW_REST 106/4

#define PITCH_MIN -81/4
#define PITCH_MAX 92/4
#define PITCH_REST 0

#define ROLL_MIN 82/4
#define ROLL_MAX 121/4
#define ROLL_REST 114/4

#define ELEV_MIN 38
#define ELEV_MAX 109


struct {
	fjoy_callback callback[FJOY_MAX_CALLBACKS];
	s32 yawSum;
	s32 pitchSum;
	s32 rollSum;
	s32 elevSum;
	bool axesRead;
} fjoy_data;

bool fjoy_isInit = _FALSE;


void fjoy_Init(void)
{
	u8 i;
	
	if (fjoy_isInit == _TRUE)
		return;
	
	
	for (i = 0; i < FJOY_MAX_CALLBACKS; i++)
		fjoy_data.callback[i] = NULL;

	rti_Init();	
	//rti_Register(fjoy_UpdateStatus, NULL, RTI_MS_TO_TICKS(FJOY_SAMPLE_PERIOD_MS), RTI_NOW); BUTTON SMAPLING FUNCT
	
	atd_Init(FJOY_ATD_MODULE);
	fjoy_data.axesRead = _FALSE;
	FJOY_READ_CHANNEL(FJOY_ATD_FIRST_CHANN, fjoy_ATDCallback);

	while (fjoy_data.axesRead == _FALSE) // && !buttons_read
		;
	
	rti_Register(fjoy_UpdateStatus, NULL, RTI_MS_TO_TICKS(FJOY_SAMPLE_PERIOD_MS), RTI_NOW);
	
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

void fjoy_ATDCallback (s16* mem, const atd_task* taskData)
{
	u8 i;

	switch (taskData->channel)
	{
		case FJOY_YAW_CHANN:
		
			fjoy_data.yawSum = 0;
			for (i = 0; i < FJOY_ATD_OVERSAMPLING; i++)
				fjoy_data.yawSum += mem[i];
				
			FJOY_READ_CHANNEL(FJOY_PITCH_CHANN, fjoy_ATDCallback);
			
			break;
		case FJOY_PITCH_CHANN:
		
			fjoy_data.pitchSum = 0;
			for (i = 0; i < FJOY_ATD_OVERSAMPLING; i++)
				fjoy_data.pitchSum += mem[i];
				
			FJOY_READ_CHANNEL(FJOY_ROLL_CHANN, fjoy_ATDCallback);
			
			break;
		case FJOY_ROLL_CHANN:
		
			fjoy_data.rollSum = 0;
			for (i = 0; i < FJOY_ATD_OVERSAMPLING; i++)
				fjoy_data.rollSum += mem[i];
				
			FJOY_READ_CHANNEL(FJOY_ELEV_CHANN, fjoy_ATDCallback);
			
			break;
		case FJOY_ELEV_CHANN:
		
			fjoy_data.elevSum = 0;
			for (i = 0; i < FJOY_ATD_OVERSAMPLING; i++)
				fjoy_data.elevSum += mem[i];
				
			fjoy_data.axesRead = _TRUE;
			
			break;
	}
}
#include <stdio.h>

void fjoy_UpdateStatus (void *data, rti_time period, rti_id id)
{
	u8 i;
	
	if (fjoy_data.axesRead == _FALSE)
		err_Throw("fjoy: axes sampling frequency is too low.\n");
	
	// do stuff (average samples of buttons and analog inputs, scale inputs)
	
	
	// Division is required since the inputs are sampled at 10 bits resolution (which must be lowered to the different amounts of bits)
	// Yaw, pitch and roll are s8, so the lower bound must be substracted from them (so that zero maps to that value)
	fjoy_data.yawSum = fjoy_data.yawSum / (FJOY_ATD_OVERSAMPLING * POW_2(10-FJOY_YAW_BITS)) - POW_2(FJOY_YAW_BITS-1);
	fjoy_data.pitchSum = fjoy_data.pitchSum / (FJOY_ATD_OVERSAMPLING * POW_2(10-FJOY_PITCH_BITS)) - POW_2(FJOY_PITCH_BITS-1);
	fjoy_data.rollSum = fjoy_data.rollSum / (FJOY_ATD_OVERSAMPLING * POW_2(10-FJOY_ROLL_BITS)) - POW_2(FJOY_ROLL_BITS-1);
	// Elevation potentiometer is u8, but the potentiometer is inverted, substracting the measurement from the upper bound fixes that
	fjoy_data.elevSum = POW_2(FJOY_ELEV_BITS) - 1 - (fjoy_data.elevSum / (FJOY_ATD_OVERSAMPLING * POW_2(10-FJOY_ELEV_BITS))); 

	printf("y %ld p %ld r %ld e %ld\n", fjoy_data.yawSum, fjoy_data.pitchSum, fjoy_data.rollSum, fjoy_data.elevSum);
	// Scaling and saturation
	fjoy_data.yawSum = LINEAR_SCALE_S(fjoy_data.yawSum, YAW_MIN, YAW_MAX, YAW_REST, FJOY_YAW_BITS);
	fjoy_status.yaw = SATURATE_S(fjoy_data.yawSum, FJOY_YAW_BITS);

	fjoy_data.rollSum = LINEAR_SCALE_S(fjoy_data.rollSum, ROLL_MIN, ROLL_MAX, ROLL_REST, FJOY_PITCH_BITS);
	fjoy_status.roll = SATURATE_S(fjoy_data.rollSum, FJOY_PITCH_BITS);
	
	fjoy_data.pitchSum = LINEAR_SCALE_S(fjoy_data.pitchSum, PITCH_MIN, PITCH_MAX, PITCH_REST, FJOY_ROLL_BITS);
	fjoy_status.pitch = SATURATE_S(fjoy_data.pitchSum, FJOY_ROLL_BITS);
	
	fjoy_data.elevSum = LINEAR_SCALE_U(fjoy_data.elevSum, ELEV_MIN, ELEV_MAX, FJOY_ELEV_BITS);
	fjoy_status.elev = SATURATE_U(fjoy_data.elevSum, FJOY_ELEV_BITS);
	

	for (i = 0; i < FJOY_MAX_CALLBACKS; i++)
		if (fjoy_data.callback[i] != NULL)
			(*fjoy_data.callback[i]) ();
		
	fjoy_data.axesRead = _FALSE;
	FJOY_READ_CHANNEL(FJOY_ATD_FIRST_CHANN, fjoy_ATDCallback);
	
	return;
}


