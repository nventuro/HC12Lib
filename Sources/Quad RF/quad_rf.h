#ifndef _QUAD_RF_H
#define _QUAD_RF_H

#include "common.h"

extern u16 packetLossCount;

// Initializes the LCD (on LCD_2004 mode), FJOY and nRF (on PTX mode) modules. This requires interrupts to be enabled.
void qrf_Init(void);

void qrf_SendJoyMeasurements(void);
void qrf_PrintJoyMeasurements(void);

#endif