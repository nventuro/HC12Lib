#include "rftx.h"
#include "hamming1511.h"
#include "cb.h"
#include "timers.h"

#define RFTX_DATA GLUE(PTT_PTT,RFTX_DATA_TIMER)
#define RFTX_DATA_DDR GLUE(DDRT_DDRT,RFTX_DATA_TIMER)

//f = 3khz
#define RFTX_1_HIGH_TIME_US 130
#define RFTX_1_LOW_TIME_US 200
#define RFTX_0_HIGH_TIME_US 200
#define RFTX_0_LOW_TIME_US 130

#define RFTX_DEAD_TIME_US 400
#define RFTX_START_TIME_US 100

#define RFTX_QUEUE_SIZE 8


struct rftx_commData
{
	u8 id;
	u8 *data;
	u8 length;
	rftx_ptr eot;
};

typedef struct rftx_commData rftx_commData;

typedef cbuf rfqueue;

#define RFQUEUE_EMPTY CB_EMPTY
#define RFQUEUE_FULL CB_FULL

rfqueue rfqueue_Create(u8 *mem, u16 len);
#define rfqueue_Status(queue) cb_status(queue)
bool rfqueue_Push(rfqueue *queue, rftx_commData data);
rftx_commData rfqueue_Pop(rfqueue *queue);

u8 rftx_queueMemory[sizeof(rftx_commData)*RFTX_QUEUE_SIZE];


typedef enum
{
	IDLE,
	SENDING,
	WAITING_FOR_DEAD_TIME_TO_END,
} RFTX_STATUS;

struct
{
	bool ecc;
	RFTX_STATUS status;
	rfqueue queue;
	rftx_commData currComm;
	u16 currData;
	u8 currDataIndex;
	u8 dataIndex;
} rftx_data;

bool rftx_isInit = _FALSE;

void rftx_TimerCallback(void);

void rftx_Init (bool ecc)
{
	if (rftx_isInit == _TRUE)
		return;	
	
	rftx_isInit = _TRUE;
	
	RFTX_DATA_DDR = DDR_OUT;
	RFTX_DATA = 1;
	
	rftx_data.queue = rfqueue_Create(rftx_queueMemory,RFTX_QUEUE_SIZE);
	
	tim_Init();	
	tim_GetTimer (TIM_OC, rftx_TimerCallback, NULL, RFTX_DATA_TIMER);
	
	rftx_data.status = WAITING_FOR_DEAD_TIME_TO_END;
	tim_SetValue(RFTX_DATA_TIMER, tim_GetValue(RFTX_DATA_TIMER) + TIM_US_TO_TICKS(RFTX_DEAD_TIME_US));
	tim_ClearFlag(RFTX_DATA_TIMER);
	tim_DisconnectOutput(RFTX_DATA_TIMER);
	tim_EnableInterrupts(RFTX_DATA_TIMER);
	
	return;
}

bool rftx_Send(u8 id, u8 *data, u8 length, rftx_ptr eot)
{
	if (rftx_data.status == IDLE)
	{
		rftx_data.status = SENDING;
		rftx_data.currComm.id = id & 0x07;
		rftx_data.currComm.data = data;
		rftx_data.currComm.length = length & 0x7F;
		rftx_data.currComm.eot = eot;
	
		rftx_data.dataIndex = 0; //No data is being sent yet
		
		rftx_data.currData = BIT(10) & (rftx_data.currComm.id << 7) & (rftx_data.currComm.length); 
		hamm_GetParityBits(& rftx_data.currData);
		rftx_data.currDataIndex = 15; //Start of command
		
		RFTX_DATA = 0;
		tim_SetValue(RFTX_DATA_TIMER, tim_GetValue(RFTX_DATA_TIMER) + TIM_US_TO_TICKS(RFTX_START_TIME_US));
		
		return _TRUE;
	}
	else
	{
		if (rfqueue_Status(&rftx_data.queue) == RFQUEUE_FULL)
			return _FALSE;
		else
		{
			rftx_commData aux;
			aux.id = id;
			aux.data = data;
			aux.length = length;
			aux.eot = eot;
			rfqueue_Push(&rftx_data.queue,aux);
			
			return _TRUE;
		}
	}
}

void rftx_TimerCallback(void)
{
	if (rftx_data.status == WAITING_FOR_DEAD_TIME_TO_END)
	{
		if (rfqueue_Status(&rftx_data.queue) == RFQUEUE_EMPTY)
		{
			rftx_data.status = IDLE;
			//inhibit interrupts?
			return;
		}
		else
		{
			rftx_data.status = SENDING;
			// if queue not empty, send
		}
	}
	else // SENDING
	{
		// if bit sent, send next
		// if bit half sent, send complementary halfbit
		// if tranmission done, go to waiting, call eot
	}
}


rfqueue rfqueue_Create(u8 *mem, u16 len)
{
	return cb_create(mem, sizeof(rftx_commData)*len);
}

bool rfqueue_Push(rfqueue* queue, rftx_commData data)
{
	if (rfqueue_Status(queue) == RFQUEUE_FULL)
		return _FALSE;
	else
	{
		cb_push(queue, data.length);
		cb_push(queue, (u8) data.eot);
		cb_push(queue, (u8) (((u16)data.eot) >> 8));		
		cb_push(queue, (u8) data.data);
		cb_push(queue, (u8) (((u16)data.data) >> 8));		
		cb_push(queue, data.id);
		
		return _TRUE;
	}
}

rftx_commData rfqueue_Pop(rfqueue* queue)
{
	rftx_commData read;
	
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