#ifndef _FJOY_H
#define _FJOY_H

#include "common.h"
#include "atd.h"
#include "mc9s12xdp512.h"

#define FJOY_SAMPLE_PERIOD_MS 20
#define FJOY_BUTTON_SAMPLE_PERIOD_MS 4 
#define FJOY_BUTTON_OVERSAMPLING 3 // Must be odd
// FJOY_BUTTON_SAMPLE_PERIOD_MS * (FJOY_BUTTON_OVERSAMPLING + 1) must be lower than FJOY_SAMPLE_PERIOD_MS

#define FJOY_ATD_OVERSAMPLING 8

#define FJOY_MAX_CALLBACKS 3
#define FJOY_BUTTONS 11

// Connections
#define FJOY_ATD_MODULE ATD0
#define FJOY_YAW_CHANN 1
#define FJOY_ROLL_CHANN 0
#define FJOY_PITCH_CHANN 3
#define FJOY_ELEV_CHANN 2

#define FJOY_TRIGGER 0
#define FJOY_UP 1
#define FJOY_DOWN 2
#define FJOY_LEFT 3
#define FJOY_RIGHT 4
#define FJOY_B2 5
#define FJOY_B3 6
#define FJOY_B4 7
#define FJOY_B5 8
#define FJOY_B6 9
#define FJOY_B7 10

//#define FJOY_TRIGGER_PIN 0
//#define FJOY_UP_PIN 1
//#define FJOY_DOWN_PIN 2
//#define FJOY_LEFT_PIN 3
//#define FJOY_RIGHT_PIN 4
//#define FJOY_B2_PIN 5
//#define FJOY_B3_PIN 6
//#define FJOY_B4_PIN 7
//#define FJOY_B5_PIN 8
#define FJOY_B6_PIN PTS_PTS0
#define FJOY_B7_PIN PTS_PTS1

#define FJOY_B6_DDR DDRS_DDRS0
#define FJOY_B7_DDR DDRS_DDRS1

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