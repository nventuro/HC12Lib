#ifndef _FJOY_H
#define _FJOY_H

#include "common.h"
#include "atd.h"

#define FJOY_SAMPLE_PERIOD_MS 20

#define FJOY_BUTTONS 11

#define FJOY_MAX_CALLBACKS 3

#define FJOY_ATD_OVERSAMPLING 8
#define FJOY_BUTTON_OVERSAMPLING 3

// Connections
#define FJOY_ATD_MODULE ATD0
#define FJOY_YAW_CHANN 1
#define FJOY_ROLL_CHANN 0
#define FJOY_PITCH_CHANN 3
#define FJOY_ELEV_CHANN 2

#define FJOY_YAW_BITS 5
#define FJOY_PITCH_BITS 5
#define FJOY_ROLL_BITS 5
#define FJOY_ELEV_BITS 7


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