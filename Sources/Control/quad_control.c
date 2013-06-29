/*
 * quad_control.c
 *
 */


#include <limits.h>
#include "quad_control.h"
#include <stdio.h>


controlData_T controlData = {{0,0,0,0}, {0, 0, 0}, {0,0,0}, 0};


#define ATT_DECIMATION_SIZE 4
#define ATT_D_FILTER_SIZE 8

#define prop_gain_frac 6500
//#define prop_gain_int
//#define prop_gain_divide 8000

#define int_gain_divide 8000

#define der_gain_int (1)
//#define der_gain_frac 2000
//#define der_gain_divide

#define integral_error_limit 300

vec3 adv_att_control(quat setpoint, quat att, vec3 angle_rate)
{
	static quat att_prev = UNIT_Q;
	static vec3 d_prev[ATT_D_FILTER_SIZE] = {VEC0};
	static int d_prev_i = 0;
	static vec3 error_sat_prev = VEC0;
	static evec3 integral_out_prev = VEC0;
		
	int i;
	
	//quat setp_c = qconj(setpoint);
	/* Hacia donde me tengo que mover para ir hacia el setpoint*/
	//vec3 error = qmul(setp_c, att).v;
	vec3 error = qerror2(setpoint, att);	
	/* Hacia donde me tengo que mover para volver a la posición anterior*/
	//vec3 damp = qmul(att_prev, qconj(att)).v;
/*	vec3 damp = qerror2(att_prev, att);
	evec3 damp_lp = VEC0;
*/
	vec3 torques = VEC0;
	evec3 ctrl_signal;
	
	vec3 error_sat = vsat(error, integral_error_limit);
	evec3 integral_out;
/*	
	d_prev[d_prev_i] = damp;
	d_prev_i++;
	if (d_prev_i >= ATT_D_FILTER_SIZE)
		d_prev_i = 0;
	
	for (i = 0; i < ATT_D_FILTER_SIZE; i++)
		damp_lp = dvsum(damp_lp, v_to_extended(d_prev[i]));
*/
	
	integral_out = dvsum(dvsum(v_to_extended(error_sat_prev), v_to_extended(error_sat)), integral_out_prev);
	
	integral_out_prev = integral_out;
	error_sat_prev = error_sat;
/*	
	ctrl_signal = dvsum(dvsum(
					v_to_extended(vdiv(error, c1_gain_divide)),
					dvdiv(evimul(damp_lp, c2_gain), ATT_D_FILTER_SIZE)),//vimul2(damp, c2_gain)
					dvdiv(integral_out, c1_int_gain_divide)
					);
*/
	ctrl_signal = dvsum(
						dvsub(
						#ifdef prop_gain_frac
							v_to_extended(vfmul(error, prop_gain_frac)),
						#elif (defined prop_gain_int) 
							vimul2(error, prop_gain_int),
						#elif (defined prop_gain_divide)
							v_to_extended(vdiv(error, prop_gain_divide)),
						#else 
							#error "Define gain for proportional control."
						
						#endif
							
						#ifdef der_gain_frac
						 	v_to_extended(vfmul(angle_rate, der_gain_frac))),
						#elif (defined der_gain_int)
						 	vimul2(angle_rate, der_gain_int)),
						#elif (defined der_gain_divide)
							v_to_extended(vdiv(angle_rate, der_gain_divide))),
						#else
							#error "Define gain for derivative control."
						#endif
					
					dvdiv(integral_out, int_gain_divide)
					);

	att_prev = att;
	
	torques = evclip(ctrl_signal);
	
	// FIXME: esto esta mal, es para poder probar sin que moleste el yaw 
	torques.z = 0;
	return torques;
}
/*
vec3 att_control_decimate(dvec3 torque_in)
{
	static dquat d_prev[ATT_DECIMATION_SIZE] = {VEC0};
	static d_prev_i = 0;
	
	int i;
	evec3 torques = VEC0;
	
	d_prev[d_prev_i] = torque_in;
	d_prev_i++;
	if (d_prev_i >= ATT_D_FILTER_SIZE)
		d_prev_i = 0;
	for (i = 0; i < ATT_DECIMATION_SIZE; i++)
		torques = dvsum(torques, d_prev[i]);
		
}
*/
#define h_Kp_mul 1
#define h_Kd_mul 10

frac h_control(frac setpoint, frac h)
{
	static frac err_prev = 0;
	frac h_error = setpoint - h, thrust;

	thrust = h_error*h_Kp_mul + (h_error - err_prev)*h_Kd_mul;
	err_prev = h_error;

	return thrust;
}

#define mix_thrust_shift 0
#define	mix_roll_shift_r 0
#define mix_pitch_shift_r 0
#define mix_yaw_shift 3

frac gammainv(frac T, frac t1, frac t2, frac t3)
{
	dfrac r = 0;
	
	r += (((dfrac)T) << mix_thrust_shift);
	r += (((dfrac)t1) >> mix_roll_shift_r);
	r += (((dfrac)t2) >> mix_pitch_shift_r);
	r += (((dfrac)t3) << mix_yaw_shift);
	
	return fsqrt((r > 0)? ((r < FRAC_1)? r : FRAC_1) : 0);
	//return ((r > 0)? ((r < FRAC_1)? r : FRAC_1) : 0);
}				

void control_mixer(frac thrust, vec3 torque, struct motorData* output)
{
	// 0 y 2: brazo rojo.
	
	output->speed[0] = gammainv(thrust, 0, torque.y, -torque.z);
	output->speed[1] = 0;//gammainv(thrust, -torque.x, 0, torque.z);
	output->speed[2] = gammainv(thrust, 0, -torque.y, -torque.z);
	output->speed[3] = 0;//gammainv(thrust, torque.x, 0, torque.z);
	
	return;
}
