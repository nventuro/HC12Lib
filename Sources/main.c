 #include "mc9s12xdp512.h"
 #include <stdio.h>
 #include "common.h"
#include "dmu.h"
#include "dmu_macros.h"
#include "rti.h"
#include "timers.h"
 #include "pll.h"
#include "quick_serial.h"
#include "usonic.h"
#include "error.h"
#include "quad_control.h"
#include "nlcf.h"
#include "arith.h"
#include "atd.h"
#include "motors.h"

//#include "batt.h"
#include "lcd.h"
#include "rfrx.h"
#define DMU_TIMER 1
#define C1_ID 0
#define C2_ID 1
#define SHIFT_ID 2
#define BATT_ID 3

#define UP 0xFF
#define DOWN 0x00

extern struct dmu_data_T dmu_data;
 
void Init (void);
void PrintMeas (s32 measurement);
void GetMeasurementsMask(void *data, rti_time period, rti_id id);
void rti_MotDelay(void *data, rti_time period, rti_id id);
void GetSamplesMask(void *data, rti_time period, rti_id id);
void dataReady_Srv(void);
void dataReady_Ovf(void);
void fifoOvf_Srv(void);
void icFcn(void);

 
struct tim_channelData dmu_timerData = {0,0};

u16 overflowCnt = 0;
u16 lastEdge = 0;

extern void att_process(void);
extern bool have_to_output;

extern bool readyToCalculate;
extern struct motorData motData;
extern controlData_T controlData;

bool have_to_output = 0;

quat setpoint = UNIT_Q;

void att_process(void)
{
	static int ccount = 0;

	PORTA_PA0 = 1;
	{
		att_estim(dmu_measurements.gyro, dmu_measurements.accel, 
							&controlData.QEst, &controlData.bff_angle_rate);

		// El control se corre sólo después de inicializar los motores, para que el control integral
		// no empiece acumulando error.
		if (motData.mode == MOT_AUTO)
		{
			controlData.torque = adv_att_control(setpoint, controlData.QEst, controlData.bff_angle_rate);
			controlData.thrust = 11500;
		}
		
		if (++ccount == 20) {
			ccount = 0;
			have_to_output = 1;
		}
	}
	PORTA_PA0 = 0;
}

void sample_ready(void)
{	
	if (tim_GetEdge(DMU_TIMER) == EDGE_RISING) 
	{
		tim_SetFallingEdge(DMU_TIMER);
		dmu_GetMeasurements(att_process);
		
	} else {
		tim_SetRisingEdge(DMU_TIMER);
	}
}

/* Main para control */

#define Q_COMPONENTS(q) (q).r, (q).v.x, (q).v.y, (q).v.z
extern vec3 Bias;


#define OC_PERIOD ((u8)62500)
#define TIM4_DUTY 14000
#define TIM5_DUTY 5000
#define TIM6_DUTY 10000
#define TIM7_DUTY 9375

u8 batt1;
u8 batt2;

u8 rx_data;

void lowBatt(void)
{
	err_Throw("Low battery.\n");
}

void c1_rx(u8 length)
{/*
	if (rx_data == UP) // Tocaron a
	{
		quat aux = {32488, 3024, -3024, 0};
		setpoint = aux;
	}//aumentar c1 (en lo que te pinte)
	else // tocaron S
	{
		quat aux = UNIT_Q;
		setpoint = aux;
	}//bajar c1*/
}

void c2_rx(u8 length)
{
	if (rx_data == UP) // tocaron d
	{
		quat aux = {32488, -3024, 3024, 0};
		setpoint = aux;
		putchar('a');	

	}
	if (rx_data == DOWN) //tocaron q
	{
		putchar('b');
		motData.mode = MOT_MANUAL;
		
		motData.speed[0] = 0;
		motData.speed[1] = 0;
		motData.speed[2] = 0;
		motData.speed[3] = 0;
		
//		while (1)
//			;

	}
}
u8 start = _FALSE;

void shift_rx(u8 length)
{
	if (rx_data == UP)
	{
		putchar('c');
		if (controlData.thrust + 200 > 0) 
			controlData.thrust += 200;//aumentar shift (en lo que te pinte)
	}
	else
	{
		putchar('d');
		if (controlData.thrust - 200 > 0) 
			controlData.thrust -= 200;
	}
	//bajar shift*/
}

void batt_rx(u8 length)
{
	putchar('e');
	start = _TRUE;
}



void main (void)
{
	u8 userInput;
	u8 measurementCount = 0;
	bool motDelayDone = _FALSE;

	u16 torqueCount = 0;

	Init ();	
	DDRA_DDRA0 = 1;
	DDRA_DDRA1 = 1;
	DDRA_DDRA2 = 1;
	PORTA_PA2 = 0;
	DDRA_DDRA3 = 1;
	PORTA_PA3 = 0;
	DDRA_DDRA5 = DDR_OUT;
	PORTA_PA5 = 0;
	DDRA_DDRA6 = DDR_OUT;
	PORTA_PA6 = 0;
	
	tim_GetTimer(TIM_IC, sample_ready, dataReady_Ovf, DMU_TIMER);
	tim_SetRisingEdge(DMU_TIMER);
	tim_ClearFlag(DMU_TIMER);
	tim_EnableInterrupts(DMU_TIMER);
	
	

// COMETNADO
//	batt_AddBatt (ATD0, 0, lowBatt, BATT_MV_TO_LEVEL(3800), BATT_MV_TO_LEVEL(3600), BATT_MV_TO_LEVEL(4200), &batt1);
//	batt_AddBatt (ATD0, 1, lowBatt, BATT_MV_TO_LEVEL(3800), BATT_MV_TO_LEVEL(3600), BATT_MV_TO_LEVEL(4200), &batt2);
/*	
	rfrx_Register(C1_ID, c1_rx, &rx_data);
	rfrx_Register(C2_ID, c2_rx, &rx_data);
	rfrx_Register(SHIFT_ID, shift_rx, &rx_data);
	rfrx_Register(BATT_ID, batt_rx, &rx_data);
*/

	/*
	printf("Press 'm' to calibrate\n");

	while (measurementCount < 2)
	{
		struct cal_output calibrationOutput;
		struct qpair calibration;

		userInput = qs_getchar(0);
		
		if (userInput == 'c')
		{
			quat aux;
			asm sei;
			aux = QEst;
			asm cli;
			printf("Current quaternion: %d %d %d %d\n", Q_COMPONENTS(aux));
			continue;		
		}
		
		if (userInput != 'm')
			continue;
		
		if (measurementCount == 0)
		{	asm sei;
			calibration.p0 = QEst;
			asm cli;
			printf("First measurement done\n");
		}
		else if (measurementCount == 1)
		{
			asm sei;
			calibration.p1 = QEst;
			asm cli;
			printf("Second measurement done\n");
		}
		measurementCount++;
		
		if (measurementCount == 2)
		{
			calibrationOutput = att_calibrate(calibration.p0, calibration.p1);			
			printf("Cal output: %d\n", calibrationOutput.quality);
			printf("Correction: %d %d %d %d\n", Q_COMPONENTS(calibrationOutput.correction));
			
			if (calibrationOutput.quality == CAL_BAD)
			{
				measurementCount = 1;	// Stay looping second measurement.
				printf("Calibrate again\n");
			}

			//att_apply_correction(calibrationOutput);
		}
	}
	*/

	mot_Init();
			
	rti_Register (rti_MotDelay, &motDelayDone, RTI_ONCE, RTI_MS_TO_TICKS(3000));

	while(!motDelayDone)
		;
	
	motData.speed[0] = S16_MAX;
	motData.speed[1] = S16_MAX;
	motData.speed[2] = S16_MAX;
	motData.speed[3] = S16_MAX;
	motDelayDone = _FALSE;
	rti_Register (rti_MotDelay, &motDelayDone, RTI_ONCE, RTI_MS_TO_TICKS(2000));

	while(!motDelayDone)
		;

	motData.speed[0] = 0;
	motData.speed[1] = 0;
	motData.speed[2] = 0;
	motData.speed[3] = 0;


	motDelayDone = _FALSE;
	rti_Register (rti_MotDelay, &motDelayDone, RTI_ONCE, RTI_MS_TO_TICKS(3000));

	while(!motDelayDone)
		;

	
//	while (start == _FALSE)
//		;

	
	motData.mode = MOT_AUTO;

	while (1) {
		char input;
		input = qs_getchar(0);
		
	//	if (have_to_output)
	//	{
	//		printf("%d %d %d %d,", Q_COMPONENTS(QEst));
	//		//printf("%d %d %d\n", Bias.x, Bias.y, Bias.z);
	//		have_to_output = 0;
	//	}
/*		
		if (input == 'a')
		{
			//quat aux = {32488, 3024, -3024, 0};
			quat aux = {32488, -4277, 0, 0};
			
			setpoint = aux;
		}			
		else if (input == 's')
		{
			quat aux = UNIT_Q;
			setpoint = aux;
		}
		else if (input == 'd')
		{
			//quat aux = {32488, -3024, 3024, 0};
			quat aux = {32488, 4277, 0, 0};
			setpoint = aux;
		}
		else if (input == 'q')
		{
			motData.mode = MOT_MANUAL;
			
			motData.speed[0] = 0;
			motData.speed[1] = 0;
			motData.speed[2] = 0;
			motData.speed[3] = 0;
			
			while (1)
				;
		}
*/		
	
		/*
		
		if (readyToCalculate){
			quat QEstAux;
			vec3 torque_copy;
			
	//		asm sei;
	//		readyToCalculate = _FALSE;
	//		QEstAux = controlData.QEst;
	//		torque_copy = controlData.torque;
	//		asm cli;
			
			//controlData.thrust = h_control(6000, 0);
			//torque = adv_att_control(setpoint, QEstAux);
			
			
			if (torqueCount++ >= 20)
			{
				controlData_T *p = &controlData;
				torqueCount = 0;
				printf(">%d %d %d %d\n", Q_COMPONENTS(QEstAux));
				printf("%d %d %d\n", p->torque.x, p->torque.y, p->torque.z);
				printf("%d %d %d %d\n", motData.speed[0], motData.speed[1], motData.speed[2], motData.speed[3]);
		
			}
			

			//motData.speed[0] = 10000;
		}
		*/
	}
	
	/*
	while(1) {
		if (have_to_output) {
			quat QEstAux;
		
			asm sei;
			have_to_output = 0;
			QEstAux = controlData.QEst;
			asm cli;
			
			printf("%d %d %d %d,", Q_COMPONENTS(QEstAux));
		}
	}
	*/
}


void measure (s32 measurement);

/*
// MAIN de testeo para DMU. 
 void main (void)
 {
	int a;
	char vel;

	PLL_SPEED(BUS_CLOCK_MHZ);

	Init ();

//	DDRA = 0x01;



	tim_GetTimer(TIM_IC, dataReady_Srv, NULL, DMU_TIMER);
	tim_EnableInterrupts(DMU_TIMER);
	tim_SetRisingEdge(DMU_TIMER); 


//	mot_Init();


//	tim_GetTimer(TIM_IC, fifoOvf_Srv, NULL, DMU_TIMER);

//	tim_EnableInterrupts(DMU_TIMER);
//	tim_SetRisingEdge(DMU_TIMER); 

//	rti_Register(GetSamplesMask, NULL, RTI_MS_TO_TICKS(500), RTI_MS_TO_TICKS(

//	rti_Register(GetMeasurementsMask, NULL, RTI_MS_TO_TICKS(500), RTI_MS_TO_TICKS(500));
//	usonic_Measure(measure);
 	while (1)
 	{
 		vel = qs_getchar(0);
	 	if(vel == 'u')
	 	{
	 		motData.speed[0] += 300;
	 		motData.speed[1] += 300;
	 		motData.speed[2] += 300;
	 		motData.speed[3] += 300;
	 	}
	 	else if(vel == 'd')
	 	{
	 		motData.speed[0] -= 300;
	 		motData.speed[1] -= 300;
	 		motData.speed[2] -= 300;
	 		motData.speed[3] -= 300;
	 	}

 	}
 		
}
*/
/*
void measure (s32 measurement)
{
	if (measurement != USONIC_INVALID_MEAS)
		printf("Distance: %ld cm.\n",measurement);
	else
		printf("Invalid measurement.\n");
		
	 usonic_Measure(measure);
}
*/
void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);
	//PLL_SPEED(24);

 	// Modules that don't require interrupts to be enabled
	tim_Init();
	rti_Init();	
	qs_init(0, MON12X_BR);
 
 	asm cli;
 
 	// Modules that do require interrupts to be enabled
	iic_Init();
	dmu_Init();
//	rfrx_Init();
//	batt_Init();
//	lcd_Init(LCD_2004);


	printf("Init done");


	return;
}

void GetMeasurementsMask(void *data, rti_time period, rti_id id)
{
	dmu_GetMeasurements(dmu_PrintFormattedMeasurements_WO);	
	return;
}

void GetSamplesMask(void *data, rti_time period, rti_id id)
{
	dmu_FifoAverage(NULL);
	return;
}

void rti_MotDelay(void *data, rti_time period, rti_id id)
{
	*(bool*)data = _TRUE;
	return;
}


void PrintMeas (s32 measurement)
{
	//printf("%ld\n", measurement);
}


void dataReady_Srv(void)
{
	static u16 count=0;

	if (tim_GetEdge(DMU_TIMER) == EDGE_RISING)
	{
		tim_SetFallingEdge(DMU_TIMER);
		if (++count == 100)
		{
			dmu_GetMeasurements(dmu_PrintFormattedMeasurements);
			count = 0;
		}
	}
	else 
		tim_SetRisingEdge(DMU_TIMER);
}

void dataReady_Ovf(void)
{
//	dmu_timerData.overflowCnt++;
	overflowCnt++;

	return;
}

void fifoOvf_Srv(void)
{
//	printf("fifo ovf!!!\n");	


	if (dmu_data.fifo.enable == _FALSE)
		return;

	dmu_data.fifo.enable = _FALSE;
	dmu_ReadFifo(NULL);

//	dmu_FifoReset(NULL);

	return;
}



void icFcn()
{
	static u16 count = 0;
	u32 time;

	time = tim_GetTimeElapsed(overflowCnt, 2, lastEdge);

	lastEdge = tim_GetValue(2);
	overflowCnt = 0;
 	count++;
 
 	if (count == 10)
 	{
 		printf("t: %lu\n", time*TIM_TICK_NS);
 		count = 0;
 	}
 	return;
 }
