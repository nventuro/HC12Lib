/*
 * display.h
 * 
 * Control de Display
 * Lab de micros - 2012 - Grupo X
 *
 */
 
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "../common.h"
#include "../rti/rtix.h"

#define DISP_SIZE	4
#define	DISP_FULL_DIM		0
#define DISP_HALF_DIM 		(!DISP_FULL_DIM)
#define	DISP_NO_BLINK		0
#define	DISP_BLINK			(!DISP_NO_BLINK)

#define SPACE	(' ')

struct disp_att {
        int use_dot 	:1;                                   
        int dim		:1; //50 o 100%
		int blink	:1;
};

extern char disp_ram[DISP_SIZE];
extern struct disp_att disp_att_ram[DISP_SIZE];
void disp_set_blink_rate(rti_time rate);
rti_time disp_get_blink_rate (void);

void display_init(void);

#endif /* __DISPLAY_H__ */

