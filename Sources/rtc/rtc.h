#ifndef _RTC_H
#define _RTC_H

#include "common.h"

#define RTC_TIMER 2

typedef void (*rtc_ptr)(void);

typedef struct 
{
	u8 deca;
    u8 uni;
} decimal;


typedef enum 
{
    RTC_MONDAY,
    RTC_TUESDAY,
    RTC_WEDNESDAY,
    RTC_THURSDAY,
    RTC_FRIDAY,
    RTC_SATURDAY,
    RTC_SUNDAY
} rtc_day;

typedef struct 
{
    decimal seconds;
    decimal minutes;
    decimal hours;
    decimal date;
    decimal month;
    decimal year;
    rtc_day day;
} rtc_data_T;


#ifdef _RTC_PRIVILEGED
extern rtc_data_T rtc_data;
#else
extern const rtc_data_T rtc_data;
#endif


void rtc_init (void);
bool rtc_isAutoUpdateEnabled (void);
void rtc_enableAutoUpdate (void);
void rtc_disableAutoUpdate (void);
void rtc_assignAutoUpdateCallback (rtc_ptr cb);

void rtc_setTime(decimal sec, decimal min, decimal h, decimal date, 
						 decimal month, decimal year, rtc_day d);

u8 decimal2u8(decimal d);


#endif