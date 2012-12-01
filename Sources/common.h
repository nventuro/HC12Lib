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

#define SUCCESS (-E_OK)


#define BITx(n) (1<<(n))
#define BIT(v, i) ((v) & BITx(i))
#define _BYTE(n,b) (BITx(n)*BOOL(b))
#define BYTE(b7,b6,b5,b4,b3,b2,b1,b0) ( _BYTE(7,(b7)) | _BYTE(6,(b6)) | \
_BYTE(5,(b6)) | _BYTE(4,(b4)) | _BYTE(3,(b3)) | _BYTE(2,(b2)) | _BYTE(1, (b1))\
| _BYTE(0, (b0)))

#define BSET(p, b, v) ((p) = (((p) & ~BITx(b)) | (BITx(b) & FBOOL(v))))

#define MASKED_WRITE(p, m, d) ((p) = (((p) & ~(m)) | ((d) & (m))))

#define PORTDD_OUT 1

#endif
