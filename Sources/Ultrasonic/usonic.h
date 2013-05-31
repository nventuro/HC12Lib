#ifndef _USONIC_H
#define _USONIC_H

#include "common.h"

#define USONIC_ECHO_TIMER 5 // The timer module used for measuring the echo, connected to the device's echo pin.
#define USONIC_TRIGG_TIMER 6 // The timer moduled used for generating the trigger pulse, connected to the device's trigger pin.
// If any of the timers is changed, usonic.c must be recompiled.

#define USONIC_INVALID_MEAS (-1) // An invalid measurement, caused by a lack of echo.

typedef void (*usonic_ptr) (s32 measurement); // A function callback used by usonic_Measure. measurement is expressed in centimeters.

void usonic_Init (void);
// Initializes the Ultrasonic module. This requires the RTI and Timers modules to work, and interrupts to be enabled.

void usonic_Measure (usonic_ptr callback);
// Commences a measurement, and registers a callback function to be called after it is done.
// If the measurement was successful, the measured distance will be returned (in centimeters).
// If errors occured, or the device was not able to perform a correct measurement (because of a lack of echo),
// USONIC_INVALID_MEAS is returned. 
// The Ultrasonic module will always be ready to perform a new measurement after callback is called, even if errors occured while measuring.
// A successful measurement takes around 30ms, and a failed one no more than 300ms.

#endif