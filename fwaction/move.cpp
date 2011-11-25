// move.cpp : Defines the Move Action
//

#include "stdafx.h"
#include "move.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define DEG2RAD(d)	( (d) * M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / M_PI )

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionMove

CActionMove::CActionMove() : m_pObj(NULL)
{
}

CActionMove::~CActionMove()
{
	if (m_pObj) m_pObj->Release();
}

HRESULT CActionMove::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		FWFLOAT fTime;
		GetPhase(pEvent, &fTime);

		FWVECTOR v = {
			m_v.x * (fTime - m_fTimePrev),
			m_v.y * (fTime - m_fTimePrev),
			m_v.z * (fTime - m_fTimePrev)
		};

		m_fTimePrev = fTime;

		ITransform *p = NULL;
		m_pObj->GetLocalTransformRef(&p);
		p->MulTranslationVector(&v);
		p->Release();
		m_pObj->Invalidate();
	}
	return S_OK;
}

HRESULT CActionMove::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineObj3D, (FWPUNKNOWN*)&m_pObj); if (FAILED(h)) throw(h);
		h = pEnum->QueryVECTOR(&m_v); if (FAILED(h)) throw(h);
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
// Class CActionMoveTo

CActionMoveTo::CActionMoveTo() : m_pObj(NULL), m_pRefObj(NULL)
{
}

CActionMoveTo::~CActionMoveTo()
{
	if (m_pObj) m_pObj->Release();
	if (m_pRefObj) m_pRefObj->Release();
}

HRESULT CActionMoveTo::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		FWFLOAT fTime;
		GetProgPhase(pEvent, &fTime);

		FWVECTOR v0 = {0, 0, 0};
		m_pObj->LtoG(&v0);

		FWVECTOR v1 = { m_v.x, m_v.y, m_v.z };
		if (m_pRefObj)
			m_pRefObj->LtoG(&v1);

		FWVECTOR v = {
			(v1.x - v0.x) * fTime,
			(v1.y - v0.y) * fTime,
			(v1.z - v0.z) * fTime
		};

//		m_fTimePrev = fTime;

		ITransform *p = NULL;
		m_pObj->GetLocalTransformRef(&p);
		p->MulTranslationVector(&v);
		p->Release();
		m_pObj->Invalidate();
	}
	return S_OK;
}

HRESULT CActionMoveTo::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineObj3D, (FWPUNKNOWN*)&m_pObj); if (FAILED(h)) throw(h);
		h = pEnum->QueryVECTOR(&m_v); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_pRefObj); if (FAILED(h)) m_pRefObj = NULL;
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	pEnum->Release();
	return S_OK;
}
