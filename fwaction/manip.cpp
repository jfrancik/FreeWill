// manip.cpp : Defines the Object Manipulation Actions
//

#include "stdafx.h"
#include "manip.h"
#include "freewilltools.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define DEG2RAD(d)	( (d) * (FWFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (FWFLOAT)M_PI )

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionGrasp

CActionGrasp::CActionGrasp() : m_pBody(NULL)
{
}

CActionGrasp::~CActionGrasp()
{
	if (m_pBody) m_pBody->Release();
}

HRESULT CActionGrasp::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
	}
	return S_OK;
}

HRESULT CActionGrasp::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);
	
	// params: std & body
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); if (FAILED(h)) throw(h);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	// parse the style
	FWULONG BODY_SIDE = (IsStyle(L"left") == S_OK) ? BODY_LEFT : BODY_RIGHT;

	// rotate hand
	ITransform *pT = NULL;
	m_pBody->CreateCompatibleTransform(&pT);
	pT->FromIdentity();
	pT->MulRotationZ(DEG2RAD(-15));
	pT->MulRotationX(DEG2RAD(90));
	IAction *pAction = NULL;
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pStyle, m_pBody, BODY_HAND + BODY_SIDE, pT, BODY_ROOT };
	FWDevice()->CreateObjectEx(L"Action", L"RotateTo", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();
	pT->Release();

	// bend fingers
	ITransform *pT45 = NULL, *pT15 = NULL, *pT30 = NULL;
	m_pBody->CreateCompatibleTransform(&pT45); 
	m_pBody->CreateCompatibleTransform(&pT15);
	m_pBody->CreateCompatibleTransform(&pT30);
	pT45->FromRotationZ(DEG2RAD(45)); 
	pT15->FromRotationZ(DEG2RAD(15)); 
	pT30->FromRotationZ(DEG2RAD(30)); 
	CParam params[] = { this, m_nStartTime + m_nPeriod/2, m_nPeriod/2, m_pStyle, 
		12,
		m_pBody, 
		BODY_SIDE + BODY_FINGER + BODY_1 + 0, pT45,
		BODY_SIDE + BODY_FINGER + BODY_1 + 1, pT15,
		BODY_SIDE + BODY_FINGER + BODY_1 + 2, pT30,
		BODY_SIDE + BODY_FINGER + BODY_2 + 0, pT45,
		BODY_SIDE + BODY_FINGER + BODY_2 + 1, pT15,
		BODY_SIDE + BODY_FINGER + BODY_2 + 2, pT30,
		BODY_SIDE + BODY_FINGER + BODY_3 + 0, pT45,
		BODY_SIDE + BODY_FINGER + BODY_3 + 1, pT15,
		BODY_SIDE + BODY_FINGER + BODY_3 + 2, pT30,
		BODY_SIDE + BODY_FINGER + BODY_4 + 0, pT45,
		BODY_SIDE + BODY_FINGER + BODY_4 + 1, pT15,
		BODY_SIDE + BODY_FINGER + BODY_4 + 2, pT30,
	};
	FWDevice()->CreateObjectEx(L"Action", L"MultiRotateTo", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);
	pAction->Release();
	pT45->Release();
	pT15->Release();
	pT30->Release();

	pEnum->Release();
	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionTake

CActionTake::CActionTake() : m_pBody(NULL), m_pGoal(NULL)
{
}

CActionTake::~CActionTake()
{
	if (m_pBody) m_pBody->Release();
	if (m_pGoal) m_pGoal->Release();
}

HRESULT CActionTake::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
	}
	return S_OK;
}

HRESULT CActionTake::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	// params: std, body & goal
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_pGoal); if (FAILED(h)) throw(h);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	// parse the style
	FWULONG BODY_SIDE = (IsStyle(L"left") == S_OK) ? BODY_LEFT : BODY_RIGHT;

	IAction *pAction;

	// fire the GRASP action
	{CParam params[] = { this, m_nStartTime, m_nPeriod, m_pStyle, m_pBody };
	FWDevice()->CreateObjectEx(L"Action", L"Grasp", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->SetEnvelope(ACTION_ENV_PARA, 1.0f, 1.0f);
	pAction->Release();

	// Fire the REACH action
	FWVECTOR v = { 0.9f, 0.0f, 1.8f };	// hard-coded for the ball
	{ CParam params[] = { 
		this, m_nStartTime, m_nPeriod, m_pStyle, m_pBody, BODY_SIDE + BODY_FINGER + BODY_MIDDLE, m_pGoal, v };
	FWDevice()->CreateObjectEx(L"Action", L"Reach", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pEnum->Release();
	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionPoint

CActionPoint::CActionPoint() : m_pBody(NULL), m_pGoal(NULL)
{
}

CActionPoint::~CActionPoint()
{
	if (m_pBody) m_pBody->Release();
	if (m_pGoal) m_pGoal->Release();
}

HRESULT CActionPoint::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
	}
	return S_OK;
}

HRESULT CActionPoint::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);
	
	// params: std, body, goal, vector
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBODY(IID_IBody, (FWPUNKNOWN*)&m_pBody); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_pGoal); if (FAILED(h)) throw(h);
		memset(&m_vec, 0, sizeof(FWVECTOR));
		h = pEnum->QueryVECTOR(&m_vec);		// optional - ignore if unsuccessful
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	// parse the style
	FWULONG BODY_SIDE = (IsStyle(L"left") == S_OK) ? BODY_LEFT : BODY_RIGHT;

	IAction *pAction;

	// Fire the REACH action
	{ CParam params[] = { 
		this, m_nStartTime, m_nPeriod, m_pStyle, m_pBody, BODY_RIGHT + BODY_FINGER + BODY_INDEX, m_pGoal, m_vec };
	FWDevice()->CreateObjectEx(L"Action", L"Reach", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// configure the fingers
	// bend fingers
	ITransform *pT45 = NULL, *pT15 = NULL, *pT30 = NULL;
	m_pGoal->CreateCompatibleTransform(&pT45); 
	m_pGoal->CreateCompatibleTransform(&pT15);
	m_pGoal->CreateCompatibleTransform(&pT30);
	pT45->FromRotationZ(DEG2RAD(80)); 
	pT15->FromRotationZ(DEG2RAD(85)); 
	pT30->FromRotationZ(DEG2RAD(30)); 
	CParam params[] = { this, m_nStartTime + m_nPeriod/2, m_nPeriod/2, m_pStyle, 
		9,
		m_pBody, 
		BODY_SIDE + BODY_FINGER + BODY_1 + 0, pT45,
		BODY_SIDE + BODY_FINGER + BODY_1 + 1, pT15,
		BODY_SIDE + BODY_FINGER + BODY_1 + 2, pT30,
		BODY_SIDE + BODY_FINGER + BODY_3 + 0, pT45,
		BODY_SIDE + BODY_FINGER + BODY_3 + 1, pT15,
		BODY_SIDE + BODY_FINGER + BODY_3 + 2, pT30,
		BODY_SIDE + BODY_FINGER + BODY_4 + 0, pT45,
		BODY_SIDE + BODY_FINGER + BODY_4 + 1, pT15,
		BODY_SIDE + BODY_FINGER + BODY_4 + 2, pT30,
	};
	FWDevice()->CreateObjectEx(L"Action", L"MultiRotateTo", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);
	pAction->Release();
	pT45->Release();
	pT15->Release();
	pT30->Release();

	pEnum->Release();
	return S_OK;
}
