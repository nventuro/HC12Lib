#include "common.h"
#include "led.h"
#include "display.h"
#include "kbd.h"
#include "ir.h"
#include "rtc.h"
#include "systemInputs.h"

#include <ctype.h>
#include "rti/rtix.h"
#include "motor/motor.h"
#include "motor/ccmotor.h"
#include "hw/pwm.h"
#include "comm/spi.h"

#include "rc5table.h"
 
void init(void);
void dispTime(void);


#define MAX_SPEED 128
#define MIN_SPEED 0


struct 
{
	runMode mode;
	struct
	{
		u8 speed;
		const u8 periods[5];
	}pwm_data;
	
	struct 
	{
		bool running;
	}step_data;
	
}sys_data = {STEPPER, {0, {0, 30, 60, 90, 128}}, {_FALSE}};


void stepperInit(void);
void stepperLoop(void);
void pwmInit(void);
void pwmLoop(void);

void clearDisplay(void);
void dispTime(void);
void printStepperPos(void);
void setHour(void);


void main(void)
{		
	init();
	
	setHour();
	
	for(;;)
	{
		stepperLoop();
		pwmLoop();
	}	
}


void init (void)
{
	// Modulos que no requieren interrupciones para inicializar
	rti_init();
	led_init();
	display_init();

	kb_init();
	ir_init();

	spi_init();
	smot_init(0);
	pwm_init();
	mot_init();
	
	_asm cli;
	// Modulos que si requieren interrupciones para inicializar	
	rtc_init();
}


void stepperInit(void)
{
	led(255);
	rtc_assignAutoUpdateCallback(NULL);
	printStepperPos();	
}

void pwmInit(void)
{
	led(0);
	rtc_assignAutoUpdateCallback(dispTime);
	dispTime();
}

void stepperLoop(void)
{
	if (sys_data.mode != STEPPER)
		return;
	
	stepperInit();
	while (sys_data.mode == STEPPER)
	{
		printStepperPos();
		
		if (in_getCommand(sys_data.mode) == _FALSE)
			continue;
			
		switch (commandData.type)
		{
			case MOV_FWD:
				smot_stop();
				smot_setpoint(commandData.quantity);	// VER si está prendido
				break;
			
			case MOV_BWD:
				smot_stop();
				smot_setpoint(-(s16) (commandData.quantity));
				break;
			
			case STOP: case RUN:
				smot_spreset(); 
				smot_speed(commandData.quantity);
				break;
				
			case SP_RESET:
				smot_stop();
				smot_spreset();
				break;
				
			case MODE_TOGGLE:
				sys_data.mode = PWM;
				break;
			
			default:
				break;
		}
	}
}


void pwmLoop(void)
{

	if (sys_data.mode != PWM)
		return;
	
	pwmInit();
	
	while (sys_data.mode == PWM)
	{
		if (in_getCommand(sys_data.mode) == _FALSE)
			continue;
		
		switch (commandData.type)
		{
			case DUTY_UP:
				if (sys_data.pwm_data.speed < MAX_SPEED) 
					mot_speed(++sys_data.pwm_data.speed);
				break;
			
			case DUTY_DOWN:
				if (sys_data.pwm_data.speed > MIN_SPEED) 
					mot_speed(--sys_data.pwm_data.speed);;
				break;
			
			case PERIOD_CHANGE:
				mot_speed(sys_data.pwm_data.periods[commandData.quantity]);
				sys_data.pwm_data.speed = commandData.quantity;
				break;
				
			case MODE_TOGGLE:
				sys_data.mode = STEPPER;
				break;
			
			default:
				break;
		}
	}
}


void dispTime(void)
{
	disp_ram[0] = rtc_data.minutes.deca + '0';
	disp_ram[1] = rtc_data.minutes.uni + '0';
	disp_att_ram[1].use_dot = 1;
	disp_ram[2] = rtc_data.seconds.deca + '0';
	disp_ram[3] = rtc_data.seconds.uni + '0';
	
	return;
}

void clearDisplay(void)
{
	disp_ram[0] = SPACE;
	disp_att_ram[0].use_dot = 0;
	disp_ram[1] = SPACE;
	disp_att_ram[1].use_dot = 0;
	disp_ram[2] = SPACE;
	disp_att_ram[2].use_dot = 0;
	disp_ram[3] = SPACE;
	disp_att_ram[3].use_dot = 0;
		
	return;
}

void printStepperPos(void)
{
	s16 pos = smot_getpos();
	if (pos < 0)
	{
		disp_ram[0] = '-';
		pos = -pos;
	}
	else
		disp_ram[0] = SPACE;
	
	disp_ram[1] = (pos/100)%10 + '0';
	disp_ram[2] = (pos/10)%10 + '0';
	disp_ram[3] = (pos)%10 + '0';
	disp_att_ram[1].use_dot = 0;
	
	return;
}


#include <stdio.h>

void setHour(void)
{
	decimal d = {0, 1};
	decimal hour[2];
	s16 kbInput;
	u8 i = 0;
	bool done = _FALSE;

	clearDisplay();
	disp_ram[i] = '_';
	
	while(!done)
	{
		if ((kbInput = ir_pop()) < 0) 
			continue;

		printf("kb: %d\n", kbInput);
		
		kbInput = (s16) filterInt (kbInput);
		
		switch (kbInput)
		{
			case RC5_PREV_CH: //case KB_SPECIAL_L:
				if (i > 0)
				{
					disp_ram[i] = SPACE;
					disp_ram[--i] = '_';
				}
				break;
			case RC5_POWER: //case KB_SPECIAL_R:
				if (i > 3 || i == 0)		// i from 0 to 3
					done = _TRUE;
				break;
			default:
				if (i > 3)
					break;
				((u8*)hour)[i] = (u8)kbInput;
				disp_ram[i++] = (u8)kbInput + '0';
				if (i <= 3)
					disp_ram[i] = '_';
				break;
		
		}
	}
	
	if (i == 0)
		return;
	
	rtc_setTime( hour[1], hour[0], d, d, d, d, RTC_MONDAY);
	return;
}






/* **********************************************************************
 * Funciones de envÌo y recepciÛn de datos por puerto serie.
 * Los accesos a los streams stdout, stdin, stderr entran y salen por ac·
 * ********************************************************************** */
#include "mc9s12xdp512.h"
 
void TERMIO_PutChar(char ch) 
{
    while (!(SCI0SR1 & SCI0SR1_TDRE_MASK));
    SCI0DRL = ch;
}
char TERMIO_GetChar(void) 
{
    while (!(SCI0SR1 & SCI0SR1_RDRF_MASK));
    return SCI0DRL;
}