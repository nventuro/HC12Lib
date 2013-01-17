#ifndef _IIC_H
#define _IIC_H

#include "common.h"

#define IIC_MEM_SIZE 256

typedef void (*iic_ptr)(void);

typedef struct {
    u8 data[IIC_MEM_SIZE];
    u8 dataSize;
    u8* dataPtr;
} iic_commData_T;

extern iic_commData_T iic_commData;

void iic_Init (void);
/*	Inits I2C module. Must be called at the beginning of the program.
	Baud rate is hard-coded in IIC_SET_BAUD() macro (see iic.c).
*/

bool iic_Send (u8 slaveAddress, iic_ptr eotCB, iic_ptr commFailedCB, u8 toWrite, u8* sendBuffer);
/*	Sends data to the device specified in slaveAddress, calling eotCB when finished or commFailedCB 
	if it fails for	any reason.
	@param 'toWrite' is the size of the package to send.
	If 'sendBuffer' is not NULL, data is sent from that point in memory up to 'toWrite' bytes.
	If 'sendBuffer' is NULL, all the data to send must be written in iic_commData.data.
	Note: slaveAddress is composed by 7 bits, aligned RIGHT (X as MSB).
	The first byte to send should be write address inside slave device.
*/


bool iic_Receive (u8 slaveAddress, iic_ptr eotCB, iic_ptr commFailedCB, u8 toRead, u8* receiveBuffer);
/*	Receives data from the device specified in slaveAddress, calling eotCB when finished or commFailedCB 
	if it fails for	any reason.
	Slave device has to know where to start reading before calling iic_receive, so proper register address 
	must be sent before receiving.
	
	The size of the package to receive is specified in 'toRead', and the result is stored in 
	'receiveBuffer' (the user must provide enough memory). If 'receiveBuffer' is NULL, the result is
	stored in iic_commData.data, and the package size must be less or equal to IIC_MEM_SIZE.
	Note: slaveAddress is composed by 7 bits, aligned RIGHT (X as MSB).
*/

bool iic_ReceiveFromRegister (u8 regAddress, u8 slaveAddress, iic_ptr eotCB, iic_ptr commFailedCB, u8 toRead, u8* receiveBuffer);
/*	Complete reception from slave device and specified register. Uses iic_Receive. 
	Note: slaveAddress is composed by 7 bits, aligned RIGHT (X as MSB).
*/

#define iic_IsBusy() (IIC0_IBSR_IBB == 1) ? (_TRUE) : (_FALSE)
/*	Checks the bus' status. */


void interrupt iic0_srv (void);

#endif
