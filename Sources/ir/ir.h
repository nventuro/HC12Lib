#ifndef _IR_H
#define _IR_H

#include "common.h"

#define IR_NO_COMMAND (-1)

#define IR_IC_TIMER 1
#define IR_OC_TIMER 0

void ir_init(void);

s16 ir_push(u8 data);
s16 ir_flush(void);
s16 ir_pop(void);

bool isDigit(u8 _byte);

#endif
