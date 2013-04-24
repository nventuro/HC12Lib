#ifndef _DEBUG_H_INCLUDED_
#define _DEBUG_H_INCLUDED_


// General keys

//#define IIC_DEBUG
#define DMU_DEBUG


// Particular keys for IIC
#ifdef IIC_DEBUG

#define IIC_DEBUG_EOT

#endif	// IIC_DEBUG

#ifdef DMU_DEBUG

//#define FIFO_DEBUG_COUNT
#define DMU_DEBUG_OFFSET
//#define FIFO_DEBUG_PRINT_AVG_SAMPLES

#endif	// FIFO_DEBUG



#endif	//_DEBUG_H_INCLUDED_