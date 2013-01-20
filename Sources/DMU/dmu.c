#include "dmu.h"
#include <stdio.h>

#define PRINT_LENGTH (ADD_WHO_AM_I - ADD_SAMPLE_RATE_DIVIDER)+1


void dmu_printI2CData(void);


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

struct dmu_data_T dmu_data = {_FALSE, NULL, 0};

void dmu_Print(void);
void dmu_fifoStageRead(void);


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
#define dmu_GetFifoCount(cb) \
	(dmu_ReceiveFromRegister (ADD_FIFO_CNT_H, cb, NULL, 2, NULL))

// Reg 116
#define dmu_ReadNFifoBytes(n, cb)	\
	(dmu_ReceiveFromRegister (ADD_FIFO_RW, cb, NULL, n, NULL))

#define dmu_ReadFifo(_cb)	do {								\
	dmu_fifoData.stage = 0;		\
	dmu_fifoData.cb = _cb;		\
								\
	dmu_fifoStageRead();		\
} while(0)

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
	{volatile u16 i,j;
	case 0:
		putchar('s');putchar((u8)dmu_data.stage+'0');putchar('\n');
			
		iic_commData.data[0] = ADD_PWR_MGMT_1;
		iic_commData.data[1] = PWR_MGMT_1_RESET;

		if (dmu_Send (dmu_Init, dmu_ImFucked, 2, NULL) == _FALSE)
			return;
		dmu_data.stage++;
		iic_MakeBusReservation();
		dmu_Init();
		break;
	
	case 1:
	
		putchar('s');putchar((u8)dmu_data.stage+'0');putchar('\n');
			
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
		if (dmu_Send (dmu_Init, dmu_ImFucked, 12, NULL) == _FALSE)
			return;
		dmu_data.stage++;
		iic_MakeBusReservation();
		break;
	
	case 2:
//			printf("stage: %d\n", dmu_data.stage);
		putchar('s');putchar((u8)dmu_data.stage+'0');putchar('\n');
		iic_commData.data[0] = ADD_INT_PIN_CFG;
		iic_commData.data[1] = INT_PIN_CFG;		// 55
		iic_commData.data[2] = INT_ENABLE;

		iic_FreeBusReservation();		
		
		if (dmu_Send(dmu_Init, dmu_ImFucked, 3, NULL) == _FALSE)
			return;
		
		iic_MakeBusReservation();
		dmu_data.stage++;
		break;
		
	case 3:
		
//		for (i = 0; i < 50000 ; i++)
//			for (j = 0; j < 100; j++)
//				;
	
		putchar('r');putchar('e');putchar('a');putchar('d');putchar('\n');
		
		iic_FreeBusReservation();
	
		dmu_ReceiveFromRegister(ADD_SAMPLE_RATE_DIVIDER, dmu_printI2CData, dmu_ImFucked, PRINT_LENGTH, NULL);

		iic_MakeBusReservation();
		dmu_data.stage++;

		break;
		
		
	case 4:
		putchar('s');putchar((u8)dmu_data.stage+'0');putchar('\n');
	
		iic_commData.data[0] = ADD_MOTION_DETECT_CTRL;
		iic_commData.data[1] = MOTION_DETECT_CTRL;
		iic_commData.data[2] = USER_CTRL(FIFO_MASTER_DISABLE, FIFO_RUN, SIGNAL_PATH_RUN);	// Run means not reset.
		iic_commData.data[3] = PWR_MGMT_1_RUN;
		// PWR_MGMT_2 stays in 0 (reset value).
		iic_FreeBusReservation();
		
		if (dmu_Send(dmu_Init, dmu_ImFucked, 4, NULL) == _FALSE)
			return;
		
		iic_MakeBusReservation();
		
		dmu_data.stage++;
		break;
		
	case 5:		// Done for now - No need of resets or pwr mgmt.
		
		putchar('s');putchar((u8)dmu_data.stage+'0');putchar('\n');
		
		dmu_data.init = _TRUE;
		dmu_data.stage = 0;
		iic_FreeBusReservation();
		break;
		
	default: 
		break;
	}
	return;
}


bool dmu_GetMeasurements(void)
{
	u8* dataPtr = (u8*)&dmu_measurements;
	return dmu_ReceiveFromRegister(ADD_ACCEL_OUT, dmu_Print, NULL, sizeof(dmu_measurements), dataPtr);
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
	case 0:
		dmu_GetFifoCount(dmu_fifoStageRead);
		dmu_data.stage++;		
		break;
	case 1:
		dmu_ReadNFifoBytes( ((u16)iic_commData.data[0]), dmu_data.cb);
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
		printf("%d %x\n",ADD_SAMPLE_RATE_DIVIDER + i, iic_commData.data[i]);
		iic_commData.data[i] = '\0';
	}
}


