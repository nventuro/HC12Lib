/* pwm.c
 * 
 * Lab de micros - Grupo 2 - 2012
 * 
 */

#include "derivative.h"
#include "common.h"
#include "pwm.h"

#define DEF_POL 0x00
#define DEF_CLK 0x00
#define DEF_ALIGN 0x00
#define DEF_CON 0
#define	DEF_PWMPRE 7
#define DEF_PWMPER 0xFF
#define DEF_PWMDTY 0xC0

byte *const pwm_per [] = {&PWMPER0,&PWMPER1,&PWMPER2,&PWMPER3,&PWMPER4,&PWMPER5,&PWMPER6,&PWMPER7};
byte *const pwm_dty [] = {&PWMDTY0,&PWMDTY1,&PWMDTY2,&PWMDTY3,&PWMDTY4,&PWMDTY5,&PWMDTY6,&PWMDTY7};

void pwm_init (void) 
{
	PWMPOL = DEF_POL; 
	PWMCLK = DEF_CLK;  
	PWMCAE = DEF_ALIGN; 
	PWMCTL = DEF_CON; 
	PWMSDN = 0;
	
	PWMPRCLK_PCKB = DEF_PWMPRE; 
	PWMPRCLK_PCKA = DEF_PWMPRE;
	
	PWMPER0 = PWMPER1 = PWMPER2 = PWMPER3 = PWMPER4 = PWMPER5 = PWMPER6 = PWMPER7 = DEF_PWMPER;	//contador del PWM6
	PWMDTY0 = PWMDTY1 = PWMDTY2 = PWMDTY3 = PWMDTY4 = PWMDTY5 = PWMDTY6 = PWMDTY7 = DEF_PWMDTY;
	
}

/*
void pwm_set_chan(char chan, char pol, char clk, char cae, char ctl)
{
	
	if (pol >= 0)
		BSET(PWMPOL,chan,pol);
	if (clk >= 0)
		BSET(PWMCLK,chan,clk);
	if (cae >= 0)
		BSET(PWMCAE,chan,cae);
	if (ctl >= 0)
		BSET(PWMCTL,chan,ctl);
		
}

void pwm_set_clkpre (int prea, int preb)
{
	if (prea >= 0)
		PWMPRCLK_PCKA = prea;
	if (preb >= 0)
		PWMPRCLK_PCKB = preb;
}


void pwm_set_per (u8 chan, u8 per)
{
	*pwm_per[chan] = per;
}

void pwm_set_dty (u8 chan, u8 dty)
{
	*pwm_dty[chan] = dty;
}

void pwm_ctrl (char chan,char state)
{
	BSET(PWME,chan,state);
}

*/