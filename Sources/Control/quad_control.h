#ifndef _QUAD_CONTROL_H_INCLUDED_
#define _QUAD_CONTROL_H_INCLUDED_

#include "arith.h"
#include "motors.h"

vec3 adv_att_control(quat setpoint, quat att);
frac h_control(frac setpoint, frac h);
struct motorData control_mixer(frac thrust, vec3 torque);




#endif // _QUAD_CONTROL_H_INCLUDED_