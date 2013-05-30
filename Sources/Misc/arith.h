/* **** Fixed point arithmethic ***** */


#ifndef _ARITH_H_
#define _ARITH_H_

#include "common.h"

/* 1.15 */
typedef s16 frac;
/* 2.30 */
typedef s32 dfrac;

#define FRAC_BIT (16)
#define DFRAC_BIT (32)

frac fsqrt(frac b);

#define FRAC_1 S16_MAX /* 0.999 */
#define FRAC_0_5 (-(S16_MIN>>1)) /* 0.5 */
#define FRAC_minus1 S16_MIN

#define DFRAC_1 (-(S32_MIN>>1))  /* 1 */
#define DFRAC_0_5 (-(S32_MIN>>2))  /* .5 */
#define DFRAC_almost1 (-(S32_MIN>>1)-1) /* 0.999 */
#define DFRAC_minus1 (S32_MIN>>1) /* -1 */

#define FRAC2DBL(n) (((double)(n))/(-(double)FRAC_minus1))
#define F_TO_FRAC(f) ((frac)(-f*FRAC_minus1))

static frac fmul(frac a, frac b)
{ /* 1.15 x 1.15 = 1.15 */
	return ((((dfrac)a) * b) << 1) >> FRAC_BIT;
}

static dfrac fmul2(frac a, frac b)
{ /* 1.15 x 1.15 = 2.30 */
	return (((dfrac)a) * b);
}

static frac dtrunc(dfrac x)
{ /* 2.30 -> 1.15 */
	if (x >= DFRAC_1)
		return FRAC_1;
	else if ( x < DFRAC_minus1)
		return FRAC_minus1;
	else
		return (x << 1) >> FRAC_BIT;
}

static dfrac fexpand(frac x)
{ /* 2.30 -> 1.15 */
	return ((dfrac)x)<<(FRAC_BIT - 1);
}

/* **** Vectors in R^3 ****/

typedef struct {
	frac x,y,z;
} vec3;

typedef struct {
	dfrac x,y,z;
} dvec3;


#if (FRAC_BIT == 16)
typedef vec3 s16Vec3;
#endif

#if (DFRAC_BIT == 32)
typedef dvec3 s32Vec3;
#endif

#define VEC0 {0,0,0}

enum {AXIS_X, AXIS_Y, AXIS_Z};

static void vec_AddInPlace(s16Vec3* This, s16Vec3* b)
{
	This->x += b->x;
	This->y += b->y;
	This->z += b->z;

	return;
}

static void vec_SubInPlace(s16Vec3* This, s16Vec3* b)
{
	This->x -= b->x;
	This->y -= b->y;
	This->z -= b->z;

	return;
}


static void dVec_dAddInPlace(s32Vec3* This, s32Vec3* b)
{
	This->x += b->x;
	This->y += b->y;
	This->z += b->z;

	return;
}


static void dVec_AddInPlace(s32Vec3* This, s16Vec3* b)
{
	This->x += b->x;
	This->y += b->y;
	This->z += b->z;

	return;
}

static void vec_MulInPlace(s16Vec3* This, s16 b)
{
	This->x *= b;
	This->y *= b;
	This->z *= b;

	return;
}


static void dVec_MulInPlace(s32Vec3* This, s16 b)
{
	This->x *= b;
	This->y *= b;
	This->z *= b;

	return;
}


static void vec_DivInPlace(s16Vec3* This, s16 b)
{
	This->x /= b;
	This->y /= b;
	This->z /= b;

	return;
}


static void dVec_DivInPlace(s32Vec3* This, s16 b)
{
	This->x /= b;
	This->y /= b;
	This->z /= b;

	return;
}

static dvec3 dvec_Add(dvec3 a, dvec3 b)
{
	dvec3 c = a;

	dVec_dAddInPlace(&c, &b);

	return c;
}

static vec3 vec_Add(vec3 a, vec3 b)
{
	vec3 c = a;

	vec_AddInPlace(&c, &b);

	return c;
}

static vec3 vec_Sub(vec3 a, vec3 b)
{
	vec3 c = a;

	vec_SubInPlace(&c, &b);

	return c;
}

static vec3 vec_Div(vec3 a, int f)
{
	vec3 c = a;

	vec_DivInPlace(&c,f);

	return c;
}

static vec3 vec_Mul(vec3 a, int f)
{
	vec3 c = a;

	vec_MulInPlace(&c, f);

	return c;
}

static dvec3 vfmul2(vec3 a, frac f)
{
	dvec3 r;
	
	r.x = fmul2(a.x, f);
	r.y = fmul2(a.y, f);
	r.z = fmul2(a.z, f);
	
	return r;
}

static vec3 vec_clip_d(dvec3 a)
{
	vec3 r;
#define _DFRAC_CLIP(k) (((k) >= DFRAC_minus1)? (((k) < DFRAC_1)? dtrunc(k): FRAC_1) : FRAC_minus1)
	r.x = _DFRAC_CLIP(a.x);
	r.y = _DFRAC_CLIP(a.y);
	r.z = _DFRAC_CLIP(a.z);
	
	return r;
}

#define vdiv vec_Div
#define vsum vec_Add
#define vsub vec_Sub
#define vmul vec_Mul
#define dvsum dvec_Add

/* **** Quaternions ****/

typedef struct {
	frac r;
	vec3 v;
} quat;

typedef struct {
	dfrac r;
	dvec3 v;
} dquat;

#define UNIT_Q {FRAC_1, {0, 0, 0}}
#define UNIT_DQ {DFRAC_1, {0, 0, 0}}

static  quat qtrunc(dquat q)
{
	quat s;
#define _QTR(e) s.e = dtrunc(q.e)

	_QTR(r);
	_QTR(v.x);
	_QTR(v.y);
	_QTR(v.z);

	return s;
}

static  frac q_normerror(quat q)
{ /* Pseudo-error de la norma:
   *	calcula una aproximación a (1 - norm(q))
   *	La aproxiimación está en decir que sqrt(x) = 0.5(x + 1)
   */
	return FRAC_0_5 - dtrunc((fmul2(q.r, q.r) + fmul2(q.v.x, q.v.x) +
			fmul2(q.v.y, q.v.y) + fmul2(q.v.z, q.v.z))/2);
}

static  dquat qscale2(quat q, frac f)
{
	dquat s;

	s.r = fmul2(q.r, f);
	s.v.x = fmul2(q.v.x, f);
	s.v.y = fmul2(q.v.y, f);
	s.v.z = fmul2(q.v.z, f);

	return s;
}

static  quat qscale(quat q, frac f)
{
	quat s;

	s.r = fmul(q.r, f);
	s.v.x = fmul(q.v.x, f);
	s.v.y = fmul(q.v.y, f);
	s.v.z = fmul(q.v.z, f);

	return s;
}

static quat qconj(quat q)
{
	quat p;

	p.r = q.r;
	p.v.x = -q.v.x;
	p.v.y = -q.v.y;
	p.v.z = -q.v.z;

	return p;
}

static  quat qmul(quat q, quat p)
{
	quat s;

	s.r = (fmul(q.r,p.r)-fmul(q.v.x,p.v.x)-fmul(q.v.y,p.v.y)-fmul(q.v.z,p.v.z));
	s.v.x = (fmul(p.r,q.v.x)+fmul(p.v.x,q.r)-fmul(p.v.y,q.v.z)+fmul(p.v.z,q.v.y));
	s.v.y = (fmul(p.r,q.v.y)+fmul(p.v.x,q.v.z)+fmul(p.v.y,q.r)-fmul(p.v.z,q.v.x));
	s.v.z = (fmul(p.r,q.v.z)-fmul(p.v.x,q.v.y)+fmul(p.v.y,q.v.x)+fmul(p.v.z,q.r));

	return s;
}

static  dquat qmul2(quat q, quat p, int f)
{
	dquat s;

	s.r = (fmul2(q.r,p.r)-fmul2(q.v.x,p.v.x)-fmul2(q.v.y,p.v.y)-fmul2(q.v.z,p.v.z))/f;
	s.v.x = (fmul2(p.r,q.v.x)+fmul2(p.v.x,q.r)-fmul2(p.v.y,q.v.z)+fmul2(p.v.z,q.v.y))/f;
	s.v.y = (fmul2(p.r,q.v.y)+fmul2(p.v.x,q.v.z)+fmul2(p.v.y,q.r)-fmul2(p.v.z,q.v.x))/f;
	s.v.z = (fmul2(p.r,q.v.z)-fmul2(p.v.x,q.v.y)+fmul2(p.v.y,q.v.x)+fmul2(p.v.z,q.r))/f;

	return s;
}

#define _QSUM(e) s.e = q.e + p.e
static  dquat dqsum(dquat q, dquat p)
{
	dquat s;

	_QSUM(r);
	_QSUM(v.x);
	_QSUM(v.y);
	_QSUM(v.z);

	return s;
}

static  quat qsum(quat q, quat p)
{
	quat s;

	_QSUM(r);
	_QSUM(v.x);
	_QSUM(v.y);
	_QSUM(v.z);

	return s;
}

static quat qrenorm(quat q)
{
	frac err;
	quat correction;

	err = q_normerror(q);
	correction = qscale(q, err);
	return qsum(q, correction);
}

static dquat dqrenorm(dquat q)
{
	frac err;
	dquat correction;
	quat qh = qtrunc(q);

	err = q_normerror(qh);
	correction = qscale2(qh, err);
	return dqsum(q, correction);
}

static quat qdecompose(quat q, u8 axis)
{
	frac norm_err;
	frac the_axis;
	quat ret = {0, {0,0,0}};

	switch (axis) {
	case AXIS_X:
		the_axis = q.v.x;
		break;
	case AXIS_Y:
		the_axis = q.v.y;
		break;
	case AXIS_Z:
		the_axis = q.v.z;
		break;
	}

	norm_err = FRAC_0_5 - dtrunc(fmul2(q.r, q.r) +
						fmul2(the_axis,the_axis))/2;
	ret.r = q.r + fmul(norm_err, q.r);
	the_axis = the_axis + fmul(norm_err, the_axis);

	switch (axis) {
	case AXIS_X:
		ret.v.x = the_axis;
		break;
	case AXIS_Y:
		ret.v.y = the_axis;
		break;
	case AXIS_Z:
		ret.v.z = the_axis;
		break;
	}

	return ret;
}

#endif /* _ARITH_H_ */
