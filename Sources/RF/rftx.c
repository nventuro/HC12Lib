#include "rftx.h"
#include "hamming1511.h"
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

typedef enum
{
	IDLE,
	SENDING,
	WAITING_FOR_DEAD_TIME_TO_END,
} RFTX_STATUS;

struct rftx_commData
{
	u8 id;
	u8 *data;
	u8 length;
	rftx_ptr eot;
};

typedef struct rftx_commData rftx_commData;

struct
{
	bool ecc;
	RFTX_STATUS status;
	//queue
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
	
	//create queue
	//init queue
	
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
		rftx_data.status == SENDING;
		rftx_data.currComm.id = id & 0x07;
		rftx_data.currComm.data = data;
		rftx_data.currComm.length = length & 0x7F;
		rftx_data.currComm.eot = eot;
		
		rftx_data.currDataIndex = 16;
		rftx_data.dataIndex = 0;
		
		// poner en rftx_data.currData los primeros 11 bits, y hacer hamming si corresponde
		
		//init indexes, commence tranmission
	}
	else
	{
		// store in queue
		// if queue full, return false
	}
}

void rftx_TimerCallback(void)
{
	if (rftx_data.status == WAITING_FOR_DEAD_TIME_TO_END)
	{
		// check queue
		// if queue empty, do nothing, go to idle
		// if queue not empty, send, go to busy
	}
	else // SENDING
	{
		// if bit sent, send next
		// if bit half sent, send complementary halfbit
		// if tranmission done, go to waiting, call eot
	}
}

#define CB_EMPTY -2
#define CB_FULL -3

cbuf cb_create(u8 *mem, u16 len);

#define cb_status(cb)	((cb)->status)

s16 cb_push(cbuf* buffer, u8 data);

s16 cb_pop(cbuf* buffer);