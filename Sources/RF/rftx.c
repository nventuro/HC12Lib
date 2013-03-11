#include "rftx.h"
#include "timers.h"

#define RFTX_DATA GLUE(PTT_PTT,RFTX_DATA_TIMER)
#define RFTX_DATA_DDR GLUE(DDRT_DDRT,RFTX_DATA_TIMER)


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
} rftx_data;

bool rftx_isInit = _FALSE;


void rftx_Init (bool ecc)
{
	if (rftx_isInit == _TRUE)
		return;	

	rftx_data.status = IDLE;
	
	tim_Init();	
	
	tim_GetTimer (TIM_OC, rftx_TimerCallback, NULL, RFTX_DATA_TIMER);

	tim_DisconnectOutput(RFTX_DATA_TIMER);
	RFTX_DATA_DDR = DDR_OUT;
	RFTX_DATA = 1;
	
	return;
}

bool rftx_Send(u8 id, void *data, u8 length, rftx_ptr eot)
{

}
