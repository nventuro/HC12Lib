/* Circular buffer
 * 
 * Grupo 2 - 2012
*/

#include "common.h"
#include "cb.h"
/*
typedef int sint;
typedef char schar;

#define CB_LENGTH 32
*/

cbuf cb_create(u8 *mem, char len)
{
	cbuf r;
	
	r.mem = mem;
	r.w = r.r = r.mem;
	r.len = len;
	r.status = CB_EMPTY;
	
	return r;
}

int cb_push(cbuf* buffer, u8 data) 
{
	if (buffer->status == CB_FULL)
		return CB_FULL;
	
	*buffer->w++ = data;
	
	if ((buffer->w - buffer->mem) >= buffer->len)
		buffer->w = buffer->mem;
	
	if (buffer->w == buffer->r)
		buffer->status = CB_FULL;
	else
		buffer->status = CB_OK;
	
	return CB_OK;
}

int cb_pop(cbuf* buffer) 
{
	u8 readVal;
	
	if (buffer->status == CB_EMPTY)
		return CB_EMPTY;
	
	readVal = *(buffer->r++);
	
	if ((buffer->r - buffer->mem) >= buffer->len)
		buffer->r = buffer->mem;
	
	if (buffer->r == buffer->w)
		buffer->status = CB_EMPTY;
	else
		buffer->status = CB_OK;
	
	return readVal;
}


int cb_flush(cbuf* buffer) 
{
	int erasedSize;
	
	if (buffer->status == CB_OK) 
	{
		if(buffer->w > buffer->r)
		{
			erasedSize = buffer->w - buffer->r;
		} 
		else // buffer->w < buffer->r 
		{
			erasedSize = buffer->len - (buffer->r - buffer->w);
		}
	} 
	else if (buffer->status == CB_FULL) 
	{
		erasedSize = buffer->len;
	} 
	else // buffer->status == CB_EMPTY
	{
		erasedSize = 0;
	}
	
	buffer->r = buffer->w = buffer->mem;
	buffer->status = CB_EMPTY;
	
	return erasedSize;
}
