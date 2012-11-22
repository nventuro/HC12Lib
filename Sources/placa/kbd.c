/*
 * kbd.c
 * 
 */

#include <string.h>
#include "../common.h"
#include "../rti/rtix.h"
#include "../util/cb.h"
#include "placa.h"
#include "kbd.h"

#define POLL_INTERVAL RTI_MS2PERIOD(100)
#define SETTLING_TIME 1

#define N_ROWS 4
#define N_COLS 3

#define KB_BUF_LEN 32

static u8 kb_buf[KB_BUF_LEN];
static cbuf kb_cb;

static const char kb_xlate[N_ROWS][N_COLS] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{KB_SPECIAL_L, '0', KB_SPECIAL_R}};

static char kb_kstatus[N_ROWS][N_COLS];
int kb_active;
char lastkey;

rti_time kb_poll(void *dummy, rti_time pw);
void kb_reserve_disp_port (u8 *DATA,u8 *CTL);
void kb_restore_disp_port (u8 DATA,u8 CTL);

void kb_init(void)
{
	KB_DD |= mKB_WRITE;
	KB_DD &= ~mKB_READ;
	
	kb_cb = ARRAY2CB(kb_buf);
	kb_active = 0;
	
	rti_register(kb_poll, NULL, RTI_MS2PERIOD(100), RTI_NOW);
}

int kb_status() 
{
	return cb_status(&kb_cb);
}

int kb_read()
{
	return cb_pop(&kb_cb);
}

void kb_flush ()
{
	cb_flush(&kb_cb);
}

static void push_key(int i, int j)
{
	lastkey = kb_xlate[i][j];
	cb_push(&kb_cb, kb_xlate[i][j]);
}

void kb_activate(int n)
 {
	 kb_active = (n);
 }
 
 
rti_time kb_poll(void *data, rti_time pw)
{
	int i, j;
	u8 temp_DISP_DATA, temp_DISP_CTL;
	
	if (!kb_active)
		goto poll_end;
	
	kb_reserve_disp_port(&temp_DISP_DATA,&temp_DISP_CTL);
	
	for (i = 0; i < N_ROWS; i++) {
		u8 line;
		
		KB_PROBE(i);
		__asm NOP; //Compensan el retardo de ~50ns entre
		__asm NOP; //la salida del PORTB y el pulldown 
		__asm NOP; //del teclado.
		__asm NOP;
		__asm NOP;
		line = KB_READ();
		
		for (j = 0; j < N_COLS; j++) {
			u8 b = BIT(line, j);
				
			if (b && !kb_kstatus[i][j])
				push_key(i,j);
			
			kb_kstatus[i][j] = b;
		}
	}
	
	kb_restore_disp_port(temp_DISP_DATA,temp_DISP_CTL);

poll_end:	
	return pw;
}

void kb_reserve_disp_port (u8 *DATA,u8 *CTL) 
{
	*DATA = DISP_DATA_PORT;
	*CTL = DISP_CTL_PORT;
	DISP_DATA_PORT = 0x00;
}

void kb_restore_disp_port(u8 DATA, u8 CTL)
{
	DISP_CTL_PORT = CTL;
	DISP_DATA_PORT = DATA;
}
