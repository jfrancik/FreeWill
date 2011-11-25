#pragma once
#include <math.h>

class CMotion3
{
public:
	CMotion3();
	CMotion3(double j11, double j12, double j21, double j22, double a1, double a2, double v);
	CMotion3(double j, double a, double v);

	// Input:
	bool bSimple;	// set to enable simpler algo (single jerk and single acceleration value)
	double j11, j12, j21, j22;
	double a1, a2;
	double v;
	// calculation Outcomes
	double t, s;
	double v0, v11, v1, v12, v21, v2, v22;
	double s11, s1, s12, sv, s21, s2, s22;
	double t11, t1, t12, tv, t21, t2, t22;

	double CalcDistance(double t);
	double CalcTime(double s);

	double GetPos(double t);
	double GetPosRel(double t)		{ if (s == 0) return 0; return GetPos(t) / s; };

private:
	void CalcDistanceFull();
	void CalcDistanceSimple();
};
