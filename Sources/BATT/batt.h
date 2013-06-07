#ifndef _BATT_H
#define _BATT_H

#include "common.h"
#include "atd.h"

#define BATT_SAMPLE_PERIOD_MS 1000

#define BATT_AD_RESOLUTION_MV 5
#define BATT_MV_TO_LEVEL(mv) (mv/BATT_AD_RESOLUTION_MV)

#define BATT_MAX_BATTS 2

typedef void (*batt_callback) (void);

// Initializes the battery level-checking module. This samples the registered batteries' voltages every BATT_SAMPLE_PERIOD_MS
// and calls any registered callback when that occurs. Batteries are registed by calling AddBatt.
void batt_Init (void);

// Registers a battery for level-checking. The battery must be connected to the corresponding atd module and channel.
// If the battery's voltage level (in %, from 0 to 100) drops below lowBattLevel, cb is called. cb must not be NULL, else an
// error is thrown. The battery's % level is calculated using minLevel and maxLevel, the correspoding atd values for the maximum
// and minimum battery voltages (BATT_MV_TO_LEVEL converts milivolts to this). 
// Whenever the battery is sampled, it's % level is stored in currLevel. currLevel may be unused (by making it NULL).
// If no more batteries can be registered, an error is thrown.
// When AddBatt returns, the battery has already been measured, and it's value stored in currLevel (if it isn't NULL).
// AddBatt requires interrupts to be enabled.
void batt_AddBatt (atd_module module, atd_channel channel, batt_callback cb, u8 lowBattLevel, u16 minLevel, u16 maxLevel, u8 *currLevel);

#endif