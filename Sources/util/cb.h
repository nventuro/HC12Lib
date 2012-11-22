/*
 * cb.h
 *
 * grupo 2 - 2012
 * 
 * Buffers circulares
 */

#ifndef __CB_H__
#define __CB_H__

#include "../common.h"

enum {CB_READY, CB_EMPTY, CB_FULL};

typedef struct {
	u8 *mem;
	u8 *r;
	u8 *w;
	u8 len;
	int status;
} cbuf;

cbuf cb_create(u8 *mem, char len);

#define ARRAY2CB(a) cb_create(a, ARSIZE(a))

#define cb_status(cb)	((cb)->status)

int cb_push(cbuf* buffer, u8 data);

int cb_pop(cbuf* buffer);

int cb_flush(cbuf* buffer);

#endif /* __CB_H__ */
