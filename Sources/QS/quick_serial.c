/*
 * quick_serial.c
 * 
 */

#include "derivative.h"
#include "common.h"
#include "quick_serial.h"


/* si los de freescale hubieran hecho bien su header esto no seria
	necesario */

volatile SCI0SR2STR *sr2[] = {&_SCI0SR2, &_SCI1SR2, &_SCI2SR2, &_SCI3SR2, &_SCI4SR2, &_SCI5SR2};
#define SR2(n) sr2[n]->Bits

volatile SCI0SR1STR *sr1[] = {&_SCI0SR1, &_SCI1SR1, &_SCI2SR1, &_SCI3SR1, &_SCI4SR1, &_SCI5SR1};
#define SR1(n) sr1[n]->Bits

volatile word *bd[] = {&SCI0BD, &SCI1BD, &SCI2BD, &SCI3BD, &SCI4BD, &SCI5BD};
volatile byte *drl[] = {&SCI0DRL, &SCI1DRL, &SCI2DRL, &SCI3DRL, &SCI4DRL, &SCI5DRL};

volatile SCI0ACR2STR *cr1[] = {&_SCI0ACR2, &_SCI1ACR2, &_SCI2ACR2, &_SCI3ACR2, &_SCI4ACR2, &_SCI5ACR2};
#define CR1(n) cr1[n]->SameAddr_STR.SCI0CR1STR.Bits
volatile SCI0CR2STR *cr2[] = {&_SCI0CR2, &_SCI1CR2, &_SCI2CR2, &_SCI3CR2, &_SCI4CR2, &_SCI5CR2};
#define CR2(n) cr2[n]->Bits
#define CR2_ALL(n) cr2[n]->Byte


void qs_init(int mod, u32 brate)
{
	SR2(mod).AMAP = 0;
	
	if (brate != BRATE_NO_CHANGE) {
#define BDMASK (0x1FFF)
		u32 bdval;
		
		bdval = ((((u32)BUS_CLOCK_MHZ)*10000)/16)/(brate/100);
		*bd[mod] = (bdval & BDMASK);
		/*es importante que primero se escriba la parte alta
			y despuñes la parte baja, pero el 12 es big endian*/
	}

	CR1(mod).LOOPS = 0;
	CR1(mod).M = 0;
	CR1(mod).PE = 0;
	
	CR2_ALL(mod) = 0; /*esto deshabilita las interrupciones del sci*/
	CR2(mod).TE = 1;
	CR2(mod).RE = 1;
}

void qs_loop(int mod, int loop_mode)
{
	CR1(mod).RSRC = 0;
	
	switch (loop_mode) {
	default:
	case QS_LOOP_NONE:
		CR1(mod).LOOPS = 0;
		break;
	case QS_LOOP_EXT:
		CR1(mod).RSRC = 1;
	case QS_LOOP_INT:
		CR1(mod).LOOPS = 1;
		break;
	}
}

void qs_wait_ready(int mod)
{
	while(!SR1(mod).TDRE);
}

void qs_tx(int mod, char c)
{
	*drl[mod] = c;
}

void qs_putchar(int mod, char c)
{
	qs_wait_ready(mod);
	qs_tx(mod, c);
}

void qs_wait_data(int mod)
{
	while(!SR1(mod).RDRF);
}

char qs_rx(int mod)
{
	return *drl[mod];
}

char qs_getchar(int mod)
{
	qs_wait_data(mod);
	return qs_rx(mod);
}

void qs_write(int mod, char *buf, unsigned int len)
{
	while(len--) {
		qs_putchar(mod, *(buf++));
	}
}