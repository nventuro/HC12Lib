#ifndef _COMM_QUEUE_H
#define _COMM_QUEUE_H

#include "common.h"

void hamm_GetParityBits(u16 *data);
//Adds the 4 parity bits corresponding to a 15/11 Hamming Error Correcting Code to a 11 bit word. The 11 data bits are the
//11 least significant bits (bits 10 to 0) of data. The 4 parity bits are added on bits 14 to 11. Bit 15 is 
//unused.
//data's bits 15 to 11 must be set to 0.

bool hamm_DecodeWord(u16 *data);
//Recieves an 11 bit word encoded using hamm_GetParityBits, and decodes it, leaving the decoded 11 bits on bits 10 
// to 0. The other 5 bits are unused. 
//Returns _TRUE if no error was detected, and _FALSE if an error was detected and corrected.

#endif