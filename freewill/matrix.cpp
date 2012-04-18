// matrix.cpp : Defines matrix-based implementation for ITransform
//

#include "stdafx.h"
#include "matrix.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define EPS		0.000008f		// numerical tolerance in orthonormality tests

#include "matrix.inl"

///////////////////////////////////////////////////////////
//
// orthonormality tests

HRESULT __stdcall CTransMatrix::IsOrthonormalQuick()
{
	return m_hOrtho;
}

HRESULT __stdcall CTransMatrix::IsOrthonormal()
{
	if (m_hOrtho == S_OK)
		return S_OK;
	else
	{
		FWDOUBLE bIsOrthogonal, bIsNormal;
		IsOrthonormalEx(&bIsOrthogonal, &bIsNormal);
		if (bIsOrthogonal <= 0.02 && bIsNormal <= 0.02)
			return S_OK;
		else
			return S_FALSE;
	}
}

HRESULT __stdcall CTransMatrix::IsOrthonormalEx(FWDOUBLE *bIsOrthogonal, FWDOUBLE *bIsNormal)
{
	if (bIsOrthogonal) *bIsOrthogonal = 
		fabs(A[0][0] * A[1][0] + A[0][1] * A[1][1] + A[0][2] * A[1][2]) +
		fabs(A[0][0] * A[2][0] + A[0][1] * A[2][1] + A[0][2] * A[2][2]) +
		fabs(A[1][0] * A[2][0] + A[1][1] * A[2][1] + A[1][2] * A[2][2]);

	if (bIsNormal) *bIsNormal = 
		fabs(A[0][0] * A[0][0] + A[0][1] * A[0][1] + A[0][2] * A[0][2] - 1.0f) + 
		fabs(A[1][0] * A[1][0] + A[1][1] * A[1][1] + A[1][2] * A[1][2] - 1.0f) + 
		fabs(A[2][0] * A[2][0] + A[2][1] * A[2][1] + A[2][2] * A[2][2] - 1.0f);

	return S_OK;
}

HRESULT __stdcall CTransMatrix::Orthonormalize()
{
	// Gram-Schmidt orthogonalisation:
	// e1 = [ A[0][0], A[0][1], A[0][2] ]
	// e2 = [ A[1][0], A[1][1], A[1][2] ]
	// e3 = [ A[2][0], A[2][1], A[2][2] ]
	// . means dot product
	// e1' = e1
	// e2' = e2 - e1(e1.e2/e1^2)					= e2 - e1 F12
	// e3' = e3 - e1(e1.e3/e1^2) - e2(e2.e3/e2^2)	= e3 - e1 F13 - e2 F23

	// maximal three trials
	FWDOUBLE f1, f2;
	bool bNormal = false;
	for (FWULONG i = 0; !bNormal && i < 3; i++)
	{
		// dot products
		double fDP12 = A[0][0] * A[1][0] + A[0][1] * A[1][1] + A[0][2] * A[1][2];
		double fDP13 = A[0][0] * A[2][0] + A[0][1] * A[2][1] + A[0][2] * A[2][2];
		double fDP23 = A[1][0] * A[2][0] + A[1][1] * A[2][1] + A[1][2] * A[2][2];

		// squares of lengths
		double fL1 = A[0][0] * A[0][0] + A[0][1] * A[0][1] + A[0][2] * A[0][2];
		double fL2 = A[1][0] * A[1][0] + A[1][1] * A[1][1] + A[1][2] * A[1][2];
		double fL3 = A[2][0] * A[2][0] + A[2][1] * A[2][1] + A[2][2] * A[2][2];

		// Gram-Schmidt: e3 computation
		if (fabs(fDP13) > EPS || fabs(fDP23) > EPS)
		{
			double fF13 = fDP13 / fL1;
			double fF23 = fDP23 / fL2;
			A[2][0] = (FWDOUBLE)(A[2][0] - A[0][0] * fF13 - A[1][0] * fF23);
			A[2][1] = (FWDOUBLE)(A[2][1] - A[0][1] * fF13 - A[1][1] * fF23);
			A[2][2] = (FWDOUBLE)(A[2][2] - A[0][2] * fF13 - A[1][2] * fF23);
			fL3 = A[2][0] * A[2][0] + A[2][1] * A[2][1] + A[2][2] * A[2][2];
		}

		// Gram-Schmidt: e2 computation
		if (fabs(fDP12) > EPS)
		{
			double fF12 = fDP12 / fL1;
			A[1][0] = (FWDOUBLE)(A[1][0] - A[0][0] * fF12);
			A[1][1] = (FWDOUBLE)(A[1][1] - A[0][1] * fF12);
			A[1][2] = (FWDOUBLE)(A[1][2] - A[0][2] * fF12);
			fL2 = A[1][0] * A[1][0] + A[1][1] * A[1][1] + A[1][2] * A[1][2];
		}

		// normalisation
		if (fabs(fL1 - 1.0f) > 4*EPS)
		{
			double f = 1.0 / sqrt(fL1);
			A[0][0] *= (FWDOUBLE)f; A[0][1] *= (FWDOUBLE)f; A[0][2] *= (FWDOUBLE)f;
		}
		if (fabs(fL2 - 1.0f) > 4*EPS)
		{
			double f = 1.0 / sqrt(fL2);
			A[1][0] *= (FWDOUBLE)f; A[1][1] *= (FWDOUBLE)f; A[1][2] *= (FWDOUBLE)f;
		}
		if (fabs(fL3 - 1.0f) > 4*EPS)
		{
			double f = 1.0 / sqrt(fL3);
			A[2][0] *= (FWDOUBLE)f; A[2][1] *= (FWDOUBLE)f; A[2][2] *= (FWDOUBLE)f;
		}

		// check orthonormality
		IsOrthonormalEx(&f1, &f2);
		bNormal = f1 < 0.002 && f2 < 0.002;
	}

	if (!bNormal)
		// action of the last chance
		Reset(TRUE, FALSE);

	return S_OK;
}

///////////////////////////////////////////////////////////
//
// ITransform implementation: construction and convertion from other formats

HRESULT __stdcall CTransMatrix::FromIdentity()
{
	_Identity(A);
	m_hOrtho = S_OK;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromTranslationXYZ(FWDOUBLE x, FWDOUBLE y, FWDOUBLE z)
{
	_Identity(A);
	A[3][0] = x; A[3][1] = y; A[3][2] = z;
	m_hOrtho = S_OK;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromTranslationVector(FWVECTOR *pV)
{
	_Identity(A);
	A[3][0] = pV->x; A[3][1] = pV->y; A[3][2] = pV->z;
	m_hOrtho = S_OK;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromTranslationTransform(ITransform *pT)
{
	FWVECTOR v;
	pT->AsVector(&v);
	return FromTranslationVector(&v);
}

HRESULT __stdcall CTransMatrix::FromRotationX(FWDOUBLE fTheta)
{
	return FromRotationSinCosX((FWDOUBLE)_sin(fTheta), (FWDOUBLE)_cos(fTheta));
}

HRESULT __stdcall CTransMatrix::FromRotationTanX(FWDOUBLE fTan1, FWDOUBLE fTan2)
{
	FWDOUBLE f = (FWDOUBLE)sqrt(fTan1 * fTan1 + fTan2 * fTan2);
	return FromRotationSinCosX(fTan1 / f, fTan2 / f);
}

HRESULT __stdcall CTransMatrix::FromRotationSinCosX(FWDOUBLE fSin, FWDOUBLE fCos)
{
	_Identity(A);
	A[1][1] = fCos;		A[2][1] = -fSin;
	A[1][2] = fSin;		A[2][2] = fCos;
	m_hOrtho = S_OK;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromRotationY(FWDOUBLE fTheta)
{
	return FromRotationSinCosY((FWDOUBLE)_sin(fTheta), (FWDOUBLE)_cos(fTheta));
}

HRESULT __stdcall CTransMatrix::FromRotationTanY(FWDOUBLE fTan1, FWDOUBLE fTan2)
{
	FWDOUBLE f = (FWDOUBLE)sqrt(fTan1 * fTan1 + fTan2 * fTan2);
	return FromRotationSinCosY(fTan1 / f, fTan2 / f);
}

HRESULT __stdcall CTransMatrix::FromRotationSinCosY(FWDOUBLE fSin, FWDOUBLE fCos)
{
	_Identity(A);
	A[0][0] = fCos;		A[2][0] = fSin;
	A[0][2] = -fSin;	A[2][2] = fCos;
	m_hOrtho = S_OK;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromRotationZ(FWDOUBLE fTheta)
{
	return FromRotationSinCosZ((FWDOUBLE)_sin(fTheta), (FWDOUBLE)_cos(fTheta));
}

HRESULT __stdcall CTransMatrix::FromRotationTanZ(FWDOUBLE fTan1, FWDOUBLE fTan2)
{
	FWDOUBLE f = (FWDOUBLE)sqrt(fTan1 * fTan1 + fTan2 * fTan2);
	return FromRotationSinCosZ(fTan1 / f, fTan2 / f);
}

HRESULT __stdcall CTransMatrix::FromRotationSinCosZ(FWDOUBLE fSin, FWDOUBLE fCos)
{
	_Identity(A);
	A[0][0] = fCos;		A[1][0] = -fSin;
	A[0][1] = fSin;		A[1][1] = fCos;
	m_hOrtho = S_OK;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromRotationYawPitchRoll(FWDOUBLE fYaw, FWDOUBLE fPitch, FWDOUBLE fRoll)
{
	return FromRotationSinCosYawPitchRoll((FWDOUBLE)_sin(fYaw), (FWDOUBLE)_cos(fYaw), (FWDOUBLE)_sin(fPitch), (FWDOUBLE)_cos(fPitch), (FWDOUBLE)_sin(fRoll), (FWDOUBLE)_cos(fRoll));
}

HRESULT __stdcall CTransMatrix::FromRotationTanYawPitchRoll(FWDOUBLE fTan1Yaw, FWDOUBLE fTan2Yaw, FWDOUBLE fTan1Pitch, FWDOUBLE fTan2Pitch, FWDOUBLE fTan1Roll, FWDOUBLE fTan2Roll)
{
	FWDOUBLE fYaw = (FWDOUBLE)sqrt(fTan1Yaw * fTan1Yaw + fTan2Yaw * fTan2Yaw);
	FWDOUBLE fPitch = (FWDOUBLE)sqrt(fTan1Pitch * fTan1Pitch + fTan2Pitch * fTan2Pitch);
	FWDOUBLE fRoll = (FWDOUBLE)sqrt(fTan1Roll * fTan1Roll + fTan2Roll * fTan2Roll);
	return FromRotationSinCosYawPitchRoll(fTan1Yaw / fYaw, fTan2Yaw / fYaw, fTan1Pitch / fPitch, fTan2Pitch / fPitch, fTan1Roll / fRoll, fTan2Roll / fRoll);
}

HRESULT __stdcall CTransMatrix::FromRotationSinCosYawPitchRoll(FWDOUBLE fSinYaw, FWDOUBLE fCosYaw, FWDOUBLE fSinPitch, FWDOUBLE fCosPitch, FWDOUBLE fSinRoll, FWDOUBLE fCosRoll)
{
	_Identity(A);

	// 1st: roll(z); 2nd: pitch(x); 3rd: yaw(y)
	A[0][0] = fCosYaw * fCosRoll + fSinYaw * fSinPitch * fSinRoll;
	A[1][0] = -fCosYaw * fSinRoll + fSinYaw * fSinPitch * fCosRoll;
	A[2][0] = fSinYaw * fCosPitch;

	A[0][1] = fCosPitch * fSinRoll;
	A[1][1] = fCosPitch * fCosRoll;
	A[2][1] = -fSinPitch;

	A[0][2] = -fSinYaw * fCosRoll + fCosYaw * fSinPitch * fSinRoll;
	A[1][2] = fSinYaw * fSinRoll + fCosYaw * fSinPitch * fCosRoll;
	A[2][2] = fCosYaw * fCosPitch;

	m_hOrtho = S_OK;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromRotationEuler(FWEULER *pEuler)
{
	return FromRotationYawPitchRoll(pEuler->yaw, pEuler->pitch, pEuler->roll);
}

HRESULT __stdcall CTransMatrix::FromRotationAxisAngle(FWVECTOR *p, FWDOUBLE fAngle)
{
	_MatrixRotationAxis(A, p, fAngle);
	//D3DXMatrixRotationAxis((D3DXMATRIX*)A, (D3DXVECTOR3*)(p), fAngle);
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromRotationQuat(FWQUAT *pQuat)
{
	_Identity(A);

	A[0][0] = 1 - 2 * pQuat->y * pQuat->y - 2 * pQuat->z * pQuat->z;
	A[1][0] = 2 * pQuat->x * pQuat->y - 2 * pQuat->w * pQuat->z;
	A[2][0] = 2 * pQuat->x * pQuat->z + 2 * pQuat->w * pQuat->y;

	A[0][1] = 2 * pQuat->x * pQuat->y + 2 * pQuat->w * pQuat->z;
	A[1][1] = 1 - 2 * pQuat->x * pQuat->x - 2 * pQuat->z * pQuat->z;
	A[2][1] = 2 * pQuat->y * pQuat->z - 2 * pQuat->w * pQuat->x;

	A[0][2] = 2 * pQuat->x * pQuat->z - 2 * pQuat->w * pQuat->y;
	A[1][2] = 2 * pQuat->y * pQuat->z + 2 * pQuat->w * pQuat->x;
	A[2][2] = 1 - 2 * pQuat->x * pQuat->x - 2 * pQuat->y * pQuat->y;

	if (fabs(pQuat->x * pQuat->x + pQuat->y * pQuat->y + pQuat->z * pQuat->z + pQuat->w * pQuat->w - 1.0f) <= 3*EPS) 
		m_hOrtho = S_OK;
	else
		m_hOrtho = S_FALSE;

	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromRotationTransform(ITransform *pT)
{
	FWMATRIX9 m;
	pT->AsMatrix9(m);
	FromMatrix9(m);
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromScaling(FWDOUBLE fScaleX, FWDOUBLE fScaleY, FWDOUBLE fScaleZ)
{
	_Identity(A);
	A[0][0] = fScaleX;
	A[1][1] = fScaleY;
	A[2][2] = fScaleZ;
	m_hOrtho = S_FALSE;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromMatrix(FWMATRIX M)
{
	memcpy(A, M, sizeof(FWMATRIX));
	m_hOrtho = S_FALSE;
	m_hOrtho = S_OK;	//// temporary!!!!
	return S_OK;
}

//HRESULT __stdcall CTransMatrix::FromMatrixT(FWMATRIX M)
//{
//	A[0][0] = M[0][0]; A[1][0] = M[0][1]; A[2][0] = M[0][2]; A[3][0] = M[0][3]; 
//	A[0][1] = M[1][0]; A[1][1] = M[1][1]; A[2][1] = M[1][2]; A[3][1] = M[1][3]; 
//	A[0][2] = M[2][0]; A[1][2] = M[2][1]; A[2][2] = M[2][2]; A[3][2] = M[2][3]; 
//	A[0][3] = M[3][0]; A[1][3] = M[3][1]; A[2][3] = M[3][2]; A[3][3] = M[3][3]; 
//	m_hOrtho = S_FALSE;
//	return S_OK;
//}

HRESULT __stdcall CTransMatrix::FromMatrix12(FWMATRIX12 M12)
{
	FWDOUBLE (*M)[3] = (FWDOUBLE(*)[3])M12;
	A[0][0] = M[0][0]; A[1][0] = M[1][0]; A[2][0] = M[2][0]; A[3][0] = M[3][0]; 
	A[0][1] = M[0][1]; A[1][1] = M[1][1]; A[2][1] = M[2][1]; A[3][1] = M[3][1]; 
	A[0][2] = M[0][2]; A[1][2] = M[1][2]; A[2][2] = M[2][2]; A[3][2] = M[3][2]; 
	A[0][3] = 0.0f;    A[1][3] = 0.0f;    A[2][3] = 0.0f;    A[3][3] = 1.0f; 
	m_hOrtho = S_FALSE;
	return S_OK;
}

//HRESULT __stdcall CTransMatrix::FromMatrix12T(FWMATRIX12 M12)
//{
//	FWDOUBLE (*M)[4] = (FWDOUBLE(*)[4])M12;
//	A[0][0] = M[0][0]; A[1][0] = M[0][1]; A[2][0] = M[0][2]; A[3][0] = M[0][3]; 
//	A[0][1] = M[1][0]; A[1][1] = M[1][1]; A[2][1] = M[1][2]; A[3][1] = M[1][3]; 
//	A[0][2] = M[2][0]; A[1][2] = M[2][1]; A[2][2] = M[2][2]; A[3][2] = M[2][3]; 
//	A[0][3] = 0.0f;    A[1][3] = 0.0f;    A[2][3] = 0.0f;    A[3][3] = 1.0f; 
//	m_hOrtho = S_FALSE;
//	return S_OK;
//}

HRESULT __stdcall CTransMatrix::FromMatrix9(FWMATRIX9 M9)
{
	A[0][0] = M9[0][0]; A[1][0] = M9[1][0]; A[2][0] = M9[2][0]; A[3][0] = 0.0f;
	A[0][1] = M9[0][1]; A[1][1] = M9[1][1]; A[2][1] = M9[2][1]; A[3][1] = 0.0f;
	A[0][2] = M9[0][2]; A[1][2] = M9[1][2]; A[2][2] = M9[2][2]; A[3][2] = 0.0f;
	A[0][3] = 0.0f;     A[1][3] = 0.0f;     A[2][3] = 0.0f;     A[3][3] = 1.0f; 
	m_hOrtho = S_FALSE;
	return S_OK;
}

//HRESULT __stdcall CTransMatrix::FromMatrix9T(FWMATRIX9 M9)
//{
//	A[0][0] = M9[0][0]; A[1][0] = M9[0][1]; A[2][0] = M9[0][2]; A[3][0] = 0.0f;
//	A[0][1] = M9[1][0]; A[1][1] = M9[1][1]; A[2][1] = M9[1][2]; A[3][1] = 0.0f;
//	A[0][2] = M9[2][0]; A[1][2] = M9[2][1]; A[2][2] = M9[2][2]; A[3][2] = 0.0f;
//	A[0][3] = 0.0f;     A[1][3] = 0.0f;     A[2][3] = 0.0f;     A[3][3] = 1.0f; 
//	m_hOrtho = S_FALSE;
//	return S_OK;
//}

HRESULT __stdcall CTransMatrix::FromTransform(ITransform *pT)
{
	if (pT->IsOrthonormalQuick() == S_OK)
		m_hOrtho = S_OK;
	else
		m_hOrtho = S_FALSE;
	return pT->AsMatrix(A);
}

HRESULT __stdcall CTransMatrix::FromInvertion(ITransform *pT)
{
	HRESULT h = FromTransform(pT);
	if (FAILED(h)) return h;
	return Inverse();
}

///////////////////////////////////////////////////////////
//
// ITransform implementation: Perspective and Similar Transformations

HRESULT __stdcall CTransMatrix::FromLookAtLH(FWVECTOR *pEye, FWVECTOR *pAt, FWVECTOR *pUp)
{
	FWVECTOR xaxis, yaxis, zaxis;
	FWFLOAT l;

	//zaxis = normal(At - Eye)
	zaxis.x = pAt->x - pEye->x; zaxis.y = pAt->y - pEye->y; zaxis.z = pAt->z - pEye->z;
	l = (FWFLOAT)sqrt(zaxis.x * zaxis.x + zaxis.y * zaxis.y + zaxis.z * zaxis.z);
	zaxis.x /= l; zaxis.y /= l; zaxis.z /= l;

	//xaxis = normal(cross(Up, zaxis))
	xaxis.x = pUp->y * zaxis.z - pUp->z * zaxis.y;
	xaxis.y = pUp->z * zaxis.x - pUp->x * zaxis.z;
	xaxis.z = pUp->x * zaxis.y - pUp->y * zaxis.x;
	l = (FWFLOAT)sqrt(xaxis.x * xaxis.x + xaxis.y * xaxis.y + xaxis.z * xaxis.z);
	xaxis.x /= l; xaxis.y /= l; xaxis.z /= l;

	//yaxis = cross(zaxis, xaxis)
	yaxis.x = zaxis.y * xaxis.z - zaxis.z * xaxis.y;
	yaxis.y = zaxis.z * xaxis.x - zaxis.x * xaxis.z;
	yaxis.z = zaxis.x * xaxis.y - zaxis.y * xaxis.x;

	// xaxis.x           xaxis.y           xaxis.z          -dot(xaxis, eye)
	// yaxis.x           yaxis.y           yaxis.z          -dot(yaxis, eye)
	// zaxis.x           zaxis.y           zaxis.z          -dot(zaxis, eye)
	// 0                 0                 0                1
	A[0][0] = xaxis.x; A[1][0] = xaxis.y; A[2][0] = xaxis.z; 
	A[3][0] = -xaxis.x * pEye->x - xaxis.y * pEye->y - xaxis.z * pEye->z;
	A[0][1] = yaxis.x; A[1][1] = yaxis.y; A[2][1] = yaxis.z; 
	A[3][1] = -yaxis.x * pEye->x - yaxis.y * pEye->y - yaxis.z * pEye->z;
	A[0][2] = zaxis.x; A[1][2] = zaxis.y; A[2][2] = zaxis.z; 
	A[3][2] = -zaxis.x * pEye->x - zaxis.y * pEye->y - zaxis.z * pEye->z;
	A[0][3] = A[1][3] = A[2][3] = 0;
	A[3][3] = 1;

	m_hOrtho = S_OK;

	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromLookAtRH(FWVECTOR *pEye, FWVECTOR *pAt, FWVECTOR *pUp) 
{
	FWVECTOR xaxis, yaxis, zaxis;
	FWFLOAT l;

	//zaxis = normal(Eye - At)
	zaxis.x = pEye->x - pAt->x; zaxis.y = pEye->y - pAt->y; zaxis.z = pEye->z - pAt->z;
	l = (FWFLOAT)sqrt(zaxis.x * zaxis.x + zaxis.y * zaxis.y + zaxis.z * zaxis.z);
	zaxis.x /= l; zaxis.y /= l; zaxis.z /= l;

	//xaxis = normal(cross(Up, zaxis))
	xaxis.x = pUp->y * zaxis.z - pUp->z * zaxis.y;
	xaxis.y = pUp->z * zaxis.x - pUp->x * zaxis.z;
	xaxis.z = pUp->x * zaxis.y - pUp->y * zaxis.x;
	l = (FWFLOAT)sqrt(xaxis.x * xaxis.x + xaxis.y * xaxis.y + xaxis.z * xaxis.z);
	xaxis.x /= l; xaxis.y /= l; xaxis.z /= l;

	//yaxis = cross(zaxis, xaxis)
	yaxis.x = zaxis.y * xaxis.z - zaxis.z * xaxis.y;
	yaxis.y = zaxis.z * xaxis.x - zaxis.x * xaxis.z;
	yaxis.z = zaxis.x * xaxis.y - zaxis.y * xaxis.x;

	// xaxis.x           xaxis.y           xaxis.z          -dot(xaxis, eye)
	// yaxis.x           yaxis.y           yaxis.z          -dot(yaxis, eye)
	// zaxis.x           zaxis.y           zaxis.z          -dot(zaxis, eye)
	// 0                 0                 0                1
	A[0][0] = xaxis.x; A[1][0] = xaxis.y; A[2][0] = xaxis.z; 
	A[3][0] = -xaxis.x * pEye->x - xaxis.y * pEye->y - xaxis.z * pEye->z;
	A[0][1] = yaxis.x; A[1][1] = yaxis.y; A[2][1] = yaxis.z; 
	A[3][1] = -yaxis.x * pEye->x - yaxis.y * pEye->y - yaxis.z * pEye->z;
	A[0][2] = zaxis.x; A[1][2] = zaxis.y; A[2][2] = zaxis.z; 
	A[3][2] = -zaxis.x * pEye->x - zaxis.y * pEye->y - zaxis.z * pEye->z;
	A[0][3] = A[1][3] = A[2][3] = 0;
	A[3][3] = 1;

	m_hOrtho = S_OK;

	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromPerspectiveLH(FWDOUBLE fFOV, FWDOUBLE fZN, FWDOUBLE fZF, FWDOUBLE fAspect)
{
	FWDOUBLE yScale = _cos(fFOV/2) / _sin(fFOV/2);
	FWDOUBLE xScale = yScale / fAspect;

	//xScale     0          0               0
	//0        yScale       0               0
	//0          0       zf/(zf-zn)         -zn*zf/(zf-zn)
	//0          0          1               0

	A[0][0] = xScale; 
	A[1][0] = A[2][0] = A[3][0] = 0;

	A[1][1] = yScale;
	A[0][1] = A[2][1] = A[3][1] = 0;

	A[2][2] = fZF / (fZF - fZN);
	A[3][2] = -fZN * fZF / (fZF - fZN);
	A[0][2] = A[1][2] = 0;

	A[0][3] = A[1][3] = A[3][3] = 0;
	A[2][3] = 1;


	// unsuccessful trials with orthogonal camera
	//fZN=1;

	//double w = 1000.0f * fFOV;
	//double h = w / fAspect;
	//double zn = 0;
	//double zf = fZF;


	//A[0][0] = 2.0f / w; 
	//A[1][0] = A[2][0] = A[3][0] = 0;

	//A[1][1] = 2.0f / h;
	//A[0][1] = A[2][1] = A[3][1] = 0;

	//A[2][2] = 1.0f / (zf - zn);
	//A[0][2] = A[1][2] = 0;
	//A[3][2] = 0;

	//A[2][3] = -zn * (zf - zn);
	//A[0][3] = A[1][3] = A[2][3] = 0;
	//A[3][3] = 1;

	m_hOrtho = S_FALSE;

	return S_OK;
}

HRESULT __stdcall CTransMatrix::FromPerspectiveRH(FWDOUBLE fFOV, FWDOUBLE fZN, FWDOUBLE fZF, FWDOUBLE fAspect)
{
	FWDOUBLE yScale = _cos(fFOV/2) / _sin(fFOV/2);
	FWDOUBLE xScale = yScale / fAspect;

	//xScale     0          0               0
	//0        yScale       0               0
	//0          0       zf/(zf-zn)         -zn*zf/(zf-zn)
	//0          0          1               0

	A[0][0] = xScale; 
	A[1][0] = A[2][0] = A[3][0] = 0;

	A[1][1] = yScale;
	A[0][1] = A[2][1] = A[3][1] = 0;

	A[2][2] = fZF / (fZN - fZF);
	A[3][2] = fZN * fZF / (fZN - fZF);
	A[0][2] = A[1][2] = 0;

	A[0][3] = A[1][3] = A[3][3] = 0;
	A[2][3] = -1;

	m_hOrtho = S_FALSE;

	return S_OK;
}

///////////////////////////////////////////////////////////
//
// ITransform implementation: modifications

HRESULT __stdcall CTransMatrix::MulTranslationXYZ(FWDOUBLE x, FWDOUBLE y, FWDOUBLE z)
{
	A[3][0] += x; A[3][1] += y; A[3][2] += z;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulTranslationVector(FWVECTOR *pV)
{
	A[3][0] += pV->x; A[3][1] += pV->y; A[3][2] += pV->z;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulTranslationTransform(ITransform *pT)
{
	FWVECTOR v;
	pT->AsVector(&v);
	return MulTranslationVector(&v);
}

HRESULT __stdcall CTransMatrix::MulRotationX(FWDOUBLE fTheta)
{
	return MulRotationSinCosX((FWDOUBLE)_sin(fTheta), (FWDOUBLE)_cos(fTheta));
}

HRESULT __stdcall CTransMatrix::MulRotationTanX(FWDOUBLE fTan1, FWDOUBLE fTan2)
{
	FWDOUBLE f = (FWDOUBLE)sqrt(fTan1 * fTan1 + fTan2 * fTan2);
	return MulRotationSinCosX(fTan1 / f, fTan2 / f);
}

HRESULT __stdcall CTransMatrix::MulRotationSinCosX(FWDOUBLE fSin, FWDOUBLE fCos)
{
	FWDOUBLE x1, x2;

	x1 = fCos * A[0][1] - fSin * A[0][2];
	x2 = fSin * A[0][1] + fCos * A[0][2];
	A[0][1] = x1;
	A[0][2] = x2;

	x1 = fCos * A[1][1] - fSin * A[1][2];
	x2 = fSin * A[1][1] + fCos * A[1][2];
	A[1][1] = x1;
	A[1][2] = x2;

	x1 = fCos * A[2][1] - fSin * A[2][2];
	x2 = fSin * A[2][1] + fCos * A[2][2];
	A[2][1] = x1;
	A[2][2] = x2;

	x1 = fCos * A[3][1] - fSin * A[3][2];
	x2 = fSin * A[3][1] + fCos * A[3][2];
	A[3][1] = x1;
	A[3][2] = x2;

	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulRotationY(FWDOUBLE fTheta)
{
	return MulRotationSinCosY((FWDOUBLE)_sin(fTheta), (FWDOUBLE)_cos(fTheta));
}

HRESULT __stdcall CTransMatrix::MulRotationTanY(FWDOUBLE fTan1, FWDOUBLE fTan2)
{
	FWDOUBLE f = (FWDOUBLE)sqrt(fTan1 * fTan1 + fTan2 * fTan2);
	return MulRotationSinCosY(fTan1 / f, fTan2 / f);
}

HRESULT __stdcall CTransMatrix::MulRotationSinCosY(FWDOUBLE fSin, FWDOUBLE fCos)
{
	FWDOUBLE x0, x2;

	x0 =  fCos * A[0][0] + fSin * A[0][2];
	x2 = -fSin * A[0][0] + fCos * A[0][2];
	A[0][0] = x0;
	A[0][2] = x2;

	x0 =  fCos * A[1][0] + fSin * A[1][2];
	x2 = -fSin * A[1][0] + fCos * A[1][2];
	A[1][0] = x0;
	A[1][2] = x2;

	x0 =  fCos * A[2][0] + fSin * A[2][2];
	x2 = -fSin * A[2][0] + fCos * A[2][2];
	A[2][0] = x0;
	A[2][2] = x2;

	x0 =  fCos * A[3][0] + fSin * A[3][2];
	x2 = -fSin * A[3][0] + fCos * A[3][2];
	A[3][0] = x0;
	A[3][2] = x2;

	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulRotationZ(FWDOUBLE fTheta)
{
	return MulRotationSinCosZ(_sin(fTheta), _cos(fTheta));
}

HRESULT __stdcall CTransMatrix::MulRotationTanZ(FWDOUBLE fTan1, FWDOUBLE fTan2)
{
	FWDOUBLE f = (FWDOUBLE)sqrt(fTan1 * fTan1 + fTan2 * fTan2);
	return MulRotationSinCosZ(fTan1 / f, fTan2 / f);
}

HRESULT __stdcall CTransMatrix::MulRotationSinCosZ(FWDOUBLE fSin, FWDOUBLE fCos)
{
	FWDOUBLE x0, x1;

	x0 = fCos * A[0][0] - fSin * A[0][1];
	x1 = fSin * A[0][0] + fCos * A[0][1];
	A[0][0] = x0;
	A[0][1] = x1;

	x0 = fCos * A[1][0] - fSin * A[1][1];
	x1 = fSin * A[1][0] + fCos * A[1][1];
	A[1][0] = x0;
	A[1][1] = x1;

	x0 = fCos * A[2][0] - fSin * A[2][1];
	x1 = fSin * A[2][0] + fCos * A[2][1];
	A[2][0] = x0;
	A[2][1] = x1;

	x0 = fCos * A[3][0] - fSin * A[3][1];
	x1 = fSin * A[3][0] + fCos * A[3][1];
	A[3][0] = x0;
	A[3][1] = x1;

	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulRotationYawPitchRoll(FWDOUBLE fYaw, FWDOUBLE fPitch, FWDOUBLE fRoll)
{
	return MulRotationSinCosYawPitchRoll((FWDOUBLE)_sin(fYaw), (FWDOUBLE)_cos(fYaw), (FWDOUBLE)_sin(fPitch), (FWDOUBLE)_cos(fPitch), (FWDOUBLE)_sin(fRoll), (FWDOUBLE)_cos(fRoll));
}

HRESULT __stdcall CTransMatrix::MulRotationTanYawPitchRoll(FWDOUBLE fTan1Yaw, FWDOUBLE fTan2Yaw, FWDOUBLE fTan1Pitch, FWDOUBLE fTan2Pitch, FWDOUBLE fTan1Roll, FWDOUBLE fTan2Roll)
{
	FWDOUBLE fYaw = (FWDOUBLE)sqrt(fTan1Yaw * fTan1Yaw + fTan2Yaw * fTan2Yaw);
	FWDOUBLE fPitch = (FWDOUBLE)sqrt(fTan1Pitch * fTan1Pitch + fTan2Pitch * fTan2Pitch);
	FWDOUBLE fRoll = (FWDOUBLE)sqrt(fTan1Roll * fTan1Roll + fTan2Roll * fTan2Roll);
	return MulRotationSinCosYawPitchRoll(fTan1Yaw / fYaw, fTan2Yaw / fYaw, fTan1Pitch / fPitch, fTan2Pitch / fPitch, fTan1Roll / fRoll, fTan2Roll / fRoll);
}

HRESULT __stdcall CTransMatrix::MulRotationSinCosYawPitchRoll(FWDOUBLE fSinYaw, FWDOUBLE fCosYaw, FWDOUBLE fSinPitch, FWDOUBLE fCosPitch, FWDOUBLE fSinRoll, FWDOUBLE fCosRoll)
{
	FWMATRIX9 M;

	M[0][0] = fCosYaw * fCosRoll + fSinYaw * fSinPitch * fSinRoll;
	M[1][0] = -fCosYaw * fSinRoll + fSinYaw * fSinPitch * fCosRoll;
	M[2][0] = fSinYaw * fCosPitch;

	M[0][1] = fCosPitch * fSinRoll;
	M[1][1] = fCosPitch * fCosRoll;
	M[2][1] = -fSinPitch;

	M[0][2] = -fSinYaw * fCosRoll + fCosYaw * fSinPitch * fSinRoll;
	M[1][2] = fSinYaw * fSinRoll + fCosYaw * fSinPitch * fCosRoll;
	M[2][2] = fCosYaw * fCosPitch;

	_Multiply(M, A);
	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulRotationEuler(FWEULER *pEuler)
{
	return MulRotationYawPitchRoll(pEuler->yaw, pEuler->pitch, pEuler->roll);
}

HRESULT __stdcall CTransMatrix::MulRotationAxisAngle(FWVECTOR *p, FWDOUBLE fAngle)
{
	FWMATRIX M;
	_MatrixRotationAxis(M, p, fAngle);
	//D3DXMatrixRotationAxis((D3DXMATRIX*)N, (D3DXVECTOR3*)(p), fAngle);
	_Multiply(M, A);
	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulRotationQuat(FWQUAT *pQuat)
{
	FWMATRIX9 M;

	M[0][0] = 1 - 2 * pQuat->y * pQuat->y - 2 * pQuat->z * pQuat->z;
	M[1][0] = 2 * pQuat->x * pQuat->y - 2 * pQuat->w * pQuat->z;
	M[2][0] = 2 * pQuat->x * pQuat->z + 2 * pQuat->w * pQuat->y;

	M[0][1] = 2 * pQuat->x * pQuat->y + 2 * pQuat->w * pQuat->z;
	M[1][1] = 1 - 2 * pQuat->x * pQuat->x - 2 * pQuat->z * pQuat->z;
	M[2][1] = 2 * pQuat->y * pQuat->z - 2 * pQuat->w * pQuat->x;

	M[0][2] = 2 * pQuat->x * pQuat->z - 2 * pQuat->w * pQuat->y;
	M[1][2] = 2 * pQuat->y * pQuat->z + 2 * pQuat->w * pQuat->x;
	M[2][2] = 1 - 2 * pQuat->x * pQuat->x - 2 * pQuat->y * pQuat->y;

	if (fabs(pQuat->x * pQuat->x + pQuat->y * pQuat->y + pQuat->z * pQuat->z + pQuat->w * pQuat->w - 1.0f) >= EPS) 
		m_hOrtho = S_FALSE;
	
	_Multiply(M, A);
	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulRotationTransform(ITransform *pT)
{
	FWMATRIX9 m;
	pT->AsMatrix9(m);
	_Multiply(m, A);
	if (pT->IsOrthonormalQuick() != S_OK) 
		m_hOrtho = S_FALSE;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulScale(FWDOUBLE fScaleX, FWDOUBLE fScaleY, FWDOUBLE fScaleZ)
{
	A[0][0] *= fScaleX; A[1][0] *= fScaleX; A[2][0] *= fScaleX; A[3][0] *= fScaleX;
	A[0][1] *= fScaleY; A[1][1] *= fScaleY; A[2][1] *= fScaleY; A[3][1] *= fScaleY;
	A[0][2] *= fScaleZ; A[1][2] *= fScaleZ; A[2][2] *= fScaleZ; A[3][2] *= fScaleZ;
	m_hOrtho = S_FALSE;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::MulMatrix(FWMATRIX M)
{
	m_hOrtho = S_FALSE;
	_Multiply(M, A);
	return S_OK;
}

HRESULT __stdcall CTransMatrix::Multiply(ITransform *pT)
{
	FWMATRIX M;
	pT->AsMatrix(M);
	_Multiply(M, A);
	if (pT->IsOrthonormalQuick() != S_OK) 
		m_hOrtho = S_FALSE;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::MultiplyEx(ITransform *pT, FWULONG mode)
{
	FWMATRIX M;
	pT->AsMatrix(M);

	switch (mode)
	{
	case TRANS_REGULAR:
	case TRANS_LEFT_SIDE:
		_Multiply(M, A);
		break;
	case TRANS_INVERTED:
	case TRANS_LEFT_SIDE | TRANS_INVERTED:
		_MultiplyInverted(M, A);
		break;
	case TRANS_RIGHT_SIDE:
		_MultiplyR(A, M);
		break;
	case TRANS_RIGHT_SIDE_INVERTED:
		_MultiplyInvertedR(A, M);
		break;
	case TRANS_BOTH_SIDES:
		_Multiply(M, A);
		_MultiplyInvertedR(A, M);
		break;
	case TRANS_BOTH_SIDES_INVERTED:
		_MultiplyInverted(M, A);
		_MultiplyR(A, M);
		break;
	default:
		return ERROR(FW_E_INVALIDARG);
		break;
	}
	if (pT->IsOrthonormalQuick() != S_OK)
		m_hOrtho = S_FALSE;
	return S_OK;
}


///////////////////////////////////////////////////////////
//
// ITransform implementation: conversions to other types

HRESULT __stdcall CTransMatrix::AsVector(FWVECTOR *pV)
{
	assert(pV);
	pV->x = (FWFLOAT)A[3][0]; pV->y = (FWFLOAT)A[3][1]; pV->z = (FWFLOAT)A[3][2];
	return S_OK;
}

HRESULT __stdcall CTransMatrix::AsMatrix(FWMATRIX M)
{
	memcpy(M, A, sizeof(FWMATRIX));
	return S_OK;
}

HRESULT __stdcall CTransMatrix::AsMatrixT(FWMATRIX M)
{
	M[0][0] = A[0][0]; M[1][0] = A[0][1]; M[2][0] = A[0][2]; M[3][0] = A[0][3]; 
	M[0][1] = A[1][0]; M[1][1] = A[1][1]; M[2][1] = A[1][2]; M[3][1] = A[1][3]; 
	M[0][2] = A[2][0]; M[1][2] = A[2][1]; M[2][2] = A[2][2]; M[3][2] = A[2][3]; 
	M[0][3] = A[3][0]; M[1][3] = A[3][1]; M[2][3] = A[3][2]; M[3][3] = A[3][3]; 
	return S_OK;
}

HRESULT __stdcall CTransMatrix::AsMatrix12(FWMATRIX12 M12)
{
	FWDOUBLE (*M)[3] = (FWDOUBLE(*)[3])M12;
	M[0][0] = A[0][0]; M[1][0] = A[1][0]; M[2][0] = A[2][0]; M[3][0] = A[3][0]; 
	M[0][1] = A[0][1]; M[1][1] = A[1][1]; M[2][1] = A[2][1]; M[3][1] = A[3][1]; 
	M[0][2] = A[0][2]; M[1][2] = A[1][2]; M[2][2] = A[2][2]; M[3][2] = A[3][2]; 
	return S_OK;
}

HRESULT __stdcall CTransMatrix::AsMatrix12T(FWMATRIX12 M12)
{
	FWDOUBLE (*M)[4] = (FWDOUBLE(*)[4])M12;
	M[0][0] = A[0][0]; M[0][1] = A[1][0]; M[0][2] = A[2][0]; M[0][3] = A[3][0]; 
	M[1][0] = A[0][1]; M[1][1] = A[1][1]; M[1][2] = A[2][1]; M[1][3] = A[3][1]; 
	M[2][0] = A[0][2]; M[2][1] = A[1][2]; M[2][2] = A[2][2]; M[2][3] = A[3][2]; 
	return S_OK;
}

HRESULT __stdcall CTransMatrix::AsMatrix9(FWMATRIX9 M9)
{
	M9[0][0] = A[0][0]; M9[1][0] = A[1][0]; M9[2][0] = A[2][0];
	M9[0][1] = A[0][1]; M9[1][1] = A[1][1]; M9[2][1] = A[2][1];
	M9[0][2] = A[0][2]; M9[1][2] = A[1][2]; M9[2][2] = A[2][2];
	return S_OK;
}

HRESULT __stdcall CTransMatrix::AsMatrix9T(FWMATRIX9 M9)
{
	M9[0][0] = A[0][0]; M9[1][0] = A[0][1]; M9[2][0] = A[0][2];
	M9[0][1] = A[1][0]; M9[1][1] = A[1][1]; M9[2][1] = A[1][2];
	M9[0][2] = A[2][0]; M9[1][2] = A[2][1]; M9[2][2] = A[2][2];
	return S_OK;
}

/** This function code provided by Marek Mittmann 20/09/2005 **/
HRESULT __stdcall CTransMatrix::AsEuler(FWEULER *pR)
{
// Poprawka 8/12/2009 - I don't know how reliable it is, in terms of these 0.002 .. 0.998 ranges...
//	if (A[2][1] < FWDOUBLE(0.998))
	if (A[2][1] < FWDOUBLE(0.002))
	{
		pR->yaw = atan2(A[1][0], A[0][0]);
		pR->pitch = (FWDOUBLE)M_PI_2;
		pR->roll = FWDOUBLE(0.0);
	}
	else if (A[2][1] > FWDOUBLE(0.998))
	{
		pR->yaw = atan2(A[1][0], A[0][0]);
		pR->pitch = -(FWDOUBLE)M_PI_2;
		pR->roll = FWDOUBLE(0.0);
	}
	else
	{
		pR->yaw = atan2(A[2][0], A[2][2]); 
		pR->roll = atan2(A[0][1], A[1][1]); 
		pR->pitch = asin(-A[2][1]);
	}
	return S_OK;
}

HRESULT __stdcall CTransMatrix::AsAxisAngle(FWVECTOR *p, FWDOUBLE *pfAngle)
{
	return ERROR(E_NOTIMPL);
}

HRESULT __stdcall CTransMatrix::AsQuat(FWQUAT *pQ)
{
//	assert(A[0][0] + A[1][1] + A[2][2] >= -1.00001);
	FWDOUBLE s = (1 + A[0][0] + A[1][1] + A[2][2]) / 4;
	pQ->w = (s >= 0) ? sqrt(s) : 0;
	if (s > 0.000001)
	{
		// simpler formulas
		FWDOUBLE c = 0.25f / pQ->w;
		pQ->x = c * (A[1][2] - A[2][1]);
		pQ->y = c * (A[2][0] - A[0][2]);
		pQ->z = c * (A[0][1] - A[1][0]);
	}
	else
	{
		// more complicated formulas to avoid division by zero...
		pQ->x = (A[0][0] < -1.0f) ? 0.0f : sqrt((A[0][0] + 1 - s - s) / 2.0f);
		pQ->y = (A[1][1] < -1.0f) ? 0.0f : sqrt((A[1][1] + 1 - s - s) / 2.0f);
		pQ->z = (A[2][2] < -1.0f) ? 0.0f : sqrt((A[2][2] + 1 - s - s) / 2.0f);
	}

	return S_OK;
}

HRESULT __stdcall CTransMatrix::AsTransform(ITransform *p)
{
	assert(p);
	return p->FromTransform(this);
}

HRESULT __stdcall CTransMatrix::AsInvertion(ITransform *pT)
{
	HRESULT h = AsTransform(pT);
	if (FAILED(h)) return h;
	return pT->Inverse();
}

///////////////////////////////////////////////////////////
//
// ITransform implementation: operations

HRESULT __stdcall CTransMatrix::ApplyTo(FWVECTOR *pVector)
{
	FWDOUBLE x =  pVector->x * A[0][0]
				+ pVector->y * A[1][0]
				+ pVector->z * A[2][0]
				+ A[3][0];
	FWDOUBLE y =  pVector->x * A[0][1]
				+ pVector->y * A[1][1]
				+ pVector->z * A[2][1]
				+ A[3][1];
	FWDOUBLE z =  pVector->x * A[0][2]
				+ pVector->y * A[1][2]
				+ pVector->z * A[2][2]
				+ A[3][2];
	pVector->x = (FWFLOAT)x;
	pVector->y = (FWFLOAT)y;
	pVector->z = (FWFLOAT)z;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::ApplyInvertionTo(FWVECTOR *pVector)
{
	ITransform *pT = (ITransform*)Clone(IID_ITransform);
	AsInvertion(pT);
	HRESULT h = pT->ApplyTo(pVector);
	pT->Release();
	return h;
}

HRESULT __stdcall CTransMatrix::ApplyTranslationTo(FWVECTOR *pVector)
{
	A[3][0] += pVector->x;
	A[3][1] += pVector->y;
	A[3][2] += pVector->z;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::ApplyRotationTo(FWVECTOR *pVector)
{
	FWDOUBLE x =  pVector->x * A[0][0]
				+ pVector->y * A[1][0]
				+ pVector->z * A[2][0];
	FWDOUBLE y =  pVector->x * A[0][1]
				+ pVector->y * A[1][1]
				+ pVector->z * A[2][1];
	FWDOUBLE z =  pVector->x * A[0][2]
				+ pVector->y * A[1][2]
				+ pVector->z * A[2][2];
	pVector->x = (FWFLOAT)x;
	pVector->y = (FWFLOAT)y;
	pVector->z = (FWFLOAT)z;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::Reset(BOOL bZeroRotScale, BOOL bZeroTranslate)
{
	A[0][3] = A[1][3] = A[2][3] = 0.0f;
	A[3][3] = 1.0f;
	if (bZeroRotScale)
	{
		A[1][0] = A[2][0] = A[0][1] = A[2][1] = A[0][2] = A[1][2] = 0.0f;
		A[0][0] = A[1][1] = A[2][2] = 1.0f;
	}
	if (bZeroTranslate)
		A[3][0] = A[3][1] = A[3][2] = 0.0f;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::Transpose()
{
	FWDOUBLE fAux;
	fAux = A[1][0]; A[1][0] = A[0][1]; A[0][1] = fAux;
	fAux = A[2][0]; A[2][0] = A[0][2]; A[0][2] = fAux;
	fAux = A[3][0]; A[3][0] = A[0][3]; A[0][3] = fAux;
	fAux = A[2][1]; A[2][1] = A[1][2]; A[1][2] = fAux;
	fAux = A[3][1]; A[3][1] = A[1][3]; A[1][3] = fAux;
	fAux = A[3][2]; A[3][2] = A[2][3]; A[2][3] = fAux;
	return S_OK;
}

HRESULT __stdcall CTransMatrix::Inverse()
{
	if (IsOrthonormal() == S_OK)
		_InverseOrthogonal(A);
	else
		_Inverse(A);
	return S_OK;
}

/*#define quat FWQUAT
quat slerp(quat qa, quat qb, double t) {
	// quaternion to return
	quat qm;;// = new quat();
	// Calculate angle between them.
	double cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (abs(cosHalfTheta) >= 1.0){
		qm.w = qa.w;qm.x = qa.x;qm.y = qa.y;qm.z = qa.z;
		return qm;
	}
	// Calculate temporary values.
	double halfTheta = acos(cosHalfTheta);
	double sinHalfTheta = sqrt(1.0 - cosHalfTheta*cosHalfTheta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	if (fabs(sinHalfTheta) < 0.001){ // fabs is floating point absolute
		qm.w = (qa.w * 0.5 + qb.w * 0.5);
		qm.x = (qa.x * 0.5 + qb.x * 0.5);
		qm.y = (qa.y * 0.5 + qb.y * 0.5);
		qm.z = (qa.z * 0.5 + qb.z * 0.5);
		return qm;
	}
	double ratioA = sin((1 - t) * halfTheta) / sinHalfTheta;
	double ratioB = sin(t * halfTheta) / sinHalfTheta; 
	//calculate Quaternion.
	qm.w = (qa.w * ratioA + qb.w * ratioB);
	qm.x = (qa.x * ratioA + qb.x * ratioB);
	qm.y = (qa.y * ratioA + qb.y * ratioB);
	qm.z = (qa.z * ratioA + qb.z * ratioB);
	return qm;
}
*/

HRESULT __stdcall CTransMatrix::Interpolate(ITransform *pSrc, ITransform *pDest, FWDOUBLE fPhase)
{
	if (fPhase < 0.0001)
		FromRotationTransform(pSrc);	// changed on 17/04/2012 (was: FromTransform)
	else
	if (fPhase > 0.9999)
		FromRotationTransform(pDest);	// changed on 17/04/2012 (was: FromTransform)
	else
	{
		FWQUAT Q, Q1, Q2;
		pSrc->AsQuat(&Q1);
		pDest->AsQuat(&Q2);

		//Q = slerp(Q1, Q2, fPhase);

		// szukamy cos theta
		double cost = Q1.w * Q2.w + Q1.x * Q2.x + Q1.y * Q2.y + Q1.z * Q2.z;

		// ensure the shortest path...
		if (cost < 0)
		{
			Q1.w = -Q1.w; Q1.x = -Q1.x; Q1.y = -Q1.y; Q1.z = -Q1.z; cost = -cost; 
		}

		double f1;
		double f2;
		if (cost >= 0.99999)
		{
			f1 = 0.0;
			f2 = 1.0;
		}
		else
		{
			// szukamy sinusów
			double theta = acos(cost);
			double sint = sin(theta);
			f1 = sin(theta * (1 - fPhase)) / sint;
			f2 = sin(theta * fPhase) / sint;
		}

		Q.x = (FWDOUBLE)(f1 * Q1.x + f2 * Q2.x);
		Q.y = (FWDOUBLE)(f1 * Q1.y + f2 * Q2.y);
		Q.z = (FWDOUBLE)(f1 * Q1.z + f2 * Q2.z);
		Q.w = (FWDOUBLE)(f1 * Q1.w + f2 * Q2.w);

		HRESULT hOrtho = (pSrc->IsOrthonormalQuick() == S_OK && pDest->IsOrthonormalQuick() == S_OK)
			? S_OK : S_FALSE;

		FromRotationQuat(&Q);

		Orthonormalize();

		m_hOrtho = hOrtho;
		return m_hOrtho;
	}
	return S_OK;
}

///////////////////////////////////////////////////////////
//
// ITransform implementation: implementation dependent items

STDMETHODIMP CTransMatrix::GetItem(FWULONG nIndex, FWDOUBLE *pVal)
{
	FWULONG nCol = nIndex / 4;
	FWULONG nRow = nIndex % 4;
	*pVal = A[nCol][nRow];
	return S_OK;
}

STDMETHODIMP CTransMatrix::PutItem(FWULONG nIndex, FWDOUBLE newVal)
{
	FWULONG nCol = nIndex / 4;
	FWULONG nRow = nIndex % 4;
	if (nIndex < 0 || nRow >= 3 || nCol > 3)
		return ERROR(FW_E_INVALIDARG);
	A[nCol][nRow] = newVal;
	return S_OK;
}
