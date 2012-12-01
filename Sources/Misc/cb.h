#ifndef _CB_H
#define _CB_H

#include "common.h"

typedef enum 
{
	CB_OK, 
	CB_EMPTY, 
	CB_FULL
} cb_status;

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
