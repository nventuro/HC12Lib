#include "rfrx.h"
#include "hamming1511.h"
#include "timers.h"
#include "error.h"

#define RFRX_DATA GLUE(PTT_PTT,RFRX_DATA_TIMER)

typedef enum
{
	DESYNCHED,
	SYNCHING,
} RFRX_STATUS;

struct
{
	bool a;
} rfrx_data;

bool rfrx_isInit = _FALSE;

void rfrx_TimerCallback(void);

void rfrx_Init (void)
{
	if (rfrx_isInit == _TRUE)
		return;	
	
	rfrx_isInit = _TRUE;
	
	tim_Init();	
	tim_GetTimer (TIM_IC, rfrx_TimerCallback, NULL, RFRX_DATA_TIMER);
	
	rfrx_data.status = DESYNCHED;
	tim_SetRisingEdge(RFRX_DATA_TIMER);
	tim_ClearFlag(RFRX_DATA_TIMER);
	tim_EnableInterrupts(RFRX_DATA_TIMER);

	return;
}

void rfrx_TimerCallback(void)
{
	
}