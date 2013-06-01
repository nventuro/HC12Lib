#include "fjoy_disp.h"
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

#define BAR_DIVIDER (255/BAR_LEN)

// 20x4 display (LCD_2004)
#define COLS 20
#define ROWS 4

#define YAW_ROW 0
#define PITCH_ROW 1
#define ROLL_ROW 2
#define ELEV_ROW 3

void InitRows (void);
void u8ToChar (u8 x, char *c);
void s8ToChar (s8 x, char *c);
void u8ToBar (u8 x, char *c);
void s8ToBar (s8 x, char *c);
u8 StrLen (char *c);

char rows[ROWS][COLS];
char *staticRows[] = {"Yaw:", "Pitch:", "Roll:", "Elev:"};
char auxStr[VALUE_LEN]; // Used for converting numbers to strings

bool init = _FALSE;


void fjoy_PrintAxes (void)
{
	u8 i, len;
	
	if (init == _FALSE)
	{
		init = _TRUE;
		InitRows();		
	}
	
	
	// Yaw
	s8ToChar(fjoy_status.yaw,auxStr);
	len = StrLen(auxStr);
	for (i = 0; i < VALUE_LEN; i++)
	{
		if (i < (VALUE_LEN - len))
			rows[YAW_ROW][i+AX_LEN] = ' ';
		else
			rows[YAW_ROW][i+AX_LEN] = auxStr[i-(VALUE_LEN-len)];
	}
	rows[YAW_ROW][i+AX_LEN] = ' '; // Trailing whitespace


	s8ToBar(fjoy_status.yaw,rows[YAW_ROW]+BAR_START_POS);
	lcd_PrintRow(rows[YAW_ROW],YAW_ROW);
	
	
	// Pitch
	s8ToChar(fjoy_status.pitch,auxStr);
	len = StrLen(auxStr);
	for (i = 0; i < VALUE_LEN; i++)
	{
		if (i < (VALUE_LEN - len))
			rows[PITCH_ROW][i+AX_LEN] = ' ';
		else
			rows[PITCH_ROW][i+AX_LEN] = auxStr[i-(VALUE_LEN-len)];
	}
	rows[PITCH_ROW][i+AX_LEN] = ' '; // Trailing whitespace


	s8ToBar(fjoy_status.pitch,rows[PITCH_ROW]+BAR_START_POS);
	lcd_PrintRow(rows[PITCH_ROW],PITCH_ROW);
	
	
	// Roll
	s8ToChar(fjoy_status.roll,auxStr);
	len = StrLen(auxStr);
	for (i = 0; i < VALUE_LEN; i++)
	{
		if (i < (VALUE_LEN - len))
			rows[ROLL_ROW][i+AX_LEN] = ' ';
		else
			rows[ROLL_ROW][i+AX_LEN] = auxStr[i-(VALUE_LEN-len)];
	}
	rows[ROLL_ROW][i+AX_LEN] = ' '; // Trailing whitespace


	s8ToBar(fjoy_status.roll,rows[ROLL_ROW]+BAR_START_POS);
	lcd_PrintRow(rows[ROLL_ROW],ROLL_ROW);
	
	
	// Elevation
	u8ToChar(fjoy_status.elev,auxStr);
	len = StrLen(auxStr);
	for (i = 0; i < VALUE_LEN; i++)
	{
		if (i < (VALUE_LEN - len))
			rows[ELEV_ROW][i+AX_LEN] = ' ';
		else
			rows[ELEV_ROW][i+AX_LEN] = auxStr[i-(VALUE_LEN-len)];
	}
	rows[ELEV_ROW][i+AX_LEN] = ' '; // Trailing whitespace


	u8ToBar(fjoy_status.elev,rows[ELEV_ROW]+BAR_START_POS);
	lcd_PrintRow(rows[ELEV_ROW],ELEV_ROW);
}

void InitRows (void)
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