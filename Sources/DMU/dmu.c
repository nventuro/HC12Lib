#include "dmu.h"
#include <stdio.h>
#include "quick_serial.h"

#define PRINT_START 1
#define PRINT_LENGTH (ADD_WHO_AM_I - PRINT_START)+1

#define MAX_BURST_READS 256

void dmu_printI2CData(void);

extern void printI2CData(void);


// Measurements taken individually must respect MPU internal registers' order
struct dmu_measurements_T
{
	s16 accel_x;
	s16 accel_y;
	s16 accel_z;
	
	s16 temp;
	
	s16 gyro_x;
	s16 gyro_y;
	s16 gyro_z;

} dmu_measurements;

// For samples taken from fifo buffer
struct dmu_samples_T
{
	s16 accel_x;
	s16 accel_y;
	s16 accel_z;
	
	s16 gyro_x;
	s16 gyro_y;
	s16 gyro_z;
};


struct dmu_sampleAccumulator_T
{
	s32 accel_x;
	s32 accel_y;
	s32 accel_z;
	
	s32 gyro_x;
	s32 gyro_y;
	s32 gyro_z;
}dmu_sampleAccumulator;

struct dmu_data_T dmu_data = {_FALSE, NULL, 0, {_TRUE, 0, 0} };

void dmu_PrintFormattedMeasurements(void);
void dmu_FifoStageRead(void);
void dmu_CommFailed(void);
void dmu_printFifoCnt(void);
void dmu_AccumulateSamples(struct dmu_sampleAccumulator_T* acc, struct dmu_samples_T* samples);
void dmu_DivideAccumulator(struct dmu_sampleAccumulator_T* acc, u16 n);
void dmu_AverageSamples(void);
void dmu_StagesInit(void);	


// Reset macro - g, a, t are booleans (1/0), cb is callback after reset is done..
#define dmu_SignalReset(g, a, t, cb) do {			\
	iic_commData.data[0] = ADD_SIGNAL_PATH_RESET;	\
	iic_commData.data[1] = RESET_SIGNAL(g,a,t);		\
	dmu_Send(cb, NULL, 2, NULL);					\
} while(0)

// Reg 114	-	Gets FIFO cnt and stores it in I2C buffer, calling 'cb' when done.
#define dmu_GetFifoCount(cb) 										\
	(dmu_ReceiveFromRegister (ADD_FIFO_CNT_H, cb, NULL, 2, NULL))
// Reg 116
#define dmu_ReadNFifoBytes(_n, cb)	\
	dmu_ReceiveFromRegister (ADD_FIFO_RW, cb, NULL, _n, NULL)



void dmu_Init()
{
	if (dmu_data.init == _TRUE)
		return;
	
	iic_Init();
	
	dmu_StagesInit();	
	
	while (dmu_data.init == _FALSE)
		;

	
	return;
}

void dmu_StagesInit()
{
	switch (dmu_data.stage)
	{

	case 0:
			
		iic_commData.data[0] = ADD_PWR_MGMT_1;
		iic_commData.data[1] = PWR_MGMT_1_RESET;

		dmu_Send (dmu_StagesInit, dmu_CommFailed, 2, NULL);
		
		dmu_data.stage++;
		
		break;

	case 1:
		// Note: inserting delay / putchars here screws configuration up.	
				
		iic_commData.data[0] = ADD_SAMPLE_RATE_DIVIDER;
		iic_commData.data[1] = SAMPLE_RATE_DIVIDER;	// 25
		iic_commData.data[2] = CONFIG;				// 26
		iic_commData.data[3] = GYRO_CONFIG(GYRO_X_SELFTEST, GYRO_Y_SELFTEST, GYRO_Z_SELFTEST);		// 27
		iic_commData.data[4] = ACCEL_CONFIG(ACCEL_X_SELFTEST, ACCEL_Y_SELFTEST, ACCEL_Z_SELFTEST);	//28
		iic_commData.data[5] = FREE_FALL_THRESHOLD;
		iic_commData.data[6] = FREE_FALL_DURATION;
		iic_commData.data[7] = MOTION_INT_THRESHOLD;
		iic_commData.data[8] = MOTION_INT_DURATION;
		iic_commData.data[9] = ZERO_MOTION_THRESHOLD;
		iic_commData.data[10] = ZERO_MOTION_DURATION;
		iic_commData.data[11] = FIFO_ENABLE;
		
		dmu_Send (dmu_StagesInit, dmu_CommFailed, 12, NULL);
		
		dmu_data.stage++;
		
		break;
	
	case 2:
	
		iic_commData.data[0] = ADD_INT_PIN_CFG;
		iic_commData.data[1] = INT_PIN_CFG;		// 55
		iic_commData.data[2] = INT_ENABLE;

		dmu_Send(dmu_StagesInit, dmu_CommFailed, 3, NULL);
		
		dmu_data.stage++;
		
		break;
		
	case 3:

		iic_commData.data[0] = ADD_SIGNAL_PATH_RESET;
		iic_commData.data[1] = RESET_SIGNAL(1,1,1);
		iic_commData.data[2] = MOTION_DETECT_CTRL;
		iic_commData.data[3] = USER_CTRL(0,1,1);	// Run means not reset.
		iic_commData.data[4] = PWR_MGMT_1_RUN;
		// PWR_MGMT_2 stays in 0 (reset value).
		
		dmu_Send(dmu_StagesInit, dmu_CommFailed, 5, NULL);

		dmu_data.stage++;

		break;		


	case 4:

		iic_commData.data[0] = ADD_USER_CTRL;
		iic_commData.data[1] = USER_CTRL_INIT;	// Run means not reset.

		dmu_Send(dmu_StagesInit, dmu_CommFailed, 2, NULL);

		dmu_data.stage++;

		break;		


	case 5:
	

	case 6:		// Done for now - No need of resets or pwr mgmt.
		
		dmu_FifoReset();
		
		dmu_data.init = _TRUE;
		dmu_data.stage = 0;

		break;
		
	default: 
		break;
	}
	
	return;
}


void dmu_GetMeasurements(void)
{
	u8* dataPtr = (u8*)&dmu_measurements;
	dmu_ReceiveFromRegister(ADD_ACCEL_OUT, dmu_PrintFormattedMeasurements, NULL, sizeof(dmu_measurements), dataPtr);
}


void dmu_PrintFormattedMeasurements(void)
{
	struct dmu_measurements_T* dm = &dmu_measurements; 
	printf("ax: %d, ay: %d, az: %d\ngx: %d, gy: %d, gz: %d\n", dm->accel_x, dm->accel_y, dm->accel_z, dm->gyro_x, dm->gyro_y, dm->gyro_z);
	return;
}
	
	
void dmu_FifoStageRead(void)	
{
	switch (dmu_data.stage)
	{
	u16 fifoCount;
	case 0:
		dmu_GetFifoCount(dmu_FifoStageRead);
		dmu_data.stage++;		
		break;

	case 1:
		// Fifo reads can be 256 bytes max, even though fifo full length is 1024. 
		
		fifoCount = *((u16*)iic_commData.data);

		dmu_data.fifo.fetchTimes = fifoCount / MAX_BURST_READS;
		dmu_data.fifo.remainingBytes = fifoCount % MAX_BURST_READS;		

		dmu_data.stage++;
		// No break here.

	case 2:

		dmu_data.fifo.fetchTimes--;

		printf("fc: %d, ft: %d, rb: %d\n", fifoCount, dmu_data.fifo.fetchTimes, dmu_data.fifo.remainingBytes);
		
		if ((dmu_data.fifo.fetchTimes < 0) && (dmu_data.fifo.remainingBytes != 0))
			dmu_ReadNFifoBytes(dmu_data.fifo.remainingBytes, dmu_data.cb);
		else
			dmu_ReadNFifoBytes(MAX_BURST_READS, dmu_data.cb);
		
		break;

	default:
		break;
	}
}



#include "timers.h"
void dmu_printI2CData(void)
{

	u16 i;
	for (i = 0; i < PRINT_LENGTH; i++)
	{
		printf("%d %x\n", PRINT_START + i, iic_commData.data[i]);
		iic_commData.data[i] = '\0';
	}
}


void dmu_CommFailed()
{
	printf("comm failed, stage %d\n", dmu_data.stage);
	dmu_data.stage = 0;
}


void dmu_PrintFifoMem(void)
{
	u16 i, limit;
	
	if (dmu_data.fifo.fetchTimes < 0)
		limit = dmu_data.fifo.remainingBytes;
	else
		limit = MAX_BURST_READS;
	
	for (i = 0; i < limit / sizeof(s16); i++) 
	{
		printf("%d\t", (*(((u16*)iic_commData.data) + i) ) );
	}
		
			
	if (dmu_data.fifo.fetchTimes >= 0)
		dmu_FifoStageRead();
	else
		dmu_data.fifo.enable = _TRUE;
	
	return;
}


void dmu_printFifoCnt(void)
{
	printf("fCnt: %d\n", (*(u16*)iic_commData.data));
}


void dmu_GetSamples(void)
{
	dmu_ReadFifo(dmu_AverageSamples);
	return;
}

void dmu_AverageSamples(void)
{
	struct dmu_samples_T* dmuSamples = (struct dmu_samples_T*)iic_commData.data;
	struct dmu_sampleAccumulator_T acc = {0, 0, 0, 0, 0, 0};
	u16 i;
	u16 limit;
	
	if ((dmu_data.fifo.remainingBytes == 0) && (dmu_data.fifo.fetchTimes > 0))
		limit = MAX_BURST_READS / sizeof(struct dmu_samples_T);
	else
		limit = dmu_data.fifo.remainingBytes / sizeof(struct dmu_samples_T);
	

	for (i = 0; i < limit; i++)
		dmu_AccumulateSamples(&acc, dmuSamples++);

	dmu_DivideAccumulator(&acc, limit);

	printf("ax: %ld, ay: %ld, az: %ld\ngx: %ld, gy: %ld, gz: %ld\n", acc.accel_x, acc.accel_y, acc.accel_z, acc.gyro_x, acc.gyro_y, acc.gyro_z);	
	printf("s: %d\n", limit);	

	return;
}

void dmu_AccumulateSamples(struct dmu_sampleAccumulator_T* acc, struct dmu_samples_T* samples)
{
	acc->accel_x += samples->accel_x;
	acc->accel_y += samples->accel_y;
	acc->accel_z += samples->accel_z;

	acc->gyro_x += samples->gyro_x;
	acc->gyro_y += samples->gyro_y;
	acc->gyro_z += samples->gyro_z;
	
	return;
}

void dmu_DivideAccumulator(struct dmu_sampleAccumulator_T* acc, u16 n)
{
	acc->accel_x /= n;
	acc->accel_y /= n;
	acc->accel_z /= n;

	acc->gyro_x /= n;
	acc->gyro_y /= n;
	acc->gyro_z /= n;
	
	return;
}
