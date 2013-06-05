/*
 * quad_control.c
 *
 */


#include <limits.h>
#include "quad_control.h"

/* todo esto hay que meterlo en arith.h */
/*
void xform4(frac (*M)[4][4], frac (*v)[4], frac (*res)[4])
{
	int i;

	for (i = 0; i < 4; i++) {
		int j;
		for (j = 0; j < 4; j++)
			res[i] += fmul(M[i][j], v[j]);
	}
}
*/
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

int int_SumSat2(int x1, int x2)
{
	int result = x1+x2;
	if (x1 > 0 && x2 > 0)
		return ((result < x1) ? INT_MAX : result);
	else if (x1 < 0 && x2 < 0)
		return ((result > x1) ? INT_MIN : result);
	else
		return result;
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

#define att_Kp 4000
#define att_Kd 8000

vec3 adv_att_control(quat setpoint, quat att)
{
	static vec3 err_prev = VEC0;
//	static dvec3 d_err_prev = VEC0;
	static vec3 int_error = VEC0;

	quat setp_c = qconj(setpoint);
	vec3 t_error = qmul(setp_c, att).v;
	vec3 torques;
	dvec3 d_err;
	
	/* ¿tenemos que hacer la derivada saturada?? */
//	d_err = dvsub(vfmul2(vsub(t_error, err_prev), att_Kd), d_err_prev);	// Bilineal
	d_err = vfmul2(vsub(t_error, err_prev), att_Kd);						// Backward
	
	
	torques = vec_clip_d(
			dvsum(vfmul2(t_error, att_Kp), d_err)
			);
			
			
	err_prev = t_error;
//	d_err_prev = d_err;
	
	//torques = vec_clip_d(vfmul2(t_error, att_Kp));
	
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
	
	output.speed[0] = 0;//gammainv(thrust, 0, torque.y, -torque.z);
	output.speed[1] = gammainv(thrust, -torque.x, 0, torque.z);
	output.speed[2] = 0;//gammainv(thrust, 0, -torque.y, -torque.z);
	output.speed[3] = gammainv(thrust, torque.x, 0, torque.z);
	
	return output;
}
