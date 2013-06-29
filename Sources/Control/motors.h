#ifndef _MOTORS_H_INCLUDED_
#define _MOTORS_H_INCLUDED_

#include "arith.h"

typedef enum {MOT_MANUAL=0, MOT_AUTO}motorMode;

struct motorData{

	motorMode mode;
	frac speed[4];
};

void mot_Init(void);

#endif