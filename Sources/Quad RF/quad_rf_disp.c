#include "quad_rf_disp.h"
#include "nRF24L01+.h"
#include "fjoy.h"
#include "lcd.h"

// Format is:
// Ax:  val [---|---]
// The "Ax:" string has a length of up to AX_LEN
// The val string (not including the trailing whitespace) has a length of up to VALUE_LEN
// The bar is always of len BAR_LEN, and has two brackets: one at the beginning and another at the end
// The bar starts at BAR_START_POS (name, value and the trailing whitespace)
// Empty spaces are completed with whitespaces
#define AX_LEN 6
#define VALUE_LEN 4
#define BAR_START_POS (AX_LEN+VALUE_LEN+1)
#define BAR_LEN 7

#define BAR_DIVIDER(bits) ((POW_2(bits)-1)/BAR_LEN)

#define DISPLAY LCD_2004
#define DISPLAY_ROWS GLUE(DISPLAY,_ROWS)
#define DISPLAY_COLS GLUE(DISPLAY,_COLS)

#define YAW_ROW 0
#define PITCH_ROW 1
#define ROLL_ROW 2
#define ELEV_ROW 3

void PrintAxesNames (void);
void PrintAxesMeasurements (void);
void PrintAxisMeasurements (s8 meas, u8 row, bool isSigned);
void PrintBars (void);
void PrintBar (s8 meas, u8 row, bool isSigned, u8 bits);

void u8ToChar (u8 x, char *c);
void s8ToChar (s8 x, char *c);
void u8ToBar (u8 x, char *c, u8 bits);
void s8ToBar (s8 x, char *c, u8 bits);
u8 StrLen (char *c);

char *axesNames[] = {"Yaw:", "Pitch:", "Roll:", "Elev:"};
char *battString = "Battery ";
char auxStr[VALUE_LEN]; // Used for converting numbers to strings

void qrf_disp_PrintAxes (void)
{
	PrintAxesNames();
	PrintAxesMeasurements();
	PrintBars();
}

void PrintAxesNames (void)
{
	u8 i, j;
	for (i = 0; i < DISPLAY_ROWS; i ++)
	{
		for (j = 0; j < DISPLAY_COLS; j++)
			if (axesNames[i][j] == '\0')
				break;
			else
				lcd_memory[i*DISPLAY_COLS + j] = axesNames[i][j];
			
		while (j < DISPLAY_COLS)
			lcd_memory[i*DISPLAY_COLS + j++] = ' ';
	}
	
	return;
}

void PrintAxesMeasurements (void)
{
	PrintAxisMeasurements (fjoy_status.yaw, YAW_ROW, _TRUE);
	PrintAxisMeasurements (fjoy_status.pitch, PITCH_ROW, _TRUE);
	PrintAxisMeasurements (fjoy_status.roll, ROLL_ROW, _TRUE);
	PrintAxisMeasurements (fjoy_status.elev, ELEV_ROW, _FALSE);
}

void PrintAxisMeasurements (s8 meas, u8 row, bool isSigned)
{
	u8 i, len;
	
	if (isSigned == _TRUE)
		s8ToChar(meas,auxStr);
	else
		u8ToChar((u8)meas,auxStr);
	
	len = StrLen(auxStr);
	for (i = 0; i < VALUE_LEN; i++)
	{
		if (i < (VALUE_LEN - len))
			lcd_memory[row*DISPLAY_COLS + i + AX_LEN] = ' ';
		else
			lcd_memory[row*DISPLAY_COLS + i + AX_LEN] = auxStr[i-(VALUE_LEN-len)];
	}
	lcd_memory[row*DISPLAY_COLS + i + AX_LEN] = ' '; // Trailing whitespace	
}

void PrintBars (void)
{
	PrintBar (fjoy_status.yaw, YAW_ROW, _TRUE, FJOY_YAW_BITS);
	PrintBar (fjoy_status.pitch, PITCH_ROW, _TRUE, FJOY_PITCH_BITS);
	PrintBar (fjoy_status.roll, ROLL_ROW, _TRUE, FJOY_ROLL_BITS);
	PrintBar (fjoy_status.elev, ELEV_ROW, _FALSE, FJOY_ELEV_BITS);
}

void PrintBar (s8 meas, u8 row, bool isSigned, u8 bits)
{
	if (isSigned == _TRUE)
		s8ToBar(meas,lcd_memory + row*DISPLAY_COLS + BAR_START_POS, bits);	
	else
		u8ToBar((u8)meas,lcd_memory + row*DISPLAY_COLS + BAR_START_POS, bits);	
}

/*void qrf_disp_PrintCommAndBatt (u16 lostPacketsCount, u8 battALevel, u8 battBLevel)
{	
	u8 i, j;
	for (i = 0; i < DISPLAY_ROWS; i++)
	{
		if ((i == 0) || (i == 1))
			
	}
}*/

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

void u8ToBar (u8 x, char *c, u8 bits)
{
	u8 i;
	u8 notch = x / (BAR_DIVIDER(bits) + 1);
	
	for (i = 0; i < BAR_LEN; i++)
		if (i == notch)
			c[i+1] = '|';
		else
			c[i+1] = '-';
	
	c[0] = '[';
	c[BAR_LEN + 1] = ']';
}

void s8ToBar (s8 x, char *c, u8 bits)
{
	s16 aux = x + POW_2(bits-1);
	
	u8ToBar((u8) aux, c, bits);
}