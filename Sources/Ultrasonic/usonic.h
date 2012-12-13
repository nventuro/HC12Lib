#ifndef _USONIC_H
#define _USONIC_H

#include "common.h"
#include "mc9s12xdp512.h"

#define USONIC_TRIGG_TIMER 6
#define USONIC_ECHO_TIMER 5

typedef void (*usonic_ptr) (u32 measurement);

void usonic_Init(void);
void usonic_Measure (usonic_ptr callback);

#endif