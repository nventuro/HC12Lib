/*
 * kbd.h
 * 
 * Grupo 2 - 2012
 */

#ifndef __KB_H__
#define __KB_H__

#define KB_SPECIAL_L 'A'
#define KB_SPECIAL_R 'B'

#define KB_ACTIVE	1

extern int kb_active;

void kb_init(void);

void kb_activate(int n);

int kb_status(void);

int kb_read(void);

void kb_flush(void);

#endif /* __KB_H__ */
