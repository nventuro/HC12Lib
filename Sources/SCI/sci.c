#include "mc9s12xdp512.h"
#include "sci.h"

static unsigned char sci_int = SCI_EMPTY;  //SCI driver rx Flag =  SCI_FULL if data was received
static unsigned char rxdata;               //SCI driver rx data contains valid data when sci_int= SCI_FULL


// Driver Init
void sci_init(void) {
    //El módulo SCI0 ya está inicializado por el monitor!
    
    //Lo único que resta es activar interrupciones...
    SCI0CR2 |= SCI0CR2_RIE_MASK;
}

// Driver ISR
void interrupt ISR_sci(void)
{ 
    if (SCI0SR1 & SCI0SR1_RDRF_MASK) {
        rxdata = SCI0DRL;
        sci_int=SCI_FULL;
    }
}

// Driver Services
unsigned char sci_rx_status(void)    // returns SCI Status (SCI_FULL if data Available)
{
    return sci_int;
}

unsigned char sci_rx_data(void)  // returns SCI Data
{
    sci_int = SCI_EMPTY;
    return rxdata;
}