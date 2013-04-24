#include "mag_definitions.h"
#include "mag.h"
#include "rti.h"
#include "error.h"
#include <stdio.h>

struct mag_data_T
{
	bool init;
	
	iic_ptr userCb;
	u8 stage;
}mag_data = {_FALSE, NULL, 0};



struct mag_measurements_T mag_measurements;
struct mag_sampleAccumulator_T mag_sampleAccumulator = {0,0,0,0};

u8 temperature;


#define mag_Send(eotCB, commFailedCB, toWrite, sendBuffer)	\
	(iic_Send (MAG_ADDRESS, eotCB, commFailedCB, toWrite, sendBuffer) )

#define mag_ReceiveFromRegister(regAddress, eotCB, commFailedCB, toRead, receiveBuffer) \
	(iic_ReceiveFromRegister (regAddress, MAG_ADDRESS, eotCB, commFailedCB, toRead, receiveBuffer))


void mag_StagesInit(void);
void mag_CommFailed(void);


void mag_Init(void)
{	
	u16 offsetSampleRate;
	u8 offsetTask;
	
	if (mag_data.init == _TRUE)
		return;
	
	iic_Init();
	
	mag_StagesInit();
	
	while (mag_data.init == _FALSE)
		;
		
	rti_Init();
	
	offsetSampleRate = (((1000/MAG_SAMPLE_RATE) < 20) ? (20) : (1000/MAG_SAMPLE_RATE));
	
//	offsetTask = rti_Register(mag_GetAndAccMeasurements, NULL, RTI_MS_TO_TICKS(offsetSampleRate), RTI_MS_TO_TICKS(2*offsetSampleRate));
	
//	while (mag_sampleAccumulator.numberOfSamples < MAG_INITIAL_AVERAGE)
//		;


	return;	
}


void mag_StagesInit(void)
{	
	switch (mag_data.stage)
	{	
	case 0:
			
		iic_commData.data[0] = ADD_MAG_CTRL_1;
		iic_commData.data[1] = MAG_CTRL_1_INIT;
		iic_commData.data[2] = MAG_CTRL_2_INIT;

		mag_Send (mag_StagesInit, mag_CommFailed, 3, NULL);
		
		mag_data.stage++;
		
		break;

	case 2:
				
		mag_data.init = _TRUE;
		mag_data.stage = 0;
		
		break;
		
	default: 
		break;
	}
		
	return;
}



void mag_GetMeasurements(iic_ptr cb)
{
	mag_ReceiveFromRegister(ADD_MAG_DATA_STATUS, cb, mag_CommFailed, sizeof(struct mag_measurements_T), (u8*)&mag_measurements);
	return;
}


void mag_CorrectOffset(s16 xOff, s16 yOff, s16 zOff, iic_ptr cb)
{
	iic_commData.data[0] = ADD_MAG_OFFSET;
	((u16*)(iic_commData.data + sizeof(u8)))[1] = xOff;
	((u16*)(iic_commData.data + sizeof(u8)))[2] = yOff;
	((u16*)(iic_commData.data + sizeof(u8)))[3] = zOff;
	
	mag_Send(cb, NULL, 3*sizeof(s16) + sizeof(u8), NULL);
	return;
}


void mag_GetTemperature(iic_ptr cb)
{
	mag_ReceiveFromRegister(ADD_MAG_TEMP, cb, mag_CommFailed, sizeof(u8), &temperature);
	return;
}


void mag_PrintFormattedMeasurements(void)
{
	struct mag_measurements_T* m = &mag_measurements;
	printf("mx: %d, my: %d, mz: %d\n", m->mField.x, m->mField.y, m->mField.z);
	return;
}


void mag_PrintRawMeasurements(void)
{
	struct mag_measurements_T* m = &mag_measurements;
	printf("%d %d %d,", m->mField.x, m->mField.y, m->mField.z);
	return;
}


void mag_CommFailed(void)
{
	return;
}
