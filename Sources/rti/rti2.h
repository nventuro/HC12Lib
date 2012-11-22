/* rti2.h
 */

#ifndef __RTI_H__
#define __RTI_H__

#include "common.h"

#define RTI_COMPAT

#define RTI_FREQ 781 /*Hz*/
#define RTI_PER (1.0/RTI_FREQ) /*ms */

#ifdef RTI_COMPAT

#define RTI_ALWAYS 1
#define RTI_ONCE 0
#define RTI_CANCEL 0
#define RTI_AUTOCANCEL 0
#define RTI_NOW 1
#define RTI_INVALID_ID (-1)

#endif /* RTI_COMPAT */

#define RTI_MS2PERIOD(ms) ( DIVUP(((long long)ms)*RTI_FREQ, 1000))

#ifdef RTI_COMPAT

enum {RTI_NORMAL, RTI_PROTECT};
#define RTI_DEFAULT_PROTECT RTI_NORMAL

#endif /* RTI_COMPAT */

typedef uint rti_time ;
typedef int timer_id;
typedef int rti_srvid;

struct rti_timer {
	rti_time period;
	rti_time cnt;
};

struct rti_srv {
	int enabled;
	int stop_req;
	struct rti_timer osc;
	int ramp_cnt;
	int ramp_steps;
	int ramp_ths;
	
	int (*f)(void *, int);
	void *data;
};

void rti_init(void);

void rti_enable(rti_srvid s, int v);
void rti_runctl(rti_srvid s, int v);
void rti_set_period(rti_srvid s, int pw);

#ifdef RTI_COMPAT

timer_id rti_register(rti_time (*f)(void *, rti_time), void *data, 
					rti_time period, rti_time delay);
	/* Registra una función 'f' para que se corra a partir del un retardo
	 * inicial 'delay', y a partir de alli, cada 'period'.
	 * Period puede ser un tiempo MAYOR QUE CERO, RTI_ALWAYS para que se
	 * ejecute siempre, o RTI_ONCE para que se llame 1 vez y se cancele 
	 * (un "monoestable")
	 * Delay puede ser un tiempo MAYOR QUE CERO, o RTI_NOW para que se
	 * ejecute cuanto antes.
	 * Cuando se llama a 'f' se le pasa:
	 * 	void *data' : puntero a un dato arbitrario del usuario
	 * 	rti_time period: el tiempo desde la última llamada (el periodo 
	 * 			actual, o el delay inicial (p/ la 1era llamada)
	 * El valor que devuelve es el tiempo en el que se la debe llamar de 
	 * vuelta. Casos comunes son:
	 * 	Devolver el mismo rti_time que entró: persiste con el mismo 
	 * 	periodo (o si el usuario puso RTI_ONCE, lo respeta)
	 * 	Devolver RTI_CANCEL, para cancelarse.
	 */

timer_id rti_register2(rti_time (*f)(void *, rti_time), void *data, 
				rti_time period, rti_time delay, int protect);
	/* Esta función es como rti_register, con la diferencia que, en caso de
	 * auto-cancelación, si 'protect' está en RTI_PROTECT, el timer queda en
	 * la tabla (inactivo) hasta que el usuario le da un rti_cancel
	 */
/*	
rti_time rti_set_period(int n, rti_time period); */
	/* n : id del timer, period: nuevo periodo */
	
void rti_cancel(timer_id n);
	/* Cancelar un timer que ya está cancelado nos es seguro
	 * La excepción es si fue una auto-cancelación y el timer tenía el
	 * atributo 'protect' en RTI_PROTECT */

#endif /* RTI_COMPAT */

extern void interrupt rti_srv(void);

#ifdef RTI_COMPAT

rti_time flagger(void *flag, rti_time pw);
	/* levata un flag despues de tanto tiempo (lo pone en 1)
	 * Ejemplo rti_register(flagger, &flag, RTI_ONCE, DELAY);
	 */
#endif /* RTI_COMPAT */

#include "rti_srvs.h"

#endif /* __RTI_H__ */
