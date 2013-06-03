#ifndef _BATT_H
#define _BATT_H

#include "common.h"
#include "atd.h"

#define BATT_SAMPLE_PERIOD_MS 1000

#define BATT_MAX_BATTS 3

typedef void (*batt_callback) (void);

// Initializes the battery level-checking module. This samples the registered batteries' voltages every BATT_SAMPLE_PERIOD_MS
// and calls any registered callback when that occurs. If no callback is assigned, an error is thrown.
// Batteries are registed by calling AddBatt.
void batt_Init (void);

// Registers a battery for level-checking. The battery must be connected to the corresponding atd module and channel.
// If the battery's voltage reading (in ATD_CONV_BITS) drops below level, cb is called, or an error is thrown is cb == NULL.
void batt_AddBatt (atd_module module, atd_channel channel, u16 level, batt_callback cb);

#endif