#ifndef _MAG_H_INCLUDED
#define _MAG_H_INCLUDED

#include "common.h"
#include "iic.h"


// Magnetometer sample rate.
// To change this value, change reg 0x10 (CTRL_REG_1) initialization in mag_definitions.h, 
// and then update this definition.
#define MAG_SAMPLE_RATE 80	 // Hz

struct mag_measurements_T
{	
	u8 status;

	s16 x;
	s16 y;
	s16 z;
};

struct mag_sampleAccumulator_T
{
	s32 x;
	s32 y;
	s32 z;
	
	u16 numberOfSamples;	
};

extern struct mag_measurements_T mag_measurements;




void mag_Init(void);


void mag_GetMeasurements(iic_ptr cb);	
/*	Fetches current device's sample, and saves it in global variable dmu_measurements.
	Calls cb when transfer is finished.
	@param cb: iic_ptr.
*/

void mag_PrintRawMeasurements(void);
void mag_PrintFormattedMeasurements(void);



#endif // _MAG_H_INCLUDED