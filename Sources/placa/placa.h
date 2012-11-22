/*
 * placa.h
*
* grupo 2 - 2012
 */

#ifndef __PLACA_H__
#define __PLACA_H__

#include "derivative.h"

#define DISP_DATA_PORT	PORTA
#define DISP_DATA_DD	DDRA

#define DISP_CTL_PORT	PORTB
#define DISP_CTL_DD	DDRB

#define DISP_DOT_BIT	PORTA_PA7

#define SHARED_OUT(p, m, d) ((p) = (((p) & ~(m)) | ((d) & (m))))

#define mDISP		(0x0F)
#define DISP_CTL_OUT(v)	SHARED_OUT(DISP_CTL_PORT,mDISP, v)
#define DISP_CTL(i)	DISP_CTL_OUT(~(1<<(DISP_SIZE-1-(i))))
#define DISP_OFF()	DISP_CTL_OUT(0xFF)

#define KB_PORT	PORTB
#define KB_DD	DDRB

#define KB_READ_BOFF	4
#define mKB_READ	(0x7 << KB_READ_BOFF)
#define mKB_WRITE	0x0F
#define mKB	(mKB_READ|mKB_WRITE)
#define KB_PROBE(line)	SHARED_OUT(KB_PORT, mKB_WRITE, ~(1<<(line)))
#define KB_READ()	((KB_PORT & mKB_READ) >> KB_READ_BOFF)

#endif /* __PLACA_H__ */
