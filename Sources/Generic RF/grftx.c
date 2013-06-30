#include "grftx.h"
#include "hamming1511.h"
#include "cb.h"
#include "timers.h"
#include "error.h"

#define GRFTX_DATA GLUE(PTT_PTT,GRFTX_DATA_TIMER)
#define GRFTX_DATA_DDR GLUE(DDRT_DDRT,GRFTX_DATA_TIMER)

#define GRFTX_QUEUE_SIZE 8

struct grftx_commData
{
	u8 id;
	u8 *data;
	u8 length;
	grftx_ptr eot;
};

typedef struct grftx_commData grftx_commData;

typedef cbuf rfqueue;

#define RFQUEUE_EMPTY CB_EMPTY
#define RFQUEUE_FULL CB_FULL

rfqueue rfqueue_Create(u8 *mem, u16 len);

#define rfqueue_Status(queue) cb_status(queue)

bool rfqueue_Push(rfqueue *queue, grftx_commData data);

grftx_commData rfqueue_Pop(rfqueue *queue);

u8 grftx_queueMemory[sizeof(grftx_commData)*GRFTX_QUEUE_SIZE];


typedef enum
{
	IDLE,
	SENDING,
	WAITING_FOR_DEAD_TIME_TO_END,
	WAITING_FOR_TICK_TO_END,
} GRFTX_STATUS;

struct
{
	bool ecc;
	GRFTX_STATUS status;
	rfqueue queue;
	
	grftx_commData currComm;
	u16 currData;
	s8 currDataIndex;
	u8 dataIndex;
	bool bitHalfSent;
} grftx_data;

bool grftx_isInit = _FALSE;

void grftx_TimerCallback(void);

void grftx_CommenceTX (void);

void grftx_FetchNewData (void);

void grftx_Init (bool ecc)
{
	if (grftx_isInit == _TRUE)
		return;	
	
	grftx_isInit = _TRUE;
	
	grftx_data.ecc = ecc;
	
	GRFTX_DATA_DDR = DDR_OUT;
		
	grftx_data.queue = rfqueue_Create(grftx_queueMemory,GRFTX_QUEUE_SIZE);
	
	tim_Init();	
	tim_GetTimer (TIM_OC, grftx_TimerCallback, NULL, GRFTX_DATA_TIMER);
	
	grftx_data.status = WAITING_FOR_DEAD_TIME_TO_END;
	tim_SetValue(GRFTX_DATA_TIMER, tim_GetGlobalValue() + TIM_US_TO_TICKS(GRFTX_DEAD_TIME_US));
	tim_ClearFlag(GRFTX_DATA_TIMER);
	tim_DisconnectOutput(GRFTX_DATA_TIMER);
	tim_EnableInterrupts(GRFTX_DATA_TIMER);
	
	GRFTX_DATA = 1;

	return;
}

void grftx_Send(u8 id, u8 *data, u8 length, grftx_ptr eot)
{
	if (data == NULL)
		err_Throw("grftx: data is NULL.\n");
	
	if (grftx_data.status == IDLE)
	{		
		grftx_data.status = SENDING;
		grftx_data.currComm.id = id & 0x07;
		grftx_data.currComm.data = data;
		grftx_data.currComm.length = length & 0x7F;
		grftx_data.currComm.eot = eot;
	
		grftx_CommenceTX();
		
		return;
	}
	else
	{
		if (rfqueue_Status(&grftx_data.queue) == RFQUEUE_FULL)
	    	err_Throw("grftx: attempted to send data, but queue is full.\n");
		else
		{
			grftx_commData requestedComm;
			requestedComm.id = id & 0x07;
			requestedComm.data = data;
			requestedComm.length = length & 0x7F;
			requestedComm.eot = eot;
			rfqueue_Push(&grftx_data.queue,requestedComm);
			
			return;
		}
	}
}

void grftx_CommenceTX (void)
{
	grftx_data.dataIndex = 0; //No data is being sent yet
	
	grftx_data.currData = (grftx_data.ecc << 10) | (grftx_data.currComm.id << 7) | (grftx_data.currComm.length); 
	hamm_GetParityBits(& grftx_data.currData);
	grftx_data.currDataIndex = 14; //Start of command
	grftx_data.bitHalfSent = _FALSE;
	GRFTX_DATA = 0;
	
	tim_SetValue(GRFTX_DATA_TIMER, tim_GetGlobalValue() + TIM_US_TO_TICKS(GRFTX_START_TX_TIME_US));
	
	return;
}

void grftx_TimerCallback(void)
{	
	if (grftx_data.status == WAITING_FOR_DEAD_TIME_TO_END)
	{
		if (rfqueue_Status(&grftx_data.queue) == RFQUEUE_EMPTY)
		{
			grftx_data.status = IDLE;
			tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_TICK_TIMEOUT_US));
		}
		else
		{
			grftx_commData newComm = rfqueue_Pop(&grftx_data.queue);
			grftx_data.status = SENDING;
			grftx_data.currComm.id = newComm.id & 0x07;
			grftx_data.currComm.data = newComm.data;
			grftx_data.currComm.length = newComm.length & 0x7F;
			grftx_data.currComm.eot = newComm.eot;
		
			grftx_CommenceTX();
		}
	}
	else if (grftx_data.status == IDLE)
	{
		grftx_data.status = WAITING_FOR_TICK_TO_END;
		GRFTX_DATA = 0;
		tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_TICK_DURATION_US));

	}
	else if (grftx_data.status == WAITING_FOR_TICK_TO_END)
	{
		grftx_data.status = WAITING_FOR_DEAD_TIME_TO_END;
		GRFTX_DATA = 1;
		tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_DEAD_TIME_US));
	}
	else // SENDING
	{
		if (grftx_data.bitHalfSent == _TRUE)
		{
			GRFTX_DATA = 0;
		
			if ((grftx_data.currData & BIT(grftx_data.currDataIndex)) == 0)
				tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_0_LOW_TIME_US));
			else
				tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_1_LOW_TIME_US));
			
			grftx_data.bitHalfSent = _FALSE;
			grftx_data.currDataIndex--;
			
		}
		else
		{
			if (grftx_data.currDataIndex >= 0) // If currData has not been fully transmitted
			{
				GRFTX_DATA = 1;
			
				if ((grftx_data.currData & BIT(grftx_data.currDataIndex)) == 0)
					tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_0_HIGH_TIME_US));
				else
					tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_1_HIGH_TIME_US));
				
				grftx_data.bitHalfSent = _TRUE;
				
			}
			else
			{
				if (grftx_data.dataIndex > grftx_data.currComm.length) // If all data has been transmitted
				{
					GRFTX_DATA = 1;
					
					grftx_data.status = WAITING_FOR_DEAD_TIME_TO_END;
					tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_DEAD_TIME_US));
					
					if (grftx_data.currComm.eot != NULL)
						grftx_data.currComm.eot();
				}
				else // Fetch new data
				{
					GRFTX_DATA = 1;

					grftx_FetchNewData();
					
					if ((grftx_data.currData & BIT(grftx_data.currDataIndex)) == 0)
						tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_0_HIGH_TIME_US));
					else
						tim_SetValue(GRFTX_DATA_TIMER, tim_GetValue(GRFTX_DATA_TIMER) + TIM_US_TO_TICKS(GRFTX_1_HIGH_TIME_US));

					grftx_data.bitHalfSent = _TRUE;
				}
			}
		}
	}
}

u32 readMask[] = {0xFFE00000,0x7FF00000,0x3FF80000,0x1FFC0000,0xFFE0000,0x7FF0000,0x3FF8000,0x1FFC000};

void grftx_FetchNewData (void)
{
	u8 bitIndex; 
	u8 firstByte;

	bitIndex = grftx_data.dataIndex % 8;
	firstByte = grftx_data.dataIndex / 8;

	grftx_data.currData = (u16) (((*((u32*)(grftx_data.currComm.data + firstByte))) & readMask[bitIndex]) >> (21 - bitIndex));

	grftx_data.dataIndex = grftx_data.dataIndex + 11;
	if (grftx_data.ecc)
	{
		hamm_GetParityBits(& grftx_data.currData);
		grftx_data.currDataIndex = 14; //Start of data + parity bits
	}
	else
		grftx_data.currDataIndex = 10; //Start of data	

	return;
}


rfqueue rfqueue_Create(u8 *mem, u16 len)
{
	return cb_create(mem, sizeof(grftx_commData)*len);
}

bool rfqueue_Push(rfqueue* queue, grftx_commData data)
{
	if (rfqueue_Status(queue) == RFQUEUE_FULL)
		return _FALSE;
	else
	{
		cb_push(queue, data.id);
		cb_push(queue, (u8) data.data);
		cb_push(queue, (u8) (((u16)data.data) >> 8));		
		cb_push(queue, (u8) data.eot);
		cb_push(queue, (u8) (((u16)data.eot) >> 8));		
		cb_push(queue, data.length);
		
		return _TRUE;
	}
}

grftx_commData rfqueue_Pop(rfqueue* queue)
{
	grftx_commData read;
	
	if (rfqueue_Status(queue) == RFQUEUE_EMPTY)
	{
		read.id = 0;
		read.data = NULL;
		read.eot = NULL;
		read.length = 0;
	}
	else
	{
		read.id = cb_pop(queue);
		read.data = (u8 *) cb_pop(queue);
		read.data = (u8 *) (((u16) read.data + (((u16)cb_pop(queue)) << 8))); 
		read.eot = (u8 *) cb_pop(queue);
		read.eot = (u8 *) (((u16) read.eot + (((u16)cb_pop(queue)) << 8))); 
		read.length = cb_pop(queue);
	}
	
	return read;
}