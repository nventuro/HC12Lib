#include "spi.h"
#include "mc9s12xdp512.h"
#include "error.h"

#define SPI_SPTEF_READY 1
#define SPI_SPIF_READY 1

#define SPI_READ() SPI0DR
#define SPI_WRITE(x) (SPI0DR = x)

typedef struct 
{
	u8 *input;
	u8 *output;
	u8 length;
	spi_ptr eot;
} spi_transferData;


struct {
	bool busy;
	spi_transferData currTransfer;
	u8 index;
} spi_data;

bool spi_isInit = _FALSE;

void spi_sendNewData (void);
void spi_storeReceived (void);

void spi_Init (bool CPOL, bool CPHA)
{
	if (spi_isInit == _TRUE)
		return;
	
	spi_isInit = _TRUE;	
	
	spi_data.busy = _FALSE;
	spi_data.currTransfer.input = NULL;
	spi_data.currTransfer.output = NULL;
	spi_data.currTransfer.length = 0;
	spi_data.currTransfer.eot = NULL;

	// Configuration
	SPI0CR1_SPIE = 1; // Interrupts enabled
	SPI0CR1_SPTIE = 0; // SPTEF interrupt disabled
	SPI0CR1_MSTR = 1; // Master
	SPI0CR2_SPC0 = 0; // Unidirectional
	
	SPI0CR1_CPOL = CPOL; // Active high
	SPI0CR1_CPHA = CPHA; // Trigger on falling edge
	SPI0CR1_LSBFE = 0; // Most significant bit first
	
	SPI0CR1_SSOE = 1; // Automatic slave select
	SPI0CR2_MODFEN = 1; // MODF error enabled (required for SSOE = 1)
		
	// Baudate = 50 MHz / ((0+1)*2^(7+1)) = 195 kHz
	SPI0BR_SPR = 7;
	SPI0BR_SPPR = 0;

	SPI0CR1_SPE = 1; // Enable
	
	return;
}

void spi_Transfer (u8 *input, u8 *output, u8 length, spi_ptr eot)
{
	bool intsEnabled = SafeSei();
	
	if (spi_data.busy == _TRUE)
		err_Throw("spi: attempt to initiate a transfer while another is in progress.\n");
	
	if (input == NULL)
		err_Throw("spi: received NULL input pointer.\n");
	
	if (length == 0)
		err_Throw("spi: length must be non-zero.\n");
	
	if (eot == NULL)
		err_Throw("spi: received NULL end of transmission callback.\n");
	
	spi_data.busy = _TRUE;
	spi_data.currTransfer.input = input;
	spi_data.currTransfer.output = output;
	spi_data.currTransfer.length = length;
	spi_data.currTransfer.eot = eot;
	spi_data.index = 0;
	
	spi_sendNewData();
	SafeCli(intsEnabled);
}

void interrupt spi0_srv (void)
{
	spi_storeReceived();
	
	spi_data.index++;
	if (spi_data.index != spi_data.currTransfer.length)			
		spi_sendNewData();
	else
	{
		spi_data.busy = _FALSE;
		spi_data.currTransfer.eot();	
	}
}


void spi_sendNewData (void)
{
	while (SPI0SR_SPTEF != SPI_SPTEF_READY) // Just in case, shouldn't be too long
		;
	
	SPI_WRITE(spi_data.currTransfer.input[spi_data.index]);
}

void spi_storeReceived (void)
{
	u8 temp;
	
	while (SPI0SR_SPIF != SPI_SPIF_READY) // Just in case, shouldn't be too long
		;
	
	if (spi_data.currTransfer.output != NULL)
		spi_data.currTransfer.output[spi_data.index] = SPI_READ();
	else 
		temp = SPI_READ(); // Dummy read to clear SPIF
}