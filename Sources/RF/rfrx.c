#include "rfrx.h"
#include "hamming1511.h"
#include "timers.h"
#include "error.h"

#define RFRX_DATA GLUE(PTT_PTT,RFRX_DATA_TIMER)

#define RFRX_ID_AMOUNT 8

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
	
	tim_Init();	
	tim_GetTimer (TIM_IC, rfrx_TimerCallback, NULL, RFRX_DATA_TIMER);
	
	rfrx_data.status = DESYNCHED;
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
	/*
	if desynched (received rising)
		store time
		go to synching
		set falling
	if synching (received falling)
		check pulse width
		if less than dead time
			go to desynched
		else
			store time
			go to synched
		set rising
	if synched (received rising)
		check width
		if more than start time
			go to desynched
			set rising
		else
			go to receiving (set rising)
	if receiving
		rx logic
	*/
}