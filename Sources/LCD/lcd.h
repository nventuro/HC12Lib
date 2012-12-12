#ifndef _LCD_H
#define _LCD_H

#include "common.h"
#include "mc9s12xdp512.h"

typedef enum
{
	LCD_1602,	// 16 characters, 2 rows
	LCD_2004	// 20 characters, 4 rows
} lcd_type;

#define LCD_DATA PORTB
#define LCD_DATA_DDR DDRB
#define LCD_ENABLE PTH_PTH6
#define LCD_ENABLE_DDR DDRH_DDRH6
#define LCD_RS PTH_PTH7
#define LCD_RS_DDR DDRH_DDRH7

#define LCD_TIMER 7

void lcd_Init(lcd_type type);
void lcd_Print (char* string);

#endif