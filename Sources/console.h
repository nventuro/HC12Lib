/*
 * console.h
 * 
 * Grupo 2 - 2012
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "common.h"
#include "rti/rtix.h"

/* display control */

enum {CON_NORMAL, CON_PASSWD};
enum {ATT_DIM_BIT, ATT_DOT_BIT, ATT_BLINK_BIT};
enum {ATT_SET, ATT_ADD, ATT_CLEAR};

#define ATT_DIM (1<<ATT_DIM_BIT)
#define ATT_DOT (1<<ATT_DOT_BIT)
#define ATT_BLINK (1<<ATT_BLINK_BIT)
#define CON_TIMEOUT	NULL

/* input control*/

enum {BLOCK_INPUT, ALLOW_INPUT};

/* funciones */

void con_init(void);
void con_clear(void);
void con_input_enable(int v);
	/* BLOCK_INPUT o ALLOW_INPUT */
void con_output_mode(int m);
	/* CON_NORMAL o CON_PASSWD */
void con_setattr(u8 attrs, int mode);

int con_putchar(char c);

int con_puts(char *s);

int con_getchar_timed(rti_time timeout);

int con_getchar(void);

char *con_gets_timed(char *s, rti_time timeout);

char *con_gets(char *s);

void con_cursor_enable(int v);

int con_nchars(void);

#endif /* __CONSOLE_H__ */
