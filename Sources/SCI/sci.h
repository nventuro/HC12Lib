#ifndef _SCI
#define _SCI

#include "common.h"

void sci_init(void);
void TERMIO_PutChar(char ch);
char TERMIO_GetChar(void);
unsigned char sci_rx_status(void);
unsigned char sci_rx_data(void);

#define SCI_EMPTY   0
#define SCI_FULL    1

#endif