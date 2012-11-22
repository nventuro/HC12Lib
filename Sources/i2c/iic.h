#ifndef _IIC_H
#define _IIC_H

#include "common.h"

#define IIC_MEM_SIZE 256

typedef void (*iic_ptr)(void);

typedef struct {
    u8 data[IIC_MEM_SIZE];
    u8 dataSize;
} iic_commData_T;

extern iic_commData_T iic_commData;

void iic_init (void);
bool iic_send (u8 slvAddress, iic_ptr eotCB, iic_ptr commFailedCB);
bool iic_receive (u8 slvAddress, iic_ptr eotCB, iic_ptr commFailedCB, u8 toRead);
bool iic_isBusy(void);

void interrupt iic0_srv (void);

#endif
