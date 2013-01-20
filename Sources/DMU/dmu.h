#ifndef _DMU_H_
#define _DMU_H_

#include "common.h"
#include "dmu_definitions.h"
#include "iic.h"

struct dmu_data_T
{
	bool init;
	
	iic_ptr cb;
	u8 stage;
	
};

#define MPU_ADDRESS 0b01101000

#define dmu_Send(eotCB, commFailedCB, toWrite, sendBuffer)	\
	(iic_Send (MPU_ADDRESS, eotCB, commFailedCB, toWrite, sendBuffer) )

#define dmu_ReceiveFromRegister(regAddress, eotCB, commFailedCB, toRead, receiveBuffer) \
	(iic_ReceiveFromRegister (regAddress, MPU_ADDRESS, eotCB, commFailedCB, toRead, receiveBuffer))



void dmu_Init(void);
bool dmu_GetMeasurements(void);

void dmu_ImFucked(void);



#endif // _DMU_H_