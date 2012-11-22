/* Tabla de servicios de la RTI
 *	Grupo 2 - 2012
 *	****** Codigo generado automaticamente. NO EDITAR ********
 */

#include "../common.h"
#include "rti2.h"
#include "../led_ll.h"
#include "../motor/mot_ll.h"

struct rti_srv rti_srv_tbl[] = {
/* LED_PWM */
	{0, 0, {1, 0}, 0, 8, 0, led_pwm, NULL},
/* SMOT */
	{0, 0, {2, 0}, 0, 8, 0, smot_srv, NULL}	
};

/* fin del codigo generado automaticamente, espero que no lo hayas editado.*/
