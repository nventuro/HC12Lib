#ifndef _QUAD_RF_DISP_H
#define _QUAD_RF_DISP_H

#include "common.h"

// Prints the current axes readings (from fjoy) to a LCD display. This deletes all data written on the display.
// The LCD display must already be initialized, and be a LCD_2004 display (4 rows, 20 characters each).
void qrf_disp_PrintAxes ();

// Prints the current 
//void qrf_disp_PrintCommAndBatt (u16 lostPacketsCount, u8 battALevel, u8 battBLevel);

#endif