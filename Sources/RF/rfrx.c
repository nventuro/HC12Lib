#include "rfrx.h"
#include "rftx.h"
#include "hamming1511.h"
#include "timers.h"
#include "error.h"

#define RFRX_ID_AMOUNT 8

#define RFRX_TIME_SMALL_MARGIN_US 40
#define RFRX_TIME_LARGE_MARGIN_US 80

#define RFRX_TIME_SMALL_MARGIN_TICKS TIM_US_TO_TICKS(RFRX_TIME_SMALL_MARGIN_US)
#define RFRX_TIME_LARGE_MARGIN_TICKS TIM_US_TO_TICKS(RFRX_TIME_LARGE_MARGIN_US)


typedef enum
{
	DESYNCHED,
	SYNCHING,
	SYNCHED,
	COMMENCING_RX,
	RECEIVING,
} RFRX_STATUS;

struct rfrx_idData
{
	u8 *data;
	rfrx_ptr eot;
};

typedef struct rfrx_idData rfrx_idData;

struct
{
	RFRX_STATUS status;
	rfrx_idData idData[RFRX_ID_AMOUNT];
	u32 syncLastEdge;
	
	struct {
		u8 id;
		rfrx_ptr eot;
		bool ecc;
		
		u8 *data;
		u8 length;
		u8 dataIndex;
		
		u16 currData;
		u8 currDataIndex;
		
		u16 firstEdge;
		u16 secondEdge;
		
		bool waitingForSecondEdge;
	} currComm;
} rfrx_data;

bool rfrx_isInit = _FALSE;

void rfrx_TimerCallback(void);
void rfrx_CommenceReception(void);
void rfrx_StoreReceivedData(void);

void rfrx_Init (void)
{
	u8 i;

	if (rfrx_isInit == _TRUE)
		return;	
	
	rfrx_isInit = _TRUE;
	
	for (i = 0; i < RFRX_ID_AMOUNT; i++)
	{
		rfrx_data.idData[i].data = NULL;
		rfrx_data.idData[i].eot = NULL;
	}
		
	rfrx_data.status = DESYNCHED;
	
	tim_Init();	
	tim_GetTimer (TIM_IC, rfrx_TimerCallback, NULL, RFRX_DATA_TIMER);
		
	tim_SetRisingEdge(RFRX_DATA_TIMER);
	tim_ClearFlag(RFRX_DATA_TIMER);
	tim_EnableInterrupts(RFRX_DATA_TIMER);

	return;
}

void rfrx_Register(u8 id, rfrx_ptr eot, u8 *data)
{
	if ((eot == NULL) || (data == NULL))
		err_Throw("rfrx: received NULL callback or memory pointer.\n");
	
	if (id >= RFRX_ID_AMOUNT)
		err_Throw("rfrx: attempted to register an invalid id.\n");
	
	if (rfrx_data.idData[id].eot != NULL)
		err_Throw("rfrx: attempted to register an already registered id.\n");
	
	rfrx_data.idData[id].eot = eot;
	rfrx_data.idData[id].data = data;
	
	return;
}

void rfrx_TimerCallback(void)
{
	if (rfrx_data.status == DESYNCHED) // Received rising edge
	{
		rfrx_data.syncLastEdge = tim_GetValue(RFRX_DATA_TIMER);
		tim_SetFallingEdge(RFRX_DATA_TIMER);
		rfrx_data.status = SYNCHING;
		
		return;
	}
	else if (rfrx_data.status == SYNCHING) // Received falling edge
	{
		u16 ticksElapsed = tim_GetValue(RFRX_DATA_TIMER) - rfrx_data.syncLastEdge;

		if (ticksElapsed > (RFTX_DEAD_TIME_TICKS - RFRX_TIME_LARGE_MARGIN_TICKS))
		{
			rfrx_data.syncLastEdge = tim_GetValue(RFRX_DATA_TIMER);
			rfrx_data.status = SYNCHED;
		}
		else
			rfrx_data.status = DESYNCHED;
		
		tim_SetRisingEdge(RFRX_DATA_TIMER);
		
		return;
	}
	else if (rfrx_data.status == SYNCHED) // Received rising edge
	{
		u16 ticksElapsed = tim_GetValue(RFRX_DATA_TIMER) - rfrx_data.syncLastEdge;

		if ((ticksElapsed > (RFTX_START_TX_TIME_TICKS - RFRX_TIME_SMALL_MARGIN_TICKS)) && (ticksElapsed < (RFTX_START_TX_TIME_TICKS + RFRX_TIME_SMALL_MARGIN_TICKS)))
		{
			rfrx_data.status = COMMENCING_RX;
			rfrx_data.currComm.currData = 0;
			rfrx_data.currComm.currDataIndex = 14;
			rfrx_data.currComm.firstEdge = tim_GetValue(RFRX_DATA_TIMER);
			rfrx_data.currComm.waitingForSecondEdge = _TRUE;
			
			tim_SetFallingEdge(RFRX_DATA_TIMER);
		}
		else
		{
			rfrx_data.syncLastEdge = tim_GetValue(RFRX_DATA_TIMER);
			tim_SetFallingEdge(RFRX_DATA_TIMER);
			rfrx_data.status = SYNCHING;
		}

		return;
	}
	else if ((rfrx_data.status == COMMENCING_RX) || (rfrx_data.status == RECEIVING)) // Received either rising or falling
	{
		if (rfrx_data.currComm.waitingForSecondEdge == _TRUE)
		{
			rfrx_data.currComm.secondEdge = tim_GetValue(RFRX_DATA_TIMER);
			tim_SetRisingEdge(RFRX_DATA_TIMER);
			rfrx_data.currComm.waitingForSecondEdge = _FALSE;
		}
		else
		{
			u16 highWidth, lowWidth;
			highWidth = rfrx_data.currComm.secondEdge - rfrx_data.currComm.firstEdge;
			lowWidth = tim_GetValue(RFRX_DATA_TIMER) - rfrx_data.currComm.secondEdge;
			
			if (((highWidth + lowWidth) > (RFTX_BIT_TIME_TICKS - RFRX_TIME_SMALL_MARGIN_TICKS)) && (((highWidth + lowWidth) < (RFTX_BIT_TIME_TICKS + RFRX_TIME_SMALL_MARGIN_TICKS))))
			{
				if (highWidth < lowWidth)
					rfrx_data.currComm.currData = rfrx_data.currComm.currData | (1 << rfrx_data.currComm.currDataIndex);
				
				if (rfrx_data.currComm.currDataIndex == 0)
				{
					if (rfrx_data.status == COMMENCING_RX)
						rfrx_CommenceReception(); // Decodes the received command and prepares the module for data reception
					else
						rfrx_StoreReceivedData(); // Decodes (or doesn't, depending on the ecc bit) the received data and stores it the receiver's memory
				}
				else
				{
					rfrx_data.currComm.currDataIndex --;
					rfrx_data.currComm.firstEdge = tim_GetValue(RFRX_DATA_TIMER);
					rfrx_data.currComm.waitingForSecondEdge = _TRUE;
					tim_SetFallingEdge(RFRX_DATA_TIMER);
				}
			}
			else
			{
				rfrx_data.syncLastEdge = tim_GetValue(RFRX_DATA_TIMER);
				tim_SetFallingEdge(RFRX_DATA_TIMER);
				rfrx_data.status = SYNCHING;
			}
		}
		
		return;
	}
}

void rfrx_CommenceReception (void)
{	
	hamm_DecodeWord(&rfrx_data.currComm.currData);
	
	rfrx_data.currComm.id = (rfrx_data.currComm.currData & 0x380) >> 7;
	
	if (rfrx_data.idData[rfrx_data.currComm.id].eot == NULL) // If there's no callback for the received id, abort reception
	{
		rfrx_data.syncLastEdge = tim_GetValue(RFRX_DATA_TIMER);
		tim_SetFallingEdge(RFRX_DATA_TIMER);
		rfrx_data.status = SYNCHING;
	}
	else
	{
		rfrx_data.currComm.length = rfrx_data.currComm.currData & 0x7F;
		rfrx_data.currComm.ecc = BOOL(rfrx_data.currComm.currData & 0x400);
		rfrx_data.currComm.eot = rfrx_data.idData[rfrx_data.currComm.id].eot;
		rfrx_data.currComm.data = rfrx_data.idData[rfrx_data.currComm.id].data;
		
		rfrx_data.currComm.dataIndex = 0;
		
		rfrx_data.currComm.currData = 0;
		if (rfrx_data.currComm.ecc == _TRUE)
			rfrx_data.currComm.currDataIndex = 14;
		else
			rfrx_data.currComm.currDataIndex = 10;
		
		rfrx_data.currComm.firstEdge = tim_GetValue(RFRX_DATA_TIMER);
		rfrx_data.currComm.waitingForSecondEdge = _TRUE;
		
		tim_SetFallingEdge(RFRX_DATA_TIMER);
		
		rfrx_data.status = RECEIVING;		
	}	
}

u32 writeMask[] = {0x1FFFFF,0x800FFFFF,0xC007FFFF,0xE003FFFF,0xF001FFFF,0xF800FFFF,0xFC007FFF,0xFE003FFF};
u32 writeMaskFix[] = {0x7FE00000, 0x3FE00000, 0x1FE00000, 0xFE00000, 0x7E00000, 0x3E00000, 0x1E00000, 0xE00000, 0x600000, 0x200000};
u16 shortDataMask[] = {0x400, 0x600, 0x700, 0x780, 0x7C0, 0x7E0, 0x7F0, 0x7F8, 0x7FC, 0x7FE};

void rfrx_StoreReceivedData(void)
{
	u8 bitIndex;
	u8 firstByte;
	u8 bitsToWrite;
	u32 read;
	
	if (rfrx_data.currComm.ecc == _TRUE)
	{
		hamm_DecodeWord(&rfrx_data.currComm.currData);
		rfrx_data.currComm.currData = rfrx_data.currComm.currData & 0x7FF;
	}
	
	bitIndex = rfrx_data.currComm.dataIndex % 8;
	firstByte = rfrx_data.currComm.dataIndex / 8;
	bitsToWrite = rfrx_data.currComm.length - rfrx_data.currComm.dataIndex + 1;
	
	
	read = *((u32*)(rfrx_data.currComm.data + firstByte));
	if (bitsToWrite >= 11)
	{
		read = read & writeMask[bitIndex];
		read = read | (((u32)(rfrx_data.currComm.currData)) << (21 - bitIndex) );
	}
	else
	{
		read = read & (writeMask[bitIndex] | (writeMaskFix[bitsToWrite - 1] >> bitIndex));
		read = read | (((u32)(rfrx_data.currComm.currData & shortDataMask[bitsToWrite - 1])) << (21 - bitIndex) );
	}
	(*((u32*)(rfrx_data.currComm.data + firstByte))) = read;
	
	
	rfrx_data.currComm.dataIndex = rfrx_data.currComm.dataIndex + 11;	
	
	
	if (rfrx_data.currComm.dataIndex > rfrx_data.currComm.length)
	{
		rfrx_data.currComm.eot(rfrx_data.currComm.length);
		rfrx_data.syncLastEdge = tim_GetValue(RFRX_DATA_TIMER);
		tim_SetFallingEdge(RFRX_DATA_TIMER);
		rfrx_data.status = SYNCHING;
	}
	else
	{
		rfrx_data.currComm.currData = 0;
		if (rfrx_data.currComm.ecc == _TRUE)
			rfrx_data.currComm.currDataIndex = 14;
		else
			rfrx_data.currComm.currDataIndex = 10;
				
		rfrx_data.currComm.firstEdge = tim_GetValue(RFRX_DATA_TIMER);
		rfrx_data.currComm.waitingForSecondEdge = _TRUE;
	}
	
	tim_SetFallingEdge(RFRX_DATA_TIMER);
}