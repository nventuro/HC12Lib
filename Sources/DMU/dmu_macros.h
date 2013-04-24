#ifndef _DMU_MACROS_H_INCLUDED_
#define _DMU_MACROS_H_INCLUDED_

#include "common.h"
#include "iic.h"

#define MPU_ADDRESS 0b01101000

struct dmu_data_T
{
	bool init;
	
	iic_ptr userCb;
	u8 stage;
	
	struct
	{
		bool enable;
		s8 fetchTimes;
		u8 remainingBytes;
		u16 count;
		u8 avgDiscard;
		iic_ptr stageCb; 		
	}fifo;
};

extern struct dmu_data_T dmu_data;


extern void dmu_FifoStageRead(void);


#define dmu_Send(eotCB, commFailedCB, toWrite, sendBuffer)	\
	(iic_Send (MPU_ADDRESS, eotCB, commFailedCB, toWrite, sendBuffer) )

#define dmu_ReceiveFromRegister(regAddress, eotCB, commFailedCB, toRead, receiveBuffer) \
	(iic_ReceiveFromRegister (regAddress, MPU_ADDRESS, eotCB, commFailedCB, toRead, receiveBuffer))


#define dmu_ContinueFifoAction() do		\
{										\
	if (dmu_data.fifo.fetchTimes >= 0)	\
	{									\
		dmu_FifoStageRead();			\
		return;							\
	}									\
	else								\
		dmu_data.fifo.enable = _TRUE;	\
}while(0)


#define dmu_GetIterationLimit() ((dmu_data.fifo.fetchTimes < 0) ? (dmu_data.fifo.remainingBytes) : (MAX_BURST_READS) )
		

#endif // _DMU_MACROS_H_INCLUDED_
