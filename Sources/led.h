/*
 * led.h
 * 
 * Control de LED
 * Lab de micros - 2012 - Grupo X
 *
 */

#ifndef __LED_H__
#define __LED_H__

#include "common.h"
#include "rti.h"

#define LED_ON	255
#define LED_OFF	0
#define LED_RESTORE_DIM (-1)
#define	LED_NON_STOP 0

void led_init(void);

void led(u8 brightness);
void led_blink (u8 dim_on, u8 dim_off, rti_time t_on, rti_time t_off, rti_time dur, int end_dim);
void led_fade (rti_time t_trans, rti_time dur, int end_dim);
void led_stop(void);

#endif /* __LED_H__ */
