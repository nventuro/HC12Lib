#define _RTC_PRIVILEGED
#include "rtc.h"
#include "iic.h"
#include "timers.h"

#define RTC_ADDRESS 0x68
#define RTC_DATA_SIZE (8)
#define RTC_CONTROL_CONFIG(a) (a = 0b00010000)	// 1Hz, output enabled

#define BCD_DATE 0x0F
#define BCD_UNI 0x0F
#define BCD_SEG_DECA 0x70
#define BCD_MIN_DECA 0x70
#define BCD_HOUR_DECA 0x30
#define BCD_DATE_DECA 0x30
#define BCD_DAY_DECA 0x30
#define BCD_MONTH_DECA 0x10
#define BCD_YEAR_DECA 0xF0

#define BCD_UNI_SHIFT 0
#define BCD_DECA_SHIFT 4

#define BCD_HOUR_FORMAT 0x40
#define BCD_HOUR_FORMAT_SHIFT 6

#define RTC_24_HOUR_FORMAT 0
#define RTC_HOUR_FORMAT_SHIFT 6
#define RTC_SQWE_ENABLE 1
#define RTC_SQWE_SHIFT 4
#define RTC_RS0_1HZ 0
#define RTC_RS0_SHIFT 0
#define RTC_RS1_1HZ 0
#define RTC_RS1_SHIFT 1

#define RTC_SEC_REG 0x00
#define RTC_MIN_REG 0x01
#define RTC_HOUR_REG 0x02 
#define RTC_DAY_REG 0x03
#define RTC_DATE_REG 0x04 
#define RTC_MONTH_REG 0x05 
#define RTC_YEAR_REG 0x06 
#define RTC_CONTROL_REG 0x07

rtc_data_T rtc_data;

struct
{
	bool init;
	u8 startUpStage;
	s8 timId;
	rtc_ptr extCB;
} rtc_intData = {_FALSE, 0, 0, NULL};


void rtc_startUp (void);
void rtc_setRegAdd (u8 reg, rtc_ptr cb);
void rtc_storeReceivedData (void);
void rtc_sendLocalDataToDevice(rtc_ptr cb);
void rtc_intSrv (void);
void rtc_intAux (void);


void rtc_init (void)
{
	if (rtc_intData.init == _FALSE)
	{
		iic_Init();

		tim_Init();
		rtc_intData.timId = RTC_TIMER;
		tim_GetTimer(TIM_IC,rtc_intSrv,NULL,RTC_TIMER);
		tim_dSetRisingEdge(rtc_intData.timId);
		rtc_intData.extCB = NULL;
		
		rtc_intData.startUpStage = 0;
		rtc_startUp();
		
		while (rtc_intData.init != _TRUE)
			;
	}
	
	return;
}

void rtc_startUp (void)
{
	switch (rtc_intData.startUpStage)
	{
	case 0:
		// Preparo la lectura
		rtc_intData.startUpStage++;
		rtc_setRegAdd(RTC_SEC_REG,rtc_startUp);

		break;

	case 1:
		// Leo los 7 registros que contienen informacion de hora
		rtc_intData.startUpStage++;
		iic_Receive(RTC_ADDRESS, rtc_startUp, NULL, 7, NULL);

		break;

	case 2:
		// Guardo la informacion recibida, y configuro las settings (sin cambiar el resto de la data)
		rtc_intData.startUpStage++;
		rtc_storeReceivedData();
		rtc_sendLocalDataToDevice(rtc_startUp);

		break;

	case 3:
		// Recien ahora esta inicializado al dispositivo, se habilitan las interrupciones por el clock del RTC
		rtc_enableAutoUpdate();			
		
		rtc_intData.init = _TRUE;
		
		break;
	}
	
	return;
}

void rtc_assignAutoUpdateCallback (rtc_ptr rtc_cb)
{
	rtc_intData.extCB = rtc_cb;
	
	return;
}


void rtc_setRegAdd (u8 reg, rtc_ptr cb)
{	
	if (iic_IsBusy()) // dispatcher
		return;
	
	iic_commData.data[0] = reg;

	iic_Send(RTC_ADDRESS,cb,NULL, 1, NULL);
	
	return;
}


void rtc_storeReceivedData (void)
{
	rtc_data.seconds.uni = (iic_commData.data[RTC_SEC_REG] & BCD_UNI) >> BCD_UNI_SHIFT;
	rtc_data.seconds.deca = (iic_commData.data[RTC_SEC_REG] & BCD_SEG_DECA) >> BCD_DECA_SHIFT;
	
	rtc_data.minutes.uni = (iic_commData.data[RTC_MIN_REG] & BCD_UNI) >> BCD_UNI_SHIFT;
	rtc_data.minutes.deca = (iic_commData.data[RTC_MIN_REG] & BCD_MIN_DECA) >> BCD_DECA_SHIFT;
	
	rtc_data.hours.uni = (iic_commData.data[RTC_HOUR_REG] & BCD_UNI) >> BCD_UNI_SHIFT;
	rtc_data.hours.deca = (iic_commData.data[RTC_HOUR_REG] & BCD_HOUR_DECA) >> BCD_DECA_SHIFT; //Se asume formato 24 horas
	
	rtc_data.date.uni = (iic_commData.data[RTC_DATE_REG] & BCD_UNI) >> BCD_UNI_SHIFT;
	rtc_data.date.deca = (iic_commData.data[RTC_DATE_REG] & BCD_DATE_DECA) >> BCD_DECA_SHIFT;
	
	rtc_data.day = (iic_commData.data[RTC_DAY_REG] & BCD_DAY_DECA) >> BCD_DECA_SHIFT;
	
	rtc_data.month.uni = (iic_commData.data[RTC_MONTH_REG] & BCD_UNI) >> BCD_UNI_SHIFT;
	rtc_data.month.deca = (iic_commData.data[RTC_MONTH_REG] & BCD_MONTH_DECA) >> BCD_DECA_SHIFT;
	
	rtc_data.year.uni = (iic_commData.data[RTC_YEAR_REG] & BCD_UNI) >> BCD_UNI_SHIFT;
	rtc_data.year.deca = (iic_commData.data[RTC_YEAR_REG] & BCD_YEAR_DECA) >> BCD_DECA_SHIFT;
	
	if (rtc_intData.extCB != NULL)
		(*rtc_intData.extCB)();
	
	return;   
}


void rtc_setTime(decimal sec, decimal min, decimal h, decimal date, 
									decimal month, decimal year, rtc_day d)
{
	bool intEn = rtc_isAutoUpdateEnabled ();
	
	rtc_disableAutoUpdate();

	if (sec.deca <= 5 && sec.uni <= 9)
		rtc_data.seconds = sec;
	
	if (min.deca <= 5 && min.uni <=9)
    	rtc_data.minutes = min;
	
	if (decimal2u8(h) <= 24)
		rtc_data.hours = h;
	
	if (decimal2u8(date) <=31)
    	rtc_data.date = date;
	
    if (decimal2u8(month) <= 12)
    	rtc_data.month = month;
    
    if (decimal2u8(year) <= 99)
    	rtc_data.year = year;
    
    rtc_data.day = d;
   
    
    if (intEn)
    	rtc_sendLocalDataToDevice(rtc_enableAutoUpdate);
    else
    	rtc_sendLocalDataToDevice(NULL);
    
    if (rtc_intData.extCB != NULL)
		(*rtc_intData.extCB)();
    
    return;
}


void rtc_sendLocalDataToDevice (rtc_ptr cb)
{
	// Registro inicial de escritura
	iic_commData.data[0] = RTC_SEC_REG;
	
	iic_commData.data[1] = (rtc_data.seconds.deca << BCD_DECA_SHIFT) + (rtc_data.seconds.uni << BCD_UNI_SHIFT);
	iic_commData.data[2] = (rtc_data.minutes.deca << BCD_DECA_SHIFT) + (rtc_data.minutes.uni << BCD_UNI_SHIFT);
	iic_commData.data[3] = (rtc_data.hours.deca << BCD_DECA_SHIFT) + (rtc_data.hours.uni << BCD_UNI_SHIFT)
										+ (RTC_24_HOUR_FORMAT << RTC_HOUR_FORMAT_SHIFT);
	iic_commData.data[4] = (rtc_data.date.deca << BCD_DECA_SHIFT) + (rtc_data.date.uni << BCD_UNI_SHIFT);
	iic_commData.data[5] = (rtc_data.day << BCD_UNI_SHIFT);
	iic_commData.data[6] = (rtc_data.month.deca << BCD_DECA_SHIFT) + (rtc_data.month.uni << BCD_UNI_SHIFT);
	iic_commData.data[7] = (rtc_data.year.deca << BCD_DECA_SHIFT) + (rtc_data.year.uni << BCD_UNI_SHIFT);
	iic_commData.data[8] = (RTC_SQWE_ENABLE << RTC_SQWE_SHIFT) + (RTC_RS0_1HZ << RTC_RS0_SHIFT) + (RTC_RS1_1HZ << RTC_RS1_SHIFT);
	
	iic_Send(RTC_ADDRESS,cb,NULL, 9, NULL);
	
	return;
}

bool rtc_isAutoUpdateEnabled (void)
{
	return  tim_dAreInterruptsEnabled(rtc_intData.timId);
}


void rtc_enableAutoUpdate (void)
{
	tim_dEnableInterrupts(rtc_intData.timId);

	return;
}


void rtc_disableAutoUpdate (void)
{
	tim_dDisableInterrupts(rtc_intData.timId);

	return;
}


void rtc_intSrv (void)
{
	rtc_setRegAdd(RTC_SEC_REG,rtc_intAux);
	
	return;
}


void rtc_intAux (void)
{
	iic_Receive(RTC_ADDRESS,rtc_storeReceivedData,NULL,7,NULL);
	
	return;
}


u8 decimal2u8(decimal d)
{
	return d.deca*10+d.uni;
}