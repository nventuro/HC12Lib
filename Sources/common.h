/*
 *      common.h
 *      
 *	Cosas que vas a necesitar
 *  Grupo 2 - 2012
 */
#ifndef _LIBCOMMON_H_
#define _LIBCOMMON_H_

//#define DEBUG

#ifndef NULL
#define NULL ((void*)0)
#endif

/* cosas */

#define DIVUP(a,b) (((a)/(b))+1)
#define DIVOFF(a, b) ((a+b)/b)

/* #define PERROR(msg...) fprintf(stderr, msg) */
#define ARSIZE(a) ((sizeof(a))/(sizeof((a)[0])))
#define __MALLOC(S) (S = malloc(sizeof(*(S))))
#define NMALLOC(S, n) (S = malloc((n)*sizeof(*(S))))
#define BOOL(a) ((a)?1:0)
#define FBOOL(a) ((a)?0xFF:0)

enum {E_OK, E_NOMEM, E_BADCFG, E_GRAPHIC, E_DONEHELP, E_BADARGS, E_NOTREADY,
					E_TIMEOUT, E_OTHER, N_ECODES}; 

#define SUCCESS (-E_OK)

/* BITWISE */

#define BITx(n) (1<<(n))
#define BIT(v, i) ((v) & BITx(i))
#define _BYTE(n,b) (BITx(n)*BOOL(b))
#define BYTE(b7,b6,b5,b4,b3,b2,b1,b0) ( _BYTE(7,(b7)) | _BYTE(6,(b6)) | \
_BYTE(5,(b6)) | _BYTE(4,(b4)) | _BYTE(3,(b3)) | _BYTE(2,(b2)) | _BYTE(1, (b1))\
| _BYTE(0, (b0)))

#define BSET(p, b, v) ((p) = (((p) & ~BITx(b)) | (BITx(b) & FBOOL(v))))

/* masks */

#define MASKED_WRITE(p, m, d) ((p) = (((p) & ~(m)) | ((d) & (m))))

/*TYPEDEFS */

typedef unsigned char u8;
typedef unsigned int uint;
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

typedef unsigned char u8;
typedef char s8;
typedef unsigned int u16;
typedef int s16;
typedef unsigned long int u32;
typedef long int s32;

 typedef enum{
	_FALSE = 0,
	_TRUE
}bool;

/* puertos */

#define PORTDD_OUT 1

/* Warnings */

#ifdef DEBUG
#warning Building with DEBUG options
#endif

#endif /* _LIBCOMMON_H_ */
