#ifndef _AD_H
#define _AD_H

#include "common.h"

#define ATD_AMOUNT 8
#define TASK_MAX 8
#define INVALID_TASK_ID (-1)	// if != (-1), chech getNextTask

#define ATD_CONV_BITS 10


typedef enum 
{
	ATD_CH0 = 0, 
	ATD_CH1, 
	ATD_CH2, 
	ATD_CH3, 
	ATD_CH4, 
	ATD_CH5, 
	ATD_CH6, 
	ATD_CH7
} atd_channel;


typedef void (*atd_ptr) (s16* mem, const struct atd_task* taskData);

typedef struct
{
	u8 channel;
	u8 length;
	bool mult;
	bool scan;
	
	atd_ptr eocCB;
} atd_task;


typedef enum 
{
	ATD0 = 0,
	ATD1
} atd_module;


typedef s8 atd_taskId;


void atd_Init(atd_module module);


/* Possible wrappers */

atd_taskId atd0_SingleTask(u8 channel, u8 length, bool mult, atd_ptr cb);	// scan = 0
atd_taskId atd0_GetChannel(u8 channel, u8 length, atd_ptr cb);	 // scan = 1, mult = 0
atd_taskId atd0_SingleConversion(u8 channel, u8 length, atd_ptr cb);		// scan = 0, mult = 0


#define ATD0_SET_DIVIDER(a) do {ATD0CTL4 &= 0xE0; ATD0CTL4 |= (a/2-1);} while(0)
#define ATD1_SET_DIVIDER(a) do {ATD1CTL4 &= 0xE0; ATD1CTL4 |= (a/2-1);} while(0)

enum {LENGTH_2=0, LENGTH_4, LENGTH_8, LENGTH_16};
#define ATD0_SAMPLE_TIME(a) do {ATD0CTL4 &= 0x9F; ATD0CTL4 |= (a<<5);} while(0)
#define ATD1_SAMPLE_TIME(a) do {ATD1CTL4 &= 0x9F; ATD1CTL4 |= (a<<5);} while(0)


// setTask transfers task ownership to user
atd_taskId atd_SetTask(atd_module module, u8 channel, u8 length, bool mult, bool scan, atd_ptr eocCB);
void atd_FreeTask(atd_module module, atd_taskId id);

// Possible interface

// atd_taskId
//atd_taskId atd_multiGetChannel(u8 module, u8 firstChannel, u8 lastChannel, atd_ptr cb);
//void atd_multiFreeChannel(u8 module, u8 firstChannel, u8 lastChannel);


extern void interrupt atd0_Service(void);
extern void interrupt atd1_Service(void);

#endif // _AD_H
