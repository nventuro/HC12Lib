/* common.h
 *
 * Common Macros
 *  */

#ifndef _COMMON_H
#define _COMMON_H

#ifdef __unix__
#include <stdint.h>
#endif

#define BUS_CLOCK_MHZ 50

/*  Typedefs */

/* 8-bit */
typedef unsigned char u8;
typedef char s8;


#ifdef __unix__

#define S16_MAX INT16_MAX
#define S16_MIN INT16_MIN
typedef uint16_t u16;
typedef int16_t s16;

#define S32_MAX INT32_MAX
#define S32_MIN INT32_MIN
typedef uint32_t u32;
typedef int32_t s32;

#elif (defined __HC12__)

#define S16_MAX ((s16)((((u16)1)<<16)-1))
#define S16_MIN ((s16)(((u16)1)<<16))
typedef unsigned int u16;
typedef int s16;

#define S32_MAX ((s32)((((u32)1)<<31)-1))
#define S32_MIN ((s32)(((u32)1)<<31))
typedef unsigned long int u32;
typedef long int s32;

#endif /*__unix__*/

/* Booloean */
typedef char bool;
#define _FALSE 0
#define _TRUE 1

typedef union {
	u8 v;
	struct {
		int b0:1;
		int b1:1;
		int b2:1;
		int b3:1;
		int b4:1;
		int b5:1;
		int b6:1;
		int b7:1;
	};
} bit;

/* Macros */

#ifndef NULL
#define NULL ((void*)0)
#endif

#define DIV_CEIL(a,b) (((a) % (b) == 0) ? ((a)/(b)) : (((a)/(b))+1))

#define PIN_LOW 0
#define PIN_HIGH 1

#define BOOL(a) ((a) ? _TRUE:_FALSE)

#define BIT(n) (1<<(n))

extern u8 firstBitsMem[]; /* firtBitsMem[0] = 00000001, firstBitsMem[3] = 00001111 */
extern u8 lastBitsMem[]; /* lastBitsMem[0] = 10000000, lastBitsMem[3] = 11110000 */

#define firstBits(n) ((n > 7) ? 0xFF : ((n < 0) ? 0 : firstBitsMem[n]))
#define lastBits(n) ((n > 7) ? 0xFF : ((n < 0) ? 0 : lastBitsMem[n]))

#define GLUE(a,b) GLUE_AGAIN(a,b)
#define GLUE_AGAIN(a,b) a ## b

#define GLUE2(a,b,c) GLUE_AGAIN2(a,b,c)
#define GLUE_AGAIN2(a,b,c) a ## b ## c

#define DDR_OUT 0xFF

/* Monitor IO */

void TERMIO_PutChar(char ch);
char TERMIO_GetChar(void);


/* Interrupt inhibiting */

bool SafeSei(void);
/* Inhibits interrupts and returns the value of the I bit before performing
 * this action. To be used in conjunction with SafeCli(). */

#define SafeCli(interruptsWereEnabled) do{if (interruptsWereEnabled) {_asm cli;}} while(0)
/* Disinhibits interrupts if they were previously uninhibited. This function
 * receives the value returned by SafeSei(); */

#endif
