#ifndef _FJOY_H
#define _FJOY_H

#include "common.h"

#define FJOY_SAMPLE_PERIOD_MS 20

#define FJOY_BUTTONS 11 // 

#define FJOY_MAX_CALLBACKS 3


// Connections


extern struct 
{
	bool button[FJOY_BUTTONS];
	s8 yaw;
	s8 pitch;
	s8 roll;
	u8 elev;
} fjoy_status;

typedef void (*fjoy_callback)(void);

void fjoy_Init(void);
void fjoy_CallOnUpdate(fjoy_callback cb);

#endif