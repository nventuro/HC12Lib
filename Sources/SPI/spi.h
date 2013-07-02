#ifndef _SPI_H
#define _SPI_H

#include "common.h"
#include "mc9s12xdp512.h"

// Pin SS0 goes down at the beginning of every 8 bit SPI transfer, and up when it ends.
// Pin SPI_LONG_SS goes down at the same time as SS0, but only goes up when spi_Transfer ends. 
// Depending on the device, connect the SS (CSN) line to either SS0 or SPI_LONG_SS
#define SPI_LONG_SS PTS_PTS3 
#define SPI_LONG_SS_DDR DDRS_DDRS3 // The DDR register for SPI_LONG_SS

typedef void (*spi_ptr)(void);

void spi_Init (bool CPOL, bool CPHA);
// Initializes the SPI module. This requires no other modules to work, and doesn't require interrupts to be enabled.
// CPOL and CPHA set the SPI clock's (SCK) polarity and phase.
// The HCs12 will be set as master, and only one slave can be connected on the bus.
// The SCK frequency is set to 195 kHz (for a 50 MHz bus clock). To change that value, spi_Init in spi.c must be modified.

void spi_Transfer (u8 *input, u8 *output, u8 length, spi_ptr eot);
// Initiates a SPI data transfer. length bytes from input are sent via SPI, and the data clocked in is stored in output.
// When the transmission ends, the SPI module is ready for a new data transfer, and eot is called. eot and input must not be 
// NULL, output can be NULL, in which case it is ignored. length must be non-zero. It is up to the user to ensure there's enough
// memory in both input and output.
// If spi_Transfer is called while there's already another transfer in progress, an error is thrown.

bool spi_IsBusy (void);
// spi_Transfer can only be called if the SPI module isn't busy. spi_Transfer ensures the module won't be busy
// by the time eot is called.

void interrupt spi0_Service (void);

#endif