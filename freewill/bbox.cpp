// bbox.cpp : Defines the Bounding Box object
//

#include "stdafx.h"
#include "bbox.h"
#include <math.h>

///////////////////////////////////////////////////////////
// Convenience Functions: elementary matrix operations without use of ITransform

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

inline static void _Multiply(FWMATRIX M, FWMATRIX M1, FWMATRIX M2)	// M = M1 x M2,	  M != M2
{
	assert(M2[0][3] == 0 && M2[1][3] == 0 && M2[2][3] == 0 && M2[3][3] == 1);
	assert(M1[0][3] == 0 && M1[1][3] == 0 && M1[2][3] == 0 && M1[3][3] == 1);

	FWDOUBLE x0, x1, x2;

	x0 = M2[0][0] * M1[0][0] + M2[1][0] * M1[0][1] + M2[2][0] * M1[0][2];
	x1 = M2[0][1] * M1[0][0] + M2[1][1] * M1[0][1] + M2[2][1] * M1[0][2];
	x2 = M2[0][2] * M1[0][0] + M2[1][2] * M1[0][1] + M2[2][2] * M1[0][2];
	M[0][0] = x0; M[0][1] = x1; M[0][2] = x2; M[0][3] = 0;

	x0 = M2[0][0] * M1[1][0] + M2[1][0] * M1[1][1] + M2[2][0] * M1[1][2];
	x1 = M2[0][1] * M1[1][0] + M2[1][1] * M1[1][1] + M2[2][1] * M1[1][2];
	x2 = M2[0][2] * M1[1][0] + M2[1][2] * M1[1][1] + M2[2][2] * M1[1][2];
	M[1][0] = x0; M[1][1] = x1; M[1][2] = x2; M[1][3] = 0;

	x0 = M2[0][0] * M1[2][0] + M2[1][0] * M1[2][1] + M2[2][0] * M1[2][2];
	x1 = M2[0][1] * M1[2][0] + M2[1][1] * M1[2][1] + M2[2][1] * M1[2][2];
	x2 = M2[0][2] * M1[2][0] + M2[1][2] * M1[2][1] + M2[2][2] * M1[2][2];
	M[2][0] = x0; M[2][1] = x1; M[2][2] = x2; M[2][3] = 0;

	x0 = M2[0][0] * M1[3][0] + M2[1][0] * M1[3][1] + M2[2][0] * M1[3][2] + M2[3][0];
	x1 = M2[0][1] * M1[3][0] + M2[1][1] * M1[3][1] + M2[2][1] * M1[3][2] + M2[3][1];
	x2 = M2[0][2] * M1[3][0] + M2[1][2] * M1[3][1] + M2[2][2] * M1[3][2] + M2[3][2];
	M[3][0] = x0; M[3][1] = x1; M[3][2] = x2; M[3][3] = 1;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CBoundingBox

CBoundingBox::CBoundingBox() 
{
}

CBoundingBox::~CBoundingBox() 
{
}

///////////////////////////////////////////////////////////
// IKineNode::CreateChild

HRESULT CBoundingBox::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	return ERROR(E_NOTIMPL);
}

///////////////////////////////////////////////////////////
// Data Transfer

HRESULT CBoundingBox::PutData(enum BOUND_FORMAT nFormat, FWULONG nSize, /*[in, size_is(nSize)]*/ BYTE *pBuf)
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
			m_obb.vecSize.x = fabs(BUFAABB->vecMax.x - BUFAABB->vecMin.x) * 0.5f;
			m_obb.vecSize.y = fabs(BUFAABB->vecMax.y - BUFAABB->vecMin.y) * 0.5f;
			m_obb.vecSize.z = fabs(BUFAABB->vecMax.z - BUFAABB->vecMin.z) * 0.5f;
			memset(m_obb.M, 0, sizeof(m_obb.M));
			m_obb.M[3][0] = (BUFAABB->vecMin.x + BUFAABB->vecMax.x) * 0.5f;
			m_obb.M[3][1] = (BUFAABB->vecMin.y + BUFAABB->vecMax.y) * 0.5f;
			m_obb.M[3][2] = (BUFAABB->vecMin.z + BUFAABB->vecMax.z) * 0.5f;
			m_obb.M[0][0] = m_obb.M[1][1] = m_obb.M[2][2] = m_obb.M[3][3] = 1.0f;
			break;

		case BOUND_FORMAT_OBB:
			if (nSize != sizeof(BOUND_OBB)) return ERROR(FW_E_FORMAT);
			memcpy(&m_obb, pBuf, sizeof(m_obb));
			break;

		case BOUND_FORMAT_OBB_8:
			if (nSize != sizeof(BOUND_OBB)) return ERROR(FW_E_FORMAT);
			return ERROR(FW_E_FORMAT);

		case BOUND_FORMAT_SPHERE:
			if (nSize != sizeof(BOUND_SPHERE)) return ERROR(FW_E_FORMAT);
			m_obb.vecSize.x = m_obb.vecSize.y = m_obb.vecSize.z = BUFSPHERE->fRadius;
			memset(m_obb.M, 0, sizeof(m_obb.M));
			m_obb.M[3][0] = BUFSPHERE->vecCenter.x;
			m_obb.M[3][1] = BUFSPHERE->vecCenter.y;
			m_obb.M[3][2] = BUFSPHERE->vecCenter.z;
			m_obb.M[0][0] = m_obb.M[1][1] = m_obb.M[2][2] = m_obb.M[3][3] = 1.0f;
			break;

		case BOUND_FORMAT_CYLINDER:
			if (nSize != sizeof(BOUND_CYLINDER)) return ERROR(FW_E_FORMAT);
			return ERROR(FW_E_FORMAT);

		default:
			return ERROR(FW_E_FORMAT);
	}

	ITransform *pT = NULL;
	CreateCompatibleTransform(&pT);
	HRESULT h = GetGlobalTransform(pT);
	if (FAILED(h)) return ERROR(h);
	pT->Inverse();
	FWMATRIX  M, M1;
	pT->AsMatrix(M1);
	pT->Release();
	_Multiply(M, m_obb.M, M1);
	memcpy(m_obb.M, M, sizeof(M));
	return S_OK;
}

HRESULT CBoundingBox::GetData(enum BOUND_FORMAT nFormat, FWULONG nSize, /*[out, size_is(nSize)]*/ BYTE *pBuf)
{
#define BUFAABB		((struct BOUND_AABB*)pBuf)
#define BUFOBB		((struct BOUND_OBB*)pBuf)
#define BUFOBB_8	((struct BOUND_OBB_8*)pBuf)
#define BUFSPHERE	((struct BOUND_SPHERE*)pBuf)
#define BUFCYLINDER	((struct BOUND_CYLINDER*)pBuf)

	FWMATRIX M, M1;
	ITransform *pT = NULL;
	CreateCompatibleTransform(&pT);
	HRESULT h = GetGlobalTransform(pT);
	if (FAILED(h)) return ERROR(h);
	pT->AsMatrix(M1);
	pT->Release();
	_Multiply(M, m_obb.M, M1);

	switch (nFormat)
	{
		case BOUND_FORMAT_AABB:
			if (nSize != sizeof(BOUND_AABB)) return ERROR(FW_E_FORMAT);
			BUFAABB->vecMin.x = -m_obb.vecSize.x;
			BUFAABB->vecMin.y = -m_obb.vecSize.y;
			BUFAABB->vecMin.z = -m_obb.vecSize.z;
			BUFAABB->vecMax.x =  m_obb.vecSize.x;
			BUFAABB->vecMax.y =  m_obb.vecSize.y;
			BUFAABB->vecMax.z =  m_obb.vecSize.z;
			_Transform(&BUFAABB->vecMin, M);
			_Transform(&BUFAABB->vecMax, M);
			break;

		case BOUND_FORMAT_OBB:
			if (nSize != sizeof(BOUND_OBB)) return ERROR(FW_E_FORMAT);
			memcpy(BUFOBB->M, M, sizeof(M));
			memcpy(&BUFOBB->vecSize, &m_obb.vecSize, sizeof(m_obb.vecSize));
			break;

		case BOUND_FORMAT_OBB_8:
			BUFOBB_8->OBB[0].x = -m_obb.vecSize.x;	BUFOBB_8->OBB[0].y = -m_obb.vecSize.y;	BUFOBB_8->OBB[0].z = -m_obb.vecSize.z;
			BUFOBB_8->OBB[1].x =  m_obb.vecSize.x;	BUFOBB_8->OBB[1].y = -m_obb.vecSize.y;	BUFOBB_8->OBB[1].z = -m_obb.vecSize.z;
			BUFOBB_8->OBB[2].x = -m_obb.vecSize.x;	BUFOBB_8->OBB[2].y = -m_obb.vecSize.y;	BUFOBB_8->OBB[2].z =  m_obb.vecSize.z;
			BUFOBB_8->OBB[3].x =  m_obb.vecSize.x;	BUFOBB_8->OBB[3].y = -m_obb.vecSize.y;	BUFOBB_8->OBB[3].z =  m_obb.vecSize.z;
			BUFOBB_8->OBB[4].x = -m_obb.vecSize.x;	BUFOBB_8->OBB[4].y =  m_obb.vecSize.y;	BUFOBB_8->OBB[4].z = -m_obb.vecSize.z;
			BUFOBB_8->OBB[5].x =  m_obb.vecSize.x;	BUFOBB_8->OBB[5].y =  m_obb.vecSize.y;	BUFOBB_8->OBB[5].z = -m_obb.vecSize.z;
			BUFOBB_8->OBB[6].x = -m_obb.vecSize.x;	BUFOBB_8->OBB[6].y =  m_obb.vecSize.y;	BUFOBB_8->OBB[6].z =  m_obb.vecSize.z;
			BUFOBB_8->OBB[7].x =  m_obb.vecSize.x;	BUFOBB_8->OBB[7].y =  m_obb.vecSize.y;	BUFOBB_8->OBB[7].z =  m_obb.vecSize.z;
			for (int i = 0; i < 8; i++)
				_Transform(BUFOBB_8->OBB + i, M);
			break;

		case BOUND_FORMAT_SPHERE:
			if (nSize != sizeof(BOUND_SPHERE)) return ERROR(FW_E_FORMAT);
			BUFSPHERE->vecCenter.x = (FWFLOAT)M[3][0];
			BUFSPHERE->vecCenter.y = (FWFLOAT)M[3][1];
			BUFSPHERE->vecCenter.z = (FWFLOAT)M[3][2];
			// _Transform not needed here (no rotation)
			BUFSPHERE->fRadius = sqrt(m_obb.vecSize.x * m_obb.vecSize.x + m_obb.vecSize.y * m_obb.vecSize.y + m_obb.vecSize.z * m_obb.vecSize.z);
			break;

		case BOUND_FORMAT_CYLINDER:
			if (nSize != sizeof(BOUND_CYLINDER)) return ERROR(FW_E_FORMAT);
			BUFCYLINDER->vecPivot1.x = BUFCYLINDER->vecPivot1.z = BUFCYLINDER->vecPivot2.x = BUFCYLINDER->vecPivot2.z = 0.0f;
			BUFCYLINDER->vecPivot1.y = -m_obb.vecSize.y;
			BUFCYLINDER->vecPivot2.y =  m_obb.vecSize.y;
			_Transform(&BUFCYLINDER->vecPivot1, M);
			_Transform(&BUFCYLINDER->vecPivot2, M);
			BUFCYLINDER->fRadius = sqrt(m_obb.vecSize.x * m_obb.vecSize.x + m_obb.vecSize.z * m_obb.vecSize.z);
			break;

		default:
			return ERROR(FW_E_FORMAT);
	}

	return S_OK;
}

///////////////////////////////////////////////////////////
// Query about Supported Formats

	HRESULT CBoundingBox::_InternalQueryInputFormat(enum BOUND_FORMAT fmt, enum BOUND_PREFERENCE *pPref)
	{
		switch (fmt)
		{
			case BOUND_FORMAT_AABB:		if (pPref) *pPref = BOUND_PREF_SUPPORTED; return S_OK;
			case BOUND_FORMAT_OBB:		if (pPref) *pPref = BOUND_PREF_SUPPORTED; return S_OK;
			case BOUND_FORMAT_OBB_8:	if (pPref) *pPref = BOUND_PREF_UNSUPPORTED; return S_OK;
			case BOUND_FORMAT_SPHERE:	if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_CYLINDER:	if (pPref) *pPref = BOUND_PREF_UNSUPPORTED; return S_OK;
			default:					if (pPref) *pPref = BOUND_PREF_UNSUPPORTED; return S_FALSE;
		}
	}

	HRESULT CBoundingBox::_InternalQueryOutputFormat(enum BOUND_FORMAT fmt, enum BOUND_PREFERENCE *pPref)
	{
		switch (fmt)
		{
			case BOUND_FORMAT_AABB:		if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_OBB:		if (pPref) *pPref = BOUND_PREF_SUPPORTED; return S_OK;
			case BOUND_FORMAT_OBB_8:	if (pPref) *pPref = BOUND_PREF_SUPPORTED; return S_OK;
			case BOUND_FORMAT_SPHERE:	if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			case BOUND_FORMAT_CYLINDER:	if (pPref) *pPref = BOUND_PREF_ACCEPTED; return S_OK;
			default:					if (pPref) *pPref = BOUND_PREF_UNSUPPORTED; return S_FALSE;
		}
	}

HRESULT CBoundingBox::QueryInputFormat(enum BOUND_FORMAT fmt, /*[out, retval]*/ enum BOUND_PREFERENCE *pPref)
{
	return _InternalQueryInputFormat(fmt, pPref);
}

HRESULT CBoundingBox::QueryOutputFormat(enum BOUND_FORMAT fmt, /*[out, retval]*/ enum BOUND_PREFERENCE *pPref)
{
	return _InternalQueryOutputFormat(fmt, pPref);
}

HRESULT CBoundingBox::QueryInputFormatEx(FWULONG nLen, /*[in, size_is(nLen)]*/ enum BOUND_FORMAT *pFmt, /*[out, size_is(nLen)]*/ enum BOUND_PREFERENCE *pPref)
{
	if (!pFmt || !pPref) return ERROR(FW_E_POINTER);
	for (FWULONG i = 0; i < nLen; i++, pFmt++, pPref++)
		_InternalQueryInputFormat(*pFmt, pPref);
	return S_OK;
}

HRESULT CBoundingBox::QueryOutputFormatEx(FWULONG nLen, /*[in, size_is(nLen)]*/ enum BOUND_FORMAT *pFmt, /*[out, size_is(nLen)]*/ enum BOUND_PREFERENCE *pPref)
{
	if (!pFmt || !pPref) return ERROR(FW_E_POINTER);
	for (FWULONG i = 0; i < nLen; i++, pFmt++, pPref++)
		_InternalQueryOutputFormat(*pFmt, pPref);
	return S_OK;
}

HRESULT CBoundingBox::QueryDetect(IBounding *pWith, /*[out]*/ enum BOUND_FORMAT *pFmt, /*[out]*/ enum BOUND_PREFERENCE *pPref, /*[out]*/ enum BOUND_PRECISION *pPrec)
{
	if (!pWith) return ERROR(FW_E_POINTER);
	BOUND_FORMAT fmt[2] = { BOUND_FORMAT_OBB_8, BOUND_FORMAT_OBB };
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

HRESULT CBoundingBox::Detect(IBounding *pWith)
{
	BOUND_FORMAT fmt_my, fmt_your;
	BOUND_PREFERENCE pref_my, pref_your;
	BOUND_PRECISION prec_my, prec_your;
	bool bMe;

	QueryDetect(pWith, &fmt_my, &pref_my, &prec_my);
	pWith->QueryDetect(this, &fmt_your, &pref_your, &prec_your);

	bMe = (FWULONG)pref_my >= (FWULONG)pref_your;
	if (pref_my == pref_your) bMe = (FWULONG)prec_my >= (FWULONG)prec_your;

	if (bMe)
		return DetectEx(pWith, fmt_my);
	else
		return pWith->DetectEx(this, fmt_your);
}


bool FaceOrthogTest(FWVECTOR &A, FWVECTOR &B, FWVECTOR obb[8]);
bool EdgeOrthogTest(FWVECTOR &A1, FWVECTOR &B1, FWVECTOR &A2, FWVECTOR &B2, FWVECTOR obb1[8], FWVECTOR obb2[8]);

HRESULT CBoundingBox::DetectEx(IBounding *pWith, BOUND_FORMAT fmt)
{
	if (fmt != BOUND_FORMAT_OBB_8 && fmt != BOUND_FORMAT_OBB) return ERROR(FW_E_FORMAT);

	BOUND_OBB_8 obb1, obb2;
	HRESULT h;
	h = GetData(BOUND_FORMAT_OBB_8, sizeof(BOUND_OBB_8), (BYTE*)&obb1);
	if (FAILED(h)) return h;
	h = pWith->GetData(BOUND_FORMAT_OBB_8, sizeof(BOUND_OBB_8), (BYTE*)&obb2);
	if (FAILED(h))
	{
		// if no support for OBB_8, let's do with OBB
		BOUND_OBB obb;
		h = pWith->GetData(BOUND_FORMAT_OBB, sizeof(BOUND_OBB), (BYTE*)&obb);
		if (FAILED(h)) return h;

		obb2.OBB[0].x = -obb.vecSize.x;	obb2.OBB[0].y = -obb.vecSize.y;	obb2.OBB[0].z = -obb.vecSize.z;
		obb2.OBB[1].x =  obb.vecSize.x;	obb2.OBB[1].y = -obb.vecSize.y;	obb2.OBB[1].z = -obb.vecSize.z;
		obb2.OBB[2].x = -obb.vecSize.x;	obb2.OBB[2].y = -obb.vecSize.y;	obb2.OBB[2].z =  obb.vecSize.z;
		obb2.OBB[3].x =  obb.vecSize.x;	obb2.OBB[3].y = -obb.vecSize.y;	obb2.OBB[3].z =  obb.vecSize.z;
		obb2.OBB[4].x = -obb.vecSize.x;	obb2.OBB[4].y =  obb.vecSize.y;	obb2.OBB[4].z = -obb.vecSize.z;
		obb2.OBB[5].x =  obb.vecSize.x;	obb2.OBB[5].y =  obb.vecSize.y;	obb2.OBB[5].z = -obb.vecSize.z;
		obb2.OBB[6].x = -obb.vecSize.x;	obb2.OBB[6].y =  obb.vecSize.y;	obb2.OBB[6].z =  obb.vecSize.z;
		obb2.OBB[7].x =  obb.vecSize.x;	obb2.OBB[7].y =  obb.vecSize.y;	obb2.OBB[7].z =  obb.vecSize.z;
		for (int i = 0; i < 8; i++)
			_Transform(obb2.OBB + i, obb.M);
	}

	// apply the separating axis theorem
	// it's enough to check 15 axes if they are separating axes
	// we can stop after finding any separating axis

	// first 6: orthogonal to face of either box (parallel to edge)
	if (FaceOrthogTest(obb1.OBB[0], obb1.OBB[1], obb2.OBB)) return S_OK;
	if (FaceOrthogTest(obb1.OBB[0], obb1.OBB[2], obb2.OBB)) return S_OK;
	if (FaceOrthogTest(obb1.OBB[0], obb1.OBB[4], obb2.OBB)) return S_OK;
	if (FaceOrthogTest(obb2.OBB[0], obb2.OBB[1], obb1.OBB)) return S_OK;
	if (FaceOrthogTest(obb2.OBB[0], obb2.OBB[2], obb1.OBB)) return S_OK;
	if (FaceOrthogTest(obb2.OBB[0], obb2.OBB[4], obb1.OBB)) return S_OK;

	// next 9: orthogonal to one edge from each box
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[1], obb2.OBB[0], obb2.OBB[1], obb1.OBB, obb2.OBB)) return S_OK;
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[1], obb2.OBB[0], obb2.OBB[2], obb1.OBB, obb2.OBB)) return S_OK;
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[1], obb2.OBB[0], obb2.OBB[4], obb1.OBB, obb2.OBB)) return S_OK;
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[2], obb2.OBB[0], obb2.OBB[1], obb1.OBB, obb2.OBB)) return S_OK;
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[2], obb2.OBB[0], obb2.OBB[2], obb1.OBB, obb2.OBB)) return S_OK;
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[2], obb2.OBB[0], obb2.OBB[4], obb1.OBB, obb2.OBB)) return S_OK;
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[4], obb2.OBB[0], obb2.OBB[1], obb1.OBB, obb2.OBB)) return S_OK;
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[4], obb2.OBB[0], obb2.OBB[2], obb1.OBB, obb2.OBB)) return S_OK;
	if (EdgeOrthogTest(obb1.OBB[0], obb1.OBB[4], obb2.OBB[0], obb2.OBB[4], obb1.OBB, obb2.OBB)) return S_OK;

	return S_FALSE;
}

//////////////////////////////////////////////////////////////////
// helper functions
// test if projections of two obb's on an axis overlap.
// return true if an axis is a separating axis (both projections do not overlap)

// test A-B axis
// notice that one of obb projections is from a to b
bool FaceOrthogTest(FWVECTOR &A, FWVECTOR &B, FWVECTOR obb[8])
{
	// some simple values
	double xab = B.x - A.x;
	double yab = B.y - A.y;
	double zab = B.z - A.z;

	// variables used to compute the projection
	double mab = (fabs(xab) > fabs(yab)) ?				// max(xab, yab, zab)
				 ( fabs(xab) > fabs(zab) ? xab : zab)
				:( fabs(yab) > fabs(zab) ? yab : zab);
	double lab2 = xab * xab + yab * yab + zab * zab;	// sqare of length a-b
	
	// first obb projection extends from a to b (a treated as 0)
	double a = 0;
	double b = mab;
	if (b < 0) { a = b; b = 0; }	// swap so that a <= b

	// compare projections of both obb: test for overlapping
	int i;
	double c = mab * (xab * (obb[0].x - A.x) + yab * (obb[0].y - A.y) + zab * (obb[0].z - A.z)) / lab2;
	if (c < a)	// c is less then a; check if whole area is less then a
		for (i = 1; i < 8; i++)
		{
			double pr = mab * (xab * (obb[i].x - A.x) + yab * (obb[i].y - A.y) + zab * (obb[i].z - A.z)) / lab2;
			if (pr >= a)
				return false;	// overlap: not a separating axis
		}
	else
	if (c > b)	// c is greater then b; check if whole area is greater then b
		for (i = 1; i < 8; i++)
		{
			double pr = mab * (xab * (obb[i].x - A.x) + yab * (obb[i].y - A.y) + zab * (obb[i].z - A.z)) / lab2;
			if (pr <= b)
				return false;	// overlap: not a separating axis
		}
	else
		return false;			// c hits between a and b!

	return true;	// no overlapped: separating axis found!
}

// test an axis orthogonal to both A-B and C-D axes
bool EdgeOrthogTest(FWVECTOR &A, FWVECTOR &B, FWVECTOR &C, FWVECTOR &D, FWVECTOR obb1[8], FWVECTOR obb2[8])
{
	// some simple values
	double x1 = B.x - A.x;
	double y1 = B.y - A.y;
	double z1 = B.z - A.z;
	double x2 = D.x - C.x;
	double y2 = D.y - C.y;
	double z2 = D.z - C.z;

	// determinants
	double detxy = x1 * y2 - x2 * y1;
	double detyz = y1 * z2 - y2 * z1;
	double detzx = z1 * x2 - z2 * x1;

	// find orthogonal
	double a, b, c;
	if (detxy == 0 && detyz == 0 && detzx == 0)
	{
		// parallel axes: no need for further computation, 
		// FaceOrthogTest should already have detected this case
		return false;
	}
	else
	{
		enum { DETXY, DETYZ, DETZX } nWhich;
		if (fabs(detxy) >= fabs(detyz))
			nWhich = (fabs(detxy) >= fabs(detzx)) ? DETXY : DETZX;
		else
			nWhich = (fabs(detyz) >= fabs(detzx)) ? DETYZ : DETZX;

		switch (nWhich)
		{
		case DETXY:
			a = detyz / detxy;
			b = detzx / detxy;
			c = 1;
			break;
		case DETYZ:
			a = 1;
			b = detzx / detyz;
			c = detxy / detyz;
			break;
		case DETZX:
			a = detyz / detzx;
			b = 1;
			c = detxy / detzx;
			break;
		}
	}
	// (a, b, c) is the axis to which we project both obb's
	
	// variables used to compute the projection
	double mab = (fabs(a) > fabs(b)) ?				// max(a, b, c)
				 ( fabs(a) > fabs(c) ? a : c)
				:( fabs(b) > fabs(c) ? b : c);
	double lab2 = a * a + b * b + c * c;			// sqare of length of (0,0,0)-(a,b,c)

	// test overlapping
	double min1, max1, min2, max2;
	min1 = max1 = mab * (a * (obb1[0].x - A.x) + b * (obb1[0].y - A.y) + c * (obb1[0].z - A.z)) / lab2;
	min2 = max2 = mab * (a * (obb2[0].x - A.x) + b * (obb2[0].y - A.y) + c * (obb2[0].z - A.z)) / lab2;
	int i;
	for (i = 2; i < 8; i++)
	{
		// move min & max
		double f1 = mab * (a * (obb1[i].x - A.x) + b * (obb1[i].y - A.y) + c * (obb1[i].z - A.z)) / lab2;
		if (f1 < min1) min1 = f1;
		if (f1 > max1) max1 = f1;
		double f2 = mab * (a * (obb2[i].x - A.x) + b * (obb2[i].y - A.y) + c * (obb2[i].z - A.z)) / lab2;
		if (f2 < min2) min2 = f2;
		if (f2 > max2) max2 = f2;

		// overlapping tests
		if (min1 < min2 && max1 >= min2) return false;
		if (min2 < min1 && max2 >= min1) return false;
		if (min1 == min2) return false;
	}

	return true;	// no overlapped: separating axis found!
}
