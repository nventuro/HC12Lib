#ifndef _QUAD_RF_DISP_H
#define _QUAD_RF_DISP_H

#define QUAD_RF_DISP_REFRESH_PERIOD_MS 250

// Prints the current axes readings (from fjoy) to a LCD display. This deletes all data written on the display.
// The LCD display must already be initialized, and be a LCD_2004 display (4 rows, 20 characters each).
void qrf_disp_PrintAxes (void);

// Calls qrf_disp_PrintAxes every QUAD_RF_DISP_REFRESH_PERIOD_MS automatically.
void qrf_disp_PrintAxesPeriodically (void);

#endif