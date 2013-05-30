/*
 * nlcf.h
 *
 * Non-linear complementary filter
 */

#ifndef __NLCF_H__
#define __NLCF_H__

#include "arith.h"

enum CAL_QUALITY {CAL_EXCELLENT, CAL_GOOD, CAL_UGLY, CAL_BAD};

quat att_estim(vec3 gyro, vec3 accel);

struct qpair {
	quat p0, p1;
};

struct cal_output {
	u8 quality;
	quat correction;
};

struct qpair _calibrate(quat mes0, quat mes1);

struct cal_output att_calibrate(quat mes0, quat mes1);

void att_apply_correction(struct cal_output c);

#endif /*__NLCF_H__*/
