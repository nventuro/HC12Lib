#include "grfrx.h"
#include "grftx.h"
#include "hamming1511.h"
#include "timers.h"
#include "error.h"

#define GRFRX_ID_AMOUNT 8

#define GRFRX_TIME_SMALL_MARGIN_US 40
#define GRFRX_TIME_LARGE_MARGIN_US 80

#define GRFRX_TIME_SMALL_MARGIN_TICKS TIM_US_TO_TICKS(GRFRX_TIME_SMALL_MARGIN_US)
#define GRFRX_TIME_LARGE_MARGIN_TICKS TIM_US_TO_TICKS(GRFRX_TIME_LARGE_MARGIN_US)


typedef enum
{
	DESYNCHED,
	SYNCHING,
	SYNCHED,
	COMMENCING_RX,
	RECEIVING,
} GRFRX_STATUS;

struct grfrx_idData
{
	u8 *data;
	grfrx_ptr eot;
};

typedef struct grfrx_idData grfrx_idData;

struct
{
	GRFRX_STATUS status;
	grfrx_idData idData[GRFRX_ID_AMOUNT];
	u32 syncLastEdge;
	
	struct {
		u8 id;
		grfrx_ptr eot;
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
} grfrx_data;

bool grfrx_isInit = _FALSE;

void grfrx_TimerCallback(void);
void grfrx_CommenceReception(void);
void grfrx_StoreReceivedData(void);

void grfrx_Init (void)
{
	u8 i;

	if (grfrx_isInit == _TRUE)
		return;	
	
	grfrx_isInit = _TRUE;
	
	for (i = 0; i < GRFRX_ID_AMOUNT; i++)
	{
		grfrx_data.idData[i].data = NULL;
		grfrx_data.idData[i].eot = NULL;
	}
		
	grfrx_data.status = DESYNCHED;
	
	tim_Init();	
	tim_GetTimer (TIM_IC, grfrx_TimerCallback, NULL, GRFRX_DATA_TIMER);
		
	tim_SetRisingEdge(GRFRX_DATA_TIMER);
	tim_ClearFlag(GRFRX_DATA_TIMER);
	tim_EnableInterrupts(GRFRX_DATA_TIMER);

	return;
}

void grfrx_Register(u8 id, grfrx_ptr eot, u8 *data)
{
	if ((eot == NULL) || (data == NULL))
		err_Throw("grfrx: received NULL callback or memory pointer.\n");
	
	if (id >= GRFRX_ID_AMOUNT)
		err_Throw("grfrx: attempted to register an invalid id.\n");
	
	if (grfrx_data.idData[id].eot != NULL)
		err_Throw("grfrx: attempted to register an already registered id.\n");
	
	grfrx_data.idData[id].eot = eot;
	grfrx_data.idData[id].data = data;
	
	return;
}

void grfrx_TimerCallback(void)
{
	if (grfrx_data.status == DESYNCHED) // Received rising edge
	{
		grfrx_data.syncLastEdge = tim_GetValue(GRFRX_DATA_TIMER);
		tim_SetFallingEdge(GRFRX_DATA_TIMER);
		grfrx_data.status = SYNCHING;
		
		return;
	}
	else if (grfrx_data.status == SYNCHING) // Received falling edge
	{
		u16 ticksElapsed = tim_GetValue(GRFRX_DATA_TIMER) - grfrx_data.syncLastEdge;

		if (ticksElapsed > (GRFTX_DEAD_TIME_TICKS - GRFRX_TIME_LARGE_MARGIN_TICKS))
		{
			grfrx_data.syncLastEdge = tim_GetValue(GRFRX_DATA_TIMER);
			grfrx_data.status = SYNCHED;
		}
		else
			grfrx_data.status = DESYNCHED;
		
		tim_SetRisingEdge(GRFRX_DATA_TIMER);
		
		return;
	}
	else if (grfrx_data.status == SYNCHED) // Received rising edge
	{
		u16 ticksElapsed = tim_GetValue(GRFRX_DATA_TIMER) - grfrx_data.syncLastEdge;

		if ((ticksElapsed > (GRFTX_START_TX_TIME_TICKS - GRFRX_TIME_SMALL_MARGIN_TICKS)) && (ticksElapsed < (GRFTX_START_TX_TIME_TICKS + GRFRX_TIME_SMALL_MARGIN_TICKS)))
		{
			grfrx_data.status = COMMENCING_RX;
			grfrx_data.currComm.currData = 0;
			grfrx_data.currComm.currDataIndex = 14;
			grfrx_data.currComm.firstEdge = tim_GetValue(GRFRX_DATA_TIMER);
			grfrx_data.currComm.waitingForSecondEdge = _TRUE;
			
			tim_SetFallingEdge(GRFRX_DATA_TIMER);
		}
		else
		{
			grfrx_data.syncLastEdge = tim_GetValue(GRFRX_DATA_TIMER);
			tim_SetFallingEdge(GRFRX_DATA_TIMER);
			grfrx_data.status = SYNCHING;
		}

		return;
	}
	else if ((grfrx_data.status == COMMENCING_RX) || (grfrx_data.status == RECEIVING)) // Received either rising or falling
	{
		if (grfrx_data.currComm.waitingForSecondEdge == _TRUE)
		{
			grfrx_data.currComm.secondEdge = tim_GetValue(GRFRX_DATA_TIMER);
			tim_SetRisingEdge(GRFRX_DATA_TIMER);
			grfrx_data.currComm.waitingForSecondEdge = _FALSE;
		}
		else
		{
			u16 highWidth, lowWidth;
			highWidth = grfrx_data.currComm.secondEdge - grfrx_data.currComm.firstEdge;
			lowWidth = tim_GetValue(GRFRX_DATA_TIMER) - grfrx_data.currComm.secondEdge;
			
			if (((highWidth + lowWidth) > (GRFTX_BIT_TIME_TICKS - GRFRX_TIME_SMALL_MARGIN_TICKS)) && (((highWidth + lowWidth) < (GRFTX_BIT_TIME_TICKS + GRFRX_TIME_SMALL_MARGIN_TICKS))))
			{
				if (highWidth < lowWidth)
					grfrx_data.currComm.currData = grfrx_data.currComm.currData | (1 << grfrx_data.currComm.currDataIndex);
				
				if (grfrx_data.currComm.currDataIndex == 0)
				{
					if (grfrx_data.status == COMMENCING_RX)
						grfrx_CommenceReception(); // Decodes the received command and prepares the module for data reception
					else
						grfrx_StoreReceivedData(); // Decodes (or doesn't, depending on the ecc bit) the received data and stores it the receiver's memory
				}
				else
				{
					grfrx_data.currComm.currDataIndex --;
					grfrx_data.currComm.firstEdge = tim_GetValue(GRFRX_DATA_TIMER);
					grfrx_data.currComm.waitingForSecondEdge = _TRUE;
					tim_SetFallingEdge(GRFRX_DATA_TIMER);
				}
			}
			else
			{
				grfrx_data.syncLastEdge = tim_GetValue(GRFRX_DATA_TIMER);
				tim_SetFallingEdge(GRFRX_DATA_TIMER);
				grfrx_data.status = SYNCHING;
			}
		}
		
		return;
	}
}

void grfrx_CommenceReception (void)
{	
	hamm_DecodeWord(&grfrx_data.currComm.currData);
	
	grfrx_data.currComm.id = (grfrx_data.currComm.currData & 0x380) >> 7;
	
	if (grfrx_data.idData[grfrx_data.currComm.id].eot == NULL) // If there's no callback for the received id, abort reception
	{
		grfrx_data.syncLastEdge = tim_GetValue(GRFRX_DATA_TIMER);
		tim_SetFallingEdge(GRFRX_DATA_TIMER);
		grfrx_data.status = SYNCHING;
	}
	else
	{
		grfrx_data.currComm.length = grfrx_data.currComm.currData & 0x7F;
		grfrx_data.currComm.ecc = BOOL(grfrx_data.currComm.currData & 0x400);
		grfrx_data.currComm.eot = grfrx_data.idData[grfrx_data.currComm.id].eot;
		grfrx_data.currComm.data = grfrx_data.idData[grfrx_data.currComm.id].data;
		
		grfrx_data.currComm.dataIndex = 0;
		
		grfrx_data.currComm.currData = 0;
		if (grfrx_data.currComm.ecc == _TRUE)
			grfrx_data.currComm.currDataIndex = 14;
		else
			grfrx_data.currComm.currDataIndex = 10;
		
		grfrx_data.currComm.firstEdge = tim_GetValue(GRFRX_DATA_TIMER);
		grfrx_data.currComm.waitingForSecondEdge = _TRUE;
		
		tim_SetFallingEdge(GRFRX_DATA_TIMER);
		
		grfrx_data.status = RECEIVING;		
	}	
}

u32 writeMask[] = {0x1FFFFF,0x800FFFFF,0xC007FFFF,0xE003FFFF,0xF001FFFF,0xF800FFFF,0xFC007FFF,0xFE003FFF};
u32 writeMaskFix[] = {0x7FE00000, 0x3FE00000, 0x1FE00000, 0xFE00000, 0x7E00000, 0x3E00000, 0x1E00000, 0xE00000, 0x600000, 0x200000};
u16 shortDataMask[] = {0x400, 0x600, 0x700, 0x780, 0x7C0, 0x7E0, 0x7F0, 0x7F8, 0x7FC, 0x7FE};

void grfrx_StoreReceivedData(void)
{
	u8 bitIndex;
	u8 firstByte;
	u8 bitsToWrite;
	u32 read;
	
	if (grfrx_data.currComm.ecc == _TRUE)
	{
		hamm_DecodeWord(&grfrx_data.currComm.currData);
		grfrx_data.currComm.currData = grfrx_data.currComm.currData & 0x7FF;
	}
	
	bitIndex = grfrx_data.currComm.dataIndex % 8;
	firstByte = grfrx_data.currComm.dataIndex / 8;
	bitsToWrite = grfrx_data.currComm.length - grfrx_data.currComm.dataIndex + 1;
	
	
	read = *((u32*)(grfrx_data.currComm.data + firstByte));
	if (bitsToWrite >= 11)
	{
		read = read & writeMask[bitIndex];
		read = read | (((u32)(grfrx_data.currComm.currData)) << (21 - bitIndex) );
	}
	else
	{
		read = read & (writeMask[bitIndex] | (writeMaskFix[bitsToWrite - 1] >> bitIndex));
		read = read | (((u32)(grfrx_data.currComm.currData & shortDataMask[bitsToWrite - 1])) << (21 - bitIndex) );
	}
	(*((u32*)(grfrx_data.currComm.data + firstByte))) = read;
	
	
	grfrx_data.currComm.dataIndex = grfrx_data.currComm.dataIndex + 11;	
	
	
	if (grfrx_data.currComm.dataIndex > grfrx_data.currComm.length)
	{
		grfrx_data.currComm.eot(grfrx_data.currComm.length);
		grfrx_data.syncLastEdge = tim_GetValue(GRFRX_DATA_TIMER);
		tim_SetFallingEdge(GRFRX_DATA_TIMER);
		grfrx_data.status = SYNCHING;
	}
	else
	{
		grfrx_data.currComm.currData = 0;
		if (grfrx_data.currComm.ecc == _TRUE)
			grfrx_data.currComm.currDataIndex = 14;
		else
			grfrx_data.currComm.currDataIndex = 10;
				
		grfrx_data.currComm.firstEdge = tim_GetValue(GRFRX_DATA_TIMER);
		grfrx_data.currComm.waitingForSecondEdge = _TRUE;
	}
	
	tim_SetFallingEdge(GRFRX_DATA_TIMER);
}