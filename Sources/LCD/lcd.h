#ifndef _LCD_H
#define _LCD_H

#include "common.h"
#include "mc9s12xdp512.h"

typedef enum
{
	LCD_1602,	// 16 characters, 2 rows
	LCD_2004	// 20 characters, 4 rows
} lcd_type;

#define LCD_DATA PORTB // The 8-bit port used to send data to the device.
#define LCD_DATA_DDR DDRB // The DDR register for LCD_DATA
#define LCD_ENABLE PTH_PTH6 // The enable (clock) pin.
#define LCD_ENABLE_DDR DDRH_DDRH6 // The DDR register for LCD_ENABLE
#define LCD_RS PTH_PTH7 // The Register Select pin.
#define LCD_RS_DDR DDRH_DDRH7 // The DDR register for LCD_RS
// The Read/Write pin should be connected to ground.

// The device's pins should be connected to the MCU pins described above.
// If any of the ports or pins defined above is changed, lcd.c must be recompiled.

#define LCD_TIMER 7 // The timer module used for timing by the LCD module. This pin should be left unconnected.


void lcd_Init(lcd_type type);
// Initializes the LCD module. This requires the Timers module to work, and interrupts to be enabled.
// type determines the number of rows and columns the module expects the device to have. A wrong value on type will 
// cause faulty behavior. After lcd_Init is called, type cannot be changed.

void lcd_Print (char* string);
// Prints a string on the device, starting on the first row and first column. 
// If the string is not null-terminated, or too long, the module will only print as many characters as the device is capable of displaying.
// If the string has less characters than the device can display, the remaining characters are left empty.
// This deletes any message previously printed on the device.

#endif