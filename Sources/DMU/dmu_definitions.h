#ifndef _DMU_DEFINITIONS_H_
#define _DMU_DEFINITIONS_H_

typedef enum 
{
	FS_250 = 0,
	FS_500,
	FS_1000,
	FS_2000

}gyro_fullScale;	// Shift 3 times

typedef enum
{
	FS_2G = 0,
	FS_4G,
	FS_8G,
	FS_16G
	
}accel_fullScale;	// Shift 3 times


/*	Sample rate is Gyro sample rate / (1+SAMPLE_RATE_DIVIDER).`
	If LP_FILTER_CONFIG is 0, Gyro sample rate is 8 kHz, 
	otherwise is 1 kHz.
	Sample rate affects Gyro and Accel only if it is less
	than 1 kHz, which is Accel's max sample rate.
*/

// Reg 26 - ADD_CONFIG - HEADER
#define EXT_SYNC_SET (0)
#define LP_FILTER_CONFIG (1)	// 0 to 6 

// Reg 26 - ADD_CONFIG - DATA
#define CONFIG (LP_FILTER_CONFIG | (EXT_SYNC_SET << 3) )

// Set this value; an approximation will be used as sample rate according to the divider.
#define SAMPLE_RATE (10)

// Reg 25: ADD_SAMPLE_RATE_DIVIDER - DATA:
#define SAMPLE_RATE_DIVIDER ((LP_FILTER_CONFIG == 0 ) ? (8000/SAMPLE_RATE-1) : (1000/SAMPLE_RATE-1))	// u8 - reg 25 ADD_SAMPLE_RATE_DIVIDER

#if SAMPLE_RATE_DIVIDER > 256
#warning "Check LP_FILTER_CONFIG and SAMPLE_RATE"
#endif

// Real sampling rates after setting divider based in desired sample rate.
#define GYRO_SAMPLE_RATE ((LP_FILTER_CONFIG == 0) ? (8000/(1+SAMPLE_RATE_DIVIDER)) : (1000/(1+SAMPLE_RATE_DIVIDER)))
#define ACCEL_SAMPLE_RATE ((GYRO_SAMPLE_RATE > 1000) ? 1000 : GYRO_SAMPLE_RATE)

// Reg 27 and Reg 28: Gyro and Accel config - HEADER.
// also trigger selfTest. Not used yet.
#define GYRO_FULLSCALE (FS_2000)	
#define ACCEL_FULLSCALE (FS_16G)

#define GYRO_X_SELFTEST 0
#define GYRO_Y_SELFTEST 0
#define GYRO_Z_SELFTEST 0
#define ACCEL_X_SELFTEST 0
#define ACCEL_Y_SELFTEST 0
#define ACCEL_Z_SELFTEST 0

#define GYRO_SELFTEST(x, y, z) ((x << 7) | (y << 6) | (z << 5))
#define ACCEL_SELFTEST(x, y, z) ((x << 7) | (y << 6) | (z << 5))

// Reg 27 - ADD_GYRO_CONFIG - DATA:
#define GYRO_CONFIG(x, y, z) ((GYRO_FULLSCALE << 3) | GYRO_SELFTEST(x, y, z))		// ADD_GYRO_CONFIG (27)
// Reg 28 - ADD_ACCEL_CONFIG - DATA:
#define ACCEL_CONFIG(x, y, z) ((ACCEL_FULLSCALE << 3) | ACCEL_SELFTEST(x, y, z))	// ADD_ACCEL_CONFIG (28)

// Reg 29 and Reg 30 DATA
#define FREE_FALL_THRESHOLD	0	// u8 - ADD_FREE_FALL_THRESHOLD	(reg 29)
#define FREE_FALL_DURATION	0	// u8 - ADD_FREE_FALL_DURATION (reg 30)

// Reg 31 and Reg 32: int threshold/ duration - DATA & HEADER
#define MOTION_INT_THRESHOLD 0	// u8 - ADD_MOTION_THRESHOLD (reg 31)
#define MOTION_INT_DURATION 0 // u8 - ADD_MOTION_DURATION (reg 32)

// Reg 33 and Reg 34: Zero motion interrupt - DATA & HEADER
#define ZERO_MOTION_THRESHOLD 0	// u8 - ADD_ZERO_MOTION_THRESHOLD (reg 33)
#define ZERO_MOTION_DURATION 0		// u8 - ADD_ZERO_MOTION_DURATION (reg 34)
// Reg 35 - ADD_FIFO_ENABLE - HEADER
// Note: see reg 106 (FIFO MASTER ENABLE)
#define FIFO_XG_ENABLE 0
#define FIFO_YG_ENABLE 0
#define FIFO_ZG_ENABLE 0
#define FIFO_ACCEL_ENABLE 0

// Reg 35 (ADD_FIFO_ENABLE) - DATA:
#define FIFO_ENABLE ( (FIFO_XG_ENABLE << 6) | (FIFO_YG_ENABLE << 5) | (FIFO_ZG_ENABLE << 4) | (FIFO_ACCEL_ENABLE << 3))	

// Reg 55; Fsync not used - HEADER
enum {HIGH = 0, LOW};
#define INT_LEVEL HIGH		// Pin logic
#define INT_OPEN_DRAIN 0	// Pin logic
#define INT_LATCH_ENABLE 0	// Create pulse or latch pin to INT_LEVEL when interrupt is generated.
#define INT_FAST_CLEAR 1	// If enabled, any read operation clears interrupt.
#define I2C_BYPASS_ENABLE 0	// Not used yet.

// Reg 55: ADD_INT_PIN_CFG - DATA:
#define INT_PIN_CFG ( (INT_LEVEL << 7) | (INT_OPEN_DRAIN << 6) |		\
					(INT_LATCH_ENABLE << 5) | (INT_FAST_CLEAR << 4) | 	\
					(I2C_BYPASS_ENABLE << 1) )
					
// Reg 56: ADD_INT_ENABLE - HEADER

#define MOTION_INT_ENABLE 0
#define FIFO_OVF_INT_ENABLE 0
#define DATA_READY_INT_ENABLE 1

// Reg 56 - ADD_INT_ENABLE - DATA
#define INT_ENABLE ((MOTION_INT_ENABLE << 6) | (FIFO_OVF_INT_ENABLE << 4) | (DATA_READY_INT_ENABLE << 0))

// Reg 58 - INT_STATUS - HEADER
// Read this register to get flags, use macros as masks.
#define GET_MOTION_FLAG(intStatus) (intStatus & (1<<6))
#define GET_FIFO_OVF_FLAG(intStatus) (intStatus & (1<<4))
#define GET_DATA_READY_FLAG(intStatus) (intStatus & (1<<0))

// Reg 97 - Motion interrupt flags by Axes.

// Reg 104 - SIGNAL_PATH_RESET
#define RESET_SIGNAL(g, a, t) ((g << 2) | (a << 1) | (t << 0))

// Reg 105 - MOTION_DETECT_CTRL - HEADER
enum {DEC_RESET = 0, DEC_1, DEC_2, DEC_4};
#define ACCEL_ON_DELAY 0	// ms; 0 to 3
#define MOTION_DEC_COUNT DEC_1
#define FALL_DEC_COUNT DEC_1

// Reg 105 - DATA
#define MOTION_DETECT_CTRL ( (ACCEL_ON_DELAY << 4) | (FALL_DEC_COUNT << 2) | (MOTION_DEC_COUNT << 0) )

// Reg 106 - USER_CTRL
// Note: all resets are cleared to 0 after reset is triggered
enum {FIFO_MASTER_DISABLE = 0, FIFO_MASTER_ENABLE};
enum {I2C_MASTER_DISABLE = 0, I2C_MASTER_ENABLE};
enum {MPU_SPI_DISABLE = 0, MPU_SPI_ENABLE};
enum {FIFO_RUN = 0, FIFO_RESET};	// Run as opposite of reset (not reset)
enum {I2C_MASTER_RUN = 0, I2C_MASTER_RESET};
enum {SIGNAL_PATH_RUN = 0, SIGNAL_PATH_RESET};

#define USER_CTRL(fifo_en, fifo_res, signal_res) ( (fifo_en << 6) | (I2C_MASTER_DISABLE << 5) | \
	(MPU_SPI_DISABLE << 4) | (fifo_res << 2) | (I2C_MASTER_RESET << 1) | (signal_res <<0 ))

// Reg 107 - PWR_MGMT_1 - HEADER
enum {PWR_SLEEP_OFF=0, PWR_SLEEP_ON};		// Send device to sleep mode.
#define PWR_CYCLE 0		// Cycle between sleep and wake; see reg 108
enum {CLK_INTERNAL=0, CLK_PLL_X, CLK_PLL_Y, CLK_PLL_Z, CLK_EXT_32K, CLK_EXT_19M, CLK_STOP=7 };
#define PWR_CLK_SOURCE CLK_PLL_X
enum {PWR_RUN=0, PWR_RESET};
#define PWR_TEMP_DISABLE 0	// Temp disabled

// Reg 107 - PWR_MGMT_1 - DATA
#define PWR_MGMT_1_RESET ( (PWR_RESET << 7) | (PWR_SLEEP_OFF << 6) | (PWR_CYCLE << 5) | (PWR_TEMP_DISABLE << 3) | (PWR_CLK_SOURCE << 0) )
#define PWR_MGMT_1_RUN 	 ( (PWR_RUN << 7) | (PWR_SLEEP_OFF << 6) | (PWR_CYCLE << 5) | (PWR_TEMP_DISABLE << 3) | (PWR_CLK_SOURCE << 0) )

// Reg 108 - PWR_MGMT_2 - HEADER
// To use wake mode, cycle = 1, sleep = 0, temp_dis = 1, stdby =1
enum {WAKE_1_25=0, WAKE_5, WAKE_20, WAKE_40 };	// Wakeup rate in Hz
#define PWR_WAKE_CTRL WAKE_1_25
// Leave in reset value, no need of stdby any gyro or accel axis.

// Register addresses.
enum 
{
	ADD_SAMPLE_RATE_DIVIDER = 25,	// Divider
	ADD_CONFIG,						// LP filter config
	ADD_GYRO_CONFIG,				// Self test and fullScale
	ADD_ACCEL_CONFIG,				// Self test and fullScale
	ADD_FREE_FALL_THRESHOLD,		// Free fall interrupt threshold.
	ADD_FREE_FALL_DURATION,			// Counter for free fall interrupt
	ADD_MOTION_THRESHOLD,		// Motion threshold in 32 mg/LSB
	ADD_MOTION_DURATION,		// Counter for interrupt; ticks every 1 ms and counts upwards if samples are above threshold.
	ADD_ZERO_MOTION_THRESHOLD, 
	ADD_ZERO_MOTION_DURATION, 
	ADD_FIFO_ENABLE = 35,
	
	ADD_INT_PIN_CFG = 55,	// Interrupt config
	ADD_INT_ENABLE,			// Interrupts accepted.
	ADD_INT_STATUS = 58,	// Read-only flags; flags are cleared after reading / any read if fast clear is enabled.	
	ADD_ACCEL_OUT,			// X, Y, Z, 2 bytes each, High first (Big endian)

	ADD_ACCEL_XOUT_H = 59,
	ADD_ACCEL_XOUT_L,
	ADD_ACCEL_YOUT_H,
	ADD_ACCEL_YOUT_L,
	ADD_ACCEL_ZOUT_H,
	ADD_ACCEL_ZOUT_L,
	
	ADD_GYRO_OUT = 67,

	ADD_GYRO_XOUT_H = 67,
	ADD_GYRO_XOUT_L,
	ADD_GYRO_YOUT_H,
	ADD_GYRO_YOUT_L,
	ADD_GYRO_ZOUT_H,
	ADD_GYRO_ZOUT_L,
	
	ADD_SIGNAL_PATH_RESET = 104,
	ADD_MOTION_DETECT_CTRL,			// Interrupt decrement
	ADD_USER_CTRL,			// fifo enable, reset, etc
	ADD_PWR_MGMT_1,			// sleep, master reset, clock source, temp disable
	ADD_PWR_MGMT_2,			// wake up freq, individual ax standby
	
	ADD_FIFO_CNT_H = 114,
	ADD_FIFO_CNT_L,
	ADD_FIFO_RW,			// Read or write FIFO buffer from here (1 kb)
	ADD_WHO_AM_I

};

typedef enum 
{
	NONE = 0, ST_Z, ST_Y, ST_YZ, ST_X, ST_XZ, ST_XY, ST_XYZ,
}axes_selfTest;	// Shift 5 times.


#endif