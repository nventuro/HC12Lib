#ifndef _COMM_QUEUE_H
#define _COMM_QUEUE_H

#include "common.h"

void hamm_GetParityBits(u16 *data);
//Adds the 4 parity bits corresponding to a 15/11 Hamming Error Correcting Code to a 11 bit word. The 11 data bits are the
//11 most significant bits (bits 15 to 5) of data. The 4 parity bits are added AT THE END of data, on bits 4-1. Bit 0 is 
//unused.

bool hamm_DecodeWord(u16 *data);
//Recieves an 11 bit word encoded using hamm_GetParityBits, and decodes it, leaving the decoded 11 bits in the beginning
//of the word (bits 15 to 5). The other 5 bits are unused. 
//Returns _TRUE if no error was detected, and _FALSE if an error was detected and corrected.

#endif