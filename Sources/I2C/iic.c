#include "iic.h"
#include "mc9s12xdp512.h"
#include "debug.h"

#include <stdio.h>

#define IIC_START()	(IIC0_IBCR_MS_SL = 1)
#define IIC_STOP() do {IIC0_IBCR_MS_SL = 0; } while(0)
#define IIC_SEND(a) (IIC0_IBDR = a)
#define IIC_RECEIVE() (IIC0_IBDR)
#define READ 1
#define WRITE 0

#define IIC_MODULE_ENABLE() (IIC0_IBCR_IBEN = 1)
// modify only IBC 6 - 7; refer to HC12 interfacing manual, page 545.
#define IIC_SET_BAUD() (IIC0_IBFD = 0x5F) //80 kHz en SCL
#define IIC_FLG_CLEAR() (IIC0_IBSR_IBIF = 1)
#define IIC_INTERRUPT_ENABLE() (IIC0_IBCR_IBIE = 1)

#define IIC_SET_AS_TX() (IIC0_IBCR_TX_RX = 1)
#define IIC_SET_AS_RX() (IIC0_IBCR_TX_RX = 0)
#define IIC_ACKNOWLEDGE_DATA() (IIC0_IBCR_TXAK = 0)
#define IIC_NOT_ACKNOWLEDGE_DATA() (IIC0_IBCR_TXAK = 1)

#define IIC_ARBITRATION_LOST 1

iic_commData_T iic_commData;

iic_data_T iic_data = {NULL,NULL,NULL,0,_FALSE,_FALSE};

bool* const busIsFreePtr = &iic_data.busIsFree;

void iic_read (void);
void iic_read_start (void);
void iic_write (void);
void iic_FullStagesReceive (void);


void iic_Init (void)
{
	if (iic_data.init == _TRUE)
		return;
	
	iic_data.init = _TRUE;
	iic_data.busIsFree = _TRUE;		// Used in multi-stage transmissions.
	IIC_MODULE_ENABLE();
	IIC_SET_BAUD();
	IIC_FLG_CLEAR();
	IIC_INTERRUPT_ENABLE();
    
    return;
}


void iic_Send (u8 slvAddress, iic_ptr eotCB, iic_ptr commFailedCB, u8 toSend, u8* sendBuffer)
{
    while ((iic_IsBusy()) || !(iic_data.busIsFree));	// Block transfer until ready
    
    iic_data.eotCB = eotCB;
    iic_data.commFailedCB = commFailedCB;
    iic_data.currCB = iic_write;
    iic_data.dataIdx = 0;
    
    iic_commData.dataSize = toSend-1;	// 	New version: dataSize received as parameter for compatibility with receive.
        								//	As before, toSend is the size (to send 1 byte, toSend has to be 1), so the (-1) is for compatibility.
 	if (sendBuffer != NULL)
	 	iic_commData.dataPtr = sendBuffer;
    else
    	iic_commData.dataPtr = iic_commData.data;
    
    IIC_SET_AS_TX();
    
    IIC_START();
    IIC_SEND((slvAddress << 1) + WRITE);
}


void iic_Receive (u8 slvAddress, iic_ptr eotCB, iic_ptr commFailedCB, u8 toRead, u8* receiveBuffer)
{
	while ( (iic_IsBusy()) || !(iic_data.busIsFree));	// Block transfer until ready
	
    iic_data.eotCB = eotCB;
    iic_data.commFailedCB = commFailedCB;
    iic_data.currCB = iic_read_start;
    iic_data.dataIdx = 0;
    
    iic_commData.dataSize = toRead-1;		// toRead es lo que quiero leer, el -1 es necesario para eso.
 	
 	if (receiveBuffer != NULL)
	 	iic_commData.dataPtr = receiveBuffer;
    else
    	iic_commData.dataPtr = iic_commData.data;
    
    IIC_SET_AS_TX();
    
    IIC_START();
    IIC_SEND((slvAddress << 1) + READ);		//precedence
}

#define receive_dataCopy(receiveData, _regAddress, _slaveAddress, _eotCB, _commFailedCB, _toRead, _receiveBuffer, _stage) do \
{	receiveData.regAddress = _regAddress;		\
	receiveData.slaveAddress = _slaveAddress;	\
	receiveData.eotCB = _eotCB; 				\
	receiveData.commFailedCB = _commFailedCB; 	\
	receiveData.toRead = _toRead; 				\
	receiveData.receiveBuffer = _receiveBuffer; \
	receiveData.stage = _stage; } while(0)


void iic_ReceiveFromRegister (u8 regAddress, u8 slaveAddress, iic_ptr eotCB, iic_ptr commFailedCB, u8 toRead, u8* receiveBuffer)
{	 
	while(iic_IsBusy() || (!iic_data.busIsFree) );	// Block transfer until ready.
    	
	receive_dataCopy(iic_commData.transferParameters, regAddress, slaveAddress, eotCB, commFailedCB, toRead, receiveBuffer, 0);
	
	iic_FullStagesReceive();
}


void iic_FullStagesReceive (void)
{
	iic_receiveData_T* rData = &iic_commData.transferParameters; 
	switch (rData->stage)
	{
	case 0:		// Prepare to read: send read address to slave device.
	
		iic_Send(rData->slaveAddress, iic_FullStagesReceive, rData->commFailedCB, 1, &(rData->regAddress));	// Write start read address to slave device
		rData->stage++;
		iic_data.busIsFree = _FALSE;	// Disable bus for other transfers.

		break;

	case 1:		// Start reception itself. Data needed to call iic_Receive can be overwritten in global array if receiveBuffer is NULL, but its OK.
		iic_data.busIsFree = _TRUE;		// Re-Enable for reception.
		iic_Receive(rData->slaveAddress, rData->eotCB, rData->commFailedCB, 
											rData->toRead, rData->receiveBuffer);
		break;
		
	default:
		break;

	}
	
	return;
}


void interrupt iic0_srv (void)
{
    IIC_FLG_CLEAR();
  
    // Deteccion de eot
    if (iic_data.currCB == iic_data.eotCB)
    {

	#ifdef IIC_DEBUG_EOT
		putchar('e');putchar('o');putchar('t');putchar('\n');
	#endif

        IIC_STOP();		
       	 
    	if (IIC0_IBCR_TX_RX == 0)
    		iic_commData.dataPtr[iic_data.dataIdx] = IIC_RECEIVE();    
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
	if (iic_commData.dataSize == 0)		// Ac� pregunto por 0, a read le mando 1 porque quiero leer 1...	
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
	iic_commData.data[0] = IIC_RECEIVE(); //Dummy read - faster if the index is fixed in compile time.
}


void iic_read (void)
{
	if (iic_data.dataIdx == (iic_commData.dataSize-1))
	{
		IIC_NOT_ACKNOWLEDGE_DATA();
        iic_data.currCB = iic_data.eotCB;
    }
    
    iic_commData.dataPtr[iic_data.dataIdx] = IIC_RECEIVE();
    iic_data.dataIdx++;
        
    return;
}


void iic_write (void)
{
    if (iic_data.dataIdx == iic_commData.dataSize)
        iic_data.currCB = iic_data.eotCB;
    
    IIC_SEND(iic_commData.dataPtr[iic_data.dataIdx]);
    iic_data.dataIdx++;
 
    return;
}

void iic_FlushBuffer(void)
{
	u16 i;
	for (i = 0; i < IIC_MEM_SIZE; i++)
		iic_commData.data[i] = 0;
	
	return;
}