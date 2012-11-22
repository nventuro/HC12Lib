/*
* graphics.h
* 
* Control de LED
* Lab de micros - 2012 - Grupo X
*
*/

#include "rti/rtix.h"
#include "placa/display.h"

#define fancy_msg(err_msg) (disp_blinking_message(err_msg,RTI_MS2PERIOD(3000), RTI_MS2PERIOD(3000),RTI_MS2PERIOD(250), RTI_MS2PERIOD(300))) 

void disp_blinking_message (char* msg, rti_time dim_dur, rti_time blink_dur, rti_time dim_rate, rti_time blink_rate);
