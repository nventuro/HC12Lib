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