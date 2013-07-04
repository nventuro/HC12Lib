#include "fjoy.h"
#include "rti.h"
#include "error.h"

#define FJOY_READ_CHANNEL(chann, cb) atd_SetTask(FJOY_ATD_MODULE, chann, FJOY_ATD_OVERSAMPLING, _FALSE, _FALSE, cb)

#define FJOY_ATD_FIRST_CHANN FJOY_YAW_CHANN

#define LINEAR_SCALE_U(x,min,max) (((x-min)*(POW_2(ATD_CONV_BITS)-1))/(max-min))
#define LINEAR_SCALE_S(x,min,max,repos) ((x > repos) ? LINEAR_SCALE_S_UPP(x,repos,max) : LINEAR_SCALE_S_LOW(x,repos,min))
#define LINEAR_SCALE_S_UPP(x,repos,max) (((x-repos)*(POW_2(ATD_CONV_BITS-1)-1))/(max-repos))
#define LINEAR_SCALE_S_LOW(x,repos,min) (((x-repos)*(-(POW_2(ATD_CONV_BITS-1))))/(min-repos))

#define SATURATE_U(x,bits) ((x > (POW_2(bits)-1)) ? (POW_2(bits)-1) : ((x < 0) ? 0 : x))
#define SATURATE_S(x,bits) ((x > (POW_2(bits-1)-1)) ? (POW_2(bits-1)-1) : ((x < (-(POW_2(bits-1)))) ? (-(POW_2(bits-1))) : x))


// Calibration

#define YAW_MIN 824
#define YAW_MAX 981
#define YAW_REST 940

#define PITCH_MIN 190
#define PITCH_MAX 881
#define PITCH_REST 500

#define ROLL_MIN 849
#define ROLL_MAX 995
#define ROLL_REST 965

#define ELEV_MIN 294
#define ELEV_MAX 883


struct 
{
	bool button[FJOY_BUTTONS];
	s8 yaw;
	s8 pitch;
	s8 roll;
	u8 elev;
} fjoy_status;

struct {
	fjoy_callback callback[FJOY_MAX_CALLBACKS];
	s32 yawAvg;
	s32 pitchAvg;
	s32 rollAvg;
	s32 elevAvg;
	bool axesRead;
	bool buttonsRead;
	u8 buttonStep;
	u8 buttonCount[FJOY_BUTTONS];
} fjoy_data;

bool fjoy_isInit = _FALSE;

void fjoy_SampleButtons (void *data, rti_time period, rti_id id);
void fjoy_UpdateStatus (void *data, rti_time period, rti_id id);
void fjoy_ATDCallback (s16* mem, const struct atd_task* taskData);

void fjoy_Init(void)
{
	u8 i;
	
	if (fjoy_isInit == _TRUE)
		return;
	
	
	for (i = 0; i < FJOY_MAX_CALLBACKS; i++)
		fjoy_data.callback[i] = NULL;


	FJOY_B6_DDR = 0;
	FJOY_B7_DDR = 0;

	fjoy_data.axesRead = _FALSE;
	//fjoy_data.buttonsRead = _FALSE;
	//fjoy_data.buttonStep = 0;
	
	rti_Init();	
	//rti_Register(fjoy_SampleButtons, NULL, RTI_MS_TO_TICKS(FJOY_BUTTON_SAMPLE_PERIOD_MS), RTI_NOW); 
	
	atd_Init(FJOY_ATD_MODULE);
	FJOY_READ_CHANNEL(FJOY_ATD_FIRST_CHANN, fjoy_ATDCallback);

	while (fjoy_data.axesRead == _FALSE)// || (fjoy_data.buttonsRead == _FALSE))
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
		
			fjoy_data.yawAvg = 0;
			for (i = 0; i < FJOY_ATD_OVERSAMPLING; i++)
				fjoy_data.yawAvg += mem[i];
			fjoy_data.yawAvg /= FJOY_ATD_OVERSAMPLING;
							
			FJOY_READ_CHANNEL(FJOY_PITCH_CHANN, fjoy_ATDCallback);
			
			break;
		case FJOY_PITCH_CHANN:
		
			fjoy_data.pitchAvg = 0;
			for (i = 0; i < FJOY_ATD_OVERSAMPLING; i++)
				fjoy_data.pitchAvg += mem[i];
			fjoy_data.pitchAvg /= FJOY_ATD_OVERSAMPLING;

			FJOY_READ_CHANNEL(FJOY_ROLL_CHANN, fjoy_ATDCallback);
			
			break;
		case FJOY_ROLL_CHANN:
		
			fjoy_data.rollAvg = 0;
			for (i = 0; i < FJOY_ATD_OVERSAMPLING; i++)
				fjoy_data.rollAvg += mem[i];
			fjoy_data.rollAvg /= FJOY_ATD_OVERSAMPLING;
			
			FJOY_READ_CHANNEL(FJOY_ELEV_CHANN, fjoy_ATDCallback);
			
			break;
		case FJOY_ELEV_CHANN:
		
			fjoy_data.elevAvg = 0;
			for (i = 0; i < FJOY_ATD_OVERSAMPLING; i++)
				fjoy_data.elevAvg += mem[i];
			fjoy_data.elevAvg /= FJOY_ATD_OVERSAMPLING;
			// The elevation  potentiometer is inverted, this fixes that
			fjoy_data.elevAvg = POW_2(ATD_CONV_BITS) - 1 - fjoy_data.elevAvg;
			
			fjoy_data.axesRead = _TRUE;
			
			break;
	}
}

void fjoy_SampleButtons (void *data, rti_time period, rti_id id)
{
	u8 i;

	if (fjoy_data.buttonsRead == _TRUE)
		return;
	
	fjoy_data.buttonCount[FJOY_B6] += FJOY_B6_PIN;
	fjoy_data.buttonCount[FJOY_B7] += FJOY_B7_PIN;
	
	fjoy_data.buttonStep++;
	if (fjoy_data.buttonStep == FJOY_BUTTON_OVERSAMPLING)
		fjoy_data.buttonsRead = _TRUE;
}

void fjoy_UpdateStatus (void *data, rti_time period, rti_id id)
{
	u8 i;
	
	if (fjoy_data.axesRead == _FALSE)
		err_Throw("fjoy: axes sampling frequency is too low.\n");
	
/*	if (fjoy_data.buttonsRead == _FALSE)
		err_Throw("fjoy: button sampling frequency is too low.\n");
	
	for (i = 0; i < FJOY_BUTTONS; i++)
	{
		if (fjoy_data.buttonCount[i] > (FJOY_BUTTON_OVERSAMPLING/2))
			fjoy_status.button[i] = _TRUE;
		else
			fjoy_status.button[i] = _FALSE;
		
		fjoy_data.buttonCount[i] = 0;
	}*/
	
	// Scaling
	fjoy_data.yawAvg = LINEAR_SCALE_S(fjoy_data.yawAvg, YAW_MIN, YAW_MAX, YAW_REST);
	fjoy_data.rollAvg = LINEAR_SCALE_S(fjoy_data.rollAvg, ROLL_MIN, ROLL_MAX, ROLL_REST);
	fjoy_data.pitchAvg = LINEAR_SCALE_S(fjoy_data.pitchAvg, PITCH_MIN, PITCH_MAX, PITCH_REST);
	fjoy_data.elevAvg = LINEAR_SCALE_U(fjoy_data.elevAvg, ELEV_MIN, ELEV_MAX);
	
	// Reduce resolution to the corresponding number of bits
	fjoy_data.yawAvg = fjoy_data.yawAvg / POW_2(ATD_CONV_BITS-FJOY_YAW_BITS);
	fjoy_data.pitchAvg = fjoy_data.pitchAvg / POW_2(ATD_CONV_BITS-FJOY_PITCH_BITS);
	fjoy_data.rollAvg = fjoy_data.rollAvg / POW_2(ATD_CONV_BITS-FJOY_ROLL_BITS);
	fjoy_data.elevAvg = fjoy_data.elevAvg / POW_2(ATD_CONV_BITS-FJOY_ELEV_BITS); 

	// Saturation (calibration errors may cause the final values to lie outside their valid range.
	fjoy_status.yaw = SATURATE_S(fjoy_data.yawAvg, FJOY_YAW_BITS);
	fjoy_status.roll = SATURATE_S(fjoy_data.rollAvg, FJOY_PITCH_BITS);
	fjoy_status.pitch = SATURATE_S(fjoy_data.pitchAvg, FJOY_ROLL_BITS);
	fjoy_status.elev = SATURATE_U(fjoy_data.elevAvg, FJOY_ELEV_BITS);
	

	for (i = 0; i < FJOY_MAX_CALLBACKS; i++)
		if (fjoy_data.callback[i] != NULL)
			(*fjoy_data.callback[i]) ();
		
	fjoy_data.axesRead = _FALSE;
//	fjoy_data.buttonsRead = _FALSE;
//	fjoy_data.buttonStep = 0;
		
	FJOY_READ_CHANNEL(FJOY_ATD_FIRST_CHANN, fjoy_ATDCallback);
	
	return;
}


