#include "lcd.h"
#include "timers.h"
#include <stdio.h>

#define DDR_OUT 0xFF

#define LCD_RS_INSTR 0
#define LCD_RS_DATA 1

#define LCD_CLEAR_DISP BIT(0)

#define LCD_CURSOR_HOME BIT(1)

#define LCD_ENTRY_MODE BIT(2)
#define LCD_MOVE_DIR_UP BIT(1)
#define LCD_MOVE_DIR_DOWN 0
#define LCD_NO_SHIFT 0
#define LCD_SHIFT BIT(0)

#define LCD_DISP_CTL BIT(3)
#define LCD_DISP_ON BIT(2)
#define LCD_DISP_OFF 0
#define LCD_CURSOR_ON BIT(1)
#define LCD_CURSOR_OFF 0
#define LCD_BLINK_ON BIT(0)
#define LCD_BLINK_OFF 0

#define LCD_CUR_DISP_SHIFT BIT(4)
#define LCD_MOVE_CUR 0
#define LCD_SHIFT_DISP BIT(3)
#define LCD_SHIFT_RIGHT BIT(2)
#define LCD_SHIFT_LEFT 0

#define LCD_FUNC_SET BIT(5)
#define LCD_4BIT 0
#define LCD_8BIT BIT(4)
#define LCD_1LINE 0
#define LCD_2LINE BIT(3)
#define LCD_SMALL_FONT 0
#define LCD_LARGE_FONT BIT(2)

#define LCD_WRITE_DDRAM BIT(7)

#define LCD_ENABLE_NOPS 75
#define LCD_ENABLE_STROBE() do{u8 i;LCD_ENABLE = 1;	for (i = 0; i < LCD_ENABLE_NOPS; i++) asm nop; LCD_ENABLE = 0;} while (0)

#define LCD_MEMORY 80

#define LCD_SHORT_DELAY 125 // About 200us
#define LCD_LONG_DELAY 2000 // About 3.2ms

struct
{
	lcd_type type;
	tim_id timer;
	u8 index;
	char memory[LCD_MEMORY];
	u8 initStage;
} lcd_data;

bool lcd_isInit = _FALSE;

void lcd_InitCallback (void);
void lcd_PrintCallback (void);

void lcd_Init(lcd_type type)
{
	u32 i;
	
	if (lcd_isInit == _TRUE)
		return;
	
	// Set pins as output
	LCD_DATA_DDR = DDR_OUT;
	LCD_ENABLE_DDR = DDR_OUT;
	LCD_RS_DDR = DDR_OUT;
	LCD_ENABLE = 0;
	
	
	// Initialize the driver's memory
	lcd_data.type = type;
	for (i = 0; i < LCD_MEMORY; i++)
			lcd_data.memory[i] = ' ';
	
	lcd_data.index = 0;

	// Device initialization
	lcd_data.initStage = 0;

	tim_Init();
	lcd_data.timer = tim_GetTimer(TIM_OC, lcd_InitCallback, NULL, LCD_TIMER);

	tim_SetValue (lcd_data.timer, tim_GetGlobalValue() - 100); // Wait for about 100ms
	
	tim_ClearFlag(lcd_data.timer);
	tim_EnableInterrupts (lcd_data.timer);

	// Wait for initialization to end
	while (lcd_isInit != _TRUE)
		;
		
	return;

}

void lcd_Print (char* string)
{
	u8 i = 0;	
	
	if (lcd_data.type == LCD_1602)
	{
		while ((string[i] != '\0') && (i < 32))
		{
			lcd_data.memory[i] = string[i];
			i++;
		}
		while (i < 32)
			lcd_data.memory[i++] = ' ';
	}
	else if (lcd_data.type == LCD_2004)
	{
		while ((string[i] != '\0') && (i < 80))
		{
			lcd_data.memory[i] = string[i];
			i++;
		}
		while (i < 80)
			lcd_data.memory[i++] = ' ';
	}
}

void lcd_PrintCallback (void)
{
	LCD_RS = LCD_RS_DATA;
	LCD_DATA = lcd_data.memory[lcd_data.index++];
	LCD_ENABLE_STROBE();
		
	if ((lcd_data.index == 16) && (lcd_data.type == LCD_1602))
	{
		LCD_RS = LCD_RS_INSTR;
		LCD_DATA = (LCD_WRITE_DDRAM | 0x40);
		LCD_ENABLE_STROBE();
	}
	else if ((lcd_data.index == 32) && (lcd_data.type == LCD_1602))
	{
		LCD_RS = LCD_RS_INSTR;
		LCD_DATA = (LCD_WRITE_DDRAM | 0x00);
		LCD_ENABLE_STROBE();

		lcd_data.index = 0;
	}
	else if ((lcd_data.index == 20) && (lcd_data.type == LCD_2004))
	{
		lcd_data.index = 40;
	}
	else if ((lcd_data.index == 40) && (lcd_data.type == LCD_2004))
	{
		lcd_data.index = 60;
	}
	else if ((lcd_data.index == 60) && (lcd_data.type == LCD_2004))
	{
		lcd_data.index = 20;
	}
	else if ((lcd_data.index == 80) && (lcd_data.type == LCD_2004))
	{
		lcd_data.index = 0;
	}
	
	tim_SetValue(lcd_data.timer, tim_GetValue(lcd_data.timer) + LCD_SHORT_DELAY);
}


void lcd_InitCallback (void)
{
	switch (lcd_data.initStage)
	{
		case 0:
			LCD_RS = LCD_RS_INSTR;
			LCD_DATA = (LCD_FUNC_SET | LCD_8BIT);
			LCD_ENABLE_STROBE();
			
			lcd_data.initStage = 1;
			
			tim_SetValue (lcd_data.timer, tim_GetValue(lcd_data.timer) + 4000); // Wait for about 6.4ms
			
			break;
			
		case 1:
			LCD_RS = LCD_RS_INSTR;
			LCD_DATA = (LCD_FUNC_SET | LCD_8BIT);
			LCD_ENABLE_STROBE();
			
			lcd_data.initStage = 2;
			
			tim_SetValue (lcd_data.timer, tim_GetValue(lcd_data.timer) + 200); // Wait for about 320us
			
			break;
		case 2:
			LCD_RS = LCD_RS_INSTR;
			LCD_DATA = (LCD_FUNC_SET | LCD_8BIT);
			LCD_ENABLE_STROBE();
			
			lcd_data.initStage = 3;
			
			tim_SetValue (lcd_data.timer, tim_GetValue(lcd_data.timer) + LCD_SHORT_DELAY);
			
			break;
		case 3:
			// 8 bit mode stablished.
			LCD_RS = LCD_RS_INSTR;
			LCD_DATA = (LCD_FUNC_SET | LCD_8BIT | LCD_2LINE | LCD_SMALL_FONT);
			LCD_ENABLE_STROBE();
			
			lcd_data.initStage = 4;

			tim_SetValue (lcd_data.timer, tim_GetValue(lcd_data.timer) + LCD_SHORT_DELAY);
			
			break;
		case 4:
			LCD_RS = LCD_RS_INSTR;
			LCD_DATA = (LCD_DISP_CTL | LCD_DISP_OFF);
			LCD_ENABLE_STROBE();
			
			lcd_data.initStage = 5;
			
			tim_SetValue (lcd_data.timer, tim_GetValue(lcd_data.timer) + LCD_SHORT_DELAY);
			
			break;
			
		case 5:
			LCD_RS = LCD_RS_INSTR;
			LCD_DATA = LCD_CLEAR_DISP;
			LCD_ENABLE_STROBE();
			
			lcd_data.initStage = 6;

			tim_SetValue (lcd_data.timer, tim_GetValue(lcd_data.timer) + LCD_LONG_DELAY);

			break;
			
		case 6:
			LCD_RS = LCD_RS_INSTR;
			LCD_DATA = (LCD_ENTRY_MODE | LCD_MOVE_DIR_UP | LCD_NO_SHIFT);
			LCD_ENABLE_STROBE();
			
			lcd_data.initStage = 7;
			
			tim_SetValue (lcd_data.timer, tim_GetValue(lcd_data.timer) + LCD_SHORT_DELAY);
			
			break;
			
		case 7:
			LCD_RS = LCD_RS_INSTR;
			LCD_DATA = (LCD_DISP_CTL | LCD_DISP_ON | LCD_CURSOR_ON | LCD_BLINK_ON);
			LCD_ENABLE_STROBE();
			
			lcd_data.initStage = 8;
			
			tim_SetValue (lcd_data.timer, tim_GetValue(lcd_data.timer) + LCD_SHORT_DELAY);
			
			break;
			
		case 8:
			tim_FreeTimer(lcd_data.timer);
			lcd_data.timer = tim_GetTimer (TIM_OC, lcd_PrintCallback, NULL, LCD_TIMER);
			tim_SetValue (lcd_data.timer, tim_GetGlobalValue() - LCD_SHORT_DELAY); 
			tim_ClearFlag(lcd_data.timer);
			tim_EnableInterrupts (lcd_data.timer);
			
			lcd_isInit = _TRUE;
			
			break;
	}
		
}
