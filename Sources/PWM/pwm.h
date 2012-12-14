/* pwm.h
 */

#ifndef __PWM_H__
#define __PWM_H__

#include "common.h"

#define PWM_NO_CHANGE (-1)

#define PWM_ON 1
#define PWM_OFF 0

void pwm_init (void);
void pwm_set_chan(char chan, char pol, char clk, char cae, char ctl);
void pwm_set_clkpre (int prea, int preb);
void pwm_set_per (u8 chan, u8 per);
void pwm_set_dty (u8 chan, u8 dty);
void pwm_ctrl (char chan,char state);

#endif /* __PWM_H__ */
