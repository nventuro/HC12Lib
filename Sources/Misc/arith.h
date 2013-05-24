/* **** Aritmetica Fraccionaria (punto fijo) ***** */


#ifndef _ARITH_H_
#define _ARITH_H_

#include "common.h"

typedef s16 frac;
typedef s32 dfrac;

frac fsqrt(frac b);

#define FRAC_BIT (16)
#define DFRAC_BIT (32)

#define FRAC_1 S16_MAX /* en realidad es 0.999 */
#define FRAC_0_5 (-(S16_MIN>>1)) /* 0.5 en fraccional */
#define FRAC_minus1 S16_MIN

#define DFRAC_1 (-(S32_MIN>>1))
#define DFRAC_almost1 (-(S32_MIN>>1)-1)
#define DFRAC_minus1 (S32_MIN>>1)

static frac fmul(frac a, frac b)
{ /* Toma dos numeros fraccionarios en formato 1.15 y devuelve el producto en
 * formato 1.15 */
	return ((((dfrac)a) * b) << 1) >> FRAC_BIT;
}

static dfrac fmul2(frac a, frac b)
{ /* Toma dos numeros fraccionarios en formato 1.15 y devuelve el producto en
 * formato 2.30 */
	return (((dfrac)a) * b);
}

static frac dtrunc(dfrac x)
{ // 2.30 -> 1.15 
	if (x >= DFRAC_1)
		return FRAC_1;
	else if ( x < DFRAC_minus1)
		return FRAC_minus1;
	else
		return (x << 1) >> FRAC_BIT;
}

/* **** Vectores en R3 ****/

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


static void vec_AddInPlace(s16Vec3* This, s16Vec3* b)
{
	This->x += b->x;
	This->y += b->y;
	This->z += b->z;
	
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


static vec3 vec_Add(vec3 a, vec3 b)
{
	vec3 c = a;
	
	vec_AddInPlace(&c, &b);
		
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


/* **** Cuaterniones ****/

typedef struct {
	frac r;
	vec3 v;
} quat;

typedef struct {
	dfrac r;
	dvec3 v;
} dquat;

static const quat UNIT_Q = {FRAC_1, {0, 0, 0}};
/*const dquat UNIT_DQ = ;*/


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

static  dquat qmul2(quat q, quat p, int f)
{
	dquat s;
	
	s.r = (fmul2(q.r,p.r)-fmul2(q.v.x,p.v.x)-fmul2(q.v.y,p.v.y)-fmul2(q.v.z,p.v.z))/f;
	s.v.x = (fmul2(p.r,q.v.x)+fmul2(p.v.x,q.r)-fmul2(p.v.y,q.v.z)+fmul2(p.v.z,q.v.y))/f;
	s.v.y = (fmul2(p.r,q.v.y)+fmul2(p.v.x,q.v.z)+fmul2(p.v.y,q.r)-fmul2(p.v.z,q.v.x))/f;
	s.v.z = (fmul2(p.r,q.v.z)-fmul2(p.v.x,q.v.y)+fmul2(p.v.y,q.v.x)+fmul2(p.v.z,q.r))/f;
	
	return s;
}

static  dquat dqsum(dquat q, dquat p)
{
	dquat s;
#define _DQS(e) s.e = q.e + p.e
	
	_DQS(r);
	_DQS(v.x);
	_DQS(v.y);
	_DQS(v.z);
	
	return s;
}

#endif /* _ARITH_H_ */
