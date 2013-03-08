#ifndef _RF_H
#define _RF_H

#include "common.h"

#define USONIC_TX_TIMER 5 // The timer module used for transmitting data, connected to the transmitter's data pin.
#define USONIC_RX_TIMER 6 // The timer moduled used for measuring the received data, connected to the receiver's data pin.
// If any of the timers is changed, rf.c must be recompiled.

typedef void (*rf_ptr) (void); // A function callback for notifying the end of a transmission


// init como TX y como RX, son dos cosas distintas, dos inits
void rf_Init(void);
// Initializes the RF module. This requires the Timers module to work, and interrupts to be enabled.

//get error count, reset error count

bool usonic_Measure (usonic_ptr callback);
// Commences a measurement, and registers a callback function to be called after it is done.
// If the measurement was successful, the measured distance will be returned (in centimeters).
// If errors occured, or the device was not able to perform a correct measurement (because of a lack of echo),
// USONIC_INVALID_MEAS is returned. 
// If the device is idle and measurments can be made, _TRUE is returned. Else, usonic_Measure returns _FALSE.
// The Ultrasonic module will always be ready to perform a new measurement after callback is called, even if errors occured while measuring.
// A successful measurement takes around 30ms, and a failed one no more than 300ms.

#endif