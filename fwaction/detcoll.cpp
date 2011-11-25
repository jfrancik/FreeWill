// detcoll.cpp : Defines the DetColl Action
//

#include "stdafx.h"
#include "detcoll.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define DEG2RAD(d)	( (d) * M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / M_PI )

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CActionDetColl

CActionDetColl::CActionDetColl()
{
	m_ppBox1 = m_ppBox2 = NULL;
	m_nLen1 = m_nLen2 = 0;
	m_bColl = FALSE;
}

CActionDetColl::~CActionDetColl()
{
	if (m_ppBox1)
	{
		for (FWULONG i = 0; i < m_nLen1; i++)
			m_ppBox1[i]->Release();
		delete [] m_ppBox1;
	}
	if (m_ppBox2)
	{
		FWULONG n = m_nLen2 ? m_nLen2 : m_nLen1;
		for (FWULONG i = 0; i < n; i++)
			m_ppBox2[i]->Release();
		delete [] m_ppBox2;
	}
}

BOOL CActionDetColl::Test()
{
	FWULONG i, j;
	BOOL bColl = false;

	if (m_nLen2)
		for (i = 0; !bColl && i < m_nLen1; i++)
			for (j = 0; !bColl && j < m_nLen2; j++)
				bColl = m_ppBox1[i]->Detect(m_ppBox2[j]) == S_FALSE;
	else
		for (i = 0; !bColl && i < m_nLen1; i++)
			bColl = m_ppBox1[i]->Detect(m_ppBox2[i]) == S_FALSE;

	return bColl;
}

HRESULT CActionDetColl::HandleEvent(struct ACTION_EVENT *pEvent)
{
	if (pEvent->nEvent == EVENT_TICK && m_ppBox1 && m_ppBox2)
	{
		BOOL bColl = Test();

		if (bColl)
			RaiseEvent(pEvent->nTimeStamp, EVENT_COLLISION, 1, 0);
		else
		if (m_bColl)
			RaiseEvent(pEvent->nTimeStamp, EVENT_COLLISION, 0, 0);

		m_bColl = bColl;
	}
	return S_OK;
}

HRESULT CActionDetColl::Create(IFWCreateContext *p)
{
	return S_OK;
}

/*
HRESULT CActionDetColl::Create(IAction *pTickSource, 
						  IKineNode *pNode1, FWSTRING pStr1,
						  IKineNode *pNode2, FWSTRING pStr2)
{
	if (!pNode1 || !pNode2)
		return ERROR(ACTION_E_CANNOTSETUP);

	HRESULT h = SubscribeTicks(pTickSource);
	if (FAILED(h)) return h;

	m_ppBox1 = new IBounding*;
	m_ppBox2 = new IBounding*;
	m_nLen1 = 1;
	m_nLen2 = 0;

	// the boxes
	h = ResolveNode(pNode1, pStr1, IID_IBounding, (void**)m_ppBox1);
	if (FAILED(h)) return h;
	h = ResolveNode(pNode2, pStr2, IID_IBounding, (void**)m_ppBox2);
	if (FAILED(h)) return h;

	return S_OK;
}

HRESULT CActionDetColl::Create(IAction *pTickSource, 
						IKineNode *pNode1, FWSTRING ppStr1[], FWULONG n1,
						IKineNode *pNode2, FWSTRING ppStr2[], FWULONG n2)
{
	if (!pNode1 || !pNode2 || !n1)
		return ERROR(ACTION_E_CANNOTSETUP);
	if (!n2) return Create(pTickSource, pNode1, ppStr1, pNode2, ppStr2, n1);

	HRESULT h = SubscribeTicks(pTickSource);
	if (FAILED(h)) return h;

	m_nLen1 = n1;
	m_ppBox1 = new IBounding*[m_nLen1];
	memset(m_ppBox1, 0, m_nLen1 * sizeof(IBounding*));
	m_nLen2 = n2;
	m_ppBox2 = new IBounding*[m_nLen2];
	memset(m_ppBox2, 0, m_nLen2 * sizeof(IBounding*));

	FWULONG i;
	for (i = 0; SUCCEEDED(h) && i < m_nLen1; i++)
		h = ResolveNode(pNode1, ppStr1[i], IID_IBounding, (void**)&m_ppBox1[i]);
	for (i = 0; SUCCEEDED(h) && i < m_nLen2; i++)
		h = ResolveNode(pNode2, ppStr2[i], IID_IBounding, (void**)&m_ppBox2[i]);

	if (FAILED(h))
	{
		for (FWULONG i = 0; i < m_nLen1; i++)
			if (m_ppBox1[i]) m_ppBox1[i]->Release();
		delete [] m_ppBox1;
		for (FWULONG i = 0; i < m_nLen2; i++)
			if (m_ppBox2[i]) m_ppBox2[i]->Release();
		delete [] m_ppBox2;
		m_ppBox1 = m_ppBox2 = NULL;
		return h;
	}

	return S_OK;
}

HRESULT CActionDetColl::Create(IAction *pTickSource,
						IKineNode *pNode1, FWSTRING ppStr1[],
						IKineNode *pNode2, FWSTRING ppStr2[],
						FWULONG n)
{
	if (!pNode1 || !pNode2 || !n)
		return ERROR(ACTION_E_CANNOTSETUP);

	HRESULT h = SubscribeTicks(pTickSource);
	if (FAILED(h)) return h;

	m_nLen1 = n;
	m_nLen2 = 0;
	m_ppBox1 = new IBounding*[m_nLen1];
	memset(m_ppBox1, 0, m_nLen1 * sizeof(IBounding*));
	m_ppBox2 = new IBounding*[m_nLen1];
	memset(m_ppBox2, 0, m_nLen1 * sizeof(IBounding*));

	FWULONG i;
	for (i = 0; SUCCEEDED(h) && i < m_nLen1; i++)
	{
		h = ResolveNode(pNode1, ppStr1[i], IID_IBounding, (void**)&m_ppBox1[i]);
		if (SUCCEEDED(h))
			h = ResolveNode(pNode2, ppStr2[i], IID_IBounding, (void**)&m_ppBox2[i]);
	}

	if (FAILED(h))
	{
		for (FWULONG i = 0; i < m_nLen1; i++)
		{
			if (m_ppBox1[i]) m_ppBox1[i]->Release();
			if (m_ppBox2[i]) m_ppBox2[i]->Release();
		}
		delete [] m_ppBox1;
		delete [] m_ppBox2;
		m_ppBox1 = m_ppBox2 = NULL;
		return h;
	}

	return S_OK;
}

*/
