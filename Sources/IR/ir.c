#include "ir.h"
#include "cb.h"
#include "timers.h"

#define BUFF_LENGTH 50
#define CNT_MAX ((u32)65536)
#define HBT_TIME ((u32)8890)

#define HBT1_MAX ((u32)9336)
#define HBT1_MIN ((u32)8446)
#define HBT2_MAX ((u32)18668)
#define HBT2_MIN ((u32)16893)
#define HBT3_MAX ((u32)28001)
#define HBT3_MIN ((u32)25339)
#define HBT4_MAX ((u32)37335)
#define HBT4_MIN ((u32)33785)

#define RC5_TIMEOUT 47500
#define EDGE_TIME_MARGIN 100

#define PREVIOUS_BIT() ((icData.receivedData & (1<< ( (u8) ( icData.currentBit+1)))) ? 1 : 0) // +1: para volver al previous bit

#define SAME_INPUT_SKIP (2)

static struct
{
	u16 lastEdge;
	u16 receivedData;
	s8 currentBit;
	bool transmitting;
	u8 overflowCnt;
	bool icInhibit;
	bool init;

	struct
	{
		u8 lastByte;
		u8 count;
	}inputFlowControl;

} icData = {0,0, 13, _FALSE, 0,_FALSE, _FALSE, {0,0}};


static u8 irBuffer[BUFF_LENGTH];
static cbuf cBuffer;


void startTransmission(void);
void resetTransmission(void);
void endTransmission(void);

void store_0(void);
void store_1(void);

void ir_icSrv(void);
void ir_ocSrv(void);
void ir_ovfSrv(void);

void ir_init(void)
{

	if (icData.init == _TRUE)
		return;

	icData.init = _TRUE;

	tim_Init();

	tim_GetTimer(TIM_IC, ir_icSrv, ir_ovfSrv, IR_IC_TIMER);
	tim_GetTimer(TIM_OC, ir_ocSrv, NULL, IR_OC_TIMER);

	tim_EnableOvfInterrupts(IR_IC_TIMER);

	cBuffer = cb_create(irBuffer, BUFF_LENGTH);

	resetTransmission();

	return;
}


void resetTransmission(void)
{
	icData.transmitting = _FALSE;

	//Las interrupciones por OC solo están habilitadas durante una transmision
	icData.icInhibit = _FALSE;
	tim_EnableInterrupts(IR_IC_TIMER);
	tim_SetFallingEdge(IR_IC_TIMER);

	tim_DisableInterrupts(IR_OC_TIMER);
}


void startTransmission(void)
{
	icData.transmitting = _TRUE;

	tim_SetRisingEdge(IR_IC_TIMER);

	icData.currentBit = 14;
	icData.receivedData = 0;
	store_1();

	icData.lastEdge = tim_GetValue(IR_IC_TIMER) - HBT_TIME;	// No pasa nada aunque HBT_TIME > TC1

	if (((s32) (tim_GetValue(IR_IC_TIMER) - (s32)(HBT_TIME) )) >= 0)
		icData.overflowCnt = 0;
	else
		icData.overflowCnt = 1;

	tim_EnableInterrupts(IR_OC_TIMER);
}


void endTransmission(void)
{
	u8 data = icData.receivedData & (0x003F);
	data |= (((icData.receivedData & (1<<12)) ? 0 : 1)<<6);
	data |= (((icData.receivedData & (1<<11)) ? 1 : 0)<<7);

	if (icData.inputFlowControl.lastByte != data || icData.inputFlowControl.count == 0)
	{
		ir_push(data);

		icData.inputFlowControl.lastByte = data;
		icData.inputFlowControl.count = 1;
	}
	else if (!isDigit(data))		// Si es un número nunca lo guardo más de una vez
	{								// si hay un toggle entre dos números iguales entra al primer if.
		if ((icData.inputFlowControl.count %= SAME_INPUT_SKIP) == 0)	// Si el dato es igual y estoy en múltiplo de SKIP
			ir_push(data);												// guardo y subo. Nunca vuelvo a estar en 0.

		icData.inputFlowControl.count++;
	}

	resetTransmission();

	return;
}


void ir_icSrv(void)
{
	icData.icInhibit = _TRUE;
	tim_DisableInterrupts(IR_IC_TIMER);

	tim_ClearFlag(IR_OC_TIMER);
	tim_SetValue(IR_OC_TIMER, tim_GetValue(IR_IC_TIMER) + EDGE_TIME_MARGIN); //Margen por rise time lento

	if (icData.transmitting == _FALSE)
		startTransmission();
	else
	{
		u32 timeElapsed = (icData.overflowCnt * CNT_MAX + tim_GetValue(IR_IC_TIMER)) - icData.lastEdge;

		icData.lastEdge = tim_GetValue(IR_IC_TIMER);
		icData.overflowCnt = 0;

		if ((timeElapsed >= HBT2_MIN) && (timeElapsed < HBT2_MAX))
		{
			if (PREVIOUS_BIT() == 1)
				store_1();
			else
				store_0();
		}
		else if ((timeElapsed >= HBT3_MIN) && (timeElapsed < HBT3_MAX))
		{
			if (PREVIOUS_BIT() == 0)
				store_1();
			else
			{
				store_1();
				store_0();
			}
		}
		else if ((timeElapsed >= HBT4_MIN) && (timeElapsed < HBT4_MAX) && (PREVIOUS_BIT() == 0))
		{
			store_1();
			store_0();
		}
		else
		    resetTransmission();
	}

	if (icData.currentBit == (-1))
		endTransmission();

	return;
}


void ir_ocSrv(void)
{
    if (icData.icInhibit == _TRUE)
	{
	    icData.icInhibit = _FALSE;
	    tim_ClearFlag(IR_IC_TIMER);
	    tim_EnableInterrupts(IR_IC_TIMER);
	    tim_SetValue(IR_OC_TIMER, (icData.lastEdge + RC5_TIMEOUT) - EDGE_TIME_MARGIN);
    }
    else
    	resetTransmission();

    return;
}


void ir_ovfSrv(void)
{
	icData.overflowCnt++;

	return;
}


void store_1(void)
{
	icData.receivedData |= (1 << ((u8) (icData.currentBit)));
	icData.currentBit--;
}


void store_0(void)
{
	icData.currentBit--;
}


s16 ir_push(u8 data)
{
	return cb_push(&cBuffer, data);
}


s16 ir_pop(void)
{
	return cb_pop(&cBuffer);
}


s16 ir_flush(void)
{
	return cb_flush(&cBuffer);
}

bool isDigit(u8 _byte)
{
	_byte &= ~(1<<7);
	if (_byte <= 9)
		return _TRUE;
	else
		return _FALSE;
}
