#ifndef _DMU_H_
#define _DMU_H_

#include "dmu_definitions.h"
#include "dmu_macros.h"

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
		iic_ptr stageCb; 		
	}fifo;
};



void dmu_Init(void);
void dmu_GetMeasurements(void);
void dmu_GetSamples(void);

extern void dmu_PrintFifoMem(void);
extern void dmu_FifoAverageInit(void);


#define dmu_ReadFifo(_cb)	do {				\
	dmu_data.stage = 0;							\
	dmu_data.userCb = _cb;							\
	dmu_data.fifo.stageCb = dmu_PrintFifoMem;	\
												\
	dmu_FifoStageRead();						\
} while(0)


#define dmu_FifoAverage(_cb) do						\
{													\
	dmu_data.stage = 0;								\
	dmu_data.userCb = _cb;							\
	dmu_data.fifo.stageCb = dmu_FifoAverageInit;	\
													\
	dmu_FifoStageRead();							\
}while(0)


#define dmu_FifoReset(cb) do {												\
	iic_commData.data[0] = ADD_USER_CTRL;									\
	iic_commData.data[1] = USER_CTRL(FIFO_MASTER_ENABLE, FIFO_RESET, 1);	\
	dmu_Send(cb, NULL, 2, NULL);											\
} while(0)


#endif // _DMU_H_