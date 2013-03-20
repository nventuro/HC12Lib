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
		u8 avgDiscard;
		iic_ptr stageCb; 		
	}fifo;
};

/*	Functions needed by the macros, that should not be used by user. */
extern void dmu_PrintFifoMem(void);
extern void dmu_FifoAverageInit(void);
extern void dmu_CommFailed(void);
extern void dmu_PrintFormattedMeasurements(void);


void dmu_Init(void);
/* 	Initializes module in stages. This function must be used before getting any measurement.
	All the configuration of the module can be set in dmu_definitions.h (see the HEADER for each register to configurate).
	The function also averages some samples in order to estimate gyro offset.
*/

void dmu_GetMeasurements(iic_ptr cb);	
/*	Fetches current device's sample, and saves it in global variable dmu_measurements.
	Calls _cb when transfer is finished.
	@param _cb: iic_ptr.
*/


#define dmu_ReadFifo(_cb)	do {				\
	dmu_data.stage = 0;							\
	dmu_data.userCb = _cb;						\
	dmu_data.fifo.stageCb = dmu_PrintFifoMem;	\
												\
	dmu_FifoStageRead();						\
} while(0)
/*	Prints fifo data, and calls _cb after finished.
	@param _cb: iic_ptr.
*/


#define dmu_FifoAverage(_cb) do						\
{													\
	dmu_data.stage = 0;								\
	dmu_data.userCb = _cb;							\
	dmu_data.fifo.stageCb = dmu_FifoAverageInit;	\
													\
	dmu_FifoStageRead();							\
}while(0)
/*	Averages fifo values, and calls _cb when finished. Results are saved in dmu_sampleAccumulator.
	@param _cb: iic_ptr.
*/


#define dmu_FifoReset(cb) do {												\
	iic_commData.data[0] = ADD_USER_CTRL;									\
	iic_commData.data[1] = USER_CTRL(FIFO_MASTER_ENABLE, FIFO_RESET, 1);	\
	dmu_Send(cb, dmu_CommFailed, 2, NULL);											\
} while(0)
/*	Resets fifo and calls _cb.
	@param: iic_ptr.
*/

#endif // _DMU_H_