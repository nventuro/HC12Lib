#include "atd.h"
#include "mc9s12xdp512.h"
#include "error.h"

typedef struct
{
	atd_module module;
	bool init;
	u8 taskCount;
	
	atd_task tasksMem[TASK_MAX];
	atd_taskId currentTask;
	bool taskRunning;

	u16 conversionMem[ATD_AMOUNT];

}atd_data;


atd_data atd0_data;
atd_data atd1_data;


enum {OFF = 0, ON};		// Power
enum {FALLING_EDGE, RISING_EDGE, LOW_LEVEL, HIGH_LEVEL};


/******************		CTL2	****************/

#define ATD0_POWER(a) (ATD0CTL2_ADPU = a)
#define ATD1_POWER(a) (ATD1CTL2_ADPU = a)

#define ATD0_FAST_CLEAR(a) (ATD0CTL2_AFFC = a)
#define ATD1_FAST_CLEAR(a) (ATD1CTL2_AFFC = a)

#define ATD0_WAIT_HALT(a) (ATD0CTL2_AWAI = a)
#define ATD1_WAIT_HALT(a) (ATD1CTL2_AWAI = a)

#define ATD0_EXT_TRIGGER(a) do { (ATD0CTL2 &= ~(3<<2)); (ATD0CTL2 |= (a<<2));} while (0)
#define ATD1_EXT_TRIGGER(a) do { (ATD1CTL2 &= ~(3<<2)); (ATD1CTL2 |= (a<<2));} while (0)

#define ATD0_CH7EXT(a) (ATD0CTL2_ETRIGE = a)
#define ATD1_CH7EXT(a) (ATD1CTL2_ETRIGE = a)

#define ATD0_INTERRUPT(a) (ATD0CTL2_ASCIE = a)
#define ATD1_INTERRUPT(a) (ATD1CTL2_ASCIE = a)


/******************		CTL3	****************/

#define CHECK_U8_MAX(a,b) if ( (u8)(a) > (b)) continue
#define ATD0_CONVERSIONS(a) do {CHECK_U8_MAX(a,8); (ATD0CTL3 &= 0x07); (ATD0CTL3 |= (a<<3));} while (0)
#define ATD1_CONVERSIONS(a) do {CHECK_U8_MAX(a,8); (ATD1CTL3 &= 0x07); (ATD1CTL3 |= (a<<3));} while (0)

#define ATD0_FIFO(a) (ATD0CTL3_FIFO = a)
#define ATD1_FIFO(a) (ATD1CTL3_FIFO = a)


#define ATD0_ABORT_CONVERSION() ATD0_CONVERSIONS(0)
#define ATD1_ABORT_CONVERSION() ATD1_CONVERSIONS(0)


/******************		CTL4	****************/

#define ATD0_8BIT(a) (ATD0CTL4_SRES8 = a)
#define ATD1_8BIT(a) (ATD1CTL4_SRES8 = a)


/******************		CTL5	****************/

typedef enum {LEFT=0, RIGHT}justify;
typedef enum {UNS=0, SIG}sign;

const struct
{
	justify j;
	sign s;
	
}atd_defaults = {RIGHT, UNS};


#define ATD0_START(a) (ATD0CTL5 = a)
#define ATD1_START(a) (ATD1CTL5 = a)

void atd_conversionStart(atd_module m, justify j, sign s, bool scan, bool mult, u8 startChannel)
{
	u8 config = 0;
		
	if ( (startChannel > 7) || (m > ATD1) )
		return;
	
	config |= (j << 7);
	config |= (s << 6);
	config |= (scan << 5);
	config |= (mult << 4);
	config |= startChannel;
	
	if (m == ATD0)
		ATD0_START(config);
	else 
		ATD1_START(config);
	
	return;

}

#define ATD0_IS_SIGNED() (ATD0CTL5_DSGN)
#define ATD1_IS_SIGNED() (ATD1CTL5_DSGN)

#define ATD0_JUSTIFY(a) (ATD0CTL5_DJM = a)
#define ATD1_JUSTIFY(a) (ATD1CTL5_DJM = a)

#define ATD0_SIGNED_MODE(a) (ATD0CTL5_DSGN = a)	// justify left!
#define ATD1_SIGNED_MODE(a) (ATD1CTL5_DSGN = a)

#define ATD0_SCAN_MODE(a) (ATD0CTL5_SCAN = a)	// continuous conversion
#define ATD1_SCAN_MODE(a) (ATD1CTL5_SCAN = a)

#define ATD0_MULTI_MODE(a) (ATD0CTL5_MULT = a)
#define ATD1_MULTI_MODE(a) (ATD1CTL5_MULT = a)

#define ATD0_CHANNEL_SELECT(a) do {CHECK_U8_MAX(a,8); ATD0CTL5 &= 0xF8; ATD0CTL5 |= a;} while(0)
#define ATD1_CHANNEL_SELECT(a) do {CHECK_U8_MAX(a,8); ATD1CTL5 &= 0xF8; ATD0CTL5 |= a;} while(0)


/******************		STAT 0	****************/

#define ATD0_FLAG_CLR() (ATD0STAT0 = 0x80)
#define ATD1_FLAG_CLR() (ATD1STAT0 = 0x80)

#define ATD0_EXTFLAG_CLR() (ATD0STAT0 = 0x20)
#define ATD1_EXTFLAG_CLR() (ATD1STAT0 = 0x20)

#define ATD0_FIFOFLAG_CLR() (ATD0STAT0 = 0x10)
#define ATD1_FIFOFLAG_CLR() (ATD1STAT0 = 0x10)

u8 atd0_getConversionCounter(void)
{
	u8 aux = ATD0STAT0;
	return (aux & 0x0F);
}

u8 atd1_getConversionCounter(void)
{
	return (u8)(ATD1STAT0 & 0x0F);
}


/******************		STAT 1	****************/


void atd0_ccfRead(void)
{
	volatile int a = ATD0STAT1;
	return;
}

void atd1_ccfRead(void)
{
	volatile int a = ATD1STAT1;
	return;
}



/******************		IE register ****************/

#define ATD0_DIGITAL_ENABLE(a) (ATD0DIEN |= (1<<a))
#define ATD0_DIGITAL_DISABLE(a) (ATD0DIEN &= ~(1<<a))

#define ATD1_DIGITAL_ENABLE(a) (ATD1DIEN |= (1<<a))
#define ATD1_DIGITAL_DISABLE(a) (ATD1DIEN &= ~(1<<a))




/******************		Private functions	****************/


void atd_srv(atd_module module);

void freeTask(atd_data* moduleData, atd_taskId id);
void runTask(const atd_task* ref, atd_data* module);

void solveTasksLeft(atd_data* moduleData);
atd_taskId getNextTask(atd_task tasksMem[], atd_taskId current);


atd_data* getModuleData(atd_module module);

static void hardwareInit(atd_module module);
void copyConvResult(atd_data *modData);

void atd_Init(atd_module module)
{
	u32 i;
	atd_data *moduleData = getModuleData(module);

	if (moduleData->init == _TRUE)
		return;
	
	moduleData->module = module;
	moduleData->taskCount = 0;
	moduleData->currentTask = INVALID_TASK_ID;
	moduleData->taskRunning = _FALSE;
	
	hardwareInit(module);
	
	for (i = 0; i < TASK_MAX; i++)
		moduleData->tasksMem[i].eocCB = NULL;
		
	moduleData->init = _TRUE;
	
	return;
}


atd_taskId atd_SetTask(atd_module module, u8 channel, u8 length, bool mult, bool scan, atd_ptr cb)
{
	atd_taskId i;
	atd_data *moduleData;
	bool interruptsEnabled = SafeSei();
	moduleData = getModuleData(module);

	if (cb == NULL)
		err_Throw("atd: null callback received.\n");
	
	for (i = 0; i < TASK_MAX; i++)
	{
		if (moduleData->tasksMem[i].eocCB != NULL)
			continue;
		
		moduleData->tasksMem[i].channel = channel;
		moduleData->tasksMem[i].length 	= length;
		moduleData->tasksMem[i].mult 	= mult;
		moduleData->tasksMem[i].scan 	= scan;
		moduleData->tasksMem[i].eocCB 	= cb;
		
		moduleData->taskCount++;
		break;
	}
	
	if (i == TASK_MAX)
		err_Throw("atd: attempt to register more tasks than there's memory for.\n");
	
	if (moduleData->taskCount == 1)
	{
		moduleData->currentTask = i;	// just annotated first task
		runTask(&moduleData->tasksMem[i], moduleData);
	}
	
	SafeCli(interruptsEnabled);
	
	return i;
}


void atd_FreeTask(atd_module module, atd_taskId id)
{
	atd_data *moduleData = getModuleData(module);
	freeTask(moduleData, id);
	
	solveTasksLeft(moduleData);	// user-call free task must solve next
	
	return;
}


void freeTask(atd_data* moduleData, atd_taskId id)	// interrupt-only call does not solve next task
{
	bool interruptsEnabled;
	
	if (moduleData->tasksMem[id].eocCB == NULL)
		return;
	
	interruptsEnabled = SafeSei();
	
	if (moduleData->currentTask == id)
		if (moduleData->module == ATD0)
			ATD0_ABORT_CONVERSION();
		else
			ATD1_ABORT_CONVERSION();
	
	moduleData->tasksMem[id].eocCB = NULL;
	if (--moduleData->taskCount || (moduleData->currentTask == id) )
		moduleData->taskRunning = _FALSE;
	
	SafeCli(interruptsEnabled);
	return;
	
}


void runTask(const atd_task* ref, atd_data* modData)
{
	// Writes to CTL3 aborts conversion -> the module can't interrupt.

	if (modData->module == ATD0)
		ATD0_CONVERSIONS(ref->length);
	else
		ATD1_CONVERSIONS(ref->length);
	
	atd_conversionStart(modData->module, atd_defaults.j, atd_defaults.s, ref->scan, ref->mult, ref->channel);
	modData->taskRunning = _TRUE;
	
	return;
}

atd_data* getModuleData(atd_module module)
{
	if (module == ATD0)
		return &atd0_data;
	else if (module == ATD1)
		return &atd1_data;
	else
		return NULL;
}


void interrupt atd0_Service(void)
{
	atd_srv(ATD0);
	ATD0_FLAG_CLR();
	return;
}


void interrupt atd1_Service(void)
{
	atd_srv(ATD1);
	ATD1_FLAG_CLR();
	return;
}


void atd_srv(atd_module module)
{
	atd_data *modData = getModuleData(module);
	atd_ptr cb = modData->tasksMem[modData->currentTask].eocCB;

	copyConvResult(modData);
	
	if (modData->tasksMem[modData->currentTask].scan == _FALSE)
		freeTask(modData, modData->currentTask);	// if new task is solved here, there would be
													// two calls to solveTasksLeft (wrong)
	if (cb != NULL)
		cb(modData->conversionMem, &modData->tasksMem[modData->currentTask]);											
	
	solveTasksLeft(modData);	// new task must be solved here, not in when freed by atd_srv above
								// (AFTER callBack call)
	return;
}

// if there is more than one task, or there is only one but nothing is running, fetch next task.
#define OTHER_TASKS_LEFT(count, taskRunning) ( (count > 1) || ((taskRunning == _FALSE) && (count > 0)) )

void solveTasksLeft(atd_data* modData)
{
	if (OTHER_TASKS_LEFT(modData->taskCount, modData->taskRunning))	// if there aren't tasks left, it means count is in 0 (nothing to do)
	{																// If nothing is running and there is something left, get new task.
		modData->currentTask = getNextTask(modData->tasksMem, modData->currentTask);
		runTask(&modData->tasksMem[modData->currentTask], modData);
	}
	
	return;
}


atd_taskId getNextTask(atd_task tasksMem[], atd_taskId current)
{
	do
	{
		++current;				// if current was invalid (-1), still works
		current %= TASK_MAX;
	
	}while(tasksMem[current].eocCB == NULL);
	
	return current;
}

static void hardwareInit(atd_module module)
{
	u32 i;
	if (module == ATD0)
	{
		ATD0_POWER(ON);
		ATD0_FAST_CLEAR(ON);
		ATD0_INTERRUPT(ON);
		ATD0_FIFO(OFF);
		ATD0_FLAG_CLR();
	
		ATD0_SET_DIVIDER(26);	// Max: 40 MHz
		ATD0_SAMPLE_TIME(LENGTH_2);
	}
	else
	{
		ATD1_FAST_CLEAR(ON);
		ATD1_INTERRUPT(ON);
		ATD1_FIFO(OFF);
		ATD1_SET_DIVIDER(20);	// Max: 40 MHz
		ATD1_SAMPLE_TIME(LENGTH_2);
		ATD1_POWER(ON);
		ATD1_FLAG_CLR();
	}
	
	return;
}


void copyConvResult(atd_data *modData)	// copies only up to length
{

	s16* resultBase;
	u8 i;	
	
	if (modData->module == ATD0)
		resultBase = (s16*)&ATD0DR0;//+atd0_getConversionCounter();
	else
		resultBase = (s16*)&ATD1DR0;//+atd1_getConversionCounter();
	
	for (i = 0; i < modData->tasksMem[modData->currentTask].length; i++)
		modData->conversionMem[i] = resultBase[i];

	return;
}
