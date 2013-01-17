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
	
};


void dmu_Init(void);
bool dmu_GetMeasurements(void);


#endif // _DMU_H_