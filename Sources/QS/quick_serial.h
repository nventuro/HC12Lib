/*
 * quick_serial.h
 * 
 */

#ifndef __QS_H__
#define __QS_H__

#include "common.h"

#define BRATE_NO_CHANGE 0
#define MON12X_BR 115200

enum {QS_LOOP_NONE, QS_LOOP_INT, QS_LOOP_EXT};

void qs_init(int mod, u32 brate);

void qs_loop(int mod, int loop_mode);

void qs_wait_ready(int mod);
void qs_tx(int mod, char c);
void qs_putchar(int mod, char c);

void qs_wait_data(int mod);
char qs_rx(int mod);
char qs_getchar(int mod);

void qs_write(int mod, char *buf, unsigned int len);

#endif /* __QS_H__ */
