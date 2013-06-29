#ifndef _QUAD_CONTROL_H_INCLUDED_
#define _QUAD_CONTROL_H_INCLUDED_

#include "arith.h"
#include "motors.h"

typedef struct
{
	quat QEst;
	vec3 bff_angle_rate;
	vec3 torque;
	frac thrust;
}controlData_T;


vec3 adv_att_control(quat setpoint, quat att, vec3 angle_rate);
frac h_control(frac setpoint, frac h);
void control_mixer(frac thrust, vec3 torque, struct motorData* output);




#endif // _QUAD_CONTROL_H_INCLUDED_