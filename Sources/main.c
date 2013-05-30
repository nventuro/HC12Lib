 #include "mc9s12xdp512.h"
 #include "common.h"
#include "dmu.h"
#include "dmu_macros.h"
#include "rti.h"
#include "timers.h"
#include <stdio.h>
 #include "pll.h"
#include "quick_serial.h"
#include <limits.h>
#include "usonic.h"

#include "quad_control.h"
#include "nlcf.h"

#define DMU_TIMER 1

extern struct dmu_data_T dmu_data;
 
void Init (void);
void PrintMeas (s32 measurement);
void GetMeasurementsMask(void *data, rti_time period, rti_id id);
void GetSamplesMask(void *data, rti_time period, rti_id id);
void dataReady_Srv(void);
void dataReady_Ovf(void);
void fifoOvf_Srv(void);
void icFcn(void);

 
struct tim_channelData dmu_timerData = {0,0};

u16 overflowCnt = 0;
u16 lastEdge = 0;

extern quat QEst;
extern void att_process(void);
extern bool have_to_output;

extern bool readyToCalculate;
extern struct motorData motData;

void sample_ready(void)
{
	if (tim_GetEdge(DMU_TIMER) == EDGE_RISING) {
		tim_SetFallingEdge(DMU_TIMER);
		dmu_GetMeasurements(att_process);
	} else {
		tim_SetRisingEdge(DMU_TIMER);
	}
}

/* Main para control */

#define Q_COMPONENTS(q) (q).r, (q).v.x, (q).v.y, (q).v.z
extern vec3 Bias;
void main (void)
{
	u8 userInput;
	u8 measurementCount = 0;

	u16 torqueCount = 0;
	quat setpoint = {FRAC_1,0,0,0};
	Init ();	
	DDRA_DDRA0 = 1;
	DDRA_DDRA1 = 1;
	
	tim_GetTimer(TIM_IC, sample_ready, NULL, DMU_TIMER);
	tim_SetRisingEdge(DMU_TIMER);
	tim_ClearFlag(DMU_TIMER);
	tim_EnableInterrupts(DMU_TIMER);

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

			att_apply_correction(calibrationOutput);
		}
	}

*/
//	mot_Init();


	while (1) {
	

		if (have_to_output)
		{
			printf("%d %d %d %d,", Q_COMPONENTS(QEst));
			//printf("%d %d %d\n", Bias.x, Bias.y, Bias.z);
			have_to_output = 0;
		}
	
	/*
		frac thrust = 0;
		vec3 torque = {0,0,0};
		
		if (readyToCalculate){
			quat QEstAux;
			
			asm sei;
			readyToCalculate = _FALSE;
			QEstAux = QEst;
			asm cli;
			
			thrust = h_control(10000, 0);
			torque = adv_att_control(setpoint, QEstAux);
			
			if (torqueCount++ >= 10)
			{
				torqueCount = 0;
				printf(">%d %d %d %d\n", Q_COMPONENTS(QEstAux));
				printf("%d %d %d\n", torque.x, torque.y, torque.z);
			}
			
			motData = control_mixer(thrust, torque);
		
		}
	*/
	}
}



#define OC_PERIOD ((u8)62500)
#define TIM4_DUTY 14000
#define TIM5_DUTY 5000
#define TIM6_DUTY 10000
#define TIM7_DUTY 9375


/*
int main(void)
{
	volatile frac f = FRAC_0_5;
	
	PLL_SPEED(BUS_CLOCK_MHZ);
	qs_init(0, MON12X_BR);
 
 	asm cli;
 	
 	mot_Init();
 	
	
	
	while(1);

}

*/
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
void measure (s32 measurement)
{
	if (measurement != USONIC_INVALID_MEAS)
		printf("Distance: %ld cm.\n",measurement);
	else
		printf("Invalid measurement.\n");
		
	 usonic_Measure(measure);
}

void Init (void)
{
	PLL_SPEED(BUS_CLOCK_MHZ);

 	// Modules that don't require interrupts to be enabled
	tim_Init();
//	rti_Init();	
	qs_init(0, MON12X_BR);
 
 	asm cli;
 
 	// Modules that do require interrupts to be enabled
	iic_Init();
	dmu_Init();
//	usonic_Init();

	printf("Init done\n");		


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


void PrintMeas (s32 measurement)
{
	printf("%ld\n", measurement);
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


/*
void icFcn()
{
	u32 time;

	time = tim_GetTimeElapsed(overflowCnt, 0, lastEdge);

	lastEdge = tim_GetValue(0);
	overflowCnt = 0;
 
	printf("t: %ld\n", time*TIM_TICK_NS);
 
 	return;
 }
*/