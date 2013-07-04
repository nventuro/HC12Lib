#ifndef _QUAD_RF_H
#define _QUAD_RF_H

#define QUAD_RF_DISP_REFRESH_PERIOD_MS 250

#define QRF_INVALID_BATT_LEVEL 127

#include "common.h"

extern u16 lostPacketsCount;
extern u8 battALevel;
extern u8 battBLevel;

// Initializes the LCD (on LCD_2004 mode), FJOY and nRF (on PTX mode) modules. This requires interrupts to be enabled.
void qrf_Init(void);

void qrf_SendJoyMeasurements(void);

void qrf_PrintCommInfo(void);

#endif