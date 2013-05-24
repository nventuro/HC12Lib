/*
 * quad_control.c
 *
 */


#include <limits.h>
#include "arith.h"

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

#define att_pre_err_div 1
#define att_Kp_div 1
#define att_Kd_div 25

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

#define att_Kd_mul 1

vec3 adv_att_control(quat setpoint, quat att)
{
	static vec3 err_prev = VEC0;
	static vec3 int_error = VEC0;

	quat setp_c = qconj(setpoint);
	vec3 t_error = vdiv(qmul(setp_c, att).v, att_pre_err_div);
	vec3 torques;
	/* ¿tenemos que hacer la derivada saturada?? */
	torques = vsum(	vdiv(t_error, att_Kp_div),
			vmul(	vsub(t_error, err_prev),
				att_Kd_mul
			)
		);
	err_prev = t_error;

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

#define k_thrust_
#define k_roll_div
#define k_pitch_div
#define k_yaw_div
/*
struct vec4 control_mixer(frac thrust, vec3 torque)
{
	1;
}

int main(int argc, char **argv)
{

	return 0;
}
*/