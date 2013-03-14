#ifndef _CB_H
#define _CB_H

#include "common.h"
#define CB_OK (-1)
#define CB_EMPTY (-2)
#define CB_FULL (-3)

typedef struct {
	u8 *mem;
	u8 *r;
	u8 *w;
	u8 len;
	s16 status;
} cbuf;

cbuf cb_create(u8 *mem, u16 len);

#define cb_status(cb) ((cb)->status)

s16 cb_push(cbuf* buffer, u8 data);

s16 cb_pop(cbuf* buffer);

s16 cb_flush(cbuf* buffer);

#endif
