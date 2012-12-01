#ifndef _COMMON_H
#define _COMMON_H


// Typedefs

typedef unsigned char u8;
typedef char s8;
typedef unsigned int u16;
typedef int s16;
typedef unsigned long int u32;
typedef long int s32;

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


// Macros

#ifndef NULL
#define NULL ((void*)0)
#endif

#define DIV_CEIL(a,b) (((a)/(b))+1) // ceil(a/b)

#define BOOL(a) ((a)?1:0)

#define BIT(n) (1<<(n))


// Monitor IO

void TERMIO_PutChar(char ch);
char TERMIO_GetChar(void);

#endif
