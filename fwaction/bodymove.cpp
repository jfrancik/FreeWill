// bodymove.cpp : Defines the Body Motion Actions
//

#include "stdafx.h"
#include "bodymove.h"
#include "freewilltools.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define DEG2RAD(d)	( (d) * (FWFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (FWFLOAT)M_PI )

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionBend

CActionBend::CActionBend() : m_pBody(NULL)
{
}

CActionBend::~CActionBend()
{
	if (m_pBody) m_pBody->Release();
}

HRESULT CActionBend::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
	}
	return S_OK;
}

HRESULT CActionBend::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	// params: std & body
	FWFLOAT fAngle1, fAngle2;
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); if (FAILED(h)) throw(h);
		h = pEnum->QueryFLOAT(&fAngle1);
		if (FAILED(h)) fAngle1 = DEG2RAD(50);
		h = pEnum->QueryFLOAT(&fAngle2);
		if (FAILED(h)) fAngle2 = 0;
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	// create transform
	IAction *pAction = NULL;

	ITransform *pT, *pX;
	m_pBody->CreateCompatibleTransform(&pT);
	m_pBody->CreateCompatibleTransform(&pX);

	// plan lower level actions
	pT->FromRotationZ(fAngle1);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_SPINE, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->FromRotationZ(-fAngle2);
	pX->FromRotationZ(fAngle2);
	{ CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_PELVIS, BODY_FOOT + BODY_LEFT, pT, BODY_LEG + BODY_LEFT, pX };
	FWDevice()->CreateObjectEx(L"Action", L"RotateInv", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction); }
	pAction->Release();

	pT->FromRotationZ(fAngle2);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_LEG + BODY_RIGHT, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->FromRotationZ(-fAngle2);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_FOOT + BODY_RIGHT, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->Release();
	pX->Release();
	pEnum->Release();
	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionSquat

CActionSquat::CActionSquat() : m_pBody(NULL)
{
}

CActionSquat::~CActionSquat()
{
	if (m_pBody) m_pBody->Release();
}

HRESULT CActionSquat::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
	}
	return S_OK;
}

HRESULT CActionSquat::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	// params: std & body
	FWFLOAT fAngle1, fAngle2, fAngle3;
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); if (FAILED(h)) throw(h);

		h = pEnum->QueryFLOAT(&fAngle1);
		if (FAILED(h)) fAngle1 = DEG2RAD(50);
		h = pEnum->QueryFLOAT(&fAngle2);
		if (FAILED(h)) fAngle2 = DEG2RAD(8);
		h = pEnum->QueryFLOAT(&fAngle3);
		if (FAILED(h)) fAngle3 = DEG2RAD(30);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	// create transform
	IAction *pAction = NULL;

	ITransform *pT, *pX, *pY;
	m_pBody->CreateCompatibleTransform(&pT);
	m_pBody->CreateCompatibleTransform(&pX);
	m_pBody->CreateCompatibleTransform(&pY);

	// plan lower level actions
	pT->FromRotationZ(fAngle1);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_SPINE, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// PROBLEM!!! CURRENT VERSION DOES NOT ACCEPT 3 LIMBS
	pT->FromRotationZ(fAngle3 - fAngle2);
	pX->FromRotationZ(-fAngle3-fAngle3);
	pY->FromRotationZ(fAngle3 + fAngle2);
	{ CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_PELVIS, BODY_FOOT + BODY_LEFT, pT, BODY_LEG + 1 + BODY_LEFT, pX, BODY_LEG + BODY_LEFT, pY };
	FWDevice()->CreateObjectEx(L"Action", L"RotateInv", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction); }
	pAction->Release();

	pT->FromRotationZ(fAngle2 + fAngle3);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_LEG + BODY_RIGHT, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->FromRotationZ(-fAngle3 - fAngle3);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_LEG + 1 + BODY_RIGHT, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->FromRotationZ(-fAngle2 + fAngle3);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_FOOT + BODY_RIGHT, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->Release();
	pX->Release();
	pY->Release();
	pEnum->Release();
	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionSwing

CActionSwing::CActionSwing() : m_pBody(NULL)
{
}

CActionSwing::~CActionSwing()
{
	if (m_pBody) m_pBody->Release();
}

HRESULT CActionSwing::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
	}
	return S_OK;
}

HRESULT CActionSwing::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	// params: std & body
	FWFLOAT fAngle1, fAngle2;
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); if (FAILED(h)) throw(h);

		h = pEnum->QueryFLOAT(&fAngle1);
		if (FAILED(h)) fAngle1 = DEG2RAD(40);
		h = pEnum->QueryFLOAT(&fAngle2);
		if (FAILED(h)) fAngle2 = fAngle1 * 3 / 4;
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	// create transform
	IAction *pAction = NULL;

	ITransform *pT;
	m_pBody->CreateCompatibleTransform(&pT);

	pT->FromRotationZ(fAngle1);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_SPINE, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->FromRotationZ(-fAngle2);
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pBody, BODY_LEG + BODY_RIGHT, pT };
	FWDevice()->CreateObjectEx(L"Action", L"Rotate", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->Release();
	pEnum->Release();
	return S_OK;
}

