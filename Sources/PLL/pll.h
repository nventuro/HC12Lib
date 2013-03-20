#ifndef _PLL_H_INCLUDED_
#define _PLL_H_INCLUDED_

#include "common.h"

#define XTAL_KHZ ((long long)16000)

#define SYNR_80 4
#define REFDV_80 0

#define SYNR_72 17
#define REFDV_72 3

#define SYNR_60 14
#define REFDV_60 3

#define SYNR_50 24
#define REFDV_50 7

#define SYNR_40 4
#define REFDV_40 2

#define SYNR_48 5
#define REFDV_48 2

#define SYNR_24 2
#define REFDV_24 1

#define PLL_SPEED(s) pll_init(GLUE(SYNR_,s), GLUE(REFDV_,s))
// Use this macro with one of the numbers above to initialize the PLL with custom speed.


#define BUS_KHZ(synr, refdv) (((XTAL_KHZ)*((synr)+1))/((refdv)+1))

void pll_init(char synr, char refdv);

#endif