// walk.cpp : Defines the Walking Actions
//

#include "stdafx.h"
#include "walk.h"
#include "freewilltools.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <string.h>

#define DEG2RAD(d)	( (d) * (FWFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (FWFLOAT)M_PI )
#define SQR(x) ((x)*(x))

//#include "../diag.inl"

#define sgn(x)	( (x) >= 0 ? 1 : -1 )

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionLegged

CActionLegged::CActionLegged() : m_pBody(NULL)
{
}

CActionLegged::~CActionLegged()
{
	if (m_pBody) m_pBody->Release();
}

void CActionLegged::ResolveLeggedPose(bool bStepForward)
{
	// some useful measurements
	FWVECTOR vecLegL, vecFootL, vecLegR, vecFootR, vecPelv;
	m_pBody->GetXYZ(BODY_PELVIS, &vecPelv);
	m_pBody->GetXYZ(BODY_LEFT + BODY_LEG, &vecLegL);
	m_pBody->GetXYZ(BODY_LEFT + BODY_FOOT, &vecFootL);
	m_pBody->GetXYZ(BODY_RIGHT + BODY_LEG, &vecLegR);
	m_pBody->GetXYZ(BODY_RIGHT + BODY_FOOT, &vecFootR);

	m_fHeight = sqrt(SQR(vecLegL.x-vecFootL.x) + SQR(vecLegL.y-vecFootL.y) + SQR(vecLegL.z-vecFootL.z));
	m_fSpan   = sqrt(SQR(vecLegL.x-vecPelv.x) + SQR(vecLegL.y-vecPelv.y));
	FWFLOAT fFootL = ((vecLegL.x-vecPelv.x) * (vecFootL.y-vecLegL.y) - (vecLegL.y-vecPelv.y) * (vecFootL.x-vecLegL.x)) / m_fSpan;
	FWFLOAT fFootR = ((vecLegR.x-vecPelv.x) * (vecFootR.y-vecLegR.y) - (vecLegR.y-vecPelv.y) * (vecFootR.x-vecLegR.x)) / m_fSpan;
	
	// left/right resolution
	m_bRight = (fFootR > 0);
	if (abs(fFootR) < 1e-4)
	{
		if (IsStyle(L"left") == S_OK) m_bRight = false;
		else if (IsStyle(L"right") == S_OK) m_bRight = true;
		else m_bRight = rand() & 1;
	}
	if (IsStyle(L"force-left") == S_OK)	 { m_bRight = false; fFootL = min(0, fFootL); }
	if (IsStyle(L"force-right") == S_OK) { m_bRight = true; fFootR = max(0, fFootR); }
	if (!bStepForward) m_bRight = !m_bRight;

	m_fFoot = m_bRight ? fFootR : -fFootL;
	if (m_bRight) m_fSpan = -m_fSpan;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionStep

CActionStep::CActionStep() : CActionLegged(),
		m_param1(0), m_param1Flag(PARAM1_DEF), m_param2(0), m_param2Flag(PARAM2_DEF), m_param3(0)
{
}

CActionStep::~CActionStep()
{
}

void CActionStep::SetParam1(PARAM1 flag)
{
	if (m_param1Flag == PARAM1_DEF) m_param1Flag = flag;
	else throw (HRESULT)FW_E_PARAMS_INCONSISTENT;
}

void CActionStep::SetParam2(PARAM2 flag)
{
	if (m_param2Flag == PARAM2_DEF) m_param2Flag = flag;
	else throw (HRESULT)FW_E_PARAMS_INCONSISTENT;
}

HRESULT CActionStep::ResolveAngleAngle(FWFLOAT fStep, FWFLOAT fHead)
{
	m_fStep = asin(m_fFoot / m_fHeight) + fStep / 2;
	m_fHead = fHead;
	return S_OK;
}

HRESULT CActionStep::ResolveAngleAside(FWFLOAT fStep, FWFLOAT fAside)
{
	return E_NOTIMPL;
}

HRESULT CActionStep::ResolveLenAngle(FWFLOAT fLen, FWFLOAT fHead)
{
	if (abs(fLen/2) > m_fHeight) fLen = 2 * m_fHeight * sgn(fLen);
	m_fStep = asin(m_fFoot / m_fHeight) + asin(fLen / 2 / m_fHeight);
	m_fHead = fHead;
	return S_OK;
}

// old version of ResolveFdAside
//	FWFLOAT fLen = sqrt(fFd * fFd + fAside * fAside - 2 * m_fSpan * fAside);
//	m_fHead = -(atan2(fLen, m_fSpan) - atan2(fFd, m_fSpan - fAside));
//	m_fStep = asin(m_fFoot / m_fHeight) + asin(fLen / m_fHeight);


HRESULT CActionStep::ResolveAbsAbs(FWFLOAT x, FWFLOAT y)
{
	return E_NOTIMPL;
}

HRESULT CActionStep::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_BEGIN)
	{
		//// DIAGNOSTICS ONLY
		//static FWULONG nCount = 0; TRACE(L"STEP #%d\n", ++nCount);
		//static CDiag diag, diagl, diagr;
		//diagl.Init(L"  Actual values Lt Leg", m_pBody, BODY_LEG + BODY_LEFT, CDiag::DELTA);
		//diag .Init(L"  Actual values Pelvis", m_pBody, BODY_PELVIS, (CDiag::MODE)(CDiag::DELTA | CDiag::HEAD));
		//diagr.Init(L"  Actual values Rt Leg", m_pBody, BODY_LEG + BODY_RIGHT, CDiag::DELTA);
		//TRACE(L"%s", (CString)diagl);
		//TRACE(L"%s", (CString)diag);
		//TRACE(L"%s", (CString)diagr);

		static int n = 0;
		n++;

		// Make useful measurements and resolve left/right issues
		ResolveLeggedPose(m_param1 >= 0);

		FWULONG BODY_STAT = m_bRight ? BODY_LEFT : BODY_RIGHT;
		FWULONG BODY_MOBI = m_bRight ? BODY_RIGHT : BODY_LEFT;

		// resolve parameters
		HRESULT h = S_OK;
		switch (m_param1Flag)
		{
			case PARAM1_ANGLE:
				switch (m_param2Flag)
				{
				case PARAM2_ANGLE:	h = ResolveAngleAngle(m_param1, m_param2); break;
				case PARAM2_ASIDE:	h = ResolveAngleAside(m_param1, m_param2); break;
				case PARAM2_DEF:	h = ResolveAngleAngle(m_param1, 0); break;
				default:			h = E_FAIL; break;
				}
				break;
			case PARAM1_LEN:
				switch (m_param2Flag)
				{
				case PARAM2_ANGLE:	h = ResolveLenAngle(m_param1, m_param2); break;
				case PARAM2_ASIDE:	h = ResolveLenAngle(m_param1, atan2(m_param2, m_param1)); break;
				case PARAM2_DEF:	h = ResolveLenAngle(m_param1, 0); break;
				default:			h = E_FAIL; break;
				}
				break;

			case PARAM1_FD:
				switch (m_param2Flag)
				{
				case PARAM2_ANGLE:	h = ResolveLenAngle(m_param1 / cos(m_param2), m_param2); break;
				case PARAM2_ASIDE:	h = ResolveLenAngle(sqrt(m_param1*m_param1 + m_param2*m_param2), asin(m_param2 / m_param1)); break;
				case PARAM2_DEF:	h = ResolveLenAngle(m_param1, 0); break;
				default:			h = E_FAIL; break;
				}
				break;
			case PARAM1_ABS: 
				switch (m_param2Flag)
				{
				case PARAM2_ABS:	h = ResolveAbsAbs(m_param1, m_param2); break;
				default:			h = E_FAIL; break;
				}
				break;
			default:				h = E_FAIL; break;
		}
		if FAILED(h) 
			return ERROR(h);

		//// DIAGNOSTICS ONLY
		//FWFLOAT fLenLeg = (sin(m_fStep - asin(m_fFoot / m_fHeight)) * m_fHeight + m_fFoot) * cos(m_fHead / 2);
		//FWFLOAT R = fLenLeg / 2 / sin(m_fHead / 2);
		//FWFLOAT fLenPelv = _finite(R) ? fLenLeg * (R + m_fSpan) / R : fLenLeg;
		//TRACE("Expected value Leg---:\tlen: %lf\nExpected value Pelvis:\tlen: %lf\n", fLenLeg, fLenPelv);
		//// *** DEBUG VERIFICATION
		//TRACE(L"--> Angles before ResolveFdAside:\t m_fStep = %lf \t m_fHead = %lf\n", RAD2DEG(m_fStep), RAD2DEG(m_fHead));
		//FWFLOAT mfs = m_fStep, mfh = m_fHead;
		//ResolveLenAngle((fLenLeg-m_fFoot)*2, m_fHead);
		//TRACE(L"--> Angles after ResolveFdAside:\t m_fStep = %lf \t m_fHead = %lf", RAD2DEG(m_fStep), RAD2DEG(m_fHead));
		//TRACE(L"\t Deltas:   %lf  %lf\n", RAD2DEG(m_fStep-mfs), RAD2DEG(m_fHead-mfh));


		// prepare transforms
		ITransform *pT1, *pT2, *pT3, *pT4;
		m_pBody->CreateCompatibleTransform(&pT1);
		pT2 = (ITransform*)pT1->Clone(IID_ITransform);
		pT3 = (ITransform*)pT1->Clone(IID_ITransform);
		pT4 = (ITransform*)pT1->Clone(IID_ITransform);

		pT1->FromRotationX(m_fHead);
		pT2->FromRotationZ(m_fStep);
		pT3->FromRotationZ(-m_fStep);

		IAction *pAction = NULL;

		// Ground Foot
		pAction = (IAction*)FWCreateObjWeakPtr(FWDevice(), L"Action", L"RotateInv", 
			this, m_nStartTime, m_nPeriod, m_pBody, BODY_PELVIS, BODY_STAT + BODY_FOOT, pT1, pT2, BODY_STAT + BODY_LEG, pT3);
//		pAction->SetEnvelope(ACTION_ENV_NONE, 0, 1);

		// Kicking Leg
		FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", 
			this, m_nStartTime, m_nPeriod, m_pBody, BODY_MOBI + BODY_LEG, pT2);

		pT1->FromIdentity();
		pT1->MulRotationY(M_PI);
		pT1->MulRotationX(m_param3);
		FWCreateObjWeakPtr(FWDevice(), L"Action", L"RotateTo", 
			this, m_nStartTime, m_nPeriod, m_pBody, BODY_MOBI + BODY_FOOT, pT1, BODY_PELVIS);

		
		// Bend the Mobile Leg in the Knee

		FWFLOAT fBend = max(abs(m_fStep), m_fBend);

		pT1->FromRotationZ(fBend/2);
		pT2->FromRotationZ(-fBend);

		FWCreateObjWeakPtr(FWDevice(), L"Action", L"MultiRotate", 
			this, m_nStartTime, m_nPeriod*2/4, 
			2, m_pBody, 
			BODY_LEG + BODY_MOBI, pT1,
			BODY_LEG + 1 + BODY_MOBI, pT2
			);

		pT1->FromRotationZ(-fBend/2);
		pT2->FromRotationZ(fBend);
		FWCreateObjWeakPtr(FWDevice(), L"Action", L"MultiRotate", 
			this, m_nStartTime+m_nPeriod*2/4, m_nPeriod*2/4, 
			2, m_pBody, 
			BODY_LEG + BODY_MOBI, pT1,
			BODY_LEG + 1 + BODY_MOBI, pT2);

		// move arms...
		pT1->FromRotationZ(m_fStep);
		pT2->FromRotationZ(-m_fStep);
		pT3->FromRotationZ(1*m_fStep);
		pT4->FromRotationZ(-1*m_fStep);
		FWCreateObjWeakPtr(FWDevice(), L"Action", L"MultiRotate", 
			this, m_nStartTime, m_nPeriod, 
			4, m_pBody, 
			BODY_ARM + BODY_MOBI, pT1,
			BODY_ARM + BODY_STAT, pT2,
			BODY_ARM + 1 + BODY_MOBI, pT3,
			BODY_ARM + 1 + BODY_STAT, pT4);


		pT1->Release(); pT2->Release(); pT3->Release(); pT4->Release();
	}

	return S_OK;
}

HRESULT CActionStep::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;
	p->EnumParamsEx(NULL, &pEnum);

	static const FWSTRING PARAM_ANGLE	= L"angle";
	static const FWSTRING PARAM_LENGTH	= L"length";
	static const FWSTRING PARAM_FORWARD = L"forward";
	static const FWSTRING PARAM_HEADING = L"heading";
	static const FWSTRING PARAM_ASIDE	= L"aside";
	static const FWSTRING PARAM_LEFT	= L"left";
	static const FWSTRING PARAM_RIGHT	= L"right";
	static const FWSTRING PARAM_ABS		= L"abs";
	static const FWSTRING PARAM_BEND	= L"bend";

	// params
	try
	{
		// std & obligatory params
		h = QueryStdParams(pEnum); 
		if (FAILED(h)) throw(h);
		
		// body - the obligatory param
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); 
		if (FAILED(h)) throw(h);

		// angle + heading - optional unnamed
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param1)) SetParam1(PARAM1_ANGLE);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) SetParam2(PARAM2_ANGLE);
		pEnum->Release();

		p->EnumParamsEx(PARAM_ANGLE, &pEnum);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param1)) SetParam1(PARAM1_ANGLE);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) SetParam2(PARAM2_ANGLE);
		pEnum->Release();

		p->EnumParamsEx(PARAM_LENGTH, &pEnum);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param1)) SetParam1(PARAM1_LEN);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) SetParam2(PARAM2_ANGLE);
		pEnum->Release();

		p->EnumParamsEx(PARAM_FORWARD, &pEnum);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param1)) SetParam1(PARAM1_FD);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) SetParam2(PARAM2_ASIDE);
		pEnum->Release();

		p->EnumParamsEx(PARAM_HEADING, &pEnum);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) SetParam2(PARAM2_ANGLE);
		pEnum->Release();

		p->EnumParamsEx(PARAM_ASIDE, &pEnum);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) SetParam2(PARAM2_ASIDE);
		pEnum->Release();

		p->EnumParamsEx(PARAM_LEFT, &pEnum);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) { m_param2 = -m_param2; SetParam2(PARAM2_ASIDE); }
		pEnum->Release();

		p->EnumParamsEx(PARAM_RIGHT, &pEnum);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) SetParam2(PARAM2_ASIDE);
		pEnum->Release();

		p->EnumParamsEx(PARAM_ABS, &pEnum);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param1)) SetParam1(PARAM1_ABS);
		if SUCCEEDED(pEnum->QueryFLOAT(&m_param2)) SetParam2(PARAM2_ABS);
		if (m_param1Flag == PARAM1_ABS && m_param2Flag != PARAM1_ABS || m_param1Flag != PARAM1_ABS && m_param2Flag == PARAM1_ABS)
			throw (HRESULT)FW_E_PARAMS_INCONSISTENT;
		pEnum->Release();

		// Additional Parameter?
		p->EnumParamsEx(PARAM_BEND, &pEnum);
		if FAILED(pEnum->QueryFLOAT(&m_fBend)) m_fBend = 0;
		pEnum->Release();
		

		if (m_param1Flag == PARAM1_DEF && m_param2Flag != PARAM2_ABS)
		{
			m_param1 = DEG2RAD(30); m_param1Flag = PARAM1_ANGLE;
		}
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	return S_OK;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionTurn

CActionTurn::CActionTurn() : CActionLegged()
{
}

CActionTurn::~CActionTurn()
{
}

HRESULT CActionTurn::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK && !AnySubscriptionsLeft())
		Die(pEvent);

	if (pEvent->nEvent == EVENT_BEGIN)
	{
		// some useful measurements
		ResolveLeggedPose();

		if (m_fRot == 0) m_fRot = m_bRight ? (FLOAT)M_PI : -(FLOAT)M_PI;
		
		IFWUnknown *p = NULL;
		FWSTRING pLRStyle[2] = { L"force-left", L"force-right" };
		int nLR = m_bRight ? 1 : 0;

		p = FWCreateObjWeakPtr(FWDevice(), L"Action", L"Generic", this, m_nStartTime, 0);

		FWFLOAT fAngle = m_fRot / m_nSteps;
		for (FWULONG i = 0; i < m_nSteps; i++)
		{
			FWFLOAT fDist = (i == m_nSteps-1) ? m_fDist : 0;
			p = FWCreateObjWeakPtr(FWDevice(), L"Action", L"Step", this, p, m_nPeriod, pLRStyle[nLR], m_pBody, fDist, fAngle, CParam(DEG2RAD(20), L"bend"));
			nLR = 1 - nLR;
		}
		if (IsStyle(L"open") == S_FALSE)
			p = FWCreateObjWeakPtr(FWDevice(), L"Action", L"Step", this, p, m_nPeriod, pLRStyle[nLR], m_pBody, 0, 0, CParam(DEG2RAD(20), L"bend"));

		((IAction*)p)->Subscribe(this, EVENT_END, ACTION_ANY | ACTION_ONCE | ACTION_WEAKPTR, 0, 4637, NULL);
	}
	return S_OK;
}

HRESULT CActionTurn::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;
	p->EnumParamsEx(NULL, &pEnum);

	// params
	try
	{
		// std & obligatory params
		ACTION_SUBS *pSubs;
		h = QueryStdParams(pEnum, &pSubs); 
		if (FAILED(h)) throw(h);

		// change to manual mode of unsubscription
		pSubs->nFlags &= ~ACTION_MASK_MODE;
		pSubs->nFlags |= ACTION_MANUAL;
		
		// body - the obligatory param
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); 
		if (FAILED(h)) throw(h);

		// Optional Param: Total Rotation (M_PI or 180 degrees by default)
		if FAILED(pEnum->QueryFLOAT(&m_fRot)) m_fRot = 0;
		// Optional Param: Step Count: default value taken so that rotation at each step is close to 60 degrees.
		if FAILED(pEnum->QueryULONG(&m_nSteps)) m_nSteps = (m_fRot == 0) ? 3 : (FWULONG)((abs(m_fRot) - 0.2) / M_PI * 3) + 1;
		// Optional Param: Distance to proceed after completion the turn (0 by default)
		if FAILED(pEnum->QueryFLOAT(&m_fDist)) m_fDist = 0;
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	pEnum->Release();
	return S_OK;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionWalk

CActionWalk::CActionWalk() : CActionLegged(), m_pActionStep(NULL), m_pSubs(NULL)
{
}

CActionWalk::~CActionWalk()
{
}

IAction *CActionWalk::MakeStep(FWULONG nTimeStamp)
{
	bool bOpen = IsStyle(L"open") == S_OK;

	m_pSubs = NULL;

	IKineNode *pPelvis = m_pBody->BodyNode(BODY_PELVIS);
	FWVECTOR vec = { m_x, m_y, 0 };
	pPelvis->GtoL(&vec); vec.x = vec.z;
	pPelvis->Release();

	FWFLOAT fAngle = atan2(vec.x, vec.y);
	FWFLOAT fDist  = sqrt(vec.x * vec.x + vec.y * vec.y);

	if (bOpen && fDist < m_fLen)
		return NULL;				// in open mode - we are close enough...

	FWFLOAT fLen = m_fLen;
	if (fDist < fLen / 10						// if already there...
	|| (abs(fAngle) > m_fHead && fDist < fLen))	// or have just stepped over it
	{
		if (bOpen) return NULL;		// in open mode that's all, folks
		fLen = 0;					// in close mode - final "0" step
		fAngle = 0;
	}

	if (!bOpen)
		fLen = min(fLen, fDist - m_fFoot);

	fLen = max(0, fLen);

	fAngle = min(fAngle, abs(m_fHead));
	fAngle = max(fAngle, -abs(m_fHead));

	IFWUnknown *p = NULL;
	p = FWCreateObjWeakPtr(FWDevice(), L"Action", L"Step", this, nTimeStamp, m_nPeriod, L"left", m_pBody, 
		CParam(fLen, L"length"), CParam(fAngle, L"heading"));

	((IAction*)p)->Subscribe(this, EVENT_END, ACTION_ANY | ACTION_ONCE | ACTION_WEAKPTR, 0, /*fLen ? (FWULONG)(this) :*/ NULL, &m_pSubs);
	if (!fLen) m_pSubs = NULL;

	return (IAction*)p;
}

HRESULT CActionWalk::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK && m_pActionStep == NULL)
		Die(pEvent);

	if (pEvent->nEvent == EVENT_BEGIN
		|| (pEvent->nEvent == EVENT_END && m_pSubs && pEvent->pSubs == m_pSubs))
	{
		ResolveLeggedPose();

		if (m_pActionStep) m_pActionStep->Suspend(pEvent->nSubCode);
		m_pActionStep = MakeStep(pEvent->nSubCode);

		if (m_pActionStep)
		{
			ACTION_EVENT ev = { pEvent->nTimeStamp, EVENT_TICK, pEvent->nTimeStamp, 0, NULL, (ACTION_EVENT*)(pEvent->nReserved) };
			RaiseEventEx(&ev);
			return S_FALSE;
		}
	}

	return S_OK;
}

HRESULT CActionWalk::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;
	p->EnumParamsEx(NULL, &pEnum);

	// params
	try
	{
		// std & obligatory params
		ACTION_SUBS *pSubs;
		h = QueryStdParams(pEnum, &pSubs); 
		if (FAILED(h)) throw(h);

		// change to manual mode of unsubscription
		pSubs->nFlags &= ~ACTION_MASK_MODE;
		pSubs->nFlags |= ACTION_MANUAL;
		
		// body - the obligatory param
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); 
		if (FAILED(h)) throw(h);

		// Obligatory Params: destination coordinates
		h = pEnum->QueryFLOAT(&m_x); if FAILED(h) throw(h);
		h = pEnum->QueryFLOAT(&m_y); if FAILED(h) throw(h);

		m_fLen = m_fHead = 0;

		// Optional Param: max length of step: default 0 (to be resolved later)
		if FAILED(pEnum->QueryFLOAT(&m_fLen)) m_fLen = 0;

		// Optional Param: max heading (turn): default 80 degrees
		if FAILED(pEnum->QueryFLOAT(&m_fHead)) m_fHead = DEG2RAD(80);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	pEnum->Release();
	return S_OK;
}


