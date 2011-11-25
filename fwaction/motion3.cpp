#include "StdAfx.h"
#include "Motion3.h"

CMotion3::CMotion3()
{
	bSimple = false;
	this->j11 = this->j12 = this->j21 = this->j22 = 0;
	this->a1 = this->a2 = 0;
	this->v = 0;
	t = s = 0;
	v0 = v11 = v1 = v12 = v21 = v2 = v22 = 0;
	s11 = s1 = s12 = sv = s21 = s2 = s22 = 0;
	t11 = t1 = t12 = tv = t21 = t2 = t22 = 0;
}

CMotion3::CMotion3(double j11, double j12, double j21, double j22, double a1, double a2, double v)
{
	bSimple = false;
	this->j11 = j11;
	this->j12 = j12;
	this->j21 = j21;
	this->j22 = j22;
	this->a1 = a1;
	this->a2 = a2;
	this->v = v;
	t = s = 0;
	v0 = v11 = v1 = v12 = v21 = v2 = v22 = 0;
	s11 = s1 = s12 = sv = s21 = s2 = s22 = 0;
	t11 = t1 = t12 = tv = t21 = t2 = t22 = 0;
}

CMotion3::CMotion3(double j, double a, double v)
{
	bSimple = true;
	this->j11 = this->j12 = this->j21 = this->j22 = j;
	this->a1 = this->a2 = a;
	this->v = v;
	t = s = 0;
	v0 = v11 = v1 = v12 = v21 = v2 = v22 = 0;
	s11 = s1 = s12 = sv = s21 = s2 = s22 = 0;
	t11 = t1 = t12 = tv = t21 = t2 = t22 = 0;
}

double CMotion3::CalcDistance(double _t)
{
	t = _t;
	if (bSimple) CalcDistanceSimple();
	else CalcDistanceFull();
	return s;
}

void CMotion3::CalcDistanceFull()
{
	// harmonic mean of jerks
	double j1 = 2 * j11 * j12 / (j11 + j12);
	double j2 = 2 * j21 * j22 / (j21 + j22);

	// calculation of maximum acceleration
	double amax1 = sqrt(v * j1);
	double amax2 = sqrt(v * j2);
	if (a1 > amax1) a1 = amax1;
	if (a2 > amax2) a2 = amax2;

	// time for jerked motion
	t11 = a1/j11;
	t12 = a1/j12;
	t21 = a2/j21;
	t22 = a2/j22;
	
	if (t11 + t12 + t21 + t22 > t)
	{
		// if no time left for uniform acceleration 
		// jerked periods to be proportinally scaled down, accelerations amended
		t11 = t*a2*j12/(a1+a2)/(j11+j12);
		t12 = t*a2*j11/(a1+a2)/(j11+j12);
		t21 = t*a1*j22/(a1+a2)/(j21+j22);
		t22 = t*a1*j21/(a1+a2)/(j21+j22);
		a1 = j11 * t11;
		a2 = j21 * t21;
	}

	// velocity deltas
	double dv11 = j11 * t11 * t11 / 2;
	double dv12 = j12 * t12 * t12 / 2;
	double dv21 = j21 * t21 * t21 / 2;
	double dv22 = j22 * t22 * t22 / 2;
	double dv1 = v - dv11 - dv12;
	double dv2 = v - dv21 - dv22;

	// time for accelerated motion
	t1 = dv1 / a1;
	t2 = dv2 / a2;

	if (t11 + t1 + t12 + t21 + t2 + t22 > t)
	{
		// if no time left for uniform velocity
		// accelerated periods to be proportinally scaled down, velocities amended
		double tx = t - t11 - t12 - t21 - t22;
		double dvx = dv11 + dv12 - dv21 - dv22;
		t1 = (tx * a2 - dvx) / (a1 + a2);
		t2 = (tx * a1 + dvx) / (a1 + a2);

		if (t1 < 0)
		{
			// a1 acceleration was still too high
			double tq = (-2 * a2 + 2 * sqrt(a2 * a2 + j1 * (dv21 + dv22 + a2 * (t - t21 - t22)))) / j1;
			t11 = tq * j12 / (j11 + j12);
			t1 = 0;
			t12 = tq * j11 / (j11 + j12);
			a1 = j1 * tq / 2;
			t2 = t - tq - t21 - t22;

			dv11 = j11 * t11 * t11 / 2;
			dv12 = j12 * t12 * t12 / 2;
		}
		else
		if (t2 < 0)
		{
			// a2 acceleration was still too high
			double tq = (-2 * a1 + 2 * sqrt(a1 * a1 + j2 * (dv11 + dv12 + a1 * (t - t11 - t12)))) / j2;
			t21 = tq * j22 / (j21 + j22);
			t2 = 0;
			t22 = tq * j21 / (j21 + j22);
			a2 = j2 * tq / 2;
			t1 = t - t11 - t12 - tq;

			dv21 = j21 * t21 * t21 / 2;
			dv22 = j22 * t22 * t22 / 2;
		}

		dv1 = a1 * t1;
		dv2 = a2 * t2;
	}

	tv = t - t11 - t1 - t12 - t21 - t2 - t22;

	v0  = 0;
	v11 = v0  + dv11;
	v1  = v11 + dv1;
	v12 = v1  + dv12;
	v          = v12;
	v21 = v   - dv21;
	v2  = v21 - dv2;
	v22 = v2  - dv22;	

	s11 = v0  * t11 + j11 * t11 * t11 * t11 / 6;
	s1  = v11 * t1  + a1 * t1 * t1 / 2;
	s12 = v1  * t12 + a1 * t12 * t12 / 2 - j12 * t12 * t12 * t12 / 6;
	sv  = v   * tv;
	s21 = v   * t21 - j21 * t21 * t21 * t21 / 6;
	s2  = v21 * t2  - a2 * t2 * t2 / 2;
	s22 = v2  * t22 - a2 * t22 * t22 / 2 + j22 * t22 * t22 * t22 / 6;

	s = s11 + s1 + s12 + sv + s21 + s2 + s22;
}

void CMotion3::CalcDistanceSimple()
{
	double j = j11;
	double a = a1;

	// calculation of maximum acceleration
	double amax = sqrt(v * j);
	if (a > amax) a = a1 = a2 = amax;

	// time for jerked motion
	double tj = a/j;
	
	if (4 * tj > t)
	{
		// if no time left for uniform acceleration 
		// jerked periods to be proportinally scaled down, accelerations amended
		tj = t / 4;
		a = a1 = a2 = j * tj;
	}

	// velocity deltas
	double dvj = j * tj * tj / 2;
	double dva = v - 2 * dvj;

	// time for accelerated motion
	double ta = dva / a;

	if (4 * tj + 2 * ta > t)
	{
		// if no time left for uniform velocity
		// accelerated periods to be proportinally scaled down, velocities amended
		ta = (t - 4 * tj) / 2;

		dva = a * ta;
	}

	t11 = t12 = t21 = t22 = tj;
	t1 = t2 = ta;
	tv = t - 4 * tj - 2 * ta;

	v0  = 0;
	v11 = v0  + dvj;
	v1  = v11 + dva;
	v12 = v1  + dvj;
	v   = v12;
	v21 = v   - dvj;
	v2  = v21 - dva;
	v22 = v2  - dvj;	

	double sj = j * tj * tj * tj / 6;
	double sa = a * ta * ta / 2;
	double saj = a * tj * tj / 2;
	s11 = v0  * tj + sj;
	s1  = v11 * ta + sa;
	s12 = v1  * tj + saj - sj;
	sv  = v   * tv;
	s21 = v   * tj - sj;
	s2  = v21 * ta - sa;
	s22 = v2  * tj - saj + sj;

	s = s11 + s1 + s12 + sv + s21 + s2 + s22;
}

double CMotion3::CalcTime(double s)
{
	// not implemented;
	return 0;
}

double CMotion3::GetPos(double T)
{
	if (s == 0 || t == 0) return 0;
	if (T >= t) return s;

	double S = 0;

	if (T < t11) return S + v0  * T + j11 * T * T * T / 6;
	S += s11;
	T -= t11;

	if (T < t1) return S + v11 * T  + a1 * T * T / 2;
	S += s1;
	T -= t1;

	if (T < t12) return S + v1  * T + a1 * T * T / 2 - j12 * T * T * T / 6;
	S += s12;
	T -= t12;

	if (T < tv) return S + v   * T;
	S += sv;
	T -= tv;

	if (T < t21) return S + v   * T - j21 * T * T * T / 6;
	S += s21;
	T -= t21;

	if (T < t2) return S + v21 * T  - a2 * T * T / 2;
	S += s2;
	T -= t2;

	if (T < t22) return S + v2  * T - a2 * T * T / 2 + j22 * T * T * T / 6;
	S += s22;
	T -= t22;

	return S;
}
