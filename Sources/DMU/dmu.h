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
void dmu_GetSamples(void);

extern void dmu_PrintFifoMem(void);
extern void dmu_FifoStageRead(void);

#define dmu_Send(eotCB, commFailedCB, toWrite, sendBuffer)	\
	(iic_Send (MPU_ADDRESS, eotCB, commFailedCB, toWrite, sendBuffer) )

#define dmu_ReceiveFromRegister(regAddress, eotCB, commFailedCB, toRead, receiveBuffer) \
	(iic_ReceiveFromRegister (regAddress, MPU_ADDRESS, eotCB, commFailedCB, toRead, receiveBuffer))

#define dmu_ReadFifo(_cb)	do {	\
	dmu_data.stage = 0;				\
	dmu_data.cb = _cb;				\
									\
	dmu_FifoStageRead();			\
} while(0)

#define dmu_FifoReset(cb) do {												\
	iic_commData.data[0] = ADD_USER_CTRL;									\
	iic_commData.data[1] = USER_CTRL(FIFO_MASTER_ENABLE, FIFO_RESET, 1);	\
	dmu_Send(cb, NULL, 2, NULL);											\
} while(0)


#endif // _DMU_H_