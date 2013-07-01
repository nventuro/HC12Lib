#ifndef _SPI_H
#define _SPI_H

#include "common.h"

typedef void (*spi_ptr)(void);

void iic_Init (void);
// 	Initializes the SPI module. This requires no other modules to work, and doesn't require interrupts to be enabled.

void spi_Transfer (u8 *input, u8 *output, u8 length, spi_ptr eot);

void interrupt spi0_srv (void);

#endif