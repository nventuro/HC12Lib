/*
 * nlcf.c
 *
 *
 * Non linear complementary filter for attitude estimation
 *
 * Referencias:
 * 	R. Mahony, T. Hamel, and J.-M. Pflimlin, “Non-linear complementary
 * 	filters on the special orthogonal group,” IEEE Trans. Automatic Control,
 * 	vol. 53, no. 5, pp. 1203–1218, June 2008.
 *
 */

#ifdef __HC12__
#include "derivative.h"
#include "dmu.h"
#include "quick_serial.h"
#endif

//#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "nlcf.h"

/* El samplerate es 1000  Hz */

/*esto hay que hacerlo a mano
 * q[n+1] = q[n] + d_q/fs = q[n] + 0.5 * q[n] * p(omega + kp*wmes) / fs
 * machine_omega = real_omega / GYRO_SCALE
 * nos olvidamos del wmes por ahora
 *  =>
 * q[n+1] = q[n] + 0.5 * q[n] * p[machine_omega] * GYRO_SCALE / fs
 * Comprimiendo las constantes:
 * q[n+1] = q[n] + q[n] * p(machine_omega) / D_Q_SCALE
 * 	donde D_Q_SCALE = 1/(0.5 * GYRO_SCALE / fs) = 2*fs / GYRO_SCALE
 */
#define GYRO_SCALE 34.90658504 /* rad / s @ fsd ; son 5.125 bits */
//#define D_Q_SCALE 3 /*en realidad es 2.864788976, o 1.518 */
#define D_Q_SCALE 57

/* esto también va a mano
 *	machine_wmes = wmes / ACC_SCALE
 * wmes va sumado al gyro, por lo tanto:
 * 	v = omega + kp*wmes
 * 	machine_v = machine_omega + machine_wmes/WMES_DIV 
 * 	machine_v = v / GYRO_SCALE = real_omega / GYRO_SCALE + kp*wmes / GYRO_SCALE
 *  =>
 * 	kp*wmes/GYRO_SCALE = kp*machine_wmes*ACC_SCALE/GYRO_SCALE = machine_wmes/ WMES_DIV
 * WMES_DIV = GYRO_SCALE / (kp * ACC_SCALE)
 */
#define ACC_SCALE (16*9.81) /* = 156.96 = 16g m*s^2 , son 7.29 bits */

/*
Estos valores le dan demasiada importancia al acelerometro
#define Kp (1)
#define WMES_DIV 0.2223 // debería ser 0.2223 o -2.16 bits
#define WMES_MUL 4 // deberia ser 1/WMES_DIV, o 4.497
*/

/*esto es mejor */
#define WMES_DIV 2 /*para probar */

/* Con el bias hay 2 grados de libertad
 *  (1) bias en sí, que depende de wmes.
 *  (2) bias con respecto a omega
 * La clave está en saber en que rango puede estar bias para aprovechar todo
 * el rango dinámico (1), y a partir de eso determinar (2)
 * MPU-6000 Zero rate output (ZRO): +- 20 °/s = 0.35 rad / s. Empirico: 0.1 max
 *
 * machine_bias = bias / BIAS_SCALE
 *
 * (1)
 * 	bias[n+1] = bias[n] + Ki * wmes / fs
 * 	machine_bias[n+1] = bias[n+1] / BIAS_SCALE
 * 			= bias[n]/BIAS_SCALE + Ki * wmes / (BIAS_SCALE * fs)
 * 				= machine_bias + machine_wmes / D_BIAS_SCALE
 * machine_wmes =
 * 	=>
 * 	Ki*wmes/(BIAS_SCALE*fs) = machine_wmes / D_BIAS_SCALE
 * 	Ki*(machine_wmes * ACC_SCALE)/(BIAS_SCALE*fs) = machine_wmes / D_BIAS_SCALE
 * 	D_BIAS_SCALE = BIAS_SCALE*fs / (ACC_SCALE * Ki)
 *
 * (2)
 *	omega_corrected = omega + bias
 * 	machine_omega_corrected = omega_corrected / GYRO_SCALE
 * 	omega/GYRO_SCALE + bias/GYRO_SCALE = machine_omega + machine_bias / BIAS_SCALE2
 * 	machine_bias * BIAS_SCALE / GYRO_SCALE = machine_bias / BIAS_SCALE2;
 * 		=> BIAS_SCALE2 = GYRO_SCALE / BIAS_SCALE
 *
 * Solución 1: BIAS_SCALE = ZRO (empirico) = 0.1
 *		BIAS_SCALE2 = 349
 * 		D_BIAS_SCALE = 5/(156*0.3) = 0.106
 * 	Problema: en una multiplico y en la otra divido
 * Solucion 2: BIAS_SCALE = 2
 * 		BIAS_SCALE2 = 17
 * 		D_BIAS_SCALE = 2
 *
 * Nota: El bias se suma para no tener que escribir una función de sustraccion
 * de vectores.
 */
/* para 1000Hz: ???*/
#define Ki 0.3

/* esto tambien es muy violento
#define D_BIAS_SCALE 2
*/
#define D_BIAS_SCALE 32

#define BIAS_SCALE2 2 /* GYRO_SCALE / BIAS_SCALE */
//#define BIAS_SCALE2 0 /* para probar */

#include "arith.h"

/* Non-linear complementary filter on SO(3) */

#define OPT_INLINE /*nada */

//static quat Q_Correction = UNIT_Q;
static quat Q_Correction = {318, {-23544, -22788, -285}};

static OPT_INLINE vec3 z_dir(quat q)
{
	vec3 zd;
	frac tmp;

	zd.x = 2*(-fmul(q.v.y, q.r) + fmul(q.v.z, q.v.x));
	zd.y = 2*(fmul(q.v.z, q.v.y) + fmul(q.v.x, q.r));
	tmp = -fmul(q.v.x, q.v.x) - fmul(q.v.y, q.v.y);
	zd.z = FRAC_1 + tmp + tmp;

	return zd;
}

static OPT_INLINE vec3 verror(vec3 y, vec3 x)
{
	vec3 zd;

	zd.x = fmul(y.z, x.y) - fmul(x.z, y.y);
	zd.y = fmul(y.x, x.z) - fmul(x.x, y.z);
	zd.z = fmul(y.y, x.x) - fmul(x.y, y.x);

	return zd;
}
//vec3 Bias;
void att_estim(vec3 gyro, vec3 accel, quat *qest_out, vec3 *gyro_out)
{
	static dquat q = UNIT_DQ;
	static vec3 bias = {0, 0, 0};
	vec3 z_estim, wmes, d_bias;
	quat p;
	quat q_lowres = qtrunc(q);
	dquat d_q, correction;
	frac err;

	/* convertimos al marco de ref del body */
	gyro = qrot(Q_Correction, gyro);
	accel = qrot(Q_Correction, accel);
	*gyro_out = gyro;

	/* wmes, bias */
	z_estim = z_dir(q_lowres);

#ifdef PRINTZ

	printf("%g %g %g\n",	FRAC2DBL(z_estim.x),
				FRAC2DBL(z_estim.y),
				FRAC2DBL(z_estim.z));
#endif /* PRINTZ */

	wmes = verror(accel, z_estim);

#ifdef PRINTW
	printf("%g %g %g\n",	FRAC2DBL(wmes.x),
				FRAC2DBL(wmes.y),
				FRAC2DBL(wmes.z));
#endif /* PRINTW */

	d_bias = vdiv(wmes, D_BIAS_SCALE);

#ifdef PRINTB
	printf("%g %g %g\n",	FRAC2DBL(bias.x),
				FRAC2DBL(bias.y),
				FRAC2DBL(bias.z));
#endif /* PRINTB */

	/* d_q */
	p.r = 0;
	/* la correccion de bias se estaba portando mal */
	// p.v = vsum(vsum(gyro, vdiv(bias, BIAS_SCALE2)), vmul(wmes, WMES_MUL));
	// p.v = vsub(vsum(gyro, vdiv(bias, BIAS_SCALE2)), vdiv(wmes, WMES_DIV));
	
	p.v = vsum(gyro, vdiv(wmes, WMES_DIV));
//	p.v = vdiv(wmes, WMES_DIV);

	d_q = qmul2(q_lowres, p, D_Q_SCALE);

	/* bias, q */
	bias = vsub(bias, d_bias);
	q = dqsum(q, d_q);
//	Bias = bias;
	/* renormalización */
	err = q_normerror(qtrunc(q));
	correction = qscale2(q_lowres, err);
	q = dqsum(q, correction);

	*qest_out = qtrunc(q);
	//*qest_out = qmul(qtrunc(q), Q_Correction);
}

/* Calibración:
	
	p es la orientación del IMU con respecto al body:
		X_body = p*X_imu*p'
	q es la orientación del body con respecto la tierra:
		X_earth = q*X_body*q'
		
	por lo tanto:
		X_earth = q*p*X_imu*p'*q'
		q*p = M (salida del estimador) => X_earth = M*X_imu*M'
	para obtener q:
		M*p' = q*p*p' = q (porque son cuaterniones unitarios)
	
	La otra manera es convertir las mediciones al marco de referencia
	del vehículo:
		w_body = p*w_imu*p'
		
*/

#define CAL_ITERATIONS 12

#define Q_COMPONENTS(q) (q).r, (q).v.x, (q).v.y, (q).v.z

struct qpair _calibrate(quat mes0, quat mes1)
{
	const quat pos0 = UNIT_Q, pos1 = {23170, {16384, 16384, 0}};
	quat k0 = UNIT_Q, k1 = UNIT_Q;
	dquat p0, p1;
	struct qpair r;
	int i;

	for (i = 0; i < CAL_ITERATIONS; i++) {
		p0 = qmul2(qconj(qmul(k0, pos0)), mes0, 1);
		p1 = qmul2(qconj(qmul(k1, pos1)), mes1, 1);

		p0 = dqrenorm(p0);
		p1 = dqrenorm(p1);
#ifdef PRINT_CAL_STEPS
		{
			printf("%d %d %d %d, ", Q_COMPONENTS(qtrunc(p0)));
			printf("%d %d %d %d\n", Q_COMPONENTS(qtrunc(p1)));
		}
#endif /*PRINT_CAL_STEPS*/
		k0 = qmul(mes0, qconj(qmul(pos0, qtrunc(p1))));
		k1 = qmul(mes1, qconj(qmul(pos1, qtrunc(p0))));

		k0 = qdecompose(k0, AXIS_Z);
		k1 = qdecompose(k1, AXIS_Z);
	}

	r.p0 = qtrunc(p0);
	r.p1 = qtrunc(p1);

	return r;
}

#define ERR_1DG ((dfrac)1073700939)
#define ERR_2DG ((dfrac)1073578288)
#define ERR_5DG ((dfrac)1072719860)

struct cal_output att_calibrate(quat mes0, quat mes1)
{
	struct qpair p;
	dfrac err;
	struct cal_output r;
	
	p = _calibrate(mes0, mes1);
	err = qmul2(p.p0, qconj(p.p1), 1).r;
	
	//r.correction = qconj(p.p0);
	r.correction = p.p0;
	
	if (err > ERR_1DG)
		r.quality = CAL_EXCELLENT;
	else if (err > ERR_2DG)
		r.quality = CAL_GOOD;
	else if (err > ERR_5DG)
		r.quality = CAL_UGLY;
	else
		r.quality = CAL_BAD;
	
	return r;
}

void att_apply_correction(struct cal_output c)
{
	Q_Correction = c.correction;
}


#ifdef __HC12__

#endif
