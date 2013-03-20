#include "hamming1511.h"

u8 hamm_lookupTable[1024];

void hamm_GetParityBits(u16 *data)
{
	u16 index;
	index = (*data) & 0x03FF;
	
	if ((*data) < 32768) 
		(*data) = (*data) + (((u16)(hamm_lookupTable[index] & 0xF0)) << 7) ;
	else
		(*data) = (*data) + (((u16)(hamm_lookupTable[index] & 0x0F)) << 11);
}

bool hamm_DecodeWord(u16 *data)
{
	u16 index;
	u8 parity;
	
	index = (*data) & 0x03FF;
	parity = ((*data) & 0x7800) >> 11;
	
	if ((*data) < 32768) 
		if (parity != (u8) (((u16)(hamm_lookupTable[index] & 0xF0) >> 3)))
		{
			//correct error
			return _FALSE;
		}
	else
		if (parity != (u8) (((u16)(hamm_lookupTable[index] & 0xF0) << 1)))
		{
			//correct error
			return _FALSE;
		}
	
	return _TRUE;
}