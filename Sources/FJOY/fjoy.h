/*#ifndef _FJOY_H
#define _FJOY_H

#include "common.h"

#define FJOY_SAMPLE_PERIOD_MS 20

#define FJOY_BUTTONS 6 // 


// Connections
#define P_TRIGGER 
#define PORT DDR

typedef enum
{
	TRIGGER,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
	BUTTON_5,
	BUTTON_6,
	BUTTON_7,
	UP,
	DOWN,
	LEFT,
	RIGHT
} FJOY_BUTTONS;

extern struct 
{
	bool button[FJOY_BUTTONS];
	s8 yaw;
	s8 pitch;
	s8 roll;
	u8 elevation;
} fjoy_data;

void fjoy_Init(void);

#endif*/