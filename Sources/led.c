/*
 * led.c
 * 
 * Control de LED
 * Lab de micros - 2012 - Grupo X
 *
 */

#include <limits.h>
#include "derivative.h"
#include "led.h"
#include "rti.h"

#define DIM_BITS 3
#define DIM_MIN 0
#define DIM_MAX ((1<<(DIM_BITS))-1)
#define LED_PORT PORTB_PB7
#define LED_PORT_DD DDRB_DDRB7

#define BRIGHT2DIM(br) ((br)>>(CHAR_BIT - DIM_BITS))
#define DIM2BRIGHT(di) ((di)<<(CHAR_BIT - DIM_BITS))

#define LED_BLINKING	1
#define	LED_NOT_BLINKING	(!LED_BLINKING)

#define LED_CTL(v) do{LED_PORT = (v)? LED_LOGIC1 : LED_LOGIC0;}while(0)

#define IS_LED_ON() (LED_PORT == LED_LOGIC1)

enum {LED_LOGIC0, LED_LOGIC1};

enum {FADE_UP, FADE_DOWN};
	

struct LEDData 
{
	u8 dim;
	u8 blinking;
	u8 fading;
};

struct LEDBlink
{
	u8 blink_status;
	u8 blink_dim_on;
	u8 blink_dim_off;
	u8 dim_end;
	rti_time blink_t_on;
	rti_id blink_t_off;	
	rti_id blink_func;
	rti_id stop_func;
	
	rti_id fade_func;
	u8 fade_dir;
};

rti_time led_pwm_ctrl(void *_data, rti_time pw);
rti_time led_blink_toggle_dim (void *_data, rti_time pw);
rti_time led_autostop (void *_data, rti_time pw);
rti_time led_fader (void *_data, rti_time pw);

static struct LEDData led_cfg;
static struct LEDBlink led_fancy_cfg;

void led_init(void)
{
	LED_PORT_DD = PORTDD_OUT;
	
	led_cfg.dim = LED_OFF;
	led_cfg.blinking = LED_NOT_BLINKING;
	led_fancy_cfg.stop_func = RTI_INVALID_ID;
		
	//rti_register(led_pwm_ctrl,NULL, RTI_ALWAYS, RTI_NOW);
	//rti_enable(SRV_LED_PWM, 1);
}

void led_up()
{
	led_cfg.dim = (led_cfg.dim + 1)%(DIM_MAX+1);
}

void led_down()
{
//	led_cfg.dim--;
	led_cfg.dim = (led_cfg.dim - 1)%(DIM_MAX+1);
}

void led(u8 brightness) 
{
	led_cfg.dim = BRIGHT2DIM(brightness);
}

rti_time led_pwm_ctrl(void *_data, rti_time pw)
{
	rti_time r;
	
	if (led_cfg.dim == DIM_MIN) {
		LED_CTL(0);
		r = DIM_MAX;
	} else if (led_cfg.dim  == DIM_MAX) {
		LED_CTL(1);
		r = DIM_MAX;
	} else {
		if (!IS_LED_ON())
			r = led_cfg.dim;
		else
			r = DIM_MAX - led_cfg.dim;
		LED_PORT = !LED_PORT;
	}

	return r;

}


int led_pwm(void *data, int t)
{
	if (!t) {
		if (led_cfg.dim  == DIM_MIN)
			LED_CTL(0);
		else
			LED_CTL(1);
		t = led_cfg.dim;
	} else {
		LED_CTL(0);
		t = 0;
	}
	return t;
}

static void led_fancy_enter(int end_dim)
{
	led_stop();
		
	if (end_dim == LED_RESTORE_DIM) 
		led_fancy_cfg.dim_end = DIM2BRIGHT(led_cfg.dim);
	else
		led_fancy_cfg.dim_end = end_dim;
}

static void led_fancy_leave(rti_time dur)
{
	if (dur != LED_NON_STOP)
		led_fancy_cfg.stop_func = rti_register(led_autostop, NULL,RTI_ONCE,dur);
	else
		led_fancy_cfg.stop_func = RTI_INVALID_ID;
}

void led_blink (u8 dim_on, u8 dim_off, rti_time t_on, rti_time t_off, rti_time dur, int end_dim)
{
	led_fancy_enter(end_dim);
	
	led_cfg.blinking = LED_BLINKING;
	
	led_fancy_cfg.blink_dim_on = dim_on;
	led_fancy_cfg.blink_dim_off = dim_off;
		
	led_fancy_cfg.blink_t_on = t_on;
	led_fancy_cfg.blink_t_off = t_off;
	
	led_fancy_cfg.blink_status = LED_BLINKING;
	
	led_fancy_cfg.blink_func = rti_register(led_blink_toggle_dim, NULL, t_on, RTI_NOW);
	
	led_fancy_leave(dur);
}

void led_fade (rti_time t_trans, rti_time dur, int end_dim)
{
	led_fancy_enter(end_dim);
	
	led_cfg.fading = 1;
	
	led(0);
	led_fancy_cfg.fade_dir = FADE_UP;
	
	led_fancy_cfg.fade_func = rti_register(led_fader, NULL, t_trans, RTI_NOW);
	
	led_fancy_leave(dur);
}

rti_time led_blink_toggle_dim (void *_data, rti_time pw)
{
	if (led_fancy_cfg.blink_status == LED_BLINKING)
	{	
		led_fancy_cfg.blink_status = LED_NOT_BLINKING;
		led(led_fancy_cfg.blink_dim_off);
		return led_fancy_cfg.blink_t_off;
	}
	else
	{
		led_fancy_cfg.blink_status = LED_BLINKING;
		led(led_fancy_cfg.blink_dim_on);
		return led_fancy_cfg.blink_t_on;
	}	
}

rti_time led_autostop (void *_data, rti_time pw)
{
	if (led_cfg.blinking != LED_NOT_BLINKING) {
		rti_cancel(led_fancy_cfg.blink_func);
		led_cfg.blinking = LED_NOT_BLINKING;
	} else if (led_cfg.fading) {
		rti_cancel(led_fancy_cfg.fade_func);
		led_cfg.fading = 0;
	}
		
	led(led_fancy_cfg.dim_end);
	
	return pw;
}

rti_time led_fader (void *_data, rti_time pw)
{
	if (led_fancy_cfg.fade_dir == FADE_UP)
		led_up();
	else
		led_down();
	
	if (led_cfg.dim == DIM_MAX)
		led_fancy_cfg.fade_dir = FADE_DOWN;
	else if (led_cfg.dim == DIM_MIN)
		led_fancy_cfg.fade_dir = FADE_UP;
	
	return pw;
}

void led_stop (void)
{
	if (led_fancy_cfg.stop_func != RTI_INVALID_ID) {
		rti_cancel(led_fancy_cfg.stop_func);
		led_fancy_cfg.stop_func = RTI_INVALID_ID;
	}
	if (led_cfg.blinking == LED_BLINKING)
	{
		rti_cancel(led_fancy_cfg.blink_func);
		led_cfg.blinking = LED_NOT_BLINKING;	
	}
	if (led_cfg.fading)
	{
		rti_cancel(led_fancy_cfg.fade_func);
		led_cfg.fading = 0;	
	}
	led(led_fancy_cfg.dim_end);
}
