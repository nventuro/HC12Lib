#ifndef _MOTORS_H_INCLUDED_
#define _MOTORS_H_INCLUDED_

#include "arith.h"

struct motorData{

	frac speed[4];
};

void mot_Init(void);

#endif