//Vectores de interrupción para MonS12X

#include "rti.h"
#include "iic.h"
#include "timers.h"


#include <stdio.h>
void interrupt catch_all()
{
	PORTA_PA5 = 1;
	PORTA_PA5 = 0;
}


#ifndef NOTUSED
#define NOTUSED ((const *)0xFFFF)
#endif

extern void _Startup(); /* Startup routine */
extern void ISR_sci();

#pragma CODE_SEG __NEAR_SEG NON_BANKED /* Interrupt section for this module. Placement will be in NON_BANKED area. */

__interrupt void UnimplementedISR(void)  {                                                 
   asm BGND; /* Unimplemented ISRs trap.*/
}                                         

#pragma CONST_SEG VECTORS

void (* const interrupt_vector_table[])() ={
    NOTUSED,	// VECT119	spurious
    NOTUSED,	// VECT118	Reserved119
    NOTUSED,	// VECT117	Reserved118
    NOTUSED,	// VECT116	Reserved117
    NOTUSED,	// VECT115	Reserved116
    NOTUSED,	// VECT114	Reserved115
    NOTUSED,	// VECT113	Reserved114
    NOTUSED,	// VECT112	Reserved113
    NOTUSED,	// VECT111	Reserved112
    NOTUSED,	// VECT110	Reserved111
    NOTUSED,	// VECT109	Reserved110
    NOTUSED,	// VECT108	Reserved109
    NOTUSED,	// VECT107	Reserved108
    NOTUSED,	// VECT106	Reserved107
    NOTUSED,	// VECT105	Reserved106
    NOTUSED,	// VECT104	Reserved105
    NOTUSED,	// VECT103	Reserved104
    NOTUSED,	// VECT102	Reserved103
    NOTUSED,	// VECT101	Reserved102
    NOTUSED,	// VECT100	Reserved101
    NOTUSED,	// VECT99	Reserved100
    NOTUSED,	// VECT98	Reserved99
    NOTUSED,	// VECT97	Reserved98
    NOTUSED,	// VECT96	Reserved97
    NOTUSED,	// VECT95	Reserved96
    NOTUSED,	// VECT94	Reserved95
    NOTUSED,	// VECT93	Reserved94
    NOTUSED,	// VECT92	Reserved93
    NOTUSED,	// VECT91	Reserved92
    NOTUSED,	// VECT90	Reserved91
    NOTUSED,	// VECT89	Reserved90
    NOTUSED,	// VECT88	Reserved89
    NOTUSED,	// VECT87	Reserved88
    NOTUSED,	// VECT86	Reserved87
    NOTUSED,	// VECT85	Reserved86
    NOTUSED,	// VECT84	Reserved85
    NOTUSED,	// VECT83	Reserved84
    NOTUSED,	// VECT82	Reserved83
    NOTUSED,	// VECT81	Reserved82
    NOTUSED,	// VECT80	Reserved81
    NOTUSED,	// VECT79	xsramav
    NOTUSED,	// VECT78	xsei
    NOTUSED,	// VECT77	xst7
    NOTUSED,	// VECT76	xst6
    NOTUSED,	// VECT75	xst5
    NOTUSED,	// VECT74	xst4
    NOTUSED,	// VECT73	xst3
    NOTUSED,	// VECT72	xst2
    NOTUSED,	// VECT71	xst1
    NOTUSED,	// VECT70	xst0
    NOTUSED,	// VECT69	pit3
    NOTUSED,	// VECT68	pit2
    NOTUSED,	// VECT67	pit1
    NOTUSED,	// VECT66	pit0
    NOTUSED,	// VECT65	Reserved65
    NOTUSED,	// VECT64	api
    NOTUSED,	// VECT63	lvi
    NOTUSED,	// VECT62	iic1
    NOTUSED,	// VECT61	sci5
    NOTUSED,	// VECT60	sci4
    NOTUSED,	// VECT59	sci3
    NOTUSED,	// VECT58	sci2
    NOTUSED,	// VECT57	pwmesdn
    NOTUSED,	// VECT56	portp
    NOTUSED,	// VECT55	can4tx
    NOTUSED,	// VECT54	can4rx
    NOTUSED,	// VECT53	can4err
    NOTUSED,	// VECT52	can4wkup
    NOTUSED,	// VECT51	can3tx
    NOTUSED,	// VECT50	can3rx
    NOTUSED,	// VECT49	can3err
    NOTUSED,	// VECT48	can3wkup
    NOTUSED,	// VECT47	can2tx
    NOTUSED,	// VECT46	can2rx
    NOTUSED,	// VECT45	can2err
    NOTUSED,	// VECT44	can2wkup
    NOTUSED,	// VECT43	can1tx
    NOTUSED,	// VECT42	can1rx
    NOTUSED,	// VECT41	can1err
    NOTUSED,	// VECT40	can1wkup
    NOTUSED,	// VECT39	can0tx
    NOTUSED,	// VECT38	can0rx
    NOTUSED,	// VECT37	can0err
    NOTUSED,	// VECT36	can0wkup
    NOTUSED,	// VECT35	flash
    NOTUSED,	// VECT34	eeprom
    NOTUSED,	// VECT33	spi2
    NOTUSED,	// VECT32	spi1
    iic0_srv,	// VECT31	iic0
    NOTUSED,	// VECT30	Reserved30
    NOTUSED,	// VECT29	crgscm
    NOTUSED,	// VECT28	crgplllck
    NOTUSED,	// VECT27	timpabovf
    NOTUSED,	// VECT26	timmdcu
    NOTUSED,	// VECT25	porth
    NOTUSED,	// VECT24	portj
    NOTUSED,	// VECT23	atd1
    NOTUSED,	// VECT22	atd0
    NOTUSED,	// VECT21	sci1
    NOTUSED,	// VECT20	sci0
    NOTUSED,	// VECT19	spi0
    NOTUSED,	// VECT18	timpaie
    NOTUSED,	// VECT17	timpaaovf
    timOvf_Service,	// VECT16	timovf
    tim7_Service,	// VECT15	timch7
    tim6_Service,	// VECT14	timch6
    tim5_Service,	// VECT13	timch5
    tim4_Service,	// VECT12	timch4
    tim3_Service,	// VECT11	timch3
    tim2_Service,	// VECT10	timch2
    tim1_Service,	// VECT9	timch1
    tim0_Service,	// VECT8	timch0
    rti_Service,// VECT7	rti
    NOTUSED,	// VECT6	irq
    NOTUSED,	// VECT5	xirq
    catch_all,	// VECT4	swi
    NOTUSED,	// VECT3	trap
    NOTUSED,	// VECT2	cop
    NOTUSED,	// VECT1	clkmon
    _Startup,	// VECT0	reset
};

