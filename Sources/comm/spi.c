#include "spi.h"
#include "derivative.h"
#define SPI_DATA_REG SPI1DR




void spi_init (void) 
{
	//inicializacion del modulo SPI1
	SPI1CR1_SPIE = 0; //deshab interr
	SPI1CR1_SPTIE = 0;
	SPI1CR1_MSTR = 1; //modo master
	SPI1CR1_CPOL = 0;
	SPI1CR1_CPHA = 0;
	SPI1CR1_LSBFE = 0; //transmito MSB primero
	
	//SS is slave select output
	SPI1CR1_SSOE = 1;	
	SPI1CR2_MODFEN = 1;
	
	
	SPI1CR2_SPISWAI = 0; //SPI clock operates normally in wait mode.
	
	SPI1CR2_SPC0 = 0;
	SPI1CR2_BIDIROE = 0;
	
	
	//Baud Rate =1.25MHz
	SPI1BR_SPR0 = 1;
	SPI1BR_SPR1 = 0;
	SPI1BR_SPR0 = 0;
	SPI1BR_SPPR0 = 0;
	SPI1BR_SPPR1 = 0;
	SPI1BR_SPPR2 = 1;
	
	
	SPI1CR1_SPE = 1; //habilto mod SPI  	
}




char spi_fast_rxtx (char tx_char)
{ 	
	char rx_char;
	
	while(!(SPI1SR_SPTEF))
		;
	
	SPI_DATA_REG = tx_char;
	
	while(!(SPI1SR_SPIF))
		;
	
	rx_char = SPI_DATA_REG;
	return rx_char;	
}

void putcSPI1 (char cx) 	         // Send a character to SPI 
{
    char temp;
    
    while(!(SPI1SR_SPTEF));   /* wait until write is permissible */
    
    SPI1DR = cx;              	       /* output the byte to the SPI */
    
    while(!(SPI1SR_SPIF));     /* wait until write operation is complete */
    
    temp = SPI1DR;	     /* clear the SPIF flag */
}

