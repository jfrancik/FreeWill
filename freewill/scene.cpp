// scene.cpp : Defines the Scene object
//

#include "stdafx.h"
#include "scene.h"
#include <algorithm>


///////////////////////////////////////////////////////////
// Class CScene

CScene::CScene()
{ 
	m_bOn = TRUE;
	m_pRenderer = NULL;
	m_pCamera = NULL;
	m_nOrdinalOffset = 0;
}

CScene::~CScene()
{
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pCamera) m_pCamera->Release();
}

HRESULT __stdcall CScene::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	if (!p) return ERROR(FW_E_POINTER);
	ISceneObject *pObject;
	HRESULT h = NewObject(pLabel, &pObject);
	if (FAILED(h)) return h;
	h = pObject->QueryInterface(p);
	pObject->Release();
	if (FAILED(h)) { (*p)->Release(); return ERROR(FW_E_NOINTERFACE); }
	return S_OK;
}

	bool _LESS(ISceneRenderee *p1, ISceneRenderee *p2)
	{
		FWULONG n1, n2;
		p1->GetRenderOrdinal(&n1);
		p2->GetRenderOrdinal(&n2);
		return n1 < n2;
	}

HRESULT __stdcall CScene::AddChildEx(LPOLESTR pLabel, IKineChild *p, BOOL bSetParentalDep, IKineNode **ppParentNode, FWULONG *pnId)
{
	// call inherited function, ensure an immediate child added successfully
	HRESULT h;
	if (!ppParentNode)
	{
		IKineNode *pParentNode = NULL;
		h = SCENEBASE::AddChildEx(pLabel, p, bSetParentalDep, &pParentNode, pnId);
		pParentNode->Release();
		if (FAILED(h) || pParentNode != this) return h;
	}
	else
	{
		h = SCENEBASE::AddChildEx(pLabel, p, bSetParentalDep, ppParentNode, pnId);
		if (FAILED(h) || *ppParentNode != this) return h;
	}

	ISceneRenderee *pObj = NULL;
	if (SUCCEEDED(p->QueryInterface(&pObj)) && pObj)
	{
		DISPLIST::iterator pos = std::lower_bound(m_displist.begin(), m_displist.end(), pObj, _LESS);
		m_displist.insert(pos, pObj);
		pObj->Release();
	}

	ISceneCamera *pCam = NULL;
	if (SUCCEEDED(p->QueryInterface(&pCam)) && pCam)
	{
		if (m_pCamera) m_pCamera->Release();
		m_pCamera = pCam;
	}

	return h;
}

HRESULT __stdcall CScene::DelChildAt(FWULONG nId)
{
	IKineChild *p = NULL;
	if (SUCCEEDED(GetChildAt(nId, &p)) && p)
	{
		ISceneRenderee *pObj = NULL;
		ISceneCamera *pCam = NULL;
		if (SUCCEEDED(p->QueryInterface(&pCam)) && pCam)
		{
			if (pCam == m_pCamera) { m_pCamera->Release(); m_pCamera = NULL; }
			pCam->Release();
		}

		if (SUCCEEDED(p->QueryInterface(&pObj)) && pObj)
		{
			m_displist.remove(pObj);
			pObj->Release();
		}
		p->Release();
	}
	return SCENEBASE::DelChildAt(nId);
}

/////////////////////////////////////////////////////////////////////////////
// properties

HRESULT CScene::GetRenderer(IRndrGeneric **p)
{
	if (p)
	{
		*p = m_pRenderer;
		if (*p) (*p)->AddRef();
	}
	return S_OK;
}

HRESULT CScene::PutRenderer(IRndrGeneric *p)
{
	if (!p) return S_OK;
	if (m_pRenderer) m_pRenderer->Release();
	m_pRenderer = p;
	m_pRenderer->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Objects

HRESULT CScene::NewObject(FWSTRING pLabel, /*[out, retval]*/ ISceneObject **pObject)
{
	HRESULT h = FWDevice()->CreateObject(L"SceneObject", IID_ISceneObject, (IFWUnknown**)pObject);
	if (FAILED(h)) return h;
	return AddObject(pLabel, *pObject);
}

HRESULT CScene::GetCurrentCamera(ISceneCamera **p)
{
	if (!p) return S_OK;

	// try to recover a camera if none known....
	if (!m_pCamera)
		for (DISPLIST::iterator i = m_displist.begin(); i != m_displist.end(); i++)
		{
			ISceneCamera *pCam = NULL;
			if (SUCCEEDED((*i)->QueryInterface(&pCam)) && pCam)
				m_pCamera = pCam;
		}

	if (m_pCamera)
		m_pCamera->GetCurrentCamera(p);
	else
		*p = NULL;

	return S_OK;
}

HRESULT CScene::PutCamera(ISceneCamera *p)
{
	if (m_pCamera) m_pCamera->Release();
	m_pCamera = p;
	if (m_pCamera) m_pCamera->AddRef();
	if (m_pCamera) m_pCamera->PutVisible(TRUE);
	return S_OK;
}

HRESULT CScene::AddObject(FWSTRING pLabel, ISceneObject *pObject)
{
	HRESULT h;
	if (!m_pRenderer)
		return ERROR(FW_E_NOTREADY);

	if (!pObject) return ERROR(FW_E_POINTER);

	pObject->PutRenderer(m_pRenderer);

    h = AddChildEx(pLabel, pObject, TRUE, NULL, NULL);
	if (FAILED(h)) return h;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// rendering

HRESULT CScene::Render(IRndrGeneric *pRenderer)
{
	if (!m_bOn || !pRenderer) return S_OK;

	BOOL bReset;
	pRenderer->GetResetFlag(&bReset);

	// first render the current camera
	ISceneCamera *pCamera = NULL;
	GetCurrentCamera(&pCamera);
	if (pCamera)
	{
		if (pCamera->NeedsRendering(bReset) == S_OK)
			pCamera->Render(pRenderer);
		pCamera->Release();
	}

	// render all other objects from the display list
	for (DISPLIST::iterator i = m_displist.begin(); i != m_displist.end(); i++)
	{
		ISceneRenderee *pRenderee = *i;
		if (pRenderee->NeedsRendering(bReset) == S_OK)
			pRenderee->Render(pRenderer);
	}

	return S_OK;
}

HRESULT CScene::Turnoff(IRndrGeneric *pRenderer)
{
	if (!pRenderer) return S_OK;

	for (DISPLIST::iterator i = m_displist.begin(); i != m_displist.end(); i++)
		(*i)->Turnoff(pRenderer);

	return S_OK;
}

HRESULT _stdcall CScene::SortDisplayList()
{
	m_displist.sort(_LESS);
	return S_OK;
}
