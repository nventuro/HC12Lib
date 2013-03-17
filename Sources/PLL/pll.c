#include "pll.h"
#include "mc9s12xdp512.h"

void pll_init(char synr, char refdv)
{
	CRGINT_LOCKIE = 0;
	CLKSEL_PLLSEL = 0; /* andamos con el osc lento */
	PLLCTL_PLLON = 1;
	SYNR = synr;
	REFDV = refdv;
	/*espero que enganche */
	while (!(CRGFLG_LOCK))
		;
	CLKSEL_PLLSEL = 1; /* ponemos 5ta a fondo */
}

