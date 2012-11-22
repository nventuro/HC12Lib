/* motor.h
 * 
 * Lab de micros - Grupo 2 - 2012
 */
 
#ifndef __MOT_H__
#define __MOT_H__

/*
 * motor.c
 * 
 * Lab de micros - Grupo 2 - 2012
 */

enum {MOT_BACKWARD = -1, MOT_STOP, MOT_FORWARD};
enum {SMOT_WAVE, SMOT_FULL, SMOT_HALF};

void smot_init(int auto_off);
void smot_speed(int ppm);
void smot_setpoint(int ppm);
void smot_stop(void);
//void smot_reset(void);
void smot_setpoint(int inc);
/* NO hacer esto mientras el motor está andando !!!!! */

int smot_getdir(void);
int smot_getpos(void);
void smot_spreset(void);

#endif /* __MOT_H__ */


