#include "iic.h"
#include "mc9s12xdp512.h"

#define IIC_START()	(IIC0_IBCR_MS_SL = 1)
#define IIC_STOP() {IIC0_IBCR_MS_SL = 0; iic_data.stoppingBus = _TRUE;}
#define IIC_SEND(a) (IIC0_IBDR = a)
#define IIC_RECEIVE() (IIC0_IBDR)
#define READ 1
#define WRITE 0

#define IIC_MODULE_ENABLE() (IIC0_IBCR_IBEN = 1)
#define IIC_SET_BAUD() (IIC0_IBFD = 0x5F) //80 kHz en SCL
#define IIC_FLG_CLEAR() (IIC0_IBSR_IBIF = 1)
#define IIC_INTERRUPT_ENABLE() (IIC0_IBCR_IBIE = 1)

#define IIC_SET_AS_TX() (IIC0_IBCR_TX_RX = 1)
#define IIC_SET_AS_RX() (IIC0_IBCR_TX_RX = 0)
#define IIC_ACKNOWLEDGE_DATA() (IIC0_IBCR_TXAK = 0)
#define IIC_NOT_ACKNOWLEDGE_DATA() (IIC0_IBCR_TXAK = 1)

#define IIC_ARBITRATION_LOST 1

iic_commData_T iic_commData;

struct {
    iic_ptr currCB;
    iic_ptr eotCB;
    iic_ptr commFailedCB;
    u8 dataIdx;
    bool init;
    bool stoppingBus;
}iic_data = {NULL,NULL,NULL,0,_FALSE,_FALSE};

void iic_init (void);
void iic_read (void);
void iic_read_start (void);
void iic_write (void);

void iic_init (void)
{
	if (iic_data.init == _FALSE)
	{	
		iic_data.init = _TRUE;
		iic_data.stoppingBus = _FALSE;
    	IIC_MODULE_ENABLE();
    	IIC_SET_BAUD();
    	IIC_FLG_CLEAR();
    	IIC_INTERRUPT_ENABLE();
    }
    
    return;
}


bool iic_send (u8 slvAddress, iic_ptr eotCB, iic_ptr commFailedCB)
{
    if ((iic_isBusy()) && (iic_data.stoppingBus))
    	while (iic_isBusy()); // Se espera a que se termine de liberar el bus     
        	   
	iic_data.stoppingBus = _FALSE;    
    
    iic_data.eotCB = eotCB;
    iic_data.commFailedCB = commFailedCB;
    iic_data.currCB = iic_write;
    iic_data.dataIdx = 0;
    
    iic_commData.dataSize--;
    
    IIC_SET_AS_TX();
    
    IIC_START();
    IIC_SEND((slvAddress << 1) + WRITE);

    return _TRUE;
}


bool iic_receive (u8 slvAddress, iic_ptr eotCB, iic_ptr commFailedCB, u8 toRead)
{
	if ((iic_isBusy()) && (iic_data.stoppingBus))
    	while (iic_isBusy()); // Se espera a que se termine de liberar el bus     
        	   
	iic_data.stoppingBus = _FALSE;    
	
    iic_data.eotCB = eotCB;
    iic_data.commFailedCB = commFailedCB;
    iic_data.currCB = iic_read_start;
    iic_data.dataIdx = 0;
    
    iic_commData.dataSize = toRead-1;
    
    IIC_SET_AS_TX();
    
    IIC_START();
    IIC_SEND((slvAddress << 1) + READ);		//precedence
    
    return _TRUE;
}


bool iic_isBusy(void)
{
	if (IIC0_IBSR_IBB == 1)
		return _TRUE;
	else
		return _FALSE;
}


void interrupt iic0_srv (void)
{
    IIC_FLG_CLEAR();   
  
    // Deteccion de eot
    if (iic_data.currCB == iic_data.eotCB)
    {
        IIC_STOP();		
       	 
    	if (IIC0_IBCR_TX_RX == 0)
    		iic_commData.data[iic_data.dataIdx] = IIC_RECEIVE();
    }
    
    // Deteccion de errores
    if (((IIC0_IBSR_RXAK == 1) && (IIC0_IBCR_TX_RX == 1)) || (IIC0_IBSR_IBAL == IIC_ARBITRATION_LOST))
    {
		IIC_STOP();
		
    	iic_data.currCB = iic_data.commFailedCB;
    }
  
    if (iic_data.currCB != NULL)
    	(*iic_data.currCB)();
     
    return;
}


void iic_read_start (void)
{	
	if (iic_commData.dataSize == 0)		// Acá pregunto por 0, a read le mando 1 porque quiero leer 1...	
	{
		IIC_NOT_ACKNOWLEDGE_DATA();
		iic_data.currCB = iic_data.eotCB;
	}
	else
	{
		IIC_ACKNOWLEDGE_DATA();
		iic_data.currCB = iic_read;
	}
	
	IIC_SET_AS_RX();
	iic_commData.data[iic_data.dataIdx] = IIC_RECEIVE(); //Dummy read
}


void iic_read (void)
{
	if (iic_data.dataIdx == (iic_commData.dataSize-1))
	{
		IIC_NOT_ACKNOWLEDGE_DATA();
        iic_data.currCB = iic_data.eotCB;
    }
    
    iic_commData.data[iic_data.dataIdx] = IIC_RECEIVE();
    iic_data.dataIdx++;
        
    return;
}


void iic_write (void)
{
    if (iic_data.dataIdx == iic_commData.dataSize)
        iic_data.currCB = iic_data.eotCB;
    
    IIC_SEND(iic_commData.data[iic_data.dataIdx]);
    iic_data.dataIdx++;
        
    return;
}