#ifndef _RTI_H
#define _RTI_H

#include "common.h"

#define RTI_FREQ 781 // Hz
#define RTI_PER (1.0/RTI_FREQ) // seconds


#define RTI_ALWAYS 1
#define RTI_ONCE 0
#define RTI_CANCEL 0
#define RTI_AUTOCANCEL 0
#define RTI_NOW 1
#define RTI_INVALID_ID (-1)

#define RTI_MS2PERIOD(ms) (DIV_CEIL( (((u32)ms) > 0 ? ms : 0) * RTI_FREQ, 1000 ))

enum {RTI_NORMAL, RTI_PROTECT};
#define RTI_DEFAULT_PROTECT RTI_NORMAL

typedef u16 rti_time;
typedef s8 rti_id;
typedef rti_time (*rti_ptr) (void *data, rti_time period);

void rti_init(void);

rti_id rti_register(rti_ptr callback, void *data, rti_time period, rti_time delay);
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

rti_id rti_register2(rti_ptr callback, void *data, rti_time period, rti_time delay, s8 protect);
	/* Esta función es como rti_register, con la diferencia que, en caso de
	 * auto-cancelación, si 'protect' está en RTI_PROTECT, el timer queda en
	 * la tabla (inactivo) hasta que el usuario le da un rti_cancel
	 */
	
void rti_set_period(rti_id id, rti_time period);

	
void rti_cancel(rti_id n);
	/* Cancelar un timer que ya está cancelado nos es seguro
	 * La excepción es si fue una auto-cancelación y el timer tenía el
	 * atributo 'protect' en RTI_PROTECT */

extern void interrupt rti_srv(void);


#endif
