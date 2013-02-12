// wait.cpp : Defines the Waiting Action
//

#include "stdafx.h"
#include "wait.h"
#include "freewilltools.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <string.h>

#define DEG2RAD(d)	( (d) * (FWFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (FWFLOAT)M_PI )
#define SQR(x) ((x)*(x))

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionWait

CActionWait::CActionWait() : m_pBody(NULL), m_nTimeStamp(0)
{
}

CActionWait::~CActionWait()
{
	if (m_pBody) m_pBody->Release();
}

HRESULT CActionWait::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_BEGIN)
	{
		if (m_pBody && m_nTimeStamp >= pEvent->nTimeStamp + 6000)
		{
			// look around...
			ITransform *pT = NULL;
			m_pBody->CreateCompatibleTransform(&pT);

			FWLONG t = pEvent->nTimeStamp + 4000;

			FWFLOAT fA = (rand() & 1) ? DEG2RAD(80) : -DEG2RAD(80);

			pT->FromRotationX(fA);
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 0, 500, m_pBody, BODY_HEAD, pT);

			pT->FromRotationX(-fA);
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 1500, 500, m_pBody, BODY_HEAD, pT);

			pT->Release();
		}

		if (m_pBody && m_nTimeStamp >= pEvent->nTimeStamp + 15000)
		{
			// check the time...
			ITransform *pT = NULL;
			m_pBody->CreateCompatibleTransform(&pT);

			FWLONG t = pEvent->nTimeStamp+10000;

			pT->FromRotationY(DEG2RAD(20));
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 0, 500, m_pBody, BODY_LEFT+BODY_ARM, pT);
			pT->FromRotationZ(DEG2RAD(-40));
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 0, 500, m_pBody, BODY_LEFT+BODY_ARM, pT);

			pT->FromRotationZ(DEG2RAD(45));
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 300, 500, m_pBody, BODY_HEAD, pT);

			pT->FromRotationY(DEG2RAD(-110));
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 300, 500, m_pBody, BODY_LEFT+BODY_ARM+1, pT);

			pT->FromRotationY(DEG2RAD(110));
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 2000, 500, m_pBody, BODY_LEFT+BODY_ARM+1, pT);

			pT->FromRotationZ(DEG2RAD(-45));
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 2000, 500, m_pBody, BODY_HEAD, pT);

			pT->FromRotationZ(DEG2RAD(40));
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 2300, 500, m_pBody, BODY_LEFT+BODY_ARM, pT);
			pT->FromRotationY(DEG2RAD(-20));
			FWCreateObjWeakPtr(FWDevice(), L"Action", L"Rotate", this, t + 2300, 500, m_pBody, BODY_LEFT+BODY_ARM, pT);

			pT->Release();
		}
	}

	if (pEvent->nEvent == EVENT_TICK)
	{
		if (pEvent->nTimeStamp >= m_nTimeStamp)
			Die(pEvent);
	}

	return S_OK;
}

HRESULT CActionWait::Create(IFWCreateContext *p)
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
		
		// body - optional
		if FAILED(pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody)) 
			m_pBody = NULL;

		// when end of wait?
		h = pEnum->QueryLONG(&m_nTimeStamp);
		if (FAILED(h)) throw(h);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	pEnum->Release();
	return S_OK;
}

