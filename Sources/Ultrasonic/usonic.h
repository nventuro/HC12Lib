#ifndef _USONIC_H
#define _USONIC_H

#include "common.h"

#define USONIC_TRIGG_TIMER 6
#define USONIC_ECHO_TIMER 5

#define USONIC_INVALID_MEAS (-1)

typedef void (*usonic_ptr) (s32 measurement);

void usonic_Init(void);
void usonic_Measure (usonic_ptr callback);

#endif