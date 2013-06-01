#include "fjoy_disp.h"
#include "lcd.h"
#include <stdio.h>

#define BAR_LEN 7
#define BAR_DIVIDER (255/BAR_LEN)

// 20x4 display (LCD_2004)
#define COLS 20
#define ROWS 4
s8 yaw = -128;

void u8ToChar (u8 x, char *c);
void s8ToChar (s8 x, char *c);
void u8ToBar (u8 x, char *c);
void s8ToBar (s8 x, char *c);
u8 StrLen (char *c);

char rows[ROWS][COLS];
char *staticRows[] = {"Yaw:", "Pitch:", "Roll:", "Elev:"};
char auxStr[5]; // Used for converting numbers to strings

void fjoy_InitRows (void)
{
	u8 i, j;
	for (i = 0; i < ROWS; i ++)
	{
		for (j = 0; j < COLS; j++)
			if (staticRows[i][j] == '\0')
				break;
			else
				rows[i][j] = staticRows[i][j];
			
		while (j < COLS)
			rows[i][j++] = ' ';
	}
	
	fjoy_PrintAxes();		
}

void fjoy_PrintAxes (void)
{
	u8 i, len;
	
	s8ToChar(yaw,auxStr);
	len = StrLen(auxStr);
	for (i = 6; i < (6+5-1); i++)
	{
		if ((i-6) < (4 - len))
			rows[0][i] = ' ';
		else
			rows[0][i] = auxStr[i-6+len-4];
	}


	s8ToBar(yaw,rows[0]+11);
	lcd_PrintRow(rows[0],0);
	
	/*lcd_PrintRow("Yaw:  -128 [|------]",0);
	lcd_PrintRow("Pitch:-120 [-|-----]",1);
	lcd_PrintRow("Roll: -105 [--|----]",2);
	lcd_PrintRow("Elev:  128 [---|---]",3);*/
}


void u8ToChar (u8 x, char *c)
{
	u8 i = 0;
	
	if (x >= 100)
		c[i++] = x / 100 + '0';
	
	if (x >= 10)
		c[i++] = (x % 100) / 10 + '0';
	
	c[i++] = (x % 10) + '0';
	
	
	c[i] = '\0';
}

void s8ToChar (s8 x, char *c)
{
	if (x < 0)
	{
		c[0] = '-';
		u8ToChar((u8) (-x), c+1);
	}
	else
		u8ToChar((u8) x, c);
}

u8 StrLen (char *c)
{	
	u8 i = 0;
	while (c[i] != '\0')
		i++;
	
	return i;
}

void u8ToBar (u8 x, char *c)
{
	u8 i;
	u8 notch = x / BAR_DIVIDER;
	
	for (i = 0; i < BAR_LEN; i++)
		if (i == notch)
			c[i+1] = '|';
		else
			c[i+1] = '-';
	
	c[0] = '[';
	c[BAR_LEN + 1] = ']';
}

void s8ToBar (s8 x, char *c)
{
	s16 aux = x + 128;
	
	u8ToBar((u8) aux, c);
}