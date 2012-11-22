/*
 * display.c
 * 
 * Control de Display
 * Lab de micros - 2012 - Grupo X
 *
 */
 
#include "derivative.h"
#include "../common.h"
#include "../rti/rtix.h"
#include "display.h"
#include "../util/a7table.h"
#include "placa.h"

#define DISP_WRITE(w)	(DISP_DATA_PORT = (ascii_to_7[(u8)(w)]))

#define CNT_DISP_ON	0

char disp_ram[DISP_SIZE];
struct disp_att disp_att_ram[DISP_SIZE];

struct DispMem
{	
	u8	call_cnt;
	u8	call_idx;
	u8	blink_on;
	rti_time blink_rate;
};

static struct DispMem disp_cfg;

rti_time display_ctl(void *_data, rti_time pw);
rti_time display_toggle_blink (void *_data, rti_time pw);
void disp_set_blink_rate(rti_time rate);
void display_init(void);

rti_time display_ctl(void *_data, rti_time pw)
{
	DISP_OFF();
	
	DISP_WRITE(disp_ram[disp_cfg.call_idx]);
	DISP_DOT_BIT = disp_att_ram[disp_cfg.call_idx].use_dot;
	
	if (((disp_cfg.call_cnt == CNT_DISP_ON) || (disp_att_ram[disp_cfg.call_idx].dim == DISP_FULL_DIM)) 
		&& (disp_cfg.blink_on || (disp_att_ram[disp_cfg.call_idx].blink == DISP_NO_BLINK)))
		DISP_CTL(disp_cfg.call_idx);

	if ((++disp_cfg.call_idx) == DISP_SIZE)
	{
		disp_cfg.call_cnt = !disp_cfg.call_cnt;		
		disp_cfg.call_idx = 0;
	}
	
	return pw;
}

rti_time display_toggle_blink (void *_data, rti_time pw)
{
	disp_cfg.blink_on = !disp_cfg.blink_on;
	
 	return disp_cfg.blink_rate;	
}

void disp_set_blink_rate(rti_time rate)
{
	disp_cfg.blink_rate = rate;
}

rti_time disp_get_blink_rate (void)
{
	return disp_cfg.blink_rate;
}

void display_init(void)
{
	int i;
	
	DISP_DATA_DD = 0xFF;
	DISP_CTL_DD = (DISP_CTL_DD & (~mDISP)) | (0xFF & mDISP);
		
	for (i = 0; i < DISP_SIZE; i++)
	{
		disp_ram[i] = SPACE;
		disp_att_ram[i].use_dot = 0;
		disp_att_ram[i].dim = DISP_FULL_DIM;
		disp_att_ram[i].blink = DISP_NO_BLINK;
	}
	
	disp_cfg.call_cnt = 0;
	disp_cfg.call_idx = 0;
	disp_cfg.blink_rate = RTI_MS2PERIOD(500);
	
	rti_register(display_ctl, NULL, RTI_ALWAYS, RTI_NOW);
	rti_register(display_toggle_blink, NULL, disp_cfg.blink_rate, RTI_NOW);
}
