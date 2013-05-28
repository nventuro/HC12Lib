/*
 * nlcf.h
 *
 * Non-linear complementary filter
 */

#ifndef __NLCF_H__
#define __NLCF_H__

#include "arith.h"

quat att_estim(vec3 gyro, vec3 accel);

struct qpair {
	quat p0, p1;
};

struct qpair calibrate(quat mes0, quat mes1);

#endif /*__NLCF_H__*/
