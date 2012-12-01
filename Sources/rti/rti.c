#include <mc9s12xdp512.h>
#include "rti.h"

#define RTI_PRESCALER (0x29)
#define RTI_SETPRESCALER(presc) (RTICTL = presc)
#define RTI_ENABLE_INTERRUPTS() (CRGINT_RTIE = 1)
#define RTI_CLEAR_FLAG() (CRGFLG_RTIF = 1)

#define RTI_MAX_FCNS 20

#define RTI_IS_VALID_ID(id) (((id >= 0) && (id < RTI_MAX_FCNS)) ? _TRUE : _FALSE)

struct rti_cb {
	rti_time period;
	rti_time count;
	rti_ptr callback;
	s8 protect;
	void *data;
};

bool rti_isInit = _FALSE;

struct rti_cb rti_tbl[RTI_MAX_FCNS];


void rti_init()
{
	rti_id i;
	if (rti_isInit == _TRUE)
		return;
	
	rti_isInit = _TRUE;	
	
	for (i = 0; i < RTI_MAX_FCNS; i++)
		rti_tbl[i].callback = NULL;
	
	RTI_SETPRESCALER (RTI_PRESCALER);
	RTI_ENABLE_INTERRUPTS();
	RTI_CLEAR_FLAG();
	
	return;
}


rti_id rti_register (rti_ptr callback, void *data, rti_time period, rti_time delay)
{
	return rti_register2(callback, data, period, delay, RTI_DEFAULT_PROTECT);
}


rti_id rti_register2(rti_ptr callback, void *data, rti_time period, rti_time delay, s8 protect)
{
	rti_id i;
	for (i = 0; i < RTI_MAX_FCNS; i++) 
	{
		if (rti_tbl[i].callback == NULL) 
		{
			rti_tbl[i].callback = callback;
			rti_tbl[i].data = data;
			rti_tbl[i].period = period;
			rti_tbl[i].protect = protect;
			rti_tbl[i].count = delay;
			break;
		}
	}
		
	if (i == RTI_MAX_FCNS)
		i = RTI_INVALID_ID;
	
	return i;
}


void rti_set_period(rti_id id, rti_time period)
{
	if (!RTI_IS_VALID_ID(id))
		return;
	
	rti_tbl[id].period = period;
	
	return;
}


void rti_cancel(rti_id id)
{
	if (!RTI_IS_VALID_ID(id))
		return;
	
	rti_tbl[id].protect = RTI_NORMAL;
	rti_tbl[id].count = RTI_CANCEL;
}


void interrupt rti_srv(void)
{
	rti_id i;
	for (i = 0; i < RTI_MAX_FCNS; i++) 
	{
		if (rti_tbl[i].callback != NULL) 
		{
			if (rti_tbl[i].count == RTI_AUTOCANCEL) 
			{
				if (!rti_tbl[i].protect == RTI_PROTECT)
					rti_tbl[i].callback = NULL;
			} 
			else 
			{
				if((--rti_tbl[i].count) == 0) 
				{
					rti_tbl[i].period = rti_tbl[i].callback(rti_tbl[i].data, rti_tbl[i].period);
					rti_tbl[i].count = rti_tbl[i].period;
				}
			}
		}
	}

	RTI_CLEAR_FLAG();
	
	return;
}