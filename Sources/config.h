#ifndef _CONF
#define _CONF

/* Definir RAM o FLASH según necesite.          */
/* No olvidar cambiar el Target! (RAM o Flash)  */

#define RAM
//#define FLASH



#if !defined(FLASH) && !defined(RAM)
#error Debe definir ya sea Flash o RAM
#endif
#if defined(FLASH) && defined(RAM)
#error No puede compilar para RAM y Flash a la vez!
#else
#ifdef FLASH
#warning Recuerde que el PLL es inicializado para 24MHz de bus por el programa Monitor, pero en modo Flash el PLL no es inicializado por nadie.
#endif
#endif 

#endif