#include "common.h"
#include "mc9s12xdp512.h"
 
void TERMIO_PutChar(char ch) 
{
    while (!(SCI0SR1 & SCI0SR1_TDRE_MASK));
    SCI0DRL = ch;
}

char TERMIO_GetChar(void) 
{
    while (!(SCI0SR1 & SCI0SR1_RDRF_MASK));
    return SCI0DRL;
}

bool SafeSei(void)
{
	u8 ccrCopy;
	u8 a_save;

	_asm staa a_save;
		_asm tpa;
		_asm staa ccrCopy;
		_asm sei;
	_asm ldaa a_save;

	ccrCopy &= (1 << 4);
	
	if (ccrCopy == 0)
		return _TRUE;	// Interrupts were enabled
	else
		return _FALSE;	// Interrupts were not enabled
}