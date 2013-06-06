/*
 * quad_control.c
 *
 */


#include <limits.h>
#include "quad_control.h"
#include <stdio.h>

/*
#define att_pre_err_div 1
#define att_Kp_div 1
#define att_Kd_div 25
*/
/*
typedef struct { int min, med, max;} triplet;

triplet int_Order3(int a, int b, int c)
{
    triplet retVal;
    if (a < b)
    {
        if (a < c)
        {
            if(b < c)
            {
                retVal.min = a;
                retVal.med = b;
                retVal.max = c;
            }
            else
            {
                retVal.min = a;
                retVal.med = c;
                retVal.max = b;
            }
        }
        else
        {
            retVal.min = c;
            retVal.med = a;
            retVal.max = b;
        }
    }
    else
    {
        if (b < c)
        {
            if (a < c)
            {
                retVal.min = b;
                retVal.med = a;
                retVal.max = c;
            }
            else
            {
                retVal.min = b;
                retVal.med = c;
                retVal.max = a;
            }
        }
        else
        {
            retVal.min = c;
            retVal.med = b;
            retVal.max = a;
        }
    }

    return retVal;
}

int min3(int a, int b, int c)
{
	int min_ab = (a < b)? a: b;
	return (min_ab < c)? min_ab : c;
}


int int_SumSat3(int a, int b, int c)
{
    triplet ordVec = int_Order3(a,b,c);

    if ((ordVec.max = int_SumSat2(ordVec.min, ordVec.max)) == INT_MAX)
        return INT_MAX;
    else
        return int_SumSat2(ordVec.max, ordVec.med);
}
*/

#define att_Kp 0
#define att_Kd 0
#define att_Ki_shift 7
#define INT_DIV 8

vec3 adv_att_control(quat setpoint, quat att)
{
	static vec3 err_prev = VEC0;
	static dvec3 int_acc = VEC0;

	quat setp_c = qconj(setpoint);
	vec3 error = qmul(setp_c, att).v;
	vec3 torques;
	dvec3 d_out;
	dvec3 p_out;
	dvec3 i_out;
	
	/* Â¿tenemos que hacer la derivada saturada?? */
	// El italiano hace la derivada de la salida, no del error entre la salida y el setpoint.
	// Además, usa PID con integral saturada.
	// Dice de limitar en banda la derivada, pero para que no se produzcan impulsos (lo cual no es el caso)
	
	d_out = vfmul2(vsub(error, err_prev), att_Kd);						// Backward
	p_out = vfmul2(error, att_Kp);
//	i_out = vfmul2(vec_clip_d(dvsum(dvsum(int_prev, fexpand(error)),fexpand(err_prev)))), att_Ki);
	
	int_acc = dvsumsat(int_acc, dvec_lShift(dvsum(vexpand(err_prev),vexpand(error)), att_Ki_shift) );
	i_out = dvec_rShift(int_acc, INT_DIV);
	printf("%d, %d, %d\n",error.x, error.y, error.z);

	torques = vec_clip_d(dvsumsat(dvsum(p_out,d_out),i_out));
			
	err_prev = error;
	
	torques.z = 0;
	return torques;
}

#define h_Kp_mul 1
#define h_Kd_mul 17

frac h_control(frac setpoint, frac h)
{
	static frac err_prev = 0;
	frac h_error = setpoint - h, thrust;

	thrust = h_error*h_Kp_mul + (h_error - err_prev)*h_Kd_mul;
	err_prev = h_error;

	return thrust;
}

#define mix_thrust_shift 0
#define	mix_roll_shift 3
#define mix_pitch_shift 3
#define mix_yaw_shift 3

frac gammainv(frac T, frac t1, frac t2, frac t3)
{
	dfrac r = 0;
	
	r += (((dfrac)T) << mix_thrust_shift);
	r += (((dfrac)t1) << mix_roll_shift);
	r += (((dfrac)t2) << mix_pitch_shift);
	r += (((dfrac)t3) << mix_yaw_shift);
	
	return fsqrt((r > 0)? ((r < FRAC_1)? r : FRAC_1) : 0);
}				

struct motorData control_mixer(frac thrust, vec3 torque)
{
	struct motorData output;
	//VER SIGNO; justo el 0 y el 2 deberían estar al revés con los torques en y.
	output.speed[0] = 0;//gammainv(thrust, 0, torque.y, -torque.z);
	output.speed[1] = gammainv(thrust, -torque.x, 0, torque.z);
	output.speed[2] = 0;//gammainv(thrust, 0, -torque.y, -torque.z);
	output.speed[3] = gammainv(thrust, torque.x, 0, torque.z);
	
	return output;
}
