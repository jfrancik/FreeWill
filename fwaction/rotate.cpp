// rotate.cpp : Defines various Rotate Actions
//

#include "stdafx.h"
#include "rotate.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define DEG2RAD(d)	( (d) * M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / M_PI )

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionRotate

CActionRotate::CActionRotate()
{
	m_pObj = NULL;
	m_pTSrc = NULL;
	m_pTDest = NULL;
	m_pT = NULL;
}

CActionRotate::~CActionRotate()
{
	if (m_pObj) m_pObj->Release();
	if (m_pTSrc) m_pTSrc->Release();
	if (m_pTDest) m_pTDest->Release();
	if (m_pT) m_pT->Release();
}

HRESULT CActionRotate::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		FWFLOAT fTime;
		GetPhase(pEvent, &fTime);

		m_pObj->Transform(m_pT, KINE_INVERTED);
		m_pT->Interpolate(m_pTSrc, m_pTDest, fTime);
		m_pObj->TransformLocal(m_pT);
	}
	return S_OK;
}

HRESULT CActionRotate::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	ITransform *pT = NULL;
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_pObj); if (FAILED(h)) throw(h);
		h = pEnum->QueryPUNKNOWN(IID_ITransform, (FWPUNKNOWN*)&pT); if (FAILED(h)) throw(h);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	m_pT = (ITransform*)pT->Clone(IID_ITransform);
	m_pTSrc = (ITransform*)pT->Clone(IID_ITransform); 
	m_pTDest = (ITransform*)pT->Clone(IID_ITransform);
	pT->AsTransform(m_pTDest);
	pT->Release();

	pEnum->Release();
	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionRotateInv

CActionRotateInv::CActionRotateInv() : m_nLimbs(0), m_bInitialised(false)
{
	m_pPelvis = NULL;
	m_ppLimb = NULL;
	m_pTSrc = m_pT = m_pTX = NULL;
	m_ppTA = m_ppTB = m_ppTS = NULL;
	m_pm = NULL;
	m_bInitialised = false;
}

CActionRotateInv::~CActionRotateInv()
{
	if (m_pPelvis) m_pPelvis->Release();
	for (ULONG i = 0; i < m_nLimbs; i++)
	{
		if (m_ppLimb && m_ppLimb[i]) m_ppLimb[i]->Release();
		if (m_ppTA && m_ppTA[i]) m_ppTA[i]->Release();
		if (m_ppTB && m_ppTB[i]) m_ppTB[i]->Release();
		if (m_ppTS && m_ppTS[i]) m_ppTS[i]->Release();
	}
	if (m_ppLimb) delete [] m_ppLimb;
	if (m_ppTA) delete [] m_ppTA;
	if (m_ppTB) delete [] m_ppTB;
	if (m_ppTS) delete [] m_ppTS;
	if (m_pm) delete [] m_pm;
	if (m_pTSrc) m_pTSrc->Release();
	if (m_pT) m_pT->Release();
	if (m_pTX) m_pTX->Release();
}

HRESULT CActionRotateInv::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		FWFLOAT fTime;
		GetPhase(pEvent, &fTime);

		if (fTime >= 1.0f)
			fTime = 1.0f;

		if (!m_bInitialised)
		{
			for (FWULONG i = 0; i < m_nLimbs; i++)
			{
				m_ppTS[i]->FromIdentity();
				m_pPelvis->GetTransform(m_ppTS[i], KINE_GLOBAL);
				m_ppLimb[i]->Multiply(m_ppTS[i], KINE_GLOBAL | KINE_INVERTED);
			}

			ITransform *t;
			m_pPelvis->GetLocalTransformRef(&t); t->AsMatrix(m_m); t->Release();
			for (FWULONG i = 0; i < m_nLimbs; i++)
			{
				m_ppLimb[i]->GetLocalTransformRef(&t); t->AsMatrix(m_pm[i]); t->Release();
			}

			m_bInitialised = true;
		}
		else
		{
			ITransform *t;
			m_pPelvis->GetLocalTransformRef(&t); t->FromMatrix(m_m); t->Release();
			for (FWULONG i = 0; i < m_nLimbs; i++)
			{
				m_ppLimb[i]->GetLocalTransformRef(&t); t->FromMatrix(m_pm[i]); t->Release();
			}
		}

		for (FWULONG i = 0; i < m_nLimbs; i++)
		{
			m_pT->FromIdentity();
			m_pT->MulRotationY(DEG2RAD(180.0));
			m_pTX->Interpolate(m_pTSrc, m_ppTA[i], fTime); m_pT->Multiply(m_pTX);
			if (m_ppTB[i]) { m_pTX->Interpolate(m_pTSrc, m_ppTB[i], fTime); m_pT->Multiply(m_pTX); }
			m_pT->MulRotationY(DEG2RAD(-180.0));

			m_pT->MultiplyEx(m_ppTS[i], KINE_BOTH_SIDES);
			m_pT->Reset(FALSE, TRUE);

			m_pT->Orthonormalize();
			m_ppLimb[i]->Transform(m_pT, KINE_RIGHT_SIDE);

			m_pT->Inverse();
			m_pT->MultiplyEx(m_ppTS[i], KINE_BOTH_SIDES_INVERTED);

			m_pT->Orthonormalize();
			m_pPelvis->Transform(m_pT, KINE_LOCAL | KINE_RIGHT_SIDE);
		}
	}
	return S_OK;
}

HRESULT CActionRotateInv::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_pPelvis); if (FAILED(h)) throw(h);	// Pelvis

		// Test the desired number of limbs
		m_nLimbs = 0;
		IFWEnumParams *pEnumTest = NULL;
		pEnum->Clone(&pEnumTest);
		while (1)
		{
			if FAILED(pEnumTest->QueryPBONE(IID_IKineChild, NULL)) break;
			if FAILED(pEnumTest->QueryPUNKNOWN(IID_ITransform, NULL)) break;
			pEnumTest->QueryPUNKNOWN(IID_ITransform, NULL);
			m_nLimbs++;
		}
		pEnumTest->Release();

		// allocate memory
		m_ppLimb = new IKineChild*[m_nLimbs];	memset(m_ppLimb, 0, sizeof(void*) * m_nLimbs);
		m_ppTA   = new ITransform*[m_nLimbs];	memset(m_ppTA,    0, sizeof(void*) * m_nLimbs);
		m_ppTB   = new ITransform*[m_nLimbs];	memset(m_ppTB,    0, sizeof(void*) * m_nLimbs);
		m_ppTS   = new ITransform*[m_nLimbs];	memset(m_ppTS,    0, sizeof(void*) * m_nLimbs);
		m_pm     = new FWMATRIX[m_nLimbs];

		for (ULONG i = 0; i < m_nLimbs; i++)
		{
			h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_ppLimb[i]); if (FAILED(h)) throw(h);
			h = pEnum->QueryPUNKNOWN(IID_ITransform, (FWPUNKNOWN*)&m_ppTA[i]); if (FAILED(h)) throw(h);
			h = pEnum->QueryPUNKNOWN(IID_ITransform, (FWPUNKNOWN*)&m_ppTB[i]);
		}
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	// create transforms
	m_pTSrc = (ITransform*)m_ppTA[0]->Clone(IID_ITransform); 
	m_pT    = (ITransform*)m_pTSrc->Clone(IID_ITransform);
	m_pTX   = (ITransform*)m_pTSrc->Clone(IID_ITransform);
	for (ULONG i = 0; i < m_nLimbs; i++)
	{
		ITransform *pT;
		pT = m_ppTA[i]; m_ppTA[i] = (ITransform*)pT->Clone(IID_ITransform); m_ppTA[i]->FromTransform(pT); pT->Release();
		if (m_ppTB[i])
			{ pT = m_ppTB[i]; m_ppTB[i] = (ITransform*)pT->Clone(IID_ITransform); m_ppTB[i]->FromTransform(pT); pT->Release(); }
		m_ppTS[i] = (ITransform*)pT->Clone(IID_ITransform);
	}
	
	m_bInitialised = false;
	pEnum->Release();
	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionRotateTo

CActionRotateTo::CActionRotateTo()
{
	m_pObj = NULL;
	m_pRefObj = NULL;
	m_pTDest = NULL;
	m_pT = NULL;
}

CActionRotateTo::~CActionRotateTo()
{
	if (m_pObj) m_pObj->Release();
	if (m_pRefObj) m_pRefObj->Release();
	if (m_pTDest) m_pTDest->Release();
	if (m_pT) m_pT->Release();
}

HRESULT CActionRotateTo::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		FWFLOAT fTime;
		GetProgPhase(pEvent, &fTime);

		if (m_fTimePrev == 0.0) return S_OK;

		ITransform *pT;
		FWVECTOR vec;
		m_pObj->GetLocalTransformRef(&pT);
		pT->AsVector(&vec);
		pT->Release();

		ITransform *pTDest = NULL;
		if (m_pRefObj)
		{
			pTDest = (ITransform*)m_pTDest->Clone(IID_ITransform);
			pTDest->FromTransform(m_pTDest);
			m_pRefObj->Multiply(pTDest, KINE_GLOBAL);
			m_pObj->Multiply(pTDest, KINE_GLOBAL | KINE_INVERTED);
			m_pObj->Multiply(pTDest, KINE_LOCAL);
			pTDest->Reset(FALSE, TRUE);
		}
		else
		{
			pTDest = m_pTDest;
			pTDest->AddRef();
		}

		ITransform *pTSrc = NULL;
		m_pObj->GetLocalTransformRef(&pTSrc);
		m_pT->Interpolate(pTSrc, pTDest, fTime);
		m_pT->Orthonormalize();
		m_pT->MulTranslationVector(&vec);	// added 3/02/2011
		m_pObj->PutLocalTransform(m_pT);
		pTSrc->Release();
		pTDest->Release();
	}
	return S_OK;
}

HRESULT CActionRotateTo::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	ITransform *pT = NULL;
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_pObj); if (FAILED(h)) throw(h);
		h = pEnum->QueryPUNKNOWN(IID_ITransform, (FWPUNKNOWN*)&pT); if (FAILED(h)) throw(h);
		h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_pRefObj); if (FAILED(h)) throw(h);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}
	m_pT = (ITransform*)pT->Clone(IID_ITransform);
	m_pTDest = (ITransform*)pT->Clone(IID_ITransform);
	pT->AsTransform(m_pTDest);
	pT->Release();

	pEnum->Release();
	return S_OK;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionMultiRotate

CActionMultiRotate::CActionMultiRotate()
{
	m_ppObj = NULL;
	m_ppTSrc = NULL;
	m_ppTDest = NULL;
	m_ppT = NULL;
	m_nCount = 0;
}

CActionMultiRotate::~CActionMultiRotate()
{
	for (FWULONG i = 0; i < m_nCount; i++)
	{
		if (m_ppObj && m_ppObj[i]) m_ppObj[i]->Release();
		if (m_ppTSrc && m_ppTSrc[i]) m_ppTSrc[i]->Release();
		if (m_ppTDest && m_ppTDest[i]) m_ppTDest[i]->Release();
		if (m_ppT && m_ppT[i]) m_ppT[i]->Release();
	}
	if (m_ppObj) delete [] m_ppObj;
	if (m_ppTSrc) delete [] m_ppTSrc;
	if (m_ppTDest) delete [] m_ppTDest;
	if (m_ppT) delete [] m_ppT;
}

HRESULT CActionMultiRotate::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		FWFLOAT fTime;
		GetPhase(pEvent, &fTime);

		for (FWULONG i = 0; i < m_nCount; i++)
		{
			ITransform *pT = NULL;
			m_ppObj[i]->GetLocalTransformRef(&pT);
			m_ppT[i]->Inverse();
			pT->Multiply(m_ppT[i]);
			m_ppT[i]->Interpolate(m_ppTSrc[i], m_ppTDest[i], fTime);
			pT->Multiply(m_ppT[i]);
			pT->Release();
			m_ppObj[i]->Invalidate();
		}
	}
	return S_OK;
}

HRESULT CActionMultiRotate::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	// m_nCount
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);

		h = pEnum->QueryULONG(&m_nCount); if (FAILED(h)) throw(h);

		if (m_nCount)
		{
			m_ppObj = new IKineChild*[m_nCount];
			memset(m_ppObj, 0, sizeof(IKineChild*) * m_nCount);
			m_ppTSrc = new ITransform*[m_nCount];
			memset(m_ppTSrc, 0, sizeof(ITransform*) * m_nCount);
			m_ppTDest = new ITransform*[m_nCount];
			memset(m_ppTDest, 0, sizeof(ITransform*) * m_nCount);
			m_ppT = new ITransform*[m_nCount];
			memset(m_ppT, 0, sizeof(ITransform*) * m_nCount);

			for (FWULONG i = 0; i < m_nCount; i++)
			{
				ITransform *pT = NULL;
				h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_ppObj[i]); if (FAILED(h)) throw(h);
				h = pEnum->QueryPUNKNOWN(IID_ITransform, (FWPUNKNOWN*)&pT); if (FAILED(h)) throw(h);
				m_ppTSrc[i] = (ITransform*)pT->Clone(IID_ITransform);
				m_ppTDest[i] = (ITransform*)pT->Clone(IID_ITransform);
				m_ppT[i] = (ITransform*)pT->Clone(IID_ITransform);
				pT->AsTransform(m_ppTDest[i]);
				pT->Release();
			}
		}
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
// Class CActionMultiRotateTo

CActionMultiRotateTo::CActionMultiRotateTo()
{
	m_ppObj = NULL;
	m_ppTDest = NULL;
	m_pT = NULL;
	m_nCount = 0;
}

CActionMultiRotateTo::~CActionMultiRotateTo()
{
	for (FWULONG i = 0; i < m_nCount; i++)
	{
		if (m_ppObj && m_ppObj[i]) m_ppObj[i]->Release();
		if (m_ppTDest && m_ppTDest[i]) m_ppTDest[i]->Release();
	}
	if (m_ppObj) delete [] m_ppObj;
	if (m_ppTDest) delete [] m_ppTDest;
	if (m_pT) m_pT->Release();
}

HRESULT CActionMultiRotateTo::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK)
	{
		FWFLOAT fTime;
		GetProgPhase(pEvent, &fTime);

		if (m_fTimePrev == 0.0) return S_OK;

		for (FWULONG i = 0; i < m_nCount; i++)
		{
			ITransform *pTSrc = NULL;
			m_ppObj[i]->GetLocalTransformRef(&pTSrc);
			m_pT->Interpolate(pTSrc, m_ppTDest[i], fTime);
			m_ppObj[i]->PutLocalTransform(m_pT);
			pTSrc->Release();
		}
	}
	return S_OK;
}

HRESULT CActionMultiRotateTo::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	// m_nCount
	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);

		h = pEnum->QueryULONG(&m_nCount); if (FAILED(h)) throw(h);

		if (m_nCount)
		{
			m_ppObj = new IKineChild*[m_nCount];
			memset(m_ppObj, 0, sizeof(IKineChild*) * m_nCount);
			m_ppTDest = new ITransform*[m_nCount];
			memset(m_ppTDest, 0, sizeof(ITransform*) * m_nCount);

			for (FWULONG i = 0; i < m_nCount; i++)
			{
				ITransform *pT = NULL;
				h = pEnum->QueryPBONE(IID_IKineChild, (FWPUNKNOWN*)&m_ppObj[i]); if (FAILED(h)) throw(h);
				h = pEnum->QueryPUNKNOWN(IID_ITransform, (FWPUNKNOWN*)&pT); if (FAILED(h)) throw(h);
				m_ppTDest[i] = (ITransform*)pT->Clone(IID_ITransform);
				pT->AsTransform(m_ppTDest[i]);
				pT->Release();
			}
		}
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	m_pT = (ITransform*)m_ppTDest[0]->Clone(IID_ITransform);

	pEnum->Release();
	return S_OK;
}

