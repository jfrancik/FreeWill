// reach.cpp : Defines the Reach Action
//

#include "stdafx.h"
#include "reach.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define DEG2RAD(d)	( (d) * M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / M_PI )

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionReach

CActionReach::CActionReach()
{
	m_pArm1 = NULL;
	m_pArm2 = NULL;
	m_pTerm = NULL;
	m_pDest = NULL;
}

CActionReach::~CActionReach()
{
	if (m_pArm1) m_pArm1->Release();
	if (m_pArm2) m_pArm2->Release();
	if (m_pTerm) m_pTerm->Release();
	if (m_pDest) m_pDest->Release();
}

	inline double VectorDistance(FWVECTOR *p1, FWVECTOR *p2)
	{
		return sqrt((p1->x - p2->x) * (p1->x - p2->x) + (p1->y - p2->y) * (p1->y - p2->y) + (p1->z - p2->z) * (p1->z - p2->z));
	}

	inline double VectorLength(FWVECTOR *p)
	{
		return sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
	}

	inline double VectorDotProduct(FWVECTOR *p1, FWVECTOR *p2)
	{
		return p1->x * p2->x + p1->y * p2->y + p1->z * p2->z;
	}

	inline FWVECTOR *VectorCrossProduct(FWVECTOR *p, FWVECTOR *p1, FWVECTOR *p2)
	{
		p->x = p1->y * p2->z - p1->z * p2->y;
		p->y = p1->z * p2->x - p1->x * p2->z;
		p->z = p1->x * p2->y - p1->y * p2->x;
		return p;
	}

	inline double VectorAngle(FWVECTOR *p1, FWVECTOR *p2)
	{
		FWVECTOR vecCross;
		VectorCrossProduct(&vecCross, p1, p2);
		double dDot = VectorDotProduct(p1, p2);
		double f1 = VectorLength(p1);
		double f2 = VectorLength(p2);
		double dCosA = dDot / f1 / f2;
		double dSinA = VectorLength(&vecCross) / f1 / f2;
		return atan2(dSinA, dCosA);
	}

HRESULT CActionReach::HandleEvent(struct ACTION_EVENT *pEvent)
{
	// Sunstantially revised on 10 Jan 2009
	// See archived file for previous versions
	if (pEvent->nEvent == EVENT_TICK)
	{
		// determine the time
		FWFLOAT fTime;
		GetProgPhase(pEvent, &fTime);

		// prepare the work transform object
		ITransform *pT = NULL;
		m_pArm2->CreateCompatibleTransform(&pT);

		// extract copies of the current transforms in both arms
		ITransform *pTArm1 = NULL, *pTArm2 = NULL;
		m_pArm1->CreateCompatibleTransform(&pTArm1);
		m_pArm2->CreateCompatibleTransform(&pTArm2);
		m_pArm1->GetLocalTransform(pTArm1);
		m_pArm2->GetLocalTransform(pTArm2);

		// The current copies of transforms are kept for interpolation
		// between current and target poses.
		// The target pose is calculated from the reset pose of arms.

		// reset the arms
		m_pArm1->Reset();
		m_pArm2->Reset();

		// compute Term and Dest in local coordinates of Arm1
		FWVECTOR vecTerm = m_vDTerm, vecDest = m_vDDest;
		if (m_pTerm) m_pTerm->LtoG(&vecTerm);
		m_pArm1->GtoL(&vecTerm);
		if (m_pDest) m_pDest->LtoG(&vecDest);
		m_pArm1->GtoL(&vecDest);

		// adjust elbow bending so that to move the terminator 
		// to the exact distance of Destination point
		// (a2, b2, x2) - squares of (arm1, arm2, term) lengths, cast onto XY
		// z2 - square of z offset of term; fDest2 - square of the destination distance
		FWVECTOR vecElbow;
		m_pArm2->GetBaseVector(&vecElbow);
		double a2 = vecElbow.x * vecElbow.x + vecElbow.y * vecElbow.y;
		double b2 = (vecTerm.x - vecElbow.x) * (vecTerm.x - vecElbow.x) +  (vecTerm.y - vecElbow.y) * (vecTerm.y - vecElbow.y);
		double x2 = vecTerm.x * vecTerm.x + vecTerm.y * vecTerm.y;
		double z2 = vecTerm.z * vecTerm.z;
		double fDest2 = vecDest.x * vecDest.x + vecDest.y * vecDest.y + vecDest.z * vecDest.z;
		double fAux = (2 * sqrt(a2 * b2));	// optimisation
		FWFLOAT fAlpha = 0.0;
		if (fabs(fDest2 - a2 - b2 - z2) < fAux)
			fAlpha = -(FWFLOAT)acos((fDest2 - a2 - b2 - z2) / fAux);		// new value of fAlpha
		fAlpha -= (FWFLOAT)acos((x2 - a2 - b2) / fAux);					// present value of fAlpha
				
		// apply the elbow rotation
		pT->FromRotationZ(fAlpha);
		m_pArm2->PutLocalTransform(pT);

		// calculate new terminator position after the changes
		vecTerm = m_vDTerm;
		if (m_pTerm) m_pTerm->LtoG(&vecTerm);
		m_pArm1->GtoL(&vecTerm);

		// compute distance to the termination and destination points
		double fTerm = VectorLength(&vecTerm);
		double fDest = sqrt(fDest2);	// == VectorLength(vecDest)

		// calculate axis and angle for arm rotation
		// using cross & dot products of vecTerm & vecDest
		// results in: vecCross and dAngle
		FWVECTOR vecCross;
		VectorCrossProduct(&vecCross, &vecTerm, &vecDest);
		double dDot = VectorDotProduct(&vecTerm, &vecDest);
		double dCosA = dDot / fTerm / fDest;
		double dSinA = VectorLength(&vecCross) / fTerm / fDest;
		double dAngle = atan2(dSinA, dCosA);

		// Calculate Elbow Correction
		FWFLOAT fElbowCorrectionAngle = m_bRight ? -m_fElbowCorrectionAngle : m_fElbowCorrectionAngle;
		if (fElbowCorrectionAngle == 0.0f
		&& ((!m_bRight && vecDest.z > 0) || (m_bRight && vecDest.z < 0)))
			fElbowCorrectionAngle = atan2(-vecDest.z, -vecDest.y / 2);

		// interpolate and apply for arm 1
		pT->FromIdentity();
		pT->MulRotationAxisAngle(&vecCross, (FWFLOAT)dAngle);
		pT->MulRotationAxisAngle(&vecDest, fElbowCorrectionAngle);
		pT->Interpolate(pTArm1, pT, fTime);
		m_pArm1->PutLocalTransform(pT);
		pTArm1->Release();

		// interpolate and apply for arm2
		pT->FromIdentity();
		pT->MulRotationZ(fAlpha);
		pT->Interpolate(pTArm2, pT, fTime);
		m_pArm2->PutLocalTransform(pT);
		pTArm2->Release();

		pT->Release();
	}
	return S_OK;
}

HRESULT CActionReach::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	//params: std only...
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	// parse the style
	m_bRight = true;
	if (IsStyle(L"left") == S_OK) m_bRight = false;

	// There are two alternative param lists:
	// PBONE pArm1, PBONE pArm2, PBONE pTerm, FWVECTOR vecTerm, PBONE pDest, FWVECTOR vecDest, FWFLOAT m_fElbowCorrectionAngle
	// PBODY pBody, PBONE pTerm, FWVECTOR vecTerm, PBONE pDest, FWVECTOR vecDest, FWFLOAT m_fElbowCorrectionAngle

	// first, validate list version 1
	IFWEnumParams *pEnum1 = NULL;
	pEnum->Clone(&pEnum1);
	bool bForm1 =
		SUCCEEDED(pEnum1->QueryPBONE(IID_IUnknown, NULL)) &&
		SUCCEEDED(pEnum1->QueryPBONE(IID_IUnknown, NULL)) &&
		SUCCEEDED(pEnum1->QueryPBONE(IID_IUnknown, NULL));
	pEnum1->Release();

	// set default values
	m_pArm1 = m_pArm2 = m_pTerm = m_pDest = NULL;
	memset(&m_vDTerm, 0, sizeof(m_vDTerm));
	memset(&m_vDDest, 0, sizeof(m_vDDest));
	m_fElbowCorrectionAngle = 0.0f;
	IBody *pBody = NULL;	// extra value

	// read params
	try
	{
		if (bForm1)
		{
			h = pEnum->QueryPBONE(IID_IKineObj3D, (FWPUNKNOWN*)&m_pArm1); if (FAILED(h)) throw(h);
			h = pEnum->QueryPBONE(IID_IKineObj3D, (FWPUNKNOWN*)&m_pArm2); if (FAILED(h)) throw(h);
			h = pEnum->QueryPBONE(IID_IKineObj3D, (FWPUNKNOWN*)&m_pTerm); if (FAILED(h)) throw(h);
		}
		else
		{
			h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&pBody); if (FAILED(h)) throw(h);
			h = pEnum->QueryPBONE(IID_IKineObj3D, (FWPUNKNOWN*)&m_pTerm); if (FAILED(h)) throw(h);
		}
		h = pEnum->QueryVECTOR(&m_vDTerm);
		h = pEnum->QueryPBONE(IID_IKineObj3D, (FWPUNKNOWN*)&m_pDest);
		h = pEnum->QueryVECTOR(&m_vDDest);
		h = pEnum->QueryFLOAT(&m_fElbowCorrectionAngle);

		if (!bForm1)
		{
			m_pArm1 = pBody->BodyChild(m_bRight ? BODY_ARM + BODY_RIGHT : BODY_ARM + BODY_LEFT);
			if (m_pArm1 == NULL) throw((HRESULT)ACTION_E_CANNOTSETUP);
			m_pArm2 = pBody->BodyChild(m_bRight ? BODY_ARM+1 + BODY_RIGHT : BODY_ARM+1 + BODY_LEFT);
			if (m_pArm2 == NULL) throw((HRESULT)ACTION_E_CANNOTSETUP);
			pBody->Release();
		}
	}
	catch (HRESULT h)
	{
		// additional clean up
		if (pBody) pBody->Release(); pBody = NULL;

		return ErrorStdParams(pEnum, h);
	}

	pEnum->Release();
	return S_OK;
}
