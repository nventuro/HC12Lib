#include "dmu.h"
#include <stdio.h>


#define PRINT_START 1
#define PRINT_LENGTH (ADD_WHO_AM_I - PRINT_START)+1

#define MAX_BURST_READS 256

void dmu_printI2CData(void);

extern void printI2CData(void);
extern u8 buf[];

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

struct dmu_data_T dmu_data = {_FALSE, NULL, 0, {_TRUE, 0, 0} };

void dmu_Print(void);
void dmu_fifoStageRead(void);
void dmu_CommFailed(void);
void dmu_printFifoCnt(void);


// Reset macro - g, a, t are booleans (1/0), cb is callback after reset is done..
#define dmu_SignalReset(g, a, t, cb) do {			\
	iic_commData.data[0] = ADD_SIGNAL_PATH_RESET;	\
	iic_commData.data[1] = RESET_SIGNAL(g,a,t);		\
	dmu_Send(cb, NULL, 2, NULL);					\
} while(0)

#define dmu_FifoReset(cb) do {												\
	iic_commData.data[0] = ADD_USER_CTRL;									\
	iic_commData.data[1] = USER_CTRL(FIFO_MASTER_ENABLE, FIFO_RESET, 1);	\
	dmu_Send(cb, NULL, 2, NULL);								\
} while(0)

// Reg 114	-	Gets FIFO cnt and stores it in I2C buffer, calling 'cb' when done.
#define dmu_GetFifoCount(cb) 										\
	(dmu_ReceiveFromRegister (ADD_FIFO_CNT_H, cb, NULL, 2, NULL))
// Reg 116
#define dmu_ReadNFifoBytes(_n, cb)	\
	dmu_ReceiveFromRegister (ADD_FIFO_RW, cb, NULL, _n, NULL)

/*
void dmu_RunSelfTest(axes_selfTest test)
{
	return;
}*/


u16 dmu_GyroInit()
{
	volatile u16 a = SAMPLE_RATE_DIVIDER;
	volatile u16 b = GYRO_SAMPLE_RATE;
	volatile u16 c = ACCEL_SAMPLE_RATE;

	volatile struct dmu_measurements_T d;
	d.accel_z = 11;
	
	d.gyro_x = 1;
	d.gyro_y = 2;

	
//	data = data | (SAMPLE_RATE_DIVIDER << 3);
	return SAMPLE_RATE_DIVIDER;	

}


void dmu_Init()
{
	if (dmu_data.init == _TRUE)
		return;
	
	iic_Init();
	
	
	switch (dmu_data.stage)
	{

	case 0:
			
		iic_commData.data[0] = ADD_PWR_MGMT_1;
		iic_commData.data[1] = PWR_MGMT_1_RESET;

		dmu_Send (dmu_Init, dmu_ImFucked, 2, NULL);
		
		dmu_data.stage++;
		iic_MakeBusReservation();
		
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
		
		iic_FreeBusReservation();
		dmu_Send (dmu_Init, dmu_ImFucked, 12, NULL);
		
		dmu_data.stage++;
		iic_MakeBusReservation();
		
		break;
	
	case 2:
	
		iic_commData.data[0] = ADD_INT_PIN_CFG;
		iic_commData.data[1] = INT_PIN_CFG;		// 55
		iic_commData.data[2] = INT_ENABLE;

		iic_FreeBusReservation();		
		
		dmu_Send(dmu_Init, dmu_ImFucked, 3, NULL);
		
		iic_MakeBusReservation();
		dmu_data.stage++;
		
		break;
		
	case 3:

		iic_commData.data[0] = ADD_SIGNAL_PATH_RESET;
		iic_commData.data[1] = RESET_SIGNAL(1,1,1);
		iic_commData.data[2] = MOTION_DETECT_CTRL;
		iic_commData.data[3] = (1<<6);//USER_CTRL_INIT;	// Run means not reset.
		iic_commData.data[4] = PWR_MGMT_1_RUN;
		// PWR_MGMT_2 stays in 0 (reset value).
		iic_FreeBusReservation();
		
		dmu_Send(dmu_Init, dmu_CommFailed, 5, NULL);

		iic_MakeBusReservation();
		dmu_data.stage++;

		break;		


	case 4:

		iic_commData.data[0] = ADD_USER_CTRL;
		iic_commData.data[1] = USER_CTRL_INIT;	// Run means not reset.

		iic_FreeBusReservation();
		
		dmu_Send(dmu_Init, dmu_CommFailed, 2, NULL);

		iic_MakeBusReservation();
		dmu_data.stage++;

		break;		


	case 5:		// Done for now - No need of resets or pwr mgmt.
				
		dmu_data.init = _TRUE;
		dmu_data.stage = 0;
		iic_FreeBusReservation();

		dmu_ReceiveFromRegister(PRINT_START, dmu_printI2CData, dmu_ImFucked, PRINT_LENGTH, NULL);

		break;
		
	default: 
		break;
	}
	return;
}


void dmu_GetMeasurements(void)
{
	u8* dataPtr = (u8*)&dmu_measurements;
	dmu_ReceiveFromRegister(ADD_ACCEL_OUT, dmu_Print, NULL, sizeof(dmu_measurements), dataPtr);
}


void dmu_Print(void)
{
	struct dmu_measurements_T* dm = &dmu_measurements; 
	printf("ax: %d, ay: %d, az: %d\ngx: %d, gy: %d, gz: %d\n", dm->accel_x, dm->accel_y, dm->accel_z, dm->gyro_x, dm->gyro_y, dm->gyro_z);
	return;
}
	
	
void dmu_fifoStageRead(void)	
{
	switch (dmu_data.stage)
	{
	u16 fifoCount;
	case 0:
		dmu_GetFifoCount(dmu_fifoStageRead);
		dmu_data.stage++;		
		break;
	case 1:
		// Fifo reads can be 256 bytes max, even though fifo full length is 1024. 
		
		fifoCount = *((u16*)iic_commData.data);
		
		printf("fCnt: %d", fifoCount);
		
		if ( fifoCount > MAX_BURST_READS)
		{
			dmu_data.fifo.fetchTimes = fifoCount / MAX_BURST_READS;
			dmu_data.fifo.remainingBytes = fifoCount % MAX_BURST_READS;
		}
//		dmu_ReadNFifoBytes(dmu_data.fifoCount, dmu_data.cb);

		dmu_data.stage++;
		// No break here.

	case 2:

		dmu_data.fifo.fetchTimes--;
		
		printf("%d\n",dmu_data.fifo.fetchTimes);
		if ((dmu_data.fifo.fetchTimes < 0) && (dmu_data.fifo.remainingBytes != 0))
			dmu_ReadNFifoBytes(dmu_data.fifo.remainingBytes, dmu_data.cb);
		else
			dmu_ReadNFifoBytes(MAX_BURST_READS, dmu_data.cb);
		
		iic_MakeBusReservation();
//		}

//		dmu_data.stage = 0;

		break;
	default:
		break;
	}
}


void dmu_ImFucked()
{
	printf("Im fucked; stage: %d\n", dmu_data.stage);
}


void dmu_printI2CData(void)
{
	int i;
	for (i = 0; i < PRINT_LENGTH; i++)
	{
		printf("%d %x\n", PRINT_START + i, iic_commData.data[i]);
		iic_commData.data[i] = '\0';
	}
}

void dmu_CommFailed()
{
	printf("comm failed, stage %d\n", dmu_data.stage);
	dmu_data.init = _TRUE;
	dmu_data.stage = 0;
	iic_FreeBusReservation();

}

void dmu_PrintFifoMem(void)
{
	u16 i, limit;
	
	if (dmu_data.fifo.fetchTimes < 0)
		limit = dmu_data.fifo.remainingBytes;
	else
		limit = MAX_BURST_READS;
	
	for (i = 0; i < limit / sizeof(u16); i++) //dmu_data.fifoCount / sizeof(u16); i++)
		printf("%d\t", (*(((u16*)iic_commData.data) + i) ) );
	
	putchar('\n');
	
	iic_FreeBusReservation();
	
	if (dmu_data.fifo.fetchTimes >= 0)
		dmu_fifoStageRead();
	else
		dmu_data.fifo.enable = _TRUE;
	
	return;
}

void dmu_printFifoCnt(void)
{
	printf("fCnt: %d\n", (*(u16*)iic_commData.data));
}

