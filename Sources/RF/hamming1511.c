#include "hamming1511.h"

u8 hamm_lookupTable[1024];

void hamm_GetParityBits(u16 *data)
{
	u16 index;
	(*data) = (*data) & 0xFFE0;
	index = ((*data) & 0x7FFF) >> 5;
	
	if ((*data) < 32768) 
		(*data) = (*data) + ((u16)(hamm_lookupTable[index] & 0xF0) >> 3) ;
	else
		(*data) = (*data) + ((u16)(hamm_lookupTable[index] & 0x0F) << 1);
}

bool hamm_DecodeWord(u16 *data)
{
	u16 index;
	index = ((*data) & 0x7FF0) >> 5;
	
	if ((*data) < 32768) 
		if (((*data) & 0x1E) != ((u16)(hamm_lookupTable[index] & 0xF0) >> 3))
		{
			//correct error
			return _FALSE;
		}
	else
		if (((*data) & 0x1E) != ((u16)(hamm_lookupTable[index] & 0xF0) << 1))
		{
			//correct error
			return _FALSE;
		}
	
	return _TRUE;
}