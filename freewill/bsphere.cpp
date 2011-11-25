// bsphere.cpp : Defines the Bounding object
//

#include "stdafx.h"
#include "bsphere.h"
#include <math.h>

///////////////////////////////////////////////////////////
// Convenience Functions: elementary matrix operations without use of ITransform

inline static void _Inverse(FWMATRIX M)
{
	FWDOUBLE fAux;
	fAux = M[1][0]; M[1][0] = M[0][1]; M[0][1] = fAux;
	fAux = M[2][0]; M[2][0] = M[0][2]; M[0][2] = fAux;
	fAux = M[2][1]; M[2][1] = M[1][2]; M[1][2] = fAux;
	FWDOUBLE A30 = -M[0][0] * M[3][0] - M[1][0] * M[3][1] - M[2][0] * M[3][2];
	FWDOUBLE A31 = -M[0][1] * M[3][0] - M[1][1] * M[3][1] - M[2][1] * M[3][2];
	FWDOUBLE A32 = -M[0][2] * M[3][0] - M[1][2] * M[3][1] - M[2][2] * M[3][2];
	M[3][0] = A30;
	M[3][1] = A31;
	M[3][2] = A32;
}

inline static void _Transform(FWVECTOR *pVector, FWMATRIX M)
{
	FWDOUBLE x =	  pVector->x * M[0][0]
				+ pVector->y * M[1][0]
				+ pVector->z * M[2][0]
				+ M[3][0];
	FWDOUBLE y =	  pVector->x * M[0][1]
				+ pVector->y * M[1][1]
				+ pVector->z * M[2][1]
				+ M[3][1];
	FWDOUBLE z =	  pVector->x * M[0][2]
				+ pVector->y * M[1][2]
				+ pVector->z * M[2][2]
				+ M[3][2];
	pVector->x = (FWFLOAT)x;
	pVector->y = (FWFLOAT)y;
	pVector->z = (FWFLOAT)z;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CBoundingSphere

CBoundingSphere::CBoundingSphere() 
{
}

CBoundingSphere::~CBoundingSphere() 
{
}

///////////////////////////////////////////////////////////
// IKineNode::CreateChild

HRESULT CBoundingSphere::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	return ERROR(E_NOTIMPL);
}

///////////////////////////////////////////////////////////
// Data Transfer

HRESULT CBoundingSphere::PutData(enum BOUND_FORMAT nFormat, FWULONG nSize, /*[in, size_is(nSize)]*/ BYTE *pBuf)
{
#define BUFAABB		((struct BOUND_AABB*)pBuf)
#define BUFOBB		((struct BOUND_OBB*)pBuf)
#define BUFOBB_8	((struct BOUND_OBB_8*)pBuf)
#define BUFSPHERE	((struct BOUND_SPHERE*)pBuf)
#define BUFCYLINDER	((struct BOUND_CYLINDER*)pBuf)

	switch (nFormat)
	{
		case BOUND_FORMAT_AABB:
			if (nSize != sizeof(BOUND_AABB)) return ERROR(FW_E_FORMAT);
			m_sphere.vecCenter.x = (BUFAABB->vecMin.x + BUFAABB->vecMax.x) * 0.5f;
			m_sphere.vecCenter.y = (BUFAABB->vecMin.y + BUFAABB->vecMax.y) * 0.5f;
			m_sphere.vecCenter.z = (BUFAABB->vecMin.z + BUFAABB->vecMax.z) * 0.5f;
			m_sphere.fRadius = sqrt(
				(BUFAABB->vecMax.x - BUFAABB->vecMin.x) * (BUFAABB->vecMax.x - BUFAABB->vecMin.x) +
				(BUFAABB->vecMax.y - BUFAABB->vecMin.y) * (BUFAABB->vecMax.y - BUFAABB->vecMin.y) +
				(BUFAABB->vecMax.z - BUFAABB->vecMin.z) * (BUFAABB->vecMax.z - BUFAABB->vecMin.z)) * 0.5f;
			break;

		case BOUND_FORMAT_OBB:
			if (nSize != sizeof(BOUND_OBB)) return ERROR(FW_E_FORMAT);
			m_sphere.vecCenter.x = (FWFLOAT)BUFOBB->M[3][0];
			m_sphere.vecCenter.y = (FWFLOAT)BUFOBB->M[3][1];
			m_sphere.vecCenter.z = (FWFLOAT)BUFOBB->M[3][2];
			m_sphere.fRadius = sqrt(BUFOBB->vecSize.x * BUFOBB->vecSize.x + BUFOBB->vecSize.y * BUFOBB->vecSize.y + BUFOBB->vecSize.z * BUFOBB->vecSize.z);
			break;

		case BOUND_FORMAT_OBB_8:
			if (nSize != sizeof(BOUND_OBB_8)) return ERROR(FW_E_FORMAT);
			m_sphere.vecCenter.x = (BUFOBB_8->OBB[0].x + BUFOBB_8->OBB[7].x) * 0.5f;
			m_sphere.vecCenter.y = (BUFOBB_8->OBB[0].y + BUFOBB_8->OBB[7].y) * 0.5f;
			m_sphere.vecCenter.z = (BUFOBB_8->OBB[0].z + BUFOBB_8->OBB[7].z) * 0.5f;
			m_sphere.fRadius = sqrt(
				(BUFOBB_8->OBB[7].x - BUFOBB_8->OBB[0].x) * (BUFOBB_8->OBB[7].x - BUFOBB_8->OBB[0].x) +
				(BUFOBB_8->OBB[7].y - BUFOBB_8->OBB[0].y) * (BUFOBB_8->OBB[7].y - BUFOBB_8->OBB[0].y) +
				(BUFOBB_8->OBB[7].z - BUFOBB_8->OBB[0].z) * (BUFOBB_8->OBB[7].z - BUFOBB_8->OBB[0].z)) * 0.5f;
			break;

		case BOUND_FORMAT_SPHERE:
			if (nSize != sizeof(BOUND_SPHERE)) return ERROR(FW_E_FORMAT);
			memcpy(&m_sphere, BUFSPHERE, sizeof(m_sphere));
			break;

		case BOUND_FORMAT_CYLINDER:
			if (nSize != sizeof(BOUND_CYLINDER)) return ERROR(FW_E_FORMAT);
			m_sphere.vecCenter.x = (BUFCYLINDER->vecPivot1.x + BUFCYLINDER->vecPivot2.x) * 0.5f;
			m_sphere.vecCenter.y = (BUFCYLINDER->vecPivot1.y + BUFCYLINDER->vecPivot2.y) * 0.5f;
			m_sphere.vecCenter.z = (BUFCYLINDER->vecPivot1.z + BUFCYLINDER->vecPivot2.z) * 0.5f;
			m_sphere.fRadius = sqrt(
				(BUFCYLINDER->vecPivot2.x - BUFCYLINDER->vecPivot1.x) * (BUFCYLINDER->vecPivot2.x - BUFCYLINDER->vecPivot1.x) +
				(BUFCYLINDER->vecPivot2.y - BUFCYLINDER->vecPivot1.y) * (BUFCYLINDER->vecPivot2.y - BUFCYLINDER->vecPivot1.y) +
				(BUFCYLINDER->vecPivot2.z - BUFCYLINDER->vecPivot1.z) * (BUFCYLINDER->vecPivot2.z - BUFCYLINDER->vecPivot1.z)) * 0.5f;
			m_sphere.fRadius = sqrt(m_sphere.fRadius * m_sphere.fRadius + BUFCYLINDER->fRadius * BUFCYLINDER->fRadius);
			break;

		default:
			return ERROR(FW_E_FORMAT);
	}

	ITransform *pT = NULL;
	CreateCompatibleTransform(&pT);
	HRESULT h = GetGlobalTransform(pT);
	if (FAILED(h)) return ERROR(h);
	pT->Inverse();
	pT->ApplyTo(&m_sphere.vecCenter);
	pT->Release();
	return S_OK;
}

HRESULT CBoundingSphere::GetData(enum BOUND_FORMAT nFormat, FWULONG nSize, /*[out, size_is(nSize)]*/ BYTE *pBuf)
{
#define BUFAABB		((struct BOUND_AABB*)pBuf)
#define BUFOBB		((struct BOUND_OBB*)pBuf)
#define BUFOBB_8	((struct BOUND_OBB_8*)pBuf)
#define BUFSPHERE	((struct BOUND_SPHERE*)pBuf)
#define BUFCYLINDER	((struct BOUND_CYLINDER*)pBuf)

	FWVECTOR vecCenter;
	FWFLOAT fRadius = m_sphere.fRadius;

	memcpy(&vecCenter, &m_sphere.vecCenter, sizeof(FWVECTOR));
	ITransform *pT = NULL;
	CreateCompatibleTransform(&pT);
	HRESULT h = GetGlobalTransform(pT);
	if (FAILED(h)) return ERROR(h);
	pT->ApplyTo(&vecCenter);
	pT->Release();

	switch (nFormat)
	{
		case BOUND_FORMAT_AABB:
			if (nSize != sizeof(BOUND_AABB)) return ERROR(FW_E_FORMAT);
			BUFAABB->vecMin.x = vecCenter.x - fRadius;
			BUFAABB->vecMin.y = vecCenter.y - fRadius;
			BUFAABB->vecMin.z = vecCenter.z - fRadius;
			BUFAABB->vecMax.x = vecCenter.x + fRadius;
			BUFAABB->vecMax.y = vecCenter.y + fRadius;
			BUFAABB->vecMax.z = vecCenter.z + fRadius;
			break;

		case BOUND_FORMAT_OBB:
			if (nSize != sizeof(BOUND_OBB)) return ERROR(FW_E_FORMAT);
			BUFOBB->vecSize.x = BUFOBB->vecSize.y = BUFOBB->vecSize.z = fRadius;
			memset(BUFOBB->M, 0, sizeof(BUFOBB->M));
			BUFOBB->M[3][0] = vecCenter.x;
			BUFOBB->M[3][1] = vecCenter.y;
			BUFOBB->M[3][2] = vecCenter.z;
			BUFOBB->M[0][0] = BUFOBB->M[1][1] = BUFOBB->M[2][2] = BUFOBB->M[3][3] = 1.0f;
			break;

		case BOUND_FORMAT_OBB_8:
			BUFOBB_8->OBB[0].x = vecCenter.x - fRadius;	BUFOBB_8->OBB[0].y = vecCenter.y - fRadius;	BUFOBB_8->OBB[0].z = vecCenter.z - fRadius;
			BUFOBB_8->OBB[1].x = vecCenter.x + fRadius;	BUFOBB_8->OBB[1].y = vecCenter.y - fRadius;	BUFOBB_8->OBB[1].z = vecCenter.z - fRadius;
			BUFOBB_8->OBB[2].x = vecCenter.x - fRadius;	BUFOBB_8->OBB[2].y = vecCenter.y - fRadius;	BUFOBB_8->OBB[2].z = vecCenter.z + fRadius;
			BUFOBB_8->OBB[3].x = vecCenter.x + fRadius;	BUFOBB_8->OBB[3].y = vecCenter.y - fRadius;	BUFOBB_8->OBB[3].z = vecCenter.z + fRadius;
			BUFOBB_8->OBB[4].x = vecCenter.x - fRadius;	BUFOBB_8->OBB[4].y = vecCenter.y + fRadius;	BUFOBB_8->OBB[4].z = vecCenter.z - fRadius;
			BUFOBB_8->OBB[5].x = vecCenter.x + fRadius;	BUFOBB_8->OBB[5].y = vecCenter.y + fRadius;	BUFOBB_8->OBB[5].z = vecCenter.z - fRadius;
			BUFOBB_8->OBB[6].x = vecCenter.x - fRadius;	BUFOBB_8->OBB[6].y = vecCenter.y + fRadius;	BUFOBB_8->OBB[6].z = vecCenter.z + fRadius;
			BUFOBB_8->OBB[7].x = vecCenter.x + fRadius;	BUFOBB_8->OBB[7].y = vecCenter.y + fRadius;	BUFOBB_8->OBB[7].z = vecCenter.z + fRadius;
			break;

		case BOUND_FORMAT_SPHERE:
			if (nSize != sizeof(BOUND_SPHERE)) return ERROR(FW_E_FORMAT);
			memcpy(&BUFSPHERE->vecCenter, &vecCenter, sizeof(FWVECTOR));
			BUFSPHERE->fRadius = fRadius;
			break;

		case BOUND_FORMAT_CYLINDER:
			if (nSize != sizeof(BOUND_CYLINDER)) return ERROR(FW_E_FORMAT);
			BUFCYLINDER->vecPivot1.x = BUFCYLINDER->vecPivot2.x = vecCenter.x;
			BUFCYLINDER->vecPivot1.z = BUFCYLINDER->vecPivot2.z = vecCenter.z;
			BUFCYLINDER->vecPivot1.y = vecCenter.y - fRadius;
			BUFCYLINDER->vecPivot2.y = vecCenter.y + fRadius;
			BUFCYLINDER->fRadius = fRadius;
			break;

		default:
			return ERROR(FW_E_FORMAT);
	}

	return S_OK;
}

///////////////////////////////////////////////////////////
// Query about Supported Formats

	HRESULT CBoundingSphere::_InternalQueryInputFormat(enum BOUND_FORMAT fmt, enum BOUND_PREFERENCE *pPref)
	{
		switch (fmt)
		{
			case BOUND_FORMAT_AABB:		if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_OBB:		if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_OBB_8:	if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_SPHERE:	if (pPref) *pPref = BOUND_PREF_SUPPORTED; return S_OK;
			case BOUND_FORMAT_CYLINDER:	if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			default:					if (pPref) *pPref = BOUND_PREF_UNSUPPORTED; return S_FALSE;
		}
	}

	HRESULT CBoundingSphere::_InternalQueryOutputFormat(enum BOUND_FORMAT fmt, enum BOUND_PREFERENCE *pPref)
	{
		switch (fmt)
		{
			case BOUND_FORMAT_AABB:		if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_OBB:		if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_OBB_8:	if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_SPHERE:	if (pPref) *pPref = BOUND_PREF_SUPPORTED; return S_OK;
			case BOUND_FORMAT_CYLINDER:	if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			default:					if (pPref) *pPref = BOUND_PREF_UNSUPPORTED; return S_FALSE;
		}
	}

HRESULT CBoundingSphere::QueryInputFormat(enum BOUND_FORMAT fmt, /*[out, retval]*/ enum BOUND_PREFERENCE *pPref)
{
	return _InternalQueryInputFormat(fmt, pPref);
}

HRESULT CBoundingSphere::QueryOutputFormat(enum BOUND_FORMAT fmt, /*[out, retval]*/ enum BOUND_PREFERENCE *pPref)
{
	return _InternalQueryOutputFormat(fmt, pPref);
}

HRESULT CBoundingSphere::QueryInputFormatEx(FWULONG nLen, /*[in, size_is(nLen)]*/ enum BOUND_FORMAT *pFmt, /*[out, size_is(nLen)]*/ enum BOUND_PREFERENCE *pPref)
{
	if (!pFmt || !pPref) return ERROR(FW_E_POINTER);
	for (FWULONG i = 0; i < nLen; i++, pFmt++, pPref++)
		_InternalQueryInputFormat(*pFmt, pPref);
	return S_OK;
}

HRESULT CBoundingSphere::QueryOutputFormatEx(FWULONG nLen, /*[in, size_is(nLen)]*/ enum BOUND_FORMAT *pFmt, /*[out, size_is(nLen)]*/ enum BOUND_PREFERENCE *pPref)
{
	if (!pFmt || !pPref) return ERROR(FW_E_POINTER);
	for (FWULONG i = 0; i < nLen; i++, pFmt++, pPref++)
		_InternalQueryOutputFormat(*pFmt, pPref);
	return S_OK;
}

HRESULT CBoundingSphere::QueryDetect(IBounding *pWith, /*[out]*/ enum BOUND_FORMAT *pFmt, /*[out]*/ enum BOUND_PREFERENCE *pPref, /*[out]*/ enum BOUND_PRECISION *pPrec)
{
	if (!pWith) return ERROR(FW_E_POINTER);
	BOUND_FORMAT fmt[2] = { BOUND_FORMAT_SPHERE, BOUND_FORMAT_OBB };
	BOUND_PRECISION prec[2] = { BOUND_PREC_TIGHT, BOUND_PREC_TIGHT };
	BOUND_PREFERENCE pref[2];

	pWith->QueryOutputFormatEx(2, fmt, pref);
	FWULONG i = (pref[1] > pref[0]) ? 1 : 0;
	if (pFmt) *pFmt = fmt[i];
	if (pPref) *pPref = pref[i];
	if (pPrec) *pPrec = prec[i];
	return pref[i] == BOUND_PREF_UNSUPPORTED ? S_FALSE : S_OK;
}

///////////////////////////////////////////////////////////
// Collision Detection Test

HRESULT CBoundingSphere::Detect(IBounding *pWith)
{
	BOUND_FORMAT fmt_my, fmt_your;
	BOUND_PREFERENCE pref_my, pref_your;
	BOUND_PRECISION prec_my, prec_your;
	bool bMe;

	QueryDetect(pWith, &fmt_my, &pref_my, &prec_my);
	QueryDetect(pWith, &fmt_your, &pref_your, &prec_your);

	bMe = (FWULONG)pref_my >= (FWULONG)pref_your;
	if (pref_my == pref_your) bMe = (FWULONG)prec_my >= (FWULONG)prec_your;

	if (bMe)
		return DetectEx(pWith, fmt_my);
	else
		return pWith->DetectEx(this, fmt_your);
}

HRESULT CBoundingSphere::DetectEx(IBounding *pWith, BOUND_FORMAT fmt)
{
	if (fmt == BOUND_FORMAT_SPHERE)
	{
		BOUND_SPHERE sphere1, sphere2;

		HRESULT h;
		h = GetData(BOUND_FORMAT_SPHERE, sizeof(BOUND_SPHERE), (BYTE*)&sphere1);
		if (FAILED(h)) return h;
		h = pWith->GetData(BOUND_FORMAT_SPHERE, sizeof(BOUND_SPHERE), (BYTE*)&sphere2);
		if (FAILED(h)) return h;

		// collision test
		if ((sphere1.vecCenter.x - sphere2.vecCenter.x) * (sphere1.vecCenter.x - sphere2.vecCenter.x) +
			(sphere1.vecCenter.y - sphere2.vecCenter.y) * (sphere1.vecCenter.y - sphere2.vecCenter.y) +
			(sphere1.vecCenter.z - sphere2.vecCenter.z) * (sphere1.vecCenter.z - sphere2.vecCenter.z)
			 > (sphere1.fRadius + sphere2.fRadius) * (sphere1.fRadius + sphere2.fRadius))
			return S_OK;
		else
			return S_FALSE;
	}
	else
	if (fmt == BOUND_FORMAT_OBB) 
	{
		BOUND_SPHERE sphere;
		BOUND_OBB obb;

		HRESULT h;
		h = GetData(BOUND_FORMAT_SPHERE, sizeof(BOUND_SPHERE), (BYTE*)&sphere);
		if (FAILED(h)) return h;
		h = pWith->GetData(BOUND_FORMAT_OBB, sizeof(BOUND_OBB), (BYTE*)&obb);
		if (FAILED(h)) return h;

		// collision test

		// put sphere into coordinates where the obb is axis-aligned and located in the center
		_Inverse(obb.M);
		_Transform(&sphere.vecCenter, obb.M);

		// test along 3 axes
		FWFLOAT r = sphere.fRadius;
		FWFLOAT x = fabs(sphere.vecCenter.x) - obb.vecSize.x; if (x < 0.0f) x = 0.0f;
		FWFLOAT y = fabs(sphere.vecCenter.y) - obb.vecSize.y; if (y < 0.0f) y = 0.0f;
		FWFLOAT z = fabs(sphere.vecCenter.z) - obb.vecSize.z; if (z < 0.0f) z = 0.0f;

		if (x * x + y * y + z * z > r * r)
			return S_OK;
		else
			return S_FALSE;
	}
	else
		return ERROR(FW_E_FORMAT);
}

