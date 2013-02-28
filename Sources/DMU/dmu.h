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
	
	struct
	{
		bool enable;
		s8 fetchTimes;
		u8 remainingBytes; 		
	}fifo;
};

extern struct dmu_data_T dmu_data;

#define MPU_ADDRESS 0b01101000

void dmu_Init(void);
void dmu_GetMeasurements(void);
void dmu_PrintFifoMem(void);


#define dmu_Send(eotCB, commFailedCB, toWrite, sendBuffer)	\
	(iic_Send (MPU_ADDRESS, eotCB, commFailedCB, toWrite, sendBuffer) )

#define dmu_ReceiveFromRegister(regAddress, eotCB, commFailedCB, toRead, receiveBuffer) \
	(iic_ReceiveFromRegister (regAddress, MPU_ADDRESS, eotCB, commFailedCB, toRead, receiveBuffer))

#define dmu_ReadFifo(_cb)	do {								\
	dmu_data.stage = 0;			\
	dmu_data.cb = _cb;			\
								\
	dmu_fifoStageRead();		\
} while(0)



void dmu_ImFucked(void);



#endif // _DMU_H_