#pragma once

#include <math.h>

inline FWVECTOR Vector(FWFLOAT x, FWFLOAT y, FWFLOAT z)
{
	FWVECTOR v = {x, y, z };
	return v;
}

inline FWVECTOR VectorCross(const FWVECTOR &a, const FWVECTOR &b)
{
	FWVECTOR v = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	return v;
}

inline FWFLOAT VectorDot(const FWVECTOR &a, const FWVECTOR &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline FWVECTOR VectorAdd(const FWVECTOR &a, const FWVECTOR &b)
{
	FWVECTOR v = { a.x + b.x, a.y + b.y, a.z + b.z };
	return v;
}

inline FWVECTOR VectorSub(const FWVECTOR &a, const FWVECTOR &b)
{
	FWVECTOR v = { a.x - b.x, a.y - b.y, a.z - b.z };
	return v;
}

inline FWVECTOR VectorMul(const FWVECTOR &a, FWFLOAT f)
{
	FWVECTOR v = { a.x * f, a.y * f, a.z * f };
	return v;
}

inline FWFLOAT VectorLen(const FWVECTOR &a)
{
	return (FWFLOAT)sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline void VectorNormalise(FWVECTOR &a)
{
	FWFLOAT fLen = VectorLen(a);
	a.x /= fLen; a.y /= fLen; a.z /= fLen;
}

inline FWVECTOR VectorNormalisedCross(const FWVECTOR &a, const FWVECTOR &b)
{
	FWVECTOR v = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	VectorNormalise(v);
	return v;
}

inline FWVECTOR operator+ (FWVECTOR &v1, FWVECTOR &v2)
{
	return VectorAdd(v1, v2);
}

inline FWVECTOR operator- (FWVECTOR &v1, FWVECTOR &v2)
{
	return VectorSub(v1, v2);
}

inline FWVECTOR operator* (FWFLOAT f, FWVECTOR &v)
{
	return VectorMul(v, f);
}

inline FWVECTOR operator* (FWVECTOR &v, FWFLOAT f)
{
	return VectorMul(v, f);
}
