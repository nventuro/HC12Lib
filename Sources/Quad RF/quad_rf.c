#include "quad_rf.h"
#include "quad_rf_disp.h"
#include "nRF24L01+.h"
#include "LCD.h"
#include "fjoy.h"

#define QRF_INVALID_BATT_LEVEL 127

void qrf_fjoyUpdateCallback (void);
void qrf_nrfCallback (bool success, u8 *payloadData, u8 length);

u8 qrf_fjoyNRFData[4];
u16 qrf_packetLossCount = 0;
u8 qrf_battALevel = QRF_INVALID_BATT_LEVEL;
u8 qrf_battBLevel = QRF_INVALID_BATT_LEVEL;

void qrf_Init (void)
{
	nrf_Init(PTX);
	lcd_Init(LCD_2004);
	fjoy_Init();
}

void qrf_fjoyUpdateCallback (void)
{
	qrf_fjoyNRFData[0] = fjoy_status.yaw;
	qrf_fjoyNRFData[1] = fjoy_status.pitch;
	qrf_fjoyNRFData[2] = fjoy_status.roll;
	qrf_fjoyNRFData[3] = fjoy_status.elev;
	
	nrf_Transmit(qrf_fjoyNRFData, 4, qrf_nrfCallback);
}

void qrf_SendJoyMeasurements(void)
{
	fjoy_CallOnUpdate(qrf_fjoyUpdateCallback);
}

void qrf_PrintJoyMeasurements(void)
{
	qrf_disp_PrintAxesPeriodically();
}	

void qrf_nrfCallback (bool success, u8 *payloadData, u8 length)
{
	if (success != _TRUE)
		qrf_packetLossCount++;
			
	if (payloadData != NULL)
	{
		qrf_battALevel = payloadData[0];
		qrf_battBLevel = payloadData[0];
	}
}