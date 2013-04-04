#include "rfrx.h"
#include "rftx.h"
#include "hamming1511.h"
#include "timers.h"
#include "error.h"

#define RFRX_ID_AMOUNT 8

#define TIME_LARGE_MARGIN_US 80
#define TIME_SMALL_MARGIN_US 40

typedef enum
{
	DESYNCHED,
	SYNCHING,
	SYNCHED,
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
} rfrx_data;

bool rfrx_isInit = _FALSE;

void rfrx_TimerCallback(void);

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
	
	if (id > 7)
		err_Throw("rfrx: attempted to register an invalid id.\n");
	
	if (rfrx_data.idData[id].eot != NULL)
		err_Throw("rfrx: attempted to register an already registered id.\n");
	
	rfrx_data.idData[id].eot = eot;
	rfrx_data.idData[id].data = data;
	
	return;
}

void rfrx_Delete(u8 id)
{
	if (id > 7)
		err_Throw("rfrx: attempted to delete an invalid id.\n");
	
	if (rfrx_data.idData[id].eot == NULL)
		err_Throw("rfrx: attempted to delete an already deleted id.\n");
	
	rfrx_data.idData[id].eot = NULL;
	rfrx_data.idData[id].data = NULL;
	
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
		u16 timeElapsed = TIM_TICKS_TO_US(tim_GetValue(RFRX_DATA_TIMER) - rfrx_data.syncLastEdge);
		
		if (timeElapsed > (RFTX_DEAD_TIME_US - TIME_LARGE_MARGIN_US))
		{
			rfrx_data.syncLastEdge = tim_GetValue(RFRX_DATA_TIMER);
			rfrx_data.status = SYNCHED;
		}
		else
			rfrx_data.status = DESYNCHED;
		
		tim_SetRisingEdge(RFRX_DATA_TIMER);
		
		return;
	}
	else if (rfrx_data.status == SYNCHING) // Received rising edge
	{
		u16 timeElapsed = TIM_TICKS_TO_US(tim_GetValue(RFRX_DATA_TIMER) - rfrx_data.syncLastEdge);
		
		if ((timeElapsed > (RFTX_START_TX_TIME_US - TIME_SMALL_MARGIN_US)) && (timeElapsed < (RFTX_START_TX_TIME_US + TIME_SMALL_MARGIN_US)))
		{
			rfrx_data.status = RECEIVING;
			/*commence rx*/
		}
		else
		{
			rfrx_data.syncLastEdge = tim_GetValue(RFRX_DATA_TIMER);
			tim_SetFallingEdge(RFRX_DATA_TIMER);
			rfrx_data.status = SYNCHING;
		}
		
		
		return;
	}
	/*if receiving*/
}