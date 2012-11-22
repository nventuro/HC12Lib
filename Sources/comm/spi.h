/*
 * spi.h
 * 
 * Modulo SPI
 * Lab de micros - 2012 - Grupo X
 *
 */

#ifndef __SPI_H__
#define __SPI_H__

#include "common.h"

void spi_init (void); 
char spi_fast_rxtx (char tx_char);
void putcSPI1 (char cx);


#endif /* __SPI_H__ */
