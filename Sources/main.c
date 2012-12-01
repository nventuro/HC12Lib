#include "common.h"
 
void init(void);


void main(void)
{		
	init();
	
	for(;;)
	{

	}	
}


void init (void)
{
	// Modulos que no requieren interrupciones para inicializar

	_asm cli;
	
	// Modulos que si requieren interrupciones para inicializar	
}


/* **********************************************************************
 * Funciones de envÃo y recepci€n de datos por puerto serie.
 * Los accesos a los streams stdout, stdin, stderr entran y salen por ac∑
 * ********************************************************************** */
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