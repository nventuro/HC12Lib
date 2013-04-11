#ifndef _MAG_DEFINITIONS_H
#define _MAG_DEFINITIONS_H

#define MAG_ADDRESS (0x0E)


// Data status masks
#define MAG_ZYX_OVERWRITE(_stat) (_stat & (1<<7))
#define MAG_Z_OVERWRITE(_stat) (_stat & (1<<6))
#define MAG_Y_OVERWRITE(_stat) (_stat & (1<<5))
#define MAG_X_OVERWRITE(_stat) (_stat & (1<<4))

#define MAG_ZYX_READY(_stat) (_stat & (1<<3))
#define MAG_Z_READY(_stat) (_stat & (1<<2))
#define MAG_Y_READY(_stat) (_stat & (1<<1))
#define MAG_X_READY(_stat) (_stat & (1<<0))


// Use NONRAW for user corrected data
enum {MAG_STANDBY=0, MAG_ACTIVE_RAW, MAG_ACTIVE_NONRAW};
#define SYSTEM_MODE MAG_ACTIVE_RAW

// Reg 0x10
// CTRL_1_INIT - HEADER
// See page 19, table 31 and 32 of user manual.
// Any changes in this register must be done in standby mode (except changes in operation bit).

#define MAG_DATA_RATE_INIT 0		// 0 to 7
#define MAG_OVERSAMPLING_INIT 0		// 0 to 3
#define MAG_FAST_READ_INIT 0		// fast read disabled
enum {MAG_TRIGGER_AUTO=0, MAG_TRIGGER_NOW};
#define MAG_TRIGGER_MODE_INIT MAG_TRIGGER_AUTO
enum {MAG_OPERATION_STANDBY=0, MAG_OPERATION_ACTIVE};
#define MAG_OPERATION_INIT MAG_OPERATION_ACTIVE

#define MAG_TRIGGER_MODE(_x) (_x << 1)
#define MAG_OPERATION(_x) (_x)

// Reg 0x10, CTRL_1_INIT - DATA
#define MAG_CTRL_1_INIT ( (MAG_DATA_RATE_INIT << 5) | (MAG_OVERSAMPLING_INIT << 3) | \
			(MAG_FAST_READ_INIT << 2) | MAG_TRIGGER_MODE(MAG_TRIGGER_MODE_INIT) | MAG_OPERATION(MAG_OPERATION_INIT) )

// Reg 0x11
// CTRL_2_INIT - HEADER
#define MAG_AUTO_RESET 0
enum {MAG_DATA_CORRECT=0, MAG_DATA_RAW};
#define MAG_DATA_CORRECTION_INIT MAG_DATA_CORRECT


#define MAG_RESET(_x) (_x << 4)
#define MAG_DATA_CORRECT(_x) (_x << 5)

// Reg 0x11, CTRL_2_INIT - DATA
#define MAG_CTRL_2_INIT ( (MAG_AUTO_RESET << 7) | MAG_RESET(1) | MAG_DATA_CORRECT(MAG_DATA_CORRECTION_INIT) )


enum 
{	
	ADD_MAG_DATA_STATUS = 0x00,
	
	ADD_MAG_OUT = 0x01,

	ADD_MAG_XOUT_H = 0x01,
	ADD_MAG_XOUT_L,
	ADD_MAG_YOUT_H,
	ADD_MAG_YOUT_L,
	ADD_MAG_ZOUT_H,
	ADD_MAG_ZOUT_L,
	
	ADD_MAG_WHO_AM_I,
	ADD_SYSTEM_MODE,	// Read only register

	ADD_MAG_OFFSET = 0x09,

	ADD_MAG_XOFFSET_H = 0x09,
	ADD_MAG_XOFFSET_L,
	ADD_MAG_YOFFSET_H,
	ADD_MAG_YOFFSET_L,
	ADD_MAG_ZOFFSET_H,
	ADD_MAG_ZOFFSET_L,
	
	ADD_MAG_TEMP,
	
	ADD_MAG_CTRL_1,
	ADD_MAG_CTRL_2,
};


#endif // _MAG_DEFINITIONS_H